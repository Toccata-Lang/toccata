#include <string.h>
#include "new.h"
#include "runtime3.h"
#include "stack.h"

// Global heap
#ifdef NON_ATOMIC
static u64* BUFF = NULL;
#else
static a64* BUFF = NULL;
#endif
a64 RNOD_END = 0; // Only need to track the end of the node space
static u64 BUFF_SIZE = 0; // Size of the main buffer for bounds checking

// Free list for O(1) pair allocation
__thread Location FREE_LIST = EMPTY_FREE_LIST; // Head of the free list (atomic for thread safety)

// Redex stack
__thread Pairs pairs;
Term* RBAG_BUFF = NULL; // Using Term (u64) instead of atomic (a64)
static u64 RBAG_SIZE = 0x1000;
a64 RBAG_END; // Only need to track the end of the redex stack

// interaction jump table
interactionFn interactions[16][16];

// Mutex for thread-safe redex operations
pthread_mutex_t redex_mutex;

// Condition variable for signaling when redex is available
pthread_cond_t redex_cond;

// Convert a tag to its string representation
const char* tag_to_str(Tag tag) {
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
  return tag_to_str(term_tag(t));
}

// TODO: write a time64() function that returns the time as fast as possible as a u64
u64 time64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

// abort on invalid reduction
void ABRT(Term neg, Term pos) {
  fprintf(stderr, "Bad interaction: %s %s\n",
	  tag_to_str(term_tag(neg)), tag_to_str(term_tag(pos)));
  fprintf(stderr, "a: %p b: %p\n", (void *)neg, (void *)pos);
  /*
    if (term_tag(pos) == VAL) {
    fprintf(stderr, "val type %d: %ld\n", __LINE__, ((Value *)((u64)a & ~7))->type);
    }
    // */
  fprintf(dotFile, "}\n");
  fclose(dotFile);
  abort();
}

void *boom(char *msg, char *file, int line) {
#ifndef SINGLE_THREAD
  pthread_mutex_lock(&redex_mutex);
#endif
  fprintf(stderr, "%s at %s:%d\n", msg, file, line);
  fprintf(dotFile, "}\n");
  fclose(dotFile);
  abort();
}

// Create a new term with given tag, label, and location
Term term_new(Tag tag, Lab lab, Location loc) {
  if (tag == VAL) {
    BOOM("Can't create VAL's with 'term-new'\n");
  }

  u64 loc_bits = ((u64)loc) & LOC_MASK;
  u64 lab_bits = ((u64)lab) & LAB_MASK;
  u64 tag_bits = ((u64)tag) & TAG_MASK;

  Term r = (loc_bits << (TAG_SIZE + LAB_SIZE)) |
    (lab_bits << TAG_SIZE) |
    tag_bits;

  return r;
}

// Get the tag of a term
Tag term_tag(Term term) {
  Tag t = (Tag)(term & TAG_MASK);
  if (t == VL1)
    return VAL;
  else
    return t;
}

Term term_val(Term val) {
  // ensure a Term is a valid native value
  unsigned type = ((Value *)val)->type;
  if (val & VAL_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__);
    fprintf(stderr, "val: %p\n", (void *)val);
    abort();
  }
  return val;
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
  case F60: {
    char msg[100];
    snprintf(msg, 95, "term has no location: %s", tag_to_str(term_tag(term))); 
    BOOM(msg);
  }
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

void store_redex(Term neg, Term pos) {
  /*
  fprintf(stderr, "store: ");
  print_raw_term(neg);
  fprintf(stderr, "  ");
  print_raw_term(pos);
  fprintf(stderr, "\n");
  // */
#ifdef SAFETY
  if (neg == 0 && pos == 0)
    // shutdown the threads
    neg = 0;
  else if (is_positive(neg) || is_negative(pos)) {
    BOOM("bad redex");
    // } else if (interactions[term_tag(neg)][term_tag(pos)] == &ABRT) {
    // BOOM("bad redex");
  }
#endif

  pairs.rdxs[pairs.count][0] = neg;
  pairs.rdxs[pairs.count++][1] = pos;
  if (pairs.count >= LOCAL_PAIRS_SIZE) {
    link_redexes();
  }
}

// Get term at location
Term get(Location loc) {
#ifdef NON_ATOMIC
  Term result = BUFF[loc];
#else
  Term result = atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
#endif
  return result;
}

void freeLoc(Location loc) {
#ifdef NON_ATOMIC
  BUFF[loc] = VOID;
#else
  atomic_store_explicit(&BUFF[loc], VOID, memory_order_relaxed);
#endif
  Location evenLoc = loc & 0xFFFFFFFE;
  if (get(evenLoc) == VOID && get(evenLoc + 1) == VOID) {
    pair_free(evenLoc);
  }
}

// Take the term at the given location, replacing it with VOID
// And freeing a VAR chain if needed
// Returns a positive value.
Term take(Location loc) {
  Tag takenTag;
  Term taken;
  do {
    taken = get(loc);
    takenTag = term_tag(taken);
    switch(takenTag){
    case SUB:
#ifdef SAFETY
      /*
      if (taken != SUB) {
	char msg[200];
	sprintf(msg, "should never happen! %p", (void *)taken);
	BOOM(msg);
      }
      // */
#endif
    case LAZ:
      break;

    default:
      freeLoc(loc);
      if (takenTag == VAR) {
	loc = term_loc(taken);
      }
    }
  } while (takenTag == VAR);

  switch(takenTag) {
  case SUB:
  case LAZ:
    return term_new(VAR, 0, loc);

  default:
    return taken;
  }
}

// Atomic swap operation
// If a deferred redex is found, queue it up and return SUB
// Otherwise, return a positive value.
Term swapStore(Location loc, Term term) {
#ifdef SAFETY
  if (term == VOID)
    BOOM("bad swap");
#endif
#ifdef NON_ATOMIC
  Term result = BUFF[loc];
  BUFF[loc] = term;
#else
  Term result = atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
#endif
  switch(term_tag(result)) {
  case SUB:
    if (result != SUB) {
      Term neg = get(port(1, term_loc(result)));
      Term pos = get(port(2, term_loc(result)));
      store_redex(neg, pos);
      pair_free(term_loc(result));
      result = SUB;
    }
    break;

  case ERA:
    freeLoc(loc);
    store_redex(result, term);
    break;
  }
  return result;
}

