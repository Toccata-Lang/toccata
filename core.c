
#include <stdlib.h>
#include "core.h"

void prefs(char *tag, Value *v) {
  printf("%s: %p %d\n", tag, v, v->refs);
}

#ifdef CHECK_MEM_LEAK
int64_t malloc_count = 0;
int64_t free_count = 0;
#endif

Value *nothing = (Value *)&(Maybe){MaybeType, -1, 0};
List *empty_list = &(List){ListType,-1,0,0,0};
Vector *empty_vect = &(Vector){VectorType,-1,0,5,0,0};

int32_t refsInit = 1;
int32_t staticRefsInit = -1;
int32_t refsError = -10;
Value *my_malloc(int64_t sz) {
#ifdef CHECK_MEM_LEAK
  __atomic_fetch_add(&malloc_count, 1, __ATOMIC_ACQ_REL);
#endif
  Value *val = malloc(sz);
  if (sz > sizeof(Value))
    __atomic_store(&val->refs, &refsInit, __ATOMIC_RELAXED);
  return(val);
}

typedef struct {Value *head; uintptr_t aba;} FreeValList;

Value *removeFreeValue(FreeValList *freeList) {
  FreeValList orig;
  __atomic_load((long long *)freeList, (long long *)&orig, __ATOMIC_RELAXED);
  FreeValList next = orig;
  Value *item = (Value *)0;
  if (orig.head != (Value *)0) {
    do {
      item = orig.head;
      next.head = item->next;
      next.aba = orig.aba + 1;
    } while (!__atomic_compare_exchange((long long *)freeList,
                                        (long long *)&orig,
                                        (long long *)&next, 1,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED) &&
             orig.head != (Value *)0);
    if (orig.head == (Value *)0)
      item = (Value *)0;
  }

  if (item == (Value *)0) {
    return((Value *)0);
  } else {
    int32_t refs;
    __atomic_load(&item->refs, &refs, __ATOMIC_RELAXED);
    if (refs != -10) {
      fprintf(stderr, "failure in removeFreeValue: %d\n", refs);
      abort();
    }
    return(item);
  }
}

FreeValList centralFreeIntegers = (FreeValList){(Value *)0, 0};
__thread FreeValList freeIntegers = {(Value *)0, 0};
Integer *malloc_integer() {
  Integer *newInteger = (Integer *)freeIntegers.head;
  if (newInteger == (Integer *)0) {
    newInteger = (Integer *)removeFreeValue(&centralFreeIntegers);
    if (newInteger == (Integer *)0) {
      Integer *numberStructs = (Integer *)my_malloc(sizeof(Integer) * 100);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 99, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 99; i++) {
        __atomic_store(&numberStructs[i].refs, &refsError, __ATOMIC_RELAXED);
        ((Value *)&numberStructs[i])->next = (Value *)&numberStructs[i + 1];
      }
      __atomic_store(&numberStructs[99].refs, &refsError, __ATOMIC_RELAXED);
      ((Value *)&numberStructs[99])->next = (Value *)0;
      freeIntegers.head = (Value *)&numberStructs[1];

      numberStructs->type = IntegerType;
      __atomic_store(&numberStructs->refs, &refsInit, __ATOMIC_RELAXED);
      return(numberStructs);
    }
  } else {
    freeIntegers.head = freeIntegers.head->next;
  }
  newInteger->type = IntegerType;
  __atomic_store(&newInteger->refs, &refsInit, __ATOMIC_RELAXED);
  return(newInteger);
}

