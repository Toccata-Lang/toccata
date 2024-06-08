
#include <inttypes.h>

// Booleans
#define TRUE  1
#define FALSE 0

// Constants
#define FREE 0x00000000
#define ROOT 0xFFFFFFF8
#define NONE 0xFFFFFFFF

// Configuration
// -------------

// Threads per CPU
#ifndef TPC_L2
#define TPC_L2 4 // 16 cores
#endif
#define TPC (1ul << TPC_L2)

// Integers
// --------

typedef uint8_t bool;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef  int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef    float f32;
typedef   double f64;

typedef _Atomic(u8) a8;
typedef _Atomic(u16) a16;
typedef _Atomic(u32) a32;
typedef _Atomic(u64) a64;

typedef a64 APair; // atomic Pair
typedef a32 APort; // atomic Port

typedef u32 Port; // Port ::= Tag + Val (fits a u32)
typedef u64 Pair; // Pair ::= Port + Port (fits a u64)

// Global Net
#define HLEN (1ul << 16) // max 16k high-priority redexes
#define RLEN (1ul << 24) // max 16m low-priority redexes
#define G_NODE_LEN (1ul << 29) // max 536m nodes
#define G_VARS_LEN (1ul << 29) // max 536m vars
#define G_RBAG_LEN (TPC * RLEN)

// Local Thread Memory
typedef struct TM {
  u32  tid; // thread id
  u32  itrs; // interaction count
  u32  nput; // next node allocation attempt index
  u32  vput; // next vars allocation attempt index
  u32  hput; // next hbag push index
  u32  rput; // next rbag push index
  u32  sidx; // steal index
  u32  nloc[0xFFF]; // node allocation indices
  u32  vloc[0xFFF]; // vars allocation indices
  Pair hbag_buf[HLEN]; // high-priority redexes
} TM;

typedef struct Net {
  APair node_buf[G_NODE_LEN]; // global node buffer
  APort vars_buf[G_VARS_LEN]; // global vars buffer
  APair rbag_buf[G_RBAG_LEN]; // global rbag buffer
  a64 itrs; // interaction count
  a32 idle; // idle thread counter
} Net;

// Types
// -----

// Local Types
typedef u8  Tag;  // Tag  ::= 3-bit (rounded up to u8)
typedef u32 Val;  // Val  ::= 29-bit (rounded up to u32)

// Rules
typedef u8 Rule; // Rule ::= 3-bit (rounded up to 8)

// Numbs
typedef u32 Numb; // Numb ::= 29-bit (rounded up to u32)

// Tags
#define VAR 0x0 // variable
#define REF 0x1 // reference
#define ERA 0x2 // eraser
#define NUM 0x3 // number
#define VAL 0x4 // operator
#define CON 0x5 // constructor
#define DUP 0x6 // duplicator
#define SWI 0x7 // switch


typedef bool (InteractFn)(Net *net, TM *tm, Port a, Port b);

typedef struct {
  int count;
  InteractFn *fns[];
} InteractFns;

extern InteractFns *interactions;

// Declared here for use in call interactions.
bool interact_eras(Net* net, TM* tm, Port a, Port b);
Tag get_tag(Port port);
u32 vars_alloc_1(Net* net, TM* tm, u32* lps);
u32 node_alloc_1(Net* net, TM* tm, u32* lps);
void node_create(Net* net, u32 loc, Pair val);
void vars_create(Net* net, u32 var, Port val);
void link(Net* net, TM* tm, Port A, Port B);
Port new_port(Tag tag, Val val);
Tag get_tag(Port port);
Val get_val(Port port);
const Pair new_pair(Port fst, Port snd);
Port get_fst(Pair pair);
Port get_snd(Pair pair);
void node_store(Net* net, u32 loc, Pair val);
Pair node_load(Net* net, u32 loc);
Pair node_take(Net* net, u32 loc);
Port enter(Net* net, Port var);
Port vars_take(Net* net, u32 var);
u32 get_u24(Numb word);
Numb new_u24(u32 val);
void hvm_c(u32* book_buffer);
