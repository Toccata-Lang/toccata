#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "runtime3.h"
// #include <unistd.h>

// Thread local values
__thread u32 tid;

const Term VOID = 0;

// Global heap
static a64* BUFF     = NULL;
static u64  RNOD_INI = 0;
static u64  RNOD_END = 0;
static u64  RBAG     = 0x1000;
static u64  RBAG_INI = 0;
static u64  RBAG_END = 0;

static Book BOOK = {
  .defs = NULL,
  .len = 0,
  .cap = 0,
};

// TODO: write a time64() function that returns the time as fast as possible as a u64
u64 time64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

// Debugging
char* tag_to_str(Tag tag);
void dump_buff();

void *boom(char *msg, char *file, int line) {
  fprintf(stderr, "%s at %s:%d\n", msg, file, line);
  abort();
}

// Term operations
Term term_new(Tag tag, Lab lab, Loc loc) {
  Term tag_enc = tag;
  Term lab_enc = ((Term)lab) << TAG_SIZE;
  Term loc_enc = ((Term)loc) << (TAG_SIZE + LAB_SIZE);

  return loc_enc | lab_enc | tag_enc;
}

Term term_val(Term val) {
  // ensure a Term is a native value
  if (val & VAL_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__); 
    fprintf(stderr, "val: %p\n", (void *)val);
    abort();
  }
  return val;
}

Tag term_tag(Term term) {
  if ((term & 0xF) == REF) {
    return REF;
  } else if (term & VAL_MASK) {
    return term & TAG_MASK;
  } else {
    return VAL;
  }
}

Lab term_lab(Term term) {
  return (term >> TAG_SIZE) & LAB_MASK;
}

Loc term_loc(Term term) {
  return (term >> (TAG_SIZE + LAB_SIZE)) & LOC_MASK;
}

// Memory operations
Term swap(Loc loc, Term term) {
  return atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
}

Term getAndCheck(Loc loc) {
  Term gotten = atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
  if (term_tag(gotten) == SUB) {
    BOOM("got SUB");
  }
  return gotten;
}

Term get(Loc loc) {
  Term gotten = atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
  return gotten;
}

Term takeAndCheck(Loc loc) {
  Term taken = atomic_exchange_explicit(&BUFF[loc], VOID, memory_order_relaxed);
  if (term_tag(taken) == SUB) {
    BOOM("took SUB");
  }
  return taken;
}

Term take(Loc loc) {
  Term taken = atomic_exchange_explicit(&BUFF[loc], VOID, memory_order_relaxed);
  return taken;
}

void set(Loc loc, Term term) {
  atomic_store_explicit(&BUFF[loc], term, memory_order_relaxed);
}

Loc port(u64 n, Loc x) {
  return n + x - 1;
}

// Allocation
a64 node_count;
int max_node = 0;
Loc alloc_node(u64 arity) {
  Loc loc = RNOD_END;
  RNOD_END += arity;
  /*
  if (loc > 0 && isEmpty(*elem)) {
    int nc = atomic_fetch_add_explicit(&node_count, 1, memory_order_relaxed);
    // fprintf(stderr, "node allocd: %d %p\n", __LINE__, (void *)elem);
    if (max_node < nc)
      max_node = nc;
  }
  // */
  return loc;
}

bool isNegative(Term trm) {
  switch(term_tag(trm)) {
  case SUB:
  case ERA:
  case APP:
  case DUP:
  case OPY:
    return TRUE;

  default:
    return FALSE;
  }
}

bool isPositive(Term trm) {
  switch(term_tag(trm)) {
  case LAZ:
  case VAL:
  case VAR:
  case NUL:
  case LAM:
  case REF:
  case SUP:
  case OPX:
    return TRUE;

  default:
    return FALSE;
  }
}

Term pair_make(Tag tag, Term fst, Term snd) {
  /*
  Port n0;
  while (TRUE) {
    u32 lc = tid*(G_NODE_LEN/TPC) + (tm->nput%(G_NODE_LEN/TPC));
    Pair* elem = (Pair *)&globalNet->node_buf[lc];
    tm->nput += 1;
    if (lc > 0 && isEmpty(*elem)) {
      node_count++;
      if (max_node < node_count)
	max_node = node_count;
      n0 = (Port)elem;
      break;
    }
  }
  // */

  switch(tag) {
  case LAZ:
  case SUB:
  case LAM:
    if (isPositive(fst) || isNegative(snd)) {
      fprintf(stderr, "fst: %s %d  snd: %s %d\n",
	      tag_to_str(term_tag(fst)), isPositive(fst),
	      tag_to_str(term_tag(snd)), isNegative(snd));
      char s[50];
      sprintf(s, "bad %s pair", tag_to_str(tag));
      BOOM(s);
    }
    break;
    
  case APP:
    if (isNegative(fst) || isPositive(snd)) {
      fprintf(stderr, "fst: %s %d  snd: %s %d\n",
	      tag_to_str(term_tag(fst)), isNegative(fst),
	      tag_to_str(term_tag(snd)), isPositive(snd));
      char s[50];
      sprintf(s, "bad %s pair", tag_to_str(tag));
      BOOM(s);
    }
    break;
    
  case DUP:
    if (isPositive(fst) || isPositive(snd)) {
      fprintf(stderr, "fst: %s %d  snd: %s %d\n",
	      tag_to_str(term_tag(fst)), isNegative(fst),
	      tag_to_str(term_tag(snd)), isNegative(snd));
      char s[50];
      sprintf(s, "bad %s pair", tag_to_str(tag));
      BOOM(s);
    }
    break;
    
  default:
    fprintf(stderr, "bad tag: %s\n", tag_to_str(tag));
    BOOM("unhandled tag");
    break;
  }

  // TM *tm = tms[tid];
  Loc loc = alloc_node(2);
  set(port(1, loc), fst);
  set(port(2, loc), snd);
  return term_new(tag, 0, loc);
}

