
/*
  TODO: Possible major perf boost.
The only way values can be passed between threads is by a closure arity being passed to an agent or
a future. (Ponder this more).

In those cases, a special bit could be set that would cause any ref updates to be done atomically.
If the bit is not set, the ref update could be done immediately.
Problems with values nested in others. And values delivered to Promises.
 */

#include <stdlib.h>
#include <stdatomic.h>
#include "core.h"

REFS_SIZE refsInit = 1;
REFS_SIZE refsError = -10;
REFS_SIZE refsConstant = -1;
REFS_SIZE refsStatic = REFS_STATIC;

Value *universalProtoFn = (Value *)0;
Integer const0 = {IntegerType, -2, 0};
Value *const0Ptr = (Value *)&const0;
List *globals = &empty_list_struct;
int cleaningUp = 0;

// Immutable hash-map ported from Clojure
BitmapIndexedNode emptyBMI = {BitmapIndexedType, -2, 0, 0};

// threads that have been replaced, but haven't exited
pthread_mutex_t lingeringAccess = PTHREAD_MUTEX_INITIALIZER;
Value *lingeringThreads = (Value *)&emptyBMI;

Value *maybeNothing;
FILE *outstream;
Value *(*prErrSTAR)(Value *str);

void prefs(char *tag, Value *v) {
  if (v != (Value *)0)
    fprintf(stderr, "%s: %p %d\n", tag, v, v->refs);
  else
    fprintf(stderr, "%s: %p\n", tag, v);
}

int64_t malloc_count = 0;
int64_t free_count = 0;

int64_t type_mallocs[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int64_t type_frees[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void incTypeMalloc(TYPE_SIZE type, int delta) {
  if (type < 0) {
    fprintf(stderr, "type malloc inc failure\n");
    abort();
  }
  else if (type == TypeCount)
    __atomic_fetch_add(&type_mallocs[19], delta, __ATOMIC_ACQ_REL);
  else if (type > OpaqueType)
    __atomic_fetch_add(&type_mallocs[0], delta, __ATOMIC_ACQ_REL);
  else if (type == SymbolType)
    __atomic_fetch_add(&type_mallocs[5], delta, __ATOMIC_ACQ_REL);
  else
    __atomic_fetch_add(&type_mallocs[type], delta, __ATOMIC_ACQ_REL);
}

void incTypeFree(TYPE_SIZE type, int delta) {
  if (type < 0) {
    fprintf(stderr, "type free inc failure\n");
    abort();
  }
  else if (type == TypeCount)
    __atomic_fetch_add(&type_frees[19], delta, __ATOMIC_ACQ_REL);
  else if (type > OpaqueType)
    __atomic_fetch_add(&type_frees[0], delta, __ATOMIC_ACQ_REL);
  else if (type == SymbolType)
    __atomic_fetch_add(&type_frees[5], delta, __ATOMIC_ACQ_REL);
  else
    __atomic_fetch_add(&type_frees[type], delta, __ATOMIC_ACQ_REL);
}

Maybe nothing_struct = {MaybeType, -2, 0, 0};
Value *nothing = (Value *)&nothing_struct;
List empty_list_struct = (List){ListType,-2,0,0,0,0};
List *empty_list = &empty_list_struct;
Vector empty_vect_struct = (Vector){VectorType,-2,0,0,5,0,0};
Vector *empty_vect = &empty_vect_struct;

ReifiedVal all_values_struct = {AllValuesType, -2, 0};
Value *all_values = (Value *)&all_values_struct;

#ifdef SINGLE_THREADED
#define NUM_WORKERS 1
#else
#ifndef NUM_WORKERS
#define NUM_WORKERS 10
#endif
#endif
pthread_t workers[NUM_WORKERS];
int32_t runningWorkers = NUM_WORKERS;
int8_t mainThreadDone = 0;

int mask(int64_t hash, int shift) {
  return (hash >> shift) & 0x1f;
}

int bitpos(int64_t hash, int shift) {
  return 1 << mask(hash, shift);
}

Value *my_malloc(int64_t sz) {
#ifdef CHECK_MEM_LEAK
  __atomic_fetch_add(&malloc_count, 1, __ATOMIC_ACQ_REL);
#endif
  Value *val = malloc(sz);
  if (sz > sizeof(Value)) {
#ifndef SINGLE_THREADED
#ifdef FAST_INCS
    fprintf(stderr, "FAST_INCS can only be defined when SINGLE_THREADED is defined.\n");
    abort();
#endif
#ifdef FAST_DECS
    fprintf(stderr, "FAST_DECS can only be defined when SINGLE_THREADED is defined.\n");
    abort();
#endif
#endif
    val->refs = refsInit;
  }
  return(val);
}

void cleanupMemory (Value *the_final_answer, Value *maybeNothing, List *argVect) {
  // TODO: change the type of argVect to Vector eventually
#ifdef CHECK_MEM_LEAK
  dec_and_free(the_final_answer, 1);
  freeGlobal((Value *)argVect);
  freeGlobal(maybeNothing);
  for (List *l = globals; l != (List *)0 && l->tail != (List *)0; l = l->tail) {
    if (l->head->refs == refsConstant)
      l->head->refs = 1;
  }
  dec_and_free((Value *)globals, 1);
  freeAll();
#endif
}

typedef struct {Value *head; uintptr_t aba;} FreeValList;

Value *removeFreeValue(FreeValList *freeList) {
  Value *item = (Value *)0;
  FreeValList orig;
#ifdef SINGLE_THREADED
  orig = *freeList;
  FreeValList next = orig;
  item = orig.head;
  if (item == (Value *)0) {
    return((Value *)0);
  } else {
    next.head = item->next;
    freeList = &next;
    if (item->refs != refsError) {
      fprintf(stderr, "failure in removeFreeValue: %d\n", item->refs);
      abort();
    }
    return(item);
  }
#else
  __atomic_load((FreeValList *)freeList, (FreeValList *)&orig, __ATOMIC_RELAXED);
  FreeValList next = orig;
  if (orig.head != (Value *)0) {
    do {
      item = orig.head;
      next.head = item->next;
      next.aba = orig.aba + 1;
    } while (!__atomic_compare_exchange((FreeValList *)freeList,
					(FreeValList *)&orig,
					(FreeValList *)&next, 1,
					__ATOMIC_RELAXED, __ATOMIC_RELAXED) &&
	     orig.head != (Value *)0);
    if (orig.head == (Value *)0)
      item = (Value *)0;
  }

  if (item == (Value *)0) {
    return((Value *)0);
  } else {
    REFS_SIZE refs;
    __atomic_load(&item->refs, &refs, __ATOMIC_RELAXED);
    if (refs != refsError) {
      fprintf(stderr, "failure in removeFreeValue: %d\n", refs);
      abort();
    }
    return(item);
  }
#endif
}

int decRefs(Value *v, int deltaRefs) {
#ifndef FAST_DECS
#ifdef SINGLE_THREADED
  if (v->refs == refsConstant || v->refs == refsStatic)
    return(v->refs);

  if (v->refs < deltaRefs) {
    fprintf(stderr, "\nfailure in decRefs, refs too small: %d %p\n", v->refs, v);
    abort();
  } else if (v->refs == deltaRefs)
    v->refs = refsError;
  else
    v->refs -= deltaRefs;
  return(v->refs);
#else
  // not SINGLE_THREADED
  if (v->refs == refsConstant || v->refs == refsStatic)
    return(v->refs);

  REFS_SIZE newRefs = __atomic_fetch_sub(&v->refs, deltaRefs, __ATOMIC_ACQ_REL);
  if (newRefs > deltaRefs)
    return(newRefs - deltaRefs);
  else if (newRefs == deltaRefs) {
    v->refs = refsError;
    return(refsError);
  }

  fprintf(stderr, "\nfailure in decRefs, refs too small: %d %d %p\n", deltaRefs, v->refs, v);
  abort();
  return(refsError);
#endif
#else
  // FAST_DECS
  if (v->refs == refsConstant ||
      v->refs == refsStatic)
    return(v->refs);

  if (v->refs == deltaRefs)
    v->refs = refsError;
  else
    v->refs -= deltaRefs;
  return(v->refs);
#endif
}

void moveToCentral(FreeValList *freeList, FreeValList *centralList) {
  Value *tail = freeList->head;
  while (tail != (Value *)0 && tail->next != (Value *)0) {
    tail = tail->next;
  }

  if (tail == (Value *)0)
    return;
  else {
    FreeValList orig;
#ifdef SINGLE_THREADED
    orig = *centralList;
    FreeValList next = orig;
    tail->next = orig.head;
    next.head = freeList->head;
    next.aba = orig.aba + 1;
    *centralList = next;
#else
    FreeValList next;
    __atomic_load((FreeValList *)centralList, (FreeValList *)&orig, __ATOMIC_RELAXED);
    do {
      tail->next = orig.head;
      next.head = freeList->head;
      next.aba = orig.aba + 1;
    } while (!__atomic_compare_exchange((FreeValList *)centralList,
					(FreeValList *)&orig,
					(FreeValList *)&next,
					1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
#endif
    freeList->head = (Value *)0;
    return;
  }
}

void decValuePtrRef(Value **ptr) {
  Value *toFree = (Value *)0;
  Value *oldPtr = (Value *)0;

  __atomic_exchange(ptr, &toFree, &oldPtr, __ATOMIC_RELAXED);
  if (oldPtr != (Value *)0) {
    dec_and_free((Value *)oldPtr, 1);
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
      // incTypeMalloc(TypeCount, 1);
#endif
      for (int i = 1; i < 99; i++) {
	numberStructs[i].refs = refsError;
        ((Value *)&numberStructs[i])->next = (Value *)&numberStructs[i + 1];
      }
      numberStructs[99].refs = refsError;
      ((Value *)&numberStructs[99])->next = (Value *)0;
      freeIntegers.head = (Value *)&numberStructs[1];
      moveToCentral(&freeIntegers, &centralFreeIntegers);

      newInteger = numberStructs;
    }
  } else {
    freeIntegers.head = freeIntegers.head->next;
  }
  // incTypeMalloc(IntegerType, 1);
  newInteger->type = IntegerType;
  newInteger->refs = refsInit;
  return(newInteger);
}

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
      }
    } else {
      freeStrings.head = freeStrings.head->next;
    }
  }
  // incTypeMalloc(StringBufferType, 1);
  str->refs = refsInit;
  str->hashVal = 0;
  str->type = StringBufferType;
  str->len = len;
  return(str);
}

void freeString(Value *v) {
  int64_t len = ((String *)v)->len;
  if (len <= STRING_RECYCLE_LEN) {
    v->next = freeStrings.head;
    freeStrings.head = v;
  } else {
#ifdef CHECK_MEM_LEAK
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
    if (!cleaningUp)
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
    }
  } else {
    freeSubStrings.head = freeSubStrings.head->next;
  }
  // incTypeMalloc(SubStringType, 1);
  subStr->refs = refsInit;
  subStr->hashVal = 0;
  return(subStr);
}

void freeSubString(Value *v) {
  Value *src = ((SubString *)v)->source;
  if (src != (Value *)0) {
    dec_and_free(src, 1);
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
    }
  } else {
    freeFnArities.head = freeFnArities.head->next;
  }
  // incTypeMalloc(FnArityType, 1);
  newFnArity->parent = (Value *)0;
  newFnArity->type = FnArityType;
  newFnArity->refs = refsInit;
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
  }
  // incTypeMalloc(FunctionType, 1);
  newFunction->type = FunctionType;
  ((Function *)newFunction)->refs = refsInit;
  return((Function *)newFunction);
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
    if (!cleaningUp)
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
#ifdef CHECK_MEM_LEAK
      // incTypeMalloc(TypeCount, 1);
      __atomic_fetch_add(&malloc_count, 99, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 99; i++) {
        listStructs[i].refs = refsError;
        ((Value *)&listStructs[i])->next = (Value *)&listStructs[i + 1];
      }
      listStructs[99].refs = refsError;
      ((Value *)&listStructs[99])->next = freeLists.head;
      freeLists.head = (Value *)&listStructs[1];
      moveToCentral(&freeLists, &centralFreeLists);

      newList = listStructs;
    }
  } else {
    freeLists.head = freeLists.head->next;
  }

  // incTypeMalloc(ListType, 1);
  newList->type = ListType;
  newList->refs = refsInit;
  newList->hashVal = 0;
  newList->head = (Value *)0;
  newList->tail = (List *)0;
  newList->len = 0;
  return(newList);
}

void freeList(Value *v) {
  List *l = (List *)v;
  Value *head = l->head;
  if (head != (Value *)NULL) {
    dec_and_free(head, 1);
  }
  List *tail = l->tail;
  l->tail = (List *)0;
  v->next = freeLists.head;
  freeLists.head = v;
#ifdef SINGLE_THREADED
  if (tail != (List *)0) {
    if (tail->refs == 1) {
      tail->refs = refsError;
      freeList((Value *)tail);
    } else {
      decRefs((Value *)tail, 1);
    }
  }
#else
  if (tail != (List *)0) {
    REFS_SIZE refs = tail->refs;
    if (refs != 1) {
      decRefs((Value *)tail, 1);
    } else {
      tail->refs = refsError;
      freeList((Value *)tail);
    }
  }
#endif
}

FreeValList centralFreeMaybes = (FreeValList){(Value *)0, 0};
__thread FreeValList freeMaybes = {(Value *)0, 0};
Maybe *malloc_maybe() {
  Maybe *newMaybe = (Maybe *)freeMaybes.head;
  if (newMaybe == (Maybe *)0) {
    newMaybe = (Maybe *)removeFreeValue(&centralFreeMaybes);
    if (newMaybe == (Maybe *)0) {
      Maybe *maybeStructs = (Maybe *)my_malloc(sizeof(Maybe) * 50);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 49, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 49; i++) {
        maybeStructs[i].refs = refsError;
        ((Value *)&maybeStructs[i])->next = (Value *)&maybeStructs[i + 1];
      }
      maybeStructs[49].refs = refsError;
      ((Value*)&maybeStructs[49])->next = (Value *)0;
      freeMaybes.head = (Value *)&maybeStructs[1];
      moveToCentral(&freeMaybes, &centralFreeMaybes);

      newMaybe = maybeStructs;
    }
  } else {
    freeMaybes.head = freeMaybes.head->next;
  }
  // incTypeMalloc(MaybeType, 1);
  newMaybe->type = MaybeType;
  newMaybe->refs = refsInit;
  newMaybe->hashVal = 0;
  newMaybe->value = (Value *)0;
  return(newMaybe);
}

void freeMaybe(Value *v) {
  Value *value = ((Maybe *)v)->value;
  if (value != (Value *)0)
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
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 49, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 49; i++) {
        nodeStructs[i].refs = refsError;
        ((Value *)&nodeStructs[i])->next = (Value *)&nodeStructs[i + 1];
      }
      nodeStructs[49].refs = refsError;
      ((Value*)&nodeStructs[49])->next = (Value *)0;
      freeVectorNodes.head = (Value *)&nodeStructs[1];
      moveToCentral(&freeVectorNodes, &centralFreeVectorNodes);

      newVectorNode = nodeStructs;
    }
  } else {
    freeVectorNodes.head = freeVectorNodes.head->next;
  }
  // incTypeMalloc(VectorNodeType, 1);
  newVectorNode->type = VectorNodeType;
  newVectorNode->refs = refsInit;
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
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 299, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 299; i++) {
        vectorStructs[i].refs = refsError;
        ((Value *)&vectorStructs[i])->next = (Value *)&vectorStructs[i + 1];
      }
      vectorStructs[299].refs = refsError;
      ((Value*)&vectorStructs[299])->next = (Value *)0;
      freeVectors.head = (Value *)&vectorStructs[1];
      moveToCentral(&freeVectors, &centralFreeVectors);

      newVector = vectorStructs;
    }
  } else {
    freeVectors.head = freeVectors.head->next;
  }
  // incTypeMalloc(VectorType, 1);
  newVector->type = VectorType;
  newVector->refs = refsInit;
  newVector->count = 0;
  newVector->shift = 5;
  newVector->root = (VectorNode *)0;
  newVector->hashVal = 0;
  memset(&newVector->tail, 0, sizeof(Value *) * VECTOR_ARRAY_LEN);
  return(newVector);
}

void freeVector(Value *v) {
  Value *root = (Value *)((Vector *)v)->root;
  if (root != (Value *)0) {
    dec_and_free((Value *)root, 1);
  }

  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((Vector *)v)->tail[i] != (Value *)0)
      dec_and_free(((Vector *)v)->tail[i], 1);
  }
  v->next = freeVectors.head;
  freeVectors.head = v;
}

FreeValList centralFreeReified[20] = {(FreeValList){(Value *)0, 0},
				      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0},
                                      (FreeValList){(Value *)0, 0}};
__thread FreeValList freeReified[20] = {{(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0},
                                        {(Value *)0, 0}};
ReifiedVal *malloc_reified(int64_t implCount) {
  ReifiedVal *newReifiedVal;
  if (implCount > 19) {
    newReifiedVal = (ReifiedVal *)my_malloc(sizeof(ReifiedVal) + sizeof(Function *) * implCount);
  } else {
    newReifiedVal = (ReifiedVal *)freeReified[implCount].head;
    if (newReifiedVal == (ReifiedVal *)0) {
      newReifiedVal = (ReifiedVal *)removeFreeValue(&centralFreeReified[implCount]);
      if (newReifiedVal == (ReifiedVal *)0) {
	int rvSize = sizeof(ReifiedVal) + sizeof(Function *) * implCount;
	int rvCount = 100000;
	char *reifiedStructs = (char *)my_malloc(rvSize * rvCount);
	for (int i = 1; i < (rvCount - 1); i++) {
	  ((ReifiedVal *)&reifiedStructs[i * rvSize])->refs = refsError;
	  ((Value *)&reifiedStructs[i * rvSize])->next = (Value *)&reifiedStructs[(i + 1) * rvSize];
	}
	((ReifiedVal *)&reifiedStructs[(rvCount - 1) * rvSize])->refs = refsError;
	((Value *)&reifiedStructs[(rvCount - 1) * rvSize])->next = (Value *)0;
	freeReified[implCount].head = (Value *)&reifiedStructs[rvSize];
	moveToCentral(&freeReified[implCount], &centralFreeReified[implCount]);

	newReifiedVal = (ReifiedVal *)reifiedStructs;
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, (rvCount - 1), __ATOMIC_ACQ_REL);
#endif
      }
    } else {
      freeReified[implCount].head = freeReified[implCount].head->next;
    }
  }
  // incTypeMalloc(0, 1);
  newReifiedVal->refs = refsInit;
  newReifiedVal->hashVal = 0;
  newReifiedVal->implCount = implCount;
  return(newReifiedVal);
}