void eraseLazy(Term lazyVar) {
  Term laz;
  switch (term_tag(lazyVar)) {
  case VAR:
    laz = swapStore(term_loc(lazyVar), ERA);
    break;

  case LAZ:
    laz = lazyVar;
    break;

  default:
    BOOM("Trying to erase a non-var/lazy Term");
    break;
  }
  Location lazyLoc = term_loc(laz);
  Term negLaz = get(port(1, lazyLoc));
  Term posLaz = get(port(2, lazyLoc));
  switch(term_tag(negLaz)) {
  case DUP: {
    Term dup1 = get(port(1, term_loc(negLaz)));
    Term dup2 = get(port(2, term_loc(negLaz)));

    if (term_tag(dup1) == ERA && term_tag(dup2) == ERA) {
      take(port(1, term_loc(negLaz)));
      take(port(2, term_loc(negLaz)));
      freeLoc(port(1, lazyLoc));
      freeLoc(port(2, lazyLoc));
      interact(negLaz, NUL);
      interact(ERA, posLaz);
    } else {
      print_term("laz", laz);
      print_term("dup1", dup1);
      print_term("dup2", dup2);
      pb();
      BOOM("what to do here");
    }
  }
    break;

  case APP:
  case OPX:
    freeLoc(port(1, lazyLoc));
    freeLoc(port(2, lazyLoc));
    interact(negLaz, NUL);
    interact(ERA, posLaz);
    break;

  default:
    if (1) {
      char s[50];
      sprintf(s, "unhandled kind of lazy  %s", tag_to_str(term_tag(negLaz)));
      BOOM(s);
    }
    break;
  }
}

void forceLazy(Term z) {
  if (term_tag(z) != LAZ)
    return;

  // 'z' is a LAZ term
  Term neg = take(port(1, term_loc(z)));
  Term pos = take(port(2, term_loc(z)));
  if (term_tag(neg) == DUP) {
    // this is a lazy DUP, which ever port points to itself
    // gets replaced with SUB
    Term curr = get(port(1, term_loc(neg)));
    if (curr == z)
      swapStore(port(1, term_loc(neg)), SUB);
    curr = get(port(2, term_loc(neg)));
    if (curr == z)
      swapStore(port(2, term_loc(neg)), SUB);

    if (term_tag(pos) == VAR) {
      Term lz = swapStore(term_loc(pos), pair_make(SUB, 6, neg, pos));
      if (term_tag(lz) == LAZ)
	forceLazy(lz);
    } else
      store_redex(neg, pos);
  } else {
    store_redex(neg, pos);
  }
}

// Move a positive term into a negative location
// If anything besides a deferred redex is there, it must be a
// negative and should be reduced with 'pos'
void moveStore(Location neg_loc, Term pos) {
  Term neg = swapStore(neg_loc, pos);
  Tag negTag = term_tag(neg);

#ifdef SAFETY
  if (is_negative(pos)) {
    char s[50];
    sprintf(s,"trying to move a negative to location %.3x: %p", neg_loc, (void *)neg);
    BOOM(s);
  }
  if (is_positive(neg)) {
    char s[50];
    print_term("moved pos", pos);
    print_term("pos at neg", neg);
    sprintf(s,"found positive at move target %.3x: %p", neg_loc, (void *)neg);
    BOOM(s);
  }
#endif
  if (negTag != SUB && negTag != ERA) {
    freeLoc(neg_loc);
    store_redex(neg, pos);
  }
}

void moveDuped(Location neg_loc, Term pos) {
  Term neg = get(neg_loc);
  Tag negTag = term_tag(neg);
  switch (negTag) {
  case SUB:
    swapStore(neg_loc, pos);
    break;
    
  case APP:
  case ERA:
  case DUP:
  case OPX:
  case OPY:
    take(neg_loc);
    store_redex(neg, pos);
    break;

  default: {
    print_term("moveDuped neg", neg);
    print_term("moveDuped pos", pos);
    pb();
    char s[50];
    sprintf(s, "unhandled kind of lazy %s", tag_to_str(negTag));
    BOOM(s);
  }
    break;
  }
}

int threadCount = 1;
pthread_t threads[2050];

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
  // TODO: remove this eventually
  return result;

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
  // printf("alloc: %x\n", loc);
  return (Location)loc;
}

// Free a pair by adding it to the free list - O(1)
void pair_free(Location loc) {
#ifdef SAFETY
  atomic_fetch_add_explicit(&glblAlloced, -1, memory_order_relaxed);
#endif

  // Clear the second cell
#ifdef NON_ATOMIC
  BUFF[loc + 1] = VOID;
#else
  atomic_store_explicit(&BUFF[loc + 1], VOID, memory_order_relaxed);
#endif

  Location currTop;
  do {
    currTop = FREE_LIST;
    switch(currTop) {
    case LOCK_FREE_LIST:
      break;

    default:
      // Set up the node to point to the current head
#ifdef NON_ATOMIC
      BUFF[loc] = term_new(NUL, 0, currTop);
#else
      atomic_store_explicit(&BUFF[loc], term_new(NUL, 0, currTop), memory_order_relaxed);
#endif
      FREE_LIST = loc;
      break;
    }
  } while (currTop == LOCK_FREE_LIST);
}

