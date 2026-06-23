#include <stdio.h>
#include <string.h>
#include "new.h"
#include "runtime3.h"

// Stubs required by new.c and runtime3.c (graphing symbols, not used by hash-map tests)
unsigned refsCount = 0;
refMap refNames[0];
unsigned subGraphs = 0;
void subGraph(const char *name, Term t, int depth, int id) {}

// Stub: equalSTAR aborts (only I60 terms are tested, handled separately in equal())
Value *noImpl2(FnArity *arity, Value *v1, Value *v2) { abort(); }
Value *(*equalSTAR)(FnArity *, Value *, Value *) = noImpl2;

// Forward declarations for functions defined in runtime3.c
BitmapIndexedNode *malloc_bmiNode(int itemCount);
ArrayNode *malloc_arrayNode(void);
HashCollisionNode *malloc_hashCollisionNode(int itemCount);
void freeBitmapNode(Value *v);
void freeArrayNode(Value *v);
void freeHashCollisionNode(Value *v);
int bitpos(int64_t hash, int shift);
Term integer_EQ(Term arg0, Term arg1);

// BMI operations
Value *bmiMutateAssoc(Value *node, Value *key, Value *val, int64_t hash, int shift);
Value *bmiCopyAssoc(Value *node, Value *key, Value *val, int64_t hash, int shift);
Value *bmiGet(Value *node, Value *key, Value *def, int64_t hash, int shift);
Value *bmiCount(Value *node);
int64_t nakedSha1(Term trm);

// Forward declarations for helper functions used in tests
Term nothing(void);
Term some(Term thing);

