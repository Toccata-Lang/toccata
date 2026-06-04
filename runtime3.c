
/*
  TODO: Possible major perf boost.
The only way values can be passed between threads is by a closure arity being passed to an agent or
a future. (Ponder this more).

In those cases, a special bit could be set that would cause any ref updates to be done atomically.
If the bit is not set, the ref update could be done immediately.
 */

#include <stdlib.h>
#include <stdatomic.h>
#include "runtime3.h"

FILE *dotFile;

REFS_SIZE refsInit = 1;
REFS_SIZE refsError = -10;
REFS_SIZE refsConstant = -1;
REFS_SIZE refsStatic = REFS_STATIC;

unsigned hght = HEIGHT;

Value *universalProtoFn = (Value *)0;
int cleaningUp = 0;

// Immutable hash-map ported from Clojure
BitmapIndexedNode emptyBMI = {BitmapIndexedType, -2, 0, 0};

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
  else
    __atomic_fetch_add(&type_frees[type], delta, __ATOMIC_ACQ_REL);
}

Vector empty_vect_struct = (Vector){VectorType,-2,0,0,5,0,0};
Vector *empty_vect = &empty_vect_struct;

ReifiedVal all_values_struct = {AllValuesType, -2, 0};
Value *all_values = (Value *)&all_values_struct;

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

typedef struct {Value *head; uintptr_t aba;} FreeValList __attribute__((aligned(16)));

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
      fprintf(stderr, "failure in removeFreeValue: %d %p\n", refs, item);
      abort();
    }
    return(item);
  }
#endif
}

int decRefs(Value *v, int deltaRefs) {
#ifndef FAST_DECS
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

/*
void decValuePtrRef(Value **ptr) {
  Value *toFree = (Value *)0;
  Value *oldPtr = (Value *)0;

  __atomic_exchange(ptr, &toFree, &oldPtr, __ATOMIC_RELAXED);
  if (oldPtr != (Value *)0) {
    dec_and_free((Value *)oldPtr, 1);
  }
}
// */

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
	memset(str->buffer, 0, STRING_RECYCLE_LEN + 4);
      }
    } else {
      freeStrings.head = freeStrings.head->next;
    }
  }
  // incTypeMalloc(StringBufferType, 1);
  // fprintf(stderr, "newStr %d: %p\n", __LINE__, (void *)str);
  str->refs = refsInit;
  str->hashVal = 0;
  str->type = StringBufferType;
  str->parent = NULL;
  str->len = len;
  return(str);
}