u64 inc_itr() {
  return RBAG_END / 2;
}

Loc rbag_push(Term neg, Term pos) {
  Loc loc = RBAG + RBAG_END;
  RBAG_END += 2;
  set(loc + 0, neg);
  set(loc + 1, pos);
  return loc;
}

Loc rbag_pop() {
  if (RBAG_INI < RBAG_END) {
    Loc loc = RBAG + RBAG_INI;
    RBAG_INI += 2;
    return loc;
  }

  return 0;
}

Loc rbag_ini() {
  return RBAG + RBAG_INI;
}

Loc rbag_end() {
  return RBAG + RBAG_END;
}

Loc rnod_end() {
  return RNOD_END;
}

// Book operations

// Moves the global buffer and redex bag into a new def and resets
// the global buffer and redex bag.
void def_new(char* name) {
  if (BOOK.len == BOOK.cap) {
    if (BOOK.cap == 0) {
      BOOK.cap = 32;
    } else {
      BOOK.cap *= 2;
    }

    BOOK.defs = realloc(BOOK.defs, sizeof(Def) * BOOK.cap);
  }

  Def def = {
    .name = name,
    .nodes = calloc(RNOD_END, sizeof(Term)),
    .nodes_len = RNOD_END,
    .rbag = calloc(RBAG_END, sizeof(Term)),
    .rbag_len = RBAG_END,
  };

  memcpy(def.nodes, BUFF, sizeof(Term) * def.nodes_len);
  memcpy(def.rbag, BUFF + RBAG, sizeof(Term) * def.rbag_len);

  // printf("NEW DEF '%s':\n", def.name);
  // dump_buff();
  // printf("\n");

  memset(BUFF, 0, sizeof(Term) * def.nodes_len);
  memset(BUFF + RBAG, 0, sizeof(Term) * def.rbag_len);

  RNOD_END = 0;
  RBAG_END = 0;

  BOOK.defs[BOOK.len] = def;
  BOOK.len++;
}

char* def_name(Loc def_idx) {
  return BOOK.defs[def_idx].name;
}

// Atomic Linker
void move(Loc neg_loc, u64 pos);

void link(Term neg, Term pos) {
  fprintf(stderr, "linking %d: neg: %p %s pos: %p %s\n", __LINE__,
	  (void *)neg, tag_to_str(term_tag(neg)),
	  (void *)pos, tag_to_str(term_tag(pos)));
  Tag negTag = term_tag(neg);
  Tag posTag = term_tag(pos);
  if (neg == VOID || pos == VOID)
    BOOM("linking VOID");
  if (isPositive(neg))
    BOOM("linking from a positive");
  if (isNegative(pos))
    BOOM("linking to a negative");
  if ((neg == VAL && pos == VAL) ||
      (neg == APP && pos == LAM))
    BOOM("bad link");
  else if (neg == VAL || neg == SUB)
    BOOM("bad link");
  else {
    switch (posTag) {
    case LAZ:
      rbag_push(neg, pos);
      break;
      
    case VAR:
      if (1) {
	Term far = get(term_loc(pos));
	Tag t = term_tag(far);
	if (t == VAR) {
	  take(term_loc(pos));
	  link(neg, far);
	} else if (t == LAZ) {
	  if (negTag == DUP) {
	    Term newZ = pair_make(LAZ, neg, pos);
	    move(port(1, term_loc(neg)), newZ);
	    move(port(2, term_loc(neg)), newZ);
	  }
	  else {
	    take(term_loc(pos));
	    link(neg, far);
	  }
	} else if (t != SUB) {
	  fprintf(stderr, "pos %d: %p %s neg: %p %s far: %p %s\n", __LINE__,
		  (void *)pos, tag_to_str(term_tag(pos)),
		  (void *)neg, tag_to_str(term_tag(neg)),
		  (void *)far, tag_to_str(term_tag(far)));
	  link(neg, far);
	} else if (far != SUB) {
	  fprintf(stderr, "pos %d: %p %s neg: %p %s far: %p %s\n", __LINE__,
		  (void *)pos, tag_to_str(term_tag(pos)),
		  (void *)neg, tag_to_str(term_tag(neg)),
		  (void *)far, tag_to_str(term_tag(far)));
	  take(term_loc(pos));
	  Loc sub_loc = term_loc(far);
	  Term app = takeAndCheck(port(1, sub_loc));
	  Term lam = takeAndCheck(port(2, sub_loc));
	  if (term_tag(app) == APP) {
	    // set(port(1, term_loc(app)), neg);
	    link(app, lam);
	  } else {
	    fprintf(stderr, "sub_loc: %d  pos: %s %p\n",
		    sub_loc, tag_to_str(term_tag(pos)), (void *)pos);
	    BOOM("bad link");
	  }
	}
      }
      break;
      
    default:
      rbag_push(neg, pos);
      break;
    }
  }
    // */
}