// Check if a term is positive
bool is_positive(Term term) {
  switch (term_tag(term)) {
  case VAL:
  case VL1:
  case VAR:
  case NUL:
  case LAM:
  case REF:
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
Term maker(int line, Tag tag, Lab lab, Term fst, Term snd) {
#ifdef SAFETY
  // Check port polarities based on pair type
  switch (tag) {
  case SUB:
  case LAM:
  case LAZ:
    // Port 1 must be negative
    if (!is_negative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  case OPX:
  case OPY:
  case APP:
    // Port 1 must be positive
    if (!is_positive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  case DUP:
    // Port 1 must be negative
    if (!is_negative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  case SUP:
    // Port 1 must be positive
    if (!is_positive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(term_tag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  default:
    fprintf(stderr, "Error: pair_make called with invalid tag: %s (%d)\n",
	    tag_to_str(tag), tag);
    fprintf(stderr, "  Line: %d\n", line);
    abort();
  }
#endif

  // Get a pair from the free list or by extending RNOD_END
  Location loc = pair_alloc();

#ifdef SAFETY
  if (loc & 0x1)
    BOOM("Bad pair_alloc return");
#endif

  // Store terms in their respective ports
#ifdef NON_ATOMIC
  BUFF[port(1, loc)] = fst;
  BUFF[port(2, loc)] = snd;
#else
  atomic_store_explicit(&BUFF[port(1, loc)], fst, memory_order_relaxed);
  atomic_store_explicit(&BUFF[port(2, loc)], snd, memory_order_relaxed);
#endif

  Term new_pair = term_new(tag, lab, loc);
  /*
  printf("new pair at line %u: %s %.3x %p %p\n", line, tag_to_str(tag), loc,
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
  BOOM("time to fix this");
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

	  case VAR: {
	      Term deferred = pair_make(SUB, 4, neg, val);
	      Term newVal = swapStore(term_loc(val), deferred);
	      switch(term_tag(newVal)) {
	      case SUB:
		if (newVal != SUB)
		  BOOM("This shouldn't happen, should it?");
		break;

	      case LAZ:
		BOOM("Looks like this is needed");
		break;

	      default:
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

void negvar(Term neg, Term var) {
  var = take(term_loc(var));
  if (term_tag(var) == VAR) {
    Term val = swapStore(term_loc(var), neg);
    switch(term_tag(val)) {
    case SUB:
      break;

    case LAZ: {
      forceLazy(val);
    }
      break;

    default:
      BOOM("Duping a bad var");
      // this might be the way to do it.
      // but this shouldn't happen
      take(term_loc(var));
      interact(neg, val);
      break;
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

// Application-Value interaction
void appval(Term app, Term val) {
  if (((Value *)val)->type != TermType) {
    char msg[200];
    sprintf(msg, "Invalid APP VAL pair: %d %ld\n", __LINE__, ((Value *)val)->type);
    BOOM(msg);
  }
  TermVal *tv = (TermVal *)val;
  if (tv->refs == 1) {
    Term trm = swapStore(tv->trmLoc, NUL);
    if (term_tag(trm) == VAR)
      trm = take(term_loc(trm));
    store_redex(app, trm);
  } else {
    Term dup = pair_make(DUP, 0, SUB, SUB);
    Term sub = pair_make(SUB, 7, app, term_new(VAR, 0, port(1, term_loc(dup))));
    swapStore(port(1, term_loc(dup)), sub);

    Term trm = swapStore(tv->trmLoc, term_new(VAR, 0, port(2, term_loc(dup))));
    if (term_tag(trm) == VAR)
      trm = take(term_loc(trm));
    store_redex(dup, trm);
  }
  dec_and_free(val, 1);
  return;
}

Term makeLazyDup(Lab lb, Term arg) {
  Term dp = pair_make(DUP, lb, SUB, SUB);
  Location loc = term_loc(dp);
  Term lz = pair_make(LAZ, 0, dp, arg);
  swapStore(port(1, loc), lz);
  swapStore(port(2, loc), lz);
  return dp;
}

int decSubRefs(Location sup_loc) {
  return atomic_fetch_sub_explicit(&BUFF[sup_loc], 1, memory_order_relaxed);
}

// distribute a negative through a SUP
void negsup(Term neg, Term sup) {
  Location sup_loc = term_loc(sup);
  Lab sup_lab = term_lab(sup);
  Location neg_loc = term_loc(neg);
  Tag neg_tag = term_tag(neg);
  Lab neg_lab = term_lab(neg);

  Term arg = take(port(1, neg_loc));
  Location ret = port(2, neg_loc);
  Term tm1 = take(port(1, sup_loc));
  Term tm2 = take(port(2, sup_loc));
  Term dp1 = makeLazyDup(sup_lab, arg);
  Term cn1 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(1, term_loc(dp1))),
		       SUB);
  Term lz1 = pair_make(LAZ, 5, cn1, tm1);
  swapStore(port(2, term_loc(cn1)), lz1);
  Term cn2 = pair_make(neg_tag, neg_lab,
		       term_new(VAR, 0, port(2, term_loc(dp1))),
		       SUB);
  swapStore(port(2, term_loc(cn2)), pair_make(LAZ, 6, cn2, tm2));
  // TODO: could you make the ports of the SUP store direct LAZ terms
  // and not VAR's?
  Term dp2 = pair_make(SUP, sup_lab,
		       term_new(VAR, 0, port(2, term_loc(cn1))),
		       term_new(VAR, 0, port(2, term_loc(cn2))));
  moveStore(ret, dp2);
}

// Application-Null interaction
void appnul(Term app, Term nul) {
  Location app_loc = term_loc(app);
  Term pos = take(port(1, app_loc));
  interact(ERA, pos);
  moveStore(port(2, app_loc), NUL);
  return;
}

// Duplication-Lambda interaction
void duplam(Term dup, Term lam) {
  Lab dup_lab = term_lab(dup);
  Location lam_loc = term_loc(lam);
  Location var = port(1, lam_loc);
  Term bod = take(port(2, lam_loc));
  Term l1 = pair_make(LAM, 0, SUB, NUL);
  Term l2 = pair_make(LAM, 0, SUB, NUL);
  Term du1 = pair_make(SUP, dup_lab,
		       term_new(VAR, 0, port(1, term_loc(l1))),
		       term_new(VAR, 0, port(1, term_loc(l2))));
  Term du2 = makeLazyDup(dup_lab, bod);
  swapStore(port(2, term_loc(l1)), term_new(VAR, 0, port(1, term_loc(du2))));
  swapStore(port(2, term_loc(l2)), term_new(VAR, 0, port(2, term_loc(du2))));
  moveStore(var, du1);
  moveStore(port(1, term_loc(dup)), l1);
  moveStore(port(2, term_loc(dup)), l2);
  return;
}

// Duplication-Superposition interaction
void dupsup(Term dup, Term sup) {
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
    Term dup1 = makeLazyDup(dup_lab, sup_p1);;
    Term dup2 = makeLazyDup(dup_lab, sup_p2);;

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
  }
  return;
}

// Duplication interaction with copyable term
void copy(Term dup, Term trm) {
  Location dup_loc = term_loc(dup);

  // Get port locations
  Location dp1_loc = port(1, dup_loc);
  Location dp2_loc = port(2, dup_loc);

  if (term_tag(trm) == VAL)
    incRef(trm, 1);

  // put trm in both copy ports
  moveDuped(dp2_loc, trm);
  moveDuped(dp1_loc, trm);
  return;
}

// Eeraser-Var interaction
void eravar(Term era, Term var) {
  Term val = take(term_loc(var));
  if (term_tag(val) == VAR) {
    Term lz = swapStore(term_loc(val), era);
    if (lz != SUB) {
      Term lzNeg = get(port(1, term_loc(lz)));
      switch(term_tag(lzNeg)) {
      case DUP: {
	Term dp1 = get(port(1, term_loc(lzNeg)));
	Term dp2 = get(port(2, term_loc(lzNeg)));
	if (dp1 == ERA && dp2 == ERA) {
	  take(port(1, term_loc(lz)));
	  take(port(1, term_loc(lzNeg)));
	  take(port(2, term_loc(lzNeg)));
	  Term lzPos = take(port(2, term_loc(lz)));
	  interact(ERA, lzPos);
	} else if (dp1 == sideEffects || dp2 == sideEffects)
	  forceLazy(lz);
      }
	break;

      case APP:
      case OPX:
	interact(era, lz);
	break;

      default: {
	char s[150];
	sprintf(s, "unhandled freeing lazy: %s", tag_to_str(term_tag(lzNeg)));
	BOOM(s);
	freeLoc(term_loc(val));
      }
	break;
      }
    }
  } else {
    interact(era, val);
  }
  return;
}

// Eraser-Lambda interaction
void eralam(Term era, Term lam) {
  Location lam_loc = term_loc(lam);
  store_redex(era, take(port(2, lam_loc)));
  moveStore(port(1, lam_loc), NUL);
  return;
}

void eralaz(Term era, Term laz) {
  if (term_lab(era) > 0) {
    forceLazy(laz);
  } else
    eraseLazy(laz);
}

// Eraser-Superposition interaction
void erasup(Term era, Term sup) {
  Location sup_loc = term_loc(sup);
  store_redex(era, term_new(VAR, 0, port(2, sup_loc)));
  store_redex(era, term_new(VAR, 0, port(1, sup_loc)));
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
  Location op_loc = term_loc(op);
  if (term_lab(nul) == 0) {
    interact(take(port(2, op_loc)), NUL);
    interact(ERA, take(port(1, op_loc)));
  } else {
    BOOM("test this");
    moveStore(port(2, op_loc), nul);
    store_redex(sideEffects, take(port(1, op_loc)));
  }
  return;
}

// SUB-NUL interaction
void subnul(Term sub, Term nul) {
  // Check if the SUB term has a location (label > 0)
  if (sub != SUB) {
    // The SUB term has a location pointing to a pair
    Location sub_loc = term_loc(sub);

    // Take the first port and link it with NUL
    Term t = take(port(1, sub_loc));
    interact(t, NUL);

    // Take the second port and link it with ERA
    t = take(port(2, sub_loc));
    interact(ERA, t);
  }

  return;
}

void dupnul(Term dup, Term nul) {
  Location dp1 = port(1, term_loc(dup));
  Location dp2 = port(2, term_loc(dup));
  swapStore(dp1, nul);
  swapStore(dp2, nul);
}

void duplaz(Term dup, Term laz) {
  Term dup1 = get(port(1, term_loc(dup)));
  Term dup2 = get(port(2, term_loc(dup)));
  BOOM("this call to term_new is wrong");
  Term lzVar = term_new(VAR, 0, laz);

  // TODO: remove when not needed
  char s[150];

  if (term_tag(dup1) == ERA && term_tag(dup2) == ERA) {
    sprintf(s, "test duplaz line: %p %p", (void *)dup1, (void *)dup2);
    BOOM(s);
    // the lazy value is no longer needed
    take(port(1, term_loc(dup)));
    take(port(2, term_loc(dup)));
    eraseLazy(lzVar);
  } else if (term_tag(dup1) == ERA) {
    sprintf(s, "test duplaz line: %p", (void *)dup1);
    BOOM(s);
    // the dupe is no longer needed on one branch
    take(port(1, term_loc(dup)));
    moveStore(port(2, term_loc(dup)), lzVar);
  } else if (term_tag(dup2) == ERA) {
    sprintf(s, "test duplaz line: %p", (void *)dup2);
    BOOM(s);
    // the dupe is no longer needed on the other branch
    take(port(2, term_loc(dup)));
    moveStore(port(1, term_loc(dup)), lzVar);
  } else {
    sprintf(s, "test duplaz line: %d", __LINE__);
    BOOM(s);
    Term newZ = pair_make(LAZ, 3, dup, lzVar);
    dup1 = swapStore(port(1, term_loc(dup)), newZ);
    switch (term_tag(dup1)) {
    case DUP:
      sprintf(s, "test duplaz line: %d", __LINE__);
      BOOM(s);
      store_redex(dup1, term_new(VAR, 0, port(1, term_loc(dup))));
      break;

    case SUB:
      sprintf(s, "test duplaz line: %d", __LINE__);
      BOOM(s);
      if (dup1 != SUB) {
	Location sub_loc = term_loc(dup1);
	Term subDup = take(port(1, sub_loc));
	Term subPos = take(port(2, sub_loc));
	store_redex(subDup, subPos);
      }
      break;

    default: {
      char s[150];
      sprintf(s, "unhandled kind of duped %s", tag_to_str(term_tag(dup1)));
      BOOM(s);
    }
      break;
    }

    dup2 = swapStore(port(2, term_loc(dup)), newZ);
    switch (term_tag(dup2)) {
    case DUP:
      sprintf(s, "test duplaz line: %d", __LINE__);
      BOOM(s);
      store_redex(dup2, term_new(VAR, 0, port(2, term_loc(dup))));
      break;

    case SUB:
      sprintf(s, "test duplaz line: %d", __LINE__);
      BOOM(s);
      if (dup2 != SUB) {
	Location sub_loc = term_loc(dup2);
	Term subDup = take(port(1, sub_loc));
	Term subPos = take(port(2, sub_loc));
	store_redex(subDup, subPos);
      }
      break;

    default: {
      char s[250];
      sprintf(s, "unhandled kind of duped %s", tag_to_str(term_tag(dup2)));
      BOOM(s);
    }
      break;
    }
  }
}

void YNUM(Term opy, Term num);
void XNUM(Term opx, Term num) {
  Location opx_loc = term_loc(opx);
  Term arg = swapStore(port(1, opx_loc), num);
  Lab op = term_lab(opx);
  switch (term_tag(arg)) {
  case I60:
    YNUM(term_new(OPY, op, port(1, opx_loc)), arg);
    break;

  case VAR:
    interact(term_new(OPY, op, port(1, opx_loc)), arg);
    break;

  default:
    store_redex(term_new(OPY, op, port(1, opx_loc)), arg);
  }
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
  NativeArgs arityArgs = {0, {}};
  opy = strictArgs(num, opy, 1, &arityArgs);
  if (arityArgs.count != 1) {
    return;
  }

  Location op_loc = term_loc(opy);
  Term x = arityArgs.args[0];
  Tag y_type = term_tag(num);
  Location ret = port(2, op_loc);
  u64 res;
  Lab op = term_lab(opy);

#ifdef SAFETY
  switch (term_tag(x)) {
  case I60:
  case F60:
    break;

  default: {
    char msg[200];
    sprintf(msg, "wrong value to OPY: %s", tag_to_str(term_tag(x))); 
    BOOM(msg);
  }
    break;
  }
#endif

  switch (y_type) {
  case I60: PERFORM_OP(get_u64(x), get_u64(num), op, i64); break;
    // case F60: PERFORM_OP(x, y, op, f64); break;
  }

  moveStore(ret, new_num(y_type, res));
  return;
}

void DECR(Term neg, Term pos) {
  dec_and_free(pos, 1);
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
  &ABRT,&negvar,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&negsup,&ABRT,&ABRT,&ABRT,&XNUM,&XNUM,&ABRT
// VAL    VAR    SUB    NUL   ERA   LAM   APP   REF   VL1    SUP    DUP   OPX   OPY   I60   F60   LAZ

#define OPY_INTERACTIONS\
  &ABRT,&negvar,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&negsup,&ABRT,&ABRT,&ABRT,&YNUM,&YNUM,&ABRT
// VAL    VAR    SUB    NUL   ERA   LAM   APP   REF   VL1    SUP    DUP   OPX   OPY   I60   F60   LAZ

#define ERA_INTERACTIONS\
  &DECR,&eravar,&ABRT,&NOP,&ABRT,&eralam,&ABRT,&NOP,&ABRT,&erasup,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&eralaz
// VAL    VAR    SUB   NUL   ERA   LAM    APP   REF  VL1    SUP    DUP   OPX   OPY   I60  F60  LAZ

#define APP_INTERACTIONS\
  &appval,&negvar,&ABRT,&appnul,&ABRT,&applam,&ABRT,&appref,&appval,&negsup,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT
//   VAL     VAR   SUB     NUL    ERA    LAM    APP    REF     VL1    SUP    DUP   OPX   OPY   I60   F60   LAZ

#define DUP_INTERACTIONS\
  &copy,&negvar,&ABRT,&dupnul,&ABRT,&duplam,&ABRT,&copy,&copy,&dupsup,&ABRT,&ABRT,&ABRT,&copy,&copy,&ABRT
//  VAL   VAR    SUB    NUL    ERA    LAM    APP   REF   VL1    SUP    DUP   OPX   OPY   I60   F60   LAZ

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

a64 rdxCount;
void interact(Term neg, Term pos) {
  // print_raw_term(neg);
  // printf("  ");
  // print_raw_term(pos);
  // printf("\n");
#ifdef STATS
  atomic_fetch_add_explicit(&rdxCount, 1, memory_order_relaxed);
#endif
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
  /*
  u64 waitingThreads = atomic_load_explicit(&waiting, memory_order_relaxed);
  if (waitingThreads > 0) {
    pthread_mutex_lock(&redex_mutex);
    pthread_cond_signal(&redex_cond);
    pthread_mutex_unlock(&redex_mutex);
  }
  // */

  u64 *res = malloc(sizeof(u64));
  // *res = rdxCount;
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
  }

  return tail;
}

// extract the requested number of native args. I60, F60, REF or VAL terms
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct) {
  /*
  char *refName = NULL;
  for (unsigned i = 0; i <= refsCount; i++) {
    if (refNames[i].fn == (interactionFn)(ref & ~TAG_MASK)) {
      refName = refNames[i].name;
      break;
    }
  }
  if (expected == 1) {
    if (refName != NULL) {
      char msg[200];
      sprintf(msg, "%s %03x:", refName, term_loc(args));
      graphDown(msg, args);
    } else {
      graphDown("unknown", args);
    }
    // if (strcmp(refName, "str-eq") == 0) {
    // print_term("str-eq args", args);
    // }
  }
  // */
  Tag argsTag = term_tag(args);
  if (argsTag == APP || argsTag == OPY) {
    // if 'args' is an APP term
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

    case LAM: {
      TermVal *tv = malloc_term();
      tv->trmLoc = pair_alloc();
      swapStore(tv->trmLoc, arg);

      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = (Term)tv;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
      else
	return args;
    }
      break;

    case NUL:
      moveStore(port(2, term_loc(args)), NUL);
      for (int i = 0; i < argsStruct->count; i++)
	dec_and_free(argsStruct->args[i], 1);
      break;

    case SUP:
      for(int i = 0; i < argsStruct->count; i++)
	incRef(argsStruct->args[i], 1);
      Term s1 = take(port(1, term_loc(arg)));
      Term s2 = take(port(2, term_loc(arg)));
      Lab supLabel = term_lab(arg);
      int argsCount = argsStruct->count;
      argsStruct->count += 1;

      Term tail1 = pair_make(APP, 0, s1, SUB);
      argsStruct->args[argsCount] = tail1;
      swapStore(port(2, term_loc(tail1)), pair_make(LAZ, 7, argsNet(argsStruct), ref));

      Term tail2 = pair_make(APP, 0, s2, SUB);
      argsStruct->args[argsCount] = tail2;
      swapStore(port(2, term_loc(tail2)), pair_make(LAZ, 7, argsNet(argsStruct), ref));

      Term newSup = pair_make(SUP, supLabel,
			      term_new(VAR, 0, port(2, term_loc(tail1))),
			      term_new(VAR, 0, port(2, term_loc(tail2))));
      moveStore(port(2, term_loc(args)), newSup);
      break;

    case VAR: {
      Term val = get(term_loc(arg));
      switch(term_tag(val)) {
      case LAZ:
	swapStore(term_loc(arg), SUB);
	forceLazy(val);
	// TODO:
	// BOOM("don't fall through");
	
      case SUB:
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
	break;

      default:
	print_raw_term(val);
	printf("\n");
	BOOM("nativeArgs");
	break;
      }
    }
      break;

    case LAZ:
    default:
      fprintf(stderr, "unhandled tag %s (0x%x) line: %d\n", tag_to_str(term_tag(arg)),
	     term_tag(arg), __LINE__);
      fprintf(dotFile, "}\n");
      fclose(dotFile);
      abort();
      break;
    }
    argsStruct->count = -1;
    return 0;
    // } else if (argsTag == VAR) {
    // if 'args' is a VAR term
  } else {
    fprintf(stderr, "strictArgs expected: %d\n", expected);
    print_term("strictArgs args", args);
    fprintf(stderr, "unhandled tag %s (0x%x) %p line: %d\n",
	   tag_to_str(argsTag), argsTag, (void *)args, __LINE__);
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    abort();
    return 0;
  }
}

// For testing only
void print_raw_term(Term t) {
  if (t == 0) {
    fprintf(stderr, "  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    switch(term_tag(t)) {
    case NUL:
    case ERA:
    case I60:
    case F60:
      fprintf(stderr, "%s %x", tag_to_str(tag), lab);
      break;

    case VAL:
      fprintf(stderr, "%s %llx", tag_to_str(tag), t & ~VAL_MASK);
      break;

    case REF: {
      char *refName = "";
      for (unsigned i = 0; i <= refsCount; i++) {
	if (refNames[i].fn == (interactionFn)(t & ~TAG_MASK)) {
	  refName = refNames[i].name;
	  break;
	}
      }
      fprintf(stderr, "%s %llx %s", tag_to_str(tag), t & ~TAG_MASK, refName);
    }
      break;

      // case REF:
      // printf("REF %s", refName(t));
      // break;

    default:
      fprintf(stderr, "%s %x %.3x", tag_to_str(tag), lab, term_loc(t));
      break;
    }
  }
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  fprintf(stderr, "%s:\n", prefix);
  fprintf(stderr, "  Tag: %s (%d, 0x%x)\n", tag_to_str(term_tag(term)), term_tag(term), term_tag(term));
  Lab lab = term_lab(term);
  switch(term_tag(term)) {
  case VAL:
  case NUL:
  case ERA:
  case F60:
    break;

  case REF:
      fprintf(stderr, "  Fn: %llx\n", term & ~TAG_MASK);
      break;
      
  case I60:
    fprintf(stderr, "  Val: %ld", get_i60(term));
    break;

  case VAR:
    fprintf(stderr, "  Location: %.3x\n", term_loc(term));
    // If this is a pair, print its contents
    if (term_loc(term) >= 0) {
      Term first = get(port(1, term_loc(term)));
      fprintf(stderr, "  term: ");
      print_raw_term(first);
      fprintf(stderr, "\n");
    }
    break;

  case SUP:
    if (lab == 1 || lab == 2) {
      fprintf(stderr, "  Location: %.3x\n", term_loc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
	Term first = get(port(1, term_loc(term)));
	Term second = get(port(2, term_loc(term)));
	fprintf(stderr, "  Refs: %d\n", (int)first);
	fprintf(stderr, "  Second term: ");
	print_raw_term(second);
	fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "  Location: %.3x\n", term_loc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
      // If this is a pair, print its contents
      if (term_loc(term) >= 0) {
	Term first = get(port(1, term_loc(term)));
	Term second = get(port(2, term_loc(term)));
	fprintf(stderr, "  First term: ");
	print_raw_term(first);
	fprintf(stderr, "\n");
	fprintf(stderr, "  Second term: ");
	print_raw_term(second);
	fprintf(stderr, "\n");
      }
    }
    break;

  default:
    fprintf(stderr, "  Location: %.3x\n", term_loc(term));
    fprintf(stderr, "  Label: %.3x\n", lab);
    // If this is a pair, print its contents
    if (term_loc(term) >= 0) {
      Term first = get(port(1, term_loc(term)));
      Term second = get(port(2, term_loc(term)));
      fprintf(stderr, "  First term: ");
      print_raw_term(first);
      fprintf(stderr, "\n");
      fprintf(stderr, "  Second term: ");
      print_raw_term(second);
      fprintf(stderr, "\n");
    }
    break;
  }

  fprintf(stderr, "\n");
}

#ifdef NON_ATOMIC
u64* get_buff(void) {
  return BUFF;
}
#else
a64* get_buff(void) {
  return BUFF;
}
#endif

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

#ifdef NON_ATOMIC
  BUFF = (u64*)calloc(size, sizeof(a64));
#else
  BUFF = (a64*)calloc(size, sizeof(a64));
#endif
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
  atomic_store_explicit(&rdxCount, 0, memory_order_relaxed);
  atomic_store_explicit(&waiting, 0, memory_order_relaxed);
  rdxCount = 0;
}

// For testing only
Term* get_rbag_buff(void) {
  return RBAG_BUFF;
}

// Print contents of BUFF between start and end locations
void print_buff(Location start, Location end) {
#ifdef NON_ATOMIC
  u64* buff = get_buff();
#else
  a64* buff = get_buff();
#endif
  if (!buff) {
    fprintf(stderr, "BUFF is not initialized\n");
    return;
  }
  if (start >= end) {
    fprintf(stderr, "Invalid range: start=%u end=%u\n", start, end);
    return;
  }
  fprintf(stderr, "BUFF contents from %u to %u:\n", start, end);
  for (Location i = start; i < end; i += 2) {
    Term t1 = buff[i];
    if (term_tag(t1) != NUL || buff[i + 1] != 0) {
      fprintf(stderr," %.3x  ", i);
      print_raw_term(t1);
      fprintf(stderr,"  ");
      print_raw_term(buff[i + 1]);
      fprintf(stderr,"\n");
    }
  }
  fprintf(stderr, "\n");
}

void pb() {
  print_buff(0, RNOD_END);
}

void pr() {
  for (int i = 0; i < pairs.count; i++) {
    printf(" %.3x  ", i);
    print_raw_term(pairs.rdxs[i][0]);
    printf("  ");
    print_raw_term(pairs.rdxs[i][1]);
    printf("\n");
  }
}

void check_buff() {
#ifdef NON_ATOMIC
  u64* buff = get_buff();
#else
  a64* buff = get_buff();
#endif
  if (!buff) {
    printf("BUFF is not initialized\n");
    return;
  }
  for (Location i = 0; i < RNOD_END; i += 2) {
    Term t1 = buff[i];
    if (term_tag(t1) != NUL || buff[i + 1] != 0) {
      pb();
      BOOM("Leak pairs");
    }
  }
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

Term dupeArg(Term arg, Term *dupedArg, unsigned dupLabel) {
  switch(term_tag(arg)) {
  case VAL:
    *dupedArg = incRef(arg, 1);
    return arg;
    break;

  case F60:
  case I60:
  case REF:
    *dupedArg = arg;
    return arg;
    break;

  default: {
    Term newDup = pair_make(DUP, dupLabel, SUB, SUB);
    Term z = pair_make(LAZ, 1, newDup, arg);
    swapStore(port(1, term_loc(newDup)), z);
    swapStore(port(2, term_loc(newDup)), z);

    *dupedArg = term_new(VAR, 0, port(2, term_loc(newDup)));
    return term_new(VAR, 0, port(1, term_loc(newDup)));
  }
    break;
  }
}

Term make_op(Lab op, Term x, Term y) {
  Term t = pair_make(OPX, op, y, SUB);
  Term ret = term_new(VAR, 0, port(2, term_loc(t)));
  swapStore(term_loc(ret), pair_make(LAZ, 2, t, x));
  return ret;
}

char *nodeFormat =  "x%x [label=\"%d\",  height=0.4, width=0.4, fixedsize=true,  shape=triangle, orientation=%d]\n";
char *noOutline =  "x%d_%x [label=\"%s\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n";
char *nodeXlblFormat =  "x%d_%x [label=\"%s\",  height=0.4, width=0.4, fixedsize=true,  shape=triangle, orientation=%d, xlabel=\"%s\"]\n";
char *eraseFormat =  "x%d_%x [label=\"\", height=0.1, width=0.1, color=black, fixedsize=true,  shape=circle, style=filled]\n";

char *nodeLabels[25] = {"V", " ", " ", " ", " ", "L", "A", "F", "V", "S", "D",
                        " ", " ", "#", "#", "Z"};

typedef struct graphNode {
  Term trm;
  Location node;
  struct graphNode *left;
  struct graphNode *right;
  Location lazyAPP;
} graphNode;

#define NODE_STACK_SIZE 1000
graphNode nodeStack[NODE_STACK_SIZE];

void fatal_error(char *fmt, unsigned bytes) {
  fprintf(stderr, fmt, bytes);
  abort();
}

char hasLocation(Term tree) {
  Tag t = term_tag(tree);

  if (tree == SUB)
    return 0;
  
  switch(t) {
  case VAL:
  case NUL:
  case REF:
  case ERA:
  case I60:
  case F60:
    return 0;
    break;

    // Allow SUB terms to have locations
  case SUB:
  default:
    return 1;
    break;
  }
}

unsigned nodeCount = 0;
unsigned otherNodes;
unsigned subGraphs = 0;

unsigned graphSubUp(unsigned graphNum, Term tree);
unsigned upBranch(Term tree, unsigned pt, unsigned graphNum) {
  Tag t = term_tag(tree);
  unsigned nodeNum;
  char *branchPort = pt == 1 ? "nw" : "ne";
  Location loc = port(pt, term_loc(tree));
  Term branch = get(loc);
  unsigned branchNode = 65536;
  for (unsigned i = 0; i < nodeCount; i++) {
    graphNode *gn = &nodeStack[i];
    if (gn->trm == branch) {
      branchNode = gn->node;
      break;
    }
  }

  Tag bt = term_tag(branch);
  return graphSubUp(graphNum, branch);
}

void graphLink( unsigned graphNum, unsigned nodeNum, unsigned pt, Term branch, unsigned branchNode) {
  char *branchPort = pt == 1 ? "nw" : "ne";

  if (term_tag(branch) == VAR &&
      ((term_tag(get(term_loc(branch))) == LAZ &&
	term_tag(get(port(1, term_loc(get(term_loc(branch)))))) == DUP) ||
       term_tag(get(term_loc(branch))) == SUB)) {
    if (term_loc(branch) & 1)
      fprintf(dotFile, "x%d_%x:ne -- x%d_%x:%s\n",
	      graphNum, (term_loc(branch) & 0xFFFFFFFE), graphNum, nodeNum, branchPort);
    else
      fprintf(dotFile, "x%d_%x:nw -- x%d_%x:%s\n",
	      graphNum, (term_loc(branch) & 0xFFFFFFFE), graphNum, nodeNum, branchPort);
  } else {
    fprintf(dotFile, "x%d_%x:s -- x%d_%x:%s\n",
	    graphNum, branchNode, graphNum, nodeNum, branchPort);
  }
}

unsigned graphSubUp(unsigned graphNum, Term tree) {
  char xLbl[100];
  unsigned nodeNum = 65536;
  unsigned rightBranch = 65536;
  unsigned leftBranch = 65536;
  Tag leftTag = NUL;

  if (tree == SUB) {
    return 65536;
  } else if (hasLocation(tree)) {
    nodeNum = term_loc(tree) & 0xFFFFFFFE;
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      if (gn->node == nodeNum)
	return gn->node;
    }
  } else {
    nodeNum = otherNodes++;
  }

  Tag t = term_tag(tree);
  switch(t) {
  case VAR: {
    //*
    Location loc = term_loc(tree);
    Term trm = get(loc);
    if (trm != LAZ || term_tag(get(port(1, term_loc(trm)))) != DUP) {
      return graphSubUp(graphNum, trm);
    } else {
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	if (hasLocation(gn->trm) && term_loc(gn->trm) == (loc & 0xFFFFFFFE))
	  return gn->node;
      }
    }
    // */
    return 65536;
  }
    break;

  case ERA:
    if (term_lab(tree) == 1)
      fprintf(dotFile, noOutline, graphNum, nodeNum, "SE");
    else
      fprintf(dotFile, eraseFormat, graphNum, nodeNum);
    break;

  case I60:
    snprintf(xLbl, 95, "%ld", get_i60(tree));
    fprintf(dotFile, noOutline, graphNum, nodeNum, xLbl);
    break;

  case VAL:
    fprintf(dotFile, noOutline, graphNum, nodeNum, nodeLabels[t]);
    break;

  case REF: {
    char *refName = "F";
    for (unsigned i = 0; i <= refsCount; i++) {
      if (refNames[i].fn == (interactionFn)(tree & ~7)) {
	refName = refNames[i].name;
	break;
      }
    }
    fprintf(dotFile,   "x%d_%x [label=\"%s\", shape=plaintext]\n",
	    graphNum, nodeNum, refName);
  }
    break;

  case SUB: {
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    leftTag = term_tag(get(port(1, term_loc(tree))));
    switch(leftTag) {
    case REF:
    case VAL:
    case ERA:
    case I60:
    case SUB:
    case VAR:
      rightBranch = upBranch(tree, 2, graphNum);
      leftBranch = upBranch(tree, 1, graphNum);
      break;

    default:
      leftBranch = upBranch(tree, 1, graphNum);
      rightBranch = upBranch(tree, 2, graphNum);
      break;
    }

    fprintf(dotFile, "x%d_%x [label=\"%s\",  height=0.22, width=0.22, fixedsize=true,  shape=circle]\n",
	    graphNum, nodeNum, "");
  }
    break;

  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP:
  case DUP: {
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    leftTag = term_tag(get(port(1, term_loc(tree))));
    switch(leftTag) {
    case REF:
    case VAL:
    case ERA:
    case I60:
    case SUB:
    case VAR:
      rightBranch = upBranch(tree, 2, graphNum);
      leftBranch = upBranch(tree, 1, graphNum);
      break;

    default:
      leftBranch = upBranch(tree, 1, graphNum);
      rightBranch = upBranch(tree, 2, graphNum);
      break;
    }

    snprintf(xLbl, 95, "%x:", nodeNum);
    if (t == OPX || t == OPY) {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "-", 180, xLbl);
    } else {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, nodeLabels[t], 180, xLbl);
    }
  }
    break;
    
  default:
    return 65536;
    break;
  }

  if (leftBranch != 65536) {
    Term branch = get(port(1, term_loc(tree)));
    Tag bt = term_tag(branch);
    if (t == LAZ && bt == DUP) {
      fprintf(dotFile, "x%d_%x:s -- x%d_%x:nw\n", graphNum, leftBranch, graphNum, nodeNum);
    } else if (t != DUP) {
      graphLink(graphNum, nodeNum, 1, branch, leftBranch);
    }
  }

  if (rightBranch != 65536) {
    Term branch = get(port(2, term_loc(tree)));
    Tag bt = term_tag(branch);
    if ((t == APP || t == OPX || t == OPY) && bt == LAZ) {
      // TODO: figure out a better solution later
      // fprintf(dotFile, "x%d_%x:ne -- x%d_%x:n\n", graphNum, nodeNum, graphNum, rightBranch);
    } else if (t != DUP) {
      graphLink(graphNum, nodeNum, 2, branch, rightBranch);
    }
  }
  return nodeNum;
}

void graphUp(char *title, Term root) {
  nodeCount = 0;
  unsigned graphNum = subGraphs++;

  otherNodes = RNOD_END;
  fprintf(dotFile, "subgraph cluster%d {\ngraph [color=none, label=\"%s\"]\n", graphNum, title);
  graphSubUp(graphNum, root);
  fprintf(dotFile, "}\n");
  return;
}

unsigned graphSubDown(unsigned graphNum, unsigned nodeNum, Term tree);
void downBranch(Term tree, unsigned pt, unsigned graphNum, unsigned nodeNum) {
  Tag t = term_tag(tree);
  char *branchPort = pt == 1 ? "sw" : "se";
  Location loc = port(pt, term_loc(tree));
  Term branch = get(loc);
  while (term_tag(branch) == VAR) {
    branch = get(term_loc(branch));
  }
  unsigned branchNode = 65536;
  for (unsigned i = 0; i < nodeCount; i++) {
    graphNode *gn = &nodeStack[i];
    if (gn->trm == branch) {
      branchNode = gn->node;
      break;
    }
  }

  Tag bt = term_tag(branch);
  Lab l = term_lab(tree);
  branchNode = graphSubDown(graphNum, loc, branch);

  if (pt == 2 && (t== APP || t == OPX || t == OPY) && bt == LAZ) {
    fprintf(dotFile, "x%d_%x:s -- x%d_%x:se\n", graphNum, branchNode, graphNum, nodeNum);
  } else {
    if (branchNode != 65536) {
      if (t == DUP && (bt != LAZ || get(port(1, term_loc(branch))) != tree)) {
	fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		graphNum, nodeNum, branchPort, graphNum, branchNode);
      } else if (t == DUP && (bt != LAZ || get(port(1, term_loc(branch))) == tree)) {
	return;
      } else {
	if (bt == VAR) {
	  if (term_tag(get(term_loc(branch))) == SUB || term_tag(get(term_loc(branch))) == LAZ) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (term_loc(branch) & 0xFFFFFFFE),
		    (term_loc(branch) & 1) ? "se" : "sw");
	  } else if (term_tag(get(term_loc(branch))) == LAZ &&
		     term_tag(get(port(1, term_loc(get(term_loc(branch)))))) == DUP) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (term_loc(branch) & 0xFFFFFFFE),
		    (term_loc(branch) & 1) ? "se" : "sw");
	  } else {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		    graphNum, nodeNum, branchPort, graphNum, branchNode);
	  }
	} else {
	  fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		  graphNum, nodeNum, branchPort, graphNum, branchNode);
	}
      }
    }
  }
}