void freeString(Value *v) {
  // fprintf(stderr, "freeing string %d: %p\n", __LINE__, (void *)v);
  String *str = ((String *)v);
  int64_t len = str->len;
  if (str->parent != NULL)
    dec_and_free((Term)str->parent, 1);
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

FreeValList centralFreeTerms = (FreeValList){(Value *)0, 0};
__thread FreeValList freeTerms = {(Value *)0, 0};
TermVal *malloc_term() {
  TermVal *trm;
  trm = (TermVal *)freeTerms.head;
  if (trm == (TermVal *)0) {
    trm = (TermVal *)removeFreeValue(&centralFreeTerms);
    if (trm == (TermVal *)0) {
      trm = (TermVal *)my_malloc(sizeof(TermVal));
    }
  } else {
    freeTerms.head = freeTerms.head->next;
  }
  trm->refs = refsInit;
  trm->type = TermType;
  return(trm);
}

void freeTerm(Value *v) {
  // fprintf(stderr, "freeing term %d: %p\n", __LINE__, (void *)v);
  TermVal *trm = ((TermVal *)v);
  Term trmVal = take(trm->trmLoc);
  v->next = freeTerms.head;
  freeTerms.head = v;
  store_redex(ERA, trmVal);
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
  newFnArity->type = FnArityType;
  newFnArity->refs = refsInit;
  return(newFnArity);
}

void freeFnArity(Value *v) {
  FnArity *arity = (FnArity *)v;
  v->next = freeFnArities.head;
  freeFnArities.head = v;
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
    if (((VectorNode *)v)->array[i] != 0) {
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
  // fprintf(stderr, "newVector %d: %p\n", __LINE__, (void *)newVector);
  return(newVector);
}

void freeVector(Value *val) {
  // fprintf(stderr, "freeVector %d: %p\n", __LINE__, (void *)val);
  Vector *v = (Vector *)val;
  VectorNode *root = ((Vector *)v)->root;
  if (root != (VectorNode *)NULL) {
    dec_and_free((Term)root, 1);
  }

  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (((Vector *)v)->tail[i] != 0)
      dec_and_free(((Vector *)v)->tail[i], 1);
  }
  val->next = freeVectors.head;
  freeVectors.head = val;
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
    newReifiedVal = (ReifiedVal *)my_malloc(sizeof(ReifiedVal) + sizeof(FnArity *) * implCount);
  } else {
    newReifiedVal = (ReifiedVal *)freeReified[implCount].head;
    if (newReifiedVal == (ReifiedVal *)0) {
      newReifiedVal = (ReifiedVal *)removeFreeValue(&centralFreeReified[implCount]);
      if (newReifiedVal == (ReifiedVal *)0) {
	int rvSize = sizeof(ReifiedVal) + sizeof(FnArity *) * implCount;
	int rvCount = 5000;
	char *reifiedStructs = (char *)my_malloc(rvSize * rvCount);
	for (int i = 1; i < (rvCount - 1); i++) {
	  ReifiedVal *rv = (ReifiedVal *)&reifiedStructs[i * rvSize];
	  // fprintf(stderr, "rv %d: %p\n", __LINE__, rv);
	  rv->refs = refsError;
	  ((Value *)rv)->next = (Value *)&reifiedStructs[(i + 1) * rvSize];
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
  // fprintf(stderr, "newReified %d: %ld  %p\n", __LINE__, implCount, newReifiedVal);
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
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return;
  /*
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
  // */
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
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return;
  /*
  HashCollisionNode *node = (HashCollisionNode *)v;
  for (int i = 0; i < node->count; i++) {
    if (node->array[i] != 0) {
      dec_and_free(node->array[i], 1);
    }
  }
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  if (!cleaningUp)
    free(v);
  // */
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
    if (node->array[i] != 0) {
      dec_and_free(node->array[i], 1);
    }
  }
  v->next = freeArrayNodes.head;
  freeArrayNodes.head = v;
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
				       NULL,
				       &freeString,
				       &freeFnArity,
				       NULL,
				       &freeTerm,
				       NULL,
				       NULL,
				       &freeVector,
				       &freeVectorNode,
				       NULL,
				       &freeBitmapNode,
				       &freeArrayNode,
				       &freeHashCollisionNode,
				       NULL,
				       NULL,
				       NULL,
				       NULL,
				       &freeOpaquePtr};

void decValRef(Term pv, int deltaRefs) {
  Value *v;
  switch (term_tag(pv)) {
  case F60:
  case I60:
    break;

  case VAL:
    v = (Value *)pv;
    if (v == (Value *)0 ||
	v->refs == refsStatic ||
	v->refs == refsConstant ||
	decRefs(v, deltaRefs) >= refsConstant){
      return;
    }

    if (v->type < CoreTypeCount) {
      // incTypeFree(v->type, 1);
      // fprintf(stderr, "freeing core type: %d\n", __LINE__);
      freeJmpTbl[v->type](v);
    } else {
      ReifiedVal *rv = (ReifiedVal *)v;
      // fprintf(stderr, "freeing reified %d: %ld %p\n", __LINE__, rv->implCount, rv);
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
    break;

  default:
    fprintf(stderr, "HVM error %s:%d\n", __FILE__, __LINE__);
    abort();
    break;
  }
};

void dec_and_free(Term pv, int deltaRefs) {
  if (pv == 0) {
    return;
  }

  Value *v;
  switch (term_tag(pv)) {
  case F60:
  case I60:
    break;

  case VAL:
    decValRef(pv, deltaRefs);
    break;

  default:
    // fprintf(stderr, "freeing interaction combinator: %d %p\n", __LINE__, (void *)v);
    store_redex(ERA, (Term)pv);
    break;
  }
}

#ifndef FAST_INCS
Term incRef(Term val, int deltaRefs) {
  Tag t = term_tag(val);
  if (t == I60 || t == F60 || t == REF)
    return val;

  Value *v = (Value *)val;
  if (v == (Value *)NULL) {
    fprintf(stderr, "bad incRef value: %p\n", v);
    abort();
  }
  if (deltaRefs < 0) {
    fprintf(stderr, "bad deltaRefs: %p\n", v);
    abort();
  } else if (deltaRefs < 1)
    return(val);

  REFS_SIZE refs;
  __atomic_load(&v->refs, &refs, __ATOMIC_RELAXED);

  REFS_SIZE newRefs;
  do {
    if (refs == refsStatic || refs == refsConstant)
      return(val);

    if (refs < refsStatic) {
      fprintf(stderr, "failure in incRef: %d %p\n", refs, (void *)v);
      abort();
    }

    newRefs = refs + deltaRefs;
  } while (!__atomic_compare_exchange(&v->refs, &refs, &newRefs, 1,
				      __ATOMIC_RELAXED, __ATOMIC_RELAXED));
  return(val);
}
#else
Value *simpleIncRef(Value *v, int n) {
  v->refs += n;
  return(v);
}
#endif

void moveFreeToCentral() {
  for (int i = 0; i < BMI_RECYCLE_COUNT; i++) {
    moveToCentral(&freeBMINodes[i], &centralFreeBMINodes[i]);
  }
  for (int i = 0; i < 20; i++) {
    moveToCentral(&freeReified[i], &centralFreeReified[i]);
  }
  moveToCentral(&freeTerms, &centralFreeTerms);
  moveToCentral(&freeStrings, &centralFreeStrings);
  moveToCentral(&freeArrayNodes, &centralFreeArrayNodes);
  moveToCentral(&freeVectors, &centralFreeVectors);
  moveToCentral(&freeVectorNodes, &centralFreeVectorNodes);
  moveToCentral(&freeFnArities, &centralFreeFnArities);
}

void emptyFreeList(FreeValList *freeLinkedList) {
  FreeValList listHead;
#ifdef SINGLE_THREADED
  listHead = *freeLinkedList;
#else
  __atomic_load((FreeValList *)freeLinkedList, (FreeValList *)&listHead, __ATOMIC_RELAXED);
#endif
  for(Value *item = (Value *)listHead.head;
      item != (Value *)0;
      item =  item->next) {
#ifdef CHECK_MEM_LEAK
    __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
#endif
  }
}

void freeAll() {
  moveFreeToCentral();

  for (int i = 0; i < 20; i++) {
    emptyFreeList(&centralFreeReified[i]);
  }
  for (int i = 0; i < BMI_RECYCLE_COUNT; i++) {
    emptyFreeList(&centralFreeBMINodes[i]);
  }
  emptyFreeList(&centralFreeArrayNodes);
  emptyFreeList(&centralFreeFnArities);
  emptyFreeList(&centralFreeVectors);
  emptyFreeList(&centralFreeVectorNodes);
  emptyFreeList(&centralFreeStrings);
  emptyFreeList(&centralFreeTerms);

//*
#ifdef SINGLE_THREADED
  printf("\nmalloc count: %" PRId64 "  free count: %" PRId64 "  diff: %" PRId64 "\n",
          malloc_count, free_count, malloc_count - free_count);
  fprintf(stderr, "\nmalloc count: %" PRId64 "  free count: %" PRId64 "  diff: %" PRId64 "\n",
          malloc_count, free_count, malloc_count - free_count);
#else
  int64_t mallocs;
  __atomic_load(&malloc_count, &mallocs, __ATOMIC_RELAXED);
  int64_t frees;
  __atomic_load(&free_count, &frees, __ATOMIC_RELAXED);
  printf("malloc count: %" PRId64 "  free count: %" PRId64 "  diff: %" PRId64 "\n",
          mallocs, frees, mallocs - frees);
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
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return (0);
  /*
  Integer *hashVal;
  int64_t hash;
  switch (v1->type) {
  case IntegerType:
    hash = integerSha1(v1);
    break;

  case StringBufferType:
  case SubStringType:
    hash = strSha1(v1);
    break;

  case ListType:
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
  // */
}

char *extractStr(Value *v) {
  // Should only be used to print an error meessage when calling 'abort'
  // Leaks a String value
  if (v->type == StringBufferType) {
    return(((String *)v)->buffer);
  } else {
    fprintf(stderr, "\ninvalid type for 'extractStr'\n");
    abort();
  }
}

Value *prSTAR(Value *str) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  int bytes;
  if (str->type == StringBufferType) {
    bytes = fprintf(outstream, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    bytes = fprintf(outstream, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  }
  dec_and_free(str, 1);
  return(new_num(new_i24(bytes));
  // */
}

Value *defaultPrErrSTAR(Value *str) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  int bytes;
  if (str->type == StringBufferType) {
    bytes = fprintf(stderr, "%-.*s", (int)((String *)str)->len, ((String *)str)->buffer);
  } else if (str->type == SubStringType) {
    bytes = fprintf(stderr, "%-.*s", (int)((SubString *)str)->len, ((SubString *)str)->buffer);
  }
  dec_and_free(str, 1);
  return(new_num(new_i24(bytes));
  // */
}

Term number_str(Term arg0) {
  String *numStr = malloc_string(50);
  sprintf(numStr->buffer, "%ld", get_i60(arg0));
  return(term_val((Term)numStr));
}

Value *isInstance(Value *arg0, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  TYPE_SIZE typeNum = ((Integer *)arg0)->numVal;
  if (typeNum == arg1->type) {
     dec_and_free(arg1, 1);
     return(maybe((FnArity *)0, (Value *)0, arg0));
  } else if (StringBufferType == typeNum && SubStringType == arg1->type) {
     dec_and_free(arg1, 1);
     return(maybe((FnArity *)0, (Value *)0, arg0));
  // } else if (HashMapType == typeNum && (BitmapIndexedType == arg1->type ||
                                        // ArrayNodeType == arg1->type ||
                                        // HashCollisionNodeType == arg1->type)) {
     // dec_and_free(arg1, 1);
     // return(maybe((FnArity *)0, (Value *)0, arg0));
  } else {
     dec_and_free(arg0, 1);
     dec_and_free(arg1, 1);
     return(nothing);
  }
  // */
}

Term dupeVal(Term *v) {
  Tag t = term_tag(*v);
  if (t == I60 || t == F60)
    return *v;
  else if (t == VAL)
    return incRef(*v, 1);

  fprintf(stderr, "Compiler error at %s: %d\nt: %d %p\n", __FILE__, __LINE__, t, (void *)*v);
  abort();
}

Vector *newVector(Term array[], int indexToSkip) {
  Vector *ret = malloc_vector();
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (array[i] != 0 && i != indexToSkip) {
      ret->tail[i] = dupeVal(&array[i]);
    }
  }
  return(ret);
}

VectorNode *newVectorNode(Term array[], int indexToSkip) {
  VectorNode *ret = malloc_vectorNode();
  for (int i = 0; i < VECTOR_ARRAY_LEN; i++) {
    if (array[i] != 0 && i != indexToSkip) {
      ret->array[i] = array[i];
      incRef(array[i], 1);
    }
  }
  return(ret);
}

Term *arrayFor(Vector *v, unsigned index) {
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
    return((Term *)0);
  }
}

VectorNode *newPath(int level, VectorNode *node) {
  if (level == 0) {
    return(node);
  } else {
    VectorNode *ret = malloc_vectorNode();
    ret->array[0] = (Term)newPath(level - 5, node);
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
  ret->array[subidx] = (Term)nodeToInsert;
  return(ret);
}

Vector *vectConj(Vector *vect, Term val) {
  if (vect->refs == 1) {
    Vector *newVect = mutateVectConj(vect, val);
    return(newVect);
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
    if (newVect->root != (VectorNode *)NULL) {
      incRef((Term)newVect->root, 1);
    }

    // add value to tail of new vector
    newVect->tail[vect->count & 0x1F] = val;
    /*
    fprintf(stderr, "vectConj %d: %p %ld %p\n", __LINE__, (void *)vect,
	    get_i60(val), (void *)newVect);
    // */
    dec_and_free((Term)vect, 1);
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
      newRoot->array[0] = (Term)vect->root;
      incRef((Term)newRoot->array[0], 1);

      // and make a new path that includes that node
      newRoot->array[1] = (Term)newPath(vect->shift, tailNode);
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
    dec_and_free((Term)vect, 1);
    return(newVect);
  }
}

void vectConjFn(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term vect_1 = arityArgs.args[0];
    Term v_2 = arityArgs.args[1];

    result = term_val((Term)vectConj((Vector *)vect_1, v_2));
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term vectConjRef = new_ref(vectConjFn);

void vectMap(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  if (arityArgs.count == 2) {
    Vector *vect = (Vector *)arityArgs.args[0];
    Term f = arityArgs.args[1];
    Term newV = term_val((Term)empty_vect);
    if (vect->count == 0) {
      dec_and_free(f, 1);
      moveStore(port(2, term_loc(args)), (Term)vect);
    } else {
      incRef(f, vect->count - 1);
      for (unsigned i = 0; i < vect->count; i++) {
	Term mArgs = pair_make(APP, 0, vectGet(vect, i), SUB);
	swapStore(port(2, term_loc(mArgs)), pair_make(LAZ, 0, mArgs, f));
	Term cArgs1 = pair_make(APP, 0, term_new(VAR, 0, port(2, term_loc(mArgs))), SUB);
	Term cArgs2 = pair_make(APP, 0, newV, cArgs1);
	Term conjNode = pair_make(LAZ, 0, cArgs2, vectConjRef);
	swapStore(port(2, term_loc(cArgs1)), conjNode);
	newV = term_new(VAR, 0, port(2, term_loc(cArgs1)));
      }
      dec_and_free((Term)vect, 1);
      moveStore(port(2, term_loc(args)), newV);
    }
  }
  return;
}

Vector *mutateVectConj(Vector *vect, Term val) {
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
      vect->tail[i] = 0;
    }
    int newShift = vect->shift;

    // if the root of 'vect' is completely full
    if ((vect->count >> 5) > (1 << vect->shift)) {
      // make new vector one level deeper
      newRoot = malloc_vectorNode();
      newRoot->array[0] = (Term)vect->root;

      // and make a new path that includes that node
      newRoot->array[1] = (Term)newPath(vect->shift, tailNode);
      newShift += 5;
    } else {
      // make new vector one level deeper
      // otherwise, push the tail node down, creating a new root
      newRoot = pushTail(vect->count, vect->shift, vect->root, tailNode);
      if (vect->root != (VectorNode *)0)
        dec_and_free((Term)vect->root, 1);
    }
    vect->count += 1;
    vect->tailOffset = (vect->count - 1) & ~0x1f;
    vect->shift = newShift;
    vect->root = newRoot;
    vect->tail[0] = val;
    return(vect);
  }
}

void hvmVectFn(Term ref, Term args){
  NativeArgs arityArgs = {0, {}};
  Term newArgs = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    newArgs = take(port(2, term_loc(newArgs)));
    long vectLen = get_i60(arityArgs.args[0]);
    if (vectLen > MAX_ARGS)
      BOOM("too many items in vector literal");
    Term lastArgs = strictArgs(ref, newArgs, vectLen, &arityArgs);
    if (arityArgs.count == vectLen + 1) {
      Vector *newV = empty_vect;
      for (int i = 0; i < arityArgs.count; i++)
	newV = vectConj(newV, arityArgs.args[i + 1]);
      swapStore(port(2, term_loc(lastArgs)), term_val((Term)newV));
    }
  }
  return;
}
Term hvmVect = new_ref(hvmVectFn);

VectorNode *copyVectStore(int level, VectorNode *node, unsigned index, Term val) {
  if (level == 0) {
    int arrayIndex = index & 0x1f;
    VectorNode *newNode = newVectorNode(node->array, arrayIndex);
    newNode->array[arrayIndex] = val;
    return(newNode);
  } else {
    int arrayIndex = (index >> level) & 0x1f;
    VectorNode *newNode = newVectorNode(node->array, arrayIndex);
    newNode->array[arrayIndex] = (Term)copyVectStore(level - 5,
						     (VectorNode *)node->array[arrayIndex],
						     index, val);
    return(newNode);
  }
}

Term nothing() {
  ReifiedVal *rv = malloc_reified(0);
  rv->type = NoneType;
  __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
  return(term_val((Term)rv));
}

Term some(Term thing) {
  ReifiedVal *rv = malloc_reified(1);
  rv->type = SomeType;
  rv->impls[0] = thing;
  __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
  return(term_val((Term)rv));
}

Term vectStore(Vector *vect, unsigned index, Term val) {
  // TODO: check the refs count and mutate if equal 1
  // but only if all nodes 'above' this one are mutate-able
  // and if you do mutate this vect, clear the cached hash value (once that's implemented)
  if (index < vect->count) {
    if (index >= vect->tailOffset) {
      // storing into the tail
      unsigned newIndex = index & (VECTOR_ARRAY_LEN - 1);
      Vector *ret = newVector(vect->tail, newIndex);
      ret->tail[newIndex] = val;
      ret->count = vect->count;
      ret->tailOffset = vect->tailOffset;
      ret->shift = vect->shift;
      ret->root = vect->root;
      if (ret->root != (VectorNode *)0) {
        incRef((Term)(ret->root), 1);
      }
      dec_and_free((Term)vect, 1);
      return(some((Term)ret));
    } else {
      Vector *ret = newVector(vect->tail, VECTOR_ARRAY_LEN);
      ret->count = vect->count;
      ret->tailOffset = vect->tailOffset;
      ret->shift = vect->shift;
      ret->root = copyVectStore(vect->shift, vect->root, index, val);
      dec_and_free((Term)vect, 1);
      return(some((Term)ret));
    }
  } else if (index == vect->count) {
    Vector *ret = vectConj(vect, val);
    return(some((Term)ret));
  } else {
    dec_and_free((Term)vect, 1);
    dec_and_free(val, 1);
    return(nothing());
  }
}

#if 0
Vector *fastVectStore(Vector *vect, unsigned index, Term val) {
  if (index < vect->count &&
      index >= vect->tailOffset &&
      vect->refs == 1) {
    unsigned newIndex = index & 0x1f;
    dec_and_free(vect->tail[newIndex], 1);

    vect->tail[newIndex] = val;
    return(vect);
  } else {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Vector *)NULL);
  /*
    Vector *result = vectStore(vect, index, val);
    if (isNothing(result)) {
      fprintf(stderr, "*** Improper use of fastVectStore\n");
      abort();
    } else {
      Value *inner = ((Maybe *)result)->value;
      incRef(inner, 1);
      dec_and_free(result, 1);
      dec_and_free((Value *)vect, 1);
      return(inner);
    }
    // */
  }
}

ReifiedVal *updateField(ReifiedVal *rval, Term field, int64_t idx) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((ReifiedVal *)NULL);
  /*
  ReifiedVal *template = (ReifiedVal *)rval;
  if (idx >= template->implCount) {
    fprintf(stderr, "Field index for type '%s' out of bounds: %" PRId64 ". Max: %" PRId64 "\n",
	    extractStr(type_name((FnArity *)0, (Value *)rval)), idx, template->implCount);
    abort();
  }
  if (rval->refs == 1) {
    dec_and_free(template->impls[idx], 1);
    template->impls[idx] = field;

    return(rval);
  } else {
    ReifiedVal *rv = malloc_reified(template->implCount);
    int rvSize = sizeof(ReifiedVal) + sizeof(FnArity *) * template->implCount;
    memcpy(rv, template, rvSize);
    __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
    for (int i = 0; i < template->implCount; i++) {
      if (i != idx) {
        incRef(template->impls[i], 1);
      }
    }
    rv->impls[idx] = field;
    dec_and_free((Term)rval, 1);
    return(rv);
  }
  // */
}

