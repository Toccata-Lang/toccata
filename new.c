#include "new.h"
#include <string.h>

// Global heap
static a64* BUFF = NULL;
static u64 RNOD_INI = 0;
u64 RNOD_END = 0;
static u64 RBAG = 0x1000;
u64 RBAG_INI = 0;
u64 RBAG_END = 0;

// Mutex for thread-safe redex operations
pthread_mutex_t redex_mutex;

// Condition variable for signaling when redex is available
pthread_cond_t redex_cond;

// For testing only
a64* get_buff(void) {
    return BUFF;
}

void *boom(char *msg, char *file, int line) {
  fprintf(stderr, "%s at %s:%d\n", msg, file, line);
  abort();
}

// Initialize the virtual machine with a given heap size
void hvm_init(u64 size) {
    BUFF = (a64*)calloc(size, sizeof(a64));
    if (!BUFF) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    RNOD_INI = 0;
    RNOD_END = 0;
    RBAG_INI = RBAG;
    RBAG_END = RBAG;
    
    // Initialize mutex for thread-safe redex operations
    if (pthread_mutex_init(&redex_mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        free(BUFF);
        BUFF = NULL;
        exit(1);
    }
    
    // Initialize condition variable for redex signaling
    if (pthread_cond_init(&redex_cond, NULL) != 0) {
        fprintf(stderr, "Failed to initialize condition variable\n");
        pthread_mutex_destroy(&redex_mutex);
        free(BUFF);
        BUFF = NULL;
        exit(1);
    }
}

// Free allocated memory
void hvm_free(void) {
    if (BUFF == NULL) {
        return;
    }
    // Destroy mutex and condition variable
    pthread_cond_destroy(&redex_cond);
    pthread_mutex_destroy(&redex_mutex);
    free(BUFF);
    BUFF = NULL;
}

// Reset node and bag indices
void hvm_reset(void) {
    if (BUFF == NULL) {
        fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvm_init first.\n");
        exit(1);
    }
    
    // Clear memory to prevent stale data
    memset(BUFF, 0, RBAG);
    
    // Reset node indices
    RNOD_INI = 0;
    RNOD_END = 0;
    
    // Reset bag indices
    RBAG_INI = RBAG;
    RBAG_END = RBAG;
    
    // Verify indices are valid
    if (RNOD_END >= RBAG_INI) {
        fprintf(stderr, "Error: Node space overlaps with reduction bag space.\n");
        fprintf(stderr, "RNOD_END: %lu, RBAG_INI: %lu\n", RNOD_END, RBAG_INI);
        exit(1);
    }
}

// Convert a tag to its string representation
const char* tag_to_string(Tag tag) {
    switch (tag) {
        case VAL: return "VAL";
        case VAR: return "VAR";
        case SUB: return "SUB";
        case NUL: return "NUL";
        case ERA: return "ERA";
        case LAM: return "LAM";
        case APP: return "APP";
        case REF: return "REF";
        case VL1: return "VL1";
        case SUP: return "SUP";
        case DUP: return "DUP";
        case OPX: return "OPX";
        case OPY: return "OPY";
        case I56: return "I56";
        case F56: return "F56";
        case LAZ: return "LAZ";
        default: return "UNKNOWN";
    }
}

// Create a new term with given tag, label, and location
Term term_new(Tag tag, Lab lab, Location loc) {
    u64 loc_bits = ((u64)loc) & LOC_MASK;
    u64 lab_bits = ((u64)lab) & LAB_MASK;
    u64 tag_bits = ((u64)tag) & TAG_MASK;
    return (loc_bits << (TAG_SIZE + LAB_SIZE)) |
           (lab_bits << TAG_SIZE) |
           tag_bits;
}

// Get the tag of a term
Tag term_tag(Term term) {
  Tag t = (Tag)(term & TAG_MASK);
  if (t == VL1)
    return VAL;
  else
    return t;
}

// Get the label of a term
Lab term_lab(Term term) {
    return (Lab)((term >> TAG_SIZE) & LAB_MASK);
}

// Get the location of a term
Location term_loc(Term term) {
  switch(term_tag(term)) {
  case VAL:
  case SUB:
  case NUL:
  case REF:
  case ERA:
  case I56:
  case F56:
    BOOM("term has no location");
    break;
    
  default:
    return (Location)(term >> (TAG_SIZE + LAB_SIZE));
    break;
  }
  return 0;
}

Location port(u64 n, Location x) {
  if (n != 1 && n != 2) {
    fprintf(stderr, "Error: Invalid port number %lu. Port must be 1 or 2.\n", n);
    exit(1);
  }
  return n + x - 1;
}

// Atomic swap operation
Term swap(Location loc, Term term) {
    return atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
}

Term take(Location loc) {
  Term taken = swap(loc, 0);
  /*
  while (term_tag(taken) == VAR) {
    taken = swap(term_loc(taken), 0);
  }
  // */
  return taken;
}

// Check if a term is positive
bool is_positive(Term term) {
    switch (term_tag(term)) {
        case VAL:
        case VAR:
        case NUL:
        case LAM:
        case REF:
        case VL1:
        case SUP:
        case I56:
        case F56:
        case LAZ:
            return true;
        default:
            return false;
    }
}

// Check if a term is negative
bool is_negative(Term term) {
    switch (term_tag(term)) {
        case SUB:
        case ERA:
        case APP:
        case DUP:
        case OPX:
        case OPY:
            return true;
        default:
            return false;
    }
}

// Get term at location
Term get(Location loc) {
    return atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
}

// Set term at location
void set(Location loc, Term term) {
    atomic_store_explicit(&BUFF[loc], term, memory_order_relaxed);
}

// Create a new pair with given tag, label, and terms
Term pair_make(Tag tag, Lab lab, Term fst, Term snd) {
    // Check if we have enough space for the pair
    if (RNOD_END + 2 >= RBAG_INI) {
        fprintf(stderr, "Error: Not enough space to create pair. RNOD_END=%lu, RBAG_INI=%lu\n",
                RNOD_END, RBAG_INI);
        exit(1);
    }

    // Check port polarities based on pair type
    switch (tag) {
    case LAM:
      // Port 1 must be negative
      if (!is_negative(fst)) {
	fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_string(tag));
	fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      // Port 2 must be positive
      if (!is_positive(snd)) {
	fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_string(tag));
	fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      break;

    case OPX:
    case OPY:
    case APP:
      // Port 1 must be positive
      if (!is_positive(fst)) {
	fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_string(tag));
	fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      // Port 2 must be negative
      if (!is_negative(snd)) {
	fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_string(tag));
	fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      break;

    case DUP:
      // Port 1 must be negative
      if (!is_negative(fst)) {
	fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_string(tag));
	fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      // Port 2 must be negative
      if (!is_negative(snd)) {
	fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_string(tag));
	fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      break;

    case SUP:
      // Port 1 must be positive
      if (!is_positive(fst)) {
	fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_string(tag));
	fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      // Port 2 must be positive
      if (!is_positive(snd)) {
	fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_string(tag));
	fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
	exit(1);
      }
      break;

    default:
      fprintf(stderr, "Error: pair_make called with invalid tag: %s (%d)\n",
	      tag_to_string(tag), tag);
      exit(1);
    }
    
    Location loc = RNOD_END;
    RNOD_END += 2;
    
    // Store terms in their respective ports
    set(port(1, loc), fst);
    set(port(2, loc), snd);
    
    return term_new(tag, lab, loc);
}

