
#include <stdlib.h>
#include "core.h"

void prefs(char *tag, Value *v) {
  printf("%s: %p %d\n", tag, v, v->refs);
}

#ifdef CHECK_MEM_LEAK
_Atomic long long malloc_count = 0;
_Atomic long long free_count = 0;
#endif

Value *nothing = (Value *)&(Maybe){MaybeType, -1, 0};
List *empty_list = &(List){ListType,-1,0,0,0};

Value *my_malloc(int64_t sz) {
#ifdef CHECK_MEM_LEAK
  atomic_fetch_add(&malloc_count, 1);
#endif
  Value *val = malloc(sz);
  if (sz > sizeof(Value))
    atomic_store(&val->refs, 1);
  return(val);
}

typedef struct {Value *head; uintptr_t aba;} FreeValList;

Value *removeFreeValue(_Atomic FreeValList *freeList) {
  FreeValList orig = atomic_load(freeList);
  FreeValList next = orig;
  Value *item = (Value *)0;
  if (orig.head != (Value *)0) {
    do {
      item = orig.head;
      next.head = item->next;
      next.aba = orig.aba + 1;
    } while (!atomic_compare_exchange_weak(freeList, &orig, next) &&
             orig.head != (Value *)0);
    if (orig.head == (Value *)0)
      item = (Value *)0;
  }

  if (item == (Value *)0) {
    return((Value *)0);
  } else {
    int32_t refs = atomic_load(&item->refs);
    if (refs != -10) {
      fprintf(stderr, "failure in removeFreeValue: %d\n", refs);
      abort();
    }
    return(item);
  }
}

_Atomic FreeValList centralFreeIntegers = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
__thread FreeValList freeIntegers = {(Value *)0, 0};
Integer *malloc_integer() {
  Integer *newInteger = (Integer *)freeIntegers.head;
  if (newInteger == (Integer *)0) {
    newInteger = (Integer *)removeFreeValue(&centralFreeIntegers);
    if (newInteger == (Integer *)0) {
      Integer *numberStructs = (Integer *)my_malloc(sizeof(Integer) * 100);
#ifdef CHECK_MEM_LEAK
      atomic_fetch_add(&malloc_count, 99);
#endif
      for (int i = 1; i < 99; i++) {
        atomic_store(&numberStructs[i].refs, -10);
        ((Value *)&numberStructs[i])->next = (Value *)&numberStructs[i + 1];
      }
      atomic_store(&numberStructs[99].refs, -10);
      ((Value *)&numberStructs[99])->next = (Value *)0;
      freeIntegers.head = (Value *)&numberStructs[1];

      numberStructs->type = IntegerType;
      atomic_store(&numberStructs->refs, 1);
      return(numberStructs);
    }
  } else {
    freeIntegers.head = freeIntegers.head->next;
  }
  newInteger->type = IntegerType;
  atomic_store(&newInteger->refs, 1);
  return(newInteger);
}

int decRefs(Value *v, int deltaRefs) {
  int32_t refs = atomic_load(&v->refs);
  if (refs == -1)
    return(refs);

  int32_t newRefs;
  do {
    if (refs < deltaRefs) {
      fprintf(stderr, "failure in decRefs: %d %p\n", refs, v);
      abort();
    } else if (refs == deltaRefs)
      newRefs = -10;
    else
      newRefs = refs - deltaRefs;
  } while (!atomic_compare_exchange_strong(&v->refs, &refs, newRefs));

  return(newRefs);
}

typedef void (*freeValFn)(Value *);

void freeInteger(Value *v) {
  v->next = freeIntegers.head;
  freeIntegers.head = v;
}

_Atomic FreeValList centralFreeStrings = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
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
  atomic_store(&str->refs, 1);
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
      atomic_fetch_add(&free_count, 1);
      // atomic_fetch_add(&free_strs, 1);
#endif
      free(v);
    }
  }

_Atomic FreeValList centralFreeSubStrings = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
__thread FreeValList freeSubStrings = {(Value *)0, 0};
SubString *malloc_substring() {
  SubString *subStr = (SubString *)freeSubStrings.head;
  if (subStr == (SubString *)0) {
    subStr = (SubString *)removeFreeValue(&centralFreeSubStrings);
    if (subStr == (SubString *)0) {
      subStr = (SubString *)my_malloc(sizeof(SubString));
      subStr->hash = (Integer *)0;
      atomic_store(&subStr->refs, 1);
      return(subStr);
    }
  } else {
    freeSubStrings.head = freeSubStrings.head->next;
  }
  atomic_store(&subStr->refs, 1);
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

_Atomic FreeValList centralFreeFnArities = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
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
  atomic_store(&newFnArity->refs, 1);
  return(newFnArity);
}

void freeFnArity(Value *v) {
  FnArity *arity = (FnArity *)v;
  dec_and_free((Value *)arity->closures, 1);
  v->next = freeFnArities.head;
  freeFnArities.head = v;
}

_Atomic FreeValList centralFreeFunctions[10] = {ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0})),
                                                ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}))};
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
    atomic_store(&((Function *)newFunction)->refs, 1);
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
    atomic_store(&((Function *)newFunction)->refs, 1);
    return((Function *)newFunction);
  }
}

void freeFunction(Value *v) {
  Function *f = (Function *)v;
  for (int i = 0; i < f->arityCount; i++) {
    dec_and_free((Value *)f->arities[i], 1);
  }
  // fprintf(stderr, \"%p freed\\n\", v);
  if (f->arityCount < 10) {
    v->next = freeFunctions[f->arityCount].head;
    freeFunctions[f->arityCount].head = v;
  } else {
#ifdef CHECK_MEM_LEAK
    atomic_fetch_add(&free_count, 1);
#endif
    free(v);
  }
}