Vector *vectorReverse(Vector *v) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Vector *)NULL);
  /*
  int i;
  Vector *newVect = empty_vect;
  for (i = v->count - 1; i >= 0; i--) {
    Term val = vectGet(v, i);
    incRef(val, 1);
    newVect = mutateVectConj(newVect, val);
  }
  dec_and_free((Term)v, 1);
  return(newVect);
  // */
}
#endif

Term vectGet(Vector *vect, unsigned index) {
  // this fn does not dec_and_free vect on purpose
  // it lets calling functions do that.
  Term *array = arrayFor(vect, index);
  return(dupeVal(&array[index & 0x1f]));
}

Term strEQ(Term sT, Term startT, Term lenT, Term tgtT) {
  String *str0 = (String *)sT; 
  char *s1, *s2;
  long start = get_i60(startT);
  int len = (int)get_i60(lenT);

  s1 = &str0->buffer[start];

  if (((Value *)tgtT)->type == StringBufferType) {
    String *str1 = (String *)tgtT; 
    if (len != str1->len)
      return(nothing());

    s2 = str1->buffer;
  } else if (((Value *)tgtT)->type == SubStringType) {
    ReifiedVal *str1 = (ReifiedVal *)tgtT;
    String *parent = (String *)str1->impls[0];
    long start = get_i60(str1->impls[1]);

    if ((int)get_i60(str1->impls[2]) != len)
      return(nothing());

    s2 = &parent->buffer[start];
  }

  if (strncmp(s1, s2, len) == 0) {
    if (start == 0) {
      dec_and_free(tgtT, 1);
      return(some(sT));
    } else {
      dec_and_free(sT, 1);
      return(some(tgtT));
    }
  } else {
    dec_and_free(sT, 1);
    dec_and_free(tgtT, 1);
    return(nothing());
  }
}