#define BMI_RECYCLE_COUNT 20
FreeValList centralFreeBMINodes[BMI_RECYCLE_COUNT] = {(FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0},
						      (FreeValList){(Value *)0, 0}};
__thread FreeValList freeBMINodes[BMI_RECYCLE_COUNT] = {{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0},
							{(Value *)0, 0}};
BitmapIndexedNode *malloc_bmiNode(int itemCount) {
  int nodeSize = sizeof(BitmapIndexedNode) + sizeof(Value *) * (itemCount * 2);
  BitmapIndexedNode *bmiNode;

  if (itemCount >= BMI_RECYCLE_COUNT) {
    bmiNode = (BitmapIndexedNode *)my_malloc(nodeSize);
  } else {
    bmiNode = (BitmapIndexedNode *)freeBMINodes[itemCount].head;
    if (bmiNode == (BitmapIndexedNode *)0) {
      bmiNode = (BitmapIndexedNode *)removeFreeValue(&centralFreeBMINodes[itemCount]);
      if (bmiNode == (BitmapIndexedNode *)0) {
	BitmapIndexedNode *bmiNodes;
	bmiNodes = (BitmapIndexedNode *)my_malloc(nodeSize * 10);
#ifdef CHECK_MEM_LEAK
	__atomic_fetch_add(&malloc_count, 9, __ATOMIC_ACQ_REL);
#endif
	bmiNode = (BitmapIndexedNode *)((void *)bmiNodes + nodeSize);
	for (int i = 1; i < 9; i++) {
	  bmiNode->refs = refsError;
	  ((Value *)bmiNode)->next = (Value *)((void *)bmiNode +  nodeSize);
	  bmiNode = (BitmapIndexedNode *)((Value *)bmiNode)->next;
	}
	bmiNode = (BitmapIndexedNode *)((void *)bmiNodes + (9 * nodeSize));
	bmiNode->refs = refsError;
	((Value *)bmiNode)->next = (Value *)0;
	freeBMINodes[itemCount].head = (Value *)((void *)bmiNodes + nodeSize);
	moveToCentral(&freeBMINodes[itemCount], &centralFreeBMINodes[itemCount]);

	bmiNode = bmiNodes;
      }
    } else {
      // fprintf(stderr, "%d from local node: %p\n", itemCount, bmiNode); 
      freeBMINodes[itemCount].head = freeBMINodes[itemCount].head->next;
    }
  }
  // incTypeMalloc(BitmapIndexedType, 1);
  bmiNode->type = BitmapIndexedType;
  bmiNode->refs = refsInit;
  bmiNode->hashVal = 0;
  bmiNode->bitmap = 0;
  memset(&bmiNode->array, 0, sizeof(Value *) * (itemCount * 2));
  return(bmiNode);
}

void freeBitmapNode(Value *v) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)v;
  int cnt = __builtin_popcount(node->bitmap);
  for (int i = 0; i < (2 * cnt); i++) {
    if (node->array[i] != (Value *)0) {
      dec_and_free(node->array[i], 1);
    }
  }
  if (cnt >= BMI_RECYCLE_COUNT) {
#ifdef CHECK_MEM_LEAK
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
    if (!cleaningUp)
      free(v);
  } else {
    v->next = freeBMINodes[cnt].head;
    freeBMINodes[cnt].head = v;
  }
}

HashCollisionNode *malloc_hashCollisionNode(int itemCount) {
  if (itemCount > 30000) {
    fprintf(stderr, "Catastrophic failure: Too many hash collisions\n");
    abort();
  }
  int nodeSize = sizeof(HashCollisionNode) + sizeof(Value *) * (itemCount * 2);
  HashCollisionNode *collisionNode;
  collisionNode = (HashCollisionNode *)my_malloc(nodeSize);
  // incTypeMalloc(HashCollisionNodeType, 1);
  memset(collisionNode, 0, nodeSize);
  collisionNode->type = HashCollisionNodeType;
  collisionNode->count = itemCount * 2;
  collisionNode->hashVal = 0;
  collisionNode->refs = refsInit;
  return(collisionNode);
}

void freeHashCollisionNode(Value *v) {
  HashCollisionNode *node = (HashCollisionNode *)v;
  for (int i = 0; i < node->count; i++) {
    if (node->array[i] != (Value *)0) {
      dec_and_free(node->array[i], 1);
    }
  }
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  if (!cleaningUp)
    free(v);
}

FreeValList centralFreeArrayNodes = (FreeValList){(Value *)0, 0};
__thread FreeValList freeArrayNodes = {(Value *)0, 0};
ArrayNode *malloc_arrayNode() {
  ArrayNode *arrayNode = (ArrayNode *)freeArrayNodes.head;
  if (arrayNode == (ArrayNode *)0) {
    arrayNode = (ArrayNode *)removeFreeValue(&centralFreeArrayNodes);
    if (arrayNode == (ArrayNode *)0) {
      ArrayNode *arrayNodes = (ArrayNode *)my_malloc(sizeof(ArrayNode) * 10);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 9, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 9; i++) {
	arrayNodes[i].refs = refsError;
        ((Value *)&arrayNodes[i])->next = (Value *)&arrayNodes[i + 1];
      }
      arrayNodes[9].refs = refsError;
      ((Value *)&arrayNodes[9])->next = (Value *)0;
      freeArrayNodes.head = (Value *)&arrayNodes[1];
      moveToCentral(&freeArrayNodes, &centralFreeArrayNodes);

      arrayNode = arrayNodes;
    }
  } else {
    freeArrayNodes.head = freeArrayNodes.head->next;
  }
  // incTypeMalloc(ArrayNodeType, 1);
  memset(arrayNode, 0, sizeof(ArrayNode));
  arrayNode->type = ArrayNodeType;
  arrayNode->hashVal = 0;
  arrayNode->refs = refsInit;
  return(arrayNode);
}

void freeArrayNode(Value *v) {
  ArrayNode *node = (ArrayNode *)v;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) {
    if (node->array[i] != (Value *)0) {
      dec_and_free(node->array[i], 1);
    }
  }
  v->next = freeArrayNodes.head;
  freeArrayNodes.head = v;
}

FreeValList centralFreePromises = (FreeValList){(Value *)0, 0};
__thread FreeValList freePromises = {(Value *)0, 0};
Promise *malloc_promise() {
  Promise *newPromise = (Promise *)freePromises.head;
  if (newPromise == (Promise *)0) {
    newPromise = (Promise *)removeFreeValue(&centralFreePromises);
    if (newPromise == (Promise *)0) {
      Promise *promises = (Promise *)my_malloc(sizeof(Promise) * 10);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 9, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 9; i++) {
	promises[i].refs = refsError;
        ((Value *)&promises[i])->next = (Value *)&promises[i + 1];
      }
      promises[9].refs = refsError;
      ((Value *)&promises[9])->next = (Value *)0;
      freePromises.head = (Value *)&promises[1];
      moveToCentral(&freePromises, &centralFreePromises);

      newPromise = promises;
    }
  } else {
    freePromises.head = freePromises.head->next;
  }
  // incTypeMalloc(PromiseType, 1);
  memset(newPromise, 0, sizeof(Promise));
  newPromise->type = PromiseType;
  newPromise->refs = refsInit;
  newPromise->result = (Value *)0;
  newPromise->actions = empty_list;
  pthread_cond_init(&newPromise->delivered, NULL);
  pthread_mutex_init(&newPromise->access, NULL);
  return(newPromise);
}

void freePromise(Value *v) {
  // TODO: make sure this is thread safe
  Promise *p = (Promise *)v;
  if (p->actions != (List *)0) {
    Value *acts = (Value *)p->actions;
    p->actions = (List *)0;
    dec_and_free(acts, 1);
  }
  if (p->result != (Value *)0) {
    Value *res = p->result;
    p->result = (Value *)0;
    dec_and_free(res, 1);
  }
  p->refs = refsError;
  v->next = freePromises.head;
  freePromises.head = v;
}

FreeValList centralFreeFutures = (FreeValList){(Value *)0, 0};
__thread FreeValList freeFutures = {(Value *)0, 0};
Future *malloc_future(int line) {
  Future *newFuture = (Future *)freeFutures.head;
  if (newFuture == (Future *)0) {
    newFuture = (Future *)removeFreeValue(&centralFreeFutures);
    if (newFuture == (Future *)0) {
      Future *futures = (Future *)my_malloc(sizeof(Future) * 10);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&malloc_count, 9, __ATOMIC_ACQ_REL);
#endif
      for (int i = 1; i < 9; i++) {
	futures[i].refs = refsError;
        ((Value *)&futures[i])->next = (Value *)&futures[i + 1];
      }
      futures[9].refs = refsError;
      ((Value *)&futures[9])->next = (Value *)0;
      freeFutures.head = (Value *)&futures[1];
      moveToCentral(&freeFutures, &centralFreeFutures);

      newFuture = futures;
    }
  } else {
    freeFutures.head = freeFutures.head->next;
  }
  // incTypeMalloc(FutureType, 1);
  memset(newFuture, 0, sizeof(Future));
  newFuture->type = FutureType;
  newFuture->refs = refsInit;
  newFuture->result = (Value *)0;
  newFuture->action = (Value *)0;
  newFuture->errorCallback = (Value *)0;
  pthread_cond_init(&newFuture->delivered, NULL);
  pthread_mutex_init(&newFuture->access, NULL);
  return(newFuture);
}

void freeFuture(Value *v) {
  List *actions = ((Future *)v)->actions;
  if (actions != (List *)0)
    dec_and_free((Value *)actions, 1);
  Value *action = ((Future *)v)->action;
  if (action != (Value *)0)
    dec_and_free(action, 1);
  Value *result = ((Future *)v)->result;
  if (result != (Value *)0)
    dec_and_free(result, 1);
  v->next = freeFutures.head;
  freeFutures.head = v;
}

void emptyAgent(Agent *agent) {
  pthread_mutex_lock (&agent->access);
  REFS_SIZE refs;
#ifdef SINGLE_THREADED
  refs = agent->output->refs;
#else
  __atomic_load(&agent->output->refs, &refs, __ATOMIC_RELAXED);
#endif
  if (refs != 1 &&
      refs != refsConstant &&
      refs != refsStatic) {
    fprintf(stderr, "failure in emptyAgent()\n");
    abort();
  }
  dec_and_free((Value *)agent->output, 1);

  List *l;
#ifdef SINGLE_THREADED
  l = agent->input;
  l->refs = refs;
#else
  __atomic_load(&agent->input, &l, __ATOMIC_RELAXED);
  __atomic_load(&l->refs, &refs, __ATOMIC_RELAXED);
#endif
  if (refs != 1 &&
      refs != refsConstant &&
      refs != refsStatic) {
    fprintf(stderr, "failure in emptyAgent()\n");
    abort();
  }
  dec_and_free((Value *)l, 1);
  pthread_mutex_unlock (&agent->access);
}

void freeAgent(Value *v) {
  // TODO: must add loop detection. It's too easy to screw up and cause a cycle
  Value *val = ((Agent *)v)->val;
  REFS_SIZE refs;
#ifdef SINGLE_THREADED
  refs = val->refs;
#else
  __atomic_load(&val->refs, &refs, __ATOMIC_RELAXED);
#endif
  emptyAgent((Agent *)v);
  if (val != (Value *)0) {
    dec_and_free(val, 1);
  }
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  if (!cleaningUp)
    free(v);
}

void freeOpaquePtr(Value *v) {
  // call the destructor with the pointer
  Opaque *opaque = (Opaque *)v;
  if (opaque->destruct != NULL)
    opaque->destruct(opaque->ptr);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  if (!cleaningUp)
    free(v);
}

typedef void (*freeValFn)(Value *);

freeValFn freeJmpTbl[CoreTypeCount] = {NULL,
				       &freeInteger,
				       &freeString,
				       &freeFnArity,
				       &freeFunction,
				       &freeSubString,
				       &freeList,
				       &freeMaybe,
				       &freeVector,
				       &freeVectorNode,
				       &freeSubString,
				       &freeBitmapNode,
				       &freeArrayNode,
				       &freeHashCollisionNode,
				       NULL,
				       &freePromise,
				       &freeFuture,
				       &freeAgent,
				       &freeOpaquePtr};

void breakCycle(Value *val, Value *cycVal);

void cyclePromise(Value *val, Value *cycVal) {
  if (val == cycVal) {
    decRefs(val, 1);
  } else {
    breakCycle(((Promise *)val)->result, cycVal);
    breakCycle((Value *)(((Promise *)val)->actions), cycVal);
  }
}

void cycleMaybe(Value *val, Value *cycVal) {
  breakCycle(((Maybe *)val)->value, cycVal);
}

void cycleList(Value *val, Value *cycVal) {
  List *lval = (List *)val;
  breakCycle((Value *)lval->head, cycVal);
  if (lval->tail != (List *)NULL) {
    cycleList((Value *)lval->tail, cycVal);
  }
}

void cycleFnArity(Value *val, Value *cycVal) {
  FnArity *arity = (FnArity *)val;
  breakCycle((Value *)arity->closures, cycVal);
}

void cycleFunction(Value *val, Value* cycVal) {
  Function *f = (Function *)val;
  for (int i = 0; i < f->arityCount; i++) {
    if (f->arities[i] != NULL) {
      cycleFnArity((Value *)f->arities[i], cycVal);
    }
  }
}

void cycleVector(Value *v, Value* cycVal) {
  Value *root = (Value *)((Vector *)v)->root;
  if (root != (Value *)NULL) {
    breakCycle((Value *)root, cycVal);
  }

  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((Vector *)v)->tail[i] != (Value *)NULL)
      breakCycle(((Vector *)v)->tail[i], cycVal);
  }
}

void cycleVectorNode(Value *v, Value* cycVal) {
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((VectorNode *)v)->array[i] != (Value *)NULL) {
      breakCycle(((VectorNode *)v)->array[i], cycVal);
    }
  }
}

void cycleBitmapNode(Value *v, Value* cycVal) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)v;
  int cnt = __builtin_popcount(node->bitmap);
  for (int i = 0; i < (2 * cnt); i++) {
    if (node->array[i] != (Value *)0) {
      breakCycle(node->array[i], cycVal);
    }
  }
}

void cycleArrayNode(Value *v, Value* cycVal) {
  ArrayNode *node = (ArrayNode *)v;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) {
    if (node->array[i] != (Value *)0) {
      breakCycle(node->array[i], cycVal);
    }
  }
}

void cycleHashCollisionNode(Value *v, Value* cycVal) {
  HashCollisionNode *node = (HashCollisionNode *)v;
  for (int i = 0; i < node->count; i++) {
    if (node->array[i] != (Value *)0) {
      breakCycle(node->array[i], cycVal);
    }
  }
}

typedef void (*cycleValFn)(Value *, Value *);

void noCycle(Value *val, Value *cycVal){
  // fprintf(stderr, "val: %p\n", val);
  // need no op
  int x = 1;
}

cycleValFn cycleJmpTbl[CoreTypeCount] = {NULL,
                                         &noCycle, // 1
				         &noCycle, // 2
				         &cycleFnArity, // 3
				         &cycleFunction, // 4
				         &noCycle, // 5
				         &cycleList, // 6
				         &cycleMaybe, // 7
				         &cycleVector, // 8
				         &cycleVectorNode, // 9
				         &noCycle, // 10
				         &cycleBitmapNode, // 11
				         &cycleArrayNode, // 12
				         &cycleHashCollisionNode, // 13
				         NULL,
				         &cyclePromise, // 15
					 // TODO: need to finish these
				         NULL, // &freeFuture,
				         NULL, // &freeAgent,
				         NULL};

void breakCycle(Value *val, Value *cycVal) {
  if (val != (Value *)NULL) {
    if (val->type < CoreTypeCount) {
      // fprintf(stderr, "breaking: %p %ld\n", val, val->type);
      // incTypeFree(v->type, 1);
      cycleJmpTbl[val->type](val, cycVal);
    } else {
      ReifiedVal *rv = (ReifiedVal *)val;
      for (int i = 0; i < rv->implCount; i++) {
	breakCycle(rv->impls[i], cycVal);
      }
    }
  }
}

void dec_and_free(Value *v, int deltaRefs) {
  if (v == (Value *)0 ||
      v->refs == refsStatic ||
      v->refs == refsConstant ||
      decRefs(v, deltaRefs) >= refsConstant)
    return;

  if (v->type < CoreTypeCount) {
    // incTypeFree(v->type, 1);
    freeJmpTbl[v->type](v);
  } else {
    ReifiedVal *rv = (ReifiedVal *)v;
    for (int i = 0; i < rv->implCount; i++) {
      dec_and_free(rv->impls[i], 1);
    }

    // incTypeFree(0, 1);
    if (rv->implCount < 20) {
      int64_t implCount = rv->implCount;
      v->next = freeReified[implCount].head;
      freeReified[implCount].head = v;
    } else {
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
      if (!cleaningUp)
	free(v);
    }
  }
#ifdef CHECK_MEM_LEAK
  // fprintf(stderr, "malloc_count: %ld free_count: %ld\r", malloc_count, free_count);
#endif
};