int decRefs(Value *v, int deltaRefs) {
  int32_t refs;
  __atomic_load(&v->refs, &refs, __ATOMIC_RELAXED);
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
  } while (!__atomic_compare_exchange(&v->refs, &refs, &newRefs, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

  return(newRefs);
}

typedef void (*freeValFn)(Value *);

void freeInteger(Value *v) {
  v->next = freeIntegers.head;
  freeIntegers.head = v;
}

FreeValList centralFreeStrings = (FreeValList){(Value *)0, 0};
__thread FreeValList freeStrings = {(Value *)0, 0};
#define STRING_RECYCLE_LEN 100
String *malloc_string(int len) {
  String *str;
  if (len > STRING_RECYCLE_LEN) {
    str = (String *)my_malloc(sizeof(String) + len + 4);
    memset(str->buffer, 0, len + 4);
  } else {
    str = (String *)freeStrings.head;
    if (str == (String *)0) {
      str = (String *)removeFreeValue(&centralFreeStrings);
      if (str == (String *)0) {
        str = (String *)my_malloc(sizeof(String) + STRING_RECYCLE_LEN + 4);
        memset(str->buffer, 0, STRING_RECYCLE_LEN);
        str->hash = (Integer *)0;
        str->type = StringType;
        str->len = len;
      }
    } else {
      freeStrings.head = freeStrings.head->next;
    }
  }
  __atomic_store(&str->refs, &refsInit, __ATOMIC_RELAXED);
  str->hash = (Integer *)0;
  str->type = StringType;
  str->len = len;
  return(str);
}

void freeString(Value *v) {
    Integer *hash = ((String *)v)->hash;
    if (hash != (Integer *)0) {
      dec_and_free((Value *)hash, 1);
    }

    int64_t len = ((String *)v)->len;
    if (len <= STRING_RECYCLE_LEN) {
      v->next = freeStrings.head;
      freeStrings.head = v;
    } else {
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
      // __atomic_fetch_add(&free_strs, 1, __ATOMIC_ACQ_REL);
#endif
      free(v);
    }
  }

FreeValList centralFreeSubStrings = (FreeValList){(Value *)0, 0};
__thread FreeValList freeSubStrings = {(Value *)0, 0};
SubString *malloc_substring() {
  SubString *subStr = (SubString *)freeSubStrings.head;
  if (subStr == (SubString *)0) {
    subStr = (SubString *)removeFreeValue(&centralFreeSubStrings);
    if (subStr == (SubString *)0) {
      subStr = (SubString *)my_malloc(sizeof(SubString));
      subStr->hash = (Integer *)0;
      __atomic_store(&subStr->refs, &refsInit, __ATOMIC_RELAXED);
      return(subStr);
    }
  } else {
    freeSubStrings.head = freeSubStrings.head->next;
  }
  __atomic_store(&subStr->refs, &refsInit, __ATOMIC_RELAXED);
  subStr->hash = (Integer *)0;
  return(subStr);
}

void freeSubString(Value *v) {
  Value *src = ((SubString *)v)->source;
  Integer *hash = ((SubString *)v)->hash;
  if (src != (Value *)0) {
    dec_and_free(src, 1);
  }
  if (hash != (Integer *)0) {
    dec_and_free((Value *)hash, 1);
  }
  v->next = freeSubStrings.head;
  freeSubStrings.head = v;
}

FreeValList centralFreeFnArities = (FreeValList){(Value *)0, 0};
__thread FreeValList freeFnArities = {(Value *)0, 0};
FnArity *malloc_fnArity() {
  FnArity *newFnArity = (FnArity *)freeFnArities.head;
  if (newFnArity == (FnArity *)0) {
    newFnArity = (FnArity *)removeFreeValue(&centralFreeFnArities);
    if (newFnArity == (FnArity *)0) {
      newFnArity = (FnArity *)my_malloc(sizeof(FnArity));
      newFnArity->type = FnArityType;
      newFnArity->refs = 1;
      return(newFnArity);
    }
  } else {
    freeFnArities.head = freeFnArities.head->next;
  }
  newFnArity->type = FnArityType;
  __atomic_store(&newFnArity->refs, &refsInit, __ATOMIC_RELAXED);
  return(newFnArity);
}

void freeFnArity(Value *v) {
  FnArity *arity = (FnArity *)v;
  dec_and_free((Value *)arity->closures, 1);
  v->next = freeFnArities.head;
  freeFnArities.head = v;
}

FreeValList centralFreeFunctions[10] = {(FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0},
                                        (FreeValList){(Value *)0, 0}};
__thread FreeValList freeFunctions[10] = {{(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0},
                                          {(Value *)0, 0}};
Function *malloc_function(int arityCount) {
  Function *newFunction;
  if (arityCount > 9) {
    newFunction = (Function *)my_malloc(sizeof(Function) + sizeof(FnArity *) * arityCount);
    __atomic_store(&((Function *)newFunction)->refs, &refsInit, __ATOMIC_RELAXED);
    newFunction->type = FunctionType;
    return(newFunction);
  } else {
    newFunction = (Function *)freeFunctions[arityCount].head;
    if (newFunction == (Function *)0) {
      newFunction = (Function *)removeFreeValue(&centralFreeFunctions[arityCount]);
      if (newFunction == (Function *)0) {
        newFunction = (Function *)my_malloc(sizeof(Function) + sizeof(FnArity *) * arityCount);
      }
    } else {
      freeFunctions[arityCount].head = freeFunctions[arityCount].head->next;
    }
    newFunction->type = FunctionType;
    __atomic_store(&((Function *)newFunction)->refs, &refsInit, __ATOMIC_RELAXED);
    return((Function *)newFunction);
  }
}

void freeFunction(Value *v) {
  Function *f = (Function *)v;
  for (int i = 0; i < f->arityCount; i++) {
    dec_and_free((Value *)f->arities[i], 1);
  }
  // fprintf(stderr, "%p freed\n", v);
  if (f->arityCount < 10) {
    v->next = freeFunctions[f->arityCount].head;
    freeFunctions[f->arityCount].head = v;
  } else {
#ifdef CHECK_MEM_LEAK
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
    free(v);
  }
}

FreeValList centralFreeLists = (FreeValList){(Value *)0, 0};
__thread FreeValList freeLists = (FreeValList){(Value *)0, 0};
List *malloc_list() {
  List *newList = (List *)freeLists.head;
  if (newList == (List *)0) {
    newList = (List *)removeFreeValue(&centralFreeLists);
    if (newList == (List *)0) {
      List *listStructs = (List *)my_malloc(sizeof(List) * 100);
      for (int i = 1; i < 99; i++) {
        __atomic_store(&listStructs[i].refs, &refsError, __ATOMIC_RELAXED);
        ((Value *)&listStructs[i])->next = (Value *)&listStructs[i + 1];
      }
      __atomic_store(&listStructs[99].refs, &refsError, __ATOMIC_RELAXED);
      ((Value *)&listStructs[99])->next = freeLists.head;
      freeLists.head = (Value *)&listStructs[1];
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 99, __ATOMIC_ACQ_REL);
#endif

      listStructs->type = ListType;
      __atomic_store(&listStructs->refs, &refsInit, __ATOMIC_RELAXED);
      listStructs->head = (Value *)0;
      listStructs->tail = (List *)0;
      listStructs->len = 0;
      return(listStructs);
    }
  } else {
    freeLists.head = freeLists.head->next;
  }

  newList->type = ListType;
  newList->refs = 1;
  newList->head = (Value *)0;
  newList->tail = (List *)0;
  newList->len = 0;
  return(newList);
}

void freeList(Value *v) {
  List *l = (List *)v;
  Value *head = l->head;
  if (head != (Value *)0)
    dec_and_free(head, 1);
  List *tail = l->tail;
  l->tail = (List *)0;
  v->next = freeLists.head;
  freeLists.head = v;
  if (tail != (List *)0)
    dec_and_free((Value *)tail, 1);
}

FreeValList centralFreeMaybes = (FreeValList){(Value *)0, 0};
__thread FreeValList freeMaybes = {(Value *)0, 0};
Maybe *malloc_maybe() {
  Maybe *newMaybe = (Maybe *)freeMaybes.head;
  if (newMaybe == (Maybe *)0) {
    newMaybe = (Maybe *)removeFreeValue(&centralFreeMaybes);
    if (newMaybe == (Maybe *)0) {
      Maybe *maybeStructs = (Maybe *)my_malloc(sizeof(Maybe) * 50);
      for (int i = 1; i < 49; i++) {
        __atomic_store(&maybeStructs[i].refs, &refsError, __ATOMIC_RELAXED);
        ((Value *)&maybeStructs[i])->next = (Value *)&maybeStructs[i + 1];
      }
      __atomic_store(&maybeStructs[49].refs, &refsError, __ATOMIC_RELAXED);
      ((Value*)&maybeStructs[49])->next = (Value *)0;
      freeMaybes.head = (Value *)&maybeStructs[1];
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 49, __ATOMIC_ACQ_REL);
#endif

      maybeStructs->type = MaybeType;
      __atomic_store(&maybeStructs->refs, &refsInit, __ATOMIC_RELAXED);
      return(maybeStructs);
    }
  } else {
    freeMaybes.head = freeMaybes.head->next;
  }
  newMaybe->type = MaybeType;
  __atomic_store(&newMaybe->refs, &refsInit, __ATOMIC_RELAXED);
  return(newMaybe);
}

