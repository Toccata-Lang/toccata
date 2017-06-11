
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

Value *nothing;

#define IntegerType 1
#define StringType 2
#define FnArityType 3
#define FunctionType 4
#define SubStringType 5
#define ListType 6

FILE *outstream;
List *empty_list;

Integer trueVal;
Value* true;

void dec_and_free(Value *v, int deltaRefs);
Value *pr_STAR(Value *);