// Helper: check if a term is an I60 with the same value as key
static int subNodeEqualsKey(Term t, Term key) {
  return termTag(t) == I60 && getI60(t) == getI60(key);
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

// Reset malloc/free counters. Called before each test to establish baseline.
static void reset_counters(void) {
  malloc_count = 0;
  free_count = 0;
}

// Verify malloc_count and free_count match expected values.
// Prints diagnostic on failure and aborts.
static void check_counts(const char *test_name, unsigned expected_malloc,
                         unsigned expected_free) {
  if (malloc_count != (int64_t)expected_malloc) {
    fprintf(stderr, "FAIL %s: expected malloc_count=%u, got %lld\n",
            test_name, expected_malloc, (long long)malloc_count);
    BOOM("malloc_count mismatch");
  }
  if (free_count != (int64_t)expected_free) {
    fprintf(stderr, "FAIL %s: expected free_count=%u, got %lld\n",
            test_name, expected_free, (long long)free_count);
    BOOM("free_count mismatch");
  }
}

// Test: create empty BMI node, verify bitmap=0
// Pool created: malloc_count += 9 (spare nodes). Node freed → pool recycle.
void testEmptyBmiNode(void) {
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
  check_counts("testEmptyBmiNode", 10, 0);
}

// Test: create BMI node with 1 item
// Different itemCount (1 vs 0) → different pool index → new pool created.
// Node freed → pool recycle.
void testBmiNodeOneItem(void) {
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(1);
  if (node->bitmap != 0) {
    BOOM("bitmap should be 0 after malloc");
  }

  freeBitmapNode((Value *)node);

  // Different itemCount = different pool index, so new pool created.
  // malloc_count=10, node recycled, free_count=0.
  check_counts("testBmiNodeOneItem", 10, 0);
}

// Test: create ArrayNode
// New pool created (separate from BMI pool): malloc_count += 9.
// Node freed → pool recycle.
void testArrayNode(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  if (node->type != ArrayNodeType) {
    BOOM("type should be ArrayNodeType");
  }

  freeArrayNode((Value *)node);

  // ArrayNode pool allocated 10 nodes: my_malloc(+1) + explicit +9 = 10.
  // Node recycled, not freed.
  check_counts("testArrayNode", 10, 0);
}

// Test: create HashCollisionNode
// No pool. malloc_count += 1, free_count += 1 via dec_and_free.
void testCollisionNode(void) {
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
  check_counts("testCollisionNode", 1, 1);
}

// Test: freeBitmapNode actually frees when cnt >= BMI_RECYCLE_COUNT
// Pool created for itemCount=20: malloc_count += 9.
// Node freed with cnt=20 >= 20: actually freed, free_count += 1.
void testFreeBitmapNodeHighCount(void) {
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
  check_counts("testFreeBitmapNodeHighCount", 1, 1);
}

// Test: freeArrayNode works (even though it recycles into pool)
// Pool already exists (created by testArrayNode), no new malloc_count change.
// Node recycled, free_count += 0.
void testFreeArrayNode(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();

  freeArrayNode((Value *)node);

  check_counts("testFreeArrayNode", 0, 0);
}

// Test: freeHashCollisionNode works correctly
// No pool. malloc_count += 1, free_count += 1.
void testFreeHashCollisionNode(void) {
  reset_counters();

  HashCollisionNode *node = malloc_hashCollisionNode(2);

  freeHashCollisionNode((Value *)node);

  check_counts("testFreeHashCollisionNode", 1, 1);
}

// Test: add key/value to empty BMI → single-item BMI
// Then verify structure is correct
void testBmiCopyAssoc(void) {
  reset_counters();

  // Create empty BMI node
  BitmapIndexedNode *node = malloc_bmiNode(0);

  // Create key = I60(137), value = I60(251)
  Term key = newI60(137);
  Term val = newI60(251);

  // Compute hash of key
  int64_t hash = nakedSha1(key);

  // Add key/value to empty BMI at shift=0
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

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
  if (((BitmapIndexedNode *)result)->array[2 * idx] != (Value *)key) {
    BOOM("key not at correct index");
  }
  if (((BitmapIndexedNode *)result)->array[2 * idx + 1] != (Value *)val) {
    BOOM("val not at correct index");
  }

  // Clean up
  dec_and_free((Term)result, 1);

  // Both pools (itemCount=0 and itemCount=1) were created by earlier tests
  // and recycled, so both malloc calls pull from existing pools. No malloc/free.
  check_counts("testBmiCopyAssoc", 0, 0);
}

// Test: same key, different value → in-place update (1c)
// bmiMutateAssoc with refs==1, bit set, keys equal, values different
void testBmiMutateAssocUpdateValue(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  Term newVal = newI60(999);

  // Call bmiMutateAssoc with same key, different value
  // This should trigger path 1c: keys equal, values different → in-place update
  Value *updateResult = bmiMutateAssoc((Value *)original, (Value *)key, (Value *)newVal, hash, 0);

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
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx] != (Value *)key) {
    BOOM("bmiMutateAssoc update: key should be unchanged");
  }

  // Verify the value was updated
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx + 1] != (Value *)newVal) {
    BOOM("bmiMutateAssoc update: value should be updated");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiMutateAssocUpdateValue", 0, 0);
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
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // Find key2 with same bit position
  Term key2 = newI60(1000);
  int64_t hash2 = nakedSha1(key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = nakedSha1(key2);
    bit2 = bitpos(hash2, 0);
  }

  Term val2 = newI60(888);
  result = bmiCopyAssoc((Value *)original, (Value *)key2, (Value *)val2, hash2, 0);

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
  Value *updateResult = bmiMutateAssoc((Value *)bm, (Value *)key1, (Value *)newVal, hash1, 0);

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
  Value *foundVal = NULL;
  if (subNodeEqualsKey((Term)(Value *)updatedSub->array[0], key1)) {
    foundVal = updatedSub->array[1];
  } else if (subNodeEqualsKey((Term)(Value *)updatedSub->array[2], key1)) {
    foundVal = updatedSub->array[3];
  }
  if (foundVal == NULL) {
    BOOM("sub-node should contain key1");
  }
  if (getI60((Term)foundVal) != 999) {
    BOOM("sub-node value should be updated to 999");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiMutateAssocSubNodeRecurse", 0, 0);
}

// Test: same key + same value → no-op, return original node (1b)
// bmiMutateAssoc with refs==1, bit set, keys equal, values equal
void testBmiMutateAssocNoOp(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = nakedSha1(key);

  // Add key/value — first mutateAssoc call
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Set refs==1 so bmiMutateAssoc takes the in-place path
  ((Value *)result)->refs = 1;

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Call bmiMutateAssoc with same key and same value
  // This should trigger path 1b: keys equal, values equal → no-op
  Value *noOpResult = bmiMutateAssoc((Value *)original, (Value *)key, (Value *)val, hash, 0);

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
  if (((BitmapIndexedNode *)noOpResult)->array[2 * idx] != (Value *)key) {
    BOOM("bmiMutateAssoc no-op: key should be unchanged");
  }
  if (((BitmapIndexedNode *)noOpResult)->array[2 * idx + 1] != (Value *)val) {
    BOOM("bmiMutateAssoc no-op: value should be unchanged");
  }

  // Clean up
  dec_and_free((Term)noOpResult, 1);

  // The key and val passed to the no-op call are freed by the function.
  // No new allocations.
  check_counts("testBmiMutateAssocNoOp", 0, 0);
}

// Test: same key, same value → no-op, return original node (A2a)
void testBmiCopyAssocNoOp(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(42);
  Term val = newI60(99);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Store original pointer
  BitmapIndexedNode *original = (BitmapIndexedNode *)result;

  // Call bmiCopyAssoc with same key/value — should return original (no clone)
  Value *noOpResult = bmiCopyAssoc((Value *)original, (Value *)key, (Value *)val, hash, 0);

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

  check_counts("testBmiCopyAssocNoOp", 0, 0);
}

// Test: same key, different value → clone with updated value (A2b)
void testBmiCopyAssocUpdate(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(42);
  Term val = newI60(99);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  Term newVal = newI60(77);

  // Call bmiCopyAssoc with same key, different value — should clone
  Value *updateResult = bmiCopyAssoc((Value *)original, (Value *)key, (Value *)newVal, hash, 0);

  // Verify different pointer returned (clone created)
  if (updateResult == (Value *)original) {
    BOOM("different value should return cloned node (A2b)");
  }

  // Verify bitmap unchanged
  if (((BitmapIndexedNode *)updateResult)->bitmap != original->bitmap) {
    BOOM("bitmap should be unchanged");
  }

  // Verify value was updated
  int bit = bitpos(hash, 0);
  int idx = __builtin_popcount(((BitmapIndexedNode *)updateResult)->bitmap & (bit - 1));
  if (((BitmapIndexedNode *)updateResult)->array[2 * idx + 1] != (Value *)newVal) {
    BOOM("value should be updated");
  }

  // Clean up
  dec_and_free((Term)updateResult, 1);

  check_counts("testBmiCopyAssocUpdate", 0, 0);
}

// Test: lookup existing key returns the value
void testBmiGet(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Lookup existing key
  // bmiGet frees the node and default, returns incRef'd value
  Value *found = bmiGet((Value *)result, (Value *)key, (Value *)nothing(), hash, 0);

  // Verify result is the I60 value (bmiGet returns raw value, not Maybe-wrapped)
  if (termTag((Term)found) != I60) {
    BOOM("get should return I60 value");
  }
  if (getI60((Term)found) != 251) {
    BOOM("get should return correct value");
  }

  // Clean up — only free the found value (bmiGet already freed node+default)
  dec_and_free((Term)found, 1);

  check_counts("testBmiGet", 0, 0);
}

// Test: remove key from single-item BMI → returns emptyBMI
void testBmiDissoc(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Remove the only key — should return emptyBMI
  Value *afterDissoc = bmiDissoc((Value *)result, (Value *)key, hash, 0);

  // Verify result is emptyBMI
  if (afterDissoc != (Value *)&emptyBMI) {
    BOOM("dissoc from single-item should return emptyBMI");
  }

  // Clean up — emptyBMI is a singleton, no need to free
  check_counts("testBmiDissoc", 0, 0);
}

// Test: remove key from multi-item BMI → returns smaller map (not emptyBMI)
void testBmiDissocEmpty(void) {
  reset_counters();

  // Create two-item BMI node using keys with different bit positions
  BitmapIndexedNode *node = malloc_bmiNode(2);
  Term key1 = newI60(0);   // hash bit 13
  Term val1 = newI60(251);
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  Term key2 = newI60(1);   // hash bit 29
  Term val2 = newI60(888);
  int64_t hash2 = nakedSha1(key2);
  result = bmiMutateAssoc(result, (Value *)key2, (Value *)val2, hash2, 0);

  // Verify we have 2 entries
  BitmapIndexedNode *bm2 = (BitmapIndexedNode *)result;
  if (__builtin_popcount(bm2->bitmap) != 2) {
    BOOM("should have 2 entries");
  }

  // Remove key1 — should return a single-item map (not emptyBMI)
  Value *afterDissoc = bmiDissoc(result, (Value *)key1, hash1, 0);

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
  if (newNode->array[0] != (Value *)key2) {
    BOOM("remaining key should be key2");
  }

  // Clean up
  dec_and_free((Term)afterDissoc, 1);

  // Pool for itemCount=2 created by malloc_bmiNode: +10 mallocs, recycled on free
  check_counts("testBmiDissocEmpty", 10, 0);
}

// Test: count returns N for N-entry map
void testBmiCount(void) {
  reset_counters();

  // Create two-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(2);
  Term key1 = newI60(0);
  Term val1 = newI60(251);
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  Term key2 = newI60(1);
  Term val2 = newI60(888);
  int64_t hash2 = nakedSha1(key2);
  result = bmiMutateAssoc(result, (Value *)key2, (Value *)val2, hash2, 0);

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
  check_counts("testBmiCount", 0, 0);
}

// Test: lookup missing key returns nothing
void testBmiGetMiss(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key = newI60(137);
  Term val = newI60(251);
  int64_t hash = nakedSha1(key);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key, (Value *)val, hash, 0);

  // Create a different key that won't match
  Term missKey = newI60(999);
  int64_t missHash = nakedSha1(missKey);

  // Lookup missing key
  // bmiGet frees the node but NOT the default, returns the default (nothing)
  Value *notFound = bmiGet((Value *)result, (Value *)missKey, (Value *)nothing(), missHash, 0);

  // Verify result is nothing
  if (termTag((Term)notFound) != VAL) {
    BOOM("get miss should return VAL (Maybe type)");
  }
  Value *v = (Value *)notFound;
  if (v->type != NoneType) {
    BOOM("get miss should return NoneType");
  }

  // Clean up — bmiGet freed node, we free the returned nothing
  dec_and_free((Term)notFound, 1);

  check_counts("testBmiGetMiss", 0, 0);
}

