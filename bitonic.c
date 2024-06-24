//#include <dlfcn.h>
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Integers
// --------

typedef uint8_t bool;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef  int64_t i64;
typedef uint64_t u64;
typedef   double f64;

typedef _Atomic(u8) a8;
typedef _Atomic(u16) a16;
typedef _Atomic(u32) a32;
typedef _Atomic(u64) a64;

// Configuration
// -------------

// Threads per CPU
#ifndef TPC_L2
#define TPC_L2 4 // 16 cores
#endif
#define TPC (1ul << TPC_L2)

// Types
// -----

// Local Types
typedef u8  Tag;  // Tag  ::= 3-bit (rounded up to u8)
typedef u64 Val;  // Val  ::= 29-bit (rounded up to u32)

#define MAG 0x00000141

// Constants
#define NONE -1
#define FREE 0

typedef u64 Port; // Port ::= Tag + Val (fits a u32)

typedef struct {Port fst; Port snd;} Pair; // Pair ::= Port + Port (fits a u64)
Pair emptyPair = {0, 0};

static inline u8 isEmpty(Pair p) {
  return p.fst == 0 && p.snd == 0;
}

typedef _Atomic(Port) APort; // atomic Port
typedef _Atomic(Pair) APair; // atomic Pair

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

Port erase = ERA;

// Numbers
static const u64 U24_MAX = ((u64)1 << 56) - 1;
static const u64 U24_MIN = 0.0;
static const i64 I24_MAX = ((i64)1 << 55) - 1;
static const i64 I24_MIN = (i64) ((u64)-1 << 55);
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

// Global Net
#define HLEN (1ul << 16) // max 16k high-priority redexes
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

Net *globalNet;

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

// Debugger
// --------

typedef struct {
  char x[13];
} Show;

void put_u16(char* B, u16 val);
Show show_port(Port port);
//void print_rbag(RBag* rbag);
void pretty_print_numb(Numb word);
void pretty_print_port(Port port);

// Port: Constructor and Getters
// -----------------------------

static inline Port new_num(Port val) {
  return (val << 3) | NUM;
}

static inline Val get_num(Port port) {
  return port >> 3;
}

static inline Port new_port(Tag tag, Port val) {
  return (u64)val | tag;
}

// Keep for type checking
static inline Port new_ref(interactionFn val) {
  return (u64)val | REF;
}

static inline Tag get_tag(Port port) {
  return port & 7;
}

// Pair: Constructor and Getters
// -----------------------------

static inline const Pair new_pair(Port fst, Port snd) {
  return (Pair){fst, snd};
}

// Utils
// -----

// Swaps two ports.
static inline void swap(Port *a, Port *b) {
  Port x = *a; *a = *b; *b = x;
}

inline u64 min(u64 a, u64 b) {
  return (a < b) ? a : b;
}

f64 clamp(f64 x, f64 min, f64 max) {
  const f64 t = x < min ? min : x;
  return (t > max) ? max : t;
}

// A simple spin-wait barrier using atomic operations
a64 a_reached = 0; // number of threads that reached the current barrier
a64 a_barrier = 0; // number of barriers passed during this program
void sync_threads() {
  u64 barrier_old = atomic_load_explicit(&a_barrier, memory_order_relaxed);
  if (atomic_fetch_add_explicit(&a_reached, 1, memory_order_relaxed) == (TPC - 1)) {
    // Last thread to reach the barrier resets the counter and advances the barrier
    atomic_store_explicit(&a_reached, 0, memory_order_relaxed);
    atomic_store_explicit(&a_barrier, barrier_old + 1, memory_order_release);
  } else {
    u32 tries = 0;
    while (atomic_load_explicit(&a_barrier, memory_order_acquire) == barrier_old) {
      sched_yield();
    }
  }
}

// TODO: write a time64() function that returns the time as fast as possible as a u64
static inline u64 time64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

// Ports / Pairs / Rules
// ---------------------

// Should we swap ports A and B before reducing this rule?
static inline bool should_swap(Port A, Port B) {
  return get_tag(B) < get_tag(A);
}

// Gets a rule's priority
u8 interactionPriority[8][8] = {
  //VAR   REF   ERA   NUM   CON   DUP   OPR   SWI
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE}, // VAR
  {TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE}, // REF
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE}, // ERA
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE}, // NUM
  {TRUE, FALSE,TRUE, TRUE, TRUE, FALSE,FALSE,FALSE}, // CON
  {TRUE, FALSE,TRUE, TRUE, FALSE,TRUE, FALSE,FALSE}, // DUP
  {TRUE, FALSE,TRUE, FALSE,FALSE,FALSE,TRUE, FALSE}, // OPR
  {TRUE, FALSE,TRUE, FALSE,FALSE,FALSE,FALSE,TRUE} // SWI
};

static inline bool is_high_priority(Pair AB) {
  return interactionPriority[get_tag(AB.fst)][get_tag(AB.snd)];
}

// Numbs
// -----

// Constructor and getters for SYM (operation selector)
static inline Numb new_sym(u64 val) {
  return (val << 5) | TY_SYM;
}

static inline u64 get_sym(Numb word) {
  return (word >> 5);
}

// Constructor and getters for U24 (unsigned 24-bit integer)
static inline Numb new_u24(u64 val) {
  return (val << 5) | TY_U24;
}

static inline u64 get_u24(Numb word) {
  return word >> 5;
}

// Constructor and getters for I24 (signed 24-bit integer)
static inline Numb new_i24(i64 val) {
  return ((u64)val << 5) | TY_I24;
}

static inline i64 get_i24(Numb word) {
  return ((i64)word) << 3 >> 8;
}

// Constructor and getters for F24 (24-bit float)
static inline Numb new_f24(float val) {
  u64 bits = *(u64*)&val;
  u64 shifted_bits = bits >> 8;
  u64 lost_bits = bits & 0xFF;
  // round ties to even
  shifted_bits += (!isnan(val)) & ((lost_bits - ((lost_bits >> 7) & !shifted_bits)) >> 7);
  // ensure NaNs don't become infinities
  shifted_bits |= isnan(val);
  return (shifted_bits << 5) | TY_F24;
}

static inline f64 get_f24(Numb word) {
  u64 bits = (word << 3) & 0xFFFFFFFFFFFFFF00;
  return *(f64*)&bits;
}

// Flip flag
static inline Tag get_typ(Numb word) {
  return word & 0x1F;
}

static inline bool is_num(Numb word) {
  return get_typ(word) >= TY_U24 && get_typ(word) <= TY_F24;
}

static inline bool is_cast(Numb word) {
  return get_typ(word) == TY_SYM && get_sym(word) >= TY_U24 && get_sym(word) <= TY_F24;
}

// Partial application
static inline Numb partial(Numb a, Numb b) {
  return (b & ~0x1F) | get_sym(a);
}

// Cast a number to another type.
// The semantics are meant to spiritually resemble rust's numeric casts:
// - i24 <-> u24: is just reinterpretation of bits
// - f24  -> i24,
//   f24  -> u24: casts to the "closest" integer representing this float,
//                saturating if out of range and 0 if NaN
// - i24  -> f24,
//   u24  -> f24: casts to the "closest" float representing this integer.
static inline Numb cast(Numb a, Numb b) {
  if (get_sym(a) == TY_U24 && get_typ(b) == TY_U24) return b;
  if (get_sym(a) == TY_U24 && get_typ(b) == TY_I24) {
    // reinterpret bits
    i64 val = get_i24(b);
    return new_u24(*(u64*) &val);
  }
  if (get_sym(a) == TY_U24 && get_typ(b) == TY_F24) {
    f64 val = get_f24(b);
    if (isnan(val)) {
      return new_u24(0);
    }
    return new_u24((u64) clamp(val, U24_MIN, U24_MAX));
  }

  if (get_sym(a) == TY_I24 && get_typ(b) == TY_U24) {
    // reinterpret bits
    u64 val = get_u24(b);
    return new_i24(*(i64*) &val);
  }
  if (get_sym(a) == TY_I24 && get_typ(b) == TY_I24) return b;
  if (get_sym(a) == TY_I24 && get_typ(b) == TY_F24) {
    f64 val = get_f24(b);
    if (isnan(val)) {
      return new_i24(0);
    }
    return new_i24((i64) clamp(val, I24_MIN, I24_MAX));
  }

  if (get_sym(a) == TY_F24 && get_typ(b) == TY_U24) return new_f24((f64) get_u24(b));
  if (get_sym(a) == TY_F24 && get_typ(b) == TY_I24) return new_f24((f64) get_i24(b));
  if (get_sym(a) == TY_F24 && get_typ(b) == TY_F24) return b;

  return new_u24(0);
}

