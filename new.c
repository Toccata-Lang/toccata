#include "new.h"
#include <string.h>

// Global heap
static a64* BUFF = NULL;
static Term* RBAG_BUFF = NULL; // Using Term (u64) instead of atomic (a64)
u64 RNOD_END = 0; // Only need to track the end of the node space
static u64 RBAG_SIZE = 0x1000;
u64 RBAG_END = 0; // Only need to track the end of the redex stack
static u64 BUFF_SIZE = 0; // Size of the main buffer for bounds checking

// Free list for O(1) pair allocation
_Atomic Location FREE_LIST = EMPTY_FREE_LIST; // Head of the free list (atomic for thread safety)

// Mutex for thread-safe redex operations
pthread_mutex_t redex_mutex;

// Condition variable for signaling when redex is available
pthread_cond_t redex_cond;

// For testing only
a64* get_buff(void) {
  return BUFF;
}

// For testing only
Term* get_rbag_buff(void) {
  return RBAG_BUFF;
}

void print_raw_term(Term t) {
  if (t == 0) {
    printf("  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    switch(term_tag(t)) {
    case VAL:
    case NUL:
    case REF:
    case ERA:
    case I60:
    case F60:
      printf("%s %x", tag_to_string(tag), lab);
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
  case SUB:
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
    printf(" %.3x  ", i);
    print_raw_term(buff[i]);
    printf("  ");
    print_raw_term(buff[i + 1]);
    printf("\n");
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

void *boom(char *msg, char *file, int line) {
  fprintf(stderr, "%s at %s:%d\n", msg, file, line);
  abort();
}

int threadCount = 1;

void spawn_threads() {
  /*
    long num_cores = 1; // sysconf(_SC_NPROCESSORS_ONLN);, tmp
    if (num_cores < 1) {
    perror("sysconf");
    exit(EXIT_FAILURE);
    }
    // */

  pthread_t threads[threadCount];
  for (long i = 0; i < threadCount; i++) {
    pthread_create(&threads[i], NULL, normalize, (void*)i);
  }
}

// Initialize the virtual machine with a given heap size
void hvm_init(u64 size) {
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

  RNOD_END = 0;
  RBAG_END = 0;
  FREE_LIST = EMPTY_FREE_LIST;  // Initially no free pairs

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

// Reset node and bag indices
// Initialize the free list by linking all available pairs
void init_free_list(u64 start, u64 end) {
  start = (start + 1) & 0xFFFFFFFe;
  end = end & 0xFFFFFFFe;

  // Clear the list initially
  FREE_LIST = EMPTY_FREE_LIST;

  // Create a linked list of free pairs
  for (Location loc = end - 2; loc >= start; loc -= 2) {
    // Store the current head as the 'next' pointer
    set(loc, term_new(NUL, 0, FREE_LIST));
    // Mark the second cell as free
    set(loc + 1, 0);
    // Update the free list head
    FREE_LIST = loc;
	
    // Break if we've reached the start (needed for unsigned wrap-around)
    if (loc == start) break;
  }
}

int alloced = 0;

// Allocate a pair from the free list - O(1)
Location pair_alloc(void) {
  alloced++;

  // Get a pair from the free list and update FREE_LIST atomically
  Location expected = atomic_load(&FREE_LIST);
  Location loc, new_free_list;
  Term next;

  do {
    // If free list is empty
    if (expected == EMPTY_FREE_LIST) {
      // Check if we have space in the buffer
      if (RNOD_END + 2 >= BUFF_SIZE) {
        fprintf(stderr, "Error: Not enough space to allocate pair. RNOD_END=%lu, BUFF_SIZE=%lu\n",
	  RNOD_END, BUFF_SIZE);
        abort();
      }
      Location loc = RNOD_END;
      RNOD_END += 2;
      return loc;
    }

    // Get the location we want to return
    loc = expected;

    // Get the next free pair location
    next = get(loc);
    new_free_list = (Location)(next >> (TAG_SIZE + LAB_SIZE));

    // Try to update FREE_LIST, retry if it changed
  } while (!atomic_compare_exchange_weak(&FREE_LIST, &expected, new_free_list));

  return loc;
}

// Free a pair by adding it to the free list - O(1)
void pair_free(Location loc) {
  alloced--;

  // Clear the second cell
  set(loc + 1, 0);

  // Atomically update FREE_LIST
  Location expected, desired;
  do {
    // Read the current free list head
    expected = atomic_load(&FREE_LIST);

    // Set up the node to point to the current head
    set(loc, term_new(NUL, 0, expected)); // Store next free pair location

    // Try to update FREE_LIST to point to our node
    desired = loc;
  } while (!atomic_compare_exchange_weak(&FREE_LIST, &expected, desired));
}

void hvm_reset(void) {
  if (BUFF == NULL || RBAG_BUFF == NULL) {
    fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvm_init first.\n");
    abort();
  }

  // Clear memory to prevent stale data
  memset(BUFF, 0, RNOD_END * sizeof(a64));
  memset(RBAG_BUFF, 0, RBAG_SIZE * sizeof(Term));

  // Reset node index
  RNOD_END = 0;

  // Reset bag index
  RBAG_END = 0;

  // Initialize the free list (initially empty)
  FREE_LIST = EMPTY_FREE_LIST;
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
  case I60: return "I60";
  case F60: return "F60";
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
  if (n != 1 && n != 2) {
    fprintf(stderr, "Error: Invalid port number %lu. Port must be 1 or 2.\n", n);
    abort();
  }
  return n + x - 1;
}

// Atomic swap operation
Term swap(Location loc, Term term) {
  Term result = atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
  return result;
}

Term take(Location loc) {
  // Take the term at the given location, replacing it with 0
  Tag takenTag;
  Term taken;
  do {
    taken = get(loc);
    takenTag = term_tag(taken);
    if (takenTag != SUB) {
      set(loc, 0);
      if (get(loc & 0xFFFFFFFE) == 0 && get((loc & 0xFFFFFFFE) + 1) == 0) {
	pair_free(loc & 0xFFFFFFFE);
      }
      if (takenTag == VAR) {
	loc = term_loc(taken);
      }
    }
  } while (takenTag == VAR);
  if (takenTag == SUB)
    return term_new(VAR, 0, loc);
  else
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

  // Store terms in their respective ports
  set(port(1, loc), fst);
  set(port(2, loc), snd);

  return term_new(tag, lab, loc);
}

// Move a positive term into a negative location
void move(Location neg_loc, Term pos) {
  Term neg = swap(neg_loc, pos);
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
  if (term_tag(neg) == SUB) {
    if (term_lab(neg) > 0) {
      // If SUB has a location, link the pair at that location
      Location sub_loc = term_loc(neg);

      // Get the terms at the SUB location
      Term sub_neg = get(port(1, sub_loc));
      Term sub_pos = get(port(2, sub_loc));

      // Link the terms - use the first term in the pair (which should be a negative term)
      // and the positive term that was moved to the SUB location
      term_link(sub_neg, sub_pos);
    }
  } else {
    pos = take(neg_loc);
    term_link(neg, pos);
  }
}

// Link two terms together
// Push a redex (pair of terms) to the reduction bag
void term_link(Term neg, Term pos) {
#ifdef SAFETY
  // Check if terms have the correct polarity
  if (is_positive(neg)) {
    fprintf(stderr, "Error: term_link called with positive term in negative position: %s\n",
            tag_to_string(term_tag(neg)));
    BOOM("bad redex - positive term in negative position");
  }

  if (is_negative(pos)) {
    fprintf(stderr, "Error: term_link called with negative term in positive position: %s\n",
            tag_to_string(term_tag(pos)));
    BOOM("bad redex - negative term in positive position");
  }
#endif

  Term neg_var;
  switch(term_tag(pos)) {
  case VAR:
    neg_var = swap(term_loc(pos), neg);
    if (term_tag(neg_var) != SUB) {
      move(term_loc(pos), neg_var);
    }
    break;

  case I60:
  case F60:
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
#ifdef SAFETY
  if (term_tag(neg) == ERA)
    BOOM("don't push ERA redex");
  if (term_tag(pos) == NUL)
    BOOM("don't push NUL redex");
  if (is_positive(neg) || is_negative(pos))
    BOOM("bad redex");
#endif

#ifndef SINGLE_THREAD
  // Acquire mutex before modifying the redex bag
  pthread_mutex_lock(&redex_mutex);
#endif

#ifdef SAFETY
  // Check if there's space in the bag
  if (RBAG_END + 2 > RBAG_SIZE) {
    fprintf(stderr, "Error: Redex bag is full. RBAG_END=%lu, RBAG_SIZE=%lu\n",
	    RBAG_END, RBAG_SIZE);
    abort();
  }
#endif

  // Store the redex in the bag
  RBAG_BUFF[RBAG_END] = neg;
  RBAG_BUFF[RBAG_END + 1] = pos;
  RBAG_END += 2;

  // Signal that a redex is available
  pthread_cond_signal(&redex_cond);

  // Release mutex
#ifndef SINGLE_THREAD
  pthread_mutex_unlock(&redex_mutex);
#endif
}

bool stop_reducing = false;

// Pop a redex (pair of terms) from the reduction bag
// Returns false if the bag is empty, true otherwise
bool pop_redex(Term* neg, Term* pos) {
  bool result = false;

  // Acquire mutex before accessing the redex bag
#ifndef SINGLE_THREAD
  pthread_mutex_lock(&redex_mutex);
#endif

  // Check if the bag is empty
  if (RBAG_END > 0) {
    // Get the redex from the bag (LIFO order - pop from the end)
    RBAG_END -= 2;
    *neg = RBAG_BUFF[RBAG_END];
    *pos = RBAG_BUFF[RBAG_END + 1];
    result = true;
  }

  // Release mutex
#ifndef SINGLE_THREAD
  pthread_mutex_unlock(&redex_mutex);
#endif

  return result;
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
  move(ret, dp2);
  term_link(dp1, arg);
  term_link(cn1, tm1);
  term_link(cn2, tm2);
  return true;
}

// Application-Null interaction
bool appnul(Term app, Term nul) {
  Location app_loc = term_loc(app);
  move(port(2, app_loc), NUL);
  term_link(ERA, take(port(1, app_loc)));
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

// Create a REF term with a specific interaction function
Term ref_make(interactionFn fn) {
  // Store the function pointer in the term, with the REF tag
  return ((Term)fn & ~0xF) | REF;
}

// APP-REF interaction
// When an APP term meets a REF term, call the function stored in the REF term
bool appref(Term app, Term ref) {
  interactionFn fnPtr;
  fnPtr = (interactionFn)(ref & ~0xF);
  fnPtr(ref, app);
  return true;
}

bool appnum(Term app, Term num) {
  Location app_loc = term_loc(app);
  move(port(2, app_loc), num);
  term_link(ERA, take(port(1, app_loc)));
  return true;
}

bool opnul(Term op, Term nul) {
  Location op_loc = term_loc(op);
  move(port(2, op_loc), nul);
  term_link(ERA, take(port(1, op_loc)));
  return true;
}

// SUB-NUL interaction
bool subnul(Term sub, Term nul) {
  // Check if the SUB term has a location (label > 0)
  Lab lab = term_lab(sub);
  if (lab > 0) {
    // The SUB term has a location pointing to a pair
    Location sub_loc = term_loc(sub);

    // Take the first port and link it with NUL
    Term t = take(port(1, sub_loc));
    term_link(t, NUL);

    // Take the second port and link it with ERA
    t = take(port(2, sub_loc));
    term_link(ERA, t);
  }

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

bool YNUM(Term opy, Term num) {
  Location op_loc = term_loc(opy);
  Term x = take(port(1, op_loc));
  Tag y_type = term_tag(num);
  Location ret = port(2, op_loc);
  u32 res;
  Lab op = term_lab(opy);

  switch (y_type) {
  case I60: PERFORM_OP(get_u64(x), get_u64(num), op, i64); break;
    // case F60: PERFORM_OP(x, y, op, f64); break;
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
  &ABRT,&ABRT,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&XNUM,&XNUM,&ABRT
// VAL   VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define OPY_INTERACTIONS						\
  &ABRT,&ABRT,&ABRT,&opnul,&ABRT,&ABRT,&ABRT,&ABRT,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&YNUM,&YNUM,&ABRT
// VAL   VAR   SUB    NUL   ERA   LAM   APP   REF   VL1   SUP   DUP   OPX   OPY   I60   F60   LAZ

#define ERA_INTERACTIONS						\
  &ABRT,&ABRT,&ABRT,&NOP,&ABRT,&eralam,&ABRT,&NOP,&ABRT,&erasup,&ABRT,&ABRT,&ABRT,&NOP,&NOP,&ABRT
// VAL   VAR   SUB   NUL   ERA   LAM    APP   REF  VL1    SUP    DUP   OPX   OPY   I60  F60  LAZ

#define APP_INTERACTIONS						\
  &ABRT,&ABRT,&ABRT,&appnul,&ABRT,&applam,&ABRT,&appref,&ABRT,&DNEG,&ABRT,&ABRT,&ABRT,&appnum,&appnul,&ABRT
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

unsigned reduced = 0;
bool interact(Term neg, Term pos) {
  reduced++;
  // Gets the rule type.
  interactionFn rule = interactions[term_tag(neg)][term_tag(pos)];

  // Swaps ports if necessary.
  rule(neg, pos);
  return true;
}

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void *normalize(void *v) {
  printf("normalizing\n");
  Term neg, pos;

  // Process redexes until the stack is empty
  while (pop_redex(&neg, &pos)) {
    // Perform the interaction
    interact(neg, pos);
  }

  return NULL;
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

// extract the requested number of native args. I60, F60, REF or VAL terms
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct) {
  // if (argsStruct->count > 0) {
  // fprintf(stderr, "nativeArg %d: %p %d %p\n", __LINE__, (void *)ref, argsStruct->count,
  // (void *)get_i24(get_val(argsStruct->args[argsStruct->count - 1])));
  // } else {
  // fprintf(stderr, "nativeArg %d: %p %d\n", __LINE__, (void *)ref, argsStruct->count);
  // }
  // 'args' will only ever be a negative term
  Tag argsTag = term_tag(args);
  // fprintf(stderr, "argsTag %d: %s (%d) in %p\n", __LINE__,
  //	  tag_to_string(argsTag), argsTag, (void *)args);
  Term arg;
  Term varVal;
  Term retry;
  Term newArgs;
  Term newArg;
  switch(argsTag) {
  case APP:
    arg = take(port(1, term_loc(args)));
    if (expected == 0) {
      return args;
    }

    // 'arg' will only ever be a positive term
    Tag argTag = term_tag(arg);
    // fprintf(stderr, "arg 2 %d: %d %p\n", __LINE__, argTag, (void *)arg);
    switch(argTag) {
      // the strict arg types
    case VAL:
    case I60:
    case F60:
    case REF:
      argsStruct->args[argsStruct->count++] = arg;
      if (expected > 1)
	return strictArgs(ref, take(port(2, term_loc(args))), expected - 1, argsStruct);
      else
	return args;
      break;

    case VAR: {
      Term valVar = get(term_loc(arg));
      switch(term_tag(valVar)) {
	// the strict arg types
      case VAL:
      case I60:
      case F60:
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
	if (valVar != SUB)
	  BOOM("nativeArgs");
	else {
	  argsStruct->args[argsStruct->count++] = args;
	  newArgs = argsNet(argsStruct);
	  set(port(1, term_loc(args)), arg);
	  retry = pair_make(SUB, 0, newArgs, ref);
	  newArg = swap(term_loc(arg), retry);
	  if (newArg != SUB) {
	    // someone slipped the needed arg in since we last looked
	    set(term_loc(arg), newArg);
	    take(port(1, term_loc(retry)));
	    take(port(2, term_loc(retry)));
	    term_link(newArgs, ref);
	  }
	  argsStruct->count = -1;
	  return 0;
	}
	break;

      default: {
	char s[50];
	sprintf(s, "bad %s valVar", tag_to_string(term_tag(valVar)));
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
    // */
    return 0;
    break;

  case SUB:
  case ERA:
  case DUP:
  case OPX:
  case OPY:
  default:
    printf("unhandled tag %s (0x%x) %p line: %d\n",
	   tag_to_string(argsTag), argsTag, (void *)arg, __LINE__);
    abort();
    return 0;
    break;
  }
}

