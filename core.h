
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct Value {int64_t type; _Atomic int32_t refs; struct Value* next;} Value;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t numVal;} Integer;
typedef struct {int64_t type; _Atomic int32_t refs; int64_t len; Integer *hash; char buffer[0];} String;

#define IntegerType 1
#define StringType 2