unsigned graphSubDown(unsigned graphNum, unsigned nodeNum, Term tree) {
  char xLbl[100];
  Tag t = term_tag(tree);

  if (tree == SUB) {
    return 65536;
  } else if (hasLocation(tree)) {
    nodeNum = term_loc(tree) & 0xFFFFFFFE;
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      if (gn->node == nodeNum)
	return gn->node;
    }
  } else {
    nodeNum = otherNodes++;
  }

  switch(t) {
  case VAR: {
    Location loc = term_loc(tree);
    Term trm = get(loc);
    if (trm != LAZ || term_tag(get(port(1, term_loc(trm)))) != DUP) {
      return graphSubDown(graphNum, nodeNum, trm);
    } else {
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	if (hasLocation(gn->trm) && term_loc(gn->trm) == (loc & 0xFFFFFFFE))
	  return gn->node;
      }
    }
    return 65536;
  }
    break;

  case ERA:
    if (term_lab(tree) == 1)
      fprintf(dotFile, noOutline, graphNum, nodeNum, "SE");
    else
      fprintf(dotFile, eraseFormat, graphNum, nodeNum);
    break;

  case I60:
    snprintf(xLbl, 95, "%ld", get_i60(tree));
    fprintf(dotFile, noOutline, graphNum, nodeNum, xLbl);
    break;

  case VAL:
    if (tree == VOID)
      fprintf(dotFile, noOutline, graphNum, nodeNum, "VOID");
    else
      fprintf(dotFile, noOutline, graphNum, nodeNum, nodeLabels[t]);
    break;

  case REF: {
    char *refName = "F";
    for (unsigned i = 0; i <= refsCount; i++) {
      if (refNames[i].fn == (interactionFn)(tree & ~7)) {
	refName = refNames[i].name;
	break;
      }
    }
    fprintf(dotFile,   "x%d_%x [label=\"%s\", shape=plaintext]\n",
	    graphNum, nodeNum, refName);
  }
    break;

  case SUB: {
    fprintf(dotFile, "x%d_%x [label=\"%s\",  height=0.22, width=0.22, fixedsize=true,  shape=circle]\n",
	    graphNum, nodeNum, "");
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    downBranch(tree, 1, graphNum, nodeNum);
    downBranch(tree, 2, graphNum, nodeNum);
  }
    break;

  case SUP:
  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP:
  case DUP: {
    Lab lab = term_lab(tree);
    snprintf(xLbl, 95, "%x:", nodeNum);
    if (t == OPX || t == OPY) {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "-", 0, xLbl);
    } else {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, nodeLabels[t], 0, xLbl);
    }
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    if (t == DUP) {
      Location dLoc = term_loc(tree);
      Term b1 = get(port(1, dLoc));
      Tag bt1 = term_tag(b1);
      if (bt1 == LAZ && tree != get(port(1, term_loc(b1)))) {
	downBranch(tree, 1, graphNum, nodeNum);
      }

      Term b2 = get(port(2, dLoc));
      Tag bt2 = term_tag(b2);
      if (bt2 == LAZ && tree != get(port(2, term_loc(b2)))) {
	downBranch(tree, 2, graphNum, nodeNum);
      }
    } else {
      downBranch(tree, 1, graphNum, nodeNum);
      downBranch(tree, 2, graphNum, nodeNum);
    }
  }
    break;
    
  default:
    return 65536;
    break;
  }
  return nodeNum;
}