#ifndef FAST_INCS
Value *incRef(Value *v, int deltaRefs) {
  if ((Value *)v == 0) {
    fprintf(stderr, "bad incRef value: %p\n", v);
    abort();
  }
  if (deltaRefs < 0) {
    fprintf(stderr, "bad deltaRefs: %p\n", v);
    abort();
  } else if (deltaRefs < 1)
    return(v);

#ifdef SINGLE_THREADED
  if (v->refs == refsStatic || v->refs == refsConstant)
    return(v);

  if (v->refs < refsStatic) {
    fprintf(stderr, "failure in incRef: %d %p\n", v->refs, v);
    abort();
  }

  if (v->refs >= 0) {
    v->refs += deltaRefs;;
  }
#else
  REFS_SIZE refs;
  __atomic_load(&v->refs, &refs, __ATOMIC_RELAXED);

  REFS_SIZE newRefs;
  do {
    if (refs == refsStatic || refs == refsConstant)
      return(v);

    if (refs < refsStatic) {
      fprintf(stderr, "failure in incRef: %d %p\n", refs, v);
      abort();
    }

    newRefs = refs + deltaRefs;
  } while (!__atomic_compare_exchange(&v->refs, &refs, &newRefs, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
#endif
  return(v);
}
#else
Value *simpleIncRef(Value *v, int n) {
  v->refs += n;
  return(v);
}
#endif

void moveFreeToCentral() {
  moveToCentral(&freeLists, &centralFreeLists);
  for (int i = 0; i < 10; i++) {
    moveToCentral(&freeFunctions[i], &centralFreeFunctions[i]);
  }
  for (int i = 0; i < BMI_RECYCLE_COUNT; i++) {
    moveToCentral(&freeBMINodes[i], &centralFreeBMINodes[i]);
  }
  for (int i = 0; i < 20; i++) {
    moveToCentral(&freeReified[i], &centralFreeReified[i]);
  }
  moveToCentral(&freeStrings, &centralFreeStrings);
  moveToCentral(&freeArrayNodes, &centralFreeArrayNodes);
  moveToCentral(&freeSubStrings, &centralFreeSubStrings);
  moveToCentral(&freeIntegers, &centralFreeIntegers);
  moveToCentral(&freeMaybes, &centralFreeMaybes);
  moveToCentral(&freeVectors, &centralFreeVectors);
  moveToCentral(&freeVectorNodes, &centralFreeVectorNodes);
  moveToCentral(&freeFnArities, &centralFreeFnArities);
  moveToCentral(&freePromises, &centralFreePromises);
  moveToCentral(&freeFutures, &centralFreeFutures);
}


void freeGlobal(Value *x) {
  if (x == (Value*)0 ||
      x->refs == refsError ||
      x->refs == refsStatic ||
      x == (Value *)&emptyBMI)
    return;
  x->refs = refsInit;
  dec_and_free(x, 1);
  x->refs = refsStatic;
}

void emptyFreeList(FreeValList *freeLinkedList) {
  FreeValList listHead;
#ifdef SINGLE_THREADED
  listHead = *freeLinkedList;
#else
  __atomic_load((FreeValList *)freeLinkedList, (FreeValList *)&listHead, __ATOMIC_RELAXED);
#endif
  for(Value *item = listHead.head;
      item != (Value *)0;
      item =  item->next) {
#ifdef CHECK_MEM_LEAK
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  }
}

void freeAll() {
  moveFreeToCentral();

  for (int i = 0; i < 10; i++) {
    emptyFreeList(&centralFreeFunctions[i]);
  }
  for (int i = 0; i < 20; i++) {
    emptyFreeList(&centralFreeReified[i]);
  }
  for (int i = 0; i < BMI_RECYCLE_COUNT; i++) {
    emptyFreeList(&centralFreeBMINodes[i]);
  }
  emptyFreeList(&centralFreeFutures);
  emptyFreeList(&centralFreePromises);
  emptyFreeList(&centralFreeArrayNodes);
  emptyFreeList(&centralFreeSubStrings);
  emptyFreeList(&centralFreeFnArities);
  emptyFreeList(&centralFreeLists);
  emptyFreeList(&centralFreeMaybes);
  emptyFreeList(&centralFreeVectors);
  emptyFreeList(&centralFreeVectorNodes);
  emptyFreeList(&centralFreeStrings);
  emptyFreeList(&centralFreeIntegers);

//*
#ifdef SINGLE_THREADED
  fprintf(stderr, "\nmalloc count: %" PRId64 "  free count: %" PRId64 "  diff: %" PRId64 "\n",
          malloc_count, free_count, malloc_count - free_count);
#else
  int64_t mallocs;
  __atomic_load(&malloc_count, &mallocs, __ATOMIC_RELAXED);
  int64_t frees;
  __atomic_load(&free_count, &frees, __ATOMIC_RELAXED);
  fprintf(stderr, "malloc count: %" PRId64 "  free count: %" PRId64 "  diff: %" PRId64 "\n",
          mallocs, frees, mallocs - frees);

/*
  int64_t totalMallocs = 0;
  int64_t totalFrees = 0;
  for (int i = 0; i < 20; i++) {
    fprintf(stderr, "%d %ld %ld %ld\n", i, type_mallocs[i], type_frees[i], type_mallocs[i] - type_frees[i]);
    if (i != 19) {
      totalMallocs += type_mallocs[i];
      totalFrees += type_frees[i];
    }
  }
  totalMallocs = totalMallocs - type_mallocs[19];
  fprintf(stderr, "\ntotalMallocs %ld\n", totalMallocs);
  fprintf(stderr, "malloc diff %ld\n", mallocs - totalMallocs);
  fprintf(stderr, "\ntotalFrees %ld\n", totalFrees);
  fprintf(stderr, "free diff %ld\n", frees - totalFrees);
// */

#endif
// */
}

int64_t nakedSha1(Value *v1) {
  Integer *hashVal;
  int64_t hash;
  switch (v1->type) {
  case IntegerType:
    hash = integerSha1(v1);
    break;

  case StringBufferType:
  case SubStringType:
  case SymbolType:
    hash = strSha1(v1);
    break;

  case ListType:
  case MaybeType:
  case VectorType:
  case BitmapIndexedType:
  case ArrayNodeType:
  case HashCollisionNodeType:
    if (((HashedValue *)v1)->hashVal != 0) {
      hash = ((HashedValue *)v1)->hashVal;
      dec_and_free(v1, 1);
    } else {
      hashVal = (Integer *)sha1((FnArity *)0, v1);
      hash = hashVal->numVal;
      ((HashedValue *)v1)->hashVal = hash;
      dec_and_free((Value *)hashVal, 1);
    }
    break;
    
  default:
    if (v1->type > CoreTypeCount) {
      if (((HashedValue *)v1)->hashVal != 0) {
	hash = ((HashedValue *)v1)->hashVal;
	dec_and_free(v1, 1);
      } else {
	hashVal = (Integer *)sha1((FnArity *)0, v1);
	hash = hashVal->numVal;
	((HashedValue *)v1)->hashVal = hash;
	dec_and_free((Value *)hashVal, 1);
      }
    } else {
      hashVal = (Integer *)sha1((FnArity *)0, v1);
      hash = hashVal->numVal;
      dec_and_free((Value *)hashVal, 1);
    }
    break;
  }
  return(hash);
}

List *reverseList(List *input) {
  List *output = empty_list;
  Value *item;
  List *l = input;
  while(l != (List *)0 && l->head != (Value *)0) {
    item = l->head;
    incRef(item, 1);
    output = listCons(item, output);
    l = l->tail;
  }
  dec_and_free((Value *)input, 1);
  return(output);
}

FuturesQueueStruct futuresQueue;

void scheduleFuture(Future *fut) {
  List *newList = malloc_list();
  newList->head = (Value *)fut;
  List *input;
#ifdef SINGLE_THREADED
  input = futuresQueue.input;
  newList->len = input->len + 1;
  newList->tail = input;
  futuresQueue.input = newList;
#else
  __atomic_load(&futuresQueue.input, &input, __ATOMIC_RELAXED);
  do {
    newList->len = input->len + 1;
    newList->tail = input;
  } while (!__atomic_compare_exchange(&futuresQueue.input, &input, &newList, 1,
				      __ATOMIC_RELAXED, __ATOMIC_RELAXED));
#endif

  // It is unusual to not hold the mutex when signalling the condition. But
  // in this case it's ok. All the threads waiting on the condition are of
  // equal priority, so it doesn't matter which one gets the next item in
  // the queue. See this explanation for the reasoning behind this:
  // https://groups.google.com/forum/?hl=ky#!msg/comp.programming.threads/wEUgPq541v8/ZByyyS8acqMJ
  pthread_cond_signal(&futuresQueue.notEmpty);
}

void waitForWorkers() {
#ifdef SINGLE_THREADED
  // no need to wait
#else
  pthread_cond_broadcast(&futuresQueue.notEmpty);
  for (int8_t i = 0; i < NUM_WORKERS; i++) {
    pthread_join(workers[i], NULL);
  }
  pthread_mutex_lock (&futuresQueue.mutex);
  List *l;
  __atomic_load(&futuresQueue.output, &l, __ATOMIC_RELAXED);
  dec_and_free((Value *)l, 1);

  __atomic_load(&futuresQueue.input, &l, __ATOMIC_RELAXED);
  dec_and_free((Value *)l, 1);
  pthread_mutex_unlock (&futuresQueue.mutex);

#ifdef WAIT_FOR_LINGERING
  int done = 0;
  do {
    pthread_mutex_lock (&lingeringAccess);
    List *lingering = (List *)vals((FnArity *)0, lingeringThreads);
    lingeringThreads = (Value *)&emptyBMI;
    pthread_mutex_unlock (&lingeringAccess);

    l = lingering;
    for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
      pthread_t threadId = (pthread_t)((Integer *)x)->numVal;
      pthread_join(threadId, NULL);
    }
    if (lingering->len == 0)
      done = 1;
    dec_and_free((Value *)lingering, 1);
  } while(!done);
#else
  fprintf(stderr, "\n\n*** not waiting on lingering threads\n\n");
#endif
#endif
}

Value *shutDown_impl(FnArity *arity) {
  Value *item;
#ifdef CHECK_MEM_LEAK
  moveFreeToCentral();
#endif
  pthread_exit(NULL);
  return(nothing);
 };

FnArity shutDown_arity = {FnArityType, -2, 0, (Vector *)0, (Value *)0, 0, shutDown_impl};
Function shutDownFn = {FunctionType, -2, "shutdown-workers", 1, {&shutDown_arity}};
Future shutDown = {FutureType, -2, (Value *)&shutDownFn, (Value *)0, (List *)0, (Value *)0, 0};

void stopWorkers() {
  for (int32_t i = 0; i < NUM_WORKERS; i++)
    scheduleFuture(&shutDown);
}

Value *readFuturesQueue() {
  List *output;
#ifdef SINGLE_THREADED
  // do nothing with futures in a single threaded system
  return((Value *)0);
#else
  __atomic_load(&futuresQueue.output, &output, __ATOMIC_RELAXED);
  while (output != (List *)0 && output->len != 0 &&
         !__atomic_compare_exchange(&futuresQueue.output,
				    &output,
				    &output->tail,
				    1, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
    ;

  if (output != (List *)0 && output->len != 0) {
    Value *item = output->head;
    output->head = (Value *)0;
    output->tail = (List *)0;
    REFS_SIZE refs;
    __atomic_load(&output->refs, &refs, __ATOMIC_RELAXED);
    if (refs != 1) {
      fprintf(stderr, "error reading futures queue 1 %d\n", refs);
      abort();
    }
    dec_and_free((Value *)output, 1);
    return(item);
  } else {
    pthread_mutex_lock (&futuresQueue.mutex);
    __atomic_load(&futuresQueue.output, &output, __ATOMIC_RELAXED);
    while (output != (List *)0 && output->len != 0 &&
           !__atomic_compare_exchange(&futuresQueue.output,
				      &output,
				      &output->tail,
				      1, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
      ;
    if (output != (List *)0 && output->len != 0) {
      Value *item = output->head;
      output->head = (Value *)0;
      output->tail = (List *)0;
      REFS_SIZE refs;
      __atomic_load(&output->refs, &refs, __ATOMIC_RELAXED);
      if (refs != 1) {
        fprintf(stderr, "error reading futures queue 2\n");
        abort();
      }
      dec_and_free((Value *)output, 1);
      pthread_mutex_unlock (&futuresQueue.mutex);
      return(item);
    } else {
      List *input;
      __atomic_exchange((List **)&futuresQueue.input,
			(List **)&empty_list,
			(List **)&input,
			__ATOMIC_RELAXED);

      if (input == (List *)0 || input->len == 0) {
        int32_t numRunning = __atomic_fetch_sub(&runningWorkers, 1, __ATOMIC_ACQ_REL);
        if (numRunning <= 1 && mainThreadDone) {
          stopWorkers();
        } else {
#ifdef CHECK_MEM_LEAK
	  moveFreeToCentral();
#endif
          pthread_cond_wait(&futuresQueue.notEmpty, &futuresQueue.mutex);
          __atomic_fetch_add(&runningWorkers, 1, __ATOMIC_ACQ_REL);
        }
        pthread_mutex_unlock (&futuresQueue.mutex);
        return(readFuturesQueue());
      } else {
        output = reverseList(input);

        __atomic_store(&futuresQueue.output, &output->tail, __ATOMIC_RELAXED);
        pthread_cond_signal(&futuresQueue.notEmpty);
        pthread_mutex_unlock (&futuresQueue.mutex);

        Value *item = output->head;
        output->head = (Value *)0;
        output->tail = (List *)0;
        REFS_SIZE refs;
        __atomic_load(&output->refs, &refs, __ATOMIC_RELAXED);
        if (refs != 1) {
          fprintf(stderr, "error reading futures queue 3\n");
          abort();
        }
        dec_and_free((Value *)output, 1);
        return(item);
      }
    }
  }
#endif
}

Value *deliverFuture(Value *fut, Value *val) {
  Future *future = (Future *)fut;
  if (future->result == (Value *)0) {
    pthread_mutex_lock (&future->access);
    future->result = val;
    List *l = future->actions;
    List *head = l;
    future->actions = (List *)0;
    pthread_cond_broadcast(&future->delivered);
    pthread_mutex_unlock (&future->access);

    // perform actions
    if (l != (List *)0 && l->len != 0) {
      for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
	incRef(x, 1);
	incRef(val, 1);
	Value *trash = dynamicCall1Arg(x, val);
	dec_and_free(trash, 1);
      }
      dec_and_free((Value *)head, 1);
    }
  } else {
    dec_and_free(val, 1);
  }
  return fut;
}

__thread int64_t workerIndex;
void *futuresThread(void *input) {
  workerIndex = (int64_t)input;
  Future *future;
  Value *result;
  if (workerIndex >= 0)
    future = (Future *)readFuturesQueue();
  while(workerIndex >= 0 && future != (Future *)0) {
    Value *f = future->action;
    if(f->type != FunctionType) {
      result = invoke0Args((FnArity *)0, incRef(f, 1));
    } else {
      FnArity *arity = findFnArity(f, 0);
      if(arity != (FnArity *)0 && !arity->variadic) {
	FnType0 *fn = (FnType0 *)arity->fn;
	result = fn(arity);
      } else if(arity != (FnArity *)0 && arity->variadic) {
	FnType1 *fn = (FnType1 *)arity->fn;
	result = fn(arity, (Value *)empty_list);
      } else {
	fprintf(stderr, "\n*** no arity found for '%s'.\n", ((Function *)f)->name);
	abort();
      }
    }
    deliverFuture((Value *)future, result);
    dec_and_free((Value *)future, 1);
    if (workerIndex >= 0) {
      future = (Future *)readFuturesQueue();
    }
  }
#ifdef SINGLE_THREADED
  runningWorkers--;
#else
  __atomic_fetch_sub(&runningWorkers, 1, __ATOMIC_ACQ_REL);
#endif
  Value *threadHandle = (Value *)integerValue((int64_t)pthread_self());

  pthread_mutex_lock (&lingeringAccess);
  lingeringThreads = baseDissoc(lingeringThreads, incRef(threadHandle, 1),
				nakedSha1(threadHandle), 0);
  pthread_mutex_unlock (&lingeringAccess);

#ifdef CHECK_MEM_LEAK
  moveFreeToCentral();
#endif
  return(NULL);
}

int32_t numWorkers = NUM_WORKERS;
void startWorkers() {
#ifdef SINGLE_THREADED
  runningWorkers = numWorkers;
#else
  __atomic_store(&runningWorkers, &numWorkers, __ATOMIC_RELAXED);
#endif
  for (int64_t i = 0; i < NUM_WORKERS; i++)
    pthread_create(&workers[i], NULL, futuresThread, (void *)i);
}

void replaceWorker() {
  pthread_t me = pthread_self();
  for (int64_t i = 0; i < NUM_WORKERS; i++) {
    if (pthread_equal(workers[i], me)) {
      pthread_create(&workers[workerIndex], NULL, futuresThread, (void *)i);
      workerIndex = -1;
    }
  }
  Value *threadHandle = (Value *)integerValue((int64_t)me);
  pthread_mutex_lock (&lingeringAccess);
  lingeringThreads = copyAssoc(lingeringThreads, incRef((Value *)threadHandle, 1),
			       incRef((Value *)threadHandle, 1),
			       nakedSha1(threadHandle), 0);
  pthread_mutex_unlock (&lingeringAccess);
}

char *extractStr(Value *v) {
  // Should only be used to print an error meessage when calling 'abort'
  // Leaks a String value
  if (v->type == StringBufferType)
    return(((String *)v)->buffer);
  else if (v->type == SubStringType) {
    String *newStr = (String *)my_malloc(sizeof(String) + ((String *)v)->len + 5);
    // incTypeMalloc(StringBufferType, 1);
    newStr->hashVal = 0;
    snprintf(newStr->buffer, ((String *)v)->len + 1, "%s", ((SubString *)v)->buffer);
    return(newStr->buffer);
  } else {
    fprintf(stderr, "\ninvalid type for 'extractStr'\n");
    abort();
  }
}

FnArity *findFnArity(Value *fnVal, int64_t argCount) {
  Function *fn = (Function *)fnVal;
  int arityIndex = 0;
  FnArity *arity = (FnArity *)fn->arities[arityIndex];
  FnArity *variadic = (FnArity *)0;
  while(arityIndex < fn->arityCount) {
    arity = (FnArity *)fn->arities[arityIndex];
    if (arity->variadic) {
      variadic = arity;
      arityIndex++;
    } else if (arity->count != argCount) {
      arityIndex++;
    } else
      return(arity);
  }
  return(variadic);
};

int8_t isNothing(Value *v, char *fileName, int lineNumber) {
  return(v->type == MaybeType && ((Maybe *)v)->value == (Value *)0);
}

Value *maybe(Vector *arity, Value *arg0, Value *arg1) {
  Maybe *mVal = malloc_maybe();
  mVal->value = arg1;
  return((Value *)mVal);
}

Value *prSTAR(Value *str) {
  int bytes;
  if (str->type == StringBufferType) {
    bytes = fprintf(outstream, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    bytes = fprintf(outstream, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  }
  dec_and_free(str, 1);
  return(integerValue(bytes));
}

Value *defaultPrErrSTAR(Value *str) {
  int bytes;
  if (str->type == StringBufferType) {
    bytes = fprintf(stderr, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    bytes = fprintf(stderr, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  }
  dec_and_free(str, 1);
  return(integerValue(bytes));
}

Value *add_ints(Value *arg0, Value *arg1) {
  Value *numVal = integerValue(((Integer *)arg0)->numVal + ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(numVal);
}

Value *integerValue(int64_t n) {
  Integer *numVal = malloc_integer();
  numVal->numVal = n;
  return((Value *)numVal);
};

Value *integer_str(Value *arg0) {
  String *numStr = malloc_string(50);
  snprintf(numStr->buffer, 40, "%" PRId64 "", ((Integer *)arg0)->numVal);
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
    return(maybe((Vector *)0, (Value *)0, arg0));
  }
}

Value *isInstance(Value *arg0, Value *arg1) {
  TYPE_SIZE typeNum = ((Integer *)arg0)->numVal;
  if (typeNum == arg1->type) {
     dec_and_free(arg1, 1);
     return(maybe((Vector *)0, (Value *)0, arg0));
  } else if (StringBufferType == typeNum && SubStringType == arg1->type) {
     dec_and_free(arg1, 1);
     return(maybe((Vector *)0, (Value *)0, arg0));
  // } else if (HashMapType == typeNum && (BitmapIndexedType == arg1->type ||
                                        // ArrayNodeType == arg1->type ||
                                        // HashCollisionNodeType == arg1->type)) {
     // dec_and_free(arg1, 1);
     // return(maybe((Vector *)0, (Value *)0, arg0));
  } else {
     dec_and_free(arg0, 1);
     dec_and_free(arg1, 1);
     return(nothing);
  }
}

List *listCons(Value *x, List *l) {
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

Vector *vectConj(Vector *vect, Value *val) {
  if (vect->refs == 1) {
    return(mutateVectConj((Vector *)incRef((Value *)vect, 1), val));
    // if there's room in the tail
  } else if (vect->count - vect->tailOffset < VECTOR_ARRAY_LEN) {
    // make a new vector and copy info over
    Vector *newVect = newVector(vect->tail, VECTOR_ARRAY_LEN);
    newVect->shift = vect->shift;
    newVect->count = vect->count + 1;
    if (newVect->count < VECTOR_ARRAY_LEN) {
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
  if (vect->refs <= refsConstant) {
    Vector *result = vectConj(vect, val);
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

VectorNode *copyVectStore(int level, VectorNode *node, unsigned index, Value *val) {
  if (level == 0) {
    int arrayIndex = index & 0x1f;
    VectorNode *newNode = newVectorNode(node->array, arrayIndex);
    newNode->array[arrayIndex] = val;
    return(newNode);
  } else {
    int arrayIndex = (index >> level) & 0x1f;
    VectorNode *newNode = newVectorNode(node->array, arrayIndex);
    newNode->array[arrayIndex] = (Value *)copyVectStore(level - 5, (VectorNode *)node->array[arrayIndex],
							index, val);
    return(newNode);
  }
}

Value *vectStore(Vector *vect, unsigned index, Value *val) {
  // TODO: check the refs count and mutate if equal 1
  // but only if all nodes 'above' this one are mutate-able
  // and if you do mutate this vect, clear the cached hash value (once that's implemented)
  if (index < vect->count) {
    if (index >= vect->tailOffset) {
      unsigned newIndex = index & 0x1f;
      Vector *ret = newVector(vect->tail, newIndex);
      ret->tail[newIndex] = val;
      ret->count = vect->count;
      ret->tailOffset = vect->tailOffset;
      ret->shift = vect->shift;
      ret->root = vect->root;
      if (ret->root != (VectorNode *)0) {
        incRef((Value *)ret->root, 1);
      }
      Value *mval = maybe((Vector *)0, (Value *)0, (Value *)ret);
      return(mval);
    } else {
      Vector *ret = newVector(vect->tail, VECTOR_ARRAY_LEN);
      ret->count = vect->count;
      ret->tailOffset = vect->tailOffset;
      ret->shift = vect->shift;
      ret->root = copyVectStore(vect->shift, vect->root, index, val);
      Value *mval = maybe((Vector *)0, (Value *)0, (Value *)ret);
      return(mval);
    }
  } else if (index == vect->count) {
    Value *ret = (Value *)vectConj(vect, val);
    Value *mval = maybe((Vector *)0, (Value *)0, (Value *)ret);
    return(mval);
  } else {
    dec_and_free(val, 1);
    return(nothing);
  }
}

Value *fastVectStore(Vector *vect, unsigned index, Value *val) {
  if (index < vect->count &&
      index >= vect->tailOffset &&
      vect->refs == 1) {
    unsigned newIndex = index & 0x1f;
    dec_and_free(vect->tail[newIndex], 1);

    vect->tail[newIndex] = val;
    return((Value *)vect);
  } else {
    Value *result = vectStore(vect, index, val);
    if (isNothing(result, "", 0)) {
      fprintf(stderr, "*** Improper use of fastVectStore\n");
      abort();
    } else {
      Value *inner = ((Maybe *)result)->value;
      incRef(inner, 1);
      dec_and_free(result, 1);
      dec_and_free((Value *)vect, 1);
      return(inner);
    }
  }
}

Value *updateField(Value *rval, Value *field, int64_t idx) {
  ReifiedVal *template = (ReifiedVal *)rval;
  if (idx >= template->implCount) {
    fprintf(stderr, "Field index for type '%s' out of bounds: %" PRId64 ". Max: %" PRId64 "\n",
	    extractStr(type_name((FnArity *)0, rval)), idx, template->implCount);
    abort();
  }
  if (rval->refs == 1) {
    dec_and_free(template->impls[idx], 1);
    template->impls[idx] = field;

    return(rval);
  } else {
    ReifiedVal *rv = malloc_reified(template->implCount);
    int rvSize = sizeof(ReifiedVal) + sizeof(Function *) * template->implCount;
    memcpy(rv, template, rvSize);
#ifdef SINGLE_THREADED
    rv->refs = refsInit;
#else
    __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
#endif
    for (int i = 0; i < template->implCount; i++) {
      if (i != idx) {
        incRef(template->impls[i], 1);
      }
    }
    rv->impls[idx] = field;
    dec_and_free(rval, 1);
    return((Value *)rv);
  }
}

Value *vectGet(Vector *vect, unsigned index) {
  // this fn does not dec_and_free vect on purpose
  // it lets calling functions do that.
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

Value *vectorReverse(Value *arg0) {
  Vector *v = (Vector *)arg0;
  int i;
  Vector *newVect = empty_vect;
  for (i = v->count - 1; i >= 0; i--) {
    Value *val = vectGet(v, i);
    incRef(val, 1);
    newVect = mutateVectConj(newVect, val);
  }
  dec_and_free(arg0, 1);
  return((Value *)newVect);
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

Value *strEQ(Value *arg0, Value *arg1) {
  char *s1, *s2;
  long int len;

  if (arg0->type == StringBufferType &&
      arg1->type == StringBufferType &&
      ((String *)arg0)->len == ((String *)arg1)->len) {
    s1 = ((String *)arg0)->buffer;
    len = ((String *)arg0)->len;
    s2 = ((String *)arg1)->buffer;
  } else if (arg0->type == SubStringType &&
             arg1->type == SubStringType &&
             ((SubString *)arg0)->len == ((SubString *)arg1)->len) {
    s1 = ((SubString *)arg0)->buffer;
    len = ((SubString *)arg0)->len;
    s2 = ((SubString *)arg1)->buffer;
  } else if (arg0->type == StringBufferType &&
             arg1->type == SubStringType &&
             ((String *)arg0)->len == ((SubString *)arg1)->len) {
    s1 = ((String *)arg0)->buffer;
    len = ((String *)arg0)->len;
    s2 = ((SubString *)arg1)->buffer;
  } else if (arg0->type == SubStringType &&
             arg1->type == StringBufferType &&
             ((SubString *)arg0)->len == ((String *)arg1)->len) {
    s1 = ((SubString *)arg0)->buffer;
    len = ((SubString *)arg0)->len;
    s2 = ((String *)arg1)->buffer;
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }

  if (strncmp(s1, s2, len) == 0) {
    dec_and_free(arg1, 1);
    return(maybe((Vector *)0, (Value *)0, arg0));
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }
}

Value *strLT(Value *arg0, Value *arg1) {
  char *s1, *s2;
  long int len, s1Len, s2Len;

  if (arg0->type == StringBufferType &&
      arg1->type == StringBufferType) {
    s1 = ((String *)arg0)->buffer;
    s1Len = ((String *)arg0)->len;
    s2 = ((String *)arg1)->buffer;
    s2Len = ((String *)arg1)->len;
    if (s1Len < s2Len)
      len = s1Len;
    else
      len = s2Len;
  } else if (arg0->type == SubStringType &&
             arg1->type == SubStringType) {
    s1 = ((SubString *)arg0)->buffer;
    s1Len = ((SubString *)arg0)->len;
    s2 = ((SubString *)arg1)->buffer;
    s2Len = ((SubString *)arg1)->len;
    if (s1Len < s2Len)
      len = s1Len;
    else
      len = s2Len;
  } else if (arg0->type == StringBufferType &&
             arg1->type == SubStringType) {
    s1 = ((String *)arg0)->buffer;
    s1Len = ((String *)arg0)->len;
    s2 = ((SubString *)arg1)->buffer;
    s2Len = ((SubString *)arg1)->len;
    if (s1Len < s2Len)
      len = s1Len;
    else
      len = s2Len;
  } else if (arg0->type == SubStringType &&
             arg1->type == StringBufferType) {
    s1 = ((SubString *)arg0)->buffer;
    s1Len = ((SubString *)arg0)->len;
    s2 = ((String *)arg1)->buffer;
    s2Len = ((String *)arg1)->len;
    if (s1Len < s2Len)
      len = s1Len;
    else
      len = s2Len;
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }

  int cmp = strncmp(s1, s2, len);
  if (cmp < 0 || (cmp == 0 && s1Len < s2Len)) {
    dec_and_free(arg1, 1);
    return(maybe((Vector *)0, (Value *)0, arg0));
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }
}

Value *strCount(Value *arg0) {
   Value *numVal;
   if (arg0->type == StringBufferType)
     numVal = integerValue(((String *)arg0)->len);
   else
     numVal = integerValue(((SubString *)arg0)->len);
   dec_and_free(arg0, 1);
   return(numVal);
}

Value *strList(Value *arg0) {
  List *result = empty_list;
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    for (int64_t i = s->len - 1; i >= 0; i--) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = listCons((Value *)subStr, result);
    }
    incRef(arg0, s->len);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    for (int64_t i = s->len - 1; i >= 0; i--) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = listCons((Value *)subStr, result);
    }
    incRef(arg0, s->len);
  }
  dec_and_free(arg0, 1);
  return((Value *)result);
}

Value *integer_LT(Value *arg0, Value *arg1) {
 if (((Integer *)arg0)->numVal < ((Integer *)arg1)->numVal) {
     dec_and_free(arg1, 1);
     return(maybe((Vector *)0, (Value *)0, arg0));
  } else {
     dec_and_free(arg0, 1);
     dec_and_free(arg1, 1);
     return(nothing);
  }
}

Value *checkInstance(TYPE_SIZE typeNum, Value *arg1) {
  if (typeNum == arg1->type) {
    return(maybe((Vector *)0, (Value *)0, arg1));
  } else if (StringBufferType == typeNum && SubStringType == arg1->type) {
    return(maybe((Vector *)0, (Value *)0, arg1));
  } else if (HashMapType == typeNum && (BitmapIndexedType == arg1->type ||
                                        ArrayNodeType == arg1->type ||
                                        HashCollisionNodeType == arg1->type)) {
    return(maybe((Vector *)0, (Value *)0, arg1));
  } else {
    dec_and_free(arg1, 1);
    return(nothing);
  }
}

Value *listMap(Value *arg0, Value *f) {
  // List map
  List *l = (List *)arg0;
  if (l->len == 0) {
    dec_and_free(arg0, 1);
    dec_and_free(f, 1);
    return((Value *)empty_list);
  } else {
    List *head = empty_list;
    List *tail = empty_list;
    int mutate = 0;
    FnArity *arity2;
    if(f->type == FunctionType) {
      arity2 = findFnArity(f, 1);
      if(arity2 == (FnArity *)0) {
        fprintf(stderr, "\n*** no arity found for '%s'.\n", ((Function *)f)->name);
        abort();
      }
    }
    REFS_SIZE refs;
    __atomic_load(&arg0->refs, &refs, __ATOMIC_RELAXED);
    if (refs == 1) {
      mutate = 1;
      head = l;
      head->len = 0;

      tail = l;
    }
    for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
      Value *y;
      if (mutate && l->refs > 1) {
	dec_and_free((Value *)l, 1);
	mutate = 0;
      }

      if (!mutate)
	incRef(x, 1);
      if(f->type != FunctionType) {
        incRef(f, 1);
        y = invoke1Arg((FnArity *)0, f, x);
      } else if(arity2->variadic) {
        FnType1 *fn4 = (FnType1 *)arity2->fn;
        List *varArgs3 = (List *)listCons(x, empty_list);
        y = fn4(arity2, (Value *)varArgs3);
      } else {
        FnType1 *fn4 = (FnType1 *)arity2->fn;
        y = fn4(arity2, x);
      }

      // 'y' is the value for the new list
      if (mutate) {
	l->head = y;
	tail = l;
        head->len++;
      } else if (head == empty_list) {
        // if we haven't started the new list yet
        head = malloc_list();
        head->len = 1;
        head->head = y;
        head->tail = empty_list;
        tail = head;
      } else {
        // otherwise, append to tail of list
        List *new_tail = malloc_list();
        new_tail->len = 1;
        new_tail->head = y;
        new_tail->tail = empty_list;
        tail->tail = new_tail;
        tail = new_tail;
        head->len++;
      }
    }
    if (refs != 1)
      dec_and_free(arg0, 1);
    dec_and_free(f, 1);
    return((Value *)head);
  }
}

Value *listConcat(Value *arg0) {
  // TODO: check refs count for each list and stitch them together
  List *ls = (List *)arg0;

  if (ls->len == 0) {
    dec_and_free(arg0, 1);
    return((Value *)empty_list);
  }
  else if (ls->len == 1) {
    Value *h = ls->head;
    incRef(h, 1);
    dec_and_free((Value *)ls, 1);
    if (h != (Value *)0 && h->type == VectorType) {
       return(vectSeq((Vector *)h, 0));
    } else if (h != (Value *)0 && h->type != ListType) {
      // TODO: this test should be redundant when type checker is finished. Verify
      fprintf(stderr, "*** Could not concatenate non-list value with list\n");
      abort();
    } else {
      return(h);
    }
  } else {
    List *head = empty_list;
    List *tail = empty_list;
    for (; ls != (List *)0; ls = ls->tail) {
      List *l = (List *)ls->head;
      List *newL;
      int discard = 0;
      if (l != (List *)0 && l->type == VectorType) {
        l = (List *)vectSeq((Vector *)incRef((Value *)l, 1), 0);
        discard = 1;
      } else if (l != (List *)0 && l->type != ListType) {
	// TODO: this test should be redundant when type checker is finished. Verify
	fprintf(stderr, "*** Could not concatenate non-list value with list\n");
	abort();
      }
      Value *x;
      for(; l != (List *)0 && l->head != (Value *)0; l = newL) {
        x = l->head;
        if (head == empty_list) {
          // if we haven't started the new list yet
          head = malloc_list();
          head->len = 1;
          head->head = x;
          incRef(x, 1);
          head->tail = empty_list;
          tail = head;
        } else {
          // otherwise, append to tail of list
          List *new_tail = malloc_list();
          new_tail->len = 1;
          new_tail->head = x;
          incRef(x, 1);
          new_tail->tail = empty_list;
          tail->tail = new_tail;
          tail = new_tail;
          head->len++;
        }
        newL = l->tail;
        if(discard) {
          l->tail = (List *)0;
          dec_and_free((Value *)l, 1);
        }
      }
    }
    dec_and_free(arg0, 1);
    return((Value *)head);
  }
}

Value *car(Value *arg0) {
  List *lst = (List *)arg0;
  if (lst->len == 0) {
    return(nothing);
  } else {
    Value *h = lst->head;
    incRef(h, 1);
    dec_and_free(arg0, 1);
    return(maybe((Vector *)0, (Value *)0, h));
  }
}

Value *cdr(Value *arg0) {
  List *lst = (List *)arg0;
  if (lst->len == 0) {
    dec_and_free(arg0, 1);
    return((Value *)empty_list);
  } else {
    List *tail = ((List *)arg0)->tail;
    tail->len = lst->len - 1;
    incRef((Value *)tail, 1);
    dec_and_free(arg0, 1);
    return((Value *)tail);
  }
}

Value *integerLT(Value *arg0, Value *arg1) {
  if (((Integer *)arg0)->numVal < ((Integer *)arg1)->numVal) {
    dec_and_free(arg1, 1);
    return(maybe((Vector *)0, (Value *)0, arg0));
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }
}

// SHA1 implementation courtesy of: Steve Reid <sreid@sea-to-sky.net>
// and others.
// from http://waterjuice.org/c-source-code-for-sha1/

#define SHA1_HASH_SIZE           ( 64 / 8 )

typedef struct
{
 uint8_t      bytes [SHA1_HASH_SIZE];
 } SHA1_HASH;

typedef union
{
 uint8_t     c [64];
 uint32_t    l [16];
 } CHAR64LONG16;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) |(rol(block->l[i],8)&0x00FF00FF))
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^block->l[(i+2)&15]^block->l[i&15],1))

#define R0(v,w,x,y,z,i)  z += ((w&(x^y))^y)     + blk0(i)+ 0x5A827999 + rol(v,5); w=rol(w,30);
#define R1(v,w,x,y,z,i)  z += ((w&(x^y))^y)     + blk(i) + 0x5A827999 + rol(v,5); w=rol(w,30);
#define R2(v,w,x,y,z,i)  z += (w^x^y)           + blk(i) + 0x6ED9EBA1 + rol(v,5); w=rol(w,30);
#define R3(v,w,x,y,z,i)  z += (((w|x)&y)|(w&x)) + blk(i) + 0x8F1BBCDC + rol(v,5); w=rol(w,30);
#define R4(v,w,x,y,z,i)  z += (w^x^y)           + blk(i) + 0xCA62C1D6 + rol(v,5); w=rol(w,30);

static void TransformFunction(uint32_t state[5], const uint8_t buffer[64]) {
   uint32_t            a;
   uint32_t            b;
   uint32_t            c;
   uint32_t            d;
   uint32_t            e;
   uint8_t             workspace[64];
   CHAR64LONG16*       block = (CHAR64LONG16*) workspace;

   memcpy( block, buffer, 64 );

   // Copy context->state[] to working vars
   a = state[0];
   b = state[1];
   c = state[2];
   d = state[3];
   e = state[4];

   // 4 rounds of 20 operations each. Loop unrolled.
   R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
   R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
   R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
   R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
   R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
   R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
   R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
   R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
   R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
   R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
   R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
   R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
   R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
   R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
   R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
   R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
   R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
   R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
   R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
   R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

   // Add the working vars back into context.state[]
   state[0] += a;
   state[1] += b;
   state[2] += c;
   state[3] += d;
   state[4] += e;
   }

void Sha1Initialise (Sha1Context* Context) {
   // SHA1 initialization constants
   Context->State[0] = 0x67452301;
   Context->State[1] = 0xEFCDAB89;
   Context->State[2] = 0x98BADCFE;
   Context->State[3] = 0x10325476;
   Context->State[4] = 0xC3D2E1F0;
   Context->Count[0] = 0;
   Context->Count[1] = 0;
   }

void Sha1Update (Sha1Context* Context, void* Buffer, int64_t BufferSize) {
   uint32_t    i;
   uint32_t    j;

   j = (Context->Count[0] >> 3) & 63;
   if( (Context->Count[0] += BufferSize << 3) < (BufferSize << 3) )
   {
      Context->Count[1]++;
   }

   Context->Count[1] += (BufferSize >> 29);
   if( (j + BufferSize) > 63 )
   {
      i = 64 - j;
      memcpy( &Context->Buffer[j], Buffer, i );
      TransformFunction(Context->State, Context->Buffer);
      for( ; i + 63 < BufferSize; i += 64 )
      {
         TransformFunction(Context->State, (uint8_t*)Buffer + i);
      }
      j = 0;
   }
   else
   {
      i = 0;
   }

   memcpy( &Context->Buffer[j], &((uint8_t*)Buffer)[i], BufferSize - i );
}

void Sha1Finalise (Sha1Context* Context, SHA1_HASH* Digest) {
   uint32_t    i;
   uint8_t     finalcount[8];

   for( i=0; i<8; i++ )
   {
      finalcount[i] = (unsigned char)((Context->Count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  // Endian independent
   }
   Sha1Update( Context, (uint8_t*)"\x80", 1 );
   while( (Context->Count[0] & 504) != 448 )
   {
      Sha1Update( Context, (uint8_t*)"\0", 1 );
   }

Sha1Update( Context, finalcount, 8 );  // Should cause a Sha1TransformFunction()
   for( i=0; i<SHA1_HASH_SIZE; i++ )
   {
      Digest->bytes[i] = (uint8_t)((Context->State[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
   }
}

void free_sha1(void *ptr) {
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
      // incTypeFree(IntegerType, 1);
#endif
  free(ptr);
}

Value *malloc_sha1() {
  Sha1Context *ctxt = (Sha1Context *)malloc(sizeof(Sha1Context));
  Sha1Initialise(ctxt);
#ifdef CHECK_MEM_LEAK
  __atomic_fetch_add(&malloc_count, 1, __ATOMIC_ACQ_REL);
  // incTypeMalloc(IntegerType, 1);
#endif
  return(opaqueValue(ctxt, free_sha1));
}

Value *finalize_sha1(Value *ctxt) {
  int64_t shaVal;
  Sha1Finalise(((Opaque *)ctxt)->ptr, (SHA1_HASH *)&shaVal);
  dec_and_free(ctxt, 1);
  return((Value *)integerValue(shaVal));
}

int64_t integerSha1(Value *arg0) {
  int64_t shaVal;
  Sha1Context context;
  Integer *numVal = (Integer *)arg0;

  Sha1Initialise(&context);
  Sha1Update(&context, (void *)&numVal->type, 8);
  Sha1Update(&context, (void *)&numVal->numVal, 8);
  Sha1Finalise(&context, (SHA1_HASH *)&shaVal);
  dec_and_free(arg0, 1);
  return(shaVal);
}

Value *bitAnd(Value *arg0, Value *arg1) {
  Value *result;
  result = integerValue(((Integer *)arg0)->numVal & ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

Value *bitOr(Value *arg0, Value *arg1) {
  Value *result;
  result = integerValue(((Integer *)arg0)->numVal | ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

Value *bitXor(Value *arg0, Value *arg1) {
  Value *result;
  result = integerValue(((Integer *)arg0)->numVal ^ ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

Value *bitShiftLeft(Value *arg0, Value *arg1) {
  Value *result;
  result = integerValue(((Integer *)arg0)->numVal << ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

Value *bitShiftRight(Value *arg0, Value *arg1) {
  Value *result;
  result = integerValue(((Integer *)arg0)->numVal >> ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

Value *bitNot(Value *arg0) {
  Value *result;
  result = integerValue(~((Integer *)arg0)->numVal);
  dec_and_free(arg0, 1);
  return(result);
}

Value *addIntegers(Value *arg0, Value *arg1) {
  Value *numVal = integerValue(((Integer *)arg0)->numVal + ((Integer *)arg1)->numVal);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(numVal);
}

Value *listEQ(Value *arg0, Value *arg1) {
  if (arg1->type != ListType ||
      ((List *)arg0)->len != ((List *)arg1)->len) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else {
    List *l0 = (List *)arg0;
    List *l1 = (List *)arg1;
    for (;
         l0 != (List *)0 && l0->head != (Value *)0 &&
           l1 != (List *)0 && l1->head != (Value *)0;
         l0 = l0->tail, l1 = l1->tail) {
      incRef(l0->head, 1);
      incRef(l1->head, 1);
      if (!equal(l0->head, l1->head)) {
        dec_and_free(arg0, 1);
        dec_and_free(arg1, 1);
        return(nothing);
      }
    }
    dec_and_free(arg1, 1);
    return(maybe((Vector *)0, (Value *)0, arg0));
  }
}

int8_t equal(Value *v1, Value *v2) {
  Value *equals;
  switch (v1->type) {
  case IntegerType:
    equals = integer_EQ(v1, v2);
    break;
  case SymbolType:
    equals = symEQ(v1, v2);
    break;
  default:
    equals = equalSTAR((FnArity *)0, v1, v2);
    break;
  }
   int8_t notEquals = isNothing(equals, "", 0);
   dec_and_free(equals, 1);
   return(!notEquals);
}

Value *stringValue(char *s) {
  int64_t len = strlen(s);
  String *strVal = malloc_string(len);
  strncpy(strVal->buffer, s, len);
  strVal->buffer[len] = 0;
  return((Value *)strVal);
};

Value *maybeExtract(Value *arg0) {
  Maybe *mValue = (Maybe *)arg0;
  if (mValue->value == (Value *)0) {
    (*prErrSTAR)(stringValue("\n*** The 'nothing' value can not be passed to 'extract'.\n"));
    abort();
  }
  incRef(mValue->value, 1);
  Value *result = mValue->value;
  dec_and_free(arg0, 1);
  return(result);
}

Value *fnApply(Value *arg0, Value *arg1) {
  List *argList = (List *)arg1;
  FnArity *_arity;
  if (arg0->type == FunctionType)
    _arity = findFnArity(arg0, argList->len);
  else
    _arity = (FnArity *)arg0;

  if (_arity == (FnArity *)0) {
    fprintf(stderr, "\n*** no arity of '%s' found to apply to %" PRId64 " args\n",
            ((Function *)arg0)->name, argList->len);
    abort();
  } else if(_arity->variadic) {
    FnType1 *_fn = (FnType1 *)_arity->fn;
    Value *result = _fn(_arity, arg1);
    dec_and_free(arg0, 1);
    return(result);
  } else if (argList->len == 0) {
    FnType0 *_fn = (FnType0 *)_arity->fn;
    Value *result = _fn(_arity);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 1) {
    FnType1 *_fn = (FnType1 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    Value *result = _fn(_arity, appArg0);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 2) {
    FnType2 *_fn = (FnType2 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    Value *result = _fn(_arity, appArg0, appArg1);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 3) {
    FnType3 *_fn = (FnType3 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 4) {
    FnType4 *_fn = (FnType4 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 5) {
    FnType5 *_fn = (FnType5 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    argList = argList->tail;
    Value *appArg4 = argList->head; incRef(appArg4, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3, appArg4);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 6) {
    FnType6 *_fn = (FnType6 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    argList = argList->tail;
    Value *appArg4 = argList->head; incRef(appArg4, 1);
    argList = argList->tail;
    Value *appArg5 = argList->head; incRef(appArg5, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3, appArg4, appArg5);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 7) {
    FnType7 *_fn = (FnType7 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    argList = argList->tail;
    Value *appArg4 = argList->head; incRef(appArg4, 1);
    argList = argList->tail;
    Value *appArg5 = argList->head; incRef(appArg5, 1);
    argList = argList->tail;
    Value *appArg6 = argList->head; incRef(appArg6, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3, appArg4, appArg5, appArg6);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 8) {
    FnType8 *_fn = (FnType8 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    argList = argList->tail;
    Value *appArg4 = argList->head; incRef(appArg4, 1);
    argList = argList->tail;
    Value *appArg5 = argList->head; incRef(appArg5, 1);
    argList = argList->tail;
    Value *appArg6 = argList->head; incRef(appArg6, 1);
    argList = argList->tail;
    Value *appArg7 = argList->head; incRef(appArg7, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3, appArg4, appArg5, appArg6, appArg7);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else if (argList->len == 9) {
    FnType9 *_fn = (FnType9 *)_arity->fn;
    Value *appArg0 = argList->head; incRef(appArg0, 1);
    argList = argList->tail;
    Value *appArg1 = argList->head; incRef(appArg1, 1);
    argList = argList->tail;
    Value *appArg2 = argList->head; incRef(appArg2, 1);
    argList = argList->tail;
    Value *appArg3 = argList->head; incRef(appArg3, 1);
    argList = argList->tail;
    Value *appArg4 = argList->head; incRef(appArg4, 1);
    argList = argList->tail;
    Value *appArg5 = argList->head; incRef(appArg5, 1);
    argList = argList->tail;
    Value *appArg6 = argList->head; incRef(appArg6, 1);
    argList = argList->tail;
    Value *appArg7 = argList->head; incRef(appArg7, 1);
    argList = argList->tail;
    Value *appArg8 = argList->head; incRef(appArg8, 1);
    Value *result = _fn(_arity, appArg0, appArg1, appArg2, appArg3, appArg4, appArg5, appArg6, appArg7,
                        appArg8);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  } else {
    fprintf(stderr, "error in 'fn-apply'\n");
    abort();
  }
}

Value *maybeEQ(Value *arg0, Value *arg1) {
  if (arg1->type == MaybeType &&
      ((Maybe *)arg0)->value == ((Maybe *)arg1)->value) {
    dec_and_free(arg1, 1);
    return(maybe((Vector *)0, (Value *)0, arg0));
  } else if (arg1->type == MaybeType &&
             ((Maybe *)arg0)->value != (Value *)0 &&
             ((Maybe *)arg1)->value != (Value *)0) {
    incRef(((Maybe *)arg0)->value, 1);
    incRef(((Maybe *)arg1)->value, 1);
    Value *eqResult = equalSTAR((FnArity *)0, ((Maybe *)arg0)->value, ((Maybe *)arg1)->value);
    if (isNothing(eqResult, "", 0)) {
      dec_and_free(eqResult, 1);
      dec_and_free(arg0, 1);
      dec_and_free(arg1, 1);
      return(nothing);
    } else {
      dec_and_free(eqResult, 1);
      dec_and_free(arg1, 1);
      Value *result = maybe((Vector *)0, (Value *)0, arg0);
      return(result);
    }
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }
}

Value *maybeMap(Value *arg0, Value *arg1) {
  Value *rslt6;
  Maybe *mValue = (Maybe *)arg0;
  if (mValue->value == (Value *)0) {
    dec_and_free(arg1, 1);
    return(arg0);
  } else if((arg1)->type != FunctionType) {
    incRef(arg1, 1);
    incRef(mValue->value, 1);
    rslt6 = invoke1Arg((FnArity *)0, arg1, mValue->value);
  } else {
    FnArity *arity3 = findFnArity(arg1, 1);
    if(arity3 != (FnArity *)0 && !arity3->variadic) {
      FnType1 *fn5 = (FnType1 *)arity3->fn;
      incRef(mValue->value, 1);
      rslt6 = fn5(arity3, mValue->value);
    } else if(arity3 != (FnArity *)0 && arity3->variadic) {
      FnType1 *fn5 = (FnType1 *)arity3->fn;
      List *varArgs4 = empty_list;
      incRef(mValue->value, 1);
      varArgs4 = (List *)listCons(mValue->value, varArgs4);
      rslt6 = fn5(arity3, (Value *)varArgs4);
    } else {
      fprintf(stderr, "\n*** no arity found for '%s'.\n", ((Function *)arg1)->name);
      abort();
    }
  }
  Value *result = maybe((Vector *)0, (Value *)0, rslt6);
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(result);
}

void strSha1Update(Sha1Context *ctxt, Value *arg0) {
  char *buffer;
  int64_t len;
  if (arg0->type == StringBufferType) {
    String *strVal = (String *)arg0;
    buffer = strVal->buffer;
    len = strVal->len;
  } else if (arg0->type == SubStringType) {
    SubString *strVal = (SubString *)arg0;
    buffer = strVal->buffer;
    len = strVal->len;
  }

  Sha1Update(ctxt, (void *)&arg0->type, 8);
  Sha1Update(ctxt, buffer, len);
  return;
}

int64_t strSha1(Value *arg0) {
  int64_t hash;
  char *buffer;
  int64_t len;

  if (arg0->type == StringBufferType) {
    String *strVal = (String *)arg0;
    hash = strVal->hashVal;
    buffer = strVal->buffer;
    len = strVal->len;
  } else if (arg0->type == SubStringType ||
	     arg0->type == SymbolType) {
    SubString *strVal = (SubString *)arg0;
    hash = strVal->hashVal;
    buffer = strVal->buffer;
    len = strVal->len;
  }

  if (hash != 0) {
    dec_and_free(arg0, 1);
    return(hash);
  } else {
    int64_t shaVal;
    Sha1Context context;

    Sha1Initialise(&context);
    Sha1Update(&context, (void *)&arg0->type, 8);
    Sha1Update(&context, buffer, len);
    Sha1Finalise(&context, (SHA1_HASH *)&shaVal);

    ((String *)arg0)->hashVal = shaVal;
    dec_and_free(arg0, 1);
    return(shaVal);
  }
}

Value *escapeChars(Value *arg0) {
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    String *result = malloc_string(s->len * 2);
    char *resultBuffer = result->buffer;
    int resultIndex = 0;
    for(int i = 0; i < s->len; i++) {
      if (s->buffer[i] == 10) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 110;
      } else if (s->buffer[i] == 34) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 34;
      } else if (s->buffer[i] == 13) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 114;
      } else if (s->buffer[i] == 12) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 102;
      } else if (s->buffer[i] == 8) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 98;
      } else if (s->buffer[i] == 9) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 116;
      } else if (s->buffer[i] == 92) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 92;
      } else
        resultBuffer[resultIndex++] = s->buffer[i];
    }
    resultBuffer[resultIndex] = 0;
    result->len = resultIndex;
    dec_and_free(arg0, 1);
    return((Value *)result);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    String *result = malloc_string(s->len * 2);
    char *resultBuffer = result->buffer;
    int resultIndex = 0;
    for(int i = 0; i < s->len; i++) {
      if (s->buffer[i] == 10) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 110;
      } else if (s->buffer[i] == 34) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 34;
      } else if (s->buffer[i] == 13) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 114;
      } else if (s->buffer[i] == 12) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 102;
      } else if (s->buffer[i] == 8) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 98;
      } else if (s->buffer[i] == 9) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 116;
      } else if (s->buffer[i] == 92) {
        resultBuffer[resultIndex++] = 92;
        resultBuffer[resultIndex++] = 92;
      } else
        resultBuffer[resultIndex++] = s->buffer[i];
    }
    resultBuffer[resultIndex] = 0;
    result->len = resultIndex;
    dec_and_free(arg0, 1);
    return((Value *)result);
  }
  return(arg0);
}

Value *opaqueValue(void *ptr, Destructor *destruct) {
  Opaque *opVal = (Opaque *)my_malloc(sizeof(Opaque));
  // incTypeMalloc(OpaqueType, 1);
  opVal->type = OpaqueType;
  opVal->ptr = ptr;
  opVal->destruct = destruct;
  return((Value *)opVal);
};

Value *subs2(Value *arg0, Value *arg1) {
  int64_t idx = ((Integer *)arg1)->numVal;
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    SubString *subStr = malloc_substring();
    subStr->type = SubStringType;
    if (idx < s->len) {
      subStr->len = s->len - idx;
      subStr->source = arg0;
      subStr->buffer = s->buffer + idx;
    } else {
      dec_and_free(arg0, 1);
      subStr->len = 0;
      subStr->source = (Value *)0;
      subStr->buffer = (char *)0;
    }
    dec_and_free(arg1, 1);
    return((Value *)subStr);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    SubString *subStr = malloc_substring();
    subStr->type = SubStringType;
    if (idx < s->len) {
      subStr->len = s->len - idx;
      subStr->source = ((SubString *)arg0)->source;
      incRef(subStr->source, 1);
      subStr->buffer = s->buffer + idx;
    } else {
      subStr->len = 0;
      subStr->source = (Value *)0;
      subStr->buffer = (char *)0;
    }
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return((Value *)subStr);
  }
  return(arg0);
}

Value *subs3(Value *arg0, Value *arg1, Value *arg2) {
  int64_t idx = ((Integer *)arg1)->numVal;
  int64_t len = ((Integer *)arg2)->numVal;
  if (len <= 0) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    dec_and_free(arg2, 1);
    return(stringValue(""));
  } else if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    SubString *subStr = malloc_substring();
    subStr->type = SubStringType;
    if (idx + len <= s->len) {
      subStr->len = len;
      subStr->source = arg0;
      incRef(arg0, 1);
      subStr->buffer = s->buffer + idx;
    } else {
      subStr->len = 0;
      subStr->source = (Value *)0;
      subStr->buffer = (char *)0;
    }
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    dec_and_free(arg2, 1);
    return((Value *)subStr);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    SubString *subStr = malloc_substring();
    subStr->type = SubStringType;
    if (idx + len <= s->len) {
      subStr->len = len;
      subStr->source = ((SubString *)arg0)->source;
      incRef((Value *)subStr->source, 1);
      subStr->buffer = s->buffer + idx;
    } else {
      subStr->len = 0;
      subStr->source = (Value *)0;
      subStr->buffer = (char *)0;
    }
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    dec_and_free(arg2, 1);
    return((Value *)subStr);
  }
  return(arg0);
}

Value *strSeq(Value *arg0) {
  List *result = empty_list;
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    for (int64_t i = s->len - 1; i >= 0; i--) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = listCons((Value *)subStr, result);
    }
    incRef(arg0, s->len);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    for (int64_t i = s->len - 1; i >= 0; i--) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = listCons((Value *)subStr, result);
    }
    incRef(arg0, s->len);
  }
  dec_and_free(arg0, 1);
  return((Value *)result);
}

Value *dynamicCall2Arg(Value *f, Value *arg0, Value *arg1) {
  Value *rslt;
  if(f->type != FunctionType) {
    rslt = invoke2Args((FnArity *)0, f, arg0, arg1);
  } else {
    FnArity *arity = findFnArity(f, 2);
    if(arity != (FnArity *)0 && !arity->variadic) {
      FnType2 *fn = (FnType2 *)arity->fn;
      rslt = fn(arity, arg0, arg1);
    } else if(arity != (FnArity *)0 && arity->variadic) {
      FnType1 *fn = (FnType1 *)arity->fn;
      List *dynArgs = empty_list;
      dynArgs = (List *)listCons(arg1, dynArgs);
      dynArgs = (List *)listCons(arg0, dynArgs);
      rslt = fn(arity, (Value *)dynArgs);
    } else {
      fprintf(stderr, "\n*** Invalid function for string reduction.\n");
      abort();
    }
    dec_and_free(f, 1);
  }
  return(rslt);
}

Value *strReduce(Value *s0, Value *x1, Value *f2) {
  int64_t len = ((String *)s0)->len;
  Value *result = x1;

  char *buffer;
  if (s0->type == StringBufferType)
    buffer = ((String *)s0)->buffer;
  else if (s0->type == SubStringType)
    buffer = ((SubString *)s0)->buffer;

  incRef(f2, len);
  incRef(s0, len);
  for (int64_t i = 0; i < len; i++) {
    SubString *subStr = malloc_substring();
    subStr->type = SubStringType;
    subStr->len = 1;
    subStr->source = s0;
    subStr->buffer = buffer + i;
    result = dynamicCall2Arg(f2, result, (Value *)subStr);
  }
  dec_and_free(f2, 1);
  dec_and_free(s0, 1);
  return((Value *)result);
}

Value *strVec(Value *arg0) {
  Vector *result = empty_vect;
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    for (int64_t i = 0; i < s->len; i++) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = mutateVectConj(result, (Value *)subStr);
    }
    incRef(arg0, s->len);
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    for (int64_t i = 0; i < s->len; i++) {
      SubString *subStr = malloc_substring();
      subStr->type = SubStringType;
      subStr->len = 1;
      subStr->source = arg0;
      subStr->buffer = s->buffer + i;
      result = mutateVectConj(result, (Value *)subStr);
    }
    incRef(arg0, s->len);
  }
  dec_and_free(arg0, 1);
  return((Value *)result);
}

Value *vectorGet(Value *arg0, Value *arg1) {
  Vector *vect = (Vector *)arg0;
  Integer *index = (Integer *)arg1;
  if (index->numVal < 0 || vect->count <= index->numVal) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else {
    Value *val = vectGet(vect, (unsigned)index->numVal);
    incRef(val, 1);
    Value *result = maybe((Vector *)0, (Value *)0, val);
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(result);
  }
}

Value *symbol(Value *arg0) {
  int64_t len;
  char *buffer;
  if (arg0->type == StringBufferType) {
    String *s = (String *)arg0;
    buffer = s->buffer;
    len = s->len;
  } else if (arg0->type == SubStringType) {
    SubString *s = (SubString *)arg0;
    buffer = s->buffer;
    len = s->len;
  } else if (arg0->type == SymbolType) {
    return(arg0);
  }

  SubString *subStr = malloc_substring();
  subStr->type = SymbolType;
  subStr->len = len;
  subStr->source = arg0;
  subStr->hashVal = 0;
  subStr->buffer = buffer;
  return((Value *)subStr);
}

Value *symEQ(Value *arg0, Value *arg1) {
  if (arg0->type != arg1->type) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else {
    SubString *s1 = (SubString *)arg0;
    SubString *s2 = (SubString *)arg1;
    if (s1->len == s2->len &&
	strncmp(s1->buffer, s2->buffer, s1->len) == 0) {
      dec_and_free(arg1, 1);
      return(maybe((Vector *)0, (Value *)0, arg0));
    } else {
      dec_and_free(arg0, 1);
      dec_and_free(arg1, 1);
      return(nothing);
    }
  }
}

Value *symLT(Value *arg0, Value *arg1) {
  if (arg0->type != arg1->type) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  } else {
    SubString *s0 = (SubString *)arg0;
    SubString *s1 = (SubString *)arg1;
    int64_t len;
    if (s0->len < s1->len)
      len = s0->len;
    else
      len = s1->len;

    int cmp = strncmp(s0->buffer, s1->buffer, len);
    if (cmp < 0 || (cmp == 0 && s0->len < s1->len)) {
      dec_and_free(arg1, 1);
      return(maybe((Vector *)0, (Value *)0, arg0));
    } else {
      dec_and_free(arg0, 1);
      dec_and_free(arg1, 1);
      return(nothing);
    }
  }
}

Value *listFilter(Value *arg0, Value *arg1) {
  List *l = (List *)arg0;
  if (l->len == 0) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return((Value *)empty_list);
  } else {
    List *head = empty_list;
    List *tail = empty_list;
    FnArity *arity2;
    if(arg1->type == FunctionType) {
      arity2 = findFnArity(arg1, 1);
      if(arity2 == (FnArity *)0) {
	fprintf(stderr, "\n*** no arity found for '%s'.\n", ((Function *)arg1)->name);
	abort();
      }
    }
    for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
      Value *y;
      incRef(x, 1);
      if(arg1->type != FunctionType) {
	incRef(arg1, 1);
	y = invoke1Arg((FnArity *)0, arg1, x);
      } else if(arity2->variadic) {
	FnType1 *fn4 = (FnType1 *)arity2->fn;
	List *varArgs3 = empty_list;
	varArgs3 = (List *)listCons(x, varArgs3);
	y = fn4(arity2, (Value *)varArgs3);
      } else {
	FnType1 *fn4 = (FnType1 *)arity2->fn;
	y = fn4(arity2, x);
      }

      // 'y' is the filter maybe/nothing value
      if (!isNothing(y, "", 0)) {
	incRef(x, 1);
	if (head == empty_list) {
	  // if we haven't started the new list yet
	  head = malloc_list();
	  head->len = 1;
	  head->head = x;
	  head->tail = empty_list;
	  tail = head;
	} else {
	  // otherwise, append to tail of list
	  List *new_tail = malloc_list();
	  new_tail->len = 1;
	  new_tail->head = x;
	  new_tail->tail = empty_list;
	  tail->tail = new_tail;
	  tail = new_tail;
	  head->len++;
	}
      }
      dec_and_free(y, 1);
    }
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return((Value *)head);
  }
}

BitmapIndexedNode *clone_BitmapIndexedNode(BitmapIndexedNode *node, int idx,
                                           Value *key, Value* val)
{
  int itemCount = __builtin_popcount(node->bitmap);
  BitmapIndexedNode *newNode = malloc_bmiNode(itemCount);
  newNode->bitmap = node->bitmap;
  for (int i = 0; i < itemCount; i++) {
    if (i == idx) {
      newNode->array[i * 2] = key;
      newNode->array[i * 2 + 1] = val;
    } else {
      if (node->array[i * 2] != (Value *)0) {
        incRef(node->array[i * 2], 1);
      }
      if (node->array[i * 2 + 1] != (Value *)0) {
        incRef(node->array[i * 2 + 1], 1);
      }
      newNode->array[i * 2] = node->array[i * 2];
      newNode->array[i * 2 + 1] = node->array[i * 2 + 1];
    }
  }
  return(newNode);
}

Value *createNode(int shift,
		  int64_t key1hash, Value *key1, Value *val1,
		  int64_t key2hash, Value *key2, Value *val2)
{
  if (shift > 60) {
    fprintf(stderr, "Ran out of shift!!!!!!");
    abort();
  }
  BitmapIndexedNode *newNode = malloc_bmiNode(2);
  int key1bit = bitpos(key1hash, shift);
  int key2bit = bitpos(key2hash, shift);
  newNode->bitmap = key1bit | key2bit;
  int key1idx = __builtin_popcount(newNode->bitmap & (key1bit - 1));
  int key2idx = __builtin_popcount(newNode->bitmap & (key2bit - 1));
  if (key1bit == key2bit) {
    newNode->array[0] = (Value *)0;
    newNode->array [1] = createNode(shift + 5, key1hash, key1, val1,
				    key2hash, key2, val2);
  } else {
    newNode->array[key1idx * 2] = key1;
    newNode->array[key1idx * 2 + 1] = val1;
    newNode->array[key2idx * 2] = key2;
    newNode->array[key2idx * 2 + 1] = val2;
  }
  return((Value *)newNode);
}

Value *bmiHashSeq(Value *arg0, Value *arg1) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  int cnt = __builtin_popcount(node->bitmap);
  List *seq = (List *)arg1;
  for (int i = 0; i < cnt; i++) {
    if (node->array[2 * i] == (Value *)0) {
      seq = (List *)hashSeq((FnArity *)0, incRef(node->array[2 * i + 1], 1), (Value *)seq);
    } else {
      List *pair = listCons(node->array[2 * i], listCons(node->array[2 * i + 1], empty_list));
      incRef(node->array[2 * i], 1);
      incRef(node->array[2 * i + 1], 1);
      seq = listCons((Value *)pair, seq);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)seq);
}

Value *bmiHashVec(Value *arg0, Value *arg1) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  int cnt = __builtin_popcount(node->bitmap);
  Vector *vec = (Vector *)arg1;
  for (int i = 0; i < cnt; i++) {
    if (node->array[2 * i] == (Value *)0) {
      vec = (Vector *)hashVec(incRef(node->array[2 * i + 1], 1), (Value *)vec);
    } else {
      incRef(node->array[2 * i], 1);
      incRef(node->array[2 * i + 1], 1);
      Vector *pair = mutateVectConj(empty_vect, node->array[2 * i]);
      pair = mutateVectConj(pair, node->array[2 * i + 1]);
      vec = mutateVectConj(vec, (Value *)pair);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)vec);
}

Value *bmiCount(Value *arg0) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  int cnt = __builtin_popcount(((BitmapIndexedNode *)arg0)->bitmap);
  int accum = 0;
  for(int i = 0; i < cnt; i++) {
    if (node->array[i * 2] == (Value *)0 && node->array[i * 2 + 1] != (Value *)0) {
      Integer *subCnt = (Integer *)count((FnArity *)0,
					 incRef(((BitmapIndexedNode *)arg0)->array[i * 2 + 1], 1));
      accum += subCnt->numVal;
      dec_and_free((Value *)subCnt, 1);
    } else {
      accum++;
    }
  }
  dec_and_free(arg0, 1);
  return(integerValue(accum));
}

Value *bmiCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  Value *key = arg1;
  Value *val = arg2;

  int bit = bitpos(hash, shift);
  int idx = __builtin_popcount(node->bitmap & (bit - 1));
  if (node->bitmap & bit) {
    // if the hash position is already filled
    Value *keyOrNull = node->array[2 * idx];
    Value *valOrNode = node->array[2 * idx + 1];
    if (keyOrNull == (Value *)0) {
      // There is no key in the position, so valOrNode is
      // pointer to a node.
      int newShift = shift + 5;
      Value *n = copyAssoc(incRef(valOrNode, 1), key, val, hash, newShift);
      if (n == valOrNode) {
        // the key was already associated with the value
        // so do nothing
        dec_and_free(n, 1);
        return(arg0);
      } else {
        // clone node and add n to it
        BitmapIndexedNode *newNode = clone_BitmapIndexedNode(node, idx, (Value *)0, n);
	dec_and_free(arg0, 1);
        return((Value *)newNode);
      }
    } else if (equal(incRef(key, 1), incRef(keyOrNull, 1))) {
      if (equal(incRef(val, 1), incRef(valOrNode, 1))) {
        dec_and_free(arg1, 1);
        dec_and_free(arg2, 1);
        return(arg0);
      } else {
        // if the keyOrNull points to a value that is equal to key
        // create new hash-map with valOrNode replaced by val
        // clone node and add val to it
        BitmapIndexedNode *newNode = clone_BitmapIndexedNode(node, idx, key, val);
        dec_and_free((Value *)node, 1);
        return((Value *)newNode);
      }
    } else {
      // there is already a key/val pair at the position where key
      // would be placed. Extend tree a level
      int64_t existingKeyHash = nakedSha1(incRef(keyOrNull, 1));
      if (existingKeyHash == hash) {
        // make & return HashCollisionNode
        HashCollisionNode *newLeaf = malloc_hashCollisionNode(2);
        newLeaf->array[0] = keyOrNull;
        newLeaf->array[1] = valOrNode;
        newLeaf->array[2] = key;
        newLeaf->array[3] = val;
        incRef((Value *)keyOrNull, 1);
        incRef((Value *)valOrNode, 1);

        BitmapIndexedNode *newNode = clone_BitmapIndexedNode(node, idx, (Value *)0,
                                                             (Value *)newLeaf);
        dec_and_free((Value *)node, 1);
        return((Value *)newNode);
      } else {
        Value *newLeaf = createNode(shift + 5,
                                    existingKeyHash, incRef(keyOrNull, 1), incRef(valOrNode, 1),
                                    hash, key, val);
        BitmapIndexedNode *newNode = clone_BitmapIndexedNode(node, idx, (Value *)0, newLeaf);
        dec_and_free((Value *)node, 1);
        return((Value *)newNode);
      }
    }
  } else {
    // the position in the node is empty
    int n = __builtin_popcount(node->bitmap);
    if (n >= 16) {
      ArrayNode *newNode = (ArrayNode *)malloc_arrayNode();
      int jdx = mask(hash, shift);
      int newShift = shift + 5;
      newNode->array[jdx] = copyAssoc((Value *)&emptyBMI, key, val, hash, newShift);
      for (int i = 0, j = 0; i < ARRAY_NODE_LEN; i++) {
        if ((node->bitmap >> i) & 1) {
          if (node->array[j] == (Value *)0) {
            newNode->array[i] = node->array[j + 1];
            incRef(newNode->array[i], 1);
          } else {
            incRef(node->array[j], 2);
	    newNode->array[i] = copyAssoc((Value *)&emptyBMI,
					  node->array[j],
					  incRef(node->array[j + 1], 1),
					  nakedSha1(node->array[j]),
					  newShift);
	  }
	  j += 2;
	}
      }
      dec_and_free((Value *)node, 1);
      return((Value *)newNode);
    } else {
      int itemCount = n + 1;
      BitmapIndexedNode *newNode = malloc_bmiNode(itemCount);
      newNode->bitmap = node->bitmap | bit;
      for (int i = 0; i < idx * 2; i++) {
        if (node->array[i] != (Value *)0) {
          incRef(node->array[i], 1);
        }
        newNode->array[i] = node->array[i];
      }
      newNode->array[2 * idx] = key;
      newNode->array[2 * idx + 1] = val;
      for (int i = idx * 2; i < n * 2; i++) {
        if (node->array[i] != (Value *)0) {
          incRef(node->array[i], 1);
        }
        newNode->array[i + 2] = node->array[i];
      }
      dec_and_free((Value *)node, 1);
      return((Value *)newNode);
    }
  }
}

Value *bmiMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  if (arg0->refs != 1) {
    return(bmiCopyAssoc(arg0, arg1, arg2, hash, shift));
  } else {
    BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
    Value *key = arg1;
    Value *val = arg2;

    int bit = bitpos(hash, shift);
    int idx = __builtin_popcount(node->bitmap & (bit - 1));
    if (node->bitmap & bit) {
      // if the hash position is already filled
      Value *keyOrNull = node->array[2 * idx];
      Value *valOrNode = node->array[2 * idx + 1];
      if (keyOrNull == (Value *)0) {
	// There is no key in the position, so valOrNode is
	// pointer to a node.
	Value *n;
	n = mutateAssoc(valOrNode, key, val, hash, shift + 5);
	// replace key/val at 'idx' with new stuff
	node->array[idx * 2] = (Value *)0;;
	node->array[idx * 2 + 1] = n;
	return(arg0);
      } else if (equal(incRef(key, 1), incRef(keyOrNull, 1))) {
	if (equal(incRef(val, 1), incRef(valOrNode, 1))) {
	  dec_and_free(arg1, 1);
	  dec_and_free(arg2, 1);
	  return(arg0);
	} else {
	  // if the keyOrNull points to a value that is equal to key
	  // replace key/val at 'idx' with new stuff
	  node->array[idx * 2] = key;
	  node->array[idx * 2 + 1] = val;
	  dec_and_free(valOrNode, 1);
	  dec_and_free(keyOrNull, 1);
	  return(arg0);
	}
      } else {
	// there is already a key/val pair at the position where key
	// would be placed. Extend tree a level
	int64_t existingKeyHash = nakedSha1(incRef(keyOrNull, 1));
	if (existingKeyHash == hash) {
	  // make & return HashCollisionNode
	  HashCollisionNode *newLeaf = malloc_hashCollisionNode(2);
	  newLeaf->array[0] = keyOrNull;
	  newLeaf->array[1] = valOrNode;
	  newLeaf->array[2] = key;
	  newLeaf->array[3] = val;

	  // replace key/val at 'idx' with new stuff
	  node->array[idx * 2] = (Value *)0;
	  node->array[idx * 2 + 1] = (Value *)newLeaf;
	  return(arg0);
	} else {
	  Value *newLeaf = createNode(shift + 5,
				      existingKeyHash, keyOrNull, valOrNode,
				      hash, key, val);
	  // replace key/val at 'idx' with new stuff
	  node->array[idx * 2] = (Value *)0;
	  node->array[idx * 2 + 1] = (Value *)newLeaf;
	  return(arg0);
	}
      }
    } else {
      // the position in the node is empty
      int n = __builtin_popcount(node->bitmap);
      if (n >= 16) {
	ArrayNode *newNode = (ArrayNode *)malloc_arrayNode();
	int jdx = mask(hash, shift);
	int newShift = shift + 5;
	newNode->array[jdx] = copyAssoc((Value *)&emptyBMI, key, val, hash, newShift);
	for (int i = 0, j = 0; i < ARRAY_NODE_LEN; i++) {
	  if ((node->bitmap >> i) & 1) {
	    if (node->array[j] == (Value *)0) {
	      newNode->array[i] = node->array[j + 1];
	      node->array[j + 1] = (Value *)0;
	    } else {
	      incRef(node->array[j], 1);
	      newNode->array[i] = copyAssoc((Value *)&emptyBMI,
					    node->array[j],
					    node->array[j + 1],
					    nakedSha1(node->array[j]),
					    newShift);
	      node->array[j] = (Value *)0;
	      node->array[j + 1] = (Value *)0;
	    }
	    j += 2;
	  }
	}
	node->bitmap = 0;
	dec_and_free((Value *)node, 1);
	return((Value *)newNode);
      } else {
	int itemCount = n + 1;
	BitmapIndexedNode *newNode = malloc_bmiNode(itemCount);
	newNode->bitmap = node->bitmap | bit;
	for (int i = 0; i < idx * 2; i++) {
	  if (node->array[i] != (Value *)0) {
	    incRef(node->array[i], 1);
	  }
	  newNode->array[i] = node->array[i];
	}
	newNode->array[2 * idx] = key;
	newNode->array[2 * idx + 1] = val;
	for (int i = idx * 2; i < n * 2; i++) {
	  if (node->array[i] != (Value *)0) {
	    incRef(node->array[i], 1);
	  }
	  newNode->array[i + 2] = node->array[i];
	}
	dec_and_free((Value *)node, 1);
	return((Value *)newNode);
      }
    }
  }
}

Value *bmiGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash,  int shift) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  Value *key = arg1;

  int bit = bitpos(hash, shift);
  int idx = __builtin_popcount(node->bitmap & (bit - 1));
  if (node->bitmap & bit) {
    // if the hash position is filled
    Value *keyOrNull = node->array[2 * idx];
    Value *valOrNode = node->array[2 * idx + 1];
    if (keyOrNull == (Value *)0) {
      // There is no key in the position, so valOrNode is
      // pointer to a node.
      Value *v = get((FnArity *)0, incRef(valOrNode, 1), key, arg2, hash, shift + 5);
      dec_and_free(arg0, 1);
      return(v);
    } else {
      incRef(keyOrNull, 1);
      if (equal(key, keyOrNull)) {
	// found 'key' at this position
	incRef(valOrNode, 1);
	dec_and_free(arg0, 1);
	dec_and_free(arg2, 1);
	return(valOrNode);
      } else {
	// there's a key in this position, but doesn't equal 'key'
	dec_and_free(arg0, 1);
	return(arg2);
      }
    }
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(arg2);
  }
}

