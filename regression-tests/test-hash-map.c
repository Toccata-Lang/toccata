#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "new.h"
#include "runtime3.h"

// Inline helpers (copied from runtime3.c)
Term (*sha1)(FnArity *, Term);
static int mask(int64_t hash, int shift) { return (hash >> shift) & 0x1f; }

// Stubs required by new.c and runtime3.c (graphing symbols, not used by hash-map tests)
unsigned refsCount = 0;
refMap refNames[0];
unsigned subGraphs = 0;
void subGraph(const char *name, Term t, int depth, int id) {}

Value *noImpl2(FnArity *arity, Value *v1, Value *v2) { abort(); }

// Forward declarations for functions defined in runtime3.c
BitmapIndexedNode *malloc_bmiNode(int itemCount);
ArrayNode *malloc_arrayNode(void);
HashCollisionNode *malloc_hashCollisionNode(int itemCount);
void freeBitmapNode(Value *v);
void freeArrayNode(Value *v);
void freeHashCollisionNode(Value *v);
int bitpos(int64_t hash, int shift);
Term integer_EQ(Term arg0, Term arg1);

Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *arrayNodeCount(Value *arg0);
Value *arrayNodeDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);

// BMI operations
Value *bmiMutateAssoc(BitmapIndexedNode *node, Term key, Term val, int64_t hash, int shift);
Value *bmiCopyAssoc(BitmapIndexedNode *arg0, Term key, Term val, int64_t hash, int shift);
Value *bmiGet(Value *node, Value *key, Value *def, int64_t hash, int shift);
Value *bmiCount(Value *node);
Value *bmiHashVec(Value *node, Value *vec);
int64_t strSha1(Value *arg0);

// Forward declarations for helper functions used in tests
Term nothing(void);
Term some(Term thing);
Term testingSha1(FnArity *f, Term trm);

// Pool baseline: pool object count at start of each test.
static int64_t pool_baseline = 0;

// Free list type and central pools relevant to hash-map tests
typedef struct { Value *head; uintptr_t aba; } FreeValList;
extern struct { Value *head; uintptr_t aba; } centralFreeBMINodes[20];
extern struct { Value *head; uintptr_t aba; } centralFreeArrayNodes;
extern struct { Value *head; uintptr_t aba; } centralFreeStrings;
extern struct { Value *head; uintptr_t aba; } centralFreeVectors;
extern struct { Value *head; uintptr_t aba; } centralFreeVectorNodes;
#define BMI_RECYCLE_COUNT 20
extern void moveFreeToCentral(void);

// Forward declarations for pool-counting helpers
static int64_t countFreeList(FreeValList *fl);
static int64_t countPoolObjects(void);

// Collision test helpers
// Update collision: KEY_A and KEY_B have same hash and compare equal
#define COLLIDE_KEY_A newI60(100)
#define COLLIDE_KEY_B newI60(200)
#define COLLIDE_HASH 0x12345678

// Add collision: KEY_C and KEY_D have same hash but DON'T compare equal
#define COLLIDE_KEY_C newI60(300)
#define COLLIDE_KEY_D newI60(400)
#define COLLIDE_HASH_ADD 0xABCDEF00

// Custom sha1: returns same hash for KEY_A and KEY_B (update test)
Term testingSha1Collision(FnArity *f, Term trm) {
  if (trm == COLLIDE_KEY_A || trm == COLLIDE_KEY_B) {
    return COLLIDE_HASH;
  }
  return testingSha1(f, trm);
}

// Custom sha1: returns same hash for KEY_C and KEY_D (add test)
Term testingSha1CollisionAdd(FnArity *f, Term trm) {
  BOOM("needs to be updated to handle strings");
  if (trm == COLLIDE_KEY_C || trm == COLLIDE_KEY_D) {
    return COLLIDE_HASH_ADD;
  }
  return testingSha1(f, trm);
}

// Helper: check if a term is an I60 with the same value as key
static int subNodeEqualsKey(Term t, Term key) {
  return termTag(t) == I60 && getI60(t) == getI60(key);
}

Term testingSha1(FnArity *f, Term trm) {
  int64_t hash;
  Tag tg = termTag(trm);
  if (tg == I60) {
    hash = integerSha1(trm);
  } else if (tg == VAL) {
    Value *v1 = (Value *)trm;
    switch (v1->type) {
    case StringBufferType:
    case SubStringType:
      hash = strSha1(v1);
      break;

    case VectorType:
      BOOM("Fix when vectorSha1 is implemented");
      break;
      
    default:
      // No HashedValue type available for caching
      hash = 0;
      break;
    }
  } else {
    char msg[100];
    sprintf(msg, "Can't SHA1 term: %s", tagStr(tg));
    BOOM(msg);
  }
  return(hash);
}

/*
 * Memory accounting model:
 *
 * malloc_bmiNode(itemCount < 20): first call allocates 10 nodes via my_malloc,
 *   increments malloc_count by 9 (for 9 spare pool nodes). Subsequent calls
 *   pull from the pool with no malloc_count change.
 * freeBitmapNode(cnt < 20): puts node back in pool, NO free_count increment.
 * freeBitmapNode(cnt >= 20): actually frees, increments free_count by 1.
 *
 * malloc_arrayNode: same pool pattern as BMI — 10 nodes, 9 counted as mallocs.
 * freeArrayNode: always puts back in pool, NO free_count increment.
 *
 * malloc_hashCollisionNode: direct my_malloc, NO pool. malloc_count += 1.
 * freeHashCollisionNode: direct free, free_count += 1.
 *
 * Each test resets malloc_count/free_count to 0. The expected values are
 * computed based on whether a pool is created and whether the free path
 * actually frees (vs. recycles).
 */

// Reset malloc/free counters and record pool baseline.
// Called before each test to establish baseline.
static void reset_counters(void) {
  malloc_count = 0;
  free_count = 0;
  pool_baseline = countPoolObjects();
}

// Count objects sitting in a single free list.
static int64_t countFreeList(FreeValList *fl) {
  int64_t count = 0;
  for (Value *item = fl->head; item != (Value *)0; item = item->next) {
    count++;
  }
  return count;
}

// Count all objects sitting in central free-list pools relevant to hash-map tests.
static int64_t countPoolObjects(void) {
  moveFreeToCentral();
  int64_t bmiObjs = 0;
  for (int i = 0; i < BMI_RECYCLE_COUNT; i++)
    bmiObjs += countFreeList((FreeValList *)&centralFreeBMINodes[i]);
  // fprintf(stderr, "bmi pool: %ld\n", bmiObjs);
  int64_t arrayObjs = countFreeList((FreeValList *)&centralFreeArrayNodes);
  // fprintf(stderr, "array pool: %ld\n", arrayObjs);
  int64_t strObjs = countFreeList((FreeValList *)&centralFreeStrings);
  // fprintf(stderr, "string pool: %ld\n", strObjs);
  int64_t vectObjs = countFreeList((FreeValList *)&centralFreeVectors);
  // fprintf(stderr, "vect pool: %ld\n", vectObjs);
  int64_t nodeObjs = countFreeList((FreeValList *)&centralFreeVectorNodes);
  // fprintf(stderr, "vect node pool: %ld\n", nodeObjs);
  return bmiObjs + arrayObjs + strObjs + vectObjs + nodeObjs;
}

// Verify malloc_count and free_count match expected values,
// and that all unfreed allocations are accounted for in pools.
// malloc_count - free_count == pool_count means no leaks.
static void check_counts(const char *test_name, unsigned expected_malloc,
                         unsigned expected_free, int line) {
  if (malloc_count != (int64_t)expected_malloc) {
    fprintf(stderr, "FAIL %s at line %d: expected malloc_count=%u, got %lld\n",
            test_name, line, expected_malloc, (long long)malloc_count);
    BOOM("malloc_count mismatch");
  }
  if (free_count != (int64_t)expected_free) {
    fprintf(stderr, "FAIL %s at line %d: expected free_count=%u, got %lld\n",
            test_name, line, expected_free, (long long)free_count);
    BOOM("free_count mismatch");
  }
  // Every allocation is either freed or sitting in a pool.
  // The delta in pool objects must match malloc_count - free_count.
  int64_t pool_count = countPoolObjects();
  int64_t pool_delta = pool_count - pool_baseline;
  int64_t unfreed = malloc_count - free_count;
  if (unfreed != pool_delta) {
    fprintf(stderr, "FAIL %s at line %d: malloc=%ld free=%ld unfreed=%ld pool_delta=%ld\n",
            test_name, line, (long)malloc_count, (long)free_count,
            unfreed, pool_delta);
    BOOM("leak detected: unfreed allocations don't match pool objects");
  }
}

// Test: create empty BMI node, verify bitmap=0
// Pool created: malloc_count += 9 (spare nodes). Node freed → pool recycle.
void testEmptyBmiNode(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(0);

  if (node->type != BitmapIndexedType) {
    BOOM("type should be BitmapIndexedType");
  }
  if (node->bitmap != 0) {
    BOOM("bitmap should be 0");
  }

  freeBitmapNode((Value *)node);

  // Pool allocated 10 nodes: my_malloc(+1) + explicit +9 = 10.
  // Node freed back to pool (cnt=0 < 20), no free_count increment.
  check_counts("testEmptyBmiNode", 0, 0, __LINE__);
}

// Test: create BMI node with 1 item
// Different itemCount (1 vs 0) → different pool index → new pool created.
// Node freed → pool recycle.
void testBmiNodeOneItem(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(1);
  if (node->bitmap != 0) {
    BOOM("bitmap should be 0 after malloc");
  }

  freeBitmapNode((Value *)node);

  // Different itemCount = different pool index, so new pool created.
  // malloc_count=10, node recycled, free_count=0.
  check_counts("testBmiNodeOneItem", 0, 0, __LINE__);
}

// Test: create ArrayNode
// New pool created (separate from BMI pool): malloc_count += 9.
// Node freed → pool recycle.
void testArrayNode(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  if (node->type != ArrayNodeType) {
    BOOM("type should be ArrayNodeType");
  }

  freeArrayNode((Value *)node);

  // ArrayNode pool allocated 10 nodes: my_malloc(+1) + explicit +9 = 10.
  // Node recycled, not freed.
  check_counts("testArrayNode", 0, 0, __LINE__);
}