unsigned graphDown(char *title, Term root) {
  char xLbl[100];
  nodeCount = 0;
  unsigned graphNum = subGraphs++;

  otherNodes = RNOD_END;
  fprintf(dotFile, "subgraph cluster%d {\ngraph [color=none, label=\"%s\"]\n", graphNum, title);

  if (term_tag(root) == LAM) {
    unsigned nodeNum = otherNodes++;
    unsigned rootNode = term_loc(root);
    snprintf(xLbl, 95, "%x:", rootNode);
    fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "L", 0, xLbl);
    // fprintf(dotFile, "{rank=min; x%d_%x;}\n", graphNum, nodeNum);

    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = root;
    gn->node = rootNode;

    Term left = get(port(1, rootNode));
    if (term_tag(left) != SUB) {
      unsigned leftNode = graphSubDown(graphNum, 65536, left);
      fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, leftNode);
    } else {
      fprintf(dotFile, "x%d_%x [label=\"\", shape=plaintext, height=0, width=0, peripheries=0]\n",
	      graphNum, rootNode);
      fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, rootNode);
      fprintf(dotFile, "{rank=max; x%d_%x;}\n", graphNum, rootNode);
    }

    unsigned rightNode = graphSubDown(graphNum, 65536, get(port(2, term_loc(root))));
    if (rightNode != 65536)
      fprintf(dotFile, "x%d_%x:se -- x%d_%x:n\n", graphNum, nodeNum, graphNum, rightNode);
    fprintf(dotFile, "}\n");
    return nodeNum;
  } else {
    unsigned rootNode = graphSubDown(graphNum, 65536, root);
    fprintf(dotFile, "}\n");
    return rootNode;
  }
}
