//#include <dlfcn.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runtime.h"

// Configuration
// -------------

Port erase = ERA;
Pair emptyPair = {FREE, FREE};
u8 isEmpty(Pair p) {
  return p.fst == 0 && p.snd == 0;
}

// Numbers
static const u64 U24_MAX = ((u64)1 << 56) - 1;
static const u64 U24_MIN = 0.0;
static const i64 I24_MAX = ((i64)1 << 55) - 1;
static const i64 I24_MIN = (i64) ((u64)-1 << 55);

// Global Net
Net *globalNet;


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

Port new_num(Port val) {
  return (val << TAG_SIZE) | NUM;
}

Val get_val(Port port) {
  return port >> TAG_SIZE;
}

Port new_port(Tag tag, Port val) {
  if (val & TAG_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__); 
    abort();
  }
  return (u64)val | tag;
}

// Keep for type checking
Port new_ref(interactionFn val) {
  if ((u64)val & TAG_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__); 
    abort();
  }
  return (u64)val | REF;
}

Tag get_tag(Port port) {
  if (port & 7)
    return port & TAG_MASK;
  else
    return VAR;
}

// Pair: Constructor and Getters
// -----------------------------

const Pair new_pair(Port fst, Port snd) {
  return (Pair){fst, snd};
}

// Utils
// -----

// Swaps two ports.
void swap(Port *a, Port *b) {
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
u64 time64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

// Ports / Pairs / Rules
// ---------------------

// Should we swap ports A and B before reducing this rule?
bool should_swap(Port A, Port B) {
  return get_tag(B) < get_tag(A);
}

// Gets a rule's priority
u8 interactionPriority[12][12] = {
  //VAR   REF   ERA   NUM   CON   DUP   OPR   SWI   VAR   RDX   VAL   ARG
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE}, // VAR
  {TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,FALSE,TRUE, FALSE,FALSE,FALSE}, // REF
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,TRUE }, // ERA
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,TRUE, FALSE,FALSE,FALSE}, // NUM
  {TRUE, FALSE,TRUE, TRUE, TRUE, FALSE,FALSE,FALSE,TRUE, FALSE,FALSE,FALSE}, // CON
  {TRUE, FALSE,TRUE, TRUE, FALSE,TRUE, FALSE,FALSE,TRUE, FALSE,FALSE,TRUE }, // DUP
  {TRUE, FALSE,TRUE, FALSE,FALSE,FALSE,TRUE, FALSE,TRUE, FALSE,FALSE,FALSE}, // OPR
  {TRUE, FALSE,TRUE, FALSE,FALSE,FALSE,FALSE,TRUE, TRUE, FALSE,FALSE,FALSE}, // SWI
  {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,FALSE,FALSE}, // VAR
  {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}, // RDX
  {FALSE,FALSE,TRUE ,FALSE,FALSE,TRUE ,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}, // VAL
  {FALSE,FALSE,TRUE ,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,TRUE }  // ARG
};

bool is_high_priority(Pair AB) {
  return interactionPriority[get_tag(AB.fst)][get_tag(AB.snd)];
}

// Numbs
// -----

// Constructor and getters for SYM (operation selector)
Numb new_sym(u64 val) {
  return (val << NUM_TAG_SIZE) | TY_SYM;
}

u64 get_sym(Numb word) {
  return (word >> NUM_TAG_SIZE);
}

// Constructor and getters for U24 (unsigned 24-bit integer)
Numb new_u24(u64 val) {
  return (val << NUM_TAG_SIZE) | TY_U24;
}

u64 get_u24(Numb word) {
  return word >> NUM_TAG_SIZE;
}

// Constructor and getters for I24 (signed 24-bit integer)
Numb new_i24(i64 val) {
  return ((u64)val) << NUM_TAG_SIZE | TY_I24;
}

i64 get_i24(Numb word) {
  return ((i64)word) << TAG_SIZE >> (TAG_SIZE + NUM_TAG_SIZE);
}

// Constructor and getters for F24 (24-bit float)
Numb new_f24(float val) {
  u64 bits = *(u64*)&val;
  u64 shifted_bits = bits >> 8;
  u64 lost_bits = bits & 0xFF;
  // round ties to even
  shifted_bits += (!isnan(val)) & ((lost_bits - ((lost_bits >> 7) & !shifted_bits)) >> 7);
  // ensure NaNs don't become infinities
  shifted_bits |= isnan(val);
  return (shifted_bits << NUM_TAG_SIZE) | TY_F24;
}