// Operate function
static inline Numb operate(Port aP, Port bP) {
  Numb a = aP >> 3;
  Numb b = bP >> 3;
  Tag at = get_typ(a);
  Tag bt = get_typ(b);
  if (at == TY_SYM && bt == TY_SYM) {
    return new_u24(0);
  }
  if (is_cast(a) && is_num(b)) {
    return cast(a, b);
  }
  if (is_cast(b) && is_num(a)) {
    return cast(b, a);
  }
  if (at == TY_SYM && bt != TY_SYM) {
    return partial(a, b);
  }
  if (at != TY_SYM && bt == TY_SYM) {
    return partial(b, a);
  }
  if (at >= OP_ADD && bt >= OP_ADD) {
    return new_u24(0);
  }
  if (at < OP_ADD && bt < OP_ADD) {
    return new_u24(0);
  }
  Tag op, ty;
  Numb swp;
  if (at >= OP_ADD) {
    op = at; ty = bt;
  } else {
    op = bt; ty = at; swp = a; a = b; b = swp;
  }
  switch (ty) {
    case TY_U24: {
      u64 av = get_u24(a);
      u64 bv = get_u24(b);
      switch (op) {
        case OP_ADD: return new_u24(av + bv);
        case OP_SUB: return new_u24(av - bv);
        case FP_SUB: return new_u24(bv - av);
        case OP_MUL: return new_u24(av * bv);
        case OP_DIV: return new_u24(av / bv);
        case FP_DIV: return new_u24(bv / av);
        case OP_REM: return new_u24(av % bv);
        case FP_REM: return new_u24(bv % av);
        case OP_EQ:  return new_u24(av == bv);
        case OP_NEQ: return new_u24(av != bv);
        case OP_LT:  return new_u24(av < bv);
        case OP_GT:  return new_u24(av > bv);
        case OP_AND: return new_u24(av & bv);
        case OP_OR:  return new_u24(av | bv);
        case OP_XOR: return new_u24(av ^ bv);
        case OP_SHL: return new_u24(av << (bv & 63));
        case FP_SHL: return new_u24(bv << (av & 63));
        case OP_SHR: return new_u24(av >> (bv & 63));
        case FP_SHR: return new_u24(bv >> (av & 63));
        default:     return new_u24(0);
      }
    }
    case TY_I24: {
      i64 av = get_i24(a);
      i64 bv = get_i24(b);
      switch (op) {
        case OP_ADD: return new_i24(av + bv);
        case OP_SUB: return new_i24(av - bv);
        case FP_SUB: return new_i24(bv - av);
        case OP_MUL: return new_i24(av * bv);
        case OP_DIV: return new_i24(av / bv);
        case FP_DIV: return new_i24(bv / av);
        case OP_REM: return new_i24(av % bv);
        case FP_REM: return new_i24(bv % av);
        case OP_EQ:  return new_u24(av == bv);
        case OP_NEQ: return new_u24(av != bv);
        case OP_LT:  return new_u24(av < bv);
        case OP_GT:  return new_u24(av > bv);
        case OP_AND: return new_i24(av & bv);
        case OP_OR:  return new_i24(av | bv);
        case OP_XOR: return new_i24(av ^ bv);
        default:     return new_i24(0);
      }
    }
    case TY_F24: {
      float av = get_f24(a);
      float bv = get_f24(b);
      switch (op) {
        case OP_ADD: return new_f24(av + bv);
        case OP_SUB: return new_f24(av - bv);
        case FP_SUB: return new_f24(bv - av);
        case OP_MUL: return new_f24(av * bv);
        case OP_DIV: return new_f24(av / bv);
        case FP_DIV: return new_f24(bv / av);
        case OP_REM: return new_f24(fmodf(av, bv));
        case FP_REM: return new_f24(fmodf(bv, av));
        case OP_EQ:  return new_u24(av == bv);
        case OP_NEQ: return new_u24(av != bv);
        case OP_LT:  return new_u24(av < bv);
        case OP_GT:  return new_u24(av > bv);
        case OP_AND: return new_f24(atan2f(av, bv));
        case OP_OR:  return new_f24(logf(bv) / logf(av));
        case OP_XOR: return new_f24(powf(av, bv));
        default:     return new_f24(0);
      }
    }
    default: return new_u24(0);
  }
}

// RBag
// ----

// FIXME: what about some bound checks?

static inline void push_redex(TM* tm, Pair redex) {
  if (is_high_priority(redex)) {
    tm->hbag_buf[tm->hput++] = redex;
  } else {
    atomic_store_explicit(&globalNet->rbag_buf[tm->tid*(G_RBAG_LEN/TPC) + (tm->rput++)], redex, memory_order_relaxed);
  }
}

static inline Pair pop_redex(TM* tm) {
  if (tm->hput > 0) {
    return tm->hbag_buf[--tm->hput];
  } else if (tm->rput > 0) {
    return atomic_exchange_explicit(&globalNet->rbag_buf[tm->tid*(G_RBAG_LEN/TPC) + (--tm->rput)],
				    emptyPair,
				    memory_order_relaxed);
  } else {
    return emptyPair;
  }
}

static inline u32 rbag_len(TM* tm) {
  return tm->rput + tm->hput;
}

// TM
// --

static TM* tm[TPC];

TM* tm_new(u32 tid) {
  TM* tm   = malloc(sizeof(TM));
  tm->tid  = tid;
  tm->itrs = 0;
  tm->nput = 1;
  tm->vput = 1;
  tm->rput = 0;
  tm->hput = 0;
  tm->sidx = 0;
  return tm;
}

void alloc_static_tms() {
  for (u32 t = 0; t < TPC; ++t) {
    tm[t] = tm_new(t);
  }
}

void free_static_tms() {
  for (u32 t = 0; t < TPC; ++t) {
    free(tm[t]);
  }
}

// Net
// ----

// Stores a new node on global.
static inline void node_create(Port loc, Pair val) {
  atomic_store_explicit((APair*)((u64)loc & ~7), val, memory_order_relaxed);
}

// Stores a var on global.
static inline void vars_create(Port var, Port val) {
  atomic_store_explicit((APort*)((u64)var & ~7), val, memory_order_relaxed);
}

// Reads a node from global.
static inline Pair node_load(Port loc) {
  return atomic_load_explicit((APair*)((u64)loc & ~7), memory_order_relaxed);
}

// Reads a var from global.
static inline Port vars_load(Port var) {
  return atomic_load_explicit((APort*)((u64)var & ~7), memory_order_relaxed);
}

// Stores a node on global.
static inline void node_store(Port loc, Pair val) {
  atomic_store_explicit((APair*)((u64)loc & ~7), val, memory_order_relaxed);
}

// Exchanges a node on global by a value. Returns old.
static inline Pair node_exchange(Port loc, Pair val) {
  return atomic_exchange_explicit((APair*)((u64)loc & ~7), val, memory_order_relaxed);
}

// Exchanges a var on global by a value. Returns old.
static inline Port vars_exchange(Port var, Port val) {
  return atomic_exchange_explicit((APort*)((u64)var & ~7), val, memory_order_relaxed);
}

// Takes a node.
static inline Pair node_take(Port loc) {
  return node_exchange(loc, emptyPair);
}

// Takes a var.
static inline Port vars_take(Port var) {
  return vars_exchange(var, 0);
}


// Net
// ---

// Initializes a net.
static inline void net_init() {
  // is that needed?
  atomic_store(&globalNet->itrs, 0);
  atomic_store(&globalNet->idle, 0);
}

// Allocator
// ---------

Port node_alloc(TM* tm, u32* lps) {
  while (TRUE) {
    u32 lc = tm->tid*(G_NODE_LEN/TPC) + (tm->nput%(G_NODE_LEN/TPC));
    Pair* elem = (Pair *)&globalNet->node_buf[lc];
    tm->nput += 1;
    if (lc > 0 && isEmpty(*elem)) {
      return (Port)elem;
    }
    // FIXME: check this decently
    if (++(*lps) >= G_NODE_LEN/TPC) printf("OOM\n");
  }
}

