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
static char* tag_to_str(Tag tag);
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

Term term_offset_loc(Term term, Loc offset) {
  // do not offset terms that use loc for something other than
  // indices into the global buffer.
  switch (term_tag(term)) {
    case SUB:
      BOOM("offset of SUB");
      break;
    case NUL:
    case ERA:
    case REF:
    case I56:
    case F56:
      return term;
  }

  Term tag = term_tag(term);
  Term lab = term_lab(term);
  Term loc = term_loc(term) + offset;

  return term_new(tag, lab, loc);
}

// Memory operations
Term swap(Loc loc, Term term) {
  return atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
}

Term get(Loc loc) {
  return atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
}

Term take(Loc loc) {
  return atomic_exchange_explicit(&BUFF[loc], VOID, memory_order_relaxed);
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

Term pair_make(Tag tag, Term fst, Term snd) {
  // TM *tm = tms[tid];
  Loc loc = alloc_node(2);
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

  RNOD_END = 2;
  RBAG_END = 0;

  BOOK.defs[BOOK.len] = def;
  BOOK.len++;
}

char* def_name(Loc def_idx) {
  return BOOK.defs[def_idx].name;
}

// Atomic Linker
static inline void move(Loc neg_loc, u64 pos);

void link(Term neg, Term pos) {
  if (term_tag(pos) == VAR) {
    Term far = swap(term_loc(pos), neg);
    Tag t = term_tag(far);
    if (t == RDX) {
      BOOM("linking RDX");
    } else if (t != SUB) {
      // leave things as they are
      move(term_loc(pos), far);
    }
  } else {
    rbag_push(neg, pos);
  }
}


static inline void move(Loc neg_loc, Term pos) {
  Term neg = swap(neg_loc, pos);
  if (term_tag(neg) != SUB) {
    // No need to take() since we already swapped
    link(neg, pos);
  }
}

// Interactions
static void interact_applam(Loc a_loc, Loc b_loc) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Loc  var = port(1, b_loc);
  Term bod = take(port(2, b_loc));
  move(var, arg);
  move(ret, bod);
}

static void interact_appref(Term app, Term ref) {
  interactionFn fnPtr;
  fnPtr = (interactionFn)(ref & ~0xF);
  fnPtr(ref, app);
}

static void interact_appsup(Loc a_loc, Loc b_loc) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = take(port(1, b_loc));
  Term tm2 = take(port(2, b_loc));
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

static void interact_appnul(Loc a_loc) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(ERA, arg);
  move(ret, term_new(NUL, 0, 0));
}

/*
static void interact_appu32(Loc a_loc, u32 num) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(term_new(U32, 0, num), arg);
  move(ret, term_new(U32, 0, num));
}

static void interact_opxnul(Loc a_loc) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  link(ERA, arg);
  move(ret, term_new(NUL, 0, 0));
}

static void interact_opxnum(Loc a_loc, Lab op, u32 num, Tag num_type) {
  Term arg = swap(port(1, a_loc), term_new(num_type, 0, num));
  link(term_new(OPY, op, a_loc), arg);
}

static void interact_opxsup(Loc a_loc, Lab op, Loc b_loc) {
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = take(port(1, b_loc));
  Term tm2 = take(port(2, b_loc));
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
  Term arg = take(port(1, a_loc));
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
  u32 x = term_loc(take(port(1, a_loc)));
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
  Term arg = take(port(1, a_loc));
  Loc  ret = port(2, a_loc);
  Term tm1 = take(port(1, b_loc));
  Term tm2 = take(port(2, b_loc));
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
  Term tm1 = take(port(1, b_loc));
  Term tm2 = take(port(2, b_loc));
  move(dp1, tm1);
  move(dp2, tm2);
}

static void interact_duplam(Loc a_loc, Loc b_loc) {
  Loc  dp1 = port(1, a_loc);
  Loc  dp2 = port(2, a_loc);
  Loc  var = port(1, b_loc);
  // TODO(enricozb): why is this the only take?
  Term bod = take(port(2, b_loc));
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
    link(ERA, take(port(i + 2, a_loc)));
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
      link(ERA, take(port(2 + i, mat_loc)));
    }
  }

  Loc ret = port(1, mat_loc);
  Term arm = take(port(2 + i_arm, mat_loc));

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

    link(term_new(DUP, 0, dui), take(port(2 + i, mat_loc)));
  }

  move(port(1, mat_loc), term_new(SUP, 0, sup));
  link(term_new(MAT, mat_len, ma0), take(port(2, sup_loc)));
  link(term_new(MAT, mat_len, ma1), take(port(1, sup_loc)));
}
// */