// Move a positive term into a negative location
void move(Location neg_loc, Term pos) {
    Term neg = swap(neg_loc, pos);
    if (term_tag(neg) != SUB) {
	take(neg_loc);
	term_link(neg, pos);
    }
}

// Link two terms together
// Push a redex (pair of terms) to the reduction bag
void term_link(Term neg, Term pos) {
  Term neg_var ;
  switch(term_tag(pos)) {
  case VAR:
    neg_var = swap(term_loc(pos), neg);
    if (term_tag(neg_var) != SUB) {
      move(term_loc(pos), neg_var);
    }
    break;

  case I56:
  case F56:
  case NUL:
    interact(neg, pos);
    break;

  default:
    switch(term_tag(neg)) {
    case ERA:
      interact(neg, pos);
      break;

    default:
      push_redex(neg, pos);
      break;
    }
  }
}



// Push a redex (pair of terms) to the reduction bag
void push_redex(Term neg, Term pos) {
  if (term_tag(neg) == ERA)
    BOOM("don't push ERA redex");
  if (term_tag(pos) == NUL)
    BOOM("don't push NUL redex");
  if (is_positive(neg) || is_negative(pos))
    BOOM("bad redex");
  
  // Lock the mutex to ensure thread safety
  pthread_mutex_lock(&redex_mutex);
    
  // Check if the reduction bag is full
  if (RBAG_END >= RBAG_INI + RBAG) {
    fprintf(stderr, "Error: Reduction bag overflow\n");
    pthread_mutex_unlock(&redex_mutex);
    exit(1);
  }
    
  // Store the redex in the bag
  set(RBAG_END, neg);
  set(RBAG_END + 1, pos);
    
  // Update the bag end pointer
  RBAG_END += 2;
    
  // Signal that a redex is available
  pthread_cond_signal(&redex_cond);
    
  // Unlock the mutex
  pthread_mutex_unlock(&redex_mutex);
}