void freeMaybe(Value *v) {
  Value *value = ((Maybe *)v)->value;
  dec_and_free(value, 1);
  v->next = freeMaybes.head;
  freeMaybes.head = v;
}

FreeValList centralFreeVectorNodes = (FreeValList){(Value *)0, 0};
__thread FreeValList freeVectorNodes = {(Value *)0, 0};
VectorNode *malloc_vectorNode() {
  VectorNode *newVectorNode = (VectorNode *)freeVectorNodes.head;
  if (newVectorNode == (VectorNode *)0) {
    newVectorNode = (VectorNode *)removeFreeValue(&centralFreeVectorNodes);
    if (newVectorNode == (VectorNode *)0) {
      VectorNode *nodeStructs = (VectorNode *)my_malloc(sizeof(VectorNode) * 50);
      for (int i = 1; i < 49; i++) {
        __atomic_store(&nodeStructs[i].refs, &refsError, __ATOMIC_RELAXED);
        ((Value *)&nodeStructs[i])->next = (Value *)&nodeStructs[i + 1];
      }
      __atomic_store(&nodeStructs[49].refs, &refsError, __ATOMIC_RELAXED);
      ((Value*)&nodeStructs[49])->next = (Value *)0;
      freeVectorNodes.head = (Value *)&nodeStructs[1];
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 49, __ATOMIC_ACQ_REL);
#endif

      nodeStructs->type = VectorNodeType;
      __atomic_store(&nodeStructs->refs, &refsInit, __ATOMIC_RELAXED);
      memset(&nodeStructs->array, 0, sizeof(Value *) * VECTOR_ARRAY_LEN);
      return(nodeStructs);
    }
  } else {
    freeVectorNodes.head = freeVectorNodes.head->next;
  }
  newVectorNode->type = VectorNodeType;
  __atomic_store(&newVectorNode->refs, &refsInit, __ATOMIC_RELAXED);
  memset(&newVectorNode->array, 0, sizeof(Value *) * VECTOR_ARRAY_LEN);
  return(newVectorNode);
}

