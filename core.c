
#include "core.h"

List *empty_list = &(List){ListType,-1,0,0,0};
Integer trueVal = {IntegerType, -1, 1};
Value* true = (Value *)&trueVal;

int decRefs(Value *v, int deltaRefs) {
  int32_t refs = atomic_load(&v->refs);
  if (refs == -1)
    return(refs);

  int32_t newRefs;
  do {
    if (refs < deltaRefs) {
      fprintf(stderr, "failure in dec_and_free: %d %p\n", refs, v);
      abort();
    } else if (refs == deltaRefs)
      newRefs = -10;
    else
      newRefs = refs - deltaRefs;
  } while (!atomic_compare_exchange_strong(&v->refs, &refs, newRefs));

  return(newRefs);
}

typedef void (*freeValFn)(Value *);


#define FREE_FN_COUNT 20
freeValFn freeJmpTbl[FREE_FN_COUNT] = {NULL
				       // &freeInteger,
				       // &freeString,
				       // &freeFnArity,
				       // &freeFunction,
				       // &freeSubString
};

void dec_and_free(Value *v, int deltaRefs) {
  if (decRefs(v, deltaRefs) >= -1)
    return;

  if (v->type < FREE_FN_COUNT) {
    freeJmpTbl[v->type](v);
  }
#ifdef CHECK_MEM_LEAK
  // fprintf(stderr, "malloc_count: %lld free_count: %lld\r", malloc_count, free_count);
#endif
};
