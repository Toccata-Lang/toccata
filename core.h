
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

Value *intValue(int64_t n);
Value *pr_STAR(Value *);
Value *add_ints(Value *arg0, Value *arg1);
Value *integer_str(Value *arg0);
Value *integer_EQ(Value *arg0, Value *arg1);
Vector *mutateVectConj(Vector *vect, Value *val);
List *listCons(Value *x, List *l);
void destructValue(char *fileName, char *lineNum, Value *val, int numArgs, Value **args[]);
Value *maybe(List *closures, Value *arg0, Value *arg1);
int8_t isNothing(Value *v);


void prefs(char *tag, Value *v);