Value *strLT(Value *arg0, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  TYPE_SIZE typeNum = ((Integer *)arg0)->numVal;
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
    return(maybe((FnArity *)0, (Value *)0, arg0));
  } else {
    dec_and_free(arg0, 1);
    dec_and_free(arg1, 1);
    return(nothing);
  }
  // */
}

Term strCount(Term s) {
   String *str = (String *)((u64)s & ~7);
   Term numVal = new_i60(str->len);
   // dec_and_free(arg0, 1);
   return(numVal);
}

Value *checkInstance(TYPE_SIZE typeNum, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  TYPE_SIZE typeNum = ((Integer *)arg0)->numVal;
  if (typeNum == arg1->type) {
    return(maybe((FnArity *)0, (Value *)0, arg1));
  } else if (StringBufferType == typeNum && SubStringType == arg1->type) {
    return(maybe((FnArity *)0, (Value *)0, arg1));
  } else if (HashMapType == typeNum && (BitmapIndexedType == arg1->type ||
                                        ArrayNodeType == arg1->type ||
                                        HashCollisionNodeType == arg1->type)) {
    return(maybe((FnArity *)0, (Value *)0, arg1));
  } else {
    dec_and_free(arg1, 1);
    return(nothing);
  }
  // */
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

long finalize_sha1(Value *ctxt) {
  int64_t shaVal;
  Sha1Finalise(((Opaque *)ctxt)->ptr, (SHA1_HASH *)&shaVal);
  dec_and_free((Term)ctxt, 1);
  return(shaVal);
}

int64_t integerSha1(Value *arg0) {
  int64_t shaVal;
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  /*
  Sha1Context context;
  Integer *numVal = (Integer *)arg0;

  Sha1Initialise(&context);
  Sha1Update(&context, (void *)&numVal->type, 8);
  Sha1Update(&context, (void *)&numVal->numVal, 8);
  Sha1Finalise(&context, (SHA1_HASH *)&shaVal);
  dec_and_free(arg0, 1);
  // */
  return(shaVal);
}

Term integer_EQ(Term arg0, Term arg1) {
  i64 x = get_i60(arg0);
  i64 y = get_i60(arg1);

  if (x != y) {
    return(nothing());
  } else {
    return(some(arg0));
  }
}

Term integer_LT(Term arg0, Term arg1) {
  i64 x = get_i60(arg0);
  i64 y = get_i60(arg1);

  Term result;

  if (x >= y) {
    result = nothing();
  } else {
    result = some(arg0);
  }
  // printf("integer_LT x: %ld  y: %ld  type: %ld\n", x, y, ((Value *)result)->type);
  return result;
}

int8_t equal(Value *v1, Value *v2) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return (0);
  /*
  Value *equals;
  switch (v1->type) {
  case IntegerType:
    equals = integer_EQ(v1, v2);
    break;
  default:
    equals = equalSTAR((FnArity *)0, v1, v2);
    break;
  }
  int8_t notEquals = isNothing(equals);
  dec_and_free(equals, 1);
  return(!notEquals);
  // */
}

