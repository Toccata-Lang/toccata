
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/wait.h>

extern void abort();

#define VECTOR_ARRAY_LEN 32

typedef struct Value {int64_t type; int32_t refs; struct Value* next;} Value;
typedef struct {int64_t type; int32_t refs; int64_t numVal;} Integer;
typedef struct {int64_t type; int32_t refs; int64_t len; Integer *hash; char buffer[0];} String;
typedef struct {int64_t type; int32_t refs; int64_t len; Integer *hash; Value *source; char *buffer;} SubString;
typedef struct List {int64_t type; int32_t refs; int64_t len; Value* head; struct List *tail;} List;
typedef struct {int64_t type; int32_t refs; int count; List *closures; int variadic; void *fn;} FnArity;
typedef struct {int64_t type; int32_t refs; char *name; int64_t arityCount; FnArity *arities[];} Function;
typedef struct {int64_t type; int32_t refs; Value* value;} Maybe;
typedef struct {int64_t type; int32_t refs; Value *array[VECTOR_ARRAY_LEN];} VectorNode;
typedef struct {int64_t type; int32_t refs; int32_t count; int8_t shift; int64_t tailOffset;
                VectorNode *root; Value *tail[VECTOR_ARRAY_LEN];} Vector;
typedef struct {int64_t type; int32_t refs; Value *typeArgs; int implCount;
                Value* impls[];} ReifiedVal;


typedef struct {int64_t type; Value *implFn;} ProtoImpl;
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

Value *nothing;

#define IntegerType 1
#define StringType 2
#define FnArityType 3
#define FunctionType 4
#define SubStringType 5
#define ListType 6
#define MaybeType 7
#define VectorType 8
#define VectorNodeType 9
#define SymbolType 10

#define BitmapIndexedType 0
#define ArrayNodeType 0
#define HashCollisionNodeType 0
#define HashMapType 0

FILE *outstream;
List *empty_list;
Vector *empty_vect;

#define CHECK_MEM_LEAK 1

#ifdef CHECK_MEM_LEAK
int64_t malloc_count;
int64_t free_count;
void freeAll();
void freeGlobal(Value *x);
#endif
void dec_and_free(Value *v, int deltaRefs);
Value *incRef(Value *v, int deltaRefs);

Value *proto1Arg(ProtoImpls *protoImpls, char *name, Value *arg0,
                 char *file, int64_t line);
Value *proto2Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1,
                 char *file, int64_t line);
Value *proto3Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 char *file, int64_t line);
Value *proto4Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, char *file, int64_t line);
Value *proto5Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, Value *arg4, char *file, int64_t line);
Value *proto6Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, Value *arg4, Value *arg5, char *file, int64_t line);
Value *proto7Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, Value *arg4, Value *arg5, Value *arg6,
                 char *file, int64_t line);
Value *proto8Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, Value *arg4, Value *arg5, Value *arg6, Value *arg7,
                 char *file, int64_t line);
Value *proto9Arg(ProtoImpls *protoImpls, char *name, Value *arg0, Value *arg1, Value *arg2,
                 Value *arg3, Value *arg4, Value *arg5, Value *arg6, Value *arg7,
                 Value *arg8, char *file, int64_t line);

void prefs(char *tag, Value *v);

Value *(*equalSTAR)(List *closures, Value *, Value *);
// Value *(*assoc)(List *closures, Value *, Value *, Value *, Value *, Value *);
// Value *(*dissoc)(List *closures, Value *, Value *, Value *, Value *);
// Value *(*get)(List *, Value *, Value *, Value *, Value *, Value *);
// Value *(*sha1)(List *, Value *);
// Value *(*hashSeq)(List *, Value* n, Value *s);
// Value *(*count)(List *, Value* n);
// Value *(*apply)(List *closures, Value *f, Value *args);
Value *(*invoke0Args)(List *closures, Value *f);
Value *(*invoke1Arg)(List *closures, Value *f, Value* arg);
Value *(*invoke2Args)(List *closures, Value *f, Value* arg0, Value* arg1);
Value *(*type_name)(List *closures, Value *t);
// Value *(*fn_apply)(List *closures, Value *f, Value *args);

List *malloc_list();
Value *vectSeq(Vector *vect, int index);
FnArity *malloc_fnArity();
Function *malloc_function(int arityCount);
String *malloc_string(int len);
FnArity *findFnArity(Value *fnVal, int64_t argCount);
ReifiedVal *malloc_reified(int implCount);


Value *isInstance(Value *arg0, Value *arg1);
Value *intValue(int64_t n);
Value *pr_STAR(Value *);
Value *add_ints(Value *arg0, Value *arg1);
Value *integer_str(Value *arg0);
Value *integer_EQ(Value *arg0, Value *arg1);
Value *integer_LT(Value *arg0, Value *arg1);
Value *integerValue(int64_t n);
Vector *mutateVectConj(Vector *vect, Value *val);
List *listCons(Value *x, List *l);
void destructValue(char *fileName, char *lineNum, Value *val, int numArgs, Value **args[]);
Value *maybe(List *closures, Value *arg0, Value *arg1);
int8_t isNothing(Value *v);
Value *strCount(Value *arg0);
Value *strEQ(Value *arg0, Value *arg1);
Value *strList(Value *arg0);
Value *strVect(Value *arg0);
Value *checkInstance(Value *arg0, Value *arg1);
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
Value *strVec(Value *arg0);
Value *strLT(Value *arg0, Value *arg1);
Value *vectorGet(Value *arg0, Value *arg1);
Value *symbol(Value *arg0);
Value *symbolSha1(Value *arg0);
Value *symEQ(Value *arg0, Value *arg1);
Value *symLT(Value *arg0, Value *arg1);
Value *stringValue(char *s);
Value *maybeInvoke(Value *arg0, Value *arg1, Value *arg2);
