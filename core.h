
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

extern void abort();

typedef struct Value {int64_t type; _Atomic int32_t refs; struct Value* next;} Value;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t numVal;} Integer;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t len; Integer *hash; char buffer[0];} String;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t len; Integer *hash; Value *source; char *buffer;} SubString;
typedef struct List {int64_t type; _Atomic int32_t refs; int64_t len; Value* head; struct List *tail;} List;
typedef struct {int64_t type; _Atomic int32_t refs; int count; List *closures; int variadic; void *fn;} FnArity;
typedef struct {int64_t type; _Atomic int32_t refs; char *name; int64_t arityCount; FnArity *arities[];} Function;
typedef struct {int64_t type; _Atomic int32_t refs; Value* value;} Maybe;

Value *nothing;

#define IntegerType 1
#define StringType 2
#define FnArityType 3
#define FunctionType 4
#define SubStringType 5
#define ListType 6
#define MaybeType 7

FILE *outstream;
List *empty_list;

#define CHECK_MEM_LEAK 1

#ifdef CHECK_MEM_LEAK
_Atomic long long malloc_count;
_Atomic long long free_count;
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


void prefs(char *tag, Value *v);