Value *bmiDissoc(Value *arg0, Value* arg1, int64_t hash, int shift) {
  BitmapIndexedNode *node = (BitmapIndexedNode *)arg0;
  Value *key = arg1;

  int bit = bitpos(hash, shift);
  int idx = __builtin_popcount(node->bitmap & (bit - 1));
  if (node->bitmap & bit) {
    // if the hash position is already filled
    Value *keyOrNull = node->array[2 * idx];
    Value *valOrNode = node->array[2 * idx + 1];
    if (keyOrNull == (Value *)0) {
      // There is no key in the position, so valOrNode is
      // pointer to a node.
      Value *n = baseDissoc(incRef(valOrNode, 1), key, hash, shift + 5);
      if (n == valOrNode) {
	// the key was not in the hash-map
	// so do nothing
	dec_and_free(n, 1);
	return(arg0);
      } else if (n == (Value *)&emptyBMI && __builtin_popcount(node->bitmap) == 1) {
	// the subtree is now empty, and this node only points to it, so propagate
	dec_and_free(arg0, 1);
	return(n);
      } else {
	// clone node and add n to it
	BitmapIndexedNode *newNode = clone_BitmapIndexedNode(node, idx, (Value *)0, n);
	dec_and_free(arg0, 1);
	return((Value *)newNode);
      }
    } else if (equal(key, incRef(keyOrNull, 1))) {
      // if the keyOrNull points to a value that is equal to key
      if (__builtin_popcount(node->bitmap) == 1) {
	// and that is the only entry in this node
	dec_and_free(arg0, 1);
	return((Value *)&emptyBMI);
      } else {
	// create new hash-map with keyOrNull and valOrNode replaced by (Value *)0
	int itemCount = __builtin_popcount(node->bitmap);
	BitmapIndexedNode *newNode = malloc_bmiNode(itemCount - 1);
	newNode->bitmap = node->bitmap;
        int i, j;
        for (i = 0, j = 0; i < itemCount; i++) {
          if (i != idx) {
            if (node->array[i * 2] != (Value *)0) {
              incRef(node->array[i * 2], 1);
            }
            if (node->array[i * 2 + 1] != (Value *)0) {
              incRef(node->array[i * 2 + 1], 1);
            }
            newNode->array[j * 2] = node->array[i * 2];
            newNode->array[j * 2 + 1] = node->array[i * 2 + 1];
            j++;
          }
        }
        newNode->bitmap &= ~bit;
        dec_and_free(arg0, 1);
        return((Value *)newNode);
      }
    } else {
      // there is already a key/val pair at the position where key
      // would be. Do nothing
      return(arg0);
    }
  } else {
    // the position in the node is empty, do nothing
    dec_and_free(arg1, 1);
    return(arg0);
  }
}

Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  ArrayNode *node = (ArrayNode *)arg0;
  Value *key = arg1;
  Value *val = arg2;
  int idx = mask(hash, shift);
  int newShift = shift + 5;
  ArrayNode *newNode;

  Value *subNode = node->array[idx];
  int64_t keyHash = nakedSha1(incRef(key, 1));
  if (subNode == (Value *)0) {
    newNode = (ArrayNode *)malloc_arrayNode();
    for (int i = 0; i < ARRAY_NODE_LEN; i++) {
      if (node->array[i] != (Value *)0) {
	newNode->array[i] = node->array[i];
	incRef(newNode->array[i], 1);
      }
    }
    newNode->array[idx] = copyAssoc((Value *)&emptyBMI, key, val, keyHash, newShift);
  } else {
    Value *n = copyAssoc(incRef(subNode, 1), key, val, keyHash, newShift);
    if (n == subNode) {
      dec_and_free(n, 1);
      return((Value *)node);
    } else {
      newNode = (ArrayNode *)malloc_arrayNode();
      for (int i = 0; i < ARRAY_NODE_LEN; i++) {
	if (i != idx && node->array[i] != (Value *)0) {
	  newNode->array[i] = node->array[i];
	  incRef(newNode->array[i], 1);
	}
      }
      newNode->array[idx] = n;
    }
  }
  dec_and_free((Value *)node, 1);
  return((Value *)newNode);
}