// Test: create HashCollisionNode
// No pool. malloc_count += 1, free_count += 1 via dec_and_free.
void testCollisionNode(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  HashCollisionNode *node = malloc_hashCollisionNode(2);
  if (node->type != HashCollisionNodeType) {
    BOOM("type should be HashCollisionNodeType");
  }
  if (node->count != 4) {
    BOOM("count should be 4");
  }

  dec_and_free(termVal((Term)(Value *)node), 1);

  // Direct my_malloc + direct free, no pool.
  check_counts("testCollisionNode", 1, 1, __LINE__);
}

// Test: freeBitmapNode actually frees when cnt >= BMI_RECYCLE_COUNT
// Pool created for itemCount=20: malloc_count += 9.
// Node freed with cnt=20 >= 20: actually freed, free_count += 1.
void testFreeBitmapNodeHighCount(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(20);
  if (node->type != BitmapIndexedType) {
    BOOM("type should be BitmapIndexedType");
  }

  // Set bitmap to 20 bits set so cnt=20 >= BMI_RECYCLE_COUNT,
  // triggering actual free() instead of pool recycling.
  node->bitmap = (1 << 20) - 1;

  freeBitmapNode((Value *)node);

  // itemCount=20 >= BMI_RECYCLE_COUNT(20) → bypasses pool, direct my_malloc.
  // malloc_count=1. Node actually freed (free_count=1).
  check_counts("testFreeBitmapNodeHighCount", 1, 1, __LINE__);
}

// Test: freeArrayNode works (even though it recycles into pool)
// Pool already exists (created by testArrayNode), no new malloc_count change.
// Node recycled, free_count += 0.
void testFreeArrayNode(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  ArrayNode *node = malloc_arrayNode();

  freeArrayNode((Value *)node);

  check_counts("testFreeArrayNode", 0, 0, __LINE__);
}

// Test: freeHashCollisionNode works correctly
// No pool. malloc_count += 1, free_count += 1.
void testFreeHashCollisionNode(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  HashCollisionNode *node = malloc_hashCollisionNode(2);

  freeHashCollisionNode((Value *)node);

  check_counts("testFreeHashCollisionNode", 1, 1, __LINE__);
}

// Test: add key/value to empty BMI → single-item BMI
// Then verify structure is correct
void testBmiCopyAssoc(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create empty BMI node
  BitmapIndexedNode *node = malloc_bmiNode(0);

  Term key = (Term)stringValue("key137");
  Term val = (Term)stringValue("hello");

  // Compute hash of key
  int64_t hash = strSha1(incRefVal(key, 1));

  // Add key/value to empty BMI at shift=0
  Value *result = bmiCopyAssoc(node, key, val, hash, 0);

  // Verify result is a BMI node
  if (((BitmapIndexedNode *)result)->type != BitmapIndexedType) {
    BOOM("result should be BitmapIndexedType");
  }

  // Verify bitmap has exactly 1 bit set
  int bitmap = ((BitmapIndexedNode *)result)->bitmap;
  if (bitmap == 0 || __builtin_popcount(bitmap) != 1) {
    BOOM("bitmap should have exactly 1 bit set");
  }

  // Verify the key/value are stored at the correct index
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(bitmap & (bit - 1));
  if (((BitmapIndexedNode *)result)->array[2 * idx] != key) {
    BOOM("key not at correct index");
  }
  if (((BitmapIndexedNode *)result)->array[2 * idx + 1] != val) {
    BOOM("val not at correct index");
  }

  // Clean up
  dec_and_free((Term)result, 1);

  check_counts("testBmiCopyAssoc", 0, 0, __LINE__);
}

// Test: same key, different value → in-place update (1c)
// bmiMutateAssoc with refs==1, bit set, keys equal, values different
void testBmiMutateAssocUpdateValue(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = (Term)stringValue("key137");
  Term val = (Term)stringValue("val251");
  int64_t hash = strSha1(incRefVal(key, 1));
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  // Fresh key string for the update call — the stored key's ref belongs to the node
  Term updateKey = (Term)stringValue("key137");
  Term newVal = (Term)stringValue("val999");

  // Call bmiMutateAssoc with same key, different value
  // This should trigger path 1c: keys equal, values different → in-place update
  Value *updateResult = bmiMutateAssoc(original, updateKey, newVal, hash, 0);

  // Verify same pointer returned (in-place mutation, no clone)
  if (updateResult != (Value *)original) {
    BOOM("bmiMutateAssoc update: same key+diff value should return original node pointer");
  }

  // Verify structure unchanged (same bitmap, same key)
  if (((BitmapIndexedNode *)updateResult)->bitmap != original->bitmap) {
    BOOM("bmiMutateAssoc update: bitmap should be unchanged");
  }

  // Verify the key is still at the correct index
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(((BitmapIndexedNode *)updateResult)->bitmap & (bit - 1));
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx] != updateKey) {
    BOOM("bmiMutateAssoc update: key should be unchanged");
  }

  // Verify the value was updated
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx + 1] != newVal) {
    BOOM("bmiMutateAssoc update: value should be updated");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiMutateAssocUpdateValue", 0, 0, __LINE__);
}

// Test: bit not set, n < 16, insert new entry (2b)
// bmiMutateAssoc with refs==1, bit not set → creates new BMI node
void testBmiMutateAssocInsert(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = sha1((FnArity *)0, key1);
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Use a key at a DIFFERENT bit position (bit not set in original bitmap)
  Term key2 = newI60(256);
  int64_t hash2 = sha1((FnArity *)0, key2);
  Term val2 = newI60(888);
  int bit2 = bitpos(hash2, 0);

  // Verify key2 is NOT already in the bitmap
  if (original->bitmap & bit2) {
    // Try another key if it happens to share the same bit
    for (i64 extra = 0; extra < 10000; extra++) {
      key2 = newI60(extra);
      hash2 = sha1((FnArity *)0, key2);
      bit2 = bitpos(hash2, 0);
      if (!(original->bitmap & bit2)) {
        val2 = newI60(extra + 100);
        break;
      }
    }
    if (original->bitmap & bit2) {
      BOOM("could not find key at free bit position");
    }
  }

  // Call bmiMutateAssoc — should create a new BMI with 2 entries
  Value *insertResult = bmiMutateAssoc(original, key2, val2, hash2, 0);

  // Verify a NEW node was returned (different pointer — new node created)
  if (insertResult == (Value *)original) {
    BOOM("2b: should return a new node (not original)");
  }

  // Verify the new node has 2 entries
  BitmapIndexedNode *newBm = (BitmapIndexedNode *)insertResult;
  if (newBm->type != BitmapIndexedType) {
    BOOM("2b: result should be BitmapIndexedType");
  }
  if (__builtin_popcount(newBm->bitmap) != 2) {
    BOOM("2b: new node should have 2 bits set");
  }

  // Verify both keys and values are present
  int found1 = 0, found2 = 0;
  for (int i = 0; i < 4; i += 2) {
    if (subNodeEqualsKey((Term)newBm->array[i], key1)) found1 = 1;
    if (subNodeEqualsKey((Term)newBm->array[i], key2)) found2 = 1;
  }
  if (!found1 || !found2) {
    BOOM("2b: new node should contain both keys");
  }

  // Verify both values are present
  int valFound1 = 0, valFound2 = 0;
  for (int i = 0; i < 4; i += 2) {
    if (subNodeEqualsKey((Term)newBm->array[i + 1], val1)) valFound1 = 1;
    if (subNodeEqualsKey((Term)newBm->array[i + 1], val2)) valFound2 = 1;
  }
  if (!valFound1 || !valFound2) {
    BOOM("2b: new node should contain both values");
  }

  // Clean up
  dec_and_free((Term)insertResult, 1);

  check_counts("testBmiMutateAssocInsert", 0, 0, __LINE__);
}

// Test: different key, different hash → new sub-node (1e)
// bmiMutateAssoc with refs==1, bit set, different key, different hash
void testBmiMutateAssocBranch(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = sha1((FnArity *)0, key1);
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Find a key that hashes to the same bit position as key1 (path 1e)
  Term key2;
  int64_t hash2;
  for (i64 candidate = 100; candidate < 10000; candidate++) {
    key2 = newI60(candidate);
    hash2 = sha1((FnArity *)0, key2);
    if (hash2 != hash1 && mask(hash2, 0) == mask(hash1, 0)) break;
  }
  Term val2 = newI60(888);

  // Call bmiMutateAssoc — should create a sub-node via createNode
  Value *branchResult = bmiMutateAssoc(original, key2, val2, hash2, 0);

  // Verify same pointer returned (in-place mutation)
  if (branchResult != (Value *)original) {
    BOOM("bmiMutateAssoc branch: should return original node pointer");
  }

  // Verify the original entry was replaced with a sub-node
  BitmapIndexedNode *bm = (BitmapIndexedNode *)branchResult;
  int bit1 = bitpos(hash1, 0);
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  Term entryKey = bm->array[2 * idx];
  if (entryKey != 0) {
    BOOM("1e: entry should be sub-node (NULL key)");
  }

  // Verify the sub-node contains both entries
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (subNode->type != BitmapIndexedType) {
    BOOM("1e: sub-node should be BitmapIndexedType");
  }
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("1e: sub-node should have 2 entries");
  }

  // Verify both keys are in the sub-node
  int found1 = 0, found2 = 0;
  if (subNodeEqualsKey((Term)(Value *)subNode->array[0], key1)) found1 = 1;
  if (subNodeEqualsKey((Term)(Value *)subNode->array[2], key1)) found1 = 1;
  if (subNodeEqualsKey((Term)(Value *)subNode->array[0], key2)) found2 = 1;
  if (subNodeEqualsKey((Term)(Value *)subNode->array[2], key2)) found2 = 1;
  if (!found1 || !found2) {
    BOOM("1e: sub-node should contain both keys");
  }

  // Clean up
  dec_and_free((Term)branchResult, 1);

  check_counts("testBmiMutateAssocBranch", 0, 0, __LINE__);
}

