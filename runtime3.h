
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "new.h"

#define CLOSURE_INFO 1

extern void abort();

// make sure VECTOR_ARRAY_LEN is always a power of 2
#define VECTOR_ARRAY_LEN 32
// make sure ARRAY_NODE_LEN is always a power of 2
#define ARRAY_NODE_LEN 32

#ifdef TOCCATA_WASM
#define SINGLE_THREADED 1
#endif

#ifndef TYPE_SIZE
#define TYPE_SIZE int64_t
#endif

#ifndef REFS_SIZE
#define REFS_SIZE int32_t
#endif

#define REFS_STATIC -2

typedef struct {
  uint32_t State[5];
  uint32_t Count[2];
  uint8_t Buffer[64];
} Sha1Context;

typedef void(Destructor)(void *);
// TODO: add hash cache and meta data. And update 'make-static-*' as well
typedef struct Value {
  TYPE_SIZE type;
  REFS_SIZE refs;
  struct Value *next;
} Value;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  Location trmLoc;
  char padding[sizeof(Value *) - sizeof(Location)];
} TermVal;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  Value *parent;
  int64_t len;
  char buffer[0];
} String;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  Term array[VECTOR_ARRAY_LEN];
} VectorNode;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  int32_t count;
  int8_t shift;
  int64_t tailOffset;
  VectorNode *root;
  Term tail[VECTOR_ARRAY_LEN];
} Vector;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int count;
  int variadic;
  interactionFn fn;
  Value *parent;
} FnArity;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  int32_t bitmap;
  Value *array[];
} BitmapIndexedNode;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  Term array[ARRAY_NODE_LEN];
} ArrayNode;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  int16_t count;
  Value *array[];
} HashCollisionNode;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t hashVal;
  int64_t implCount;
  int64_t padding;
  Term impls[];
} ReifiedVal;
typedef struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  void *ptr;
  Destructor *destruct;
} Opaque;

typedef struct {
  TYPE_SIZE type;
  Value *implFn;
} ProtoImpl;
typedef struct {
  int64_t implCount;
  ProtoImpl impls[];
} ProtoImpls;

typedef Value *(FnType0)(FnArity *);
typedef Value *(FnType1)(FnArity *, Value *);
typedef Value *(FnType2)(FnArity *, Value *, Value *);
typedef Value *(FnType3)(FnArity *, Value *, Value *, Value *);
typedef Value *(FnType4)(FnArity *, Value *, Value *, Value *, Value *);
typedef Value *(FnType5)(FnArity *, Value *, Value *, Value *, Value *,
                         Value *);
typedef Value *(FnType6)(FnArity *, Value *, Value *, Value *, Value *, Value *,
                         Value *);
typedef Value *(FnType7)(FnArity *, Value *, Value *, Value *, Value *, Value *,
                         Value *, Value *);
typedef Value *(FnType8)(FnArity *, Value *, Value *, Value *, Value *, Value *,
                         Value *, Value *, Value *);
typedef Value *(FnType9)(FnArity *, Value *, Value *, Value *, Value *, Value *,
                         Value *, Value *, Value *, Value *);

typedef struct {
  int64_t sym_counter;
} intGenerator;

extern REFS_SIZE refsInit;
extern REFS_SIZE refsError;
extern REFS_SIZE refsConstant;
extern REFS_SIZE refsStatic;

#define IgnoreType -1
#define UnknownType 0
#define IntegerType 1
#define StringBufferType 2
#define FnArityType 3
#define FunctionType 4
#define TermType 6
// #define Wut 7
#define VectorType 8
#define VectorNodeType 9
// #define Wut 10
#define BitmapIndexedType 11
#define ArrayNodeType 12
#define HashCollisionNodeType 13
#define HashMapType 14
#define OpaqueType 18
#define FloatType 19
#define CoreTypeCount 20
#define TypeConstraintType 21
#define NoValuesType 22
#define AllValuesType 23
#define MultiConstraintType 24
#define ResultConstraintType 25
#define ItemsConstraintType 26
#define FieldConstraintType 27
#define StaticIntConstraintType 28
#define MinValueType 29
#define MaxValueType 30
#define InferredInnerType 31
#define StaticLengthConstraintType 32
#define StaticStrConstraintType 33
#define ContentsConstraintType 34
#define HashSetType 35
#define ProtoDispatcherType 36
#define SumConstraintType 37
#define KeysConstraintType 38
#define TypeOfConstraintType 39
#define InnerTypeOfConstraintType 40
#define DebugConstraintType 41
#define NoneType 42
#define SomeType 43
#define SubStringType 44
#define TypeCount 45