Value *stringValue(char *s) {
  int64_t len = strlen(s);
  String *strVal = malloc_string(len + 5);
  strncpy(strVal->buffer, s, len);
  strVal->buffer[len] = 0;
  return((Value *)strVal);
};

Value *opaqueValue(void *ptr, Destructor *destruct) {
  Opaque *opVal = (Opaque *)my_malloc(sizeof(Opaque));
  // incTypeMalloc(OpaqueType, 1);
  opVal->type = OpaqueType;
  opVal->ptr = ptr;
  opVal->destruct = destruct;
  return((Value *)opVal);
};

Term vectorGet(Term v, Term n) {
  long index = get_i60(n);
  Vector *vect = (Vector *)((u64)v & ~7);
  if (index < 0 || vect->count <= index) {
    dec_and_free(v, 1);
    return(nothing());
  } else {
    Term val = vectGet(vect, index);
    dec_and_free(v, 1);
    return(some(val));
  }
}

BitmapIndexedNode *clone_BitmapIndexedNode(BitmapIndexedNode *node, int idx,
                                           Value *key, Value* val)
{
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((BitmapIndexedNode *)NULL);
  /*
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
  // */
}

Value *createNode(int shift,
		  int64_t key1hash, Value *key1, Value *val1,
		  int64_t key2hash, Value *key2, Value *val2)
{
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *bmiHashVec(Value *arg0, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *bmiCount(Value *arg0) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  return(new_num(new_i24(accum));
  // */
}

Value *bmiCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *bmiMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *bmiGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash,  int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *bmiDissoc(Value *arg0, Value* arg1, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *arrayNodeMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *collisionAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value notFound = {0, -2};
Value *notFoundPtr = &notFound;

Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *arrayNodeCount(Value *arg0) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  int accum = 0;
  for(int i = 0; i < ARRAY_NODE_LEN; i++){
    if (((ArrayNode *)arg0)->array[i] != (Value *)0) {
      Integer *subCnt = (Integer *)count((FnArity *)0, incRef(((ArrayNode *)arg0)->array[i], 1));
      accum += subCnt->numVal;
      dec_and_free((Value *)subCnt, 1);
    }
  }
  dec_and_free(arg0, 1);
  return(new_num(new_i24(accum));
  // */
}

Value *collisionCount(Value *arg0) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  Value *result = new_num(new_i24(((HashCollisionNode *) arg0)->count / 2);
  dec_and_free(arg0, 1);
  return(result);
  // */
}

Value *collisionVec(Value *arg0, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *collisionDissoc(Value *arg0, Value *arg1, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *collisionGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
}

Value *arrayNodeVec(Value *arg0, Value *arg1) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  ArrayNode *node = (ArrayNode *)arg0;
  Vector *vec = (Vector *)arg1;
  for (int i = 0; i < ARRAY_NODE_LEN; i++) {
    if (node->array[i] != (Value *)0) {
      incRef(node->array[i], 1);
      vec = (Vector *)hashVec(node->array[i], (Value *)vec);
    }
  }
  dec_and_free((Term)node, 1);
  return((Term)vec);
  // */
}

Value *arrayNodeDissoc(Value *arg0, Value *arg1, int64_t hash, int shift) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  ArrayNode *node = (ArrayNode *)arg0;
  Value *key = arg1;
  int idx = mask(hash, shift);
  ArrayNode *newNode;

  Term subNode = node->array[idx];
  if (subNode == 0) {
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
      dec_and_free((Term)arg0, 1);
  }
  return((Value *)newNode);
  // */
}

/*
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
// */

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
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
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
  // */
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
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  TYPE_SIZE typeNum = ((Integer *)arg0)->numVal;
  int64_t hash = nakedSha1(incRef(arg1, 1));
  Value *found = get((FnArity *)0, arg0, arg1, notFoundPtr, hash, 0);
  if (found == notFoundPtr) {
    return(nothing);
  } else {
    return(maybe((FnArity *)0, (Value *)0, found));
  }
  // */
}

/*
// used for static encoding hash maps and other things
Value *hashMapAssoc(Value *arg0, Value *arg1, Value *arg2) {
  int64_t hash = nakedSha1(incRef(arg1, 1));
  return(mutateAssoc(arg0, arg1, arg2, hash, 0));
}

void freeExtractCache(void *cachePtr) {
  extractCache *cacheTail = (extractCache *)cachePtr;
  if (cacheTail != (extractCache *)0) {
    dec_and_free((Term)cacheTail->tail, 1);
    if (!cleaningUp)
      free(cacheTail);
#ifdef CHECK_MEM_LEAK
      __atomic_fetch_add(&free_count, 1, __ATOMIC_ACQ_REL);
      // incTypeFree(14, 1);
#endif
   }
}
// */

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
  dec_and_free((Term)s, 1);
  return(arg0Str);
}

/*
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
  for (Term h = l->head; l != (List *)NULL && h != 0; h = l->head) {
    incRef(h, 1);
    prErrSTAR(h);
    l = l->tail;
  }
  fprintf(stderr, "\n");
  dec_and_free((Value *)strings, 1);
  return;
}
// */