Port vars_alloc(TM* tm, u32* lps) {
  while (TRUE) {
    u32 lc = tm->tid*(G_NODE_LEN/TPC) + (tm->vput%(G_NODE_LEN/TPC));
    Port* elem = (Port*)&globalNet->vars_buf[lc];
    tm->vput += 1;
    if (lc > 0 && *elem == 0) {
      return (Port)elem;
    }
    // FIXME: check this decently
    if (++(*lps) >= G_NODE_LEN/TPC) printf("OOM\n");
  }
}

// Linking
// -------

// Finds a variable's value.
static inline Port enter(Port var) {
  // While `B` is VAR: extend it (as an optimization)
  while (get_tag(var) == VAR) {
    // Takes the current `var` substitution as `val`
    Port val = vars_exchange(var, NONE);
    // If there was no `val`, stop, as there is no extension
    if (val == NONE || val == 0) {
      break;
    }
    // Otherwise, delete `B` (we own both) and continue
    vars_take(var);
    var = val;
  }
  return var;
}

// Atomically Links `A ~ B`.
static inline void link(TM* tm, Port A, Port B) {
  //printf("LINK %s ~> %s\n", show_port(A).x, show_port(B).x);

  // Attempts to directionally point `A ~> B`
  while (TRUE) {
    // If `A` is NODE: swap `A` and `B`, and continue
    if (get_tag(A) != VAR && get_tag(B) == VAR) {
      Port X = A; A = B; B = X;
    }

    // If `A` is NODE: create the `A ~ B` redex
    if (get_tag(A) != VAR) {
      push_redex(tm, new_pair(A, B)); // TODO: move global ports to local
      break;
    }

    // Extends B (as an optimization)
    B = enter(B);

    // Since `A` is VAR: point `A ~> B`.
    if (TRUE) {
      // Stores `A -> B`, taking the current `A` subst as `A'`
      Port A_ = vars_exchange(A, B);
      // If there was no `A'`, stop, as we lost B's ownership
      if (A_ == NONE) {
        break;
      }
      //if (A_ == 0) { ? } // FIXME: must handle on the move-to-global algo
      // Otherwise, delete `A` (we own both) and link `A' ~ B`
      vars_take(A);
      A = A_;
    }
  }
}

// Links `A ~ B` (as a pair).
static inline void link_pair(TM* tm, Pair AB) {
  //printf("link_pair %016llx\n", AB);
  link(tm, AB.fst, AB.snd);
}

// Interactions
// ------------

// The Link Interaction.
bool LINK(TM* tm, Port a, Port b) {
  // Links.
  link_pair(tm, new_pair(a, b));

  return TRUE;
}

bool CALL(TM *tm, Port a, Port b) {
  interactionFn fnPtr;

  fnPtr = (interactionFn)(a & ~3);
  return fnPtr(tm, a, b);
}

// The Void Interaction.
static inline bool VOID(TM* tm, Port a, Port b) {
  return TRUE;
}

// The Eras Interaction.
static inline bool ERAS(TM* tm, Port a, Port b) {
  // Checks availability
  if (isEmpty(node_load(b))) {
    //printf("[%04x] unavailable0: %s\n", tid, show_port(b).x);
    return FALSE;
  }

  // Loads ports.
  Pair B  = node_exchange(b, emptyPair);
  Port B1 = B.fst;
  Port B2 = B.snd;

  //if (B == 0) printf("[%04x] ERROR2: %s\n", tid, show_port(b).x);

  // Links.
  link_pair(tm, new_pair(a, B1));
  link_pair(tm, new_pair(a, B2));

  return TRUE;
}

// The Anni Interaction.
static inline bool ANNI(TM* tm, Port a, Port b) {
  // Checks availability
  if (isEmpty(node_load(a)) || isEmpty(node_load(b))) {
    //printf("[%04x] unavailable1: %s | %s\n", tid, show_port(a).x, show_port(b).x);
    //printf("BBB\n");
    return FALSE;
  }

  // Loads ports.
  Pair A  = node_take(a);
  Port A1 = A.fst;
  Port A2 = A.snd;
  Pair B  = node_take(b);
  Port B1 = B.fst;
  Port B2 = B.snd;

  //if (A == 0) printf("[%04x] ERROR3: %s\n", tid, show_port(a).x);
  //if (B == 0) printf("[%04x] ERROR4: %s\n", tid, show_port(b).x);

  // Links.
  link_pair(tm, new_pair(A1, B1));
  link_pair(tm, new_pair(A2, B2));

  return TRUE;
}

// The Comm Interaction.
static inline bool COMM(TM* tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);

  // Checks availability
  if (isEmpty(node_load(a)) || isEmpty(node_load(b))) {
    //printf("[%04x] unavailable2: %s | %s\n", tid, show_port(a).x, show_port(b).x);
    return FALSE;
  }

  // Loads ports.
  Pair A  = node_take(a);
  Port A1 = A.fst;
  Port A2 = A.snd;
  Pair B  = node_take(b);
  Port B1 = B.fst;
  Port B2 = B.snd;

  //if (A == 0) printf("[%04x] ERROR5: %s\n", tid, show_port(a).x);
  //if (B == 0) printf("[%04x] ERROR6: %s\n", tid, show_port(b).x);

  // Stores new vars.
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);

  // Stores new nodes.
  node_create(n0, new_pair(new_port(VAR, v0), new_port(VAR, v1)));
  node_create(n1, new_pair(new_port(VAR, v2), new_port(VAR, v3)));
  node_create(n2, new_pair(new_port(VAR, v0), new_port(VAR, v2)));
  node_create(n3, new_pair(new_port(VAR, v1), new_port(VAR, v3)));

  // Links.
  link_pair(tm, new_pair(new_port(get_tag(b), n0), A1));
  link_pair(tm, new_pair(new_port(get_tag(b), n1), A2));
  link_pair(tm, new_pair(new_port(get_tag(a), n2), B1));
  link_pair(tm, new_pair(new_port(get_tag(a), n3), B2));

  return TRUE;
}

// The Oper Interaction.
static inline bool OPER(TM* tm, Port a, Port b) {
  //printf("OPER %08x %08x\n", a, b);

  // Allocates needed nodes and vars.
  u32 nl = 0;
  Port n0 = node_alloc(tm, &nl);

  // Checks availability
  if (isEmpty(node_load(b))) {
    return FALSE;
  }

  // Loads ports.
  Pair B  = node_take(b);
  Port B1 = B.fst;
  Port B2 = enter(B.snd);

  // Performs operation.
  if (get_tag(B1) == NUM) {
    Numb cv = operate(a, B1);
    link_pair(tm, new_pair(new_num(cv), B2));
  } else {
    node_create(n0, new_pair(a, B2));
    link_pair(tm, new_pair(B1, new_port(OPR, n0)));
  }

  return TRUE;
}

// The Swit Interaction.
static inline bool SWIT(TM* tm, Port a, Port b) {
  u32 nl = 0;
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);

  // Checks availability
  if (isEmpty(node_load(b))) {
    return FALSE;
  }

  // Loads ports.
  u64  av = get_u24(get_num(a));
  Pair B  = node_take(b);
  Port B1 = B.fst;
  Port B2 = B.snd;

  // Stores new nodes.
  if (av == 0) {
    node_create(n0, new_pair(B2, erase));
    link_pair(tm, new_pair(new_port(CON, n0), B1));
  } else {
    node_create(n0, new_pair(erase, new_port(CON, n1)));
    node_create(n1, new_pair(new_num(new_u24(av-1)), B2));
    link_pair(tm, new_pair(new_port(CON, n0), B1));
  }

  return TRUE;
}

interactionFn interactions[8][8] = {
  //VAR   REF   ERA   NUM   CON   DUP   OPR   SWI
  {&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK}, // VAR
  {&LINK,&VOID,&VOID,&VOID,&CALL,&CALL,&CALL,&CALL}, // REF
  {&LINK,&VOID,&VOID,&VOID,&ERAS,&ERAS,&ERAS,&ERAS}, // ERA
  {&LINK,&VOID,&VOID,&VOID,&ERAS,&ERAS,&OPER,&SWIT}, // NUM
  {&LINK,&CALL,&ERAS,&ERAS,&ANNI,&COMM,&COMM,&COMM}, // CON
  {&LINK,&CALL,&ERAS,&ERAS,&COMM,&ANNI,&COMM,&COMM}, // DUP
  {&LINK,&CALL,&ERAS,&OPER,&COMM,&COMM,&ANNI,&COMM}, // OPR
  {&LINK,&CALL,&ERAS,&SWIT,&COMM,&COMM,&COMM,&ANNI} // SWI
};

interactionFn get_rule(Port a, Port b) {
  return interactions[get_tag(a)][get_tag(b)];
}

