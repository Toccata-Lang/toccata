
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct Value {int64_t type; _Atomic int32_t refs; struct Value* next;} Value;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t numVal;} Integer;

#define IntegerType 1