typedef struct {
  unsigned typeNum;
  char *name;} typeNameMap;
extern typeNameMap typeNames[];
char *typeName(unsigned typeNum);

extern FILE *outstream;
extern Vector *empty_vect;
extern Vector empty_vect_struct;
extern BitmapIndexedNode emptyBMI;
extern Value *universalProtoFn;
extern int typeCount;

extern ReifiedVal all_values_struct;
extern Value *all_values;

extern int8_t mainThreadDone;
extern Term mainFn;

extern int cleaningUp;

extern int64_t malloc_count;
extern int64_t free_count;
void freeAll();

#ifndef FAST_INCS
Term incRef(Term v, int deltaRefs);
#define incRefVal(v, n) ((Value *)incRef((Term)(v), (n)))
#else
Value *simpleIncRef(Value *v, int n);
#define incRef(V, N) ((V)->refs >= 0 ? simpleIncRef(V, N) : V)
#endif

void dec_and_free(Term v, int deltaRefs);
void decValRef(Term pv, int deltaRefs);

void prefs(char *tag, Value *v);

extern Value *(*equalSTAR)(FnArity *, Value *, Value *);
extern Value *(*dissoc)(FnArity *, Value *, Value *, Value *, Value *);
extern Term (*sha1)(FnArity *, Term);
extern Value *(*hashSeq)(FnArity *, Value *, Value *s);
extern Value *(*count)(FnArity *, Value *);
extern Value *(*vals)(FnArity *, Value *);
extern Value *(*zero)(FnArity *, Value *);
extern Value *(*invoke0Args)(FnArity *, Value *f);
extern Value *(*invoke1Arg)(FnArity *, Value *f, Value *arg);
extern Value *(*invoke2Args)(FnArity *, Value *f, Value *arg0, Value *arg1);
extern Value *(*type_name)(FnArity *, Value *t);
extern Value *(*seq)(FnArity *, Value *t);
extern Value *(*newHashSet)(FnArity *, Value *t);
extern Value *(*first)(FnArity *, Value *t);
extern Value *(*rest)(FnArity *, Value *t);
extern Value *(*showFn)(FnArity *, Value *t);
extern Value *(*fn_apply)(FnArity *, Value *f, Value *args);
extern Value *(*hasField)(FnArity *, Value *v, Value *field);
extern Value *(*prErrSTAR)(Value *str);
extern Value *(*prValue)(FnArity *, Value *v);

Value *my_malloc(int64_t sz);
FnArity *malloc_fnArity();
String *malloc_string(long len);
Vector *malloc_vector();
ReifiedVal *malloc_reified(int64_t implCount);
TermVal *malloc_term();

char *extractStr(Value *v);
Value *isInstance(Value *arg0, Value *arg1);
Value *prSTAR(Value *);
Value *add_ints(Value *arg0, Value *arg1);
Term number_str(Term arg0);
Term integer_EQ(Term arg0, Term arg1);
Term integer_LT(Term arg0, Term arg1);
Value *integerValue(int64_t n);
Vector *vectConj(Vector *vect, Term val);
void vectConjFn(Term ref, Term args);
void vectMap(Term ref, Term args);
extern Term vectConjRef;
Vector *mutateVectConj(Vector *vect, Term val);
Term vectGet(Vector *vect, unsigned index);
Term vectStore(Vector *vect, unsigned index, Term val);
ReifiedVal *updateField(ReifiedVal *rval, Term field, int64_t idx);
Vector *vectorReverse(Vector *v);
void destructValue(char *fileName, char *lineNum, Value *val, int numArgs,
                   Value **args[]);