// Pops a local redex and performs a single interaction.
static inline bool interact(TM* tm) {
  // Pops a redex.
  Pair redex = pop_redex(tm);

  // If there is no redex, stop.
  if (!isEmpty(redex)) {
    // Gets redex ports A and B.
    Port a = redex.fst;
    Port b = redex.snd;

    // Gets the rule type.
    interactionFn rule = get_rule(a, b);

    // Swaps ports if necessary.
    if (should_swap(a,b)) {
      swap(&a, &b);
    }

    // If error, pushes redex back.
    if (!rule(tm, a, b)) {
      push_redex(tm, redex);
      return FALSE;
    // Else, increments the interaction count.
    } else if (rule != LINK) {
      tm->itrs += 1;
    }
  }

  return TRUE;
}

// Evaluator
// ---------

void evaluator(TM* tm) {
  // Initializes the global idle counter
  atomic_store_explicit(&globalNet->idle, TPC - 1, memory_order_relaxed);
  sync_threads();

  // Performs some interactions
  u32  tick = 0;
  bool busy = tm->tid == 0;
  while (TRUE) {
    tick += 1;

    //if (tm->tid == 1) printf("think %d\n", rbag_len(net, tm));

    // If we have redexes...
    if (rbag_len(tm) > 0) {
      // Update global idle counter
      if (!busy) atomic_fetch_sub_explicit(&globalNet->idle, 1, memory_order_relaxed);
      busy = TRUE;
      // Perform an interaction
      interact(tm);
    // If we have no redexes...
    } else {
      // Update global idle counter
      if (busy) atomic_fetch_add_explicit(&globalNet->idle, 1, memory_order_relaxed);
      busy = FALSE;

      //// Peeks a redex from target
      u32  sid = (tm->tid - 1) % TPC;
      u32  idx = sid*(G_RBAG_LEN/TPC) + (tm->sidx++);

      // Steal Parallel: this will only steal parallel redexes

      //Pair trg = atomic_load_explicit(&net->rbag_buf[idx], memory_order_relaxed);
      //// If we're ahead of target, reset
      //if (trg == 0) {
        //tm->sidx = 0;
      //// If the redex is parallel, attempt to steal it
      //} else if (get_par_flag(trg)) {
        //bool stolen = atomic_compare_exchange_weak_explicit(&net->rbag_buf[idx], &trg, 0, memory_order_relaxed, memory_order_relaxed);
        //if (stolen) {
          //push_redex(tm, trg);
        //} else {
          //// do nothing: will sched_yield
        //}
      //// If we see a non-stealable redex, try the next one
      //} else {
        //continue;
      //}

      // Stealing Everything: this will steal all redexes

      Pair got = atomic_exchange_explicit(&globalNet->rbag_buf[idx], emptyPair, memory_order_relaxed);
      if (!isEmpty(got)) {
        //printf("[%04x] stolen one task from %04x | itrs=%d idle=%d | %s ~ %s\n", tm->tid, sid, tm->itrs, atomic_load_explicit(&net->idle, memory_order_relaxed),show_port(got)).x, show_port(got).snd.x.fst;
        push_redex(tm, got);
        continue;
      } else {
        //printf("[%04x] failed to steal from %04x | itrs=%d idle=%d |\n", tm->tid, sid, tm->itrs, atomic_load_explicit(&net->idle, memory_order_relaxed));
        tm->sidx = 0;
      }

      // Chill...
      sched_yield();
      // Halt if all threads are idle
      if (tick % 256 == 0) {
        if (atomic_load_explicit(&globalNet->idle, memory_order_relaxed) == TPC) {
          break;
        }
      }
    }
  }

  sync_threads();

  atomic_fetch_add(&globalNet->itrs, tm->itrs);
  tm->itrs = 0;
}

// Normalizer
// ----------

// Thread data
typedef struct {
  TM*   tm;
} ThreadArg;

void* thread_func(void* arg) {
  ThreadArg* data = (ThreadArg*)arg;
  evaluator(data->tm);
  return NULL;
}

// Evaluates all redexes.
// TODO: cache threads to avoid spawning overhead
void normalize() {
  // Inits thread_arg objects
  ThreadArg thread_arg[TPC];
  for (u32 t = 0; t < TPC; ++t) {
    thread_arg[t].tm   = tm[t];
  }

  // Spawns the evaluation threads
  pthread_t threads[TPC];
  for (u32 t = 0; t < TPC; ++t) {
    pthread_create(&threads[t], NULL, thread_func, &thread_arg[t]);
  }

  // Wait for the threads to finish
  for (u32 t = 0; t < TPC; ++t) {
    pthread_join(threads[t], NULL);
  }
}

// Debug Printing
// --------------

void pretty_print_numb(Numb word) {
  switch (get_typ(word)) {
    case TY_SYM: {
      switch (get_sym(word)) {
        // types
        case TY_U24: printf("[u24]"); break;
        case TY_I24: printf("[i24]"); break;
        case TY_F24: printf("[f24]"); break;
        // operations
        case OP_ADD: printf("[+]"); break;
        case OP_SUB: printf("[-]"); break;
        case FP_SUB: printf("[:-]"); break;
        case OP_MUL: printf("[*]"); break;
        case OP_DIV: printf("[/]"); break;
        case FP_DIV: printf("[:/]"); break;
        case OP_REM: printf("[%%]"); break;
        case FP_REM: printf("[:%%]"); break;
        case OP_EQ:  printf("[=]"); break;
        case OP_NEQ: printf("[!]"); break;
        case OP_LT:  printf("[<]"); break;
        case OP_GT:  printf("[>]"); break;
        case OP_AND: printf("[&]"); break;
        case OP_OR:  printf("[|]"); break;
        case OP_XOR: printf("[^]"); break;
        case OP_SHL: printf("[<<]"); break;
        case FP_SHL: printf("[:<<]"); break;
        case OP_SHR: printf("[>>]"); break;
        case FP_SHR: printf("[:>>]"); break;
        default:     printf("[?]"); break;
      }
      break;
    }
    case TY_U24: {
      printf("%lu", get_u24(word));
      break;
    }
    case TY_I24: {
      printf("%+ld", get_i24(word));
      break;
    }
    case TY_F24: {
      if (isinf(get_f24(word))) {
        if (signbit(get_f24(word))) {
          printf("-inf");
        } else {
          printf("+inf");
        }
      } else if (isnan(get_f24(word))) {
        printf("+NaN");
      } else {
        printf("%.7e", get_f24(word));
      }
      break;
    }
    default: {
      switch (get_typ(word)) {
        case OP_ADD: printf("[+0x%07lX]", get_u24(word)); break;
        case OP_SUB: printf("[-0x%07lX]", get_u24(word)); break;
        case FP_SUB: printf("[:-0x%07lX]", get_u24(word)); break;
        case OP_MUL: printf("[*0x%07lX]", get_u24(word)); break;
        case OP_DIV: printf("[/0x%07lX]", get_u24(word)); break;
        case FP_DIV: printf("[:/0x%07lX]", get_u24(word)); break;
        case OP_REM: printf("[%%0x%07lX]", get_u24(word)); break;
        case FP_REM: printf("[:%%0x%07lX]", get_u24(word)); break;
        case OP_EQ:  printf("[=0x%07lX]", get_u24(word)); break;
        case OP_NEQ: printf("[!0x%07lX]", get_u24(word)); break;
        case OP_LT:  printf("[<0x%07lX]", get_u24(word)); break;
        case OP_GT:  printf("[>0x%07lX]", get_u24(word)); break;
        case OP_AND: printf("[&0x%07lX]", get_u24(word)); break;
        case OP_OR:  printf("[|0x%07lX]", get_u24(word)); break;
        case OP_XOR: printf("[^0x%07lX]", get_u24(word)); break;
        case OP_SHL: printf("[<<0x%07lX]", get_u24(word)); break;
        case FP_SHL: printf("[:<<0x%07lX]", get_u24(word)); break;
        case OP_SHR: printf("[>>0x%07lX]", get_u24(word)); break;
        case FP_SHR: printf("[:>>0x%07lX]", get_u24(word)); break;
        default:     printf("[?0x%07lX]", get_u24(word)); break;
      }
      break;
    }
  }

}