_Atomic FreeValList centralFreeLists = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
__thread FreeValList freeLists = (FreeValList){(Value *)0, 0};
List *malloc_list() {
  List *newList = (List *)freeLists.head;
  if (newList == (List *)0) {
    newList = (List *)removeFreeValue(&centralFreeLists);
    if (newList == (List *)0) {
      List *listStructs = (List *)my_malloc(sizeof(List) * 100);
      for (int i = 1; i < 99; i++) {
        atomic_store(&listStructs[i].refs, -10);
        ((Value *)&listStructs[i])->next = (Value *)&listStructs[i + 1];
      }
      atomic_store(&listStructs[99].refs, -10);
      ((Value *)&listStructs[99])->next = freeLists.head;
      freeLists.head = (Value *)&listStructs[1];
#ifdef CHECK_MEM_LEAK
      atomic_fetch_add(&malloc_count, 99);
#endif

      listStructs->type = ListType;
      atomic_store(&listStructs->refs, 1);
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

_Atomic FreeValList centralFreeMaybes = ATOMIC_VAR_INIT(((FreeValList){(Value *)0, 0}));
__thread FreeValList freeMaybes = {(Value *)0, 0};
Maybe *malloc_maybe() {
  Maybe *newMaybe = (Maybe *)freeMaybes.head;
  if (newMaybe == (Maybe *)0) {
    newMaybe = (Maybe *)removeFreeValue(&centralFreeMaybes);
    if (newMaybe == (Maybe *)0) {
      Maybe *maybeStructs = (Maybe *)my_malloc(sizeof(Maybe) * 50);
      for (int i = 1; i < 49; i++) {
        atomic_store(&maybeStructs[i].refs, -10);
        ((Value *)&maybeStructs[i])->next = (Value *)&maybeStructs[i + 1];
      }
      atomic_store(&maybeStructs[49].refs, -10);
      ((Value*)&maybeStructs[49])->next = (Value *)0;
      freeMaybes.head = (Value *)&maybeStructs[1];
#ifdef CHECK_MEM_LEAK
      atomic_fetch_add(&malloc_count, 49);
#endif

      maybeStructs->type = MaybeType;
      atomic_store(&maybeStructs->refs, 1);
      return(maybeStructs);
    }
  } else {
    freeMaybes.head = freeMaybes.head->next;
  }
  newMaybe->type = MaybeType;
  atomic_store(&newMaybe->refs, 1);
  return(newMaybe);
}

void freeMaybe(Value *v) {
  Value *value = ((Maybe *)v)->value;
  dec_and_free(value, 1);
  v->next = freeMaybes.head;
  freeMaybes.head = v;
}

#define FREE_FN_COUNT 20
freeValFn freeJmpTbl[FREE_FN_COUNT] = {NULL,
				       &freeInteger,
				       &freeString,
				       &freeFnArity,
				       &freeFunction,
				       &freeSubString,
				       &freeList,
				       &freeMaybe
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

Value *incRef(Value *v, int deltaRefs) {
  if (deltaRefs < 1)
    return(v);

  int32_t refs = atomic_load(&v->refs);
  if (refs < -1) {
    fprintf(stderr, "failure in incRef: %d %p\n", refs, v);
    abort();
  }

  if (refs >= 0)
    atomic_fetch_add(&v->refs, deltaRefs);
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

void emptyFreeList(_Atomic FreeValList *freeLinkedList) {
  FreeValList listHead = atomic_load(freeLinkedList);
  for(Value *item = listHead.head;
      item != (Value *)0;
      item =  item->next) {
    atomic_fetch_add(&free_count, 1);
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
  // FreeValList listHead = atomic_load(&centralFreeFutures);
  // emptyFreeList(&centralFreePromises);
  // emptyFreeList(&centralFreeArrayNodes);
  emptyFreeList(&centralFreeSubStrings);
  emptyFreeList(&centralFreeFnArities);
  emptyFreeList(&centralFreeLists);
  emptyFreeList(&centralFreeMaybes);
  // emptyFreeList(&centralFreeVectors);
  // emptyFreeList(&centralFreeVectorNodes);
  emptyFreeList(&centralFreeStrings);
  emptyFreeList(&centralFreeIntegers);

//*
  fprintf(stdout, "malloc count: %lld  free count: %lld  diff: %lld\n",
          atomic_load(&malloc_count), atomic_load(&free_count),
          malloc_count - free_count);
// */
}
#endif

void moveToCentral(FreeValList *freeList, _Atomic FreeValList *centralList) {
  Value *tail = freeList->head;
  while (tail != (Value *)0 && tail->next != (Value *)0) {
    tail = tail->next;
  }

  if (tail == (Value *)0)
    return;
  else {
    FreeValList orig = atomic_load(centralList);
    FreeValList next = orig;
    do {
      tail->next = orig.head;
      next.head = freeList->head;
      next.aba = orig.aba + 1;
    } while (!atomic_compare_exchange_weak(centralList, &orig, next));
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
  // moveToCentral(&freeVectors, &centralFreeVectors);
  // moveToCentral(&freeVectorNodes, &centralFreeVectorNodes);
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
  snprintf(numStr->buffer, 9, "%lld", ((Integer *)arg0)->numVal);
  numStr->len = strlen(numStr->buffer);
  dec_and_free(arg0, 1);
  return((Value *)numStr);
}

Value *integer_EQ(Value *arg0, Value *arg1) {
  if (IntegerType != arg0->type != arg1->type) {
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