// Test: add key with same bit position but different hash → branch node (A2d)
void testBmiCopyAssocBranch(void) {
  reset_counters();

  // Create single-item BMI node
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // Find key2 whose hash has the same bit position at shift=0
  // but a different full hash — this triggers A2d (branch node)
  Term key2 = newI60(1000);
  int64_t hash2 = nakedSha1(key2);
  int bit2 = bitpos(hash2, 0);

  // Keep trying until we find a key with the same bit position
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = nakedSha1(key2);
    bit2 = bitpos(hash2, 0);
  }

  // Call bmiCopyAssoc — should create a branch node (A2d)
  Term newVal = newI60(888);
  Value *branchResult = bmiCopyAssoc((Value *)original, (Value *)key2, (Value *)newVal, hash2, 0);

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
  Value *entryKey = bm->array[2 * idx];
  if (entryKey != (Value *)0) {
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
  if (subNode->array[0] != (Value *)key1 && subNode->array[0] != (Value *)key2) {
    BOOM("A2d: sub-node should contain key1");
  }
  if (subNode->array[2] != (Value *)key1 && subNode->array[2] != (Value *)key2) {
    BOOM("A2d: sub-node should contain key2");
  }

  // Clean up
  dec_and_free((Term)branchResult, 1);

  // Both pools (itemCount=1 and itemCount=2) already created by earlier tests,
  // both pulled from existing pools. No malloc/free.
  check_counts("testBmiCopyAssocBranch", 0, 0);
}