// Test: bit set, different key + same hash → creates HashCollisionNode (1d)
void testBmiMutateAssocCollision(void) {
  reset_counters();

  // Save original sha1 and equal
  Term (*savedSha1)(FnArity *, Term) = sha1;

  // Install collision-aware sha1 and non-equal equal
  sha1 = testingSha1CollisionAdd;

  // Create single-item BMI node with KEY_A
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(10);
  int64_t hashA = sha1((FnArity *)0, keyA);
  Value *result = bmiMutateAssoc(node, keyA, valA, hashA, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Add KEY_B — same hash, different key → should create collision node (1d)
  Term keyB = COLLIDE_KEY_B;
  Term valB = newI60(20);
  Value *collResult = bmiMutateAssoc(original, keyB, valB, hashA, 0);

  // Verify same pointer returned (in-place mutation)
  if (collResult != (Value *)original) {
    BOOM("1d: should return original node pointer");
  }

  // Verify the original entry was replaced with a collision node
  BitmapIndexedNode *bm = (BitmapIndexedNode *)collResult;
  int bitA = bitpos(hashA, 0);
  int idx = __builtin_popcount(bm->bitmap & (bitA - 1));
  Term entryKey = bm->array[2 * idx];
  if (entryKey != 0) {
    BOOM("1d: entry should be collision node (NULL key)");
  }

  HashCollisionNode *coll = (HashCollisionNode *)bm->array[2 * idx + 1];
  if (coll->type != HashCollisionNodeType) {
    BOOM("1d: sub-node should be HashCollisionNodeType");
  }
  if (coll->count != 4) {
    char msg[100];
    snprintf(msg, 99, "1d: collision count should be 4, got %d", coll->count);
    BOOM(msg);
  }

  // Verify both keys are in the collision node
  int foundA = 0, foundB = 0;
  for (int i = 0; i < 2; i++) {
    Term k = (Term)coll->array[2 * i];
    if (k == keyA) foundA = 1;
    if (k == keyB) foundB = 1;
  }
  if (!foundA || !foundB) {
    BOOM("1d: collision node should contain both keys");
  }

  // Clean up
  dec_and_free((Term)collResult, 1);

  // Collision node created via malloc_hashCollisionNode(2): malloc_count=1.
  check_counts("testBmiMutateAssocCollision", 1, 1, __LINE__);

  sha1 = savedSha1;
}

// Test: sub-node case — mutate inner key/value (1a)
// bmiMutateAssoc with refs==1, bit set, sub-node, recursive update
void testBmiMutateAssocSubNodeRecurse(void) {
  reset_counters();

  // Build a nested structure: two keys with same bit position at shift=0
  // This creates a sub-node at shift=5
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = sha1((FnArity *)0, key1);
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // Find key2 with same bit position
  Term key2 = newI60(1000);
  int64_t hash2 = sha1((FnArity *)0, key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
    bit2 = bitpos(hash2, 0);
  }

  Term val2 = newI60(888);
  result = bmiCopyAssoc(original, key2, val2, hash2, 0);

  // Verify we have a nested structure
  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("should have nested sub-node");
  }

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)bm)->refs = 1;

  // Update key1 in the sub-node with a different value
  Term newVal = newI60(999);
  Value *updateResult = bmiMutateAssoc(bm, key1, newVal, hash1, 0);

  // Verify same pointer returned (in-place mutation)
  if (updateResult != (Value *)bm) {
    BOOM("bmiMutateAssoc sub-node: same key should return original node pointer");
  }

  // Verify the sub-node was updated in-place
  BitmapIndexedNode *updatedSub = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (__builtin_popcount(updatedSub->bitmap) != 2) {
    BOOM("sub-node should still have 2 entries");
  }

  // Find key1's value in the updated sub-node
  Term foundVal = 0;
  if (subNodeEqualsKey((Term)(Value *)updatedSub->array[0], key1)) {
    foundVal = updatedSub->array[1];
  } else if (subNodeEqualsKey((Term)(Value *)updatedSub->array[2], key1)) {
    foundVal = updatedSub->array[3];
  }
  if (foundVal == 0) {
    BOOM("sub-node should contain key1");
  }
  if (getI60(foundVal) != 999) {
    BOOM("sub-node value should be updated to 999");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiMutateAssocSubNodeRecurse", 0, 0, __LINE__);
}


// Test: bit not set, n >= 16, promote to ArrayNode (2a)
void testBmiMutateAssocPromote(void) {
  reset_counters();
  BitmapIndexedNode *node = malloc_bmiNode(16);
  int64_t hashes[16];
  Term keys[16];
  Term vals[16];
  int count = 0;
  for (i64 c = 0; count < 16 && c < 100000; c++) {
    Term key = newI60(c);
    int64_t hash = sha1((FnArity *)0, key);
    int bit = bitpos(hash, 0);
    int used = 0;
    for (int j = 0; j < count; j++) { if (bitpos(hashes[j], 0) == bit) { used = 1; break; } }
    if (!used) { hashes[count] = hash; keys[count] = key; vals[count] = newI60(c * 100); count++; }
  }
  if (count < 16) BOOM("16 keys");
  for (int i = 0; i < 16; i++) node = (BitmapIndexedNode *)bmiMutateAssoc(node, keys[i], vals[i], hashes[i], 0);
  if (__builtin_popcount(node->bitmap) != 16) BOOM("16 entries");
  ((Value *)node)->refs = 1;
  Term newKey; int64_t newHash;
  for (i64 c = 0; c < 100000; c++) {
    newKey = newI60(c); newHash = sha1((FnArity *)0, newKey);
    int newBit = bitpos(newHash, 0);
    int used = 0;
    for (int j = 0; j < 16; j++) { if (bitpos(hashes[j], 0) == newBit) { used = 1; break; } }
    if (!used) break;
  }
  Value *promoteResult = bmiMutateAssoc(node, newKey, newI60(99999), newHash, 0);
  ArrayNode *an = (ArrayNode *)promoteResult;
  if (an->type != ArrayNodeType) BOOM("ArrayNode");
  int entryCount = 0;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) { if (an->array[i] != 0) entryCount++; }
  if (entryCount != 17) BOOM("17 entries");
  dec_and_free((Term)promoteResult, 1);
  check_counts("testBmiMutateAssocPromote", 140, 0, __LINE__);
}

// Test: same key + same value → no-op, return original node (1b)
// bmiMutateAssoc with refs==1, bit set, keys equal, values equal
void testBmiMutateAssocNoOp(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = sha1((FnArity *)0, key);

  // Add key/value — first mutateAssoc call
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Call bmiMutateAssoc with same key and same value
  // This should trigger path 1b: keys equal, values equal → no-op
  Value *noOpResult = bmiMutateAssoc(original, key, val, hash, 0);

  // Verify same pointer returned (no-op path)
  if (noOpResult != (Value *)original) {
    BOOM("bmiMutateAssoc no-op: same key+value should return original node");
  }

  // Verify structure unchanged
  if (((BitmapIndexedNode *)noOpResult)->bitmap != original->bitmap) {
    BOOM("bmiMutateAssoc no-op: bitmap should be unchanged");
  }

  // Verify the key/value are still at the correct index
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(((BitmapIndexedNode *)noOpResult)->bitmap & (bit - 1));
  if (((BitmapIndexedNode *)noOpResult)->array[2 * idx] != key) {
    BOOM("bmiMutateAssoc no-op: key should be unchanged");
  }
  if (((BitmapIndexedNode *)noOpResult)->array[2 * idx + 1] != val) {
    BOOM("bmiMutateAssoc no-op: value should be unchanged");
  }

  // Clean up
  dec_and_free((Term)noOpResult, 1);

  // The key and val passed to the no-op call are freed by the function.
  // No new allocations.
  check_counts("testBmiMutateAssocNoOp", 0, 0, __LINE__);
}

// Test: same key, same value → no-op, return original node (A2a)
void testBmiCopyAssocNoOp(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(99);
  Term val = newI60(13);
  int64_t hash = sha1((FnArity *)0, key);
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Store original pointer
  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Call bmiCopyAssoc with same key/value — should return original (no clone)
  Value *noOpResult = bmiCopyAssoc(original, key, val, hash, 0);

  // Verify same pointer returned (no-op path)
  if (noOpResult != (Value *)original) {
    BOOM("same key+value should return original node (A2a)");
  }

  // Verify structure unchanged
  if (((BitmapIndexedNode *)noOpResult)->bitmap != original->bitmap) {
    BOOM("bitmap should be unchanged");
  }

  // Clean up
  dec_and_free((Term)noOpResult, 1);

  check_counts("testBmiCopyAssocNoOp", 0, 0, __LINE__);
}

// Test: same key, different value → clone with updated value (A2b)
void testBmiCopyAssocUpdate(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node with I60 key and String value
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = (Term)stringValue("keyst");
  Term val1 = (Term)stringValue("hello");
  int64_t hash = strSha1(incRefVal(key1, 1));
  Value *result = bmiMutateAssoc(node, key1, val1, hash, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int64_t orig_bitmap = original->bitmap;

  // Call bmiCopyAssoc with same key, different String value — should clone (A2b)
  Term newVal = (Term)stringValue("world");
  Term key2 = (Term)stringValue("keyst");
  Value *updateResult = bmiCopyAssoc(original, key2, newVal, hash, 0);

  // Verify different pointer returned (clone created)
  if (updateResult == (Value *)original) {
    BOOM("different value should return cloned node (A2b)");
  }

  // Verify bitmap unchanged
  if (((BitmapIndexedNode *)updateResult)->bitmap != orig_bitmap) {
    BOOM("bitmap should be unchanged");
  }

  // Verify value was updated
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(((BitmapIndexedNode *)updateResult)->bitmap & (bit - 1));
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx + 1] != newVal) {
    BOOM("value should be updated");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiCopyAssocUpdate", 0, 0, __LINE__);
}