void forceLazy(Term z) {
  fprintf(stderr, "forcing lazy %d: %p\n", __LINE__, (void *)z);
  Term neg = take(port(1, term_loc(z)));
  Term pos = get(port(2, term_loc(z)));
  if (neg != VOID) {
    if (term_tag(neg) == DUP && term_tag(pos) == LAZ) {
      BOOM("ever happen?");
      Term curr = swap(port(1, term_loc(neg)), SUB);
      if (curr != z)
	set(port(1, term_loc(neg)), z);
      curr = swap(port(2, term_loc(neg)), SUB);
      if (curr != z)
	set(port(2, term_loc(neg)), z);
      set(port(2, term_loc(z)), neg);
      forceLazy(pos);
    } else if (term_tag(neg) == DUP && term_tag(pos) == VAR) {
      Term curr = swap(port(1, term_loc(neg)), SUB);
      if (curr != z)
	set(port(1, term_loc(neg)), z);
      curr = swap(port(2, term_loc(neg)), SUB);
      if (curr != z)
	set(port(2, term_loc(neg)), z);

      Term newPos = get(term_loc(pos));
      while (term_tag(newPos) == VAR) {
	pos = newPos;
	newPos = get(term_loc(pos));
      }
      if (term_tag(newPos) == LAZ) {
	set(term_loc(pos), pair_make(SUB, neg, pos));
	forceLazy(newPos);
      } else
	link(neg, newPos);
      take(port(2, term_loc(z)));
    } else {
      take(port(2, term_loc(z)));
      link(neg, pos);
    }
  }
}

void move(Loc neg_loc, Term pos) {
  Term neg = swap(neg_loc, pos);
  Tag negTag = term_tag(neg);
  if (negTag == SUB) {
    if (neg != SUB) {
      Loc sub_loc = term_loc(neg);
      Term neg = takeAndCheck(port(1, sub_loc));
      Term pos = takeAndCheck(port(2, sub_loc));
      link(neg, pos);
    }
  } else if (negTag == LAZ) {
    forceLazy(neg);
  } else {
    // No need to take() since we already swapped
    link(neg, pos);
  }
}

// Interactions
static void interact_applam(Loc a_loc, Loc b_loc) {
  // a_loc is APP, b_loc is LAM
  if (a_loc == 0 && b_loc == 0)
    return;

  Term arg = get(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Loc  var = port(1, b_loc);
  Term bod = get(port(2, b_loc));
  if(term_tag(arg) == SUB) {
    fprintf(stderr, "applam %d: arg: %p %s  ret: %p %s\n", __LINE__,
	    (void *)arg, tag_to_str(term_tag(arg)),
	    (void *)get(ret), tag_to_str(term_tag(get(ret))));
    return;
  }
  if(term_tag(arg) == SUB) {
    fprintf(stderr, "applam %d: arg: %p %s  ret: %p %s\n", __LINE__,
	    (void *)arg, tag_to_str(term_tag(arg)),
	    (void *)get(ret), tag_to_str(term_tag(get(ret))));
    return;
  }
  arg = take(port(1, a_loc));
  bod = take(port(2, b_loc));
  move(var, arg);
  move(ret, bod);
}

static void interact_appref(Term app, Term ref) {
  interactionFn fnPtr;
  fnPtr = (interactionFn)(ref & ~0xF);
  fnPtr(ref, app);
}

static void interact_appsup(Loc a_loc, Loc b_loc) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = takeAndCheck(port(1, b_loc));
  Term tm2 = takeAndCheck(port(2, b_loc));
  Loc  dp1 = alloc_node(2);
  Loc  dp2 = alloc_node(2);
  Loc  cn1 = alloc_node(2);
  Loc  cn2 = alloc_node(2);
  set(port(1, dp1), term_new(SUB, 0, 0));
  set(port(2, dp1), term_new(SUB, 0, 0));
  set(port(1, dp2), term_new(VAR, 0, port(2, cn1)));
  set(port(2, dp2), term_new(VAR, 0, port(2, cn2)));
  set(port(1, cn1), term_new(VAR, 0, port(1, dp1)));
  set(port(2, cn1), term_new(SUB, 0, 0));
  set(port(1, cn2), term_new(VAR, 0, port(2, dp1)));
  set(port(2, cn2), term_new(SUB, 0, 0));
  link(term_new(DUP, 0, dp1), arg);
  move(ret, term_new(SUP, 0, dp2));
  link(term_new(APP, 0, cn1), tm1);
  link(term_new(APP, 0, cn2), tm2);
}

static void interact_subnul(Loc a_loc) {
  if (a_loc == 0)
    return;

  Term pos = take(port(1, a_loc));
  Tag posTag = term_tag(pos);
  Term neg = take(port(2, a_loc));
  Tag negTag = term_tag(neg);
  if (pos == SUB)
    BOOM("took SUB");
  if (posTag == APP) 
    link(pos, NUL);
  else if (posTag == SUB) {
    Loc posLoc = term_loc(get(port(1, term_loc(pos))));
    if (posLoc == a_loc) {
      set(port(1, term_loc(pos)), VOID);
      Term posNeg = get(port(2, term_loc(pos)));
      if (isPositive(posNeg))
	link(ERA, posNeg);
      else
	link(posNeg, NUL);
    } else {
      // SUB is a negative
      link(pos, NUL);
    }
  } else
    link(ERA, pos);
  if (negTag == REF)
    link(ERA, neg);
  else
    link(neg, NUL);
}

static void interact_appnul(Loc a_loc) {
  if (a_loc == 0)
    return;

  Term pos = take(port(1, a_loc));
  Tag posTag = term_tag(pos);
  Term neg = take(port(2, a_loc));
  Tag negTag = term_tag(neg);
  if (pos == SUB)
    BOOM("took SUB");
  else if (posTag == APP) 
    BOOM("what's an APP doing here");
  else if (posTag == SUB) {
    link(pos, NUL);
  } else
    link(ERA, pos);
  link(neg, NUL);
}