int64_t countSeq(Value *seq) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return (0);
  /*
  Integer *len = (Integer *)count((FnArity *)0, seq);
  int64_t result = len->numVal;
  dec_and_free((Value *)len, 1);
  return(result);
  // */
}

Value *reifiedTypeArgs(Term x) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  if (((Value *)x)->type < CoreTypeCount) {
    dec_and_free(x, 1);
    return((Value *)empty_vect);
  } else {
    Vector *typeArgs = empty_vect;
    ReifiedVal *rv = (ReifiedVal *)x;
    for (int i = 0; i < rv->implCount; i++) {
      if (rv->impls[i] != 0) {
	typeArgs = mutateVectConj(typeArgs, incRef(rv->impls[i], 1));
      }
    }
    dec_and_free(x, 1);
    return((Value *)typeArgs);
  }
  // */
}

Value *newTypeValue(int typeNum, Vector *vect) {
  ReifiedVal *rv = malloc_reified(vect->count);
  rv->type = typeNum;
  for (int i = 0; i < vect->count; i++) {
    rv->impls[i] = vect->tail[i];
    vect->tail[i] = 0;
  }
  __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
  dec_and_free((Term)vect, 1);
  return((Value *)rv);
}

Term dupeGlobal(Location glbl) {
  Term duper = pair_make(DUP, 0, SUB, SUB);
  Term gTerm = swapStore(glbl, term_new(VAR, 0, port(1, term_loc(duper))));

  switch(term_tag(gTerm)) {
  case VAL:
  case I60:
  case F60:
  case NUL:
  case REF:
    swapStore(glbl, gTerm);
    store_redex(duper, gTerm);
    break;

  case LAM:
  case SUP:
    store_redex(duper, gTerm);
    break;

  case VAR:
    if (1) {
      Term varTerm = swapStore(term_loc(gTerm), duper);
      varTerm = take(term_loc(varTerm));
      switch(term_tag(varTerm)) {
      case VAL:
      case I60:
      case F60:
      case NUL:
      case REF:
	swapStore(glbl, varTerm);
	store_redex(duper, varTerm);
	break;

      case LAM:
      case SUP:
	store_redex(duper, varTerm);
	break;

      case SUB:
	break;
	
      default:
	if (1) {
	  char s[50];
	  sprintf(s, "bad global tag %s", tag_to_str(term_tag(varTerm)));
	  BOOM(s);
	}
      }
    }
     
    break;

  case LAZ:
  default:
    if (1) {
      char s[50];
      sprintf(s, "bad global tag %s", tag_to_str(term_tag(gTerm)));
      BOOM(s);
    }
  }
  return term_new(VAR, 0, port(2, term_loc(duper)));
}

void constructFn(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  Term newArgs = strictArgs(ref, args, 2, &arityArgs);
  if (arityArgs.count == 2) {
    newArgs = take(port(2, term_loc(newArgs)));
    int typeNum = get_i60(arityArgs.args[0]);
    int numArgs = get_i60(arityArgs.args[1]);
    Term lastArgs = strictArgs(ref, newArgs, numArgs, &arityArgs);
    if (arityArgs.count == numArgs + 2) {
      ReifiedVal *rv = malloc_reified(numArgs);
      rv->type = typeNum;
      for (int i = 0; i < numArgs; i++) {
	Term field = arityArgs.args[i + 2];
	// fprintf(stderr, "field val %d: %d %p\n", __LINE__, term_tag((Term)field), field); 
	rv->impls[i] = field;
      }
      __atomic_store(&rv->refs, &refsInit, __ATOMIC_RELAXED);
      moveStore(port(2, term_loc(lastArgs)), term_val((Term)rv));
    }
  }
  return;
}
Term construct = new_ref(constructFn);

void accessFieldFn(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  if (arityArgs.count == 2) {
    int fldIdx = get_i60(arityArgs.args[0]);
    ReifiedVal *value = (ReifiedVal *)arityArgs.args[1];
    Term fld = value->impls[fldIdx];
    incRef(fld, 1);
    dec_and_free((Term)value, 1);
    moveStore(port(2, term_loc(args)), (Term)fld);
    /*
      args = take(port(2, term_loc(args)));
      Tag argsTag = term_tag(args);
      switch(argsTag) {
      case APP: {
      Term lastArgs = strictArgs(ref, args, 1, &arityArgs);
      if (arityArgs.count == 3) {
      int fldIdx = get_i60(arityArgs.args[0]);
      ReifiedVal *value = (ReifiedVal *)arityArgs.args[1];
      value->impls[fldIdx] = arityArgs.args[2];
      swapStore(port(2, term_loc(lastArgs)), term_val((Term)value));
      }
      }
      break;

      case SUB: {
      }
      break;

      // TODO: what other tags need to be handled
      default: {
      char s[50];
      sprintf(s,"unhandled tag %s (%0d) line: %d\n", tag_to_str(argsTag), argsTag, __LINE__);
      BOOM(s);
      }
      break;
      }
      // */
  }
  return;
}
Term accessField = new_ref(accessFieldFn);

char *typeName(unsigned typeNum) {
  for (unsigned i = 0; i < typeCount; i++) {
    if (typeNames[i].typeNum == typeNum) {
      printf("typeName: %s\n", typeNames[i].name);
      return typeNames[i].name;
    }
  }
  return "<unknown>";
}

void freeGlobal(Term p) {
  // fprintf(stderr, "glbl: %d %p\n", __LINE__, (void *)p);
  interact(ERA, p);
}

Location resultLocation;
u64 start;
int bashResult = 1;
u64 node_count = 1;

void exitProg(Term ref, Term args) {
  graphDown("exitProg", args, 0, subGraphs++);
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    Term result = arityArgs.args[0];
    Tag resultTag = term_tag(result);
    switch (resultTag) {
    case I60:
      bashResult = (int)get_i60(result);
      printf("result: %p bashResult: %d\n", (void *)result, bashResult);
      break;

    case F60:
      // TODO: handle
      BOOM("can't return a float as a result");
      break;
	
    case VAL:
      fprintf(stderr, "val: %ld\n", ((Value *)result)->type);
      dec_and_free(result, 1);
      result = new_i60(0);
      break;

    default: {
      fprintf(stderr, "bad result %s (%d) pair\n", tag_to_str(resultTag), resultTag);
      graphDown("result", result, 0, subGraphs++);
      exit(1);
    }
      break;
    }

    double duration = (time64() - start) / 1000000000.0; // seconds
    u64 itrs = atomic_load(&rdxCount);
    printf("- Threads: %u\n", threadCount);
    printf("- ITRS: %" PRIu64 "\n", itrs);

    Term neg, pos;
    while (pop_redex(&neg, &pos)) {
      graphDown("NEG", neg, 0, subGraphs++);
      graphDown("POS", pos, nodeCount, subGraphs++);
      interact(neg, NUL);
      interact(ERA, pos);
    }
    node_count = atomic_load(&glblAlloced);
    u64 max_node = atomic_load(&RNOD_END);
    // printf("- ITRS: %" PRIu64 " TIME: %.2fs  MIPS: %.2f\n", itrs, duration, (double)itrs / duration / 1000000.0);
    if (node_count != 0) {
      fprintf(stderr, "remaining nodes: %ld (%ld)\n", node_count, max_node);
      check_buff();
    }

    fprintf(dotFile, "}\n");
    fclose(dotFile);

#ifdef CHECK_MEM_LEAK
    freeGlobals();
    cleaningUp = 1;
    freeAll();
    if (malloc_count - free_count != 0 || node_count != 0)
      exit(1);
#endif
    hvm_free();
    /*
      free_static_tms();
      free(globalNet);
      // */

    exit(bashResult);

  }
  return;
}
Term exitRef = new_ref(exitProg);