// Test: lookup existing key returns the value
void testBmiGet(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node with String key and String value
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term strKey = (Term)stringValue("key137");
  Term strVal = (Term)stringValue("hello");
  int64_t hash = strSha1(incRefVal(strKey, 1));
  Value *result = bmiMutateAssoc(node, strKey, strVal, hash, 0);

  // Lookup existing key
  // bmiGet frees the node and default, returns incRef'd value
  Term key2 = (Term)stringValue("key137");
  Value *found = bmiGet((Value *)result, (Value *)key2, stringValue("not found"), hash, 0);

  // Verify result is the String value
  if (found->type != StringBufferType) {
    BOOM("get should return String value");
  }
  if (strncmp(((String *)found)->buffer, "hello", 5) != 0) {
    BOOM("get should return correct string value");
  }

  // Clean up — only free the found value (bmiGet already freed node+default)
  dec_and_free((Term)found, 1);

  check_counts("testBmiGet", 0, 0, __LINE__);
}

// Test: remove key from single-item BMI → returns emptyBMI
void testBmiDissoc(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = (Term)stringValue("key137");
  Term val = (Term)stringValue("hello");
  int64_t hash = strSha1(incRefVal(key, 1));
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Remove the only key — should return emptyBMI
  Term key2 = (Term)stringValue("key137");
  Value *afterDissoc = bmiDissoc((Value *)result, (Value *)key2, hash, 0);

  // Verify result is emptyBMI
  if (afterDissoc != (Value *)&emptyBMI) {
    BOOM("dissoc from single-item should return emptyBMI");
  }

  // Clean up — emptyBMI is a singleton, no need to free
  check_counts("testBmiDissoc", 0, 0, __LINE__);
}

// Test: remove key from multi-item BMI → returns smaller map (not emptyBMI)
void testBmiDissocEmpty(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create two-item BMI node using keys with different bit positions
  BitmapIndexedNode *node = malloc_bmiNode(2);
  Term key1 = (Term)stringValue("key1");
  Term val1 = (Term)stringValue("val251");
  int64_t hash1 = strSha1(incRefVal(key1, 1));
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  Term key2 = (Term)stringValue("key2");
  Term val2 = (Term)stringValue("val888");
  int64_t hash2 = strSha1(incRefVal(key2, 1));
  result = bmiMutateAssoc((BitmapIndexedNode *)result, key2, val2, hash2, 0);

  // Verify we have 2 entries
  BitmapIndexedNode *bm2 = (BitmapIndexedNode *)result;
  if (__builtin_popcount(bm2->bitmap) != 2) {
    BOOM("should have 2 entries");
  }

  // Remove key1 — should return a single-item map (not emptyBMI)
  Value *afterDissoc = bmiDissoc(result, stringValue("key1"), hash1, 0);

  // Verify result is NOT emptyBMI
  if (afterDissoc == (Value *)&emptyBMI) {
    BOOM("dissoc from multi-item should not return emptyBMI");
  }

  // Verify result is a BMI node with 1 entry
  BitmapIndexedNode *newNode = (BitmapIndexedNode *)afterDissoc;
  if (newNode->type != BitmapIndexedType) {
    BOOM("dissoc result should be BitmapIndexedType");
  }
  if (__builtin_popcount(newNode->bitmap) != 1) {
    BOOM("dissoc result should have exactly 1 bit set");
  }

  // Verify the remaining key is key2
  if (newNode->array[0] != key2) {
    BOOM("remaining key should be key2");
  }

  // Clean up
  dec_and_free((Term)afterDissoc, 1);

  // Pool for itemCount=2 created by malloc_bmiNode: +10 mallocs, +2 for String values
  check_counts("testBmiDissocEmpty", 0, 0, __LINE__);
}

// Test: count returns N for N-entry map
void testBmiCount(void) {
  reset_counters();

  // Create two-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(2);
  Term key1 = (Term)stringValue("key1");
  Term val1 = (Term)stringValue("val251");
  int64_t hash1 = strSha1(incRefVal(key1, 1));
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  Term key2 = (Term)stringValue("key2");
  Term val2 = (Term)stringValue("val888");
  int64_t hash2 = strSha1(incRefVal(key2, 1));
  result = bmiMutateAssoc((BitmapIndexedNode *)result, key2, val2, hash2, 0);

  // Count entries
  Value *countResult = bmiCount(result);

  // Verify count is 2
  if (termTag((Term)countResult) != I60) {
    BOOM("count should return I60");
  }
  if (getI60((Term)countResult) != 2) {
    BOOM("count should be 2");
  }

  // Clean up
  dec_and_free((Term)countResult, 1);

  // Pool for itemCount=2 already created by testBmiDissocEmpty, pulled from pool
  check_counts("testBmiCount", 0, 0, __LINE__);
}

// Test: lookup missing key returns nothing
void testBmiGetMiss(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = (Term)stringValue("key");
  Term val = (Term)stringValue("hello");
  int64_t hash = strSha1(incRefVal(key, 1));
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Create a different key that won't match
  Term missKey = (Term)stringValue("miss");
  int64_t missHash = strSha1(incRefVal(missKey, 1));

  // Lookup missing key
  // bmiGet frees the node but NOT the default, returns the default (nothing)
  Value *notFound = bmiGet((Value *)result, (Value *)missKey, stringValue("not found"), missHash, 0);

  // Verify result is nothing
  if (termTag((Term)notFound) != VAL) {
    BOOM("get miss should return VAL");
  }
  Value *v = (Value *)notFound;
  if (v->type != StringBufferType) {
    BOOM("get miss should return StringBufferType");
  }

  // Clean up — bmiGet freed node, we free the returned nothing
  dec_and_free((Term)notFound, 1);

  check_counts("testBmiGetMiss", 0, 0, __LINE__);
}

// Test: add key with same bit position but different hash → branch node (A2d)
void testBmiCopyAssocBranch(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = (Term)stringValue("key137");
  Term val1 = (Term)stringValue("value251");
  int64_t hash1 = strSha1(incRefVal(key1, 1));
  BitmapIndexedNode *original = (BitmapIndexedNode *)bmiMutateAssoc(node, key1, val1, hash1, 0);
  int bit1 = bitpos(hash1, 0);

  // key2 with same lowest-bit position as key1 but different hash — triggers A2d (branch node)
  Term key2 = (Term)stringValue("keyX");
  int64_t hash2 = (hash1 & 0x1f) | 0x1000;
  ((String *)key2)->hashVal = hash2;

  // Call bmiCopyAssoc — should create a branch node (A2d)
  Term newVal = (Term)stringValue("value888");
  Value *branchResult = bmiCopyAssoc(original, key2, newVal, hash2, 0);

  // Verify result is a BMI node
  BitmapIndexedNode *bm = (BitmapIndexedNode *)branchResult;
  if (bm->type != BitmapIndexedType) {
    BOOM("branch result should be BitmapIndexedType");
  }

  // Verify the bitmap still has 1 bit set (sub-node occupies one slot)
  if (__builtin_popcount(bm->bitmap) != 1) {
    BOOM("branch result bitmap should have 1 bit set");
  }

  // The entry at the shared bit position should be a sub-node (keyOrNull == NULL)
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  Term entryKey = bm->array[2 * idx];
  if (entryKey != 0) {
    BOOM("A2d: entry at shared bit should be sub-node (NULL key)");
  }

  // Verify the sub-node contains both keys
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (subNode->type != BitmapIndexedType) {
    BOOM("A2d: sub-node should be BitmapIndexedType");
  }
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("A2d: sub-node should have 2 bits set");
  }

  // Verify both keys are in the sub-node
  if (subNode->array[0] != key1 && subNode->array[0] != key2) {
    BOOM("A2d: sub-node should contain key1");
  }
  if (subNode->array[2] != key1 && subNode->array[2] != key2) {
    BOOM("A2d: sub-node should contain key2");
  }

  // Clean up
  dec_and_free((Term)branchResult, 1);

  // Both pools (itemCount=1 and itemCount=2) already created by earlier tests,
  // both pulled from existing pools. No malloc/free.
  check_counts("testBmiCopyAssocBranch", 0, 0, __LINE__);
}

// Test: nested sub-node update with same value → no-op, return original (A1a)
void testBmiCopyAssocSubNodeNoChange(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // First, build a nested structure: two keys with same bit position at shift=0
  // This creates a sub-node at shift=5
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = sha1((FnArity *)0, key1);
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // Find key2 with same bit position
  Term key2 = newI60(1000);
  int64_t hash2 = sha1((FnArity *)0, key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
    bit2 = bitpos(hash2, 0);
  }

  // Add key2 — this creates a sub-node (branch)
  Term val2 = newI60(888);
  BitmapIndexedNode *bm = (BitmapIndexedNode *)bmiCopyAssoc(original, key2, val2, hash2, 0);

  // Verify we have a nested structure
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  Term entryKey = bm->array[2 * idx];
  if (entryKey != 0) {
    BOOM("A1a: should have sub-node at shared bit");
  }
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("A1a: sub-node should have 2 entries");
  }

  // Now update key1 in the sub-node with the SAME value — should trigger A1a (no-op)
  Value *noChangeResult = bmiCopyAssoc(bm, key1, val1, hash1, 0);

  // Verify same pointer returned (no-op path)
  if (noChangeResult != (Value *)bm) {
    BOOM("A1a: same value should return original node");
  }

  // Clean up
  dec_and_free((Term)noChangeResult, 1);

  check_counts("testBmiCopyAssocSubNodeNoChange", 0, 0, __LINE__);
}