/*
static void interact_appu32(Loc a_loc, u32 num) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(term_new(U32, 0, num), arg);
  move(ret, term_new(U32, 0, num));
}

static void interact_opxnul(Loc a_loc) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(ERA, arg);
  move(ret, term_new(NUL, 0, 0));
}

static void interact_opxnum(Loc a_loc, Lab op, u32 num, Tag num_type) {
  Term arg = swap(port(1, a_loc), term_new(num_type, 0, num));
  link(term_new(OPY, op, a_loc), arg);
}

static void interact_opxsup(Loc a_loc, Lab op, Loc b_loc) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = takeAndCheck(port(1, b_loc));
  Term tm2 = takeAndCheck(port(2, b_loc));
  Loc  dp1 = alloc_node(2);
  Loc  dp2 = alloc_node(2);
  Loc  cn1 = alloc_node(2);
  Loc  cn2 = alloc_node(2);
  set(port(1, dp1), term_new(SUB, 0, 0));
  set(port(2, dp1), term_new(SUB, 0, 0));
  set(port(1, dp2), term_new(VAR, 0, port(2, cn1)));
  set(port(2, dp2), term_new(VAR, 0, port(2, cn2)));
  set(port(1, cn1), term_new(VAR, 0, port(1, dp1)));
  set(port(2, cn1), term_new(SUB, 0, 0));
  set(port(1, cn2), term_new(VAR, 0, port(2, dp1)));
  set(port(2, cn2), term_new(SUB, 0, 0));
  link(term_new(DUP, 0, dp1), arg);
  move(ret, term_new(SUP, 0, dp2));
  link(term_new(OPX, op, cn1), tm1);
  link(term_new(OPX, op, cn2), tm2);
}

static void interact_opynul(Loc a_loc) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(ERA, arg);
  move(ret, term_new(NUL, 0, 0));
}

// Utilities
u32 u32_to_u32(u32 u) { return         u; }
i32 u32_to_i32(u32 u) { return *(i32*)&u; }
f32 u32_to_f32(u32 u) { return *(f32*)&u; }
u32 i32_to_u32(i32 i) { return *(u32*)&i; }
u32 f32_to_u32(f32 f) { return *(u32*)&f; }

static void interact_opynum(Loc a_loc, Lab op, u32 y, Tag y_type) {
  u32 x = term_loc(takeAndCheck(port(1, a_loc)));
  Loc ret = port(2, a_loc);
  u32 res;

  // Optimized fast path for common case U32)
  if (y_type == U32) {
    switch (op) {
      case OP_ADD: res = x + y; break;
      case OP_SUB: res = x - y; break;
      case OP_MUL: res = x * y; break;
      case OP_DIV: res = x / y; break;
      case OP_EQ : res = x == y; break;
      case OP_NE : res = x != y; break;
      case OP_LT : res = x < y; break;
      case OP_GT : res = x > y; break;
      case OP_LTE: res = x <= y; break;
      case OP_GTE: res = x >= y; break;
      case OP_MOD: res = x % y; break;
      case OP_AND: res = x & y; break;
      case OP_OR : res = x | y; break;
      case OP_XOR: res = x ^ y; break;
      case OP_LSH: res = x << y; break;
      case OP_RSH: res = x >> y; break;
      default: res = 0;
    }
  } else {
    // if not, defer
    #define CASES_u32(a, b)                     \
              case OP_MOD: val = a %  b; break; \
              case OP_AND: val = a &  b; break; \
              case OP_OR : val = a |  b; break; \
              case OP_XOR: val = a ^  b; break; \
              case OP_LSH: val = a << b; break; \
              case OP_RSH: val = a >> b; break;
    #define CASES_i32(a, b) CASES_u32(a, b)
    #define CASES_f32(a, b)

    #define PERFORM_OP(x, y, op, type)          \
      {                                         \
          type val;                             \
          type a = u32_to_##type(x);            \
          type b = u32_to_##type(y);            \
          switch (op) {                         \
              case OP_ADD: val = a +  b; break; \
              case OP_SUB: val = a -  b; break; \
              case OP_MUL: val = a *  b; break; \
              case OP_DIV: val = a /  b; break; \
              case OP_EQ : val = a == b; break; \
              case OP_NE : val = a != b; break; \
              case OP_LT : val = a <  b; break; \
              case OP_GT : val = a >  b; break; \
              case OP_LTE: val = a <= b; break; \
              case OP_GTE: val = a >= b; break; \
              CASES_##type(a, b)                \
          }                                     \
          res = type##_to_u32(val);             \
      }

    switch (y_type) {
      case I32: PERFORM_OP(x, y, op, i32); break;
      case F32: PERFORM_OP(x, y, op, f32); break;
    }
  }

  move(ret, term_new(y_type, 0, res));
}

static void interact_opysup(Loc a_loc, Loc b_loc) {
  Term arg = takeAndCheck(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = takeAndCheck(port(1, b_loc));
  Term tm2 = takeAndCheck(port(2, b_loc));
  Loc  dp1 = alloc_node(2);
  Loc  dp2 = alloc_node(2);
  Loc  cn1 = alloc_node(2);
  Loc  cn2 = alloc_node(2);
  set(port(1, dp1), term_new(SUB, 0, 0));
  set(port(2, dp1), term_new(SUB, 0, 0));
  set(port(1, dp2), term_new(VAR, 0, port(2, cn1)));
  set(port(2, dp2), term_new(VAR, 0, port(2, cn2)));
  set(port(1, cn1), term_new(VAR, 0, port(1, dp1)));
  set(port(2, cn1), term_new(SUB, 0, 0));
  set(port(1, cn2), term_new(VAR, 0, port(2, dp1)));
  set(port(2, cn2), term_new(SUB, 0, 0));
  link(term_new(DUP, 0, dp1), arg);
  move(ret, term_new(SUP, 0, dp2));
  link(term_new(OPY, 0, cn1), tm1);
  link(term_new(OPY, 0, cn2), tm2);
}
// */

static void interact_dupsup(Loc a_loc, Loc b_loc) {
  Loc  dp1 = port(1, a_loc);
  Loc  dp2 = port(2, a_loc);
  Term tm1 = takeAndCheck(port(1, b_loc));
  Term tm2 = takeAndCheck(port(2, b_loc));
  move(dp1, tm1);
  move(dp2, tm2);
}