Term strCount(Term s);
#define STR_EQ 1
#define STR_LT 2
#define STR_GT 4
Term strCmp(Term sT, Term tgtT, int success);
Value *strVect(Value *arg0);
Value *checkInstance(TYPE_SIZE typeNum, Value *arg1);
Value *listMap(Value *arg0, Value *arg1);
Value *integerLT(Value *arg0, Value *arg1);
int64_t integerSha1(Term arg0);
Value *bitAnd(Value *arg0, Value *arg1);
Value *bitOr(Value *arg0, Value *arg1);
Value *bitXor(Value *arg0, Value *arg1);
Value *bitShiftLeft(Value *arg0, Value *arg1);
Value *bitShiftRight(Value *arg0, Value *arg1);
Value *bitNot(Value *arg0);
Value *addIntegers(Value *arg0, Value *arg1);
Value *listEQ(Value *arg0, Value *arg1);
int8_t equal(Value *v1, Value *v2);
int64_t strSha1(Value *arg0);
Value *escapeChars(Term arg0);
Value *subs2(Value *arg0, Value *arg1);
Value *subs3(Value *arg0, Value *arg1, Value *arg2);
Value *strSeq(Value *arg0);
Value *strReduce(Value *s0, Value *x1, Value *f2);
Value *strVec(Value *arg0);
Term vectorGet(Term v, Term n);
Value *stringValue(char *s);
Value *opaqueValue(void *ptr, Destructor *destruct);
Value *listFilter(Value *arg0, Value *arg1);
Value *bmiHashSeq(Value *arg0, Value *arg1);
Value *bmiCount(Value *arg0);
Value *bmiCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                    int shift);
Value *bmiMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                      int shift);
Value *bmiGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *bmiDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                          int shift);
Value *arrayNodeMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                            int shift);
Value *collisionAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                      int shift);
Value *hashMapGet(Value *arg0, Value *arg1);
Value *hashMapAssoc(Value *arg0, Value *arg1, Value *arg2);
Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                    int shift);
Value *collisionCount(Value *arg0);
Value *collisionVec(Value *arg0, Value *arg1);
Value *arrayNodeCount(Value *arg0);
Value *collisionSeq(Value *arg0, Value *arg1);
Value *collisionDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
Value *collisionGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash,
                    int shift);
Value *arrayNodeSeq(Value *arg0, Value *arg1);
Value *arrayNodeDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
void freeExtractCache(void *cachePtr);
void freeIntGenerator(void *ptr);
Value *dynamicCall1Arg(Value *f, Value *arg);
String *nullTerm(Value *s);
int64_t countSeq(Value *seq);
Value *malloc_sha1();
long finalize_sha1(Value *ctxt);
void Sha1Update(Sha1Context *Context, void *Buffer, int64_t BufferSize);
void strSha1Update(Sha1Context *ctxt, Value *arg0);
Value *reifiedTypeArgs(Term x);
Value *dispatchProto(Value *protocols, Value *protoSym, Value *fnSym,
                     Value *dispValue, Value *args);
extern Value *baseDissoc(Value *node, Value *key, int64_t hash, int shift);
extern Value *dissoc_impl(FnArity *arity, Value *node, Value *key, Value *hash, Value *shift);
extern Value *mapGet(FnArity *, Value *, Value *, Value *, int64_t hash, int shift);
Value *baseDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
Value *copyAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift);
Value *mutateAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift);
Value *newTypeValue(int typeNum, Vector *fields);
Vector *listVec(Value *list);
Value *defaultPrErrSTAR(Value *str);
Value *hashMapVec(Value *m);
void incTypeMalloc(TYPE_SIZE type, int delta);
void incTypeFree(TYPE_SIZE type, int delta);
Value *hashVec(Value *n, Value *s);

ArrayNode *malloc_arrayNode();
HashCollisionNode *malloc_hashCollisionNode(int itemCount);
void moveFreeToCentral();
void freeGlobals();
void normGlobals();
void freeGlobal(Term p);
Term dupeGlobal(Location glbl);
Term dupeVal(Term *v);
extern Term construct;
Term nothing();
extern Term accessField;
extern Term hvmVect;
extern Term discard;
extern Value *baseDissoc(Value *node, Value *key, int64_t hash, int shift);
extern Value *dissoc_impl(FnArity *arity, Value *node, Value *key, Value *hash, Value *shift);
extern Value *(*dissoc_fn)(FnArity *, Value *, Value *, Value *, Value *);
extern Value *countImpl(FnArity *arity, Value *arg);

void discardFn(Term ref, Term args);
void accessFieldFn(Term ref, Term args);
void constructFn(Term ref, Term args);
void hvmVectFn(Term ref, Term args);
void exitProg(Term ref, Term args);
void graphFn(Term ref, Term args);