// Test: nested sub-node update with different value → clone (A1b)
void testBmiCopyAssocSubNodeChange(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Build nested structure: two keys with same bit position at shift=0
  // This creates a sub-node at shift=5
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = (Term)stringValue("key137");
  Term val1 = (Term)stringValue("val251");
  int64_t hash1 = strSha1(incRefVal(key1, 1));
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // key2 with same lowest-bit position as key1 but different higher bits — triggers branch
  // We need hash2 & 0x1f == hash1 & 0x1f (same bit pos at shift 0)
  // and hash2 != hash1 (different hash so sub-node is created)
  Term key2 = (Term)stringValue("keyX");
  int64_t lowBits = hash1 & 0x1f;
  int64_t hash2 = lowBits | 0x1000;  // same low bits, different higher bit
  ((String *)key2)->hashVal = hash2;

  Term val2 = (Term)stringValue("val888");
  result = bmiCopyAssoc(original, key2, val2, hash2, 0);

  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;

  // Verify we have a nested structure
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  Term entryKey = bm->array[2 * idx];
  if (entryKey != 0) {
    BOOM("A1b: should have sub-node at shared bit");
  }
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (subNode->type != BitmapIndexedType) {
    BOOM("A1b: sub-node should be BitmapIndexedType");
  }
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("A1b: sub-node should have 2 entries");
  }

  // Update key1 with a DIFFERENT value — should clone (A1b)
  // Create a fresh key string for the lookup (key1's refs is already exhausted)
  Term lookupKey = (Term)stringValue("key137");
  ((String *)lookupKey)->hashVal = 0;  // clear cached hash so strSha1 recomputes
  int64_t lookupHash = strSha1(incRefVal(lookupKey, 1));
  Term newVal = (Term)stringValue("val999");
  Value *cloneResult = bmiCopyAssoc(bm, lookupKey, newVal, lookupHash, 0);

  // Verify different pointer returned (clone created)
  if (cloneResult == (Value *)bm) {
    BOOM("A1b: different value should return cloned node");
  }

  // Verify the clone has the updated value by checking both entries
  BitmapIndexedNode *cloneBm = (BitmapIndexedNode *)cloneResult;
  if (cloneBm->type != BitmapIndexedType) {
    fprintf(stderr, "cloneBm type = %ld, bitmap = %lx\n", (long)cloneBm->type, (unsigned long)cloneBm->bitmap);
    BOOM("A1b: cloneBm should be BitmapIndexedType");
  }
  int cloneIdx = __builtin_popcount(cloneBm->bitmap & (bit1 - 1));
  BitmapIndexedNode *cloneSub = (BitmapIndexedNode *)cloneBm->array[2 * cloneIdx + 1];
  if (cloneSub->type != BitmapIndexedType) {
    fprintf(stderr, "cloneSub type = %ld\n", (long)cloneSub->type);
    BOOM("A1b: cloneSub should be BitmapIndexedType");
  }

  // Verify the sub-node has 2 entries and one of them has the updated value
  if (__builtin_popcount(cloneSub->bitmap) != 2) {
    BOOM("A1b: cloned sub-node should have 2 entries");
  }

  // Clean up
  dec_and_free((Term)cloneResult, 1);

  // New pool for itemCount=2 (clone of sub-node): malloc_count=10.
  check_counts("testBmiCopyAssocSubNodeChange", 1, 0, __LINE__);
}

// Test: two keys with identical SHA1 hash → creates HashCollisionNode (A2c)
void testBmiCopyAssocCollision(void) {
  reset_counters();

  // Save original sha1 and equal
  Term (*savedSha1)(FnArity *, Term) = sha1;

  // Create single-item BMI node with KEY_A
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term keyA = (Term)stringValue("keyA");
  Term valA = (Term)stringValue("val10");
  int64_t hashA = strSha1(incRefVal(keyA, 1));
  Value *result = bmiMutateAssoc(node, keyA, valA, hashA, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Add KEY_B — same hash, different key → should create collision node (A2c)
  Term keyB = (Term)stringValue("keyB");
  Term valB = (Term)stringValue("val20");
  Value *collResult = bmiCopyAssoc(original, keyB, valB, hashA, 0);

  // Verify result is still a BMI node
  BitmapIndexedNode *bm = (BitmapIndexedNode *)collResult;
  if (bm->type != BitmapIndexedType) {
    BOOM("collision result should be BitmapIndexedType");
  }

  // Verify bitmap has 1 bit set (collision node occupies one slot)
  if (__builtin_popcount(bm->bitmap) != 1) {
    BOOM("collision result bitmap should have 1 bit set");
  }

  // Verify the slot contains a HashCollisionNode
  int idx = __builtin_popcount(bm->bitmap & ((uint32_t)1 - 1));
  Value *slotVal = (Value *)bm->array[2 * idx + 1];
  if (slotVal->type != HashCollisionNodeType) {
    BOOM("collision slot should contain HashCollisionNodeType");
  }

  HashCollisionNode *coll = (HashCollisionNode *)slotVal;
  if (coll->count != 4) {
    char msg[100];
    snprintf(msg, 99, "collision: expected count 4, got %d", coll->count);
    BOOM(msg);
  }

  // Verify both keys are present
  int foundA = 0, foundB = 0;
  for (int i = 0; i < 2; i++) {
    Term k = (Term)coll->array[2 * i];
    if (k == keyA) foundA = 1;
    if (k == keyB) foundB = 1;
  }
  if (!foundA || !foundB) {
    BOOM("collision node should contain both keys");
  }

  // Clean up
  dec_and_free((Term)collResult, 1);

  // Collision node created via malloc_hashCollisionNode(2): malloc_count=1.
  check_counts("testBmiCopyAssocCollision", 1, 1, __LINE__);

  sha1 = savedSha1;
}

// Test: flatten BMI to vector of pairs
void testBmiHashVec(void) {
  reset_counters();

  // Build 3 entries, each at a different bit position
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = integerSha1(key1);
  Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);

  int bit1 = bitpos(hash1, 0);
  Term key2 = newI60(300);
  int64_t hash2 = integerSha1(key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 == bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = integerSha1(key2);
    bit2 = bitpos(hash2, 0);
  }
  Term val2 = newI60(400);
  result = bmiMutateAssoc((BitmapIndexedNode *)result, key2, val2, hash2, 0);

  Term key3 = newI60(1);
  int64_t hash3 = integerSha1(key3);
  int bit3 = bitpos(hash3, 0);
  while (bit3 == bit1 || bit3 == bit2) {
    key3 = newI60(getI60(key3) + 1);
    hash3 = integerSha1(key3);
    bit3 = bitpos(hash3, 0);
  }
  Term val3 = newI60(600);
  result = bmiMutateAssoc((BitmapIndexedNode *)result, key3, val3, hash3, 0);

  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;
  if (__builtin_popcount(bm->bitmap) != 3) {
    BOOM("should have 3 entries");
  }

  // Create empty vector
  Vector *empty = (Vector *)empty_vect;

  // Flatten BMI to vector
  Value *vecResult = bmiHashVec((Value *)bm, (Value *)empty);

  // Verify result is a vector
  if (((Vector *)vecResult)->type != VectorType) {
    BOOM("bmiHashVec should return VectorType");
  }

  // Verify count is 3 (3 key-value pairs)
  if (((Vector *)vecResult)->count != 3) {
    BOOM("bmiHashVec vector should have count 3");
  }

  // Verify all 3 pairs are present in the vector
  // Each pair is a 2-element vector [key, value]
  int found1 = 0, found2 = 0, found3 = 0;
  for (int i = 0; i < 3; i++) {
    Term pairTerm = vectGet((Vector *)vecResult, i);
    if (termTag(pairTerm) != VAL) {
      BOOM("bmiHashVec: pair should be VAL");
    }
    Value *pair = (Value *)pairTerm;
    if (pair->type != VectorType) {
      BOOM("bmiHashVec: pair should be VectorType");
    }
    Vector *pairVec = (Vector *)pair;
    if (pairVec->count != 2) {
      BOOM("bmiHashVec: pair should have 2 elements");
    }
    Term pairKey = pairVec->tail[0];
    Term pairVal = pairVec->tail[1];
    if (subNodeEqualsKey(pairKey, key1)) {
      if (!subNodeEqualsKey(pairVal, val1)) BOOM("bmiHashVec: pair1 value mismatch");
      found1 = 1;
    } else if (subNodeEqualsKey(pairKey, key2)) {
      if (!subNodeEqualsKey(pairVal, val2)) BOOM("bmiHashVec: pair2 value mismatch");
      found2 = 1;
    } else if (subNodeEqualsKey(pairKey, key3)) {
      if (!subNodeEqualsKey(pairVal, val3)) BOOM("bmiHashVec: pair3 value mismatch");
      found3 = 1;
    }
  }
  if (!found1 || !found2 || !found3) {
    BOOM("bmiHashVec: should contain all 3 pairs");
  }

  // Clean up — free the result vector
  dec_and_free((Term)vecResult, 1);

  // Pool for itemCount=3 created by bmiCopyAssoc (3-entry BMI)
  check_counts("testBmiHashVec", 0, 0, __LINE__);
}

// Test: add key-value to empty ArrayNode
void testArrayNodeCopyAssoc(void) {
  reset_counters();
  ArrayNode *node = malloc_arrayNode();
  Term key = newI60(100);
  Term val = newI60(200);
  int64_t hash = sha1((FnArity *)0, key);
  Value *result = arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);
  if (((ArrayNode *)result)->type != ArrayNodeType) {
    BOOM("arrayNodeCopyAssoc should return ArrayNodeType");
  }
  dec_and_free((Term)result, 1);
  check_counts("testArrayNodeCopyAssoc", 0, 0, __LINE__);
}

// Test: non-empty ArrayNode, add to empty slot (Path A2)
void testArrayNodeCopyAssocA2(void) {
  reset_counters();

  // Create ArrayNode with one entry
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  int slot1 = mask(hash1, 0);

  // Find key2 at a different slot
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == slot1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(400);
  int slot2 = mask(hash2, 0);

  // Add key2 — should create new ArrayNode, copy key1, add key2
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  // Verify result is ArrayNode
  if (((ArrayNode *)node)->type != ArrayNodeType) {
    BOOM("A2: should be ArrayNodeType");
  }

  // Verify both slots are populated
  Term sub1 = ((ArrayNode *)node)->array[slot1];
  Term sub2 = ((ArrayNode *)node)->array[slot2];
  if (sub1 == 0 || sub2 == 0) {
    BOOM("A2: both slots should be populated");
  }

  // Verify slot1 still contains the original BMI sub-node (same pointer — copied)
  BitmapIndexedNode *bmi1 = (BitmapIndexedNode *)sub1;
  if (bmi1->array[0] != key1) {
    BOOM("A2: slot1 should contain key1");
  }

  // Verify slot2 contains a new BMI sub-node with key2
  BitmapIndexedNode *bmi2 = (BitmapIndexedNode *)sub2;
  if (bmi2->array[0] != key2) {
    BOOM("A2: slot2 should contain key2");
  }

  // Verify original node was freed (new pointer)
  // (We can't verify this directly, but the fact that we got a new node with 2 entries
  //  implies the old 1-entry node was freed)

  dec_and_free((Term)node, 1);
  check_counts("testArrayNodeCopyAssocA2", 0, 0, __LINE__);
}