void pretty_print_port(Port port) {
  Port stack[256];
  stack[0] = port;
  u32 len = 1;
  u32 num = 0;
  while (len > 0) {
    Port cur = stack[--len];
    switch (get_tag(cur)) {
      case CON: {
        Pair node = node_load(cur);
        Port p2   = node.snd;
        Port p1   = node.fst;
        printf("(");
        stack[len++] = new_num((u32)(')'));
        stack[len++] = p2;
        stack[len++] = new_num((u32)(' '));
        stack[len++] = p1;
        break;
      }
      case ERA: {
        if ((cur & ~7) != 0) {
          printf("%c", (char)cur & ~7);
        } else {
          printf("*");
        }
        break;
      }
      case VAR: {
        Port got = vars_load(cur);
        if (got != NONE) {
          stack[len++] = got;
        } else {
          printf("x%lx", cur & ~7);
        }
        break;
      }
      case NUM: {
        pretty_print_numb(get_num(cur));
        break;
      }
      case DUP: {
        Pair node = node_load(cur);
        Port p2   = node.snd;
        Port p1   = node.fst;
        printf("{");
        stack[len++] = new_num((u32)('}'));
        stack[len++] = p2;
        stack[len++] = new_num((u32)(' '));
        stack[len++] = p1;
        break;
      }
      case OPR: {
        Pair node = node_load(cur);
        Port p2   = node.snd;
        Port p1   = node.fst;
        printf("$(");
        stack[len++] = new_num((u32)(')'));
        stack[len++] = p2;
        stack[len++] = new_num((u32)(' '));
        stack[len++] = p1;
        break;
      }
      case SWI: {
        Pair node = node_load(cur);
        Port p2   = node.snd;
        Port p1   = node.fst;
        printf("?(");
        stack[len++] = new_num((u32)(')'));
        stack[len++] = p2;
        stack[len++] = new_num((u32)(' '));
        stack[len++] = p1;
        break;
      }
    }
  }
}

// Main
// ----

// The Call Interaction.
bool CALL_main__C1(TM *tm, Port a, Port b);
bool CALL_sum(TM *tm, Port a, Port b);
bool CALL_main(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0 || !n1) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n1, new_pair(new_ref(CALL_main__C1),new_port(VAR,v0)));
  node_create(n0, new_pair(new_num(MAG),new_port(CON,n1)));
  link(tm, new_ref(CALL_sum), new_port(CON,n0));
  return TRUE;
}

bool CALL_down__C0(TM *tm, Port a, Port b);
bool CALL_down(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_down__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_flow(TM *tm, Port a, Port b);
bool CALL_down__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v7), k16);
  } else {
    k16 = new_port(VAR,v7);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v6), k15);
  } else {
    k15 = new_port(VAR,v6);
  }
  if (!k13) {
    node_create(n6, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n6), k12);
    } else {
      k12 = new_port(CON,n6);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k11;
    k19 = k11;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v5), k19);
  } else {
    k19 = new_port(VAR,v5);
  }
  if (k18 != NONE) {
    link(tm, new_port(VAR,v4), k18);
  } else {
    k18 = new_port(VAR,v4);
  }
  if (!k17) {
    node_create(n5, new_pair(k18,k19));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n5), k11);
    } else {
      k11 = new_port(DUP,n5);
    }
  }
  if (!k9) {
    node_create(n4, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n4), k8);
    } else {
      k8 = new_port(CON,n4);
    }
  }
  bool k20 = 0;
  Pair k21 = emptyPair;
  Port k22 = NONE;
  Port k23 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k20 = 1;
    k21 = node_take(k7);
    k22 = k21.fst;
    k23 = k21.snd;
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,v3), k23);
  } else {
    k23 = new_port(VAR,v3);
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v2), k22);
  } else {
    k22 = new_port(VAR,v2);
  }
  if (!k20) {
    node_create(n3, new_pair(k22,k23));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k24 = 0;
  Port k25 = NONE;
  Port k26 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = k3;
    k26 = k3;
  }
  if (k26 != NONE) {
    link(tm, new_port(VAR,v1), k26);
  } else {
    k26 = new_port(VAR,v1);
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v0), k25);
  } else {
    k25 = new_port(VAR,v0);
  }
  if (!k24) {
    node_create(n1, new_pair(k25,k26));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n9, new_pair(new_port(VAR,v2),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v4),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_flow), new_port(CON,n7));
  node_create(nc, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nb, new_pair(new_port(VAR,v5),new_port(CON,nc)));
  node_create(na, new_pair(new_port(VAR,v1),new_port(CON,nb)));
  link(tm, new_ref(CALL_flow), new_port(CON,na));
  return TRUE;
}

bool CALL_flow__C0(TM *tm, Port a, Port b);
bool CALL_flow(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_flow__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp(TM *tm, Port a, Port b);
bool CALL_flow__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v6), k12);
  } else {
    k12 = new_port(VAR,v6);
  }
  bool k13 = 0;
  Port k14 = NONE;
  Port k15 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = k11;
    k15 = k11;
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v5), k15);
  } else {
    k15 = new_port(VAR,v5);
  }
  if (k14 != NONE) {
    link(tm, new_port(VAR,v4), k14);
  } else {
    k14 = new_port(VAR,v4);
  }
  if (!k13) {
    node_create(n6, new_pair(k14,k15));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n6), k11);
    } else {
      k11 = new_port(DUP,n6);
    }
  }
  if (!k9) {
    node_create(n5, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n5), k8);
    } else {
      k8 = new_port(CON,n5);
    }
  }
  bool k16 = 0;
  Pair k17 = emptyPair;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k16 = 1;
    k17 = node_take(k7);
    k18 = k17.fst;
    k19 = k17.snd;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v3), k19);
  } else {
    k19 = new_port(VAR,v3);
  }
  if (k18 != NONE) {
    link(tm, new_port(VAR,v2), k18);
  } else {
    k18 = new_port(VAR,v2);
  }
  if (!k16) {
    node_create(n4, new_pair(k18,k19));
    if (k7 != NONE) {
      link(tm, new_port(CON,n4), k7);
    } else {
      k7 = new_port(CON,n4);
    }
  }
  if (!k5) {
    node_create(n3, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n3), k4);
    } else {
      k4 = new_port(CON,n3);
    }
  }
  bool k20 = 0;
  Port k21 = NONE;
  Port k22 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = k3;
    k22 = k3;
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v1), k22);
  } else {
    k22 = new_port(VAR,v1);
  }
  bool k23 = 0;
  Port k24 = NONE;
  // fast oper
  if (get_tag(k21) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k23 = 1;
    k24 = new_num(operate(k21, new_num(0x00000024)));
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,v0), k24);
  } else {
    k24 = new_port(VAR,v0);
  }
  if (!k23) {
    node_create(n2, new_pair(new_num(0x00000024),k24));
    if (k21 != NONE) {
      link(tm, new_port(OPR, n2), k21);
    } else {
      k21 = new_port(OPR, n2);
    }
  }
  if (!k20) {
    node_create(n1, new_pair(k21,k22));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n9, new_pair(new_port(VAR,v7),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v4),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_down), new_port(CON,n7));
  node_create(nd, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nc, new_pair(new_port(VAR,v2),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v5),new_port(CON,nc)));
  node_create(na, new_pair(new_port(VAR,v1),new_port(CON,nb)));
  link(tm, new_ref(CALL_warp), new_port(CON,na));
  return TRUE;
}

bool CALL_gen__bend0(TM *tm, Port a, Port b);
bool CALL_gen(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  if (k4 != NONE) {
    link(tm, new_port(VAR,v1), k4);
  } else {
    k4 = new_port(VAR,v1);
  }
  if (k3 != NONE) {
    link(tm, new_port(VAR,v0), k3);
  } else {
    k3 = new_port(VAR,v0);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n2, new_pair(new_num(0x00000001),new_port(VAR,v1)));
  node_create(n1, new_pair(new_port(VAR,v0),new_port(CON,n2)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,n1));
  return TRUE;
}