bool stop_reducing = false;

// Pop a redex (pair of terms) from the reduction bag
// Returns false if the bag is empty, true otherwise
bool pop_redex(Term* neg, Term* pos) {
  // Lock the mutex to ensure thread safety
  pthread_mutex_lock(&redex_mutex);
    
  // Check if the reduction bag is empty
  if (RBAG_END <= RBAG_INI) {
    if (stop_reducing) {
      pthread_mutex_unlock(&redex_mutex);
      return false;
    }

    // Wait for a signal that a redex is available
    pthread_cond_wait(&redex_cond, &redex_mutex);
	
    // Check again if the bag is still empty after waking up
    if (RBAG_END <= RBAG_INI) {
      pthread_mutex_unlock(&redex_mutex);
      return false;
    }
  }
    
  // Update the bag end pointer
  RBAG_END -= 2;
    
  // Get the redex from the bag
  *neg = take(RBAG_END);
  *pos = take(RBAG_END + 1);
    
  // Unlock the mutex
  pthread_mutex_unlock(&redex_mutex);
    
  return true;
}

// Application-Lambda interaction
bool applam(Term app, Term lam) {
  Location app_loc = term_loc(app);
  Location lam_loc = term_loc(lam);
    
  // Bounds checking
  if (app_loc >= RNOD_END || lam_loc >= RNOD_END) {
    fprintf(stderr, "Invalid locations: app_loc=%u lam_loc=%u RNOD_END=%lu\n",
	    app_loc, lam_loc, RNOD_END);
    return false;
  }

  // Get locations for each port
  Location arg_loc = port(1, app_loc);
  Location ret_loc = port(2, app_loc);
  Location var_loc = port(1, lam_loc);
  Location bod_loc = port(2, lam_loc);

  // Take the positive terms
  Term arg_val = take(arg_loc);
  Term bod_val = take(bod_loc);

  // Move terms to their new locations
  move(var_loc, arg_val);
  move(ret_loc, bod_val);
  return true;
}