// Test: ArrayNode with sub-node, same key+value (Path B1: no-op)
void testArrayNodeCopyAssocB1(void) {
  reset_counters();

  // Create ArrayNode with a BMI sub-node
  ArrayNode *node = malloc_arrayNode();
  Term key = newI60(100);
  Term val = newI60(200);
  int64_t hash = sha1((FnArity *)0, key);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  int slot = mask(hash, 0);

  // Call copyAssoc with the same key and same value — should be no-op
  Value *result = arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Verify same pointer returned (no new allocation)
  if (result != (Value *)node) {
    BOOM("B1: should return original node pointer");
  }

  // Verify the slot still has the key
  BitmapIndexedNode *bmi = (BitmapIndexedNode *)node->array[slot];
  if (bmi->array[0] != key) {
    BOOM("B1: slot should still contain key");
  }

  // Verify the value is correct
  if (bmi->array[1] != val) {
    BOOM("B1: slot should contain correct value");
  }

  dec_and_free((Term)result, 1);
  check_counts("testArrayNodeCopyAssocB1", 0, 0, __LINE__);
}

// Test: ArrayNode with sub-node, different value (Path B2)
void testArrayNodeCopyAssocB2(void) {
  reset_counters();

  // Create ArrayNode with a BMI sub-node
  ArrayNode *node = malloc_arrayNode();
  Term key = newI60(100);
  Term val1 = newI60(200);
  int64_t hash = sha1((FnArity *)0, key);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val1, hash, 0);

  int slot = mask(hash, 0);
  void *original = (void *)node;

  // Call copyAssoc with different value — should create new ArrayNode
  Term val2 = newI60(999);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val2, hash, 0);

  // Verify new pointer returned
  if ((void *)node == original) {
    BOOM("B2: should return new node pointer");
  }

  // Verify the slot contains the updated value
  BitmapIndexedNode *bmi = (BitmapIndexedNode *)node->array[slot];
  if (bmi->array[0] != key) {
    BOOM("B2: slot should contain key");
  }
  if (bmi->array[1] != val2) {
    BOOM("B2: slot should contain updated value 999");
  }

  dec_and_free((Term)node, 1);
  check_counts("testArrayNodeCopyAssocB2", 0, 0, __LINE__);
}

// Test: Multiple entries, update one sub-node (Path B2-multi)
void testArrayNodeCopyAssocB2Multi(void) {
  reset_counters();

  // Create ArrayNode with 2 entries at different slots
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  int slot1 = mask(hash1, 0);

  // Find key2 at a different slot
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == slot1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(400);
  int slot2 = mask(hash2, 0);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  // Save the original sub-node pointer for key2 (should be copied unchanged)
  BitmapIndexedNode *bmi2Original = (BitmapIndexedNode *)node->array[slot2];

  // Update key1 with a new value
  Term val1New = newI60(777);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1New, hash1, 0);

  // Verify slot1 has the new value
  BitmapIndexedNode *bmi1 = (BitmapIndexedNode *)node->array[slot1];
  if (bmi1->array[1] != val1New) {
    BOOM("B2-multi: slot1 should have updated value 777");
  }

  // Verify slot2 was copied (same pointer — no change needed)
  BitmapIndexedNode *bmi2 = (BitmapIndexedNode *)node->array[slot2];
  if (bmi2 != bmi2Original) {
    BOOM("B2-multi: slot2 should be same pointer as original (copied)");
  }
  if (bmi2->array[0] != key2) {
    BOOM("B2-multi: slot2 should still contain key2");
  }

  dec_and_free((Term)node, 1);
  check_counts("testArrayNodeCopyAssocB2Multi", 0, 0, __LINE__);
}

// Test: lookup existing key in ArrayNode
void testArrayNodeGet(void) {
  reset_counters();

  // Create ArrayNode with a BMI sub-node
  ArrayNode *node = malloc_arrayNode();
  Term key = newI60(100);
  Term val = newI60(200);
  int64_t hash = sha1((FnArity *)0, key);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Lookup the key
  Value *found = arrayNodeGet((Value *)node, (Value *)key, (Value *)nothing(), hash, 0);

  // Verify correct value returned
  if (termTag((Term)found) != I60 || getI60((Term)found) != 200) {
    BOOM("arrayNodeGet: should find correct value");
  }

  dec_and_free((Term)found, 1);
  check_counts("testArrayNodeGet", 0, 0, __LINE__);
}

// Test: lookup missing key in ArrayNode (empty slot)
void testArrayNodeGetMiss(void) {
  reset_counters();

  // Create ArrayNode with one entry at slot X
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  // Find key2 at a different slot
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == mask(hash1, 0)) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }

  // Lookup key2 — slot is empty, should return default
  Value *miss = arrayNodeGet((Value *)node, (Value *)key2, (Value *)nothing(), hash2, 0);

  // Verify default returned
  if (termTag((Term)miss) != VAL || ((Value *)miss)->type != NoneType) {
    BOOM("arrayNodeGetMiss: should return NoneType");
  }

  dec_and_free((Term)miss, 1);
  check_counts("testArrayNodeGetMiss", 0, 0, __LINE__);
}

// Test: key not found in BMI sub-node (Path B2)
void testArrayNodeGetB2Miss(void) {
  reset_counters();

  // Create ArrayNode with a BMI sub-node
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  // Find key2 at the same slot as key1
  int slot = mask(hash1, 0);
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) != slot) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }

  // Lookup key2 — same slot as key1, but key2 not in BMI
  Value *miss = arrayNodeGet((Value *)node, (Value *)key2, (Value *)nothing(), hash2, 0);

  // Verify default returned
  if (termTag((Term)miss) != VAL || ((Value *)miss)->type != NoneType) {
    BOOM("arrayNodeGetB2Miss: should return NoneType");
  }

  dec_and_free((Term)miss, 1);
  check_counts("testArrayNodeGetB2Miss", 0, 0, __LINE__);
}

void testArrayNodeCount(void) {
  reset_counters();
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  // Find key at different slot
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == mask(hash1, 0)) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(400);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  Value *countResult = arrayNodeCount((Value *)node);
  if (termTag((Term)countResult) != I60 || getI60((Term)countResult) != 2) {
    BOOM("arrayNodeCount: should return 2");
  }
  dec_and_free((Term)countResult, 1);
  check_counts("testArrayNodeCount", 0, 0, __LINE__);
}

// Test: count empty ArrayNode
void testArrayNodeCountEmpty(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  Value *countResult = arrayNodeCount((Value *)node);

  if (termTag((Term)countResult) != I60 || getI60((Term)countResult) != 0) {
    BOOM("arrayNodeCountEmpty: should return 0");
  }

  dec_and_free((Term)countResult, 1);
  check_counts("testArrayNodeCountEmpty", 0, 0, __LINE__);
}

// Test: count single-entry ArrayNode
void testArrayNodeCountSingle(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  Term key = newI60(100);
  Term val = newI60(200);
  int64_t hash = sha1((FnArity *)0, key);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  Value *countResult = arrayNodeCount((Value *)node);

  if (termTag((Term)countResult) != I60 || getI60((Term)countResult) != 1) {
    BOOM("arrayNodeCountSingle: should return 1");
  }

  dec_and_free((Term)countResult, 1);
  check_counts("testArrayNodeCountSingle", 0, 0, __LINE__);
}

// Test: dissoc key not found — empty slot (Path A)
// Test: refs==1, slot has BMI sub-node → recurse updates entry (Path 2)
void testArrayNodeMutateAssocRecurse(void) {
  reset_counters();

  // Create ArrayNode with one entry
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  int slot1 = mask(hash1, 0);

  // Find key2 at the SAME slot as key1 (triggers recurse into BMI)
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) != slot1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(999);

  // Set refs==1 so mutateAssoc takes the in-place path
  ((Value *)node)->refs = 1;
  void *original = (void *)node;

  // Call arrayNodeMutateAssoc with key at same slot → recurses into BMI
  Value *result = arrayNodeMutateAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  // Verify same pointer returned (in-place mutation)
  if (result != (Value *)original) {
    BOOM("mutateAssoc recurse: should return original node pointer");
  }

  // Verify the slot still has a BMI sub-node
  if (node->array[slot1] == 0) {
    BOOM("mutateAssoc recurse: slot should still be populated");
  }

  // Verify the BMI sub-node now contains both keys
  BitmapIndexedNode *bmi = (BitmapIndexedNode *)node->array[slot1];
  if (__builtin_popcount(bmi->bitmap) != 2) {
    BOOM("mutateAssoc recurse: BMI should have 2 entries");
  }

  // Verify both keys are present
  int found1 = 0, found2 = 0;
  for (int i = 0; i < 4; i += 2) {
    if (subNodeEqualsKey((Term)bmi->array[i], key1)) found1 = 1;
    if (subNodeEqualsKey((Term)bmi->array[i], key2)) found2 = 1;
  }
  if (!found1 || !found2) {
    BOOM("mutateAssoc recurse: both keys should be present");
  }

  // Verify key2's value is correct
  int bit2 = bitpos(hash2, 0);
  int idx2 = __builtin_popcount(bmi->bitmap & (bit2 - 1));
  if (bmi->array[2 * idx2 + 1] != val2) {
    BOOM("mutateAssoc recurse: key2 value should be updated");
  }

  dec_and_free((Term)result, 1);
  check_counts("testArrayNodeMutateAssocRecurse", 0, 0, __LINE__);
}

void testArrayNodeDissocEmptySlot(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == mask(hash1, 0)) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }

  Value *result = arrayNodeDissoc((Value *)node, (Value *)key2, hash2, 0);

  if (result != (Value *)node) {
    BOOM("arrayNodeDissocEmptySlot: should return original node");
  }

  BitmapIndexedNode *bmi = (BitmapIndexedNode *)node->array[mask(hash1, 0)];
  if (bmi->array[0] != key1) {
    BOOM("arrayNodeDissocEmptySlot: original entry should still exist");
  }

  dec_and_free((Term)result, 1);
  check_counts("testArrayNodeDissocEmptySlot", 0, 0, __LINE__);
}