void freeVectorNode(Value *v) {
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((VectorNode *)v)->array[i] != (Value *)0) {
      dec_and_free(((VectorNode *)v)->array[i], 1);
    }
  }
  v->next = freeVectorNodes.head;
  freeVectorNodes.head = v;
}

FreeValList centralFreeVectors = (FreeValList){(Value *)0, 0};
__thread FreeValList freeVectors = {(Value *)0, 0};
Vector *malloc_vector() {
  Vector *newVector = (Vector *)freeVectors.head;
  if (newVector == (Vector *)0) {
    newVector = (Vector *)removeFreeValue(&centralFreeVectors);
    if (newVector == (Vector *)0) {
      Vector *vectorStructs = (Vector *)my_malloc(sizeof(Vector) * 300);
      for (int i = 1; i < 299; i++) {
        __atomic_store(&vectorStructs[i].refs, &refsError, __ATOMIC_RELAXED);
        ((Value *)&vectorStructs[i])->next = (Value *)&vectorStructs[i + 1];
      }
      __atomic_store(&vectorStructs[299].refs, &refsError, __ATOMIC_RELAXED);
      ((Value*)&vectorStructs[299])->next = (Value *)0;
      freeVectors.head = (Value *)&vectorStructs[1];
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 299, __ATOMIC_ACQ_REL);
#endif

      vectorStructs->type = VectorType;
      __atomic_store(&vectorStructs->refs, &refsInit, __ATOMIC_RELAXED);
      vectorStructs->count = 0;
      vectorStructs->shift = 5;
      vectorStructs->root = (VectorNode *)0;
      vectorStructs->root = (VectorNode *)0;
      memset(&vectorStructs->tail, 0, sizeof(Value *) * VECTOR_ARRAY_LEN);
      return(vectorStructs);
    }
  } else {
    freeVectors.head = freeVectors.head->next;
  }
  newVector->type = VectorType;
  __atomic_store(&newVector->refs, &refsInit, __ATOMIC_RELAXED);
  newVector->count = 0;
  newVector->shift = 5;
  newVector->root = (VectorNode *)0;
  newVector->root = (VectorNode *)0;
  memset(&newVector->tail, 0, sizeof(Value *) * VECTOR_ARRAY_LEN);
  return(newVector);
}