Value *arrayNodeMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  if (arg0->refs != 1) {
    return(arrayNodeCopyAssoc(arg0, arg1, arg2, hash, shift));
  } else {
    ArrayNode *node = (ArrayNode *)arg0;
    Value *key = arg1;
    Value *val = arg2;
    int idx = mask(hash, shift);

    Value *subNode = node->array[idx];
    int64_t keyHash = nakedSha1(incRef(key, 1));
    if (subNode == (Value *)0) {
      node->array[idx] = copyAssoc((Value *)&emptyBMI, key, val, keyHash, shift + 5);
    } else {
      Value *n = mutateAssoc(subNode, key, val, keyHash, shift + 5);
      node->array[idx] = n;
    }
    return((Value *)node);
  }
}

Value *collisionAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  HashCollisionNode *node = (HashCollisionNode *)arg0;
  Value *key = arg1;
  Value *val = arg2;
  int itemCount = node->count / 2;

  if(nakedSha1(incRef(node->array[0], 1)) == hash) {
    HashCollisionNode *newNode = malloc_hashCollisionNode(itemCount + 1);
    for (int i = 0; i < itemCount; i++) {
      if (equal(incRef(key, 1), incRef(node->array[2 * i], 1))) {
	newNode->array[2 * i] = key;
	newNode->array[2 * i + 1] = val;
	newNode->count -= 2;
      } else {
	newNode->array[2 * i] = node->array[2 * i];
	newNode->array[2 * i + 1] = node->array[2 * i + 1];
	incRef(node->array[2 * i], 1);
	incRef(node->array[2 * i + 1], 1);
      }
    }
    if (newNode->count / 2 != itemCount) {
      newNode->array[2 * itemCount] = key;
      newNode->array[2 * itemCount + 1] = val;
    }
    dec_and_free(arg0, 1);
    return((Value *)newNode);
  } else {
    BitmapIndexedNode * bmi = (BitmapIndexedNode *)copyAssoc((Value *)&emptyBMI,
							     key, val, hash, 0);
    for (int i = 0; i < itemCount; i++) {
      bmi = (BitmapIndexedNode *)mutateAssoc((Value *)bmi,
					     incRef(node->array[2 * i], 1),
					     incRef(node->array[2 * i + 1], 1),
					     nakedSha1(incRef(node->array[2 * i], 1)), 0);
    }
    dec_and_free(arg0, 1);
    return((Value *)bmi);
  }
}

