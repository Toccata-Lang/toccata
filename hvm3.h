
#include <inttypes.h>

#define HVM

typedef uint8_t  Tag;  //  8 bits
typedef uint32_t Lab;  // 24 bits
typedef uint32_t Loc;  // 32 bits
typedef uint64_t Term; // Loc | Lab | Tag
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  i32;
typedef float    f32;

// Constants
#define VAR 0x01
#define SUB 0x02
#define NUL 0x03
#define ERA 0x04
#define LAM 0x05
#define APP 0x06
#define SUP 0x07
#define DUP 0x08
#define REF 0x09
#define OPX 0x0A
#define OPY 0x0B
#define U32 0x0C
#define I32 0x0D
#define F32 0x0E
#define MAT 0x0F

const Term VOID = 0;

// Operators
#define OP_ADD 0x00
#define OP_SUB 0x01
#define OP_MUL 0x02
#define OP_DIV 0x03
#define OP_MOD 0x04
#define OP_EQ  0x05
#define OP_NE  0x06
#define OP_LT  0x07
#define OP_GT  0x08
#define OP_LTE 0x09
#define OP_GTE 0x0A
#define OP_AND 0x0B
#define OP_OR  0x0C
#define OP_XOR 0x0D
#define OP_LSH 0x0E
#define OP_RSH 0x0F

// Types
typedef uint64_t u64;
typedef _Atomic(u64) a64;

// Global book
typedef struct Def {
  char* name;
  Term* nodes;
  u64 nodes_len;
  Term* rbag;
  u64 rbag_len;
} Def;

typedef struct Book {
  Def* defs;
  u32 len;
  u32 cap;
} Book;