bool CALL_gen__bend0__C0(TM *tm, Port a, Port b);
bool CALL_gen__bend0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  if (k4 != NONE) {
    link(tm, new_port(VAR,v2), k4);
  } else {
    k4 = new_port(VAR,v2);
  }
  bool k5 = 0;
  Port k6 = NONE;
  Port k7 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k5 = 1;
    k6 = k3;
    k7 = k3;
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  node_create(n5, new_pair(new_port(VAR,v0),new_port(VAR,v0)));
  node_create(n4, new_pair(erase,new_port(CON,n5)));
  node_create(n3, new_pair(new_port(CON,n4),new_ref(CALL_gen__bend0__C0)));
  node_create(n6, new_pair(new_port(VAR,v1),new_port(VAR,v2)));
  node_create(n2, new_pair(new_port(CON,n3),new_port(CON,n6)));
  if (k6 != NONE) {
    link(tm, new_port(SWI,n2), k6);
  } else {
    k6 = new_port(SWI,n2);
  }
  if (!k5) {
    node_create(n1, new_pair(k6,k7));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_gen__bend0__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v5), k16);
  } else {
    k16 = new_port(VAR,v5);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v4), k15);
  } else {
    k15 = new_port(VAR,v4);
  }
  if (!k13) {
    node_create(na, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,na), k12);
    } else {
      k12 = new_port(CON,na);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k11;
    k19 = k11;
  }
  bool k20 = 0;
  Port k21 = NONE;
  // fast oper
  if (get_tag(k19) == NUM && get_tag(new_num(0x00000047)) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = new_num(operate(k19, new_num(0x00000047)));
  }
  if (k21 != NONE) {
    link(tm, new_port(VAR,v3), k21);
  } else {
    k21 = new_port(VAR,v3);
  }
  if (!k20) {
    node_create(n9, new_pair(new_num(0x00000047),k21));
    if (k19 != NONE) {
      link(tm, new_port(OPR, n9), k19);
    } else {
      k19 = new_port(OPR, n9);
    }
  }
  bool k22 = 0;
  Port k23 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x00000047)) == NUM) {
    tm->itrs += 1;
    k22 = 1;
    k23 = new_num(operate(k18, new_num(0x00000047)));
  }
  bool k24 = 0;
  Port k25 = NONE;
  // fast oper
  if (get_tag(k23) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = new_num(operate(k23, new_num(0x00000024)));
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v2), k25);
  } else {
    k25 = new_port(VAR,v2);
  }
  if (!k24) {
    node_create(n8, new_pair(new_num(0x00000024),k25));
    if (k23 != NONE) {
      link(tm, new_port(OPR, n8), k23);
    } else {
      k23 = new_port(OPR, n8);
    }
  }
  if (!k22) {
    node_create(n7, new_pair(new_num(0x00000047),k23));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n7), k18);
    } else {
      k18 = new_port(OPR, n7);
    }
  }
  if (!k17) {
    node_create(n6, new_pair(k18,k19));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n6), k11);
    } else {
      k11 = new_port(DUP,n6);
    }
  }
  if (!k9) {
    node_create(n5, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n5), k8);
    } else {
      k8 = new_port(CON,n5);
    }
  }
  bool k26 = 0;
  Port k27 = NONE;
  Port k28 = NONE;
  // fast copy
  if (get_tag(k7) == NUM) {
    tm->itrs += 1;
    k26 = 1;
    k27 = k7;
    k28 = k7;
  }
  bool k29 = 0;
  Port k30 = NONE;
  // fast oper
  if (get_tag(k28) == NUM && get_tag(new_num(0x00000026)) == NUM) {
    tm->itrs += 1;
    k29 = 1;
    k30 = new_num(operate(k28, new_num(0x00000026)));
  }
  if (k30 != NONE) {
    link(tm, new_port(VAR,v1), k30);
  } else {
    k30 = new_port(VAR,v1);
  }
  if (!k29) {
    node_create(n4, new_pair(new_num(0x00000026),k30));
    if (k28 != NONE) {
      link(tm, new_port(OPR, n4), k28);
    } else {
      k28 = new_port(OPR, n4);
    }
  }
  bool k31 = 0;
  Port k32 = NONE;
  // fast oper
  if (get_tag(k27) == NUM && get_tag(new_num(0x00000026)) == NUM) {
    tm->itrs += 1;
    k31 = 1;
    k32 = new_num(operate(k27, new_num(0x00000026)));
  }
  if (k32 != NONE) {
    link(tm, new_port(VAR,v0), k32);
  } else {
    k32 = new_port(VAR,v0);
  }
  if (!k31) {
    node_create(n3, new_pair(new_num(0x00000026),k32));
    if (k27 != NONE) {
      link(tm, new_port(OPR, n3), k27);
    } else {
      k27 = new_port(OPR, n3);
    }
  }
  if (!k26) {
    node_create(n2, new_pair(k27,k28));
    if (k7 != NONE) {
      link(tm, new_port(DUP,n2), k7);
    } else {
      k7 = new_port(DUP,n2);
    }
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  // fast void
  if (get_tag(k3) == ERA || get_tag(k3) == NUM) {
    tm->itrs += 1;
  } else {
    if (k3 != NONE) {
      link(tm, erase, k3);
    } else {
      k3 = erase;
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nc, new_pair(new_port(VAR,v2),new_port(VAR,v4)));
  node_create(nb, new_pair(new_port(VAR,v0),new_port(CON,nc)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,nb));
  node_create(ne, new_pair(new_port(VAR,v3),new_port(VAR,v5)));
  node_create(nd, new_pair(new_port(VAR,v1),new_port(CON,ne)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,nd));
  return TRUE;
}

bool CALL_main__C0(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n0, new_pair(new_num(MAG),new_port(VAR,v0)));
  link(tm, new_ref(CALL_gen), new_port(CON,n0));
  return TRUE;
}

bool CALL_sort(TM *tm, Port a, Port b);
bool CALL_main__C1(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n2, new_pair(new_ref(CALL_main__C0),new_port(VAR,v0)));
  node_create(n1, new_pair(new_num(0x00000001),new_port(CON,n2)));
  node_create(n0, new_pair(new_num(MAG),new_port(CON,n1)));
  link(tm, new_ref(CALL_sort), new_port(CON,n0));
  return TRUE;
}

bool CALL_sort__C0(TM *tm, Port a, Port b);
bool CALL_sort(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_sort__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_sort__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port v8 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  Port nf = node_alloc(tm, &nl);
  Port n10 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !v8 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne || !nf || !n10) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  vars_create(v8, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v6), k12);
  } else {
    k12 = new_port(VAR,v6);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v5), k11);
  } else {
    k11 = new_port(VAR,v5);
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k7);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v4), k16);
  } else {
    k16 = new_port(VAR,v4);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v3), k15);
  } else {
    k15 = new_port(VAR,v3);
  }
  if (!k13) {
    node_create(n5, new_pair(k15,k16));
    if (k7 != NONE) {
      link(tm, new_port(CON,n5), k7);
    } else {
      k7 = new_port(CON,n5);
    }
  }
  if (!k5) {
    node_create(n4, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n4), k4);
    } else {
      k4 = new_port(CON,n4);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k3;
    k19 = k3;
  }
  bool k20 = 0;
  Port k21 = NONE;
  Port k22 = NONE;
  // fast copy
  if (get_tag(k19) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = k19;
    k22 = k19;
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v2), k22);
  } else {
    k22 = new_port(VAR,v2);
  }
  if (k21 != NONE) {
    link(tm, new_port(VAR,v1), k21);
  } else {
    k21 = new_port(VAR,v1);
  }
  if (!k20) {
    node_create(n3, new_pair(k21,k22));
    if (k19 != NONE) {
      link(tm, new_port(DUP,n3), k19);
    } else {
      k19 = new_port(DUP,n3);
    }
  }
  bool k23 = 0;
  Port k24 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k23 = 1;
    k24 = new_num(operate(k18, new_num(0x00000024)));
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,v0), k24);
  } else {
    k24 = new_port(VAR,v0);
  }
  if (!k23) {
    node_create(n2, new_pair(new_num(0x00000024),k24));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n2), k18);
    } else {
      k18 = new_port(OPR, n2);
    }
  }
  if (!k17) {
    node_create(n1, new_pair(k18,k19));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(na, new_pair(new_port(VAR,v7),new_port(VAR,v8)));
  node_create(n9, new_pair(new_port(CON,na),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v5),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_flow), new_port(CON,n7));
  node_create(nd, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nc, new_pair(new_num(0x00000001),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v1),new_port(CON,nc)));
  link(tm, new_ref(CALL_sort), new_port(CON,nb));
  node_create(n10, new_pair(new_port(VAR,v4),new_port(VAR,v8)));
  node_create(nf, new_pair(new_num(0x00000021),new_port(CON,n10)));
  node_create(ne, new_pair(new_port(VAR,v2),new_port(CON,nf)));
  link(tm, new_ref(CALL_sort), new_port(CON,ne));
  return TRUE;
}