// Distribure a negative term
bool DNEG(Term neg, Term sup) {
  Tag neg_tag = term_tag(neg);
  Lab sup_lab = term_lab(sup);
  Lab neg_lab = term_lab(neg);
  Location neg_loc = term_loc(neg);
  Location sup_loc = term_loc(sup);

  Term arg = take(port(1, neg_loc));
  Location ret = port(2, neg_loc);
  Term tm1 = take(port(1, sup_loc));
  Term tm2 = take(port(2, sup_loc));
  Term dp1 = pair_make(DUP, sup_lab,
		       term_new(SUB, 0, 0),
		       term_new(SUB, 0, 0));
  Term cn1 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(1, term_loc(dp1))),
		       term_new(SUB, 0, 0));
  Term cn2 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(2, term_loc(dp1))),
		       term_new(SUB, 0, 0));
  Term dp2 = pair_make(SUP, sup_lab,
		       term_new(VAR, 0, port(2, term_loc(cn1))),
		       term_new(VAR, 0, port(2, term_loc(cn2))));
  term_link(dp1, arg);
  move(ret, dp2);
  term_link(cn1, tm1);
  term_link(cn2, tm2);
  return true;
}

// Application-Null interaction
bool appnul(Term app, Term nul) {
  Location app_loc = term_loc(app);
  term_link(ERA, take(port(1, app_loc)));
  move(port(2, app_loc), NUL);
  return true;
}

// Duplication-Lambda interaction
bool DLAM(Term dup, Term lam) {
  Lab dup_lab = term_lab(dup);
  Location lam_loc = term_loc(lam);
  Location var = port(1, lam_loc);
  Term bod = take(port(2, lam_loc));
  Term co1 = pair_make(LAM, 0,
		       term_new(SUB, 0, 0),
		       term_new(VAR, 0, 0));
  Term co2 = pair_make(LAM, 0,
		       term_new(SUB, 0, 0),
		       term_new(VAR, 0, 0));
  Term du1 = pair_make(SUP, dup_lab,
		       term_new(VAR, 0, port(1, term_loc(co1))),
		       term_new(VAR, 0, port(1, term_loc(co2))));
  Term du2 = pair_make(DUP, dup_lab,
		       term_new(SUB, 0, 0),
		       term_new(SUB, 0, 0));
  set(port(2, term_loc(co1)), term_new(VAR, 0, port(1, term_loc(du2))));
  set(port(2, term_loc(co2)), term_new(VAR, 0, port(2, term_loc(du2))));
  move(port(1, term_loc(dup)), co1);
  move(port(2, term_loc(dup)), co2);
  move(var, du1);
  term_link(du2, bod);
  return true;
}

// Duplication-Superposition interaction
bool DSUP(Term dup, Term sup) {
  Lab dup_lab = term_lab(dup);
  Lab sup_lab = term_lab(sup);

  if (dup_lab == sup_lab) {
    // Special case: when DUP and SUP have the same label, they annihilate
    // Get the ports of the DUP node
    Location dup_loc = term_loc(dup);
    Location dup_p1 = port(1, dup_loc);
    Location dup_p2 = port(2, dup_loc);
    
    // Get the ports of the SUP node
    Location sup_loc = term_loc(sup);
    Term sup_p1 = take(port(1, sup_loc));
    Term sup_p2 = take(port(2, sup_loc));
    
    // Direct connection of the ports
    move(dup_p1, sup_p1);
    move(dup_p2, sup_p2);
  } else {
    // Get the ports of the DUP node
    Location dup_loc = term_loc(dup);
    Location dup_p1 = port(1, dup_loc);
    Location dup_p2 = port(2, dup_loc);
    
    // Get the ports of the SUP node
    Location sup_loc = term_loc(sup);
    Term sup_p1 = take(port(1, sup_loc));
    Term sup_p2 = take(port(2, sup_loc));
    
    // Create two new DUP nodes with the same label
    Term dup1 = pair_make(DUP, dup_lab, term_new(SUB, 0, 0), term_new(SUB, 0, 0));
    Term dup2 = pair_make(DUP, dup_lab, term_new(SUB, 0, 0), term_new(SUB, 0, 0));
    
    // Create two new SUP nodes with the same label
    Term sup1 = pair_make(SUP, sup_lab,
			  term_new(VAR, 0, port(1, term_loc(dup1))),
			  term_new(VAR, 0, port(1, term_loc(dup2))));
    Term sup2 = pair_make(SUP, sup_lab,
			  term_new(VAR, 0, port(2, term_loc(dup1))),
			  term_new(VAR, 0, port(2, term_loc(dup2))));
    
    // Connect the new nodes
    move(dup_p1, sup1);
    move(dup_p2, sup2);
    term_link(dup1, sup_p1);
    term_link(dup2, sup_p2);
  }
    
  return true;
}