void freeVector(Value *v) {
  Value *root = (Value *)((Vector *)v)->root;
  if (root != (Value *)0)
    dec_and_free(root, 1);
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((Vector *)v)->tail[i] != (Value *)0) {
      dec_and_free(((Vector *)v)->tail[i], 1);
    }
  }
  v->next = freeVectors.head;
  freeVectors.head = v;
}

#define FREE_FN_COUNT 20
freeValFn freeJmpTbl[FREE_FN_COUNT] = {NULL,
				       &freeInteger,
				       &freeString,
				       &freeFnArity,
				       &freeFunction,
				       &freeSubString,
				       &freeList,
				       &freeMaybe,
				       &freeVector,
				       &freeVectorNode
};

void dec_and_free(Value *v, int deltaRefs) {
  if (decRefs(v, deltaRefs) >= -1)
    return;

  if (v->type < FREE_FN_COUNT) {
    freeJmpTbl[v->type](v);
  }
#ifdef CHECK_MEM_LEAK
  // fprintf(stderr, "malloc_count: %ld free_count: %ld\r", malloc_count, free_count);
#endif
};

Value *incRef(Value *v, int deltaRefs) {
  if (deltaRefs < 0) {
    fprintf(stderr, "bad deltaRefs: %p\n", v);
    abort();
  }

  if (deltaRefs < 1)
    return(v);

  int32_t refs;
  __atomic_load(&v->refs, &refs, __ATOMIC_RELAXED);
  if (refs < -1) {
    fprintf(stderr, "failure in incRef: %d %p\n", refs, v);
    abort();
  }

  if (refs >= 0)
    __atomic_fetch_add(&v->refs, deltaRefs, __ATOMIC_ACQ_REL);
  return(v);
}

#ifdef CHECK_MEM_LEAK
void moveFreeToCentral();

void freeGlobal(Value *x) {
  if (x->refs == -10)
    return;
  x->refs = 1;
  dec_and_free(x, 1);
}

void emptyFreeList(FreeValList *freeLinkedList) {
  FreeValList listHead;
  __atomic_load((long long *)freeLinkedList, (long long *)&listHead, __ATOMIC_RELAXED);
  for(Value *item = listHead.head;
      item != (Value *)0;
      item =  item->next) {
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
  }
}

void freeAll() {
  moveFreeToCentral();

  for (int i = 0; i < 10; i++) {
    emptyFreeList(&centralFreeFunctions[i]);
  }
  // for (int i = 0; i < 20; i++) {
  // emptyFreeList(&centralFreeReified[i]);
  // }
  // for (int i = 0; i < 20; i++) {
  // emptyFreeList(&centralFreeBMINodes[i]);
  // }
  // emptyFreeList(&centralFreeFutures);
  // FreeValList listHead;
  // __atomic_load((long long *)&centralFreeFutures,
  //               (long long *)&listHead, __ATOMIC_RELAXED);
  // emptyFreeList(&centralFreePromises);
  // emptyFreeList(&centralFreeArrayNodes);
  emptyFreeList(&centralFreeSubStrings);
  emptyFreeList(&centralFreeFnArities);
  emptyFreeList(&centralFreeLists);
  emptyFreeList(&centralFreeMaybes);
  emptyFreeList(&centralFreeVectors);
  emptyFreeList(&centralFreeVectorNodes);
  emptyFreeList(&centralFreeStrings);
  emptyFreeList(&centralFreeIntegers);

//*
  int64_t mallocs;
  __atomic_load(&malloc_count, &mallocs, __ATOMIC_RELAXED);
  int64_t frees;
  __atomic_load(&free_count, &frees, __ATOMIC_RELAXED);
  fprintf(stderr, "malloc count: %ld  free count: %ld  diff: %ld\n",
          mallocs, frees, mallocs - frees);
// */
}
#endif