bool CALL_sum__C0(TM *tm, Port a, Port b);
bool CALL_sum(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2 || !n3) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k5 = NONE;
  Port k3 = NONE;
  Port k4 = NONE;
  //fast switch
  if (get_tag(b) == CON) {
    k2 = node_load(b);
    k5 = enter(k2.fst);
    if (get_tag(k5) == NUM) {
      tm->itrs += 3;
      vars_take(v1);
      k1 = 1;
      if (get_u24(get_num(k5)) == 0) {
        node_take(b);
        k3 = k2.snd;
        k4 = erase;
      } else {
        node_store(b, new_pair(new_num(new_u24(get_u24(get_num(k5))-1)), k2.snd));
        k3 = erase;
        k4 = b;
      }
    } else {
      node_store(b, new_pair(k5,k2.snd));
    }
  }
  bool k6 = 0;
  Pair k7 = emptyPair;
  Port k8 = NONE;
  Port k9 = NONE;
  // fast anni
  if (get_tag(k3) == CON && !isEmpty(node_load(k3))) {
    tm->itrs += 1;
    k6 = 1;
    k7 = node_take(k3);
    k8 = k7.fst;
    k9 = k7.snd;
  }
  if (k9 != NONE) {
    link(tm, new_port(VAR,v0), k9);
  } else {
    k9 = new_port(VAR,v0);
  }
  if (k8 != NONE) {
    link(tm, new_port(VAR,v0), k8);
  } else {
    k8 = new_port(VAR,v0);
  }
  if (!k6) {
    node_create(n3, new_pair(k8,k9));
    if (k3 != NONE) {
      link(tm, new_port(CON,n3), k3);
    } else {
      k3 = new_port(CON,n3);
    }
  }
  if (k4 != NONE) {
    link(tm, new_ref(CALL_sum__C0), k4);
  } else {
    k4 = new_ref(CALL_sum__C0);
  }
  if (!k1) {
    node_create(n0, new_pair(new_port(SWI,n1),new_port(VAR,v1)));
    node_create(n1, new_pair(new_port(CON,n2),new_port(VAR,v1)));
    node_create(n2, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON, n0), b);
    } else {
      b = new_port(CON, n0);
    }
  }
  return TRUE;
}

bool CALL_sum__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  if (k8 != NONE) {
    link(tm, new_port(VAR,v4), k8);
  } else {
    k8 = new_port(VAR,v4);
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k7);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v2), k11);
  } else {
    k11 = new_port(VAR,v2);
  }
  if (!k9) {
    node_create(n3, new_pair(k11,k12));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k13 = 0;
  Port k14 = NONE;
  Port k15 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = k3;
    k15 = k3;
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v1), k15);
  } else {
    k15 = new_port(VAR,v1);
  }
  if (k14 != NONE) {
    link(tm, new_port(VAR,v0), k14);
  } else {
    k14 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n1, new_pair(k14,k15));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n7, new_pair(new_port(VAR,v5),new_port(VAR,v4)));
  node_create(n6, new_pair(new_num(0x00000080),new_port(OPR,n7)));
  node_create(n5, new_pair(new_port(VAR,v2),new_port(OPR,n6)));
  node_create(n4, new_pair(new_port(VAR,v0),new_port(CON,n5)));
  link(tm, new_ref(CALL_sum), new_port(CON,n4));
  node_create(n9, new_pair(new_port(VAR,v3),new_port(VAR,v5)));
  node_create(n8, new_pair(new_port(VAR,v1),new_port(CON,n9)));
  link(tm, new_ref(CALL_sum), new_port(CON,n8));
  return TRUE;
}

bool CALL_swap__C0(TM *tm, Port a, Port b);
bool CALL_swap__C1(TM *tm, Port a, Port b);
bool CALL_swap(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v2), k12);
  } else {
    k12 = new_port(VAR,v2);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v0), k11);
  } else {
    k11 = new_port(VAR,v0);
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  if (!k5) {
    node_create(n5, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n5), k4);
    } else {
      k4 = new_port(CON,n5);
    }
  }
  node_create(n2, new_pair(new_ref(CALL_swap__C0),new_ref(CALL_swap__C1)));
  node_create(n4, new_pair(new_port(VAR,v1),new_port(VAR,v2)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n3)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_swap__C0(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v0), k12);
  } else {
    k12 = new_port(VAR,v0);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n2, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n2), k8);
    } else {
      k8 = new_port(CON,n2);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  if (k3 != NONE) {
    link(tm, new_port(VAR,v0), k3);
  } else {
    k3 = new_port(VAR,v0);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_swap__C1(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2 || !n3) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v1), k16);
  } else {
    k16 = new_port(VAR,v1);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v0), k15);
  } else {
    k15 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n3, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n3), k12);
    } else {
      k12 = new_port(CON,n3);
    }
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n2, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n2), k8);
    } else {
      k8 = new_port(CON,n2);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v0), k7);
  } else {
    k7 = new_port(VAR,v0);
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  // fast void
  if (get_tag(k3) == ERA || get_tag(k3) == NUM) {
    tm->itrs += 1;
  } else {
    if (k3 != NONE) {
      link(tm, erase, k3);
    } else {
      k3 = erase;
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp__C0(TM *tm, Port a, Port b);
bool CALL_warp__C1(TM *tm, Port a, Port b);
bool CALL_warp(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v3), k16);
  } else {
    k16 = new_port(VAR,v3);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v0), k15);
  } else {
    k15 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n8, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n8), k12);
    } else {
      k12 = new_port(CON,n8);
    }
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n7, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n7), k8);
    } else {
      k8 = new_port(CON,n7);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n6, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n6), k4);
    } else {
      k4 = new_port(CON,n6);
    }
  }
  node_create(n2, new_pair(new_ref(CALL_warp__C0),new_ref(CALL_warp__C1)));
  node_create(n5, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n4, new_pair(new_port(VAR,v1),new_port(CON,n5)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n3)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v5), k12);
  } else {
    k12 = new_port(VAR,v5);
  }
  bool k13 = 0;
  Port k14 = NONE;
  // fast oper
  if (get_tag(k11) == NUM && get_tag(new_num(0x00000080)) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = new_num(operate(k11, new_num(0x00000080)));
  }
  bool k15 = 0;
  Port k16 = NONE;
  // fast oper
  if (get_tag(k14) == NUM && get_tag(new_port(VAR,v2)) == NUM) {
    tm->itrs += 1;
    k15 = 1;
    k16 = new_num(operate(k14, new_port(VAR,v2)));
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v4), k16);
  } else {
    k16 = new_port(VAR,v4);
  }
  if (!k15) {
    node_create(n8, new_pair(new_port(VAR,v2),k16));
    if (k14 != NONE) {
      link(tm, new_port(OPR, n8), k14);
    } else {
      k14 = new_port(OPR, n8);
    }
  }
  if (!k13) {
    node_create(n7, new_pair(new_num(0x00000080),k14));
    if (k11 != NONE) {
      link(tm, new_port(OPR, n7), k11);
    } else {
      k11 = new_port(OPR, n7);
    }
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k7) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k7;
    k19 = k7;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v3), k19);
  } else {
    k19 = new_port(VAR,v3);
  }
  bool k20 = 0;
  Port k21 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x000001e0)) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = new_num(operate(k18, new_num(0x000001e0)));
  }
  bool k22 = 0;
  Port k23 = NONE;
  // fast oper
  if (get_tag(k21) == NUM && get_tag(new_port(VAR,v0)) == NUM) {
    tm->itrs += 1;
    k22 = 1;
    k23 = new_num(operate(k21, new_port(VAR,v0)));
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,v2), k23);
  } else {
    k23 = new_port(VAR,v2);
  }
  if (!k22) {
    node_create(n5, new_pair(new_port(VAR,v0),k23));
    if (k21 != NONE) {
      link(tm, new_port(OPR, n5), k21);
    } else {
      k21 = new_port(OPR, n5);
    }
  }
  if (!k20) {
    node_create(n4, new_pair(new_num(0x000001e0),k21));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n4), k18);
    } else {
      k18 = new_port(OPR, n4);
    }
  }
  if (!k17) {
    node_create(n3, new_pair(k18,k19));
    if (k7 != NONE) {
      link(tm, new_port(DUP,n3), k7);
    } else {
      k7 = new_port(DUP,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k24 = 0;
  Port k25 = NONE;
  Port k26 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = k3;
    k26 = k3;
  }
  if (k26 != NONE) {
    link(tm, new_port(VAR,v1), k26);
  } else {
    k26 = new_port(VAR,v1);
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v0), k25);
  } else {
    k25 = new_port(VAR,v0);
  }
  if (!k24) {
    node_create(n1, new_pair(k25,k26));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nb, new_pair(new_port(VAR,v1),new_port(VAR,v5)));
  node_create(na, new_pair(new_port(VAR,v3),new_port(CON,nb)));
  node_create(n9, new_pair(new_port(VAR,v4),new_port(CON,na)));
  link(tm, new_ref(CALL_swap), new_port(CON,n9));
  return TRUE;
}