static void interact_eralam(Loc b_loc) {
  Loc  var = port(1, b_loc);
  Term bod = take(port(2, b_loc));
  move(var, term_new(NUL, 0, 0));
  link(ERA, bod);
}

static void interact_erasup(Loc b_loc) {
  Term tm1 = take(port(1, b_loc));
  Term tm2 = take(port(2, b_loc));
  link(ERA, tm1);
  link(ERA, tm2);
}

static void interact(Term neg, Term pos) {
  Tag neg_tag = term_tag(neg);
  Tag pos_tag = term_tag(pos);
  Loc neg_loc = term_loc(neg);
  Loc pos_loc = term_loc(pos);

  switch (neg_tag) {
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
    case VAL: incRef(pos, 1); break;
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
  Term pos = take(loc + 1);

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
  RNOD_END = 2;
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
static char* tag_to_str(Tag tag) {
  switch (tag) {
    case VAL:  return "VAL";
    case VAR:  return "VAR";
    case SUB:  return "SUB";
    case NUL:  return "NUL";
    case ERA:  return "ERA";
    case LAM:  return "LAM";
    case APP:  return "APP";
    case SUP:  return "SUP";
    case DUP:  return "DUP";
    case REF:  return "REF";
      // case OPX:  return "OPX";
      // case OPY:  return "OPY";
      // case U32:  return "U32";
    case I56:  return "I56";
    case F56:  return "F56";
      // case MAT:  return "MAT";

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
  BOOM("argsNet");
  /*
  Term n0 = alloc_node(2);
  Term tail = args->args[args->count - 1];
  for (int i = args->count - 2; i >= 0; i--) {
    node_create(n0, new_pair(args->args[i], tail));
    tail = new_port(ARG, n0);
    n0 = alloc_node(2);
  }
  node_create(n0, new_pair(args->result, tail));
  return new_port(ARG, n0);
  // */
  return (Term)0;
}

// extract the requested number of native args
Term nativeArg(Term ref, Term args, NativeArgs *argsStruct) {
  if (argsStruct->count < 0 || args == VOID) {
    BOOM("natveArgs");
    return VOID;
  }

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
  switch(argsTag) {
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

  case APP:
    arg = take(port(1, term_loc(args)));
    Tag argTag = term_tag(arg);
    Term argsNode = take(port(2, term_loc(args)));
    // fprintf(stderr, "arg 2 %d: %d %p\n", __LINE__, argTag, (void *)arg);
    switch(argTag) {
    case VAL:
    case I56:
    case F56:
      // fprintf(stderr, "arg %d: %p\n", __LINE__, (void *)arg);
      argsStruct->args[argsStruct->count++] = arg;
      return argsNode;
      break;

    case VAR:
      BOOM("natveArgs");
      /*
      // fprintf(stderr, "VAR %d\n", __LINE__);
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
      return VOID;
      break;

    case DUP:
      // fprintf(stderr, "Boomity %s %d\n", __FILE__, __LINE__);
      // abort();
      // break;
      
      /*
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
      printf("unhandled tag 0x%x line: %d\n", term_tag(arg), __LINE__);
      abort();
      break;
    }
    argsStruct->count = -1;
    // */
    return VOID;
    break;

    // TODO: what other tags need to be handled
  default:
    printf("unhandled tag %s (0x%x) %p line: %d\n",
	   tag_to_str(term_tag(arg)), term_tag(arg), (void *)arg, __LINE__);
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
