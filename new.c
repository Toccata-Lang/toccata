#include <string.h>
#include "new.h"

// Global heap
#ifdef NON_ATOMIC
static u64* nodeBuff = NULL;
#else
static a64* nodeBuff = NULL;
#endif
a64 buffEnd = 0; // Only need to track the end of the node space
static u64 buffSize = 0; // Size of the main buffer for bounds checking

// Free list for O(1) pair allocation
__thread Location freeList = EMPTY_FREE_LIST; // Head of the free list (atomic for thread safety)

// Redex stack
__thread Pairs pairs;

// interaction jump table
interactionFn interactions[16][16];

// Convert a tag to its string representation
const char* tagStr(Tag tag) {
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

// TODO: write a time64() function that returns the time as fast as possible as a u64
u64 time64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
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

// abort on invalid reduction
void badrdx(Term neg, Term pos) {
  char msg[200];
  sprintf(msg, "Bad interaction: %s %s\na: %p b: %p\n",
	  tag_to_str(termTag(neg)), tag_to_str(termTag(pos)),
	  (void *)neg, (void *)pos);
  BOOM(msg);
}

// Create a new term with given tag, label, and location
Term newTerm(Tag tag, Lab lab, Location loc) {
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
Tag termTag(Term term) {
  Tag t = (Tag)(term & TAG_MASK);
  if (t == VL1)
    return VAL;
  else
    return t;
}

Term valTerm(Term val) {
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
Lab termLab(Term term) {
  return (Lab)((term >> TAG_SIZE) & LAB_MASK);
}

char hasLocation(Term tree) {
  Tag t = termTag(tree);

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

// Get the location of a term
Location termLoc(Term term) {
#ifdef SAFETY
  if (!hasLocation(term)) {
    char msg[100];
    snprintf(msg, 95, "term has no location: %s", tagStr(termTag(term))); 
    BOOM(msg);
  }
#else
  return (Location)(term >> (TAG_SIZE + LAB_SIZE));
#endif
}

Location port(u64 n, Term trm) {
#ifdef SAFETY
  if (n != 1 && n != 2) {
    fprintf(stderr, "Error: Invalid port number %lu. Port must be 1 or 2.\n", n);
    abort();
  }
#endif
  return n + termLoc(trm) - 1;
}

void pushRedex(Term neg, Term pos) {
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
    // } else if (interactions[termTag(neg)][termTag(pos)] == &ABRT) {
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
  Term result = nodeBuff[loc];
#else
  Term result = atomic_load_explicit(&nodeBuff[loc], memory_order_relaxed);
#endif
  return result;
}

// Free a pair by adding it to the free list - O(1)
void freePair(Location loc) {
#ifdef SAFETY
  atomic_fetch_add_explicit(&glblAlloced, -1, memory_order_relaxed);
#endif

  // Clear the second cell
#ifdef NON_ATOMIC
  nodeBuff[loc + 1] = VOID;
#else
  atomic_store_explicit(&nodeBuff[loc + 1], VOID, memory_order_relaxed);
#endif

  Location currTop;
  do {
    currTop = freeList;
    switch(currTop) {
    case LOCK_FREE_LIST:
      break;

    default:
      // Set up the node to point to the current head
#ifdef NON_ATOMIC
      nodeBuff[loc] = newTerm(NUL, 0xFF, currTop);
#else
      atomic_store_explicit(&nodeBuff[loc], newTerm(NUL, 0xFF, currTop), memory_order_relaxed);
#endif
      freeList = loc;
      break;
    }
  } while (currTop == LOCK_FREE_LIST);
}

void freeLoc(Location loc) {
#ifdef NON_ATOMIC
  nodeBuff[loc] = VOID;
#else
  atomic_store_explicit(&nodeBuff[loc], VOID, memory_order_relaxed);
#endif
  Location evenLoc = loc & 0xFFFFFFFE;
  if (get(evenLoc) == VOID && get(evenLoc + 1) == VOID) {
    freePair(evenLoc);
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
    takenTag = termTag(taken);
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
	loc = termLoc(taken);
      }
    }
  } while (takenTag == VAR);

  switch(takenTag) {
  case SUB:
  case LAZ:
    return newTerm(VAR, 0, loc);

  default:
    return taken;
  }
}

#define NODE_STACK_SIZE 1000
typedef struct cycleNode {
  Location loc;
  Term trm;
  struct cycleNode *left;
  struct cycleNode *right;
} cycleNode;
cycleNode cycleNodes[NODE_STACK_SIZE];
unsigned cycleNodeCount = 0;

char findCycleNode(Location nodeLoc) {
  for (unsigned i = 0; i < cycleNodeCount; i++) {
    cycleNode *nd = &cycleNodes[i];
    if (nd->loc == nodeLoc)
      return 1;
  }
  return 0;
}

unsigned callCount = 0;
int eraseSubCycle(Term tree, Location tgtLoc) {
#ifndef CHECK_MEM_LEAK
  BOOM("Not thread safe");
#endif

  if (!hasLocation(tree) || findCycleNode(termLoc(tree) & 0xFFFFFFFE)) {
    return 0;
  }
  // fprintf(stderr, "tgtLoc: %lx\n", tgtLoc);
  // print_term("tree", tree);

  cycleNode *cn = &cycleNodes[cycleNodeCount++];
  if (cycleNodeCount > 999)
    BOOM("cycleNodeCount!");
  cn->loc = hasLocation(tree) ? termLoc(tree) : RNOD_END;
  cn->trm = tree;

  Tag t = termTag(tree);
  switch(t) {
  case VAR: {
    Location loc = termLoc(tree);
    if (loc == tgtLoc) {
      return 1;
    } else {
      return eraseSubCycle(get(loc), tgtLoc);
    }
    // */
  }
    break;

  case SUP:
  case DUP: 
  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP: {
    Location loc = port(1, termLoc(tree));
    Term branch = get(loc);
    eraseSubCycle(branch, tgtLoc);
    if (termTag(branch) == VAR && termLoc(branch) == tgtLoc)
      return 1;

    loc = port(2, termLoc(tree));
    branch = get(loc);
    eraseSubCycle(branch, tgtLoc);
    if (termTag(branch) == VAR && termLoc(branch) == tgtLoc)
      return 1;
  }
    break;
  }
  return 0;
}

int eraseCycle(Term tree, Location tgtLoc) {
  callCount = 0;
  cycleNodeCount = 0;
  return eraseSubCycle(tree, tgtLoc);
}

// Atomic swap operation
// If a deferred redex is found, queue it up and return SUB
// Otherwise, return a positive value.
Term swap(Location loc, Term term) {
#ifdef SAFETY
  if (term == VOID)
    BOOM("bad swap");
#endif
#ifdef NON_ATOMIC
  Term result = nodeBuff[loc];
  nodeBuff[loc] = term;
#else
  Term result = atomic_exchange_explicit(&nodeBuff[loc], term, memory_order_relaxed);
#endif
  switch(termTag(result)) {
  case SUB:
    if (result != SUB) {
      Term neg = get(port(1, termLoc(result)));
      Term pos = get(port(2, termLoc(result)));
      pushRedex(neg, pos);
      freePair(termLoc(result));
      result = SUB;
    }
    break;

  case ERA:
    freeLoc(loc);
    interact(result, term);
    break;
  }
  return result;
}

void forceLazy(Term z) {
  if (termTag(z) != LAZ)
    return;

  Location lazLoc = termLoc(z);
  // 'z' is a LAZ term
  Term neg = take(port(1, lazLoc));
  Term pos = take(port(2, lazLoc));

  if (termTag(neg) == DUP) {
    Location negLoc = termLoc(neg);

    // this is a lazy DUP, which ever port points to itself
    // gets replaced with SUB
    Term curr = get(port(1, negLoc));
    if (curr == z)
      swap(port(1, negLoc), SUB);
    curr = get(port(2, negLoc));
    if (curr == z)
      swap(port(2, negLoc), SUB);

    if (termTag(pos) == VAR) {
      Location posLoc = termLoc(pos);
      Term lz = swap(posLoc, pair_make(SUB, 6, neg, pos));
      if (termTag(lz) == LAZ)
	forceLazy(lz);
    } else
      pushRedex(neg, pos);
  } else {
    pushRedex(neg, pos);
  }
}

// Move a positive term into a negative location
// If anything besides a deferred redex is there, it must be a
// negative and should be reduced with 'pos'
void move(Location negLoc, Term pos) {
  Term neg = swap(negLoc, pos);
  Tag negTag = termTag(neg);

#ifdef SAFETY
  if (is_negative(pos)) {
    char s[50];
    sprintf(s,"trying to move a negative to location %.3x: %p", negLoc, (void *)neg);
    BOOM(s);
  }
  if (is_positive(neg)) {
    char s[50];
    print_term("moved pos", pos);
    print_term("pos at neg", neg);
    sprintf(s,"found positive at move target %.3x: %p", negLoc, (void *)neg);
    BOOM(s);
  }
#endif
  if (negTag != SUB && negTag != ERA) {
    freeLoc(negLoc);
    if (pos == NUL)
      interact(neg, pos);
    else 
      pushRedex(neg, pos);
  }
}

void moveDuped(Location neg_loc, Term pos) {
  Term neg = get(neg_loc);
  Tag negTag = termTag(neg);
  switch (negTag) {
  case SUB:
    swap(neg_loc, pos);
    break;
    
  case APP:
  case ERA:
  case DUP:
  case OPX:
  case OPY:
    take(neg_loc);
    pushRedex(neg, pos);
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
bool popRedex(Term* neg, Term* pos) {
  bool result = false;

  if (pairs.count > 0) {
    pairs.count -= 1;
    *neg = pairs.rdxs[pairs.count][0];
    *pos = pairs.rdxs[pairs.count][1];
    return true;
  }
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
    loc = freeList;
    switch(loc) {
    case LOCK_FREE_LIST:
      break;

    case EMPTY_FREE_LIST:
      loc = atomic_fetch_add_explicit(&RNOD_END, 2, memory_order_relaxed);
      // printf("new pair: %d\n", loc);
      // Check if we have space in the buffer
      if (loc >= nodeBuff_SIZE) {
	fprintf(stderr, "Error: Not enough space to allocate pair. RNOD_END=%u, nodeBuff_SIZE=%lu\n",
		loc, nodeBuff_SIZE);
	abort();
      }
      break;

    default: {
      // Get the next free pair location
      Term next = get(loc);
      Location new_free_list = (Location)(next >> (TAG_SIZE + LAB_SIZE));
      freeList = new_free_list;
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

// Check if a term is positive
bool is_positive(Term term) {
  switch (termTag(term)) {
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
  switch (termTag(term)) {
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

long graphCount = 0;
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
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(termTag(snd)));
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
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  case DUP:
    // Port 1 must be negative
    if (!is_negative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be negative
    if (!is_negative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    break;

  case SUP:
    // Port 1 must be positive
    if (!is_positive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tag_to_str(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tag_to_str(termTag(snd)));
      fprintf(stderr, "  Line: %d\n", line);
      abort();
    }
    // Port 2 must be positive
    if (!is_positive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tag_to_str(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tag_to_str(termTag(snd)));
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
  nodeBuff[port(1, loc)] = fst;
  nodeBuff[port(2, loc)] = snd;
#else
  atomic_store_explicit(&nodeBuff[port(1, loc)], fst, memory_order_relaxed);
  atomic_store_explicit(&nodeBuff[port(2, loc)], snd, memory_order_relaxed);
#endif

  Term new_pair = newTerm(tag, lab, loc);
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

void eraseDupCycle(Term dup, Term pos) {
  Location dup_p1 = port(1, termLoc(dup));
  Location dup_p2 = port(2, termLoc(dup));
  Term dp1 = get(dup_p1);
  Term dp2 = get(dup_p2);

  if (dp1 == sideEffects)
    eraseCycle(pos, dup_p2);
  if (dp2 == sideEffects)
    eraseCycle(pos, dup_p1);
}

void negvar(Term neg, Term var) {
  var = take(termLoc(var));
  if (termTag(var) == VAR) {
    Term val = swap(termLoc(var), neg);
    switch(termTag(val)) {
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
      take(termLoc(var));
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
  Location app_loc = termLoc(app);
  Location lam_loc = termLoc(lam);

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
    Term trm = swap(tv->trmLoc, NUL);
    if (termTag(trm) == VAR)
      trm = take(termLoc(trm));
    pushRedex(app, trm);
  } else {
    Term dup = pair_make(DUP, 0, SUB, SUB);
    Term sub = pair_make(SUB, 7, app, newTerm(VAR, 0, port(1, termLoc(dup))));
    swap(port(1, termLoc(dup)), sub);

    Term trm = swap(tv->trmLoc, newTerm(VAR, 0, port(2, termLoc(dup))));
    if (termTag(trm) == VAR)
      trm = take(termLoc(trm));
    pushRedex(dup, trm);
  }
  dec_and_free(val, 1);
  return;
}

Term makeLazyDup(Lab lb, Term arg) {
  Term dp = pair_make(DUP, lb, SUB, SUB);
  Location loc = termLoc(dp);
  Tag t = termTag(arg);
  if (t == I60 || t == F60 || t == REF || t == VAL) {
    incRef(arg, 1);
    swap(port(1, loc), arg);
    swap(port(2, loc), arg);
  } else {
    Term lz = pair_make(LAZ, 0, dp, arg);
    swap(port(1, loc), lz);
    swap(port(2, loc), lz);
  }
  return dp;
}

int decSubRefs(Location sup_loc) {
  return atomic_fetch_sub_explicit(&nodeBuff[sup_loc], 1, memory_order_relaxed);
}

// distribute a negative through a SUP
void negsup(Term neg, Term sup) {
  Location sup_loc = termLoc(sup);
  Term tm1 = take(port(1, sup_loc));
  Term tm2 = take(port(2, sup_loc));
  if (termTag(tm1) == NUL) {
    pushRedex(neg, tm2);
  } else if (termTag(tm2) == NUL) {
    pushRedex(neg, tm1);
  } else {
    Lab sup_lab = term_lab(sup);
    Location neg_loc = termLoc(neg);
    Tag neg_tag = termTag(neg);
    Lab neg_lab = term_lab(neg);

    Term arg = take(port(1, neg_loc));
    Location ret = port(2, neg_loc);
    Term dp1 = makeLazyDup(sup_lab, arg);
    Term cn1 = pair_make(neg_tag, neg_lab,
			 newTerm(VAR, 0, port(1, termLoc(dp1))),
			 SUB);
    Term lz1 = pair_make(LAZ, 0, cn1, tm1);
    swap(port(2, termLoc(cn1)), lz1);
    Term cn2 = pair_make(neg_tag, neg_lab,
			 newTerm(VAR, 0, port(2, termLoc(dp1))),
			 SUB);
    swap(port(2, termLoc(cn2)), pair_make(LAZ, 0, cn2, tm2));
    // TODO: could you make the ports of the SUP store direct LAZ terms
    // and not VAR's?
    Term dp2 = pair_make(SUP, sup_lab,
			 newTerm(VAR, 0, port(2, termLoc(cn1))),
			 newTerm(VAR, 0, port(2, termLoc(cn2))));
    move(ret, dp2);
  }
}

// Application-Null interaction
void appnul(Term app, Term nul) {
  Location app_loc = termLoc(app);
  Term pos = take(port(1, app_loc));
  interact(ERA, pos);
  move(port(2, app_loc), NUL);
  return;
}

unsigned nodeCount = 0;
// Duplication-Lambda interaction
void duplam(Term dup, Term lam) {
  Location dup_loc = termLoc(dup);
  Location lam_loc = termLoc(lam);
  if (get(port(1, dup_loc)) == ERA) {
    take(port(1, dup_loc));
    move(port(2, dup_loc), lam);
  } else if (get(port(2, dup_loc)) == ERA) {
    take(port(2, dup_loc));
    move(port(1, dup_loc), lam);
  } else {
    if (dup_loc == 0x1c && lam_loc == 0xb2) {
     pb();
    }
    Lab lam_lab = term_lab(lam);
    Lab dup_lab = term_lab(dup);
    Location var = port(1, lam_loc);
    Term bod = take(port(2, lam_loc));
    Term l1 = pair_make(LAM, lam_lab, SUB, NUL);
    Term l2 = pair_make(LAM, lam_lab, SUB, NUL);
    Term du1 = pair_make(SUP, dup_lab,
			 newTerm(VAR, 0, port(1, termLoc(l1))),
			 newTerm(VAR, 0, port(1, termLoc(l2))));
    Term du2 = makeLazyDup(dup_lab, bod);
    swap(port(2, termLoc(l1)), newTerm(VAR, 0, port(1, termLoc(du2))));
    swap(port(2, termLoc(l2)), newTerm(VAR, 0, port(2, termLoc(du2))));
    move(var, du1);
    move(port(1, dup_loc), l1);
    move(port(2, dup_loc), l2);

    if (dup_loc == 0x1c && lam_loc == 0xb2) {
      graphDown("l1", l1, 0, subGraphs++);
      graphDown("l2", l2, nodeCount, subGraphs++);
      pb();
    }
  }
  return;
}

// Duplication-Superposition interaction
void dupsup(Term dup, Term sup) {
  Lab dup_lab = term_lab(dup);
  Lab sup_lab = term_lab(sup);
  Location dup_loc = termLoc(dup);
  Location sup_loc = termLoc(sup);

  // Get the ports of the DUP node
  Location dup_p1 = port(1, dup_loc);
  Location dup_p2 = port(2, dup_loc);

  if (dup_lab == sup_lab) {
    // Special case: when DUP and SUP have the same label, they annihilate
    // Get the ports of the SUP node
    Term sup_p1 = take(port(1, sup_loc));
    Term sup_p2 = take(port(2, sup_loc));

    // Direct connection of the ports
    move(dup_p1, sup_p1);
    move(dup_p2, sup_p2);
  } else {
    Term dp1 = get(dup_p1);
    Term dp2 = get(dup_p2);

    dp1 = get(dup_p1);
    dp2 = get(dup_p2);
    if (dp1 == ERA) {
      Term trm = take(port(1, dup_loc));
      move(port(2, dup_loc), sup);
    } else if (dp2 == ERA) {
      Term trm = take(port(2, dup_loc));
      move(port(1, dup_loc), sup);
    } else  {
      // Get the ports of the SUP node
      Term sup_p1 = take(port(1, sup_loc));
      Term sup_p2 = take(port(2, sup_loc));

      // Create two new DUP nodes with the same label
      Term dup1 = makeLazyDup(dup_lab, sup_p1);;
      Term dup2 = makeLazyDup(dup_lab, sup_p2);;

      // Create two new SUP nodes with the same label
      Term sup1 = pair_make(SUP, sup_lab,
			    newTerm(VAR, 0, port(1, termLoc(dup1))),
			    newTerm(VAR, 0, port(1, termLoc(dup2))));
      Term sup2 = pair_make(SUP, sup_lab,
			    newTerm(VAR, 0, port(2, termLoc(dup1))),
			    newTerm(VAR, 0, port(2, termLoc(dup2))));

      // Connect the new nodes
      move(dup_p1, sup1);
      move(dup_p2, sup2);
    }
  }
  return;
}

// Duplication interaction with copyable term
void copy(Term dup, Term trm) {
  Location dup_loc = termLoc(dup);

  // Get port locations
  Location dp1_loc = port(1, dup_loc);
  Location dp2_loc = port(2, dup_loc);

  if (termTag(trm) == VAL)
    incRef(trm, 1);

  // put trm in both copy ports
  moveDuped(dp2_loc, trm);
  moveDuped(dp1_loc, trm);
  return;
}

// Eeraser-Var interaction
void eravar(Term era, Term var) {
  Term val = take(termLoc(var));
  if (termTag(val) == VAR) {
    Term lz = swap(termLoc(val), era);
    if (lz != SUB) {
      eraseLazy(lz, era);
#if 0
      // TODO: remove this
      Term lzNeg = get(port(1, lzLoc));
      switch(termTag(lzNeg)) {
      case DUP: {
	Location dupLoc = termLoc(lzNeg);
	Term dp1 = get(port(1, dupLoc));
	Term dp2 = get(port(2, dupLoc));

	if (dp1 == ERA && dp2 == ERA) {
	  take(port(1, lzLoc));
	  take(port(1, dupLoc));
	  take(port(2, dupLoc));
	  Term lzPos = take(port(2, lzLoc));
	  interact(ERA, lzPos);
	} else if (dp1 == sideEffects || dp2 == sideEffects) {
	  forceLazy(lz);
	} else if (dp1 == lz) {
	  take(port(2, dupLoc));
	  take(port(1, lzLoc));
	  swap(port(1, dupLoc), take(port(2, lzLoc)));
	} else if (dp2 == lz) {
	  take(port(1, dupLoc));
	  take(port(1, lzLoc));
	  swap(port(2, dupLoc), take(port(2, lzLoc)));
	}
	// else if (dp1 == lz || dp2 == lz)
      }
	break;

      case APP:
      case OPX:
	interact(era, lz);
	break;

      default: {
	char s[150];
	sprintf(s, "unhandled freeing lazy: %s", tag_to_str(termTag(lzNeg)));
	BOOM(s);
	freeLoc(termLoc(val));
      }
	break;
      }
#endif
    }
  } else {
    interact(era, val);
  }
  return;
}

// Eraser-Lambda interaction
void eralam(Term era, Term lam) {
  Location lam_loc = termLoc(lam);
  Term body = take(port(2, lam_loc));
  interact(era, body);
  move(port(1, lam_loc), NUL);
  return;
}

void eralaz(Term era, Term laz) {
  if (term_lab(era) > 0) {
    forceLazy(laz);
  } else
    eraseLazy(laz, ERA);
}

// Eraser-Superposition interaction
void erasup(Term era, Term sup) {
  Location sup_loc = termLoc(sup);
  interact(era, newTerm(VAR, 0, port(2, sup_loc)));
  interact(era, newTerm(VAR, 0, port(1, sup_loc)));
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
  Location app_loc = termLoc(app);
  move(port(2, app_loc), num);
  pushRedex(ERA, take(port(1, app_loc)));
  return;
}

void opnul(Term op, Term nul) {
  Location op_loc = termLoc(op);
  if (term_lab(nul) == 0) {
    interact(take(port(2, op_loc)), NUL);
    interact(ERA, take(port(1, op_loc)));
  } else {
    BOOM("test this");
    move(port(2, op_loc), nul);
    pushRedex(sideEffects, take(port(1, op_loc)));
  }
  return;
}

// SUB-NUL interaction
void subnul(Term sub, Term nul) {
  // Check if the SUB term has a location (label > 0)
  if (sub != SUB) {
    // The SUB term has a location pointing to a pair
    Location sub_loc = termLoc(sub);

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
  Location dp1 = port(1, termLoc(dup));
  Location dp2 = port(2, termLoc(dup));
  move(dp1, nul);
  move(dp2, nul);
}

void YNUM(Term opy, Term num);
void XNUM(Term opx, Term num) {
  Location opx_loc = termLoc(opx);
  Term arg = swap(port(1, opx_loc), num);
  Lab op = term_lab(opx);
  switch (termTag(arg)) {
  case I60:
    YNUM(newTerm(OPY, op, port(1, opx_loc)), arg);
    break;

  case VAR:
    interact(newTerm(OPY, op, port(1, opx_loc)), arg);
    break;

  default:
    pushRedex(newTerm(OPY, op, port(1, opx_loc)), arg);
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

  Location op_loc = termLoc(opy);
  Term x = arityArgs.args[0];
  Tag y_type = termTag(num);
  Location ret = port(2, op_loc);
  u64 res;
  Lab op = term_lab(opy);

#ifdef SAFETY
  switch (termTag(x)) {
  case I60:
  case F60:
    break;

  default: {
    char msg[200];
    sprintf(msg, "wrong value to OPY: %s", tag_to_str(termTag(x))); 
    BOOM(msg);
  }
    break;
  }
#endif

  switch (y_type) {
  case I60: PERFORM_OP(get_u64(x), get_u64(num), op, i64); break;
    // case F60: PERFORM_OP(x, y, op, f64); break;
  }

  move(ret, new_num(y_type, res));
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

// subnul - done
// negvar - 
// opnul - done
// XNUM - done
// YNUM - done
// negsup - done
// erasup - done
// eralaz - done
// eralam - done
// appnul - done
// applam - done
// appref -
// copy - done
// dupnul - done
// duplam - done
// dupsup - done


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
unsigned otherNodes;
unsigned subGraphs = 0;

void interact(Term neg, Term pos) {
  /*
  if (1) {
    if (termTag(neg) != ERA && termTag(pos) != NUL) {
      if (1) {
	// fprintf(stderr, "%ld: ", graphCount);
	// print_raw_term(neg);
	// fprintf(stderr, " - ");
	// print_raw_term(pos);
	// fprintf(stderr, "\n");

	FILE *currDOT = dotFile;
	unsigned currSubG = subGraphs;
	subGraphs = 0;
	char dotName[100];
	sprintf(dotName, "graphs/%04ld-%ld-%ld.dot", graphCount, neg, pos);
	dotFile = fopen(dotName, "w");
	fprintf(dotFile, "graph grammar {\nranksep=0.1\n");
	graphDown("NEG", neg, 0, 0);
	graphDown("POS", pos, nodeCount, 0);
	fprintf(dotFile, "}\n");
	fclose(dotFile);
	dotFile = currDOT;
	subGraphs = currSubG;
      }
      graphCount++;
    }
  }
  if (rdxCount == 1484)
    BOOM("all done");
  // */

  // if (term_lab(pos) == SUP && termLoc(pos) == 0x15a) {
  // print_term("NEG", neg);
  // print_term("POS", pos);
  // }
  // if (term_lab(neg) == APP && termLoc(pos) == 0x2c) {
  // print_term("NEG", neg);
  // print_term("POS", pos);
  // }

#ifdef STATS
  atomic_fetch_add_explicit(&rdxCount, 1, memory_order_relaxed);
#endif
  // Gets the rule type.
  interactionFn rule = interactions[termTag(neg)][termTag(pos)];

  // Swaps ports if necessary.
  rule(neg, pos);
  return;
}

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void *normalize(void *v) {
  Term neg, pos;

  // Process redexes until the stack is empty
  while (popRedex(&neg, &pos)) {
    // Perform the interaction
    interact(neg, pos);
  }

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

void varArg(Term trm, Term ref, Term args, NativeArgs *argsStruct) {
  Location trmLoc = termLoc(trm);
  Term val = get(trmLoc);
  switch(termTag(val)) {
  case LAZ:
    swap(trmLoc, SUB);
    forceLazy(val);
	
  case SUB:
    // add the remaining args to argsStruct
    argsStruct->args[argsStruct->count++] = args;

    // create a chain of APP terms from argsStruct
    Term newArgs = argsNet(argsStruct);

    // put 'trm' back in it's place
    swap(port(1, termLoc(args)), trm);

    // make a deferred redex to retry the APP/REF pair when the value becomes available
    Term retry = pair_make(SUB, 5, newArgs, ref);

    // and put it in the location 'trm' points to
    Term newArg = swap(trmLoc, retry);
    if (newArg != SUB) {
      // someone slipped the needed trm in since we last looked
      swap(trmLoc, newArg);
      freePair(termLoc(retry));

      // so retry the original APP/REF redex
      pushRedex(newArgs, ref);
    }
    break;

  default:
    print_raw_term(val);
    printf("\n");
    BOOM("nativeArgs");
    break;
  }
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
      sprintf(msg, "%s %03x:", refName, termLoc(args));
      graphDown(msg, args);
    } else {
      graphDown("unknown", args);
    }
    // if (strcmp(refName, "str-eq") == 0) {
    // print_term("str-eq args", args);
    // }
  }
  // */
  Tag argsTag = termTag(args);
  if (argsTag == APP || argsTag == OPY) {
    // if 'args' is an APP term
    Term arg = take(port(1, termLoc(args)));
    if (expected == 0) {
      return args;
    }

    // 'arg' will only ever be a positive term
    Tag argTag = termTag(arg);
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
	return strictArgs(ref, take(port(2, termLoc(args))), expected - 1, argsStruct);
      else
	return args;
      break;

    case LAM: {
      TermVal *tv = malloc_term();
      tv->trmLoc = pair_alloc();
      swap(tv->trmLoc, arg);

      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = (Term)tv;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(port(2, termLoc(args))), expected - 1, argsStruct);
      else
	return args;
    }
      break;

    case NUL:
      move(port(2, termLoc(args)), NUL);
      for (int i = 0; i < argsStruct->count; i++)
	dec_and_free(argsStruct->args[i], 1);
      break;

    case SUP:
      for(int i = 0; i < argsStruct->count; i++)
	incRef(argsStruct->args[i], 1);
      Term s1 = take(port(1, termLoc(arg)));
      Term s2 = take(port(2, termLoc(arg)));
      Lab supLabel = term_lab(arg);
      int argsCount = argsStruct->count;
      argsStruct->count += 1;

      Term tail1 = pair_make(APP, 0, s1, SUB);
      argsStruct->args[argsCount] = tail1;
      swap(port(2, termLoc(tail1)), pair_make(LAZ, 0, argsNet(argsStruct), ref));

      Term tail2 = pair_make(APP, 0, s2, SUB);
      argsStruct->args[argsCount] = tail2;
      swap(port(2, termLoc(tail2)), pair_make(LAZ, 0, argsNet(argsStruct), ref));

      Term newSup = pair_make(SUP, supLabel,
			      newTerm(VAR, 0, port(2, termLoc(tail1))),
			      newTerm(VAR, 0, port(2, termLoc(tail2))));
      move(port(2, termLoc(args)), newSup);
      break;

    case VAR:
      varArg(arg, ref, args, argsStruct);
      break;

    case LAZ:
    default:
      fprintf(stderr, "unhandled tag %s (0x%x) line: %d\n", tag_to_str(termTag(arg)),
	     termTag(arg), __LINE__);
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
    Tag tag = termTag(t);
    Lab lab = term_lab(t);
    switch(termTag(t)) {
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
      fprintf(stderr, "%s %x %.3x", tag_to_str(tag), lab, termLoc(t));
      break;
    }
  }
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  fprintf(stderr, "%s:\n", prefix);
  fprintf(stderr, "  Tag: %s (%d, 0x%x)\n", tag_to_str(termTag(term)), termTag(term), termTag(term));
  Lab lab = term_lab(term);
  switch(termTag(term)) {
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
    fprintf(stderr, "  Location: %.3x\n", termLoc(term));
    // If this is a pair, print its contents
    if (termLoc(term) >= 0) {
      Term first = get(port(1, termLoc(term)));
      fprintf(stderr, "  term: ");
      print_raw_term(first);
      fprintf(stderr, "\n");
    }
    break;

  case SUP:
    if (lab == 1 || lab == 2) {
      fprintf(stderr, "  Location: %.3x\n", termLoc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
	Term first = get(port(1, termLoc(term)));
	Term second = get(port(2, termLoc(term)));
	fprintf(stderr, "  Refs: %d\n", (int)first);
	fprintf(stderr, "  Second term: ");
	print_raw_term(second);
	fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "  Location: %.3x\n", termLoc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
      // If this is a pair, print its contents
      if (termLoc(term) >= 0) {
	Term first = get(port(1, termLoc(term)));
	Term second = get(port(2, termLoc(term)));
	fprintf(stderr, "  First term: ");
	print_raw_term(first);
	fprintf(stderr, "\n");
	fprintf(stderr, "  Second term: ");
	print_raw_term(second);
	fprintf(stderr, "\n");
      }
    }
    break;

  case SUB:
    if (lab == 0)
      break;
    
  default:
    fprintf(stderr, "  Location: %.3x\n", termLoc(term));
    fprintf(stderr, "  Label: %.3x\n", lab);
    // If this is a pair, print its contents
    if (termLoc(term) >= 0) {
      Term first = get(port(1, termLoc(term)));
      Term second = get(port(2, termLoc(term)));
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
  return nodeBuff;
}
#else
a64* get_buff(void) {
  return nodeBuff;
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
  nodeBuff = (u64*)calloc(size, sizeof(a64));
#else
  nodeBuff = (a64*)calloc(size, sizeof(a64));
#endif
  if (!nodeBuff) {
    fprintf(stderr, "Failed to allocate memory\n");
    abort();
  }

  RBAG_nodeBuff = (Term*)calloc(RBAG_SIZE, sizeof(Term));
  if (!RBAG_nodeBuff) {
    fprintf(stderr, "Failed to allocate memory for redex stack\n");
    free(nodeBuff);
    nodeBuff = NULL;
    abort();
  }

  // Store the size of the buffer for bounds checking in pair_alloc
  nodeBuff_SIZE = size;

// Free allocated memory
void hvm_free(void) {
  if (nodeBuff == NULL) {
    return;
  }

  free(nodeBuff);
  nodeBuff = NULL;

  if (RBAG_nodeBuff != NULL) {
    free(RBAG_nodeBuff);
    RBAG_nodeBuff = NULL;
  }
}

void hvm_reset(void) {
  if (nodeBuff == NULL || RBAG_nodeBuff == NULL) {
    fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvm_init first.\n");
    abort();
  }

  // Clear memory to prevent stale data
  // memset(nodeBuff, 0, nodeBuff_SIZE * sizeof(Term));
  memset(RBAG_nodeBuff, 0, RBAG_SIZE * sizeof(Term));

  // Reset node index
  atomic_store_explicit(&RNOD_END, 0, memory_order_relaxed);;

  // Reset bag index
  atomic_store_explicit(&RBAG_END, 0, memory_order_relaxed);;

  // Initialize the free list (initially empty)
  freeList = EMPTY_FREE_LIST;
  atomic_store_explicit(&glblAlloced, 0, memory_order_relaxed);
  atomic_store_explicit(&rdxCount, 0, memory_order_relaxed);
  atomic_store_explicit(&waiting, 0, memory_order_relaxed);
  rdxCount = 0;
}

// For// Print contents of nodeBuff between start and end locations
void print_buff(Location start, Location end) {
#ifdef NON_ATOMIC
  u64* buff = get_buff();
#else
  a64* buff = get_buff();
#endif
  if (!buff) {
    fprintf(stderr, "nodeBuff is not initialized\n");
    return;
  }
  if (start >= end) {
    fprintf(stderr, "Invalid range: start=%u end=%u\n", start, end);
    return;
  }
  fprintf(stderr, "nodeBuff contents from %u to %u:\n", start, end);
  for (Location i = start; i < end; i += 2) {
    Term t1 = buff[i];
    if ((termTag(t1) != NUL || term_lab(t1) != 0xFF) || buff[i + 1] != 0) {
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
  fprintf(stderr, "Redexes: %d\n", pairs.count);
  for (int i = 0; i < pairs.count; i++) {
    fprintf(stderr, " %.3x  ", i);
    print_raw_term(pairs.rdxs[i][0]);
    fprintf(stderr, "  ");
    print_raw_term(pairs.rdxs[i][1]);
    fprintf(stderr, "\n");
  }
}

void check_buff() {
#ifdef NON_ATOMIC
  u64* buff = get_buff();
#else
  a64* buff = get_buff();
#endif
  if (!buff) {
    printf("nodeBuff is not initialized\n");
    return;
  }
  unsigned leaks = 0;
  for (Location i = 0; i < RNOD_END; i += 2) {
    Term t1 = buff[i];
    if (termTag(t1) != NUL || buff[i + 1] != 0) {
      leaks++;
    }
  }
  graphDown("leaked", get(0xaf), 0, subGraphs++);
  if (leaks) {
    fprintf(stderr, "\nLeaked pairs!!\n");
    // print_term("leaked", get(0x9c));
    pb();
    pr();
      // BOOM("Leak pairs");
  }
}

// Print the free list for debugging
void print_free_list(void) {
  printf("Free list: ");
  Location ptr = freeList;
  int count = 0;

  if (ptr == EMPTY_FREE_LIST) {
    printf("EMPTY ");
  } else {
    while (ptr != EMPTY_FREE_LIST && count < 100) { // Limit to prevent infinite loops
      printf("%u -> ", ptr);
      Term next = get(ptr);
      if (termTag(next) != NUL) {
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
  switch(termTag(arg)) {
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
    Term z = pair_make(LAZ, 0, newDup, arg);
    swap(port(1, termLoc(newDup)), z);
    swap(port(2, termLoc(newDup)), z);

    *dupedArg = newTerm(VAR, 0, port(2, termLoc(newDup)));
    return newTerm(VAR, 0, port(1, termLoc(newDup)));
  }
    break;
  }
}

Term make_op(Lab op, Term x, Term y) {
  Term t = pair_make(OPX, op, y, SUB);
  Term ret = newTerm(VAR, 0, port(2, termLoc(t)));
  swap(termLoc(ret), pair_make(LAZ, 0, t, x));
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

graphNode nodeStack[NODE_STACK_SIZE];

void fatal_error(char *fmt, unsigned bytes) {
  fprintf(stderr, fmt, bytes);
  abort();
}

unsigned graphSubUp(unsigned graphNum, Term tree);
unsigned upBranch(Term tree, unsigned pt, unsigned graphNum) {
  Tag t = termTag(tree);
  unsigned nodeNum;
  char *branchPort = pt == 1 ? "nw" : "ne";
  Location loc = port(pt, termLoc(tree));
  Term branch = get(loc);
  unsigned branchNode = 65536;
  for (unsigned i = 0; i < nodeCount; i++) {
    graphNode *gn = &nodeStack[i];
    if (gn->trm == branch) {
      branchNode = gn->node;
      break;
    }
  }

  Tag bt = termTag(branch);
  return graphSubUp(graphNum, branch);
}

void graphLink( unsigned graphNum, unsigned nodeNum, unsigned pt, Term branch, unsigned branchNode) {
  char *branchPort = pt == 1 ? "nw" : "ne";

  if (termTag(branch) == VAR &&
      ((termTag(get(termLoc(branch))) == LAZ &&
	termTag(get(port(1, termLoc(get(termLoc(branch)))))) == DUP) ||
       termTag(get(termLoc(branch))) == SUB)) {
    if (termLoc(branch) & 1)
      fprintf(dotFile, "x%d_%x:ne -- x%d_%x:%s\n",
	      graphNum, (termLoc(branch) & 0xFFFFFFFE), graphNum, nodeNum, branchPort);
    else
      fprintf(dotFile, "x%d_%x:nw -- x%d_%x:%s\n",
	      graphNum, (termLoc(branch) & 0xFFFFFFFE), graphNum, nodeNum, branchPort);
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
    nodeNum = termLoc(tree) & 0xFFFFFFFE;
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      if (gn->node == nodeNum)
	return gn->node;
    }
  } else {
    nodeNum = otherNodes++;
  }

  Tag t = termTag(tree);
  switch(t) {
  case VAR: {
    //*
    Location loc = termLoc(tree);
    Term trm = get(loc);
    if (trm != LAZ || termTag(get(port(1, termLoc(trm)))) != DUP) {
      return graphSubUp(graphNum, trm);
    } else {
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	if (hasLocation(gn->trm) && termLoc(gn->trm) == (loc & 0xFFFFFFFE))
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
    char *refName = "REF1";
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

    leftTag = termTag(get(port(1, termLoc(tree))));
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

    leftTag = termTag(get(port(1, termLoc(tree))));
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
    Term branch = get(port(1, termLoc(tree)));
    Tag bt = termTag(branch);
    if (t == LAZ && bt == DUP) {
      fprintf(dotFile, "x%d_%x:s -- x%d_%x:nw\n", graphNum, leftBranch, graphNum, nodeNum);
    } else if (t != DUP) {
      graphLink(graphNum, nodeNum, 1, branch, leftBranch);
    }
  }

  if (rightBranch != 65536) {
    Term branch = get(port(2, termLoc(tree)));
    Tag bt = termTag(branch);
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
  Tag t = termTag(tree);
  Location treeLoc = termLoc(tree);
  char *branchPort = pt == 1 ? "sw" : "se";
  Location loc = port(pt, treeLoc);
  Term branch = get(loc);
  while (termTag(branch) == VAR) {
    Term val = get(termLoc(branch));
    if (val != SUB) {
      branch = val;
    } else {
      // BOOM("Draw an edge here");
      return;
    }
  }

  if (branch == SUB) {
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      Term left = get(port(1, gn->node));
      Term right = get(port(2, gn->node));
      if (termTag(left) == VAR && termLoc(left) == treeLoc) {
	fprintf(dotFile, "x%d_%x:sw -- x%d_%x:sw\n", graphNum, gn->node, graphNum, treeLoc);
	break;
      } else if (termTag(right) == VAR && termLoc(right) == treeLoc) {
	fprintf(dotFile, "x%d_%x:se -- x%d_%x:sw\n", graphNum, gn->node, graphNum, treeLoc);
	break;
      }
    }
    return;
  }

  unsigned branchNode = 65536;
  for (unsigned i = 0; i < nodeCount; i++) {
    graphNode *gn = &nodeStack[i];
    if (gn->trm == branch) {
      branchNode = gn->node;
      break;
    }
  }
  if (branchNode == 65536)
    branchNode = graphSubDown(graphNum, loc, branch);

  Tag bt = termTag(branch);
  Lab l = term_lab(tree);

  if (pt == 2 && (t == APP || t == OPX || t == OPY) && bt == LAZ) {
    fprintf(dotFile, "x%d_%x:s -- x%d_%x:se\n", graphNum, branchNode, graphNum, nodeNum);
  } else {
    if (branchNode != 65536 ) {
      if (t == DUP && (bt != LAZ || get(port(1, termLoc(branch))) != tree)) {
	fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		graphNum, nodeNum, branchPort, graphNum, branchNode);
      } else if (t == DUP && (bt != LAZ || get(port(1, termLoc(branch))) == tree)) {
	return;
      } else {
	if (bt == VAR) {
	  Location branchLoc = termLoc(branch);
	  if (termTag(get(branchLoc)) == SUB || termTag(get(branchLoc)) == LAZ) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (branchLoc & 0xFFFFFFFE),
		    (branchLoc & 1) ? "se" : "sw");
	  } else if (termTag(get(branchLoc)) == LAZ &&
		     termTag(get(port(1, termLoc(get(branchLoc))))) == DUP) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (branchLoc & 0xFFFFFFFE),
		    (branchLoc & 1) ? "se" : "sw");
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

// graph the node and the tree under it, if needed. Return the node number
unsigned graphSubDown(unsigned graphNum, unsigned nodeNum, Term tree) {
  char xLbl[100];
  Tag t = termTag(tree);

  if (tree == SUB) {
    return 65536;
  } else if (hasLocation(tree)) {
    unsigned treeNode = termLoc(tree) & 0xFFFFFFFE;
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      if (gn->node == treeNode) {
	if (nodeNum == 65536) {
	  while (termTag(tree) == VAR) {
	    tree = get(termLoc(tree));
	    treeNode = termLoc(tree) & 0xFFFFFFFE;
	  }
	  fprintf(dotFile, "x%d_0 [label=\"\", shape=plaintext, height=0, width=0, peripheries=0]\n",
		  graphNum);
	  fprintf(dotFile, "{rank=min; x%d_0;}\n", graphNum);
	  fprintf(dotFile, "x%d_0:s -- x%d_%x:n\n", graphNum, graphNum, treeNode);
	  return 0;
	}
	return gn->node;
      }
    }
    nodeNum = treeNode;
  } else {
    nodeNum = otherNodes++;
  }

  Lab lab = term_lab(tree);
  switch(t) {
  case VAR: {
    Location loc = termLoc(tree);
    Term trm = get(loc);
    if (trm == SUB) {
      return 65536;
    } else if (termTag(trm) != LAZ || termTag(get(port(1, termLoc(trm)))) != DUP) {
      return graphSubDown(graphNum, nodeNum, trm);
    } else {
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	if (hasLocation(gn->trm) && termLoc(gn->trm) == (loc & 0xFFFFFFFE))
	  return gn->node;
      }
      return graphSubDown(graphNum, nodeNum, trm);
    }
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
    else if (((Value *)tree)->type == StringBufferType) {
      int len = (int)((String *)tree)->len;
      len = len > 10 ? 10 : len;
      fprintf(dotFile, "x%d_%x [label=\"'%-.*s'\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n",
	      graphNum, nodeNum, len, ((String *)tree)->buffer);
    } else if (((Value *)tree)->type == SubStringType) {
      ReifiedVal *ss = (ReifiedVal *)tree;
      String *parent = (String *)ss->impls[0];
      long start = get_i60(ss->impls[1]);
      int len = (int)get_i60(ss->impls[2]);
      len = len > 10 ? 10 : len;
      fprintf(dotFile, "x%d_%x [label=\"'%-.*s'\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n",
	      graphNum, nodeNum, len, &parent->buffer[start]);
    } else
      fprintf(dotFile, noOutline, graphNum, nodeNum, nodeLabels[t]);
    break;

  case REF: {
    char *refName = "REF2";
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
  case DUP: 
  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP: {
    Lab lab = term_lab(tree);
    if (t == DUP || t == SUP || t == LAM) {
      if (lab == 0 || strlen(dupLabels[lab]) == 0)
	snprintf(xLbl, 95, "%x:\n%d", nodeNum, lab);
      else
	snprintf(xLbl, 95, "%x:\n%s", nodeNum, dupLabels[lab]);
    } else
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
      Location dLoc = termLoc(tree);
      Term b1 = get(port(1, dLoc));
      Tag bt1 = termTag(b1);
      if (bt1 != LAZ || tree == get(port(1, termLoc(b1)))) {
	downBranch(tree, 1, graphNum, nodeNum);
      }

      Term b2 = get(port(2, dLoc));
      Tag bt2 = termTag(b2);
      if (bt2 != LAZ || tree == get(port(2, termLoc(b2)))) {
	downBranch(tree, 2, graphNum, nodeNum);
      }
    } else {
      downBranch(tree, 1, graphNum, nodeNum);
      downBranch(tree, 2, graphNum, nodeNum);
    }
  }
    break;
    
  default: {
    fprintf(dotFile,   "x%d_%x [label=\"%s\", shape=plaintext]\n",
	    graphNum, nodeNum, tag_to_str(t));
  }
    break;
    
  }
  return nodeNum;
}

unsigned graphDown(char *title, Term root, unsigned currNodeCount, unsigned graphNum) {
  char xLbl[100];
  nodeCount = currNodeCount;
  fprintf(dotFile, "subgraph cluster%d {\ngraph [color=none, label=\"%s\"]\n", subGraphs++, title);

  if (currNodeCount == 0)
    otherNodes = RNOD_END;

  //*
  if (termTag(root) == LAM) {
    unsigned nodeNum = otherNodes++;
    unsigned rootNode = termLoc(root);
    Lab rootLab = term_lab(root);
    if (rootLab == 0 || strlen(dupLabels[rootLab]) == 0)
      snprintf(xLbl, 95, "%x:\n%d", rootNode, rootLab);
    else
      snprintf(xLbl, 95, "%x:\n%s", rootNode, dupLabels[rootLab]);
    fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "L", 0, xLbl);
    fprintf(dotFile, "{rank=min; x%d_%x;}\n", graphNum, nodeNum);

    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = root;
    gn->node = rootNode;

    Term left = get(port(1, rootNode));
    if (termTag(left) != SUB) {
      unsigned leftNode = graphSubDown(graphNum, 65536, left);
      fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, leftNode);
    } else {
      fprintf(dotFile, "x%d_%x [label=\"\", shape=plaintext, height=0, width=0, peripheries=0]\n",
	      graphNum, rootNode);
      fprintf(dotFile, "{rank=max; x%d_%x;}\n", graphNum, rootNode);

      char foundVar = 0;
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	Term left = get(port(1, gn->node));
	Term right = get(port(2, gn->node));
	if (termTag(left) == VAR && termLoc(left) == rootNode) {
	  fprintf(dotFile, "x%d_%x:sw -- x%d_%x:sw\n", graphNum, gn->node, graphNum, nodeNum);
	  foundVar = 1;
	  break;
	} else if (termTag(right) == VAR && termLoc(right) == rootNode) {
	  fprintf(dotFile, "x%d_%x:se -- x%d_%x:sw\n", graphNum, gn->node, graphNum, nodeNum);
	  foundVar = 1;
	  break;
	}
      }
      if (!foundVar) {
	fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, rootNode);
      }
    }

    downBranch(root, 2,graphNum, nodeNum);
    fprintf(dotFile, "}\n");
    return nodeNum;
  } else
  // */
  if (root == sideEffects) {
    fprintf(dotFile, "x%d_SE [label=\"SE\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n", graphNum);
    fprintf(dotFile, "}\n");
    return 65536;
  } else {
    unsigned rootNode = graphSubDown(graphNum, 65536, root);
    fprintf(dotFile, "}\n");
    return rootNode;
  }
}

void graphFn(Term ref, Term args) {
  NativeArgs argsStruct = {0, {}};
  args = strictArgs(ref, args, 1, &argsStruct);
  if (argsStruct.count != 1) {
    return;
  }

  args = take(port(2, termLoc(args))); 
  Term arg = take(port(1, termLoc(args))); 

  if (termTag(arg) == VAR) {
    Term val = get(termLoc(arg));
    switch(termTag(val)) {
    case LAZ:
      swap(termLoc(arg), SUB);
      forceLazy(val);
	
    case SUB:
      // add the remaining args to argsStruct
      argsStruct.args[argsStruct.count++] = args;

      // create a chain of APP terms from argsStruct
      Term newArgs = argsNet(&argsStruct);

      // put 'arg' back in it's place
      swap(port(1, termLoc(args)), arg);

      // make a deferred redex to retry the APP/REF pair when the value becomes available
      Term retry = pair_make(SUB, 5, newArgs, ref);

      // and put it in the location 'arg' points to
      Term newArg = swap(termLoc(arg), retry);
      if (newArg != SUB) {
	// someone slipped the needed arg in since we last looked
	swap(termLoc(arg), newArg);
	freePair(termLoc(retry));

	// so retry the original APP/REF redex
	pushRedex(newArgs, ref);
      }
      break;

    default:
      print_raw_term(val);
      printf("\n");
      BOOM("nativeArgs");
      break;
    }
  } else {
    String *s = (String *)argsStruct.args[0];
    char cap[200];
    sprintf(cap, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    graphDown(cap, arg, 0, subGraphs++);
    move(port(2, termLoc(args)), arg);
  }
}

void intCond(Term ref, Term args) {
  NativeArgs argsStruct = {0, {}};
  args = strictArgs(ref, args, 1, &argsStruct);
  if (argsStruct.count != 1) {
    return;
  }

  args = take(port(2, termLoc(args))); 
  Term trueBranch = take(port(1, termLoc(args))); 

  if (termTag(trueBranch) == VAR) {
    varArg(trueBranch, ref, args, &argsStruct);
    return;
  }

  args = take(port(2, termLoc(args))); 
  Term falseBranch = take(port(1, termLoc(args))); 

  if (termTag(falseBranch) == VAR) {
    varArg(falseBranch, ref, args, &argsStruct);
    return;
  }

  long x = get_i60(argsStruct.args[0]);
  if (x == 0) {
    interact(ERA, trueBranch);
    move(port(2, termLoc(args)), falseBranch);
  } else {
    interact(ERA, falseBranch);
    move(port(2, termLoc(args)), trueBranch);
  }
}
