
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
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t len; Integer *hash; char buffer[0];} String;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int64_t len; Integer *hash; Value *source; char *buffer;} SubString;
typedef struct List {TYPE_SIZE type; REFS_SIZE refs; int64_t len; Value* head; struct List *tail;} List;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int count; List *closures;
                int variadic; void *fn; Value *paramConstraints; Value *resultConstraint;} FnArity;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; char *name; int64_t arityCount; FnArity *arities[];} Function;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value* value;} Maybe;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *array[VECTOR_ARRAY_LEN];} VectorNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int32_t count; int8_t shift; int64_t tailOffset;
                VectorNode *root; Value *tail[VECTOR_ARRAY_LEN];} Vector;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int32_t bitmap; Value *array[];} BitmapIndexedNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *array[ARRAY_NODE_LEN];} ArrayNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int16_t count; Value *array[];} HashCollisionNode;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *result; List *actions;
                pthread_cond_t delivered; pthread_mutex_t access;} Promise;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *action; Value* errorCallback; List *actions;
                Value *result; pthread_cond_t delivered; pthread_mutex_t access;} Future;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; Value *val; List* input; List *output;
                pthread_mutex_t access;} Agent;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; int8_t implCount; Value* impls[];} ReifiedVal;
typedef struct {TYPE_SIZE type; REFS_SIZE refs; void *ptr; Destructor *destruct;} Opaque;

Integer const0;
Value *const0Ptr;

typedef struct {TYPE_SIZE type; Value *implFn;} ProtoImpl;
typedef struct {int64_t implCount; ProtoImpl impls[];} ProtoImpls;

typedef Value *(FnType0)(List *);
typedef Value *(FnType1)(List *, Value *);
typedef Value *(FnType2)(List *, Value *, Value *);
typedef Value *(FnType3)(List *, Value *, Value *, Value *);
typedef Value *(FnType4)(List *, Value *, Value *, Value *, Value *);
typedef Value *(FnType5)(List *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType6)(List *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType7)(List *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType8)(List *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);
typedef Value *(FnType9)(List *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *, Value *);

typedef struct {
  List *tail;
  pthread_mutex_t access;} extractCache;

typedef struct {
  int64_t sym_counter;} intGenerator;

Value *nothing;
Maybe nothing_struct;
Value *maybeNothing;
REFS_SIZE refsInit;
REFS_SIZE staticRefsInit;
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
#define CoreTypeCount 19
#define TypeConstraintType 20
#define NoValuesType 21
#define AllValuesType 22
#define MultiConstraintType 23
#define ResultConstraintType 24
#define ItemsConstraintType 25
#define FieldConstraintType 26
#define StaticIntConstraintType 27
#define MinValueType 28
#define MaxValueType 29
#define InferredInnerType 30
#define StaticLengthConstraintType 31
#define StaticStrConstraintType 32
#define ContentsConstraintType 33
#define TypeCount 34

FILE *outstream;
List *empty_list;
List empty_list_struct;
Vector *empty_vect;
Vector empty_vect_struct;
BitmapIndexedNode emptyBMI;

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
void dec_and_free(Value *v, int deltaRefs);
Value *incRef(Value *v, int deltaRefs);

void prefs(char *tag, Value *v);

Value *(*equalSTAR)(List *closures, Value *, Value *);
Value *(*dissoc)(List *closures, Value *, Value *, Value *, Value *);
Value *(*sha1)(List *, Value *);
Value *(*hashSeq)(List *, Value* n, Value *s);
Value *(*count)(List *, Value* n);
Value *(*vals)(List *, Value* n);
Value *(*invoke0Args)(List *closures, Value *f);
Value *(*invoke1Arg)(List *closures, Value *f, Value* arg);
Value *(*invoke2Args)(List *closures, Value *f, Value* arg0, Value* arg1);
Value *(*type_name)(List *closures, Value *t);
Value *(*seq)(List *closures, Value *t);
Value *(*newHashSet)(List *closures, Value *t);
Value *(*rest)(List *closures, Value *t);
Value *(*showFn)(List *closures, Value *t);
Value *(*fn_apply)(List *closures, Value *f, Value *args);
Value *(*hasField)(List *closures, Value *v, Value *field);

Value *my_malloc(int64_t sz);
List *malloc_list();
Value *vectSeq(Vector *vect, int index);
FnArity *malloc_fnArity();
Function *malloc_function(int arityCount);
String *malloc_string(int len);
FnArity *findFnArity(Value *fnVal, int64_t argCount);
ReifiedVal *malloc_reified(int implCount);
Promise *malloc_promise();

void startWorkers();
void replaceWorker();
void waitForWorkers();
char *extractStr(Value *v);
Value *isInstance(Value *arg0, Value *arg1);
Value *prSTAR(Value *);
Value *prErrSTAR(Value *);
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
Value *maybe(List *closures, Value *arg0, Value *arg1);
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
Value *integerSha1(Value *arg0);
Value *bitAnd(Value *arg0, Value *arg1);
Value *bitOr(Value *arg0, Value *arg1);
Value *addIntegers(Value *arg0, Value *arg1);
Value *listEQ(Value *arg0, Value *arg1);
int8_t equal(Value *v1, Value *v2);
Value *maybeExtract(Value *arg0);
Value *fnApply(Value *arg0, Value *arg1);
Value *maybeApply(Value *arg0, Value *arg1);
Value *maybeEQ(Value *arg0, Value *arg1);
Value *maybeMap(Value *arg0, Value *arg1);
Value *strSha1(Value *arg0);
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
Value *bmiCopyAssoc(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value* arg4);
Value *bmiMutateAssoc(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value* arg4);
Value *bmiGet(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value *arg4);
Value *bmiDissoc(Value *arg0, Value* arg1, Value* arg2, Value* arg3);
Value *arrayNodeCopyAssoc(Value *arg0, Value *arg1, Value *arg2, Value* arg3, Value *arg4);
Value *arrayNodeMutateAssoc(Value *arg0, Value *arg1, Value *arg2, Value* arg3, Value *arg4);
Value *collisionAssoc(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value *arg4);
Value *hashMapGet(Value *arg0, Value *arg1);
Value *hashMapAssoc(Value *arg0, Value *arg1, Value *arg2);
Value *arrayNodeGet(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value *arg4);
Value *collisionCount(Value *arg0);
Value *arrayNodeCount(Value *arg0);
Value *collisionSeq(Value *arg0, Value *arg1);
Value *collisionDissoc(Value *arg0, Value *arg1, Value *arg2, Value *arg3);
Value *collisionGet(Value *arg0, Value *arg1, Value *arg2, Value *arg3, Value *arg4);
Value *arrayNodeSeq(Value *arg0, Value *arg1);
Value *arrayNodeDissoc(Value *arg0, Value *arg1, Value *arg2, Value *arg3);
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
Value *get(List *, Value *, Value *, Value *, Value *, Value *);
Value *copyAssoc(List *closures, Value *node, Value *k, Value *v, Value *hash, Value *shift);
Value *mutateAssoc(List *closures, Value *node, Value *k, Value *v, Value *hash, Value *shift);
Value *newTypeValue(Value *template_value, Value *fields);
Value *getField(Value *value, int fieldIndex);
