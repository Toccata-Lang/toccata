
#include <inttypes.h>

typedef uint8_t bool;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef  int64_t i64;
typedef uint64_t u64;
typedef   double f64;

typedef u64 Port; // Port ::= Tag + Val (fits a u32)

typedef struct {Port fst; Port snd;} Pair; // Pair ::= Port + Port (fits a u64)
extern Pair emptyPair;

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

typedef bool (*interactionFn)(TM* tm, Port a, Port b);

// Booleans
#define TRUE  1
#define FALSE 0

// Constants
#define NONE -1
#define FREE 0

// Local Types
typedef u8  Tag;  // Tag  ::= 3-bit (rounded up to u8)
typedef u64 Val;  // Val  ::= 29-bit (rounded up to u32)

// Numbs
typedef u64 Numb; // Numb ::= 60-bit (rounded up to u64)

// Tags
#define VAR 0x0 // variable
#define REF 0x1 // reference
#define ERA 0x2 // eraser
#define NUM 0x3 // number
#define CON 0x4 // constructor
#define DUP 0x5 // duplicator
#define OPR 0x6 // operator
#define SWI 0x7 // switch
#define RDX 0x8 // deferred redex
#define VAL 0x9 // native value

extern Port erase;

// Port: Constructor and Getters
// -----------------------------

Port new_num(Port val);
Val get_num(Port port);
Port new_port(Tag tag, Port val);
Port new_ref(interactionFn val);
Tag get_tag(Port port);
bool ERAS(TM* tm, Port a, Port b);
Port node_alloc(TM* tm, u32* lps);
Port vars_alloc(TM* tm, u32* lps);
void node_create(Port loc, Pair val);
void vars_create(Port var, Port val);
Pair node_load(Port loc);
Port vars_load(Port var);
void node_store(Port loc, Pair val);
Pair node_exchange(Port loc, Pair val);
Port vars_exchange(Port var, Port val);
Pair node_take(Port loc);
Port vars_take(Port var);
void link(TM* tm, Port A, Port B);
const Pair new_pair(Port fst, Port snd);
u8 isEmpty(Pair p);
Numb operate(Port aP, Port bP);
Port enter(Port var);
u64 get_u24(Numb word);
Numb new_u24(u64 val);
void hvm_c(interactionFn mainFn);