static void interact_duplam(Loc a_loc, Loc b_loc) {
  Loc  dp1 = port(1, a_loc);
  Loc  dp2 = port(2, a_loc);
  Loc  var = port(1, b_loc);
  // TODO(enricozb): why is this the only take?
  Term bod = takeAndCheck(port(2, b_loc));
  Loc  co1 = alloc_node(2);
  Loc  co2 = alloc_node(2);
  Loc  du1 = alloc_node(2);
  Loc  du2 = alloc_node(2);
  set(port(1, co1), term_new(SUB, 0, 0));
  set(port(2, co1), term_new(VAR, 0, port(1, du2)));
  set(port(1, co2), term_new(SUB, 0, 0));
  set(port(2, co2), term_new(VAR, 0, port(2, du2)));
  set(port(1, du1), term_new(VAR, 0, port(1, co1)));
  set(port(2, du1), term_new(VAR, 0, port(1, co2)));
  set(port(1, du2), term_new(SUB, 0, 0));
  set(port(2, du2), term_new(SUB, 0, 0));
  move(dp1, term_new(LAM, 0, co1));
  move(dp2, term_new(LAM, 0, co2));
  move(var, term_new(SUP, 0, du1));
  link(term_new(DUP, 0, du2), bod);
}

static void interact_dupnul(Loc a_loc) {
  Loc dp1 = port(1, a_loc);
  Loc dp2 = port(2, a_loc);
  move(dp1, term_new(NUL, 0, a_loc));
  move(dp2, term_new(NUL, 0, a_loc));
}

static void interact_dupval(Loc a_loc, Term pos) {
  Loc dp1 = port(1, a_loc);
  Loc dp2 = port(2, a_loc);
  incRef(pos, 1);
  move(dp1, pos);
  move(dp2, pos);
}

static void interact_dupnum(Loc a_loc, Term pos) {
  Loc dp1 = port(1, a_loc);
  Loc dp2 = port(2, a_loc);
  move(dp1, pos);
  move(dp2, pos);
}

static void interact_dupref(Loc a_loc, Loc b_loc) {
  move(port(1, a_loc), term_new(REF, 0, b_loc));
  move(port(2, a_loc), term_new(REF, 0, b_loc));
}

/*
static void interact_matnul(Loc a_loc, Lab mat_len) {
  move(port(1, a_loc), term_new(NUL, 0, 0));
  for (u32 i = 0; i < mat_len; i++) {
    link(ERA, takeAndCheck(port(i + 2, a_loc)));
  }
}

static void interact_matnum(Loc mat_loc, Lab mat_len, u32 n, Tag n_type) {
  if (n_type != U32) {
    printf("match with non-U32\n");
    exit(1);
  }

  u32 i_arm = (n < mat_len - 1) ? n : (mat_len - 1);
  for (u32 i = 0; i < mat_len; i++) {
    if (i != i_arm) {
      link(ERA, takeAndCheck(port(2 + i, mat_loc)));
    }
  }

  Loc ret = port(1, mat_loc);
  Term arm = takeAndCheck(port(2 + i_arm, mat_loc));

  if (i_arm < mat_len - 1) {
    move(ret, arm);
  } else {
    Loc app = alloc_node(2);
    set(app + 0, term_new(U32, 0, n - (mat_len - 1)));
    set(app + 1, term_new(SUB, 0, 0));
    move(ret, term_new(VAR, 0, port(2, app)));

    link(term_new(APP, 0, app), arm);
  }
}

static void interact_matsup(Loc mat_loc, Lab mat_len, Loc sup_loc) {
  Loc ma0 = alloc_node(1 + mat_len);
  Loc ma1 = alloc_node(1 + mat_len);

  Loc sup = alloc_node(2);

  set(port(1, sup), term_new(VAR, 0, port(1, ma1)));
  set(port(2, sup), term_new(VAR, 0, port(1, ma0)));
  set(port(1, ma0), term_new(SUB, 0, 0));
  set(port(1, ma1), term_new(SUB, 0, 0));

  for (u64 i = 0; i < mat_len; i++) {
    Loc dui = alloc_node(2);
    set(port(1, dui),     term_new(SUB, 0, 0));
    set(port(2, dui),     term_new(SUB, 0, 0));
    set(port(2 + i, ma0), term_new(VAR, 0, port(2, dui)));
    set(port(2 + i, ma1), term_new(VAR, 0, port(1, dui)));

    link(term_new(DUP, 0, dui), takeAndCheck(port(2 + i, mat_loc)));
  }

  move(port(1, mat_loc), term_new(SUP, 0, sup));
  link(term_new(MAT, mat_len, ma0), takeAndCheck(port(2, sup_loc)));
  link(term_new(MAT, mat_len, ma1), takeAndCheck(port(1, sup_loc)));
}
// */


static void interact_eralam(Loc b_loc) {
  Loc  var = port(1, b_loc);
  Term bod = takeAndCheck(port(2, b_loc));
  move(var, term_new(NUL, 0, 0));
  link(ERA, bod);
}

static void interact_erasup(Loc b_loc) {
  Term tm1 = takeAndCheck(port(1, b_loc));
  Term tm2 = takeAndCheck(port(2, b_loc));
  link(ERA, tm1);
  link(ERA, tm2);
}

