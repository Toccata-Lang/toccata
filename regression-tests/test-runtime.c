#include <stdio.h>
#include "new.h"
#include "runtime3.h"
#include "graph.h"

unsigned refsCount = 0;
refMap refNames[0];
char *dupLabels[] = {};

// Test malloc_reified / dec_and_free — allocates, recycles into freeReified, freeAll() balances counts
void testReifiedAllocFree(void) {
  ReifiedVal *rv = malloc_reified(0);
  rv->type = TypeCount; // > CoreTypeCount (20) so dec_and_free recycles into freeReified[0]
  dec_and_free((Term)(u64)rv, 1);
  freeAll();
  if (malloc_count != free_count) {
    fprintf(stderr, "malloc_count=%lld free_count=%lld\n",
            (long long)malloc_count, (long long)free_count);
    abort();
  }
}

int main(int argc, char *argv[]) {
  hvmInit(1024);

  testReifiedAllocFree();

  hvmFree();
  return 0;
}