f64 get_f24(Numb word) {
  u64 bits = (word << TAG_SIZE) & 0xFFFFFFFFFFFFFF00;
  return *(f64*)&bits;
}

// Flip flag
Tag get_typ(Numb word) {
  return word & 0x1F;
}

bool is_num(Numb word) {
  return get_typ(word) >= TY_U24 && get_typ(word) <= TY_F24;
}

bool is_cast(Numb word) {
  return get_typ(word) == TY_SYM && get_sym(word) >= TY_U24 && get_sym(word) <= TY_F24;
}

// Partial application
Numb partial(Numb a, Numb b) {
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
Numb cast(Numb a, Numb b) {
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
Numb operate(Numb a, Numb b) {
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

void push_redex(TM* tm, Pair redex) {
  if (is_high_priority(redex)) {
    tm->hbag_buf[tm->hput++] = redex;
  } else {
    atomic_store_explicit(&globalNet->rbag_buf[tm->tid*(G_RBAG_LEN/TPC) + (tm->rput++)], redex, memory_order_relaxed);
  }
}

Pair pop_redex(TM* tm) {
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

u32 rbag_len(TM* tm) {
  return tm->rput + tm->hput;
}

// TM
// --

TM* tms[TPC];

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
    tms[t] = tm_new(t);
  }
}

void free_static_tms() {
  for (u32 t = 0; t < TPC; ++t) {
    free(tms[t]);
  }
}

// Net
// ----

// Stores a new node on global.
void node_create(Port loc, Pair val) {
  atomic_store_explicit((APair*)((u64)loc & ~TAG_MASK), val, memory_order_relaxed);
}

// Stores a var on global.
void vars_create(Port var, Port val) {
  atomic_store_explicit((APort*)var, val, memory_order_relaxed);
}

// Reads a node from global.
Pair node_load(Port loc) {
  return atomic_load_explicit((APair*)((u64)loc & ~TAG_MASK), memory_order_relaxed);
}

// Reads a var from global.
Port vars_load(Port var) {
  return atomic_load_explicit((APort*)var, memory_order_relaxed);
}

// Stores a node on global.
void node_store(Port loc, Pair val) {
  atomic_store_explicit((APair*)((u64)loc & ~TAG_MASK), val, memory_order_relaxed);
}

// Exchanges a node on global by a value. Returns old.
Pair node_exchange(Port loc, Pair val) {
  return atomic_exchange_explicit((APair*)((u64)loc & ~TAG_MASK), val, memory_order_relaxed);
}

// Exchanges a var on global by a value. Returns old.
Port vars_exchange(Port var, Port val) {
  Port p = var;
  // while (p != FREE && p != NONE && get_tag(p) == VAR) {
  // fprintf(stderr, "vars_exchange: %d var: %p val: %p\n", __LINE__, (void *)var, (void *)val);
    p = atomic_exchange_explicit((APort*)p, val, memory_order_relaxed);
    // fprintf(stderr, "old-val: %p\n", (void *)p);
    // }
  return p;
}

// Takes a node.
Pair node_take(Port loc) {
  return node_exchange(loc, emptyPair);
}

// Takes a var.
Port vars_take(Port var) {
  return vars_exchange(var, FREE);
}


// Net
// ---

// Initializes a net.
void net_init() {
  // is that needed?
  atomic_store(&globalNet->itrs, 0);
  atomic_store(&globalNet->idle, 0);
}

// Allocator
// ---------

Port node_alloc(TM* tm) {
  while (TRUE) {
    u32 lc = tm->tid*(G_NODE_LEN/TPC) + (tm->nput%(G_NODE_LEN/TPC));
    Pair* elem = (Pair *)&globalNet->node_buf[lc];
    tm->nput += 1;
    if (lc > 0 && isEmpty(*elem)) {
      return (Port)elem;
    }
  }
}

Port vars_alloc(TM* tm) {
  while (TRUE) {
    u32 lc = tm->tid*(G_NODE_LEN/TPC) + (tm->vput%(G_NODE_LEN/TPC));
    Port* elem = (Port*)&globalNet->vars_buf[lc];
    tm->vput += 1;
    if (lc > 0 && *elem == FREE) {
      return (Port)elem;
    }
  }
}

Port vars_make(TM *tm, Port p) {
  Port v0 = vars_alloc(tm);
  vars_create(v0, p);
  return v0;
}

Port node_make(TM *tm, Port fst, Port snd) {
  Port n0 = node_alloc(tm);
  node_create(n0, new_pair(fst, snd));
  return n0;
}

// Linking
// -------

// Finds a variable's value.
Port enter(Port var) {
  // While `var` is VAR: extend it (as an optimization)
  while (get_tag(var) == VAR) {
    // Takes the current `var` substitution as `val`
    Port val = vars_exchange(var, NONE);
    // If there was no `val`, stop, as there is no extension
    if (get_tag(val) == RDX) {
      vars_exchange(var, val);
      break;
    } else if (val == NONE || val == FREE) {
      break;
    }
    // Otherwise, delete `B` (we own both) and continue
    vars_take(var);
    var = val;
  }
  return var;
}

// Atomically Links `A ~ B`.
void link(TM* tm, Port A, Port B) {
  // fprintf(stderr, "link: %d A: %p B: %p\n", __LINE__, (void *)A, (void *)B);

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
      if (A_ == NONE || A_ == FREE) {
        break;
      } else if (get_tag(B) == ERA) {
	link(tm, A_, B);
	break;
      } else if (get_tag(A_) == RDX) {
	push_redex(tm, node_take(A_));
	break;
      } else if (get_tag(B) == RDX && get_tag(A_) != VAR) {
	vars_exchange(A, A_);
	Pair rdx = node_take(B);
	push_redex(tm, rdx);
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
void link_pair(TM* tm, Pair AB) {
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
  Pair pr;

  switch(get_tag(b)) {
  case ARG:
    fnPtr = (interactionFn)(a & ~TAG_MASK);
    return fnPtr(tm, a, b);
    break;
    
  case DUP:
    if (get_tag(a) != REF) {
      // TODO: ever happen?
      fprintf(stderr, "Oooopsie line: %d\n", __LINE__);
      abort();
    }
    pr = node_take(b);
    link(tm, a, pr.fst);
    link(tm, a, pr.snd);
    return TRUE;
    break;

  default:
    printf("unhandled tag 0x%x line: %d\n", get_tag(b), __LINE__);
    abort();
    break;
  }
  return FALSE;
}

// The Void Interaction.
bool VOID(TM* tm, Port a, Port b) {
  return TRUE;
}

// The Eras Interaction.
bool ERAS(TM* tm, Port a, Port b) {
  if (get_val(b) == FREE) {
    return TRUE;
  }

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
bool ANNI(TM* tm, Port a, Port b) {
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
  // fprintf(stderr, "ANNI: %d A1: %p B1: %p\n", __LINE__, (void *)A1, (void *)B1);
  // fprintf(stderr, "          A2: %p B2: %p\n", (void *)A2, (void *)B2);
  link_pair(tm, new_pair(A1, B1));
  link_pair(tm, new_pair(A2, B2));

  return TRUE;
}

// The Comm Interaction.
bool COMM(TM* tm, Port a, Port b) {
  Port v0 = vars_alloc(tm);
  Port v1 = vars_alloc(tm);
  Port v2 = vars_alloc(tm);
  Port v3 = vars_alloc(tm);
  Port n0 = node_alloc(tm);
  Port n1 = node_alloc(tm);
  Port n2 = node_alloc(tm);
  Port n3 = node_alloc(tm);

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
  node_create(n0, new_pair(v0, v1));
  node_create(n1, new_pair(v2, v3));
  node_create(n2, new_pair(v0, v2));
  node_create(n3, new_pair(v1, v3));

  // Links.
  link_pair(tm, new_pair(new_port(get_tag(b), n0), A1));
  link_pair(tm, new_pair(new_port(get_tag(b), n1), A2));
  link_pair(tm, new_pair(new_port(get_tag(a), n2), B1));
  link_pair(tm, new_pair(new_port(get_tag(a), n3), B2));

  return TRUE;
}

// The Oper Interaction.
bool OPER(TM* tm, Port a, Port b) {
  // Checks availability
  if (isEmpty(node_load(b))) {
    return FALSE;
  }

  // Loads ports.
  Pair B  = node_take(b);
  Port B1 = B.fst;
  Port B2 = B.snd;
  B2 = enter(B2);

  // Performs operation.
  if (get_tag(B1) == NUM) {
    Numb cv = operate(get_val(a), get_val(B1));
    link(tm, new_num(cv), B2);
  } else {
    link(tm, B1, new_port(OPR, node_make(tm, a, B2)));
  }

  return TRUE;
}

// The Swit Interaction.
bool SWIT(TM* tm, Port a, Port b) {
  Port n0 = node_alloc(tm);
  Port n1 = node_alloc(tm);

  // Checks availability
  if (isEmpty(node_load(b))) {
    return FALSE;
  }

  // Loads ports.
  u64  av = get_u24(get_val(a));
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

bool DUPE(TM* tm, Port a, Port b) {
  incRef((Value *)(b & ~TAG_MASK), 1);
  Pair dupes = node_load(a);
  link(tm, dupes.fst, b);
  link(tm, dupes.snd, b);
  return TRUE;
}

bool DECF(TM* tm, Port a, Port b) {
  dec_and_free((Value *)(b & ~TAG_MASK), 1);
  return TRUE;
}

bool ARGS(TM* tm, Port a, Port b) {
  if (a == ARG && b == ARG) {
    return TRUE;
  } else if (a == ARG || b == ARG) {
    fprintf(stderr, "Implement currying: %p %p\n", (void *)a, (void *)b);
    abort();
  } else {
    return ANNI(tm, a, b);
  }
}

bool ABRT(TM* tm, Port a, Port b) {
  fprintf(stderr, "Bad interaction: 0x%x 0x%x\n", get_tag(a), get_tag(b));
  fprintf(stderr, "a: %p b: %p\n", (void *)a, (void *)b);
  abort();
}

interactionFn interactions[12][12] = {
  //VAR   REF   ERA   NUM   CON   DUP   OPR   SWI   VAR   RDX   VAL   ARG
  {&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&ABRT,&ABRT,&LINK}, // VAR
  {&LINK,&VOID,&VOID,&VOID,&ABRT,&CALL,&ABRT,&ABRT,&LINK,&ABRT,&ABRT,&CALL}, // REF
  {&LINK,&VOID,&VOID,&VOID,&ERAS,&ERAS,&ERAS,&ERAS,&LINK,&ERAS,&DECF,&ERAS}, // ERA
  {&LINK,&VOID,&VOID,&VOID,&ERAS,&ERAS,&OPER,&SWIT,&LINK,&ABRT,&ABRT,&ABRT}, // NUM
  {&LINK,&ABRT,&ERAS,&ERAS,&ANNI,&COMM,&COMM,&COMM,&LINK,&ABRT,&ABRT,&ABRT}, // CON
  {&LINK,&CALL,&ERAS,&ERAS,&COMM,&ANNI,&COMM,&COMM,&LINK,&ABRT,&DUPE,&ABRT}, // DUP
  {&LINK,&ABRT,&ERAS,&OPER,&COMM,&COMM,&ANNI,&COMM,&LINK,&ABRT,&ABRT,&ABRT}, // OPR
  {&LINK,&ABRT,&ERAS,&SWIT,&COMM,&COMM,&COMM,&ANNI,&LINK,&ABRT,&ABRT,&ABRT}, // SWI
  {&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&LINK,&ABRT,&ABRT,&LINK}, // VAR
  {&ABRT,&ABRT,&ERAS,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT}, // RDX
  {&ABRT,&ABRT,&DECF,&ABRT,&ABRT,&DUPE,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT}, // VAL
  {&LINK,&CALL,&ERAS,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ARGS}  // ARG
};

interactionFn get_rule(Port a, Port b) {
  return interactions[get_tag(a)][get_tag(b)];
}

// Pops a local redex and performs a single interaction.
bool interact(TM* tm) {
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
  moveFreeToCentral();
  return NULL;
}

// Evaluates all redexes.
// TODO: cache threads to avoid spawning overhead
void normalize() {
  if (TPC == 1) {
    printf("*** running single threaded\n");
  }
  // Inits thread_arg objects
  ThreadArg thread_arg[TPC];
  for (u32 t = 0; t < TPC; ++t) {
    thread_arg[t].tm = tms[t];
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
        if ((cur & ~TAG_MASK) != 0) {
          printf("%c", (char)cur & ~TAG_MASK);
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
          printf("x%lx", cur & ~TAG_MASK);
        }
        break;
      }
      case NUM: {
        pretty_print_numb(get_val(cur));
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

Port argsNet(TM *tm, NativeArgs *args) {
  Port n0 = node_alloc(tm);
  Port tail = args->args[args->count - 1];
  for (int i = args->count - 2; i >= 0; i--) {
    node_create(n0, new_pair(args->args[i], tail));
    tail = new_port(ARG, n0);
    n0 = node_alloc(tm);
  }
  node_create(n0, new_pair(args->result, tail));
  return new_port(ARG, n0);
}

// extract the requested number of native args
Port nativeArg(TM *tm, Port ref, Port args, NativeArgs *argsStruct) {
  Tag argsTag = get_tag(args);
  Port arg;
  Port n0;
  Pair argsNode;
  Port varVal;
  switch(get_tag(args)) {
  case 0xF:
    // TODO: test this
    fprintf(stderr, "Boom at %s: %d\n", __FILE__, __LINE__);
    abort();
    return NONE;
    break;

  case ARG:
    argsNode = node_take(args);
    arg = argsNode.fst;
    if (get_tag(arg) == VAR) {
      arg = enter(arg);
    }

    switch(get_tag(arg)) {
    case VAL:
    case NUM:
      argsStruct->args[argsStruct->count++] = arg;
      return argsNode.snd;
      break;

    case VAR:
      n0 = node_alloc(tm);
      node_create(n0, argsNode);
      argsStruct->args[argsStruct->count++] = arg;
      argsStruct->args[argsStruct->count++] = n0;
      varVal = vars_exchange(arg, new_port(RDX, node_make(tm, ref, argsNet(tm, argsStruct))));
      if (varVal != NONE && varVal != FREE) {
    // TODO: test this
    fprintf(stderr, "Boom at %s: %d\n", __FILE__, __LINE__);
    abort();
	if (get_tag(varVal) == RDX) {
	  push_redex(tm, node_take(varVal));
	} else {
	  link(tm, ref, varVal);
	  vars_take(arg);
	}
      }
      return NONE;
      break;

      // TODO: what other tags need to be handled
    default:
      printf("unhandled tag 0x%x line: %d\n", get_tag(arg), __LINE__);
      abort();
      break;
    }

  case VAR:
    // TODO: test this
    fprintf(stderr, "Boom at %s: %d\n", __FILE__, __LINE__);
    abort();

    varVal = vars_exchange(arg, new_port(RDX, node_make(tm, ref, arg)));
    if (varVal != NONE && varVal != FREE) {
      link(tm, ref, varVal);
      vars_take(arg);
    }
    return NONE;
    break;

    // TODO: what other tags need to be handled
  default:
    printf("unhandled tag 0x%x line: %d\n", get_tag(arg), __LINE__);
    abort();
    break;
  }
}

Port dupeArg(TM *tm, Port arg, Port dupeArg) {
  Port dupeNode = node_alloc(tm);
  Port dupedVar = vars_alloc(tm);
  node_create(dupeNode, new_pair(dupedVar, dupeArg));
  link(tm, arg, new_port(DUP, dupeNode));
  return dupedVar;
}

/*
void hvm_c(interactionFn mainFn, NativeArgs *args) {
  // Creates static TMs
  alloc_static_tms();

  // GMem
  globalNet = malloc(sizeof(Net));
  net_init();

  // Starts the timer
  u64 start = time64();

  // Creates an initial redex that calls main
  mainFn(tm[0], new_ref(mainFn), argsNet(tm[0], args, 0));

  // Normalizes and runs IO
  normalize();

  // Prints the result
  // printf("Result: ");
  // pretty_print_port(enter(args->result));
  // printf("\n");

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
// */

void make_op(TM *tm, int op, Port x, Port y, Port rslt) {
  if (get_tag(x) == NUM && get_tag(y) == NUM) {
    link(tm, rslt, new_num(operate(get_val(x), get_val(y) & ~0x1F | op)));
  } else {
    link(tm, x,new_port(OPR, node_make(tm, new_port(NUM, op << (NUM_TAG_SIZE + TAG_SIZE)),
				       new_port(OPR, node_make(tm, y, rslt)))));
  }
}

void printArgs(Port args) {
  if (args == ARG) {
    fprintf(stderr, "args: %p\n", (void *)args);
  } else if (get_tag(args) == ARG) {
    Pair pr = node_load(args);;
    fprintf(stderr, "args: %p arg: %p\n", (void *)args, (void *)pr.fst);
    printArgs(pr.snd);
  }
}