static void interact(Term neg, Term pos) {
  Tag neg_tag = term_tag(neg);
  Tag pos_tag = term_tag(pos);
  Loc neg_loc = term_loc(neg);
  Loc pos_loc = term_loc(pos);

  switch (neg_tag) {
  case VAR:
    move(neg_loc, pos);
    break;

  case SUB:
    if (pos == NUL) {
      set(port(1, neg_loc), VOID);
      Term subNeg = take(port(2, neg_loc));
      if (term_tag(subNeg != REF))
	link(ERA, subNeg);
    } else {
      BOOM("bad link previously");
    }
    break;

  case APP:
    switch (pos_tag) {
    case LAM: interact_applam(neg_loc, pos_loc); break;
    case NUL: interact_appnul(neg_loc); break;
      // case U32: interact_appu32(neg_loc, pos_loc); break;
    case REF: interact_appref(neg, pos); break;
    case SUP: interact_appsup(neg_loc, pos_loc); break;
    }
    break;

    /*
  case VAL:
    switch (pos_tag) {
    case SUB:
      if (1) {
	Term app = takeAndCheck(port(1, pos_loc));
	Term lam = takeAndCheck(port(2, pos_loc));
	if (pos != SUB && term_tag(app) == APP) {
	  set(port(1, term_loc(app)), neg);
	  link(app, lam);
	} else {
	  fprintf(stderr, "neg: %p, pos: %p, app %s %p\n",
		  (void *)neg, (void *)pos, tag_to_str(term_tag(app)), (void *)app);
	  BOOM("bad link");
	}
      }
      break;
      
    default:
      fprintf(stderr, "neg: %p, pos: %p\n", (void *)neg, (void *)pos);
      BOOM("bad link");
      break;
    }
    break;
    
      case OPX:
      switch (pos_tag) {
      case LAM: break;
      case NUL: interact_opxnul(neg_loc); break;
      case U32:
      case I56:
      case F56: interact_opxnum(neg_loc, term_lab(neg), pos_loc, pos_tag); break;
      case REF: link(neg, expand_ref(pos_loc)); break;
      case SUP: interact_opxsup(neg_loc, term_lab(neg), pos_loc); break;
      }
      break;
      case OPY:
      switch (pos_tag) {
      case LAM: break;
      case NUL: interact_opynul(neg_loc); break;
      case U32:
      case I56:
      case F56: interact_opynum(neg_loc, term_lab(neg), pos_loc, pos_tag); break;
      case REF: link(neg, expand_ref(pos_loc)); break;
      case SUP: interact_opysup(neg_loc, pos_loc); break;
      }
      break;
      case MAT:
      switch (pos_tag) {
      case LAM: break;
      case NUL: interact_matnul(neg_loc, term_lab(neg)); break;
      // case U32:
      case I56:
      case F56: interact_matnum(neg_loc, term_lab(neg), pos_loc, pos_tag); break;
      case REF: link(neg, expand_ref(pos_loc)); break;
      case SUP: interact_matsup(neg_loc, term_lab(neg), pos_loc); break;
      }
      break;
      // */

  case DUP:
    switch (pos_tag) {
    case VAL: interact_dupval(neg_loc, pos); break;
    case LAM: interact_duplam(neg_loc, pos_loc); break;
    case NUL: interact_dupnul(neg_loc); break;
      // case U32:
    case I56:
    case F56: interact_dupnum(neg_loc, pos); break;
      // TODO(enricozb): dup-ref optimization
    case REF: interact_dupref(neg_loc, pos_loc); break;
      // case REF: link(neg, expand_ref(pos_loc)); break;
    case SUP: interact_dupsup(neg_loc, pos_loc); break;
    }
    break;

  case ERA:
    switch (pos_tag) {
    case VAL: dec_and_free(pos, 1); break;
    case LAM: interact_eralam(pos_loc); break;
    case NUL: break;
      // case U32: break;
    case I56: break;
    case F56: break;
    case REF: break;
    case SUP: interact_erasup(pos_loc); break;
    }
    break;

  case I56:
    switch(pos_tag) {
    case ERA:
    case NUL:
    case I56:
    case F56:
      break;
      
    default:
      fprintf(stderr, "bad tag: %s (%d) %p\n", tag_to_str(neg_tag), neg_tag, (void *)neg);
      BOOM("unhandled tag");
    }
    break;

  default:
    fprintf(stderr, "bad tag: %s (%d) %p\n", tag_to_str(neg_tag), neg_tag, (void *)neg);
    BOOM("unhandled tag");
  }
}

// Evaluation
static inline int normal_step() {
  // dump_buff();

  Loc loc = rbag_pop();
  if (loc == 0) {
    // dump_buff();

    return 0;
  }

  Term neg = take(loc + 0);
  if (neg == SUB)
    BOOM("took SUB");
  Term pos = takeAndCheck(loc + 1);

  printf("\n\n%04lX: INTERACT %s ~ %s\n%p ~ %p\n",
	 inc_itr(), tag_to_str(term_tag(neg)), tag_to_str(term_tag(pos)),
	 (void *)neg, (void *)pos);

  interact(neg, pos);

  return 1;
}

// FFI exports
void hvm_init() {
  if (BUFF == NULL) {
    BUFF = calloc((1ULL << 24), sizeof(a64));
  }
  RNOD_INI = 0;
  RNOD_END = 0;
  RBAG_INI = 0;
  RBAG_END = 0;
}

void hvm_free() {
  if (BUFF != NULL) {
    free(BUFF);
    BUFF = NULL;
  }
}

void normalize() {
  while (normal_step());
}

// Debugging
char* tag_to_str(Tag tag) {
  switch (tag) {
  case VAL:  return "VAL";
  case VL1:  return "VAL";
  case VAR:  return "VAR";
  case SUB:  return "SUB";
  case NUL:  return "NUL";
  case ERA:  return "ERA";
  case LAM:  return "LAM";
  case APP:  return "APP";
  case SUP:  return "SUP";
  case DUP:  return "DUP";
  case REF:  return "REF";
  case OPX:  return "OPX";
  case OPY:  return "OPY";
  case I56:  return "I56";
  case F56:  return "F56";
  case LAZ:  return "LAZ";

  default:   return "???";
  }
}