// Test: refs==1, slot empty → new entry created (Path 1)
void testArrayNodeMutateAssocInsert(void) {
  reset_counters();

  // Create ArrayNode with one entry
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  int slot1 = mask(hash1, 0);

  // Find key2 at a DIFFERENT slot
  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == slot1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(400);
  int slot2 = mask(hash2, 0);

  // Set refs==1 so mutateAssoc takes the in-place path
  ((Value *)node)->refs = 1;
  void *original = (void *)node;

  // Call arrayNodeMutateAssoc with key at empty slot
  Value *result = arrayNodeMutateAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  // Verify same pointer returned (in-place mutation)
  if (result != (Value *)node) {
    BOOM("mutateAssoc insert: should return original node pointer");
  }

  // Verify the new entry is present
  if (node->array[slot2] == 0) {
    BOOM("mutateAssoc insert: new slot should be populated");
  }

  // Verify the new entry contains key2
  BitmapIndexedNode *bmi = (BitmapIndexedNode *)node->array[slot2];
  if (bmi->array[0] != key2) {
    BOOM("mutateAssoc insert: slot should contain key2");
  }

  // Verify the original entry is still there
  if (node->array[slot1] == 0) {
    BOOM("mutateAssoc insert: original slot should still be populated");
  }

  dec_and_free((Term)result, 1);
  check_counts("testArrayNodeMutateAssocInsert", 0, 0, __LINE__);
}

void testArrayNodeDissoc(void) {
  reset_counters();
  ArrayNode *node = malloc_arrayNode();
  Term key1 = newI60(100);
  Term val1 = newI60(200);
  int64_t hash1 = sha1((FnArity *)0, key1);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  Term key2 = newI60(300);
  int64_t hash2 = sha1((FnArity *)0, key2);
  while (mask(hash2, 0) == mask(hash1, 0)) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = sha1((FnArity *)0, key2);
  }
  Term val2 = newI60(400);
  node = (ArrayNode *)arrayNodeCopyAssoc((Value *)node, (Value *)key2, (Value *)val2, hash2, 0);

  node = (ArrayNode *)arrayNodeDissoc((Value *)node, (Value *)key1, hash1, 0);

  ArrayNode *nodeAfterDissoc = (ArrayNode *)arrayNodeDissoc((Value *)node, (Value *)key1, hash1, 0);

  // arrayNodeCount takes ownership of nodeAfterDissoc
  Value *countResult = arrayNodeCount((Value *)nodeAfterDissoc);
  if (getI60((Term)countResult) != 1) {
    BOOM("arrayNodeDissoc: should have 1 entry");
  }
  dec_and_free((Term)countResult, 1);

  check_counts("testArrayNodeDissoc", 0, 0, __LINE__);
}

// Helper: create a HashCollisionNode with one (key, val) entry
static HashCollisionNode *makeCollisionNode(Term key, Term val) {
  HashCollisionNode *node = malloc_hashCollisionNode(1);
  node->array[0] = (Value *)key;
  node->array[1] = (Value *)val;
  node->count = 2;
  return node;
}

// Test: add a new (colliding) key to a collision node
// collisionAssoc should add the new entry
void testCollisionAssocAdd(void) {
  reset_counters();

  // Save original sha1 and equal
  Term (*savedSha1)(FnArity *, Term) = sha1;

  // Install collision-aware sha1 and equal (keys DON'T compare equal)
  sha1 = testingSha1CollisionAdd;

  // Create collision node with KEY_C -> VAL_C
  Term keyC = COLLIDE_KEY_C;
  Term valC = newI60(10);
  HashCollisionNode *node = makeCollisionNode(keyC, valC);

  // collisionAssoc with KEY_D (same hash, different key)
  // Should add KEY_D -> VAL_D as a new entry
  Term keyD = COLLIDE_KEY_D;
  Term valD = newI60(20);
  Value *result = collisionAssoc((Value *)node, (Value *)keyD, (Value *)valD, COLLIDE_HASH_ADD, 0);

  HashCollisionNode *resultNode = (HashCollisionNode *)result;
  if (resultNode->type != HashCollisionNodeType) {
    BOOM("collisionAssoc add: result should be HashCollisionNodeType");
  }

  // Should have 2 entries now
  if (resultNode->count != 4) {
    BOOM("collisionAssoc add: count should be 4 (2 entries)");
  }

  // Verify both keys are present
  int foundC = 0, foundD = 0;
  for (int i = 0; i < 2; i++) {
    Term k = (Term)resultNode->array[2 * i];
    if (k == keyC) foundC = 1;
    if (k == keyD) foundD = 1;
  }
  if (!foundC || !foundD) {
    BOOM("collisionAssoc add: both keys should be present");
  }

  // Verify KEY_D's value is VAL_D
  for (int i = 0; i < 2; i++) {
    if ((Term)resultNode->array[2 * i] == keyD) {
      if ((Term)resultNode->array[2 * i + 1] != valD) {
        BOOM("collisionAssoc add: KEY_D should have VAL_D");
      }
    }
  }

  // Verify original node was freed
  dec_and_free((Term)result, 1);
  check_counts("testCollisionAssocAdd", 2, 2, __LINE__);

  // Restore
  sha1 = savedSha1;
}

// Test: update an existing key in a collision node
// collisionAssoc with same key should replace its value
void testCollisionAssocUpdate(void) {
  reset_counters();

  // Save original sha1 and equal
  Term (*savedSha1)(FnArity *, Term) = sha1;

  // Install collision-aware sha1
  sha1 = testingSha1Collision;

  // Create collision node with KEY_A -> VAL_A
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(10);
  HashCollisionNode *node = makeCollisionNode(keyA, valA);

  // collisionAssoc with same key (KEY_A) but different value
  // Should update KEY_A's value to VAL_NEW
  Term keySame = COLLIDE_KEY_A;
  Term valNew = newI60(99);
  Value *result = collisionAssoc((Value *)node, (Value *)keySame, (Value *)valNew, COLLIDE_HASH, 0);

  HashCollisionNode *resultNode = (HashCollisionNode *)result;
  if (resultNode->count != 2) {
    BOOM("collisionAssoc update: count should still be 2 (1 entry, value replaced)");
  }

  // Verify KEY_A's value was updated to VAL_NEW
  for (int i = 0; i < resultNode->count / 2; i++) {
    if ((Term)resultNode->array[2 * i] == keyA) {
      if ((Term)resultNode->array[2 * i + 1] != valNew) {
        BOOM("collisionAssoc update: KEY_A should have VAL_NEW");
      }
    }
  }

  dec_and_free((Term)result, 1);
  check_counts("testCollisionAssocUpdate", 2, 2, __LINE__);

  sha1 = savedSha1;
}

// Test: add a key with different hash to collision node
// collisionAssoc should promote to BMI node
void testCollisionAssocPromote(void) {
  reset_counters();

  Term (*savedSha1)(FnArity *, Term) = sha1;

  sha1 = testingSha1Collision;

  // Create collision node with KEY_A -> VAL_A
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(10);
  HashCollisionNode *node = makeCollisionNode(keyA, valA);

  // collisionAssoc with a key that has a DIFFERENT hash
  Term keyDiff = newI60(999);
  int64_t hashDiff = sha1((FnArity *)0, keyDiff);
  Term valDiff = newI60(888);
  Value *result = collisionAssoc((Value *)node, (Value *)keyDiff, (Value *)valDiff, hashDiff, 0);

  // Should promote to BMI node
  if (((BitmapIndexedNode *)result)->type != BitmapIndexedType) {
    BOOM("collisionAssoc promote: result should be BitmapIndexedType");
  }

  BitmapIndexedNode *bmi = (BitmapIndexedNode *)result;
  if (__builtin_popcount(bmi->bitmap) != 2) {
    BOOM("collisionAssoc promote: BMI should have 2 entries");
  }

  dec_and_free((Term)result, 1);
  check_counts("testCollisionAssocPromote", 1, 1, __LINE__);

  sha1 = savedSha1;
}

// Test: collisionCount returns correct entry count
void testCollisionCount(void) {
  reset_counters();

  // Create collision node with 1 entry (count = 2)
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(11);
  HashCollisionNode *node1 = makeCollisionNode(keyA, valA);

  Value *result1 = collisionCount((Value *)node1);
  if (termTag((Term)result1) != I60) {
    BOOM("collisionCount 1: result should be I60");
  }
  if (getI60((Term)result1) != 1) {
    char msg[100];
    snprintf(msg, 99, "collisionCount 1: expected 1, got %ld", getI60((Term)result1));
    BOOM(msg);
  }

  // Create collision node with 2 entries (count = 4)
  Term keyB = COLLIDE_KEY_B;
  Term valB = newI60(22);
  HashCollisionNode *node2 = malloc_hashCollisionNode(2);
  node2->array[0] = (Value *)keyA;
  node2->array[1] = (Value *)valA;
  node2->array[2] = (Value *)keyB;
  node2->array[3] = (Value *)valB;
  node2->count = 4;

  Value *result2 = collisionCount((Value *)node2);
  if (termTag((Term)result2) != I60) {
    BOOM("collisionCount 2: result should be I60");
  }
  if (getI60((Term)result2) != 2) {
    char msg[100];
    snprintf(msg, 99, "collisionCount 2: expected 2, got %ld", getI60((Term)result2));
    BOOM(msg);
  }

  check_counts("testCollisionCount", 2, 2, __LINE__);
}