Value notFound = {0, -2};
Value *notFoundPtr = &notFound;

Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  ArrayNode *node = (ArrayNode *)arg0;
  Value *key = arg1;
  Value *notFound = arg2;
  int idx = mask(hash, shift);

  Value *subNode = node->array[idx];
  if (subNode == (Value *)0) {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(notFound);
  } else {
    incRef(subNode, 1);
    dec_and_free(arg0, 1);
    return(get((FnArity *)0, subNode, key, notFound, hash, shift + 5));
  }
}

Value *arrayNodeCount(Value *arg0) {
  int accum = 0;
  for(int i = 0; i < ARRAY_NODE_LEN; i++){
    if (((ArrayNode *)arg0)->array[i] != (Value *)0) {
      Integer *subCnt = (Integer *)count((FnArity *)0, incRef(((ArrayNode *)arg0)->array[i], 1));
      accum += subCnt->numVal;
      dec_and_free((Value *)subCnt, 1);
    }
  }
  dec_and_free(arg0, 1);
  return(integerValue(accum));
}

Value *collisionCount(Value *arg0) {
  Value *result = integerValue(((HashCollisionNode *) arg0)->count / 2);
  dec_and_free(arg0, 1);
  return(result);
}

Value *collisionSeq(Value *arg0, Value *arg1) {
  HashCollisionNode *node = (HashCollisionNode *)arg0;
  List *seq = (List *)arg1;
  for (int i = 0; i < node->count / 2; i++) {
    if (node->array[2 * i] != (Value *)0 && node->array[2 * i + 1] != (Value *)0) {
      List *pair = listCons(node->array[2 * i], listCons(node->array[2 * i + 1], empty_list));
      incRef(node->array[2 * i], 1);
      incRef(node->array[2 * i + 1], 1);
      seq = listCons((Value *)pair, seq);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)seq);
}