void dump_buff() {
  printf("------------------\n");
  printf("      NODES\n");
  printf("ADDR   LOC LAB TAG\n");
  printf("------------------\n");
  for (Loc loc = RNOD_INI; loc < RNOD_END; loc++) {
    Term term = get(loc);
    Loc t_loc = term_loc(term);
    Lab t_lab = term_lab(term);
    Tag t_tag = term_tag(term);
    printf("%06X %03X %03X %s\n", loc, term_loc(term), term_lab(term), tag_to_str(term_tag(term)));
  }
  printf("------------------\n");
  printf("    REDEX BAG\n");
  printf("ADDR   LOC LAB TAG\n");
  printf("------------------\n");
  for (Loc loc = RBAG + RBAG_INI; loc < RBAG + RBAG_END; loc++) {
    Term term = get(loc);
    Loc t_loc = term_loc(term);
    Lab t_lab = term_lab(term);
    Tag t_tag = term_tag(term);
    printf("%06X %03X %03X %s\n", loc, term_loc(term), term_lab(term), tag_to_str(term_tag(term)));
  }
  printf("------------------\n");
}

void* thread_function(void* arg) {
    printf("Thread %ld started\n", (long)arg);
    return NULL;
}

void spawn_threads_equal_to_cores() {
    long num_cores = 1; // sysconf(_SC_NPROCESSORS_ONLN);, tmp
    if (num_cores < 1) {
        perror("sysconf");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[num_cores];

    for (long i = 0; i < num_cores; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void*)i);
    }
}

Term argsNet(NativeArgs *args) {
  //*
  Term tail;
  if(args->count < 1)
    BOOM("argsNet");
  else
    tail = args->args[args->count - 1];

  for (int i = args->count - 2; i >= 0; i--) {
    tail = pair_make(APP, args->args[i], tail);
    args->args[i] = tail;
  }
  
  return args->args[0];
  // */
}

