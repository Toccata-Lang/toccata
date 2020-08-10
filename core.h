
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/wait.h>

extern void abort();

#define VECTOR_ARRAY_LEN 32
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

typedef struct
{
 uint32_t        State[5];
 uint32_t        Count[2];
 uint8_t         Buffer[64];
 } Sha1Context;

typedef void (Destructor)(void *);
// TODO: add hash cache and meta data. And update 'make-static-*' as well
typedef struct Value {TYPE_SIZE type; REFS_SIZE refs; struct Value* next;} Value;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t numVal;} Integer;
typedef struct HashedValue {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal;} HashedValue;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int64_t len; char buffer[0];} String;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int64_t len; Value *source; char *buffer;} SubString;
typedef struct List {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal;
                     int64_t len; Value* head; struct List *tail;} List;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *array[VECTOR_ARRAY_LEN];} VectorNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int32_t count; int8_t shift; int64_t tailOffset;
                VectorNode *root; Value *tail[VECTOR_ARRAY_LEN];} Vector;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int count; Vector *closures;
                int variadic; void *fn; Value *paramConstraints; Value *resultConstraint;} FnArity;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; char *name; int64_t arityCount; FnArity *arities[];} Function;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; Value* value;} Maybe;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int32_t bitmap; Value *array[];} BitmapIndexedNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; Value *array[ARRAY_NODE_LEN];} ArrayNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int16_t count; Value *array[];} HashCollisionNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *result; List *actions;
                pthread_cond_t delivered; pthread_mutex_t access;} Promise;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *action; Value* errorCallback; List *actions;
                Value *result; pthread_cond_t delivered; pthread_mutex_t access;} Future;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *val; List* input; List *output;
                pthread_mutex_t access;} Agent;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t hashVal; int64_t implCount; Value* impls[];} ReifiedVal;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; void *ptr; Destructor *destruct;} Opaque;

Integer const0;
Value *const0Ptr;

typedef struct {TYPE_SIZE type; Value *implFn;} ProtoImpl;
typedef struct {int64_t implCount; ProtoImpl impls[];} ProtoImpls;

typedef Value *(FnType0)(Vector *);
typedef Value *(FnType1)(Vector *, Value *);
typedef Value *(FnType2)(Vector *, Value *, Value *);
typedef Value *(FnType3)(Vector *, Value *, Value *, Value *);
typedef Value *(FnType4)(Vector *, Value *, Value *, Value *, Value *);
typedef Value *(FnType5)(Vector *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType6)(Vector *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType7)(Vector *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType8)(Vector *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType9)(Vector *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);

typedef struct {
  List *tail;
  pthread_mutex_t access;} extractCache;

typedef struct {
  int64_t sym_counter;} intGenerator;

Value *nothing;
Maybe nothing_struct;
Value *maybeNothing;
REFS_SIZE refsInit;
REFS_SIZE refsError;

#define IgnoreType -1
#define UnknownType 0
#define IntegerType 1
#define StringBufferType 2
#define FnArityType 3
#define FunctionType 4
#define SubStringType 5
#define ListType 6
#define MaybeType 7
#define VectorType 8
#define VectorNodeType 9
#define SymbolType 10
#define BitmapIndexedType 11
#define ArrayNodeType 12
#define HashCollisionNodeType 13
#define HashMapType 14
#define PromiseType 15
#define FutureType 16
#define AgentType 17
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
#define TypeCount 39

FILE *outstream;
List *empty_list;
List empty_list_struct;
Vector *empty_vect;
Vector empty_vect_struct;
BitmapIndexedNode emptyBMI;
Value *universalProtoFn;
int typeCount;

ReifiedVal all_values_struct;
Value *all_values;

struct {List* input; List* output;
        pthread_mutex_t mutex; pthread_cond_t notEmpty;} futuresQueue;
Future shutDown;
int8_t mainThreadDone;

int cleaningUp;

int64_t malloc_count;
int64_t free_count;
void cleanupMemory(Value *the_final_answer, Value *maybeNothing, List *argList);
void freeAll();
void freeGlobal(Value *x);