// Duplication interaction with copyable term
bool copy(Term dup, Term trm) {
  Location dup_loc = term_loc(dup);
  
  // Get port locations
  Location dp1_loc = port(1, dup_loc);
  Location dp2_loc = port(2, dup_loc);
  
  // put trm in both copy ports
  move(dp1_loc, trm);
  move(dp2_loc, trm);
  return true;
}

// Eraser-Lambda interaction
bool eralam(Term era, Term lam) {
  Location lam_loc = term_loc(lam);
  move(port(1, lam_loc), NUL);
  term_link(ERA, take(port(2, lam_loc)));
  return true;
}

// Eraser-Superposition interaction
bool erasup(Term era, Term sup) {
  Location sup_loc = term_loc(sup);
  term_link(ERA, take(port(1, sup_loc)));
  term_link(ERA, take(port(2, sup_loc)));
  return true;
}

bool appref(Term app, Term ref) {
  interactionFn fnPtr;
  fnPtr = (interactionFn)(ref & ~0xF);
  fnPtr(ref, app);
  return true;
}

bool appnum(Term app, Term num) {
  Location app_loc = term_loc(app);
  term_link(num, take(port(1, app_loc)));
  move(port(2, app_loc), num);
  return true;
}

bool opnul(Term op, Term nul) {
  Location op_loc = term_loc(op);
  term_link(ERA, take(port(1, op_loc)));
  move(port(2, op_loc), nul);
  return true;
}

bool XNUM(Term opx, Term num) {
  Location opx_loc = term_loc(opx);
  Term arg = swap(port(1, opx_loc), num);
  term_link(term_new(OPY, term_lab(opx), opx_loc), arg);
  return true;
}

// Utilities
i64 u64_to_i64(u64 u) { return *(i64*)&u; }
// f64 u64_to_f64(u64 u) { return *(f64*)&u; }
u64 i64_to_u64(i64 i) { return *(u64*)&i; }
// u64 f64_to_u64(f64 f) { return *(u64*)&f; }

// These macros build all the different casts to make the operations work
// There's a ton of repetitive copy and paste code
#define CASES_u64(a, b) \
  case OP_MOD: val = a %  b; break; \
  case OP_AND: val = a &  b; break; \
  case OP_OR : val = a |  b; break; \
  case OP_XOR: val = a ^  b; break; \
  case OP_LSH: val = a << b; break; \
  case OP_RSH: val = a >> b; break;
#define CASES_i64(a, b) CASES_u64(a, b)
#define CASES_f64(a, b)

#define PERFORM_OP(x, y, op, type)		\
  {						\
    type val;					\
    type a = u64_to_##type(x);			\
    type b = u64_to_##type(y);			\
    switch (op) {				\
    case OP_ADD: val = a +  b; break;		\
    case OP_SUB: val = a -  b; break;		\
    case OP_MUL: val = a *  b; break;		\
    case OP_DIV: val = a /  b; break;		\
    case OP_EQ : val = a == b; break;		\
    case OP_NE : val = a != b; break;		\
    case OP_LT : val = a <  b; break;		\
    case OP_GT : val = a >  b; break;		\
    case OP_LTE: val = a <= b; break;		\
    case OP_GTE: val = a >= b; break;		\
      CASES_##type(a, b)			\
	}					\
    res = type##_to_u64(val);			\
  }

