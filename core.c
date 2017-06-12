
#include <stdlib.h>
#include "core.h"

Value *nothing = (Value *)0;

List *empty_list = &(List){ListType,-1,0,0,0};
Integer trueVal = {IntegerType, -1, 1};
Value* true = (Value *)&trueVal;

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


#define FREE_FN_COUNT 20
freeValFn freeJmpTbl[FREE_FN_COUNT] = {NULL,
				       &freeInteger,
				       &freeString
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

Value *pr_STAR(Value *str) {
  if (str->type == StringType) {
    fprintf(outstream, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    fprintf(outstream, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  } else {
    fprintf(stderr, "\ninvalid type for 'pr*'\n");
    abort();
  }
  dec_and_free(str, 1);
  return(true);
}

Value *numberValue(int64_t n) {
  Integer *numVal = malloc_integer();
  numVal->numVal = n;
  return((Value *)numVal);
};

Value *add_ints(Value *arg0, Value *arg1) {
  if (arg0->type != arg1->type) {
    fprintf(stderr, "\ninvalid types for 'add-numbers'\n");
    abort();
  } else {
    Value *numVal = numberValue(((Integer *)arg0)->numVal + ((Integer *)arg1)->numVal);
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