#ifndef FAST_INCS
Value *incRef(Value *v, int deltaRefs);
#else
Value *simpleIncRef(Value *v, int n);
#define incRef(V, N) ((V)->refs >= 0 ? simpleIncRef(V, N) : V)
#endif

void dec_and_free(Value *v, int deltaRefs);

void prefs(char *tag, Value *v);

Value *(*equalSTAR)(Vector *, Value *, Value *);
Value *(*dissoc)(Vector *, Value *, Value *, Value *, Value *);
Value *(*sha1)(Vector *, Value *);
Value *(*hashSeq)(Vector *, Value* n, Value *s);
Value *(*count)(Vector *, Value* n);
Value *(*vals)(Vector *, Value* n);
Value *(*invoke0Args)(Vector *, Value *f);
Value *(*invoke1Arg)(Vector *, Value *f, Value* arg);
Value *(*invoke2Args)(Vector *, Value *f, Value* arg0, Value* arg1);
Value *(*type_name)(Vector *, Value *t);
Value *(*seq)(Vector *, Value *t);
Value *(*newHashSet)(Vector *, Value *t);
Value *(*first)(Vector *, Value *t);
Value *(*rest)(Vector *, Value *t);
Value *(*showFn)(Vector *, Value *t);
Value *(*fn_apply)(Vector *, Value *f, Value *args);
Value *(*hasField)(Vector *, Value *v, Value *field);
Value *(*prErrSTAR)(Value *str);
Value *(*prValue)(Vector *, Value *v);

Value *my_malloc(int64_t sz);
List *malloc_list();
Value *vectSeq(Vector *vect, int index);
FnArity *malloc_fnArity();
Function *malloc_function(int arityCount);
String *malloc_string(int len);
Maybe *malloc_maybe();
Integer *malloc_integer();
Vector *malloc_vector();
FnArity *findFnArity(Value *fnVal, int64_t argCount);
ReifiedVal *malloc_reified(int64_t implCount);
Promise *malloc_promise();