// extract the requested number of native args
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct) {
  // if (argsStruct->count > 0) {
  // fprintf(stderr, "nativeArg %d: %p %d %p\n", __LINE__, (void *)ref, argsStruct->count,
  // (void *)get_i24(get_val(argsStruct->args[argsStruct->count - 1])));
  // } else {
  // fprintf(stderr, "nativeArg %d: %p %d\n", __LINE__, (void *)ref, argsStruct->count);
  // }
  Tag argsTag = term_tag(args);
  // fprintf(stderr, "argsTag %d: %s (%d) in %p\n", __LINE__,
  //	  tag_to_str(argsTag), argsTag, (void *)args);
  Term arg;
  Term varVal;
  Term retry;
  Term newArgs;
  Term newArg;
  switch(argsTag) {
  case APP:
    // TODO: walk through all the args and queue up any LAZ nodes
    arg = takeAndCheck(port(1, term_loc(args)));
    if (expected == 0) {
      return args;
    }

    // only look at port 2 of args. It will be taken in the caller if needed.
    Tag argTag = term_tag(arg);
    // fprintf(stderr, "arg 2 %d: %d %p\n", __LINE__, argTag, (void *)arg);
    switch(argTag) {
      // the strict arg types
    case VAL:
    case I56:
    case F56:
    case REF:
      argsStruct->args[argsStruct->count++] = arg;
      if (expected > 1)
	return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
      else
	return args;
      break;

    case VAR:
      if (1) {
	Term negVar = get(term_loc(arg));
	switch(term_tag(negVar)) {
	  // the strict arg types
	case VAL:
	case I56:
	case F56:
	case REF:
	  if(1) {
	    Term val = take(term_loc(arg));
	    argsStruct->args[argsStruct->count++] = val;
	    if (expected > 1)
	      return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
	    else
	      return args;
	  }
	  break;

	case SUB:
	  if (negVar != SUB)
	    BOOM("nativeArgs");
	  else {
	    argsStruct->args[argsStruct->count++] = args;
	    newArgs = argsNet(argsStruct);
	    set(port(1, term_loc(args)), arg);
	    retry = pair_make(SUB, newArgs, ref);
	    newArg = swap(term_loc(arg), retry);
	    if (newArg != SUB) {
	      // someone slipped the needed arg in since we last looked
	      set(term_loc(arg), newArg);
	      take(port(1, term_loc(retry)));
	      take(port(2, term_loc(retry)));
	      link(newArgs, ref);
	    }
	    argsStruct->count = -1;
	    return VOID;
	  }
	  break;

	case LAZ:
	  fprintf(stderr, "args %d: %p  arg: %p\n", __LINE__,
		  (void *)args, (void *)arg);
	  argsStruct->args[argsStruct->count++] = args;
	  newArgs = argsNet(argsStruct);
	  set(port(1, term_loc(args)), arg);
	  retry = pair_make(SUB, newArgs, ref);
	  forceLazy(negVar);
	  newArg = swap(term_loc(arg), retry);
	  if (newArg != negVar) {
	    // someone slipped the needed arg in since we last looked
	    set(port(1, term_loc(arg)), newArg);
	    take(port(1, term_loc(retry)));
	    take(port(2, term_loc(retry)));
	    link(newArgs, ref);
	  }
	  argsStruct->count = -1;
	  return VOID;
	  break;

	default:
	  fprintf(stderr, "arg %s %p\n", tag_to_str(term_tag(arg)), (void *)arg);
	  fprintf(stderr, "negVar %s %p\n", tag_to_str(term_tag(negVar)), (void *)negVar);
	  BOOM("natveArgs");
	  break;
	}
	/*
	  argsStruct->args[argsStruct->count++] = arg;
	  argsStruct->args[argsStruct->count++] = argsNode.snd;
	  varVal = vars_exchange(arg, node_make(RDX, ref, argsNet(argsStruct)));
	  if (varVal != NONE && varVal != FREE) {
	  // fprintf(stderr, "varVal %d: %p  %p\n", __LINE__, (void *)arg, (void *)varVal);
	  if (get_tag(varVal) == RDX) {
	  push_redex(node_take(varVal));
	  }
	  }
	  argsStruct->count = -1;
	  // */
	argsStruct->count = -1;
	return VOID;
      }
      break;

    case SUB:
      BOOM("natveArgs");
      /*
      if (arg == 0 && argsNode == 0)
	return VOID;
      else if (arg == 0 || argsNode == 0)
	BOOM("natveArgs");
      Term neg = takeAndCheck(port(1, term_loc(arg)));
      Term pos = takeAndCheck(port(2, term_loc(arg)));
      int argsCount = argsStruct->count;
      argsStruct->args[argsStruct->count++] = SUB;
      argsStruct->args[argsStruct->count++] = argsNode;
      newArgs = argsNet(argsStruct);
      retry = pair_make(SUB, newArgs, ref);
      Loc subLoc = term_loc(argsStruct->args[argsCount]);
      set(port(1, subLoc), retry);
      set(port(1, term_loc(neg)), term_new(VAR, 0, subLoc));
      link(neg, pos);
      argsStruct->count = -1;
	// */
      return VOID;
      break;

/*
    case DUP:
      // fprintf(stderr, "Boomity %s %d\n", __FILE__, __LINE__);
      // abort();
      // break;
      
	case CON:
	if (1) {
	BOOM("natveArgs");
	// fprintf(stderr, "DUP/CON %d: %d\n", __LINE__, argTag);
	Term r1 = vars_make(NONE);
	Term r2 = vars_make(NONE);
	Term finalResult = argsStruct->result;
	link(finalResult, node_make(argTag, r1, r2));
	Term args1;
	Term args2;

	if (argsNode.snd == ARG || argsNode.snd == endArgs) {
	args1 = argsNode.snd;
	args2 = argsNode.snd;
	} else {
	args1 = vars_make(NONE);
	args2 = vars_make(NONE);
	Term n = node_make(DUP, args1, args2);
	link(n, argsNode.snd);
	}

	int argsCount = argsStruct->count;
	for (int i = 0; i < argsCount; i++) {
	incRef(argsStruct->args[i], 1);
	}

	// TODO: if pr.fst is not a native val, put pr.snd with args1
	Pair pr = node_take(arg);
	argsStruct->result = r1;
	argsStruct->count = argsCount + 2;
	argsStruct->args[argsCount] = pr.fst;
	argsStruct->args[argsCount + 1] = args1;
	Term net1 = argsNet(argsStruct);
	link(ref, net1);
	
	argsStruct->result = r2;
	argsStruct->args[argsCount] = pr.snd;
	argsStruct->args[argsCount + 1] = args2;
	Term net2 = argsNet(argsStruct);
	link(ref, net2);

	// link(ref, node_make(argTag, net1, net2));
	argsStruct->count = -1;
	return VOID;
	}
	break;
// */

    case ERA:
      BOOM("natveArgs");
      /*
      // fprintf(stderr, "ERA %d: %d\n", __LINE__, argsStruct->count);
      link(argsStruct->result, erase);
      for (int i = 0; i < argsStruct->count; i++) {
      link(argsStruct->args[i], erase);
      }
      link(argsNode.snd, erase);
      // */
      break;

      // TODO: what other tags need to be handled
    default:
      printf("unhandled tag %s (0x%x) line: %d\n", tag_to_str(term_tag(arg)),
	     term_tag(arg), __LINE__);
      abort();
      break;
    }
    argsStruct->count = -1;
    // */
    return VOID;
    break;

  case VAR:
    BOOM("natveArgs");
    /*
      args = enter(args);
      // fprintf(stderr, "args VAR %d: %p %d\n", __LINE__, (void *)args, get_tag(args));
      argsStruct->args[argsStruct->count++] = args;
      if (get_tag(args) == VAR) {
      varVal = vars_exchange(args, node_make(RDX, ref, argsNet(argsStruct)));
      // fprintf(stderr, "varVal %d: %p  %p\n", __LINE__, (void *)arg, (void *)varVal);
      // if (varVal != endArgs && varVal != NONE && varVal != FREE) {
      // fprintf(stderr, "wut\n");
      // // link(ref, varVal);
      // vars_exchange(args, FREE);
      // }
      } else {
      link(ref, argsNet(argsStruct));
      }
      argsStruct->count = -1;
      // */
    return VOID;
    break;

  case SUB:
    BOOM("natveArgs");
    return VOID;
    break;

    // TODO: what other tags need to be handled
  default:
    printf("unhandled tag %s (0x%x) %p line: %d\n",
	   tag_to_str(argsTag), argsTag, (void *)arg, __LINE__);
    abort();
    return VOID;
    break;
  }
  // */
}

Term dupeArg(Term arg, Term dupedArg) {
  // fprintf(stderr, "arg: %d %p\n", __LINE__, (void *)arg);
  switch(term_tag(arg)) {
  case VAL:
    link(incRef(arg, 1), dupedArg);
    return arg;
    break;

  case F56:
  case I56:
  case REF:
    link(arg, dupedArg);
    return arg;
    break;

  default:
    if (1) {
      Term newDup = pair_make(DUP, SUB, dupedArg);
      link(arg, newDup);
      return port(1, term_loc(newDup));
    }
    break;
  }
}


/*
int main(int argc, char *argv[]) {
  printf("50: %ld\n", get_i56_(new_i56_(50)));
  printf("-50: %ld\n", get_i56_(new_i56_(-50)));
  BOOM("howdy");
}
// */
