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
    return (Location)(term >> (TAG_SIZE + LAB_SIZE));
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
  return swap(loc, 0);
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
                fprintf(stderr, "Error: LAM pair requires negative term in port 1\n");
                fprintf(stderr, "  Port 1 term tag: %d\n", term_tag(fst));
                exit(1);
            }
            // Port 2 must be positive
            if (!is_positive(snd)) {
                fprintf(stderr, "Error: LAM pair requires positive term in port 2\n");
                fprintf(stderr, "  Port 2 term tag: %d\n", term_tag(snd));
                exit(1);
            }
            break;

        case APP:
            // Port 1 must be positive
            if (!is_positive(fst)) {
                fprintf(stderr, "Error: APP pair requires positive term in port 1\n");
                fprintf(stderr, "  Port 1 term tag: %d\n", term_tag(fst));
                exit(1);
            }
            // Port 2 must be negative
            if (!is_negative(snd)) {
                fprintf(stderr, "Error: APP pair requires negative term in port 2\n");
                fprintf(stderr, "  Port 2 term tag: %d\n", term_tag(snd));
                exit(1);
            }
            break;

        case DUP:
            // Port 1 must be negative
            if (!is_negative(fst)) {
                fprintf(stderr, "Error: DUP pair requires negative term in port 1\n");
                fprintf(stderr, "  Port 1 term tag: %d\n", term_tag(fst));
                exit(1);
            }
            // Port 2 must be negative
            if (!is_negative(snd)) {
                fprintf(stderr, "Error: DUP pair requires negative term in port 2\n");
                fprintf(stderr, "  Port 2 term tag: %d\n", term_tag(snd));
                exit(1);
            }
            break;

        case SUP:
            // Port 1 must be positive
            if (!is_positive(fst)) {
                fprintf(stderr, "Error: SUP pair requires positive term in port 1\n");
                fprintf(stderr, "  Port 1 term tag: %d\n", term_tag(fst));
                exit(1);
            }
            // Port 2 must be positive
            if (!is_positive(snd)) {
                fprintf(stderr, "Error: SUP pair requires positive term in port 2\n");
                fprintf(stderr, "  Port 2 term tag: %d\n", term_tag(snd));
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
    if (stop_reducing)
      return false;

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
    return FALSE;
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
  return TRUE;
}

// Application-Duplicator interaction
bool appsup(Term app, Term sup) {
  Lab sup_lab = term_lab(sup);
  Location app_loc = term_loc(app);
  Location sup_loc = term_loc(sup);

  Term arg = take(port(1, app_loc));
  Location ret = port(2, app_loc);
  Term tm1 = take(port(1, sup_loc));
  Term tm2 = take(port(2, sup_loc));
  Term dp1 = pair_make(DUP, sup_lab,
		       term_new(SUB, 0, 0),
		       term_new(SUB, 0, 0));
  Term dp2 = pair_make(SUP, sup_lab,
		       term_new(VAR, 0, 0),
		       term_new(VAR, 0, 0));
  Term cn1 = pair_make(APP, 0,
		       term_new(VAR, 0, port(1, term_loc(dp1))),
		       term_new(SUB, 0, 0));
  Term cn2 = pair_make(APP, 0,
		       term_new(VAR, 0, port(2, term_loc(dp1))),
		       term_new(SUB, 0, 0));
  term_link(dp1, arg);
  move(ret, dp2);
  term_link(cn1, tm1);
  term_link(cn2, tm2);
  return TRUE;
}

// Application-Null interaction
bool appnul(Term app, Term nul) {
  Location app_loc = term_loc(app);
  Term pos = take(port(1, app_loc));
  
  // Get port locations
  Location ret_loc = port(2, app_loc);
  
  // Set NUL in return port
  move(ret_loc, term_new(NUL, 0, 0));
  term_link(ERA, pos);
  return TRUE;
}

// Duplication-Lambda interaction
bool duplam(Term dup, Term lam) {
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
  return TRUE;
}

// Duplication-Null interaction
bool dupnul(Term dup, Term nul) {
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
  Location var = port(1, lam_loc);
  Term bod = take(port(2, lam_loc));
  move(var, term_new(NUL, 0, 0));
  term_link(term_new(ERA, 0, 0), bod);
  return true;
}

// Eraser-Duplicator interaction
bool erasup(Term era, Term sup) {
  Location sup_loc = term_loc(sup);
  
  // Get port locations
  Location p1_loc = port(1, sup_loc);
  Location p2_loc = port(2, sup_loc);
  
  // Take terms from both ports
  Term p1 = take(p1_loc);
  Term p2 = take(p2_loc);
  
  // Set the terms at the original locations
  term_link(p1, era);
  term_link(p2, era);
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

#define ERA_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&NOP,&ABRT,&eralam,&ABRT,&NOP,&ABRT,&erasup,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
  //VAL  VAR   SUB   NUL   ERA   LAM    APP   REF  VL1   SUP     DUP   OPX   OPY  I56  F56   LAZ

#define APP_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&appnul,&ABRT,&applam,&ABRT,&ABRT,&ABRT,&appsup,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT
  //VAL  VAR   SUB    NUL    ERA    LAM    APP   REF   VL1   SUP     DUP   OPX   OPY   I56   F56   LAZ

#define DUP_INTERACTIONS \
  &ABRT,&ABRT,&ABRT,&copy,&ABRT,&duplam,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&copy,&copy,&ABRT
  //VAL  VAR   SUB   NUL   ERA    LAM    APP   REF   VL1   SUP   DUP   OPX   OPY   I56   F56   LAZ

// Initialize the interactions array with the same values in each row
interactionFn interactions[16][16] = {
  { POS_INTERACTIONS }, // VAL
  { POS_INTERACTIONS }, // VAR
  { POS_INTERACTIONS }, // SUB
  { POS_INTERACTIONS }, // NUL
  { ERA_INTERACTIONS },
  { POS_INTERACTIONS }, // LAM
  { APP_INTERACTIONS },
  { POS_INTERACTIONS }, // REF
  { POS_INTERACTIONS }, // VL1
  { POS_INTERACTIONS }, // SUP
  { DUP_INTERACTIONS },
  { POS_INTERACTIONS }, // OPX
  { POS_INTERACTIONS }, // OPY
  { POS_INTERACTIONS }, // I56
  { POS_INTERACTIONS }, // F56
  { POS_INTERACTIONS }  // LAZ
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