bool CALL_warp__C1(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port v8 = vars_alloc(tm, &vl);
  Port v9 = vars_alloc(tm, &vl);
  Port va = vars_alloc(tm, &vl);
  Port vb = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  Port nf = node_alloc(tm, &nl);
  Port n10 = node_alloc(tm, &nl);
  Port n11 = node_alloc(tm, &nl);
  Port n12 = node_alloc(tm, &nl);
  Port n13 = node_alloc(tm, &nl);
  Port n14 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !v8 || !v9 || !va || !vb || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne || !nf || !n10 || !n11 || !n12 || !n13 || !n14) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  vars_create(v8, NONE);
  vars_create(v9, NONE);
  vars_create(va, NONE);
  vars_create(vb, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  bool k17 = 0;
  Pair k18 = emptyPair;
  Port k19 = NONE;
  Port k20 = NONE;
  // fast anni
  if (get_tag(k16) == CON && !isEmpty(node_load(k16))) {
    tm->itrs += 1;
    k17 = 1;
    k18 = node_take(k16);
    k19 = k18.fst;
    k20 = k18.snd;
  }
  bool k21 = 0;
  Pair k22 = emptyPair;
  Port k23 = NONE;
  Port k24 = NONE;
  // fast anni
  if (get_tag(k20) == CON && !isEmpty(node_load(k20))) {
    tm->itrs += 1;
    k21 = 1;
    k22 = node_take(k20);
    k23 = k22.fst;
    k24 = k22.snd;
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,vb), k24);
  } else {
    k24 = new_port(VAR,vb);
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,va), k23);
  } else {
    k23 = new_port(VAR,va);
  }
  if (!k21) {
    node_create(na, new_pair(k23,k24));
    if (k20 != NONE) {
      link(tm, new_port(CON,na), k20);
    } else {
      k20 = new_port(CON,na);
    }
  }
  bool k25 = 0;
  Pair k26 = emptyPair;
  Port k27 = NONE;
  Port k28 = NONE;
  // fast anni
  if (get_tag(k19) == CON && !isEmpty(node_load(k19))) {
    tm->itrs += 1;
    k25 = 1;
    k26 = node_take(k19);
    k27 = k26.fst;
    k28 = k26.snd;
  }
  if (k28 != NONE) {
    link(tm, new_port(VAR,v9), k28);
  } else {
    k28 = new_port(VAR,v9);
  }
  if (k27 != NONE) {
    link(tm, new_port(VAR,v8), k27);
  } else {
    k27 = new_port(VAR,v8);
  }
  if (!k25) {
    node_create(n9, new_pair(k27,k28));
    if (k19 != NONE) {
      link(tm, new_port(CON,n9), k19);
    } else {
      k19 = new_port(CON,n9);
    }
  }
  if (!k17) {
    node_create(n8, new_pair(k19,k20));
    if (k16 != NONE) {
      link(tm, new_port(CON,n8), k16);
    } else {
      k16 = new_port(CON,n8);
    }
  }
  bool k29 = 0;
  Port k30 = NONE;
  Port k31 = NONE;
  // fast copy
  if (get_tag(k15) == NUM) {
    tm->itrs += 1;
    k29 = 1;
    k30 = k15;
    k31 = k15;
  }
  if (k31 != NONE) {
    link(tm, new_port(VAR,v7), k31);
  } else {
    k31 = new_port(VAR,v7);
  }
  if (k30 != NONE) {
    link(tm, new_port(VAR,v6), k30);
  } else {
    k30 = new_port(VAR,v6);
  }
  if (!k29) {
    node_create(n7, new_pair(k30,k31));
    if (k15 != NONE) {
      link(tm, new_port(DUP,n7), k15);
    } else {
      k15 = new_port(DUP,n7);
    }
  }
  if (!k13) {
    node_create(n6, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n6), k12);
    } else {
      k12 = new_port(CON,n6);
    }
  }
  bool k32 = 0;
  Pair k33 = emptyPair;
  Port k34 = NONE;
  Port k35 = NONE;
  // fast anni
  if (get_tag(k11) == CON && !isEmpty(node_load(k11))) {
    tm->itrs += 1;
    k32 = 1;
    k33 = node_take(k11);
    k34 = k33.fst;
    k35 = k33.snd;
  }
  if (k35 != NONE) {
    link(tm, new_port(VAR,v5), k35);
  } else {
    k35 = new_port(VAR,v5);
  }
  if (k34 != NONE) {
    link(tm, new_port(VAR,v4), k34);
  } else {
    k34 = new_port(VAR,v4);
  }
  if (!k32) {
    node_create(n5, new_pair(k34,k35));
    if (k11 != NONE) {
      link(tm, new_port(CON,n5), k11);
    } else {
      k11 = new_port(CON,n5);
    }
  }
  if (!k9) {
    node_create(n4, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n4), k8);
    } else {
      k8 = new_port(CON,n4);
    }
  }
  bool k36 = 0;
  Pair k37 = emptyPair;
  Port k38 = NONE;
  Port k39 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k36 = 1;
    k37 = node_take(k7);
    k38 = k37.fst;
    k39 = k37.snd;
  }
  if (k39 != NONE) {
    link(tm, new_port(VAR,v3), k39);
  } else {
    k39 = new_port(VAR,v3);
  }
  if (k38 != NONE) {
    link(tm, new_port(VAR,v2), k38);
  } else {
    k38 = new_port(VAR,v2);
  }
  if (!k36) {
    node_create(n3, new_pair(k38,k39));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k40 = 0;
  Port k41 = NONE;
  Port k42 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k40 = 1;
    k41 = k3;
    k42 = k3;
  }
  if (k42 != NONE) {
    link(tm, new_port(VAR,v1), k42);
  } else {
    k42 = new_port(VAR,v1);
  }
  if (k41 != NONE) {
    link(tm, new_port(VAR,v0), k41);
  } else {
    k41 = new_port(VAR,v0);
  }
  if (!k40) {
    node_create(n1, new_pair(k41,k42));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nf, new_pair(new_port(VAR,v9),new_port(VAR,vb)));
  node_create(ne, new_pair(new_port(VAR,v3),new_port(CON,nf)));
  node_create(nd, new_pair(new_port(VAR,v5),new_port(CON,ne)));
  node_create(nc, new_pair(new_port(VAR,v7),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v1),new_port(CON,nc)));
  link(tm, new_ref(CALL_warp), new_port(CON,nb));
  node_create(n14, new_pair(new_port(VAR,v8),new_port(VAR,va)));
  node_create(n13, new_pair(new_port(VAR,v2),new_port(CON,n14)));
  node_create(n12, new_pair(new_port(VAR,v4),new_port(CON,n13)));
  node_create(n11, new_pair(new_port(VAR,v6),new_port(CON,n12)));
  node_create(n10, new_pair(new_port(VAR,v0),new_port(CON,n11)));
  link(tm, new_ref(CALL_warp), new_port(CON,n10));
  return TRUE;
}

void hvm_c(u64* book_buffer) {
  Port answer = NONE;
  
  // Creates static TMs
  alloc_static_tms();

  // Starts the timer
  u64 start = time64();

  // GMem
  globalNet = malloc(sizeof(Net));
  net_init();

  // Creates an initial redex that calls main
  CALL_main(tm[0], new_ref(CALL_main), new_port(VAR, (Port)&answer));

  // Normalizes and runs IO
  normalize();

  // Prints the result
  printf("Result: ");
  pretty_print_port(enter(new_port(VAR, (Port)&answer)));
  printf("\n");

  // Stops the timer
  double duration = (time64() - start) / 1000000000.0; // seconds

  // Prints interactions and time
  u64 itrs = atomic_load(&globalNet->itrs);
  printf("- ITRS: %" PRIu64 "\n", itrs);
  printf("- TIME: %.2fs\n", duration);
  printf("- MIPS: %.2f\n", (double)itrs / duration / 1000000.0);

  // Frees everything
  free_static_tms();
  free(globalNet);
}

int main() {
  printf("Port size: %lu\n", sizeof(Port));
  hvm_c((u64*)NULL);
  return 0;
}
