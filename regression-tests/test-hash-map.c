#include <stdio.h>
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

// Reset malloc/free counters. Called before each test to establish baseline.
// After the test, malloc_count == free_count must hold (no leaks).
static void reset_counters(void) {
  malloc_count = 0;
  free_count = 0;
}

// Verify no memory leaks: malloc_count must equal free_count.
// Prints diagnostic on failure and aborts.
static void check_no_leaks(const char *test_name) {
  if (malloc_count != free_count) {
    fprintf(stderr, "FAIL %s: memory leak detected\n", test_name);
    fprintf(stderr, "  malloc_count: %ld  free_count: %ld  diff: %ld\n",
            (long)malloc_count, (long)free_count, (long)(malloc_count - free_count));
    BOOM("memory leak detected");
  }
}

// Test: create empty BMI node, verify bitmap=0
void testEmptyBmiNode(void) {
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(0);

  if (node->type != BitmapIndexedType) {
    BOOM("type should be BitmapIndexedType");
  }
  if (node->bitmap != 0) {
    BOOM("bitmap should be 0");
  }

  // Free — TODO: uncomment freeBitmapNode in runtime3.c
  // dec_and_free(termVal((Term)(Value *)node), 1);

  check_no_leaks("testEmptyBmiNode");
}

// Test: create BMI node with 1 item
void testBmiNodeOneItem(void) {
  reset_counters();

  BitmapIndexedNode *node = malloc_bmiNode(1);
  if (node->bitmap != 0) {
    BOOM("bitmap should be 0 after malloc");
  }

  // TODO: dec_and_free(termVal((Term)(Value *)node), 1);

  check_no_leaks("testBmiNodeOneItem");
}

// Test: create ArrayNode
void testArrayNode(void) {
  reset_counters();

  ArrayNode *node = malloc_arrayNode();
  if (node->type != ArrayNodeType) {
    BOOM("type should be ArrayNodeType");
  }

  // TODO: dec_and_free(termVal((Term)(Value *)node), 1);

  check_no_leaks("testArrayNode");
}

// Test: create HashCollisionNode
void testCollisionNode(void) {
  reset_counters();

  HashCollisionNode *node = malloc_hashCollisionNode(2);
  if (node->type != HashCollisionNodeType) {
    BOOM("type should be HashCollisionNodeType");
  }
  if (node->count != 4) {
    BOOM("count should be 4");
  }

  // TODO: dec_and_free(termVal((Term)(Value *)node), 1);

  check_no_leaks("testCollisionNode");
}

int main(int argc, char **argv) {
  testEmptyBmiNode();
  testBmiNodeOneItem();
  testArrayNode();
  testCollisionNode();
  printf("All tests passed\n");
  return 0;
}
