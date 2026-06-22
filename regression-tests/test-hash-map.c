#include <stdio.h>
#include <string.h>
#include "new.h"
#include "runtime3.h"

// Stubs required by new.c and runtime3.c (graphing symbols, not used by hash-map tests)
unsigned refsCount = 0;
refMap refNames[0];
unsigned subGraphs = 0;
void subGraph(const char *name, Term t, int depth, int id) {}

// Forward declarations for functions defined in runtime3.c
BitmapIndexedNode *malloc_bmiNode(int itemCount);
ArrayNode *malloc_arrayNode(void);
HashCollisionNode *malloc_hashCollisionNode(int itemCount);
void freeBitmapNode(Value *v);
void freeArrayNode(Value *v);
void freeHashCollisionNode(Value *v);

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

int main(int argc, char **argv) {
  mapGet_fn = &mapGet;
  testEmptyBmiNode();
  testBmiNodeOneItem();
  testArrayNode();
  testCollisionNode();
  testFreeBitmapNodeHighCount();
  testFreeArrayNode();
  testFreeHashCollisionNode();
  printf("All tests passed\n");
  return 0;
}