// Test: nested sub-node update with same value → no-op, return original (A1a)
void testBmiCopyAssocSubNodeNoChange(void) {
  reset_counters();

  // First, build a nested structure: two keys with same bit position at shift=0
  // This creates a sub-node at shift=5
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  // Find key2 with same bit position
  Term key2 = newI60(1000);
  int64_t hash2 = nakedSha1(key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = nakedSha1(key2);
    bit2 = bitpos(hash2, 0);
  }

  // Add key2 — this creates a sub-node (branch)
  Term val2 = newI60(888);
  result = bmiCopyAssoc((Value *)original, (Value *)key2, (Value *)val2, hash2, 0);

  // Verify we have a nested structure
  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;
  int idx = __builtin_popcount(bm->bitmap & (bit1 - 1));
  Value *entryKey = bm->array[2 * idx];
  if (entryKey != (Value *)0) {
    BOOM("A1a: should have sub-node at shared bit");
  }
  BitmapIndexedNode *subNode = (BitmapIndexedNode *)bm->array[2 * idx + 1];
  if (__builtin_popcount(subNode->bitmap) != 2) {
    BOOM("A1a: sub-node should have 2 entries");
  }

  // Now update key1 in the sub-node with the SAME value — should trigger A1a (no-op)
  Value *noChangeResult = bmiCopyAssoc((Value *)bm, (Value *)key1, (Value *)val1, hash1, 0);

  // Verify same pointer returned (no-op path)
  if (noChangeResult != (Value *)bm) {
    BOOM("A1a: same value should return original node");
  }

  // Clean up
  dec_and_free((Term)noChangeResult, 1);

  check_counts("testBmiCopyAssocSubNodeNoChange", 0, 0);
}

