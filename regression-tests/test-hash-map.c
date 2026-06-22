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
Value *bmiGet(Value *node, Value *key, Value *def, int64_t hash, int shift);
int64_t nakedSha1(Term trm);

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

int main(int argc, char **argv) {
extern Value *(*sha1_fn)(FnArity *, Value *);
extern Value *(*count_fn)(FnArity *, Value *);

  mapGet_fn = &mapGet;
  sha1_fn = &sha1Impl;
  count_fn = &countImpl;
  dissoc_fn = &dissoc_impl;
  testEmptyBmiNode();
  testBmiNodeOneItem();
  testArrayNode();
  testCollisionNode();
  testFreeBitmapNodeHighCount();
  testFreeArrayNode();
  testFreeHashCollisionNode();
  testBmiCopyAssoc();
  printf("All tests passed\n");
  return 0;
}
