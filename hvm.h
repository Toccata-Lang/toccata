
#include <inttypes.h>

#define HVM

// Types
// --------
typedef uint8_t bool;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef  int64_t i64;
typedef uint64_t u64;
typedef   double f64;

// Integers
// --------

typedef _Atomic(u8) a8;
typedef _Atomic(u16) a16;
typedef _Atomic(u32) a32;
typedef _Atomic(u64) a64;

typedef u64 Port; // Port ::= Tag + Val (fits a u32)
typedef struct {Port fst; Port snd;} Pair; // Pair ::= Port + Port (fits a u64)
extern Pair emptyPair;

typedef _Atomic(Port) APort; // atomic Port
typedef _Atomic(Pair) APair; // atomic Pair

// Threads per CPU
#ifndef TPC_L2
#define TPC_L2 4 // 16 cores
#endif
#define TPC (1ul << TPC_L2)
// #define TPC 1

#define HLEN (1ul << 16) // max 16k high-priority redexes
// Local Thread Memory
typedef struct TM {
  u32  tid; // thread id
  u32  itrs; // interaction count
  u32  nput; // next node allocation attempt index
  u32  vput; // next vars allocation attempt index
  u32  hput; // next hbag push index
  u32  rput; // next rbag push index
  u32  sidx; // steal index
  Pair hbag_buf[HLEN]; // high-priority redexes
} TM;

extern TM* tms[];

// Global Net
#define RLEN (1ul << 24) // max 16m low-priority redexes
#define G_NODE_LEN (1ul << 29) // max 536m nodes
#define G_VARS_LEN (1ul << 29) // max 536m vars
#define G_RBAG_LEN (TPC * RLEN)

typedef struct Net {
  APair node_buf[G_NODE_LEN]; // global node buffer
  APort vars_buf[G_VARS_LEN]; // global vars buffer
  APair rbag_buf[G_RBAG_LEN]; // global rbag buffer
  a64 itrs; // interaction count
  a32 idle; // idle thread counter
} Net;

extern Net *globalNet;
extern a32 vars_count;
extern int max_vars;
extern a32 node_count;
extern int max_node;

typedef bool (*interactionFn)(Port a, Port b);

// Booleans
#define TRUE  1
#define FALSE 0

// Constants
#define NONE -1
#define FREE 0
#define TAG_SIZE 4
#define TAG_MASK 0xf
#define NUM_TAG_SIZE 5

// Local Types
typedef u8  Tag;  // Tag  ::= 3-bit (rounded up to u8)
typedef u64 Val;  // Val  ::= 29-bit (rounded up to u32)

// Numbs
typedef u64 Numb; // Numb ::= 60-bit (rounded up to u64)

// Tags
#define VAL 0x0 // native value
#define VAR 0x1 // variable
#define REF 0x2 // reference
#define CON 0x3 // constructor
#define DUP 0x4 // duplicator
#define NUM 0x5 // number
#define OPR 0x6 // operator
#define SWI 0x7 // switch
#define VL1 0x8 // native value alias
#define VR1 0x9 // variable alias
#define RDX 0xA // deferred redex
#define ARG 0xB // function arg or param
#define ERA 0xC // eraser

extern Port erase;
extern Port endArgs;

#define MAX_ARGS 9
typedef struct {
  int count;
  Port args[MAX_ARGS];
  Port result;
} NativeArgs;

// Numbers
#define TY_SYM 0x00
#define TY_U24 0x01
#define TY_I24 0x02
#define TY_F24 0x03
#define OP_ADD 0x04
#define OP_SUB 0x05
#define FP_SUB 0x06
#define OP_MUL 0x07
#define OP_DIV 0x08
#define FP_DIV 0x09
#define OP_REM 0x0A
#define FP_REM 0x0B
#define OP_EQ  0x0C
#define OP_NEQ 0x0D
#define OP_LT  0x0E
#define OP_GT  0x0F
#define OP_AND 0x10
#define OP_OR  0x11
#define OP_XOR 0x12
#define OP_SHL 0x13
#define FP_SHL 0x14
#define OP_SHR 0x15
#define FP_SHR 0x16

#define new_i24_(x) (((u64)x << NUM_TAG_SIZE) | TY_I24)
#define new_num_(x) ((x << TAG_SIZE) | NUM)

// Port: Constructor and Getters
// -----------------------------

// this abuses the compile time functionality
// to create a Port value
#define new_port_(tag, x) (((u64)x + tag))

extern Port finalResultVar;

Port new_num(Port val);
Val get_val(Port port);
Port new_port(Tag tag, Port val);
Port new_ref(interactionFn val);
Tag get_tag(Port port);
bool ERAS(Port a, Port b);
Port node_alloc();
Port vars_alloc();
void node_create(Port loc, Pair val);
void vars_create(Port var, Port val);
Pair node_load(Port loc);
Port vars_load(Port var);
void node_store(Port loc, Pair val);
Pair node_exchange(Port loc, Pair val);
Port vars_exchange(Port var, Port val);
Pair node_take(Port loc);
Port vars_take(Port var);
void link(Port A, Port B);
const Pair new_pair(Port fst, Port snd);
u8 isEmpty(Pair p);
Numb operate(Port aP, Port bP);
Port enter(Port var);
u64 get_u24(Numb word);
Numb new_u24(u64 val);
void vars_create(Port var, Port val);
void hvm_c(interactionFn mainFn, NativeArgs *args);
u64 time64();
void alloc_static_tms();
void free_static_tms();
void normalize();
void net_init();
Port dupeArg(Port arg, Port dupeArg);
Port nativeArg(Port ref, Port args, NativeArgs *argsStruct);
i64 get_i24(Numb word);
Numb new_i24(i64 val);
Port node_make(Tag tag, Port fst, Port snd);
void make_op(int op, Port x, Port y, Port rslt);
void printArgs(Port args);
Port vars_make(Port p);
bool unwind(TM* tm, Port a, Port b);
void freeGlobal(Port p);