void moveToCentral(FreeValList *freeList, FreeValList *centralList) {
  Value *tail = freeList->head;
  while (tail != (Value *)0 && tail->next != (Value *)0) {
    tail = tail->next;
  }
  
  if (tail == (Value *)0)
    return;
  else {
    FreeValList orig;
    __atomic_load((long long *)centralList, (long long *)&orig, __ATOMIC_RELAXED);
    FreeValList next = orig;
    do {
      tail->next = orig.head;
      next.head = freeList->head;
      next.aba = orig.aba + 1;
    } while (!__atomic_compare_exchange((long long *)centralList, (long long *)&orig, (long long *)&next, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    freeList->head = (Value *)0;
    return;
  }
}

void moveFreeToCentral() {
  moveToCentral(&freeLists, &centralFreeLists);
  for (int i = 0; i < 10; i++) {
    moveToCentral(&freeFunctions[i], &centralFreeFunctions[i]);
  }
  // for (int i = 0; i < 20; i++) {
  // moveToCentral(&freeBMINodes[i], &centralFreeBMINodes[i]);
  // }
  // for (int i = 0; i < 20; i++) {
  // moveToCentral(&freeReified[i], &centralFreeReified[i]);
  // }
  moveToCentral(&freeStrings, &centralFreeStrings);
  // moveToCentral(&freeArrayNodes, &centralFreeArrayNodes);
  moveToCentral(&freeSubStrings, &centralFreeSubStrings);
  moveToCentral(&freeIntegers, &centralFreeIntegers);
  moveToCentral(&freeMaybes, &centralFreeMaybes);
  moveToCentral(&freeVectors, &centralFreeVectors);
  moveToCentral(&freeVectorNodes, &centralFreeVectorNodes);
  moveToCentral(&freeFnArities, &centralFreeFnArities);
  // moveToCentral(&freePromises, &centralFreePromises);
  // moveToCentral(&freeFutures, &centralFreeFutures);
}

int8_t isNothing(Value *v) {
  return(v->type == MaybeType && ((Maybe *)v)->value == (Value *)0);
}

Value *maybe(List *closures, Value *arg0, Value *arg1) {
  Maybe *mVal = malloc_maybe();
  mVal->value = arg1;
  return((Value *)mVal);
}

Value *intValue(int64_t n) {
  Integer *numVal = malloc_integer();
  numVal->numVal = n;
  return((Value *)numVal);
};

Value *pr_STAR(Value *str) {
  int bytes;
  if (str->type == StringType) {
    bytes = fprintf(outstream, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    bytes = fprintf(outstream, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  } else {
    fprintf(stderr, "\ninvalid type for 'pr*'\n");
    abort();
  }
  dec_and_free(str, 1);
  return(intValue(bytes));
}

Value *add_ints(Value *arg0, Value *arg1) {
  if (arg0->type != arg1->type) {
    fprintf(stderr, "\ninvalid types for 'add-numbers'\n");
    abort();
  } else {
    Value *numVal = intValue(((Integer *)arg0)->numVal + ((Integer *)arg1)->numVal);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(numVal);
  }
}

Value *integer_str(Value *arg0) {
  String *numStr = malloc_string(10);
  snprintf(numStr->buffer, 9, "%ld", ((Integer *)arg0)->numVal);
  numStr->len = strlen(numStr->buffer);
  dec_and_free(arg0, 1);
  return((Value *)numStr);
}

Value *integer_EQ(Value *arg0, Value *arg1) {
  if (IntegerType != arg0->type || IntegerType != arg1->type) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else if (((Integer *)arg0)->numVal != ((Integer *)arg1)->numVal) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else {
    dec_and_free(arg1, 1);
    return(maybe((List *)0, (Value *)0, arg0));
  }
}

List *listCons(Value *x, List *l) {
  if (l->type != ListType) {
    fprintf(stderr, "'cons' requires a list\n");
    abort();
  }
  List *newList = malloc_list();
  newList->len = l->len + 1;
  newList->head = (Value *)x;
  newList->tail = l;
  return(newList);
};

Vector *newVector(Value *array[], int indexToSkip) {
  Vector *ret = malloc_vector();
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (array[i] != (Value *)0 && i != indexToSkip) {
      ret->tail[i] = array[i];
      incRef(array[i], 1);
    }
  }
  return(ret);
}

VectorNode *newVectorNode(Value *array[], int indexToSkip) {
  VectorNode *ret = malloc_vectorNode();
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (array[i] != (Value *)0 && i != indexToSkip) {
      ret->array[i] = array[i];
      incRef(array[i], 1);
    }
  }
  return(ret);
}

Value **arrayFor(Vector *v, unsigned index) {
  if (index < v->count) {
    if (index >= v->tailOffset) {
      return(v->tail);
    } else {
      VectorNode *node = v->root;
      for (int level = v->shift; level > 0; level -= 5) {
        node = (VectorNode *)node->array[(index >> level) & 0x1f];
      }
      return(node->array);
    }
  } else {
    fprintf(stderr, "Vector index out of bounds\n");
    abort();
    return((Value **)0);
  }
}

VectorNode *newPath(int level, VectorNode *node) {
  if (level == 0) {
    return(node);
  } else {
    VectorNode *ret = malloc_vectorNode();
    ret->array[0] = (Value *)newPath(level - 5, node);
    return(ret);
  }
}

VectorNode *pushTail(unsigned count, int level, VectorNode *parent, VectorNode *tailNode) {
  int subidx = ((count - 1) >> level) & 0x1f;
  VectorNode *ret;
  if (parent != (VectorNode *)0) {
    ret = newVectorNode(parent->array, subidx);
  } else {
    ret = malloc_vectorNode();
  }
  VectorNode *nodeToInsert;
  if (level == 5) {
    nodeToInsert = tailNode;
  } else {
    VectorNode *child = (VectorNode *)parent->array[subidx];
    if (child != (VectorNode *)0) {
      nodeToInsert = pushTail(count, level - 5, child, tailNode);
    } else {
      nodeToInsert = newPath(level - 5, tailNode);
    }
  }
  ret->array[subidx] = (Value *)nodeToInsert;
  return(ret);
}

Vector *vectCons(Vector *vect, Value *val) {
  // if there's room in the tail
  if (vect->count - vect->tailOffset < VECTOR_ARRAY_LEN) {
    // make a new vector and copy info over
    Vector *newVect = newVector(vect->tail, VECTOR_ARRAY_LEN);
    newVect->shift = vect->shift;
    newVect->count = vect->count + 1;
    if (newVect->count < 32) {
      newVect->tailOffset = 0;
    } else {
      newVect->tailOffset = (newVect->count - 1) & ~0x1f;
    }
    newVect->root = vect->root;
    if (newVect->root != (VectorNode *)0) {
      incRef((Value *)newVect->root, 1);
    }

    // add value to tail of new vector
    newVect->tail[vect->count & 0x1F] = val;
    return(newVect);
  } else {
    // since tail is full, make a new node from the tail of 'vect'
    VectorNode *newRoot;
    VectorNode *tailNode = newVectorNode(vect->tail, VECTOR_ARRAY_LEN);
    int newShift = vect->shift;

    // if the root of 'vect' is completely full
    if ((vect->count >> 5) > (1 << vect->shift)) {
      // make new vector one level deeper
      newRoot = malloc_vectorNode();
      newRoot->array[0] = (Value *)vect->root;
      incRef(newRoot->array[0], 1);

      // and make a new path that includes that node
      newRoot->array[1] = (Value *)newPath(vect->shift, tailNode);
      newShift += 5;
    } else {
      // otherwise, push the tail node down, creating a new root
      newRoot = pushTail(vect->count, vect->shift, vect->root, tailNode);
    }
    Vector *newVect = malloc_vector();
    newVect->count = vect->count + 1;
    newVect->tailOffset = (newVect->count - 1) & ~0x1f;
    newVect->shift = newShift;
    newVect->root = newRoot;
    newVect->tail[0] = val;
    return(newVect);
  }
}

Vector *mutateVectConj(Vector *vect, Value *val) {
  // if 'vect' is a static vector
  if (vect->refs == -1) {
    Vector *result = vectCons(vect, val);
    return(result);
  } else if (vect->count - vect->tailOffset < VECTOR_ARRAY_LEN) {
    // if there's room in the tail, add value to tail of vector
    vect->tail[vect->count & 0x1F] = val;
    vect->count += 1;
    return(vect);
  } else {
    // since tail is full, make a new node from the tail of 'vect'
    VectorNode *newRoot;
    VectorNode *tailNode = newVectorNode(vect->tail, VECTOR_ARRAY_LEN);
    for (unsigned i = 0; i < VECTOR_ARRAY_LEN; i++) {
      dec_and_free(vect->tail[i], 1);
      vect->tail[i] = (Value *)0;
    }
    int newShift = vect->shift;

    // if the root of 'vect' is completely full
    if ((vect->count >> 5) > (1 << vect->shift)) {
      // make new vector one level deeper
      newRoot = malloc_vectorNode();
      newRoot->array[0] = (Value *)vect->root;

      // and make a new path that includes that node
      newRoot->array[1] = (Value *)newPath(vect->shift, tailNode);
      newShift += 5;
    } else {
      // make new vector one level deeper
      // otherwise, push the tail node down, creating a new root
      newRoot = pushTail(vect->count, vect->shift, vect->root, tailNode);
      if (vect->root != (VectorNode *)0)
        dec_and_free((Value *)vect->root, 1);
    }
    vect->count += 1;
    vect->tailOffset = (vect->count - 1) & ~0x1f;
    vect->shift = newShift;
    vect->root = newRoot;
    vect->tail[0] = val;
    return(vect);
  }
}

Value *vectGet(Vector *vect, unsigned index) {
  // this fn does not dec_and_free vect on purpose
  // lets calling functions do that.
  Value **array = arrayFor(vect, index);
  return(array[index & 0x1f]);
}

Value *vectSeq(Vector *vect, int index) {
  List *ret = empty_list;
  if (vect->count > 0) {
    for (int i = vect->count - 1; i >= index; i -= 1) {
      Value *v = vectGet(vect, (unsigned)i);
      incRef(v, 1);
      ret = listCons(v, ret);
    }
  }
  dec_and_free((Value *)vect, 1);
  return((Value *)ret);
}

void destructValue(char *fileName, char *lineNum, Value *val, int numArgs, Value **args[]) {
  if (val->type == ListType) {
    List *l = (List *)val;
    if (l->len < numArgs - 1) {
      fprintf(stderr, "Insufficient values in list for destructuring at %s: %s\n",
	      fileName, lineNum);
      abort();
    }
    int64_t len = l->len - numArgs + 1;
    for (int i = 0; i < numArgs - 1; i++) {
      *args[i] = l->head; l = l->tail;
      incRef(*args[i], 1);
    }
    l->len = len;
    *args[numArgs - 1] = (Value *)l;
    incRef(*args[numArgs - 1], 1);
    dec_and_free(val, 1);
  } else if (val->type == VectorType) {
    Vector *v = (Vector *)val;
    if (v->count < numArgs - 1) {
      fprintf(stderr, "Insufficient values in vector for destructuring at %s: %s\n",
	      fileName, lineNum);
      abort();
    }
    // unpack vector
    for (int i = 0; i < numArgs - 1; i++) {
      *args[i] = vectGet(v, i);
      incRef(*args[i], 1);
    }
    *args[numArgs - 1] = vectSeq(v, numArgs - 1);
  } else {
    fprintf(stderr, "Could not unpack value at %s %s\n", fileName, lineNum);
    abort();
  }
}