Term valTerm(Term val) {
  // ensure a Term is a valid native value
  unsigned type = ((Value *)val)->type;
  if (val & VAL_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__);
    fprintf(stderr, "val: %p\n", (void *)val);
    abort();
  }
  return val;
}

Term argsNet(NativeArgs *args) {
  Term tail;
  if(args->count < 1)
    BOOM("argsNet");
  else
    tail = args->args[args->count - 1];

  for (int i = args->count - 2; i >= 0; i--) {
    tail = makePair(APP, 0, args->args[i], tail);
  }

  return tail;
}

void varArg(Term trm, Term ref, Term args, NativeArgs *argsStruct) {
  Location trmLoc = termLoc(trm);
  Term val = get(trmLoc);
  switch(termTag(val)) {
  case LAZ:
    swap(trmLoc, SUB);
    forceLazy(val);
	
  case SUB:
    // add the remaining args to argsStruct
    argsStruct->args[argsStruct->count++] = args;

    // create a chain of APP terms from argsStruct
    Term newArgs = argsNet(argsStruct);

    // put 'trm' back in it's place
    swap(portLoc(1, args), trm);

    // make a deferred redex to retry the APP/REF pair when the value becomes available
    Term retry = makePair(SUB, 5, newArgs, ref);

    // and put it in the location 'trm' points to
    Term newArg = swap(trmLoc, retry);
    if (newArg != SUB) {
      // someone slipped the needed trm in since we last looked
      swap(trmLoc, newArg);
      freePair(termLoc(retry));

      // so retry the original APP/REF redex
      pushRedex(newArgs, ref);
    }
    break;

  default:
    printRawTerm(val);
    printf("\n");
    BOOM("nativeArgs");
    break;
  }
}

// extract the requested number of native args. I60, F60, REF or VAL terms
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct) {
  /*
  char *refName = NULL;
  for (unsigned i = 0; i <= refsCount; i++) {
    if (refNames[i].fn == (interactionFn)(ref & ~TAG_MASK)) {
      refName = refNames[i].name;
      break;
    }
  }
  if (expected == 1) {
    if (refName != NULL) {
      char msg[200];
      sprintf(msg, "%s %03x:", refName, termLoc(args));
      graphDown(msg, args);
    } else {
      graphDown("unknown", args);
    }
    // if (strcmp(refName, "str-eq") == 0) {
    // printTerm("str-eq args", args);
    // }
  }
  // */
  Tag argsTag = termTag(args);
  if (argsTag == APP || argsTag == OPY) {
    // if 'args' is an APP term
    Term arg = take(portLoc(1, args));
    if (expected == 0) {
      return args;
    }

    // 'arg' will only ever be a positive term
    Tag argTag = termTag(arg);
    switch(argTag) {
      // the strict arg types
    case VAL:
    case I60:
    case F60:
    case REF:
      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = arg;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(portLoc(2, args)), expected - 1, argsStruct);
      else
	return args;
      break;

    case LAM: {
      TermVal *tv = malloc_term();
      tv->trmLoc = allocPair();
      swap(tv->trmLoc, arg);

      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = (Term)tv;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(portLoc(2, args)), expected - 1, argsStruct);
      else
	return args;
    }
      break;

    case NUL:
      move(portLoc(2, args), NUL);
      for (int i = 0; i < argsStruct->count; i++)
	dec_and_free(argsStruct->args[i], 1);
      break;

    case SUP:
      for(int i = 0; i < argsStruct->count; i++)
	incRef(argsStruct->args[i], 1);
      Term s1 = take(portLoc(1, arg));
      Term s2 = take(portLoc(2, arg));
      Lab supLabel = termLab(arg);
      int argsCount = argsStruct->count;
      argsStruct->count += 1;

      Term tail1 = makePair(APP, 0, s1, SUB);
      argsStruct->args[argsCount] = tail1;
      swap(portLoc(2, tail1), makePair(LAZ, 0, argsNet(argsStruct), ref));

      Term tail2 = makePair(APP, 0, s2, SUB);
      argsStruct->args[argsCount] = tail2;
      swap(portLoc(2, tail2), makePair(LAZ, 0, argsNet(argsStruct), ref));

      Term newSup = makePair(SUP, supLabel,
			      newTerm(VAR, 0, portLoc(2, tail1)),
			      newTerm(VAR, 0, portLoc(2, tail2)));
      move(portLoc(2, args), newSup);
      break;

    case VAR:
      varArg(arg, ref, args, argsStruct);
      break;

    case LAZ:
    default:
      fprintf(stderr, "unhandled tag %s (0x%x) line: %d\n", tagStr(termTag(arg)),
	     termTag(arg), __LINE__);
      fprintf(dotFile, "}\n");
      fclose(dotFile);
      abort();
      break;
    }
    argsStruct->count = -1;
    return 0;
    // } else if (argsTag == VAR) {
    // if 'args' is a VAR term
  } else {
    fprintf(stderr, "strictArgs expected: %d\n", expected);
    printTerm("strictArgs args", args);
    fprintf(stderr, "unhandled tag %s (0x%x) %p line: %d\n",
	   tagStr(argsTag), argsTag, (void *)args, __LINE__);
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    abort();
    return 0;
  }

}

Term dupeArg(Term arg, Term *dupedArg, unsigned dupLabel) {
  switch(termTag(arg)) {
  case VAL:
    *dupedArg = incRef(arg, 1);
    return arg;
    break;

  case F60:
  case I60:
  case REF:
    *dupedArg = arg;
    return arg;
    break;

  default: {
    Term newDup = makePair(DUP, dupLabel, SUB, SUB);
    Term z = makePair(LAZ, 0, newDup, arg);
    swap(portLoc(1, newDup), z);
    swap(portLoc(2, newDup), z);

    *dupedArg = newTerm(VAR, 0, portLoc(2, newDup));
    return newTerm(VAR, 0, portLoc(1, newDup));
  }
    break;
  }
}