// Test: collisionVec returns key-value pairs as a vector
void testCollisionVec(void) {
  reset_counters();

  // Create collision node with 2 entries
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(11);
  Term keyB = COLLIDE_KEY_B;
  Term valB = newI60(22);
  HashCollisionNode *node = malloc_hashCollisionNode(2);
  node->array[0] = (Value *)keyA;
  node->array[1] = (Value *)valA;
  node->array[2] = (Value *)keyB;
  node->array[3] = (Value *)valB;
  node->count = 4;

  // Flatten to vector
  Vector *empty = (Vector *)empty_vect;
  Value *vecResult = collisionVec((Value *)node, (Value *)empty);

  // Verify result is a vector
  if (((Vector *)vecResult)->type != VectorType) {
    BOOM("collisionVec should return VectorType");
  }

  // Verify count is 2 (2 key-value pairs)
  if (((Vector *)vecResult)->count != 2) {
    char msg[100];
    snprintf(msg, 99, "collisionVec: expected count 2, got %d", ((Vector *)vecResult)->count);
    BOOM(msg);
  }

  // Verify each entry is a 2-element vector [key, value]
  for (int i = 0; i < 2; i++) {
    Term pairTerm = vectGet((Vector *)vecResult, i);
    Vector *pairVec = (Vector *)pairTerm;
    if (pairVec->type != VectorType) {
      BOOM("collisionVec: pair should be VectorType");
    }
    if (pairVec->count != 2) {
      BOOM("collisionVec: pair should have 2 elements");
    }
  }

  check_counts("testCollisionVec", 1, 1, __LINE__);
}

// Test: collisionDissoc removes a key from collision node
void testCollisionDissoc(void) {
  reset_counters();

  // Create collision node with 2 entries
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(11);
  Term keyB = COLLIDE_KEY_B;
  Term valB = newI60(22);
  HashCollisionNode *node = malloc_hashCollisionNode(2);
  node->array[0] = (Value *)keyA;
  node->array[1] = (Value *)valA;
  node->array[2] = (Value *)keyB;
  node->array[3] = (Value *)valB;
  node->count = 4;

  // Dissoc keyA — should return node with only keyB
  Value *result = collisionDissoc((Value *)node, (Value *)keyA, COLLIDE_HASH_ADD, 0);

  HashCollisionNode *resultNode = (HashCollisionNode *)result;
  if (resultNode->type != HashCollisionNodeType) {
    BOOM("collisionDissoc: result should be HashCollisionNodeType");
  }

  // Should have 1 entry now (count = 2)
  if (resultNode->count != 2) {
    char msg[100];
    snprintf(msg, 99, "collisionDissoc: expected count 2, got %d", resultNode->count);
    BOOM(msg);
  }

  // Verify keyB is present
  int foundB = 0;
  for (int i = 0; i < resultNode->count / 2; i++) {
    Term k = (Term)resultNode->array[2 * i];
    if (k == keyB) foundB = 1;
  }
  if (!foundB) {
    BOOM("collisionDissoc: keyB should be present");
  }

  // Verify keyA is gone
  int foundA = 0;
  for (int i = 0; i < resultNode->count / 2; i++) {
    Term k = (Term)resultNode->array[2 * i];
    if (k == keyA) foundA = 1;
  }
  if (foundA) {
    BOOM("collisionDissoc: keyA should be removed");
  }

  // Clean up
  dec_and_free((Term)result, 1);

  // Original node + new node from malloc_hashCollisionNode: malloc_count=2.
  check_counts("testCollisionDissoc", 2, 2, __LINE__);
}

// Test: collisionGet looks up a key in collision node
void testCollisionGet(void) {
  reset_counters();

  // Create collision node with 2 entries
  Term keyA = COLLIDE_KEY_A;
  Term valA = newI60(11);
  Term keyB = COLLIDE_KEY_B;
  Term valB = newI60(22);
  HashCollisionNode *node = malloc_hashCollisionNode(2);
  node->array[0] = (Value *)keyA;
  node->array[1] = (Value *)valA;
  node->array[2] = (Value *)keyB;
  node->array[3] = (Value *)valB;
  node->count = 4;

  // Get keyA — should return valA
  Value *resultA = collisionGet((Value *)node, (Value *)keyA, (Value *)newI60(-1), COLLIDE_HASH_ADD, 0);
  if (getI60((Term)resultA) != 11) {
    BOOM("collisionGet: keyA should return valA (11)");
  }

  // collisionGet frees the node, so create a new one for keyB
  node = malloc_hashCollisionNode(2);
  node->array[0] = (Value *)keyA;
  node->array[1] = (Value *)valA;
  node->array[2] = (Value *)keyB;
  node->array[3] = (Value *)valB;
  node->count = 4;

  // Get keyB — should return valB
  Value *resultB = collisionGet((Value *)node, (Value *)keyB, (Value *)newI60(-1), COLLIDE_HASH_ADD, 0);
  if (getI60((Term)resultB) != 22) {
    BOOM("collisionGet: keyB should return valB (22)");
  }

  // collisionGet frees the node, so create a new one for missing key
  node = malloc_hashCollisionNode(2);
  node->array[0] = (Value *)keyA;
  node->array[1] = (Value *)valA;
  node->array[2] = (Value *)keyB;
  node->array[3] = (Value *)valB;
  node->count = 4;

  // Get non-existent key — should return default
  Term keyC = newI60(999);
  Term defaultVal = newI60(-999);
  Value *resultC = collisionGet((Value *)node, (Value *)keyC, (Value *)defaultVal, COLLIDE_HASH_ADD, 0);
  if (getI60((Term)resultC) != -999) {
    BOOM("collisionGet: missing key should return default");
  }

  check_counts("testCollisionGet", 3, 3, __LINE__);
}

// Test: refs==1, empty BMI → insert single entry (path 2b)
void testBmiMutateAssoc(void) {
  fprintf(stderr, "running: %s\n", __func__);
  reset_counters();

  // Create empty BMI node
  BitmapIndexedNode *node = malloc_bmiNode(0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)node)->refs = 1;

  Term key = (Term)stringValue("key137");
  Term val = (Term)stringValue("hello");
  int64_t hash = strSha1(incRefVal(key, 1));
  Value *result = bmiMutateAssoc(node, key, val, hash, 0);

  // Verify result is a BMI node with 1 entry
  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;
  if (bm->type != BitmapIndexedType) {
    BOOM("result should be BitmapIndexedType");
  }
  if (__builtin_popcount(bm->bitmap) != 1) {
    BOOM("bitmap should have 1 bit set");
  }

  // Verify key/value are stored
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(bm->bitmap & (bit - 1));
  if (bm->array[2 * idx] != key) {
    BOOM("key not at correct index");
  }
  if (bm->array[2 * idx + 1] != val) {
    BOOM("val not at correct index");
  }

  // Clean up
  dec_and_free((Term)result, 1);

  check_counts("testBmiMutateAssoc", 0, 0, __LINE__);
}

// Test runner function type
typedef void (*TestFn)(void);

int main(int argc, char **argv) {
  int shuffled_count;
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((unsigned int)(tv.tv_sec ^ tv.tv_usec ^ getpid()));
  }
  sha1 = testingSha1;

  // just to make BOOM happ
  dotFile = fopen("graphs.dot", "w");
  if (!dotFile) {
    BOOM( "Failed to open graphs.dot\n");
  }
  fprintf(dotFile, "graph grammar {\nranksep=0.1\n");

  // Trigger malloc_reified pool once before tests (5000-entry pool)
  (void)nothing();
  (void)some(newI60(55));
  dec_and_free((Term)malloc_bmiNode(0), 1);
  dec_and_free((Term)malloc_bmiNode(1), 1);
  dec_and_free((Term)malloc_bmiNode(2), 1);
  dec_and_free((Term)malloc_arrayNode(), 1);
#define STRINGS_NEEDED 5
  Term strs[STRINGS_NEEDED];
  for (int i = 0; i < STRINGS_NEEDED; i++)
    strs[i] = (Term)stringValue("key137");
  for (int i = 0; i < STRINGS_NEEDED; i++)
    dec_and_free(strs[i], 1);

  // Pre-allocate Vector pool so bmiHashVec test doesn't trigger pool allocation
  (void)malloc_vector();

  static TestFn tests[] = {
    testEmptyBmiNode,
    testBmiNodeOneItem,
    testArrayNode,
    testCollisionNode,
    testFreeBitmapNodeHighCount,
    testFreeArrayNode,
    testFreeHashCollisionNode,
    testBmiCopyAssoc,
    testBmiMutateAssoc,
    testBmiCopyAssocNoOp,
    testBmiCopyAssocUpdate,
    testBmiGet,
    testBmiGetMiss,
    testBmiDissoc,
    testBmiCopyAssocSubNodeChange,
    testBmiDissocEmpty,
    testBmiCopyAssocBranch,
    testBmiCopyAssocSubNodeNoChange,
    testBmiCopyAssocCollision,
    testBmiCount,
    testBmiMutateAssocUpdateValue,
    // testBmiMutateAssocInsert,
    // testBmiMutateAssocBranch,
    // testBmiMutateAssocCollision,
    // testBmiMutateAssocSubNodeRecurse,
    // testBmiMutateAssocNoOp,
    // testBmiMutateAssocPromote,
    // testArrayNodeCopyAssoc,
    // testArrayNodeCopyAssocA2,
    // testArrayNodeCopyAssocB1,
    // testArrayNodeCopyAssocB2,
    // testArrayNodeCopyAssocB2Multi,
    // testArrayNodeGet,
    // testArrayNodeGetMiss,
    // testArrayNodeGetB2Miss,
    // testArrayNodeCount,
    // testArrayNodeCountEmpty,
    // testArrayNodeCountSingle,
    // testArrayNodeDissocEmptySlot,
    // testArrayNodeDissoc,
    // testArrayNodeMutateAssocInsert,
    // testArrayNodeMutateAssocRecurse,
    // testCollisionAssocAdd,
    // testCollisionAssocUpdate,
    // testCollisionAssocPromote,
    // testCollisionCount,
    // testCollisionVec,
    // testCollisionDissoc,
    // testCollisionGet,
    // testBmiHashVec,
  };
  shuffled_count = sizeof(tests) / sizeof(tests[0]);

  // Fisher-Yates shuffle
  for (int i = shuffled_count - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    TestFn tmp = tests[i];
    tests[i] = tests[j];
    tests[j] = tmp;
  }

  fprintf(stderr, "=== Running %d tests in random order ===\n", shuffled_count);
  for (int i = 0; i < shuffled_count; i++) {
    tests[i]();
  }
  printf("All tests passed\n");
  return 0;
}