bool YNUM(Term opy, Term num) {
  Location op_loc = term_loc(opy);
  Term x = take(port(1, op_loc));
  Tag y_type = term_tag(num);
  Location ret = port(2, op_loc);
  u32 res;
  Lab op = term_lab(opy);

  switch (y_type) {
  case I56: PERFORM_OP(get_u64(x), get_u64(num), op, i64); break;
    // case F56: PERFORM_OP(x, y, op, f64); break;
  }

  move(ret, new_num(y_type, res));
  return true;
}

// The Void Interaction.
bool NOP(Term neg, Term pos) {
  return true;
}

bool ABRT(Term neg, Term pos) {
  fprintf(stderr, "Bad interaction: %s %s\n",
	  tag_to_string(term_tag(neg)), tag_to_string(term_tag(pos)));
  fprintf(stderr, "a: %p b: %p\n", (void *)neg, (void *)pos);
  /*
  if (term_tag(pos) == VAL) {
    fprintf(stderr, "val type %d: %ld\n", __LINE__, ((Value *)((u64)a & ~7))->type);
  }
  // */
  abort();
}

// Define a macro for the default interaction functions
#define POS_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT
  //VAL  VAR   SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I56   F56   LAZ

#define NUM_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
  //VAL  VAR   SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I56  F56   LAZ

#define OPX_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&XNUM,&XNUM,&ABRT
  //VAL  VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I56   F56   LAZ

#define OPY_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&YNUM,&YNUM,&ABRT
  //VAL  VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I56   F56   LAZ

#define ERA_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&NOP,&ABRT,&eralam,&ABRT,&NOP,&ABRT,&erasup,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
  //VAL  VAR   SUB   NUL   ERA   LAM    APP   REF  VL1    SUP    DUP   OPX   OPY   I56  F56  LAZ

#define APP_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&appnul,&ABRT,&applam,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&appnum,&appnul,&ABRT
  //VAL  VAR   SUB    NUL    ERA    LAM    APP   REF   VL1   SUP   DUP   OPX   OPY    I56     F56   LAZ

#define DUP_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&copy,&ABRT,&DLAM,&ABRT,&copy,&ABRT,&DSUP,&ABRT,&ABRT,&ABRT,&copy,&copy,&ABRT
  //VAL  VAR   SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I56   F56   LAZ

// Initialize the interactions array with the same values in each row
interactionFn interactions[16][16] = {
  { POS_INTERACTIONS }, // VAL  +
  { POS_INTERACTIONS }, // VAR  +
  { POS_INTERACTIONS }, // SUB  - [{+ -}]
  { POS_INTERACTIONS }, // NUL  +
  { ERA_INTERACTIONS }, // ERA  -
  { POS_INTERACTIONS }, // LAM  + {- +}
  { APP_INTERACTIONS }, // APP  - {+ -}
  { POS_INTERACTIONS }, // REF  +
  { POS_INTERACTIONS }, // VL1  +
  { POS_INTERACTIONS }, // SUP  + {+ +}
  { DUP_INTERACTIONS }, // DUP  - {- -}
  { OPX_INTERACTIONS }, // OPX  - {- +}
  { OPY_INTERACTIONS }, // OPY  - {- +}
  { NUM_INTERACTIONS }, // I56  +
  { NUM_INTERACTIONS }, // F56  +
  { POS_INTERACTIONS }  // LAZ  + {+ -}
};

bool interact(Term neg, Term pos) {
  // Gets the rule type.
  interactionFn rule = interactions[term_tag(neg)][term_tag(pos)];

  // Swaps ports if necessary.
  rule(neg, pos);
  return true;
}

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void normalize(void) {
    Term neg, pos;
    
    // Process redexes until the stack is empty
    while (pop_redex(&neg, &pos)) {
        // Perform the interaction
        interact(neg, pos);
    }
    
    return;
}