void intCond(Term ref, Term args) {
  NativeArgs argsStruct = {0, {}};
  args = strictArgs(ref, args, 1, &argsStruct);
  if (argsStruct.count != 1) {
    return;
  }

  args = take(portLoc(2, args)); 
  Term trueBranch = take(portLoc(1, args)); 

  if (termTag(trueBranch) == VAR) {
    varArg(trueBranch, ref, args, &argsStruct);
    return;
  }

  args = take(portLoc(2, args)); 
  Term falseBranch = take(portLoc(1, args)); 

  if (termTag(falseBranch) == VAR) {
    varArg(falseBranch, ref, args, &argsStruct);
    return;
  }

  long x = getI60(argsStruct.args[0]);
  if (x == 0) {
    interact(ERA, trueBranch);
    move(portLoc(2, args), falseBranch);
  } else {
    interact(ERA, falseBranch);
    move(portLoc(2, args), trueBranch);
  }
}

int main (int argc, char **argv) {
  Term alts[200];
  unsigned altsCount = 0;
  
  prErrSTAR = &defaultPrErrSTAR;
#ifdef SINGLE_THREADED
#ifdef CHECK_MEM_LEAK
  fprintf(stderr, "Cannot use SINGLE_THREADED (or TOCCATA_WASM) and CHECK_MEM_LEAK   at same time.");
  abort();
#endif
#endif
  outstream = stdout;

  dotFile = fopen("graphs.dot", "w");
  fprintf(dotFile, "graph grammar {\nranksep=0.1\n");

  hvm_init(1024 * 1024 * 1024);
  hvm_reset();

  start = time64();

  int bashResult;
  Term result;

  // TM *tm = tms[0];

  // normalize the net 'iterations' times
  for (int iterations = 0; iterations < 1; iterations++) {
    normGlobals();
    Vector *argVect = empty_vect;
    for(int i = 0; i < argc; i++) {
      Value* sv = stringValue(argv[i]);
      argVect = mutateVectConj(argVect, term_val((Term)sv));
    }
    bashResult = 0;
    Term callArgs;
    callArgs = pair_make(APP, 0, term_val((Term)argVect), SUB);
    // fprintf(stderr, "argVect %d: %p\n", __LINE__, (void *)argVect);
    resultLocation = port(2, term_loc(callArgs));
    // fprintf(stderr, "resultLocation: %0x\n", resultLocation);
    store_redex(callArgs, mainFn);

    Tag resultTag;
    do {
      normalize(NULL);
      result = take(resultLocation);
      graphDown("result", result, 0, subGraphs++);
      resultTag = term_tag(result);
      if (resultTag == VAR) {
	resultLocation = term_loc(result);
	result = get(resultLocation);
	freeLoc(resultLocation);
	resultTag = term_tag(result);
      }

      // printf("result %d:\n", __LINE__);
      // print_raw_term(result);
      // printf("\n");
      switch (resultTag) {
      case I60:
      case F60:
	break;
	
      case NUL:
      case ERA:
	if (altsCount > 0) {
	  result = alts[--altsCount];
	  resultTag = term_tag(result);
	  swapStore(resultLocation, result);
	} else {
	  result = new_i60(0);
	  resultTag = I60;
	}
	break;
	
      case SUB:
	if (result != SUB) {
	  Term neg = take(port(1, term_loc(result)));
	  Term pos = take(port(2, term_loc(result)));
	  store_redex(neg, pos);
	} else if (altsCount > 0) {
	  result = alts[--altsCount];
	  resultTag = term_tag(result);
	  swapStore(resultLocation, result);
	} else {
	  // graphDown("BOOM", result);
	  print_term("result", result);
	  BOOM("Compiler screwed up. Incomplete result.");
	}
	break;

      case LAZ:
	swapStore(resultLocation, SUB);
	forceLazy(result);
	break;

      case VAL:
	dec_and_free(result, 1);
	result = new_i60(0);
	resultTag = I60;
	break;

      case SUP: {
	Lab l = term_lab(result);
	Location loc = term_loc(result);
	fprintf(stderr, "bad result %s (%d) pair\n", tag_to_str(resultTag), resultTag);
	graphDown("SUP result", result, 0, subGraphs++);
	Term newResult = take(port(2, loc)) ;
	alts[altsCount++] = newResult;
	graphDown("newResult", newResult, 0, subGraphs++);
	newResult = take(port(1, loc)) ;
	graphDown("newResult", newResult, 0, subGraphs++);
	result = newResult;
	resultTag = term_tag(result);
	swapStore(resultLocation, result);
      }
	break;

      default: {
	fprintf(stderr, "bad result %s (%d) pair\n", tag_to_str(resultTag), resultTag);
	graphDown("result", result, 0, subGraphs++);
	interact(ERA, result);
	result = new_i60(1);
	resultTag = term_tag(result);
      }
	break;
      }
    } while(resultTag != I60 && resultTag != F60 && resultTag != VAL);
  }
  for (int i = 0; i < altsCount; i++) {
    Term alt = alts[i];
    graphDown("alt", alt, 0, subGraphs++);
    print_term("alt", alt);
    if (hasLocation(alt))
      eraseCycle(alt, term_loc(alt));
    interact(ERA, alts[i]);
  }
  freeGlobals();

  double duration = (time64() - start) / 1000000000.0; // seconds
  u64 itrs = atomic_load(&rdxCount);
  u64 node_count = atomic_load(&glblAlloced);
  u64 max_node = atomic_load(&RNOD_END);
  printf("- Threads: %u\n", threadCount);
  printf("- ITRS: %" PRIu64 "\n", itrs);
  // printf("- ITRS: %" PRIu64 " TIME: %.2fs  MIPS: %.2f\n", itrs, duration, (double)itrs / duration / 1000000.0);
  printf("remaining nodes: %ld (%ld)\n", node_count, max_node);
  if (node_count != 0) {
    fprintf(stderr, "remaining nodes: %ld (%ld)\n", node_count, max_node);
    check_buff();
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    exit(1);
  }
  fprintf(dotFile, "}\n");
  fclose(dotFile);

  Tag t = term_tag(result);
  if (t == I60) {
    bashResult = (int)get_i60(result);
    printf("result: %p bashResult: %d\n", (void *)result, bashResult);
  } else if (t == F60) {
    // TODO: handle
    BOOM("can't return a float as a result");
  } else if (term_tag(result) == VAL) {
    result = (u64)result & ~7;
    printf("result %d:  %p\n", __LINE__, (void *)result);
    dec_and_free(result, 1);
  }
#ifdef CHECK_MEM_LEAK
  cleaningUp = 1;
  freeAll();
  if (malloc_count - free_count != 0 || node_count != 0)
    return(1);
#endif
  hvm_free();
/*
  free_static_tms();
  free(globalNet);
// */
  return(bashResult);
}
