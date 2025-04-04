
#include <inttypes.h>

#define HVM

#define TAG_SIZE 8
#define TAG_MASK 0xff
#define VAL_MASK 0x7
#define LAB_SIZE 24
#define LAB_MASK 0xffffff
#define LOC_MASK 0xffffffff

typedef uint8_t bool;
typedef uint8_t  Tag;  // TAG_SIZE bits
typedef uint32_t Lab;  // 24 bits
typedef uint32_t Loc;  // 32 bits
typedef uint64_t Term; // Loc | Lab | Tag
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  i32;
typedef int64_t  i64;

// Booleans
#define TRUE  1
#define FALSE 0

// Constants
// odd tags are positive, even tags are negative
#define VAL 0x00 // native value
#define VAR 0x01
#define SUB 0x02 // possible deferred redex
#define NUL 0x03
#define ERA 0x04
#define LAM 0x05
#define APP 0x06
#define REF 0x07
#define VL1 0x08 // native value alieas
#define SUP 0x09
#define DUP 0x0a
#define OPX 0x0b
#define OPY 0x0c
#define I56 0x0d
#define F56 0x0e
#define LAZ 0x0F
// #define VL2 0x18 // native value alieas

typedef bool (*interactionFn)(Term a, Term b);
#define new_ref(x) (((u64)x + REF))

#define new_i56(x) (((u64)x << TAG_SIZE) | I56)
#define get_i56(x) (i64)((i64)x >> TAG_SIZE)

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

// TODO: Not sure about this
typedef bool (*interactionFn)(Term a, Term b);

#define MAX_ARGS 9
typedef struct {
  int count;
  Term args[MAX_ARGS + 2];
} NativeArgs;

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

#define BOOM(msg) boom(msg, __FILE__, __LINE__)

extern a64 node_count;
extern int max_node;

void *boom(char *msg, char *file, int line);
Tag term_tag(Term term);
Term term_val(Term val);
Term term_new(Tag tag, Lab lab, Loc loc);
Loc term_loc(Term term);
void link(Term neg, Term pos);
void move(Loc neg_loc, Term pos);
Term takeAndCheck(Loc loc);
Term take(Loc loc);
void set(Loc loc, Term term);
Term swap(Loc loc, Term term);
Term nativeArg(Term ref, Term args, NativeArgs *argsStruct);
u64 time64();
void hvm_init();
void hvm_free();
Term pair_make(Tag tag, Term fst, Term snd);
void normalize();
Loc port(u64 n, Loc x);
Term dupeArg(Term arg, Term dupedArg);
char* tag_to_str(Tag tag);