// Test: nested sub-node update with different value → clone (A1b)
void testBmiCopyAssocSubNodeChange(void) {
  reset_counters();

  // Build nested structure (same as above)
  BitmapIndexedNode *node = malloc_bmiNode(1);
  Term key1 = newI60(137);
  Term val1 = newI60(251);
  int64_t hash1 = nakedSha1(key1);
  Value *result = bmiMutateAssoc((Value *)node, (Value *)key1, (Value *)val1, hash1, 0);

  BitmapIndexedNode *original = (BitmapIndexedNode *)result;
  int bit1 = bitpos(hash1, 0);

  Term key2 = newI60(1000);
  int64_t hash2 = nakedSha1(key2);
  int bit2 = bitpos(hash2, 0);
  while (bit2 != bit1) {
    key2 = newI60(getI60(key2) + 1);
    hash2 = nakedSha1(key2);
    bit2 = bitpos(hash2, 0);
  }

  Term val2 = newI60(888);
  result = bmiCopyAssoc((Value *)original, (Value *)key2, (Value *)val2, hash2, 0);

  BitmapIndexedNode *bm = (BitmapIndexedNode *)result;

  // Update key1 with a DIFFERENT value — should clone (A1b)
  Term newVal = newI60(999);
  Value *cloneResult = bmiCopyAssoc((Value *)bm, (Value *)key1, (Value *)newVal, hash1, 0);

  // Verify different pointer returned (clone created)
  if (cloneResult == (Value *)bm) {
    BOOM("A1b: different value should return cloned node");
  }

  // Verify the clone has the updated value
  BitmapIndexedNode *cloneBm = (BitmapIndexedNode *)cloneResult;
  int idx = __builtin_popcount(cloneBm->bitmap & (bit1 - 1));
  BitmapIndexedNode *cloneSub = (BitmapIndexedNode *)cloneBm->array[2 * idx + 1];

  // Find key1's value in the sub-node
  Value *foundVal = NULL;
  if (subNodeEqualsKey((Term)(Value *)cloneSub->array[0], key1)) {
    foundVal = cloneSub->array[1];
  } else if (subNodeEqualsKey((Term)(Value *)cloneSub->array[2], key1)) {
    foundVal = cloneSub->array[3];
  }
  if (foundVal == NULL) {
    BOOM("A1b: cloned sub-node should contain key1");
  }
  if (getI60((Term)foundVal) != 999) {
    BOOM("A1b: value should be updated to 999");
  }

  // Clean up
  dec_and_free((Term)cloneResult, 1);

  // New pool for itemCount=2 (clone of sub-node): malloc_count=10.
  check_counts("testBmiCopyAssocSubNodeChange", 10, 0);
}

int main(int argc, char **argv) {
extern Value *(*sha1_fn)(FnArity *, Value *);
extern Value *(*count_fn)(FnArity *, Value *);

  mapGet_fn = &mapGet;
  sha1_fn = &sha1Impl;
  count_fn = &countImpl;
  dissoc_fn = &dissoc_impl;

  // Trigger malloc_reified pool once before tests (5000-entry pool)
  (void)nothing();

  testEmptyBmiNode();
  testBmiNodeOneItem();
  testArrayNode();
  testCollisionNode();
  testFreeBitmapNodeHighCount();
  testFreeArrayNode();
  testFreeHashCollisionNode();
  testBmiCopyAssoc();
  testBmiCopyAssocNoOp();
  testBmiCopyAssocUpdate();
  testBmiGet();
  testBmiGetMiss();
  testBmiDissoc();
  testBmiDissocEmpty();
  testBmiCopyAssocBranch();
  testBmiCopyAssocSubNodeNoChange();
  testBmiCopyAssocSubNodeChange();
  testBmiCount();
  testBmiMutateAssocUpdateValue();
  testBmiMutateAssocSubNodeRecurse();
  testBmiMutateAssocNoOp();
  printf("All tests passed\n");
  return 0;
}