void startWorkers();
void replaceWorker();
void waitForWorkers();
char *extractStr(Value *v);
Value *isInstance(Value *arg0, Value *arg1);
Value *prSTAR(Value *);
Value *add_ints(Value *arg0, Value *arg1);
Value *integer_str(Value *arg0);
Value *integer_EQ(Value *arg0, Value *arg1);
Value *integer_LT(Value *arg0, Value *arg1);
Value *integerValue(int64_t n);
Vector *vectConj(Vector *vect, Value *val);
Vector *mutateVectConj(Vector *vect, Value *val);
Value *vectStore(Vector *vect, unsigned index, Value *val);
Value *updateField(Value *rval, Value *field, int64_t index);
Value *vectorReverse(Value *arg0);
List *listCons(Value *x, List *l);
void destructValue(char *fileName, char *lineNum, Value *val, int numArgs, Value **args[]);
Value *maybe(Vector *, Value *arg0, Value *arg1);
int8_t isNothing(Value *v, char *fileName, int lineNumber);
Value *strCount(Value *arg0);
Value *strEQ(Value *arg0, Value *arg1);
Value *strList(Value *arg0);
Value *strVect(Value *arg0);
Value *checkInstance(TYPE_SIZE typeNum, Value *arg1);
Value *listMap(Value *arg0, Value *arg1);
Value *listConcat(Value *arg0);
Value *car(Value *arg0);
Value *cdr(Value *arg0);
Value *integerLT(Value *arg0, Value *arg1);
int64_t integerSha1(Value *arg0);
Value *bitAnd(Value *arg0, Value *arg1);
Value *bitOr(Value *arg0, Value *arg1);
Value *bitXor(Value *arg0, Value *arg1);
Value *bitShiftLeft(Value *arg0, Value *arg1);
Value *bitShiftRight(Value *arg0, Value *arg1);
Value *bitNot(Value *arg0);
Value *addIntegers(Value *arg0, Value *arg1);
Value *listEQ(Value *arg0, Value *arg1);
int8_t equal(Value *v1, Value *v2);
Value *maybeExtract(Value *arg0);
Value *fnApply(Value *arg0, Value *arg1);
Value *maybeApply(Value *arg0, Value *arg1);
Value *maybeEQ(Value *arg0, Value *arg1);
Value *maybeMap(Value *arg0, Value *arg1);
int64_t strSha1(Value *arg0);
Value *escapeChars(Value *arg0);
Value *subs2(Value *arg0, Value *arg1);
Value *subs3(Value *arg0, Value *arg1, Value *arg2);
Value *strSeq(Value *arg0);
Value *strReduce(Value *s0, Value *x1, Value *f2);
Value *strVec(Value *arg0);
Value *strLT(Value *arg0, Value *arg1);
Value *vectorGet(Value *arg0, Value *arg1);
Value *symbol(Value *arg0);
Value *symbolSha1(Value *arg0);
Value *symEQ(Value *arg0, Value *arg1);
Value *symLT(Value *arg0, Value *arg1);
Value *stringValue(char *s);
Value *opaqueValue(void *ptr, Destructor *destruct);
Value *listFilter(Value *arg0, Value *arg1);
List *reverseList(List *input);
Value *bmiHashSeq(Value *arg0, Value *arg1);
Value *bmiCount(Value *arg0);
Value *bmiCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *bmiMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *bmiGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *bmiDissoc(Value *arg0, Value* arg1, int64_t hash, int shift);
Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *arrayNodeMutateAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *collisionAssoc(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *hashMapGet(Value *arg0, Value *arg1);
Value *hashMapAssoc(Value *arg0, Value *arg1, Value *arg2);
Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *collisionCount(Value *arg0);
Value *arrayNodeCount(Value *arg0);
Value *collisionSeq(Value *arg0, Value *arg1);
Value *collisionDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
Value *collisionGet(Value *arg0, Value *arg1, Value *arg2, int64_t hash, int shift);
Value *arrayNodeSeq(Value *arg0, Value *arg1);
Value *arrayNodeDissoc(Value *arg0, Value *arg1, int64_t hash, int shift);
Value *deliverPromise(Value *arg0, Value *arg1);
Value *extractPromise(Value *arg0);
Value *promiseDelivered(Value *arg0);
Value *extractFuture(Value *arg0);
Value *makeFuture(Value *arg0);
Value *makeAgent(Value *arg0);
Value *extractAgent(Value *arg0);
void scheduleAgent(Agent *agent, List *action);
void freeExtractCache(void *cachePtr);
void freeIntGenerator(void *ptr);
Value *addPromiseAction(Promise *promise, Value *action);
Value *dynamicCall1Arg(Value *f, Value *arg);
Value *deliverFuture(Value *fut, Value *val);
Value *addFutureAction(Future *p, Value *action);
String *nullTerm(Value *s);
void show(Value *v);
int64_t countSeq(Value *seq);
Value *malloc_sha1();
Value *finalize_sha1(Value *ctxt);
void Sha1Update (Sha1Context* Context, void* Buffer, int64_t BufferSize);
void strSha1Update(Sha1Context *ctxt, Value *arg0);
Value *reifiedTypeArgs(Value *x);
Value *dispatchProto(Value *protocols, Value *protoSym, Value *fnSym, Value *dispValue, Value *args);
Value *get(Vector *, Value *, Value *, Value *, int64_t hash, int shift);
Value *baseDissoc(Value *arg0, Value* arg1, int64_t hash, int shift);
int64_t nakedSha1(Value *v1);
Value *copyAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift);
Value *mutateAssoc(Value *node, Value *k, Value *v, int64_t hash, int shift);
Value *newTypeValue(int typeNum, Vector *fields);
Value *getField(Value *value, int fieldIndex);
Vector *listVec(Value *list);
Value *defaultPrErrSTAR(Value *str);
Value *vectGet(Vector *vect, unsigned index);
Value *hashMapVec(Value *m);
void incTypeMalloc(TYPE_SIZE type, int delta);
void incTypeFree(TYPE_SIZE type, int delta);
Value *hashVec(Value* n, Value *s);
