#include "new.h"
#include <string.h>

// Global heap
static a64* BUFF = NULL;
a64 RNOD_END = 0; // Only need to track the end of the node space
static u64 BUFF_SIZE = 0; // Size of the main buffer for bounds checking

// Free list for O(1) pair allocation
__thread Location FREE_LIST = EMPTY_FREE_LIST; // Head of the free list (atomic for thread safety)

// Redex stack
__thread Pairs pairs;
Term* RBAG_BUFF = NULL; // Using Term (u64) instead of atomic (a64)
static u64 RBAG_SIZE = 0x1000;
a64 RBAG_END; // Only need to track the end of the redex stack
__thread u64 rdxCount = 0;

// interaction jump table
interactionFn interactions[16][16];

// Mutex for thread-safe redex operations
pthread_mutex_t redex_mutex;

// Condition variable for signaling when redex is available
pthread_cond_t redex_cond;

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
  case I60: return "I60";
  case F60: return "F60";
  case LAZ: return "LAZ";
  default: return "UNKNOWN";
  }
}

const char* tag_str(Term t) {
  return tag_to_string(term_tag(t));
}

// abort on invalid reduction
void ABRT(Term neg, Term pos) {
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

void *boom(char *msg, char *file, int line) {
#ifndef SINGLE_THREAD
  pthread_mutex_lock(&redex_mutex);
#endif
  fprintf(stderr, "%s at %s:%d\n", msg, file, line);
  abort();
}

// Get term at location
Term get(Location loc) {
  Term result = atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
  return result;
}

// Atomic swap operation
// If a deferred redex is found, queue it up and return SUB
// Otherwise, return a positive value.
Term swapStore(Location loc, Term term) {
#ifdef SAFETY
  if (term == 0)
    BOOM("bad swap");
#endif
  Term result = atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
  if (term_tag(result) == SUB && result != SUB) {
    Term neg = get(port(1, term_loc(result)));
    Term pos = get(port(2, term_loc(result)));
    store_redex(neg, pos);
    pair_free(term_loc(result));
    return SUB;
  } else
    return result;
}

void freeLoc(Location loc) {
  atomic_store_explicit(&BUFF[loc], 0, memory_order_relaxed);
  Location evenLoc = loc & 0xFFFFFFFE;
  if (get(evenLoc) == 0 && get(evenLoc + 1) == 0) {
    pair_free(evenLoc);
  }
}

// Take the term at the given location, replacing it with 0
// And freeing a VAR chain if needed
// Returns a positive value.
Term taker(unsigned line, Location loc) {
  Tag takenTag;
  Term taken;
  do {
    taken = get(loc);
    takenTag = term_tag(taken);
    if (takenTag != SUB) {
      freeLoc(loc);
      // printf("taking: %.3x at line: %u\n", loc, line);
      if (takenTag == VAR) {
	loc = term_loc(taken);
      }
#ifdef SAFETY
    } else if (taken != SUB) {
      char msg[200];
      sprintf(msg, "should never happen! %p", (void *)taken);
      BOOM(msg);
#endif
    }
  } while (takenTag == VAR);

  if (takenTag == SUB)
    return term_new(VAR, 0, loc);
  else
    return taken;
}

void store_redex(Term neg, Term pos) {
#ifdef SAFETY
  if (neg == 0 && pos == 0)
    // shutdown the threads
    neg = 0;
  else if (is_positive(neg) || is_negative(pos)) {
    BOOM("bad redex");
  } else if (interactions[term_tag(neg)][term_tag(pos)] == &ABRT) {
    BOOM("bad redex");
  }
#endif

  pairs.rdxs[pairs.count][0] = neg;
  pairs.rdxs[pairs.count++][1] = pos;
  if (pairs.count >= LOCAL_PAIRS_SIZE) {
    link_redexes();
  }
}

// Move a positive term into a negative location
// If anything besides a deferred redex is there, it must be a
// negative and should be reduced with 'pos'
void moveStore(Location neg_loc, Term pos) {
  Term neg = swapStore(neg_loc, pos);
#ifdef SAFETY
  if (is_negative(pos)) {
    char s[50];
    sprintf(s,"trying to move a negative to location %.3x: %p", neg_loc, (void *)neg);
    BOOM(s);
  }
  if (is_positive(neg)) {
    char s[50];
    sprintf(s,"found positive at move target %.3x: %p", neg_loc, (void *)neg);
    BOOM(s);
  }
#endif
  if (term_tag(neg) != SUB) {
    freeLoc(neg_loc);
    store_redex(neg, pos);
  }
}

int threadCount = 1;
pthread_t threads[200];

a64 waiting;

// Pop a redex (pair of terms) from the reduction bag
// Returns false if the bag is empty, true otherwise
bool pop_redex(Term* neg, Term* pos) {
  bool result = false;

  if (pairs.count > 0) {
    pairs.count -= 1;
    *neg = pairs.rdxs[pairs.count][0];
    *pos = pairs.rdxs[pairs.count][1];
    return true;
  }

  u64 currTop;
  u64 waitingThreads;
  do {
    currTop = atomic_exchange_explicit(&RBAG_END, LOCK_REDEX_STACK, memory_order_relaxed);

    switch(currTop) {
    case LOCK_REDEX_STACK:
      break;

    case 0:
      pthread_mutex_lock(&redex_mutex);
      waitingThreads = atomic_fetch_add_explicit(&waiting, 1, memory_order_relaxed);
      atomic_store_explicit(&RBAG_END, 0, memory_order_relaxed);
      pthread_cond_wait(&redex_cond, &redex_mutex);
      u64 currWaiting = atomic_fetch_add_explicit(&waiting, -1, memory_order_relaxed);
      if (currWaiting > 0) {
	pthread_cond_signal(&redex_cond);
      }
      pthread_mutex_unlock(&redex_mutex);
      currTop = LOCK_REDEX_STACK;
      break;

    default:
      currTop -= 2;
      *neg = RBAG_BUFF[currTop];
      *pos = RBAG_BUFF[currTop + 1];
      atomic_store_explicit(&RBAG_END, currTop, memory_order_relaxed);
      if (*neg == 0 && *pos == 0) {
	/*
	waitingThreads = atomic_load_explicit(&waiting, memory_order_relaxed);
	if (waitingThreads > 0) {
	  pthread_mutex_lock(&redex_mutex);
	  printf("signal %d %lu %lu\n", __LINE__, currTop, waitingThreads);
	  pthread_cond_signal(&redex_cond);
	  pthread_mutex_unlock(&redex_mutex);
	}
	// */
	result = false;
      } else
	result = true;
      break;
    }
  } while (currTop == LOCK_REDEX_STACK);

#ifdef SAFETY
  if (*neg == 0 || *pos == 0)
    abort();
#endif
  return result;
}

a64 glblAlloced;

// Allocate a pair from the free list - O(1)
// By popping a value from the free stack
Location pair_alloc(void) {
#ifdef SAFETY
  atomic_fetch_add_explicit(&glblAlloced, 1, memory_order_relaxed);
#endif
  Location loc;
  do {
    loc = FREE_LIST;
    switch(loc) {
    case LOCK_FREE_LIST:
      break;

    case EMPTY_FREE_LIST:
      loc = atomic_fetch_add_explicit(&RNOD_END, 2, memory_order_relaxed);
      // printf("new pair: %d\n", loc);
      // Check if we have space in the buffer
      if (loc >= BUFF_SIZE) {
	fprintf(stderr, "Error: Not enough space to allocate pair. RNOD_END=%u, BUFF_SIZE=%lu\n",
		loc, BUFF_SIZE);
	abort();
      }
      break;
      
    default:
      if (1) {
	// Get the next free pair location
	Term next = get(loc);
	Location new_free_list = (Location)(next >> (TAG_SIZE + LAB_SIZE));
	FREE_LIST = new_free_list;
      }

      /* for the redex stack
      loc = freeStack[currTop];
      currTop--;
      atomic_store_explicit(&freeStackPtr, currTop, memory_order_relaxed);
      // */
      break;
    }
  } while (loc == LOCK_FREE_LIST);
  return (Location)loc;
}

// Free a pair by adding it to the free list - O(1)
void freer(unsigned line, Location loc) {
  // printf("free pair at: %d\n", loc);
#ifdef SAFETY
  atomic_fetch_add_explicit(&glblAlloced, -1, memory_order_relaxed);
#endif

  // Clear the second cell
  atomic_store_explicit(&BUFF[loc + 1], 0, memory_order_relaxed);

  Location currTop;
  do {
    currTop = FREE_LIST;
    switch(currTop) {
    case LOCK_FREE_LIST:
      break;

    default:
      // Set up the node to point to the current head
      atomic_store_explicit(&BUFF[loc], term_new(NUL, 0, currTop), memory_order_relaxed);
      FREE_LIST = loc;
      break;
    }
  } while (currTop == LOCK_FREE_LIST);
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
#ifdef SAFETY
  switch(term_tag(term)) {
  case VAL:
  case NUL:
  case REF:
  case ERA:
  case I60:
  case F60:
    BOOM("term has no location");
    break;

  // Allow SUB terms to have locations
  case SUB:
  default:
    return (Location)(term >> (TAG_SIZE + LAB_SIZE));
    break;
  }
  return 0;
#else
  return (Location)(term >> (TAG_SIZE + LAB_SIZE));
#endif
}

Location port(u64 n, Location x) {
#ifdef SAFETY
  if (n != 1 && n != 2) {
    fprintf(stderr, "Error: Invalid port number %lu. Port must be 1 or 2.\n", n);
    abort();
  }
#endif
  return n + x - 1;
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
  case I60:
  case F60:
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

// Create a new pair with given tag, label, and terms
Term pair_maker(unsigned line, Tag tag, Lab lab, Term fst, Term snd) {
#ifdef SAFETY
  // Check port polarities based on pair type
  switch (tag) {
  case SUB:
  case LAM:
    // Port 1 must be negative
    if (!is_negative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_string(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_string(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    break;

  case OPX:
  case OPY:
  case APP:
    // Port 1 must be positive
    if (!is_positive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_string(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_string(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    break;

  case DUP:
    // Port 1 must be negative
    if (!is_negative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_string(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_string(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    break;

  case SUP:
    // Port 1 must be positive
    if (!is_positive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_string(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_string(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_string(term_tag(snd)));
      abort();
    }
    break;

  default:
    fprintf(stderr, "Error: pair_make called with invalid tag: %s (%d)\n",
	    tag_to_string(tag), tag);
    abort();
  }
#endif

  // Get a pair from the free list or by extending RNOD_END
  Location loc = pair_alloc();

#ifdef SAFETY
  // TODO: remove
  if (loc & 0x1)
    BOOM("Bad pair_alloc return");
#endif

  // Store terms in their respective ports
  atomic_store_explicit(&BUFF[port(1, loc)], fst, memory_order_relaxed);
  atomic_store_explicit(&BUFF[port(2, loc)], snd, memory_order_relaxed);

  Term new_pair = term_new(tag, lab, loc);
  /*
  printf("new pair at line %u: %s %.3x %p %p\n", line, tag_to_string(tag), loc,
	 (void *)get(port(1, loc)),
	 (void *)get(port(2, loc)));
  // */
  return new_pair;
}

void store_pair(Pairs *pairs, Term neg, Term pos) {
  pairs->rdxs[pairs->count][0] = neg;
  pairs->rdxs[pairs->count++][1] = pos;
}

void link_redexes() {
  Pairs pushing;
  pushing.count = 0;

  Pairs immediate;
  immediate.count = 0;

  for (int i = 0; i < pairs.count; i++) {
    Term neg = pairs.rdxs[i][0];
    Term pos = pairs.rdxs[i][1];

    switch(term_tag(neg)) {
    case ERA:
      immediate.rdxs[immediate.count][0] = neg;
      immediate.rdxs[immediate.count++][1] = pos;
      break;

    default:
      switch(term_tag(pos)) {
      case I60:
      case F60:
      case NUL:
      case REF:
	immediate.rdxs[immediate.count][0] = neg;
	immediate.rdxs[immediate.count++][1] = pos;
	break;

      case VAR: {
	  Term val = take(term_loc(pos));
	  switch(term_tag(val)) {
	  case I60:
	  case F60:
	  case NUL:
	    immediate.rdxs[immediate.count][0] = neg;
	    immediate.rdxs[immediate.count++][1] = val;
	    break;

	  case VAR:
	    if (1) {
	      Term deferred = pair_make(SUB, 4, neg, val);
	      Term newVal = swapStore(term_loc(val), deferred);
	      if (term_tag(newVal) == SUB) {
		if (newVal != SUB)
		  BOOM("This shouldn't happen, should it?");
	      } else {
		// printf("deferred sub: %.3x\n", term_loc(deferred));
		// print_term("neg", neg);
		// print_term("val", val);
		// print_term("newVal", newVal);
		pair_free(term_loc(deferred));
		freeLoc(term_loc(val));
		store_redex(neg, newVal);
	      }
	    }
	    break;

	  default:
	    pushing.rdxs[pushing.count][0] = neg;
	    pushing.rdxs[pushing.count++][1] = val;
	    break;
	  }
	}
	break;

      default:
	pushing.rdxs[pushing.count][0] = neg;
	pushing.rdxs[pushing.count++][1] = pos;
	break;
      }
    }
  }
  pairs.count = 0;
  unsigned pushCount = LOCAL_PAIRS_SIZE / 2;
  if (pushCount > pushing.count)
    pushCount = pushing.count;
  printf("pushing: %u immediate: %u  pushed: %u\n", pushing.count, immediate.count, pushCount);

  if (pushing.count > 0) {
    u64 currTop;
    do {
      currTop = atomic_exchange_explicit(&RBAG_END, LOCK_REDEX_STACK, memory_order_relaxed);
      switch (currTop) {
      case LOCK_REDEX_STACK:
	break;

      default:
	if (1) {
#ifdef SAFETY
	  // Check if there's space in the bag
	  if (currTop + pushCount > RBAG_SIZE) {
	    fprintf(stderr, "Error: Redex bag is full. RBAG_END=%lu, RBAG_SIZE=%lu\n",
		    currTop, RBAG_SIZE);
	    abort();
	  }
#endif
	  u64 newTop = currTop;
	  for (int i = 1; i < pushCount; i++, newTop += 2) {
	    // Store the redex in the bag
	    RBAG_BUFF[newTop] = pushing.rdxs[i][0];
	    RBAG_BUFF[newTop + 1] = pushing.rdxs[i][1];
	  }

#ifndef SINGLE_THREAD
	  u64 waitingThreads = atomic_load_explicit(&waiting, memory_order_relaxed);
	  if (waitingThreads > 0) {
	    pthread_mutex_lock(&redex_mutex);
	    pthread_cond_signal(&redex_cond);
	    pthread_mutex_unlock(&redex_mutex);
	  }
#endif
	  atomic_store_explicit(&RBAG_END, newTop, memory_order_relaxed);
	}
      }
    } while (currTop == LOCK_REDEX_STACK);
  }

  for (int i = pushCount; i < pushing.count; i++) {
    Term neg = pushing.rdxs[i][0];
    Term pos = pushing.rdxs[i][1];

    store_redex(neg, pos);
  }

  for (int i = 0; i < immediate.count; i++) {
    Term neg = immediate.rdxs[i][0];
    Term pos = immediate.rdxs[i][1];

    store_redex(neg, pos);
  }
}

void DEFR(Term neg, Term var) {
  var = take(term_loc(var));
  if (term_tag(var) == VAR) {
    Location varLoc = term_loc(var);
    Term deferred = pair_make(SUB, 2, neg, var);
    Term newVar = swapStore(varLoc, deferred);
    if (term_tag(newVar) != SUB) {
      pair_free(term_loc(deferred));
      freeLoc(varLoc);
      store_redex(neg, newVar);
    }
  } else {
    interact(neg, var);
  }
  return;
}

// Application-Lambda interaction
void applam(Term app, Term lam) {
  Location app_loc = term_loc(app);
  Location lam_loc = term_loc(lam);

  // Get locations for each port
  Location arg_loc = port(1, app_loc);
  Location ret_loc = port(2, app_loc);
  Location var_loc = port(1, lam_loc);
  Location bod_loc = port(2, lam_loc);

  // Take the positive terms
  Term arg_val = take(arg_loc);
  Term bod_val = take(bod_loc);

  // Move terms to their new locations
  moveStore(var_loc, arg_val);
  moveStore(ret_loc, bod_val);
  return;
}

// Distribure a negative term
void DNEG(Term neg, Term sup) {
  BOOM("DNEG\n");
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
		       SUB,
		       SUB);
  Term cn1 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(1, term_loc(dp1))),
		       SUB);
  Term cn2 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(2, term_loc(dp1))),
		       SUB);
  Term dp2 = pair_make(SUP, sup_lab,
		       term_new(VAR, 0, port(2, term_loc(cn1))),
		       term_new(VAR, 0, port(2, term_loc(cn2))));
  moveStore(ret, dp2);
  store_redex(cn2, tm2);
  store_redex(cn1, tm1);
  store_redex(dp1, arg);
  return;
}

// Application-Null interaction
void appnul(Term app, Term nul) {
  Location app_loc = term_loc(app);
  store_redex(ERA, take(port(1, app_loc)));
  moveStore(port(2, app_loc), NUL);
  return;
}

// Duplication-Lambda interaction
void DLAM(Term dup, Term lam) {
  BOOM("DLAM");
  Lab dup_lab = term_lab(dup);
  Location lam_loc = term_loc(lam);
  Location var = port(1, lam_loc);
  Term bod = take(port(2, lam_loc));
  Term co1 = pair_make(LAM, 0,
		       SUB,
		       term_new(VAR, 0, 0));
  Term co2 = pair_make(LAM, 0,
		       SUB,
		       term_new(VAR, 0, 0));
  Term du1 = pair_make(SUP, dup_lab,
		       term_new(VAR, 0, port(1, term_loc(co1))),
		       term_new(VAR, 0, port(1, term_loc(co2))));
  Term du2 = pair_make(DUP, dup_lab,
		       SUB,
		       SUB);
  swapStore(port(2, term_loc(co1)), term_new(VAR, 0, port(1, term_loc(du2))));
  swapStore(port(2, term_loc(co2)), term_new(VAR, 0, port(2, term_loc(du2))));
  moveStore(port(1, term_loc(dup)), co1);
  moveStore(port(2, term_loc(dup)), co2);
  moveStore(var, du1);
  store_redex(du2, bod);
  return;
}

// Duplication-Superposition interaction
void DSUP(Term dup, Term sup) {
  printf("DSUP\n");
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
    moveStore(dup_p1, sup_p1);
    moveStore(dup_p2, sup_p2);
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
    Term dup1 = pair_make(DUP, dup_lab, SUB, SUB);
    Term dup2 = pair_make(DUP, dup_lab, SUB, SUB);

    // Create two new SUP nodes with the same label
    Term sup1 = pair_make(SUP, sup_lab,
			  term_new(VAR, 0, port(1, term_loc(dup1))),
			  term_new(VAR, 0, port(1, term_loc(dup2))));
    Term sup2 = pair_make(SUP, sup_lab,
			  term_new(VAR, 0, port(2, term_loc(dup1))),
			  term_new(VAR, 0, port(2, term_loc(dup2))));

    // Connect the new nodes
    moveStore(dup_p1, sup1);
    moveStore(dup_p2, sup2);
    store_redex(dup2, sup_p2);
    store_redex(dup1, sup_p1);
  }

  return;
}

// Duplication interaction with copyable term
void copy(Term dup, Term trm) {
#ifdef SAFETY
  // Verify term polarities
  if (!is_negative(dup) || !is_positive(trm)) {
    fprintf(stderr, "Error in copy: incorrect term polarities. dup=%s, trm=%s\n",
            tag_to_string(term_tag(dup)), tag_to_string(term_tag(trm)));
    abort();
  }
#endif

  Location dup_loc = term_loc(dup);

  // Get port locations
  Location dp1_loc = port(1, dup_loc);
  Location dp2_loc = port(2, dup_loc);

  // put trm in both copy ports
  moveStore(dp2_loc, trm);
  moveStore(dp1_loc, trm);
  return;
}

// Eraser-Lambda interaction
void eralam(Term era, Term lam) {
  BOOM("eralam");
  Location lam_loc = term_loc(lam);
  store_redex(ERA, take(port(2, lam_loc)));
  moveStore(port(1, lam_loc), NUL);
  return;
}

// Eraser-Superposition interaction
void erasup(Term era, Term sup) {
  Location sup_loc = term_loc(sup);
  store_redex(ERA, take(port(2, sup_loc)));
  store_redex(ERA, take(port(1, sup_loc)));
  return;
}

// Create a REF term with a specific interaction function
Term ref_make(interactionFn fn) {
  // Store the function pointer in the term, with the REF tag
  return ((Term)fn & ~0xF) | REF;
}

// APP-REF interaction
// When an APP term meets a REF term, call the function stored in the REF term
void appref(Term app, Term ref) {
  interactionFn fnPtr;
  fnPtr = (interactionFn)(ref & ~0xF);
  fnPtr(ref, app);
  return;
}

void appnum(Term app, Term num) {
  BOOM("appnum");
  Location app_loc = term_loc(app);
  moveStore(port(2, app_loc), num);
  store_redex(ERA, take(port(1, app_loc)));
  return;
}

void opnul(Term op, Term nul) {
  BOOM("opnul");
  Location op_loc = term_loc(op);
  moveStore(port(2, op_loc), nul);
  store_redex(ERA, take(port(1, op_loc)));
  return;
}

// SUB-NUL interaction
void subnul(Term sub, Term nul) {
  BOOM("subnul");
  // Check if the SUB term has a location (label > 0)
  if (sub != SUB) {
    // The SUB term has a location pointing to a pair
    Location sub_loc = term_loc(sub);

    // Take the first port and link it with NUL
    Term t = take(port(1, sub_loc));
    store_redex(t, NUL);

    // Take the second port and link it with ERA
    t = take(port(2, sub_loc));
    store_redex(ERA, t);
  }

  return;
}

void XNUM(Term opx, Term num) {
  Location opx_loc = term_loc(opx);
  Term arg = swapStore(port(1, opx_loc), num);
  store_redex(term_new(OPY, term_lab(opx), port(1, opx_loc)), arg);
  return;
}

// Utilities
i64 u64_to_i64(u64 u) { return *(i64*)&u; }
// f64 u64_to_f64(u64 u) { return *(f64*)&u; }
u64 i64_to_u64(i64 i) { return *(u64*)&i; }
// u64 f64_to_u64(f64 f) { return *(u64*)&f; }

// These macros build all the different casts to make the operations work
// There's a ton of repetitive copy and paste code
#define CASES_u64(a, b)				\
  case OP_MOD: val = a %  b; break;		\
  case OP_AND: val = a &  b; break;		\
  case OP_OR : val = a |  b; break;		\
  case OP_XOR: val = a ^  b; break;		\
  case OP_LSH: val = a << b; break;		\
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

void YNUM(Term opy, Term num) {
  Location op_loc = term_loc(opy);
  Term x = take(port(1, op_loc));
  Tag y_type = term_tag(num);
  Location ret = port(2, op_loc);
  u64 res;
  Lab op = term_lab(opy);

  switch (y_type) {
  case I60: PERFORM_OP(get_u64(x), get_u64(num), op, i64); break;
    // case F60: PERFORM_OP(x, y, op, f64); break;
  }

  moveStore(ret, new_num(y_type, res));
  return;
}

// The Void Interaction.
void NOP(Term neg, Term pos) {
  return;
}

// Define a macro for the default interaction functions
#define POS_INTERACTIONS\
  &ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT
// VAL  VAR    SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define SUB_INTERACTIONS\
  &ABRT,&ABRT,&ABRT,&subnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT
// VAL   VAR   SUB    NUL    ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define NUM_INTERACTIONS\
  &ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
// VAL   VAR   SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60  F60  LAZ

#define OPX_INTERACTIONS\
  &ABRT,&DEFR,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&XNUM,&XNUM,&ABRT
// VAL   VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define OPY_INTERACTIONS						\
  &ABRT,&DEFR,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&YNUM,&YNUM,&ABRT
// VAL   VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define ERA_INTERACTIONS						\
  &ABRT,&ABRT,&ABRT,&NOP,&ABRT,&eralam,&ABRT,&NOP,&ABRT,&erasup,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
// VAL   VAR   SUB   NUL   ERA   LAM    APP   REF  VL1    SUP    DUP   OPX   OPY   I60  F60  LAZ

#define APP_INTERACTIONS						\
  &ABRT,&DEFR,&ABRT,&appnul,&ABRT,&applam,&ABRT,&appref,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&appnum,&appnul,&ABRT
// VAL   VAR   SUB    NUL    ERA    LAM    APP    REF    VL1   SUP   DUP   OPX   OPY    I60     F60   LAZ

#define DUP_INTERACTIONS						\
  &ABRT,&ABRT,&ABRT,&copy,&ABRT,&DLAM,&ABRT,&copy,&ABRT,&DSUP,&ABRT,&ABRT,&ABRT,&copy,&copy,&ABRT
// VAL   VAR   SUB   NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

// Initialize the interactions array with the same values in each row
interactionFn interactions[16][16] = {
  { POS_INTERACTIONS }, // VAL  +
  { POS_INTERACTIONS }, // VAR  +
  { SUB_INTERACTIONS }, // SUB  - [{+ -}]
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
  { NUM_INTERACTIONS }, // I60  +
  { NUM_INTERACTIONS }, // F60  +
  { POS_INTERACTIONS }  // LAZ  + {+ -}
};

void interact(Term neg, Term pos) {
  // print_raw_term(neg);
  // printf("  ");
  // print_raw_term(pos);
  // printf("\n");
  rdxCount++;
  // Gets the rule type.
  interactionFn rule = interactions[term_tag(neg)][term_tag(pos)];

  // Swaps ports if necessary.
  rule(neg, pos);
  return;
}

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void *normalize(void *v) {
  Term neg, pos;

  // Process redexes until the stack is empty
  while (pop_redex(&neg, &pos)) {
    // Perform the interaction
    interact(neg, pos);
  }
  //*
  u64 waitingThreads = atomic_load_explicit(&waiting, memory_order_relaxed);
  if (waitingThreads > 0) {
    pthread_mutex_lock(&redex_mutex);
    pthread_cond_signal(&redex_cond);
    pthread_mutex_unlock(&redex_mutex);
  }
  // */

  u64 *res = malloc(sizeof(u64));
  *res = rdxCount;
  return res;
}

Term argsNet(NativeArgs *args) {
  Term tail;
  if(args->count < 1)
    BOOM("argsNet");
  else
    tail = args->args[args->count - 1];

  for (int i = args->count - 2; i >= 0; i--) {
    tail = pair_make(APP, 0, args->args[i], tail);
    args->args[i] = tail;
  }

  return args->args[0];
}

#ifdef NADA
void forceLazy(Term z) {
  // 'z' is a LAZ term
  Term neg = take(port(1, term_loc(z)));
  if (neg != VOID) {
    Term pos = take(port(2, term_loc(z)));
    if (term_tag(neg) == DUP && term_tag(pos) == LAZ) {
      BOOM("we do need this, it appears");
      /*
      Term curr = swap(port(1, term_loc(neg)), SUB);
      if (curr != z)
	set(port(1, term_loc(neg)), curr);
      BOOM("don't swap");
      curr = swap(port(2, term_loc(neg)), SUB);
      if (curr != z)
	set(port(2, term_loc(neg)), curr);
      // set(posLoc, pair_make(SUB, neg, term_new(VAR, 0, posLoc)));
      forceLazy(pos);
      // */
    } else if (term_tag(neg) == DUP && term_tag(pos) == VAR) {
      // if this is a lazy DUP, which ever port points to itself
      // gets replaced with SUB
      Term curr = get(port(1, term_loc(neg)));
      if (curr == z)
	set(port(1, term_loc(neg)), SUB);
      curr = get(port(2, term_loc(neg)));
      if (curr == z)
	set(port(2, term_loc(neg)), SUB);

      Term newPos = take(term_loc(pos));
      // newPos is the term being duped
      /*
      while (term_tag(newPos) == VAR) {
	pos = newPos;
	newPos = take(term_loc(pos));
      }
      // */
      switch(term_tag(newPos)) {
      case LAZ:
	BOOM("this is totally wrong");
	// see the loop commented out above
	set(term_loc(newPos), pair_make(SUB, 7, neg, pos));
	BOOM("*** what if newPos is not lazy? %d\n");
	forceLazy(newPos);
	break;

      case VAR:
	set(term_loc(newPos), pair_make(SUB, 6, neg, newPos)));
	break;

      default:
	store_redex(neg, newPos);
      }
    } else {
      store_redex(neg, pos);
    }
  }
}
#endif

// extract the requested number of native args. I60, F60, REF or VAL terms
//*
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct) {
  // 'args' will only ever be an APP term
  Tag argsTag = term_tag(args);
  if (argsTag == APP) {
    Term arg = take(port(1, term_loc(args)));
    if (expected == 0) {
      return args;
    }

    // 'arg' will only ever be a positive term
    Tag argTag = term_tag(arg);
    switch(argTag) {
      // the strict arg types
    case VAL:
    case I60:
    case F60:
    case REF:
      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = arg;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
      else
	return args;
      break;

    case VAR: {
      Term val = take(term_loc(arg));
      switch(term_tag(val)) {
	// the strict arg types
      case VAL:
      case I60:
      case F60:
      case REF: {
	  argsStruct->args[argsStruct->count++] = val;
	  if (expected > 1)
	    return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
	  else
	    return args;
	}
	break;

      case VAR: {
	  Term val = get(term_loc(arg));
	  if (val != SUB)
	    BOOM("nativeArgs");
	  else {
	    // add the remaining args to argsStruct
	    argsStruct->args[argsStruct->count++] = args;

	    // create a chain of APP terms from argsStruct
	    Term newArgs = argsNet(argsStruct);

	    // put 'arg' back in it's place
	    swapStore(port(1, term_loc(args)), arg);

	    // make a deferred redex to retry the APP/REF pair when the value becomes available
	    Term retry = pair_make(SUB, 5, newArgs, ref);

	    // and put it in the location 'arg' points to
	    Term newArg = swapStore(term_loc(arg), retry);
	    if (newArg != SUB) {
	      // someone slipped the needed arg in since we last looked
	      swapStore(term_loc(arg), newArg);
	      pair_free(term_loc(retry));

	      // so retry the original APP/REF redex
	      store_redex(newArgs, ref);
	    }
	    argsStruct->count = -1;
	    return 0;
	  }
	}
	break;

      default: {
	char s[50];
	sprintf(s, "bad %s val", tag_to_string(term_tag(val)));
	BOOM(s);
      }
	break;
      }
      argsStruct->count = -1;
      return 0;
    }
      break;

    case SUP:
    case LAM:
    case LAZ:
    case NUL:
    default:
      printf("unhandled tag %s (0x%x) line: %d\n", tag_to_string(term_tag(arg)),
	     term_tag(arg), __LINE__);
      abort();
      break;
    }
    argsStruct->count = -1;
    // /
    return 0;
  } else {
    printf("unhandled tag %s (0x%x) %p line: %d\n",
	   tag_to_string(argsTag), argsTag, (void *)args, __LINE__);
    abort();
    return 0;
  }
}
// */

// For testing only
void print_raw_term(Term t) {
  if (t == 0) {
    printf("  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    switch(term_tag(t)) {
    case VAL:
    case NUL:
    case ERA:
    case I60:
    case F60:
      printf("%s %x", tag_to_string(tag), lab);
      break;

    case REF:
      printf("REF %s", refName(t));
      break;

    default:
      printf("%s %x %.3x", tag_to_string(tag), lab, term_loc(t));
      break;
    }
  }
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  printf("%s:\n", prefix);
  printf("  Tag: %s (%d)\n", tag_to_string(term_tag(term)), term_tag(term));
  switch(term_tag(term)) {
  case VAL:
  case NUL:
  case REF:
  case ERA:
  case F60:
    break;

  case I60:
    printf("  Val: %ld", get_i60(term));
    break;

  case VAR:
    printf("  Location: %.3x\n", term_loc(term));
    // If this is a pair, print its contents
    if (term_loc(term) >= 0) {
      Term first = get(port(1, term_loc(term)));
      printf("  term: ");
      print_raw_term(first);
      printf("\n");
    }
    break;

  default:
    printf("  Location: %.3x\n", term_loc(term));
    // If this is a pair, print its contents
    if (term_loc(term) >= 0) {
      Term first = get(port(1, term_loc(term)));
      Term second = get(port(2, term_loc(term)));
      printf("  First term: ");
      print_raw_term(first);
      printf("\n");
      printf("  Second term: ");
      print_raw_term(second);
      printf("\n");
    }
    break;
  }

  printf("\n");
}

a64* get_buff(void) {
  return BUFF;
}

void spawn_threads() {
  /*
    long num_cores = 1; // sysconf(_SC_NPROCESSORS_ONLN);, tmp
    if (num_cores < 1) {
    perror("sysconf");
    exit(EXIT_FAILURE);
    }
    // */

  for (long i = 0; i < threadCount; i++) {
    pthread_create(&threads[i], NULL, normalize, (void*)i);
  }
}

// Initialize the virtual machine with a given heap size
void hvm_init(u64 size) {
  srand(time(NULL));
  
  BUFF = (a64*)calloc(size, sizeof(a64));
  if (!BUFF) {
    fprintf(stderr, "Failed to allocate memory\n");
    abort();
  }

  RBAG_BUFF = (Term*)calloc(RBAG_SIZE, sizeof(Term));
  if (!RBAG_BUFF) {
    fprintf(stderr, "Failed to allocate memory for redex stack\n");
    free(BUFF);
    BUFF = NULL;
    abort();
  }

  // Store the size of the buffer for bounds checking in pair_alloc
  BUFF_SIZE = size;

  // Initialize mutex for thread-safe redex operations
  if (pthread_mutex_init(&redex_mutex, NULL) != 0) {
    fprintf(stderr, "Failed to initialize mutex\n");
    abort();
  }

  // Initialize condition variable for redex signaling
  if (pthread_cond_init(&redex_cond, NULL) != 0) {
    fprintf(stderr, "Failed to initialize condition variable\n");
    abort();
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

  if (RBAG_BUFF != NULL) {
    free(RBAG_BUFF);
    RBAG_BUFF = NULL;
  }
}

void hvm_reset(void) {
  if (BUFF == NULL || RBAG_BUFF == NULL) {
    fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvm_init first.\n");
    abort();
  }

  // Clear memory to prevent stale data
  // memset(BUFF, 0, BUFF_SIZE * sizeof(Term));
  memset(RBAG_BUFF, 0, RBAG_SIZE * sizeof(Term));

  // Reset node index
  atomic_store_explicit(&RNOD_END, 0, memory_order_relaxed);;

  // Reset bag index
  atomic_store_explicit(&RBAG_END, 0, memory_order_relaxed);;

  // Initialize the free list (initially empty)
  FREE_LIST = EMPTY_FREE_LIST;
  atomic_store_explicit(&glblAlloced, 0, memory_order_relaxed);
  atomic_store_explicit(&waiting, 0, memory_order_relaxed);
  rdxCount = 0;
}

// For testing only
Term* get_rbag_buff(void) {
  return RBAG_BUFF;
}

// Print contents of BUFF between start and end locations
void print_buff(Location start, Location end) {
  a64* buff = get_buff();
  if (!buff) {
    printf("BUFF is not initialized\n");
    return;
  }
  if (start >= end) {
    printf("Invalid range: start=%u end=%u\n", start, end);
    return;
  }
  printf("BUFF contents from %u to %u:\n", start, end);
  for (Location i = start; i < end; i += 2) {
    Term t1 = buff[i];
    if (term_tag(t1) != NUL) {
      printf(" %.3x  ", i);
      print_raw_term(t1);
      printf("  ");
      print_raw_term(buff[i + 1]);
      printf("\n");
    }
  }
  printf("\n");
}

// Print the free list for debugging
void print_free_list(void) {
  printf("Free list: ");
  Location ptr = FREE_LIST;
  int count = 0;

  if (ptr == EMPTY_FREE_LIST) {
    printf("EMPTY ");
  } else {
    while (ptr != EMPTY_FREE_LIST && count < 100) { // Limit to prevent infinite loops
      printf("%u -> ", ptr);
      Term next = get(ptr);
      if (term_tag(next) != NUL) {
        printf("(INVALID: not NUL) ");
        break;
      }
      ptr = (Location)(next >> (TAG_SIZE + LAB_SIZE));
      count++;
    }
  }

  printf("END (count: %d)\n", count);
}