Value *collisionVec(Value *arg0, Value *arg1) {
  HashCollisionNode *node = (HashCollisionNode *)arg0;
  Vector *vec = (Vector *)arg1;
  for (int i = 0; i < node->count / 2; i++) {
    if (node->array[2 * i] != (Value *)0 && node->array[2 * i + 1] != (Value *)0) {
      incRef(node->array[2 * i], 1);
      incRef(node->array[2 * i + 1], 1);
      Vector *pair = mutateVectConj(empty_vect, node->array[2 * i]);
      pair = mutateVectConj(pair, node->array[2 * i + 1]);
      vec = mutateVectConj(vec, (Value *)pair);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)vec);
}

Value *collisionDissoc(Value *arg0, Value *arg1, int64_t hash, int shift) {
  HashCollisionNode *node = (HashCollisionNode *)arg0;
  Value *key = arg1;
  HashCollisionNode *newNode;
  int itemCount = node->count / 2;

  if(itemCount == 1) {
    if(equal(key, incRef(node->array[0], 1))) {
      dec_and_free(arg0, 1);
      return((Value *)&emptyBMI);
    } else {
      return(arg0);
    }
  } else {
    int keyIdx = -1;
    int i = 0;
    do {
      keyIdx = i;
      i++;
    } while (i < itemCount && !equal(incRef(key, 1), incRef(node->array[2 * i], 1)));

    if(keyIdx >= 0) {
      newNode = malloc_hashCollisionNode(itemCount - 1);
      for (int i = 0, j = 0; i < itemCount; i++) {
        if (i != keyIdx) {
          newNode->array[j * 2] = node->array[i * 2];
          newNode->array[j * 2 + 1] = node->array[i * 2 + 1];
          incRef(newNode->array[j * 2], 1);
          incRef(newNode->array[j * 2 + 1], 1);
          j++;
        }
      }
      dec_and_free(arg0, 1);
      dec_and_free(arg1, 1);
      return((Value *)newNode);
    }
  }
  return(arg0);
}

Value *collisionGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  HashCollisionNode *node = (HashCollisionNode *)arg0;
  for (int i = 0; i < node->count / 2; i++) {
    if (node->array[2 * i] != (Value *)0 && equal(incRef(arg1, 1),
						  incRef(node->array[2 * i], 1))) {
      if (node->array[2 * i + 1] != (Value *)0) {
	incRef(node->array[2 * i + 1], 1);
	dec_and_free(arg0, 1);
	dec_and_free(arg1, 1);
	dec_and_free(arg2, 1);
	return(node->array[2 * i + 1]);
      } else {
fprintf(stderr, "Trying to get an invalid value from a CollisionNode of a hash-map. This should never happen!!!");
abort();
	dec_and_free(arg0, 1);
	dec_and_free(arg1, 1);
	return(arg2);
      }
    }
  }
  dec_and_free(arg0, 1);
  dec_and_free(arg1, 1);
  return(arg2);
}

Value *arrayNodeSeq(Value *arg0, Value *arg1) {
  ArrayNode *node = (ArrayNode *)arg0;
  List *seq = (List *)arg1;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) {
    if (node->array[i] != (Value *)0) {
      incRef(node->array[i], 1);
      seq = (List *)hashSeq((FnArity *)0, node->array[i], (Value *)seq);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)seq);
}

Value *arrayNodeVec(Value *arg0, Value *arg1) {
  ArrayNode *node = (ArrayNode *)arg0;
  Vector *vec = (Vector *)arg1;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) {
    if (node->array[i] != (Value *)0) {
      incRef(node->array[i], 1);
      vec = (Vector *)hashVec(node->array[i], (Value *)vec);
    }
  }
  dec_and_free(arg0, 1);
  return((Value *)vec);
}

Value *arrayNodeDissoc(Value *arg0, Value *arg1, int64_t hash, int shift) {
  ArrayNode *node = (ArrayNode *)arg0;
  Value *key = arg1;
  int idx = mask(hash, shift);
  ArrayNode *newNode;

  Value *subNode = node->array[idx];
  if (subNode == (Value *)0) {
    // do nothing
    dec_and_free(arg1, 1);
    return(arg0);
  } else {
      int64_t hash = nakedSha1(incRef(key, 1));
      Value *n = baseDissoc(incRef(subNode, 1), key, hash, shift + 5);
      newNode = (ArrayNode *)malloc_arrayNode();
      for (int i = 0; i < ARRAY_NODE_LEN; i++) {
        if (i != idx && node->array[i] != (Value *)0) {
          newNode->array[i] = node->array[i];
          incRef(newNode->array[i], 1);
        }
      }
      newNode->array[idx] = n;
      dec_and_free(arg0, 1);
  }
  return((Value *)newNode);
}

Value *get(FnArity *arity, Value *node, Value *k, Value *v, int64_t hash, int shift) {
  switch(node->type) {
  case BitmapIndexedType:
    return(bmiGet(node, k, v, hash, shift));
  case ArrayNodeType:
    return(arrayNodeGet(node, k, v, hash, shift));
  case HashCollisionNodeType:
    return(collisionGet(node, k, v, hash, shift));
  default:
    fprintf(stderr, "Can't get from that kind of node\n");
    abort();
  }
}

Value *baseDissoc(Value *node, Value *k, int64_t hash, int shift) {
  switch(node->type) {
  case BitmapIndexedType:
    return(bmiDissoc(node, k, hash, shift));
  case ArrayNodeType:
    return(arrayNodeDissoc(node, k, hash, shift));
  case HashCollisionNodeType:
    return(collisionDissoc(node, k, hash, shift));
  default:
    fprintf(stderr, "Can't dissoc from that kind of node\n");
    abort();
  }
}

Value *hashVec(Value *node, Value *vec) {
  switch(node->type) {
  case BitmapIndexedType:
    return(bmiHashVec(node, vec));
  case ArrayNodeType:
    return(arrayNodeVec(node, vec));
  case HashCollisionNodeType:
    return(collisionVec(node, vec));
  default:
    fprintf(stderr, "Can't assoc into that kind of node\n");
    abort();
  }
}

Value *copyAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift) {
  switch(node->type) {
  case BitmapIndexedType:
    return(bmiCopyAssoc(node, k, v, hash, shift));
  case ArrayNodeType:
    return(arrayNodeCopyAssoc(node, k, v, hash, shift));
  case HashCollisionNodeType:
    return(collisionAssoc(node, k, v, hash, shift));
  default:
    fprintf(stderr, "Can't assoc into that kind of node\n");
    abort();
  }
}

Value *mutateAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift) {
  switch(node->type) {
  case BitmapIndexedType:
    return(bmiMutateAssoc(node, k, v, hash, shift));
  case ArrayNodeType:
    return(arrayNodeMutateAssoc(node, k, v, hash, shift));
    /*
  case HashCollisionNodeType:
    return(collisionAssoc(node, k, v, hash, shift));
    // */
  default:
    return(copyAssoc(node, k, v, hash, shift));
  }
}

Value *hashMapGet(Value *arg0, Value *arg1) {
  int64_t hash = nakedSha1(incRef(arg1, 1));
  Value *found = get((FnArity *)0, arg0, arg1, notFoundPtr, hash, 0);
  if (found == notFoundPtr) {
    return(nothing);
  } else {
    return(maybe((Vector *)0, (Value *)0, found));
  }
}

// used for static encoding hash maps and other things
Value *hashMapAssoc(Value *arg0, Value *arg1, Value *arg2) {
  int64_t hash = nakedSha1(incRef(arg1, 1));
  return(mutateAssoc(arg0, arg1, arg2, hash, 0));
}

Value *dynamicCall1Arg(Value *f, Value *arg) {
  Value *rslt;
  if(f->type != FunctionType) {
    rslt = invoke1Arg((FnArity *)0, f, arg);
  } else {
    FnArity *arity = findFnArity(f, 1);
    if(arity != (FnArity *)0 && !arity->variadic) {
      FnType1 *fn = (FnType1 *)arity->fn;
      rslt = fn(arity, arg);
    } else if(arity != (FnArity *)0 && arity->variadic) {
      FnType1 *fn = (FnType1 *)arity->fn;
      List *dynArgs = empty_list;
      dynArgs = (List *)listCons(arg, dynArgs);
      rslt = fn(arity, (Value *)dynArgs);
    } else {
      fprintf(stderr, "\n*** Invalid action for Promise.\n");
      abort();
    }
    dec_and_free(f, 1);
  }
  return(rslt);
}

Value *addPromiseAction(Promise *p, Value *action) {
  pthread_mutex_lock(&p->access);
  if (p->result == (Value *)0) {
    List *newList = malloc_list();
    newList->head = (Value *)action;
    List *actions;
#ifdef SINGLE_THREADED
    actions = p->actions;
    newList->len = actions->len + 1;
    newList->tail = actions;
    p->actions = newList;
#else
    __atomic_load(&p->actions, &actions, __ATOMIC_RELAXED);
    do {
      if (actions != (List *)0) {
	newList->len = actions->len + 1;
	newList->tail = actions;
      } else {
	newList->len = 1;
	newList->tail = empty_list;
      }
    } while (!__atomic_compare_exchange((List **)&p->actions, (List **)&actions, (List **)&newList,
					1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
#endif
    pthread_mutex_unlock(&p->access);
  } else {
    pthread_mutex_unlock(&p->access);
    incRef(p->result, 1);
    Value *trash = dynamicCall1Arg(action, p->result);
    dec_and_free(trash, 1);
  }
  return((Value *)p);
}

Value *deliverPromise(Value *arg0, Value *arg1) {
  // TODO: Must check for refs to arg0 in arg1
  // also for agents and maybe futures
  Promise *p = (Promise *)arg0;
  if (p->result == (Value *)0) {
    pthread_mutex_lock(&p->access);
    if (p->result == (Value *)0) {
      p->result = arg1;
      pthread_cond_broadcast(&p->delivered);
    }
    List *l = p->actions;
    List *head = l;
    p->actions = (List *)0;

    // int refs = arg0->refs;
    // breakCycle(arg1, arg0);
    // fprintf(stderr, "cycle refs: %d\n", p->cycleRefs);
    // arg0->refs = refs;

    pthread_mutex_unlock(&p->access);

    // perform actions
    if (l != (List *)0 && l->len != 0) {
      for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
	incRef(x, 1);
	incRef(arg1, 1);
	Value *trash = dynamicCall1Arg(x, arg1);
	dec_and_free(trash, 1);
      }
    }
    dec_and_free((Value *)head, 1);
  } else {
    dec_and_free(arg1, 1);
  }
  return(arg0);
}

Value *extractPromise(Value *arg0) {
  Promise *p = (Promise *)arg0;
  while (p->result == (Value *)0) {
    pthread_mutex_lock (&p->access);
    if (p->result == (Value *)0) {
#ifdef SINGLE_THREADED
      runningWorkers--;
      int rw = runningWorkers;
#else
      int rw = __atomic_fetch_sub(&runningWorkers, 1, __ATOMIC_ACQ_REL);
#endif
      replaceWorker();
      pthread_cond_wait(&p->delivered, &p->access);
#ifdef SINGLE_THREADED
      runningWorkers++;
#else
      __atomic_fetch_add(&runningWorkers, 1, __ATOMIC_ACQ_REL);
#endif
    }
    pthread_mutex_unlock (&p->access);
  }
  Value *result = p->result;
  incRef(result, 1);
  dec_and_free(arg0, 1);
  return(result);
}

Value *promiseDelivered(Value *arg0) {
  Promise *p = (Promise *)arg0;
  if(p->result == (Value *)0) {
    dec_and_free(arg0, 1);
    return(nothing);
  } else {
    Value *mv = maybe((Vector *)0, (Value *)0, p->result);
    incRef(p->result, 1);
    dec_and_free(arg0, 1);
    return((Value *)mv);
  }
}

Value *extractFuture(Value *arg0) {
  Future *f = (Future *)arg0;
  while (f->result == (Value *)0) {
    pthread_mutex_lock (&f->access);
    if (f->result == (Value *)0) {
#ifdef SINGLE_THREADED
      runningWorkers--;
#else
      __atomic_fetch_sub(&runningWorkers, 1, __ATOMIC_ACQ_REL);
#endif
      replaceWorker();
      pthread_cond_wait(&f->delivered, &f->access);
#ifdef SINGLE_THREADED
      runningWorkers++;
#else
      __atomic_fetch_add(&runningWorkers, 1, __ATOMIC_ACQ_REL);
#endif
    }
    pthread_mutex_unlock (&f->access);
  }
  Value *result = f->result;
  incRef(result, 1);
  dec_and_free((Value *)f, 1);
  return(result);
}

Value *makeFuture(Value *arg0) {
  Future *f = malloc_future(__LINE__);
  f->action = arg0;
  if (arg0 != (Value *)0) {
    incRef((Value *)f, 1);
    scheduleFuture(f);
  }
  return((Value *)f);
}

Value *addFutureAction(Future *p, Value *action) {
  pthread_mutex_lock(&p->access);
  if (p->result == (Value *)0) {
    List *newList = malloc_list();
    newList->head = (Value *)action;
    List *actions;
#ifdef SINGLE_THREADED
    actions = p->actions;
    if (actions != (List *)0) {
      newList->len = actions->len + 1;
      newList->tail = actions;
    } else {
      newList->len = 1;
      newList->tail = empty_list;
    }
    p->actions = newList;
#else
    __atomic_load(&p->actions, &actions, __ATOMIC_RELAXED);
    do {
      if (actions != (List *)0) {
	newList->len = actions->len + 1;
	newList->tail = actions;
      } else {
	newList->len = 1;
	newList->tail = empty_list;
      }
    } while (!__atomic_compare_exchange((List **)&p->actions, (List **)&actions, (List **)&newList, 1,
					__ATOMIC_RELAXED, __ATOMIC_RELAXED));
#endif
    pthread_mutex_unlock(&p->access);
  } else {
    pthread_mutex_unlock(&p->access);
    incRef(p->result, 1);
    Value *trash = dynamicCall1Arg(action, p->result);
    dec_and_free(trash, 1);
  }
  return((Value *)p);
}

Value *makeAgent(Value *arg0) {
  Agent *a = (Agent *)my_malloc(sizeof(Agent));
  // incTypeMalloc(AgentType, 1);
  a->type = AgentType;
#ifdef SINGLE_THREADED
  a->refs = refsInit;
#else
  __atomic_store(&a->refs, &refsInit, __ATOMIC_RELAXED);
#endif
  a->input = empty_list;
  a->output = empty_list;
  pthread_mutex_init(&a->access, NULL);
  a->val = arg0;
  return((Value *)a);
}

Value *extractAgent(Value *arg0) {
  pthread_mutex_lock (&((Agent *)arg0)->access);
  Value *v = ((Agent *)arg0)->val;
  incRef(v, 1);
  pthread_mutex_unlock (&((Agent *)arg0)->access);
  dec_and_free(arg0, 1);
  return(v);
}

List *readAgentQueue(Agent *agent) {
  List *output = agent->output;
  if (output != (List *)0 && output->len != 0) {
    // if there was an item in the queue, return it
    Value *item = output->head;
    agent->output = output->tail;
    output->head = (Value *)0;
    output->tail = (List *)0;
    REFS_SIZE refs;
#ifdef SINGLE_THREADED
    refs = output->refs;
#else
    __atomic_load(&output->refs, &refs, __ATOMIC_RELAXED);
#endif
    if (refs != 1) {
      fprintf(stderr, "failure in readAgentQueue()\n");
      abort();
    }
    dec_and_free((Value *)output, 1);
    return((List *)item);
  } else {
    // move the input list to the output
    // atomically get the input list and reset it to empty_list
    List *input;
#ifdef SINGLE_THREADED
    input = agent->input;
    agent->input = empty_list;
#else
    __atomic_exchange((List **)&agent->input,
		      (List **)&empty_list,
		      (List **)&input,
		      __ATOMIC_RELAXED);
#endif

    if (input == (List *)0 || input->len == 0) {
      // if the input was empty, return 0
      agent->output = input;
      return((List *)0);
    } else {
      // otherwise, move the input list to the output
      agent->output = reverseList(input);
      return(readAgentQueue(agent));
    }
  }
}

Value *updateAgent_impl(FnArity *arity) {
  Agent *agent = (Agent *)(arity->closures)->tail[0];
  if (pthread_mutex_trylock (&agent->access) == 0) { // succeeded
    List *action = readAgentQueue(agent);
    while(action != (List *)0) {
      Value *f = (Value *)action->head;
      List *args = listCons(agent->val, action->tail);
      incRef((Value *)action->tail, 1);
      agent->val = fn_apply((FnArity *)0, incRef((Value *)f, 1), (Value *)args);
      dec_and_free((Value *)action, 1);
      action = readAgentQueue(agent);
    }
    pthread_mutex_unlock (&agent->access);
  }
  return(nothing);
};

void scheduleAgent(Agent *agent, List *action) {
#ifdef SINGLE_THREADED
  Value *f = (Value *)action->head;
  incRef((Value *)agent->val, 1);
  List *args = listCons(agent->val, action->tail);
  incRef((Value *)action->tail, 1);
  agent->val = fn_apply((FnArity *)0, (Value *)f, (Value *)args);
  action->head = (Value *)0;
  dec_and_free((Value *)action, 1);
#else
  List *newList = malloc_list();
  newList->head = (Value *)action;
  List *input;
  __atomic_load(&agent->input, &input, __ATOMIC_RELAXED);
  do {
    newList->len = input->len + 1;
    newList->tail = input;
  } while (!__atomic_compare_exchange(&agent->input, &input, &newList, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

  FnArity *updateAgentArity = malloc_fnArity();
  updateAgentArity->variadic = 0;
  updateAgentArity->fn = updateAgent_impl;
  updateAgentArity->count = 0;
  incRef((Value *)agent, 1);
  updateAgentArity->closures = mutateVectConj(empty_vect, (Value *)agent);
  Function *updateAgentFn = malloc_function(1);
  updateAgentFn->name = "update-agent";
  updateAgentFn->arityCount = 1;
  updateAgentFn->arities[0] = updateAgentArity;
  Future *f = malloc_future(__LINE__);
  f->action = (Value *)updateAgentFn;
  f->actions = empty_list;
  scheduleFuture(f);
#endif
}

void freeExtractCache(void *cachePtr) {
  extractCache *cacheTail = (extractCache *)cachePtr;
  if (cacheTail != (extractCache *)0) {
    dec_and_free((Value *)cacheTail->tail, 1);
    if (!cleaningUp)
      free(cacheTail);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
      // incTypeFree(14, 1);
#endif
   }
}

void freeIntGenerator(void *ptr) {
  if (ptr != (void *)0) {
    if (!cleaningUp)
      free(ptr);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
      // incTypeFree(14, 1);
#endif
  }
}

String *nullTerm(Value *s) {
  String *arg0Str = malloc_string(((String *)s)->len);
  if (s->type == StringBufferType)
    snprintf(arg0Str->buffer, ((String *)s)->len + 1, "%s", ((String *)s)->buffer);
  else if (s->type == SubStringType)
    snprintf(arg0Str->buffer, ((String *)s)->len + 1, "%s", ((SubString *)s)->buffer);
  dec_and_free(s, 1);
  return(arg0Str);
}

void show(Value *v) {
  if (v == (Value *)0) {
    fprintf(stderr, "Null\n");
    return;
  }
  if (v->refs == refsError) {
    fprintf(stderr, "has been freed\n");
    return;
  }
  incRef(v, 1);
  List *strings = (List *)showFn((FnArity *)0, v);
  List *l = strings;
  for (Value *h = l->head; l != (List *)0 && h != (Value *)0; h = l->head) {
    incRef(h, 1);
    prErrSTAR(h);
    l = l->tail;
  }
  fprintf(stderr, "\n");
  dec_and_free((Value *)strings, 1);
  return;
}

int64_t countSeq(Value *seq) {
  Integer *len = (Integer *)count((FnArity *)0, seq);
  int64_t result = len->numVal;
  dec_and_free((Value *)len, 1);
  return(result);
}

Value *reifiedTypeArgs(Value *x) {
  if (x->type < CoreTypeCount) {
    dec_and_free(x, 1);
    return((Value *)empty_vect);
  } else {
    Vector *typeArgs = empty_vect;
    ReifiedVal *rv = (ReifiedVal *)x;
    for (int i = 0; i < rv->implCount; i++) {
      if (rv->impls[i] != (Value *)0) {
	typeArgs = mutateVectConj(typeArgs, incRef(rv->impls[i], 1));
      }
    }
    dec_and_free(x, 1);
    return((Value *)typeArgs);
  }
}

Vector *listVec(Value *list) {
  List *l = (List *)list;
  Vector *newVect = empty_vect;
  for(Value *x = l->head; x != (Value *)0; l = l->tail, x = l->head) {
    newVect = mutateVectConj(newVect, incRef(x, 1));
  }
  dec_and_free(list, 1);
  return(newVect);
}

Value *newTypeValue(int typeNum, Vector *fields) {
  Vector *vect ;
  if (fields->type == ListType) {
    incRef((Value *)fields, 1);
    vect = (Vector *)listVec((Value *)fields);
  }
  else {
    vect = fields;
  }
  ReifiedVal *rv = malloc_reified(vect->count);
  rv->type = typeNum;
  for (int i = 0; i < vect->count; i++) {
    rv->impls[i] = vect->tail[i];
    vect->tail[i] = (Value *)0;
  }
#ifdef SINGLE_THREADED
  rv->refs = refsInit;
#else
  __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
#endif
  if (fields->type == ListType) {
    dec_and_free((Value *)vect, 1);
  }
  dec_and_free((Value *)fields, 1);
  return((Value *)rv);
}
