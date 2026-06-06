#include <string.h>
#include <stdio.h>
#include "new.h"
#include "runtime3.h"

int threadCount = 1;
pthread_t threads[2050];
a64 waiting;
a64 glblAlloced;

// Mutex for thread-safe redex operations
pthread_mutex_t redexMutex;

// Condition variable for signaling when redex is available
pthread_cond_t redexCond;

unsigned nodeCount = 0;
a64 rdxCount;
FILE *dotFile;

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
  pthread_mutex_lock(&redexMutex);
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
	  tagStr(termTag(neg)), tagStr(termTag(pos)),
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
#endif
  return (Location)(term >> (TAG_SIZE + LAB_SIZE));
}

Location portLoc(u64 n, Term trm) {
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
  printRawTerm(neg);
  fprintf(stderr, "  ");
  printRawTerm(pos);
  fprintf(stderr, "\n");
  // */
#ifdef SAFETY
  if (neg == 0 && pos == 0)
    // shutdown the threads
    neg = 0;
  else if (isPositive(neg) || isNegative(pos)) {
    BOOM("bad redex");
    // } else if (interactions[termTag(neg)][termTag(pos)] == &badrdx) {
    // BOOM("bad redex");
  }
#endif

  pairs.rdxs[pairs.count][0] = neg;
  pairs.rdxs[pairs.count++][1] = pos;
  if (pairs.count >= LOCAL_PAIRS_SIZE) {
    BOOM("too many redexes");
  }
}

// Pop a redex (pair of terms) from the reduction bag
// Returns false if the bag is empty, true otherwise
bool popRedex(Term* neg, Term* pos) {
  if (pairs.count > 0) {
    pairs.count -= 1;
    *neg = pairs.rdxs[pairs.count][0];
    *pos = pairs.rdxs[pairs.count][1];
    return true;
  }
  return false;
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

// Allocate a pair from the free list - O(1)
// By popping a value from the free stack
Location allocPair(void) {
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
      loc = atomic_fetch_add_explicit(&buffEnd, 2, memory_order_relaxed);
      // printf("new pair: %d\n", loc);
      // Check if we have space in the buffer
      if (loc >= buffSize) {
	fprintf(stderr, "Error: Not enough space to allocate pair. buffEnd=%u, buffSize=%lu\n",
		loc, buffSize);
	abort();
      }
      break;

    default: {
      // Get the next free pair location
      Term next = get(loc);
      freeList = (Location)(next >> (TAG_SIZE + LAB_SIZE));
    }
      break;
    }
  } while (loc == LOCK_FREE_LIST);
  // printf("alloc: %x\n", loc);
  return (Location)loc;
}

void freeLoc(Location loc) {
  if (get(loc) == VOID) return;
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

int findCycle(Term tree, Location tgtLoc) {
#ifndef CHECK_MEM_LEAK
  BOOM("Not thread safe");
#endif

  if (!hasLocation(tree) || findCycleNode(termLoc(tree) & 0xFFFFFFFE)) {
    return 0;
  }

  Tag t = termTag(tree);
  switch(t) {
  case VAR: {
    Location loc = termLoc(tree);
    if (loc == tgtLoc) {
      return 1;
    } else {
      return findCycle(get(loc), tgtLoc);
    }
    // */
  }
    break;

  case SUB:
    if (termLab(tree) == 0)
      break;
    // else fall through

  case SUP:
  case DUP: 
  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP: {
    if (t == LAZ && termLoc(tree) == tgtLoc)
      return 1;

    cycleNode *cn = &cycleNodes[cycleNodeCount++];
    if (cycleNodeCount > 999)
      BOOM("cycleNodeCount!");
    cn->loc = termLoc(tree);
    cn->trm = tree;

    Location loc = portLoc(1, tree);
    Term branch = get(loc);
    if (findCycle(branch, tgtLoc))
      return 1;

    loc = portLoc(2, tree);
    branch = get(loc);
    if (findCycle(branch, tgtLoc))
      return 1;
  }
    break;
  }
  return 0;
}

int isCycle(Term tree, Location tgtLoc) {
  // TODO: perf optimization — replace linear findCycleNode scan with
  // a hash set for O(1) visited lookups. Each isCycle call traverses
  // a completely different subtree, so old entries are dead weight.
  // A hash set would eliminate the O(n²) worst case.
  cycleNodeCount = 0;
  return findCycle(tree, tgtLoc);
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
      Term neg = get(portLoc(1, result));
      Term pos = get(portLoc(2, result));
      pushRedex(neg, pos);
      freePair(termLoc(result));
      result = SUB;
    }
    break;

  case ERA:
    if (termTag(term) == ERA) {
      // Both old and new are ERA — just free, no interact needed
      freeLoc(loc);
      break;
    }
    freeLoc(loc);
    interact(result, term);
    break;
  }
  return result;
}

void forceLazy(Term z) {
  if (termTag(z) != LAZ)
    return;

  // 'z' is a LAZ term
  Term neg = take(portLoc(1, z));
  Term pos = take(portLoc(2, z));

  if (termTag(neg) == DUP) {
    Location negLoc = termLoc(neg);

    // this is a lazy DUP, which ever port points to itself
    // gets replaced with SUB
    Term curr = get(portLoc(1, neg));
    if (curr == z)
      swap(portLoc(1, neg), SUB);
    curr = get(portLoc(2, neg));
    if (curr == z)
      swap(portLoc(2, neg), SUB);

    if (termTag(pos) == VAR) {
      Location posLoc = termLoc(pos);
      Term lz = swap(posLoc, makePair(SUB, 6, neg, pos));
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
  if (isNegative(pos)) {
    char s[50];
    sprintf(s,"trying to move a negative to location %.3x: %p", negLoc, (void *)neg);
    BOOM(s);
  }
  if (isPositive(neg)) {
    char s[50];
    printTerm("moved pos", pos);
    printTerm("pos at neg", neg);
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
    printTerm("moveDuped neg", neg);
    printTerm("moveDuped pos", pos);
    pb();
    char s[50];
    sprintf(s, "unhandled kind of lazy %s", tagStr(negTag));
    BOOM(s);
  }
    break;
  }
}

// Check if a term is positive
bool isPositive(Term term) {
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
bool isNegative(Term term) {
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

// Create a new pair with given tag, label, and terms
Term makePair(Tag tag, Lab lab, Term fst, Term snd) {
#ifdef SAFETY
  // Check port polarities based on pair type
  switch (tag) {
  case SUB:
  case LAM:
  case LAZ:
    // Port 1 must be negative
    if (!isNegative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tagStr(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    // Port 2 must be positive
    if (!isPositive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tagStr(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    break;

  case OPX:
  case OPY:
  case APP:
    // Port 1 must be positive
    if (!isPositive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tagStr(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    // Port 2 must be negative
    if (!isNegative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tagStr(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    break;

  case DUP:
    // Port 1 must be negative
    if (!isNegative(fst)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 1\n", tagStr(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    // Port 2 must be negative
    if (!isNegative(snd)) {
      fprintf(stderr, "Error: %s pair requires negative term in port 2\n", tagStr(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    break;

  case SUP:
    // Port 1 must be positive
    if (!isPositive(fst)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 1\n", tagStr(tag));
      fprintf(stderr, "  Port 1 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    // Port 2 must be positive
    if (!isPositive(snd)) {
      fprintf(stderr, "Error: %s pair requires positive term in port 2\n", tagStr(tag));
      fprintf(stderr, "  Port 2 term tag: %s\n", tagStr(termTag(snd)));
      abort();
    }
    break;

  default:
    fprintf(stderr, "Error: makePair called with invalid tag: %s (%d)\n",
	    tagStr(tag), tag);
    abort();
  }
#endif

  // Get a pair from the free list or by extending buffEnd
  Location loc = allocPair();

#ifdef SAFETY
  if (loc & 0x1)
    BOOM("Bad allocPair return");
#endif

  // Store terms in their respective ports
#ifdef NON_ATOMIC
  nodeBuff[loc] = fst;
  nodeBuff[loc + 1] = snd;
#else
  atomic_store_explicit(&nodeBuff[loc], fst, memory_order_relaxed);
  atomic_store_explicit(&nodeBuff[loc + 1], snd, memory_order_relaxed);
#endif

  Term new_pair = newTerm(tag, lab, loc);
  /*
  printf("new pair at line %u: %s %.3x %p %p\n", line, tagStr(tag), loc,
	 (void *)get(port(1, loc)),
	 (void *)get(port(2, loc)));
  // */
  return new_pair;
}

// Create a REF term with a specific interaction function
Term ref_make(interactionFn fn) {
  // Store the function pointer in the term, with the REF tag
  return ((Term)fn & ~0xF) | REF;
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

Term argsNet(NativeArgs *args) {
  Term tail;
  if(args->count < 1)
    BOOM("argsNet");
  else
    tail = args->args[args->count - 1];

  for (int i = args->count - 2; i >= 0; i--) {
    tail = makePair(APP, 0, args->args[i], tail);
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
    swap(portLoc(1, args), trm);

    // make a deferred redex to retry the APP/REF pair when the value becomes available
    Term retry = makePair(SUB, 5, newArgs, ref);

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
    printRawTerm(val);
    printf("\n");
    BOOM("nativeArgs");
    break;
  }
}

Term termVal(Term val) {
  // ensure a Term is a valid native value
  unsigned type = ((Value *)val)->type;
  if (val & VAL_MASK) {
    fprintf(stderr, "HVM error in %s at line: %d\n", __FILE__, __LINE__);
    fprintf(stderr, "val: %p\n", (void *)val);
    abort();
  }
  return val;
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
  Location resultLoc = portLoc(2, args);
  Tag argsTag = termTag(args);
  if (argsTag == APP || argsTag == OPY) {
    // if 'args' is an APP term
    Location argLoc = portLoc(1, args);
    Term arg = take(argLoc);
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
	return strictArgs(ref, take(resultLoc), expected - 1, argsStruct);
      else
	return args;
      break;

    case LAM: {
      TermVal *tv = malloc_term();
      tv->trmLoc = allocPair();
      swap(tv->trmLoc, arg);

      // add it to argsStruct
      argsStruct->args[argsStruct->count++] = (Term)tv;
      if (expected > 1)
	// need to get more strict args
	return strictArgs(ref, take(resultLoc), expected - 1, argsStruct);
      else
	return args;
    }
      break;

    case NUL:
      move(resultLoc, NUL);
      for (int i = 0; i < argsStruct->count; i++)
	dec_and_free(argsStruct->args[i], 1);
      break;

    case SUP:
      for(int i = 0; i < argsStruct->count; i++)
	incRef(argsStruct->args[i], 1);
      Term s1 = take(argLoc);
      Term s2 = take(portLoc(2, arg));
      Lab supLabel = termLab(arg);
      int argsCount = argsStruct->count;
      argsStruct->count += 1;

      Term tail1 = makePair(APP, 0, s1, SUB);
      Location tailLoc1 = portLoc(2, tail1);
      argsStruct->args[argsCount] = tail1;
      swap(tailLoc1, makePair(LAZ, 0, argsNet(argsStruct), ref));

      Term tail2 = makePair(APP, 0, s2, SUB);
      Location tailLoc2 = portLoc(2, tail2);
      argsStruct->args[argsCount] = tail2;
      swap(tailLoc2, makePair(LAZ, 0, argsNet(argsStruct), ref));

      Term newSup = makePair(SUP, supLabel,
			      newTerm(VAR, 0, tailLoc1),
			      newTerm(VAR, 0, tailLoc2));
      move(resultLoc, newSup);
      break;

    case VAR:
      varArg(arg, ref, args, argsStruct);
      break;

    case LAZ:
    default: {
      char msg[100];
      sprintf(msg, "unhandled tag %s (0x%x) line: %d\n", tagStr(termTag(arg)),
	     termTag(arg), __LINE__);
      BOOM(msg);
    }
      break;
    }
    argsStruct->count = -1;
    return 0;
  } else {
    // not using BOOM because I want to use printTerm
    fprintf(stderr, "strictArgs expected: %d\n", expected);
    printTerm("strictArgs args", args);
    fprintf(stderr, "unhandled tag %s (0x%x) %p line: %d\n",
	   tagStr(argsTag), argsTag, (void *)args, __LINE__);
    fprintf(dotFile, "}\n");
    fclose(dotFile);
    abort();
    return 0;
  }
}

// ERA/leaf interaction - eraser consumes any leaf term
void eraLeaf(Term neg, Term pos) {
  // Both sides are leaf terms — nothing to do, both are already freed by swap/interact
}

// ERA/LAM interaction: ERA connects to LAM principal
// After: ERA → port1, NUL → port2, body is erased
void eraLam(Term neg, Term pos) {
  Term body = take(portLoc(2, pos));
  interact(ERA, body);
  move(portLoc(1, pos), NUL);
}

// APP/LAM interaction - beta reduction
void appLam(Term neg, Term pos) {
  // Take APP's port 1 (positive argument)
  Term arg = take(portLoc(1, neg));

  // Take LAM's port 2 (positive body)
  Term body = take(portLoc(2, pos));

  // Move body to APP's port 2 location (negative)
  move(portLoc(2, neg), body);

  // Move argument to LAM's port 1 location (negative)
  move(portLoc(1, pos), arg);
}

// Helper: erase a body term, handling ERA specially to avoid interact(ERA, ERA)
static void eraseBody(Term body) {
  if (termTag(body) == ERA) {
    return;
  }
  interact(ERA, body);
}

// NUL interaction for negative triangle nodes (APP, OPX, OPY)
// All have port structure {+ -}, same behavior: port1→NUL, port2→ERA
void negNul(Term neg, Term pos) {
  // get the other possible redex queued up
  move(portLoc(2, neg), NUL);
  // before freeing the entire tree at 'neg'
  interact(ERA, take(portLoc(1, neg)));
}

// SUB/NUL interaction - circle node connects to NUL
// SUB has {- +} polarities — different from APP/OPX/OPY
void subNul(Term neg, Term pos) {
  // neg = SUB, pos = NUL
  // SUB: port1=negative, port2=positive
  if (neg != SUB) {
    // get the other possible redex queued up
    move(portLoc(1, neg), NUL);
    // before freeing the entire tree at 'neg'
    interact(ERA, take(portLoc(2, neg)));
  }
}

// ERA/SUP interaction: ERA connects to SUP principal
// After: two ERAs connect to SUP's ports (x and y erased)
void eraSup(Term neg, Term pos) {
  interact(ERA, newTerm(VAR, 0, portLoc(1, pos)));
  interact(ERA, newTerm(VAR, 0, portLoc(2, pos)));
}

// DUP/NUL interaction: DUP principal connects to NUL
// After: both DUP aux ports connect to NUL (a and b erased)
// DUP/NUL and DUP/NUM: both ports get the positive term
void dupLeaf(Term neg, Term pos) {
  move(portLoc(1, neg), pos);
  move(portLoc(2, neg), pos);
}

// OPX/NUM interaction: OPX principal connects to I60 (#)
// After: # connects to OPY, OPY ports wired: port1=#, port2=b, principal=a
void opyNum(Term neg, Term pos);
void opxNum(Term neg, Term pos) {
  Term arg = swap(portLoc(1, neg), pos);
  Lab op = termLab(neg);
  switch (termTag(arg)) {
  case I60:
    opyNum(newTerm(OPY, op, portLoc(1, neg)), arg);
    break;

  case VAR:
    interact(newTerm(OPY, op, portLoc(1, neg)), arg);
    break;

  default:
    pushRedex(newTerm(OPY, op, portLoc(1, neg)), arg);
  }
  return;
}

// OPY/NUM interaction: OPY principal connects to I60 (#)
// After: b connects to result (#1 op #2)
void opyNum(Term neg, Term pos) {
  NativeArgs arityArgs = {0, {}};
  neg = strictArgs(pos, neg, 1, &arityArgs);
  if (arityArgs.count != 1) {
    return;
  }

  Term x = arityArgs.args[0];
  Tag yType = termTag(pos);
  Location ret = portLoc(2, neg);
  u64 res;
  Lab op = termLab(neg);

#ifdef SAFETY
  switch (termTag(x)) {
  case I60:
  case F60:
    break;

  default: {
    char msg[200];
    sprintf(msg, "wrong value to OPY: %s", tagStr(termTag(x))); 
    BOOM(msg);
  }
    break;
  }
#endif

  switch (yType) {
  case I60: PERFORM_OP(getU64(x), getU64(pos), op, i64); break;
    // case F60: PERFORM_OP(x, y, op, f64); break;
  }

  Term result = newNum(yType, res);
  // If old value is ERA, swap would erase the result.
  // Free the pair first, then store result directly.
  Term old = get(ret);
  if (termTag(old) == ERA) {
    Location evenLoc = ret & 0xFFFFFFFE;
    freePair(evenLoc);  // free pair while ERA is still at ret
    atomic_store_explicit(&nodeBuff[ret], result, memory_order_relaxed);
  } else {
    move(ret, result);
  }
  return;
}

void eraseLazy(Term lazyVar) {
  Term laz;
  switch (termTag(lazyVar)) {
  case VAR:
    laz = swap(termLoc(lazyVar), ERA);
    break;

  case LAZ:
    laz = lazyVar;
    break;

  default:
    BOOM("Trying to erase a non-var/lazy Term");
  }

  Location lazyLoc = termLoc(laz);
  Term negLaz = get(portLoc(1, laz));
  Term posLaz = get(portLoc(2, laz));
  switch (termTag(negLaz)) {
  case DUP: {
    // Two-phase process: capture DUP ports, then process
    Term dup1 = swap(portLoc(1, negLaz), SUB);
    Term dup2 = swap(portLoc(2, negLaz), SUB);
    // One port has ERA (from eraVar), one has LAZ
    if (termTag(dup1) == ERA && termTag(dup2) == LAZ) {
      Term context = get(portLoc(2, dup2));
      freePair(lazyLoc);
      move(portLoc(2, negLaz), context);
      freeLoc(portLoc(1, negLaz));
    } else if (termTag(dup1) == LAZ && termTag(dup2) == ERA) {
      Term context = get(portLoc(2, dup1));
      freePair(lazyLoc);
      move(portLoc(1, negLaz), context);
      freeLoc(portLoc(2, negLaz));
    }
    break;
  }

  case APP:
    interact(negLaz, NUL);
    interact(ERA, posLaz);
    freePair(lazyLoc);
    break;

  default:
    BOOM("eraseLazy: unhandled kind of lazy");
  }
}

void eraVar(Term era, Term var) {
  Term val = take(termLoc(var));
  if (termTag(val) == VAR) {
    Term lz = swap(termLoc(val), ERA);
    if (lz != SUB) {
      eraseLazy(lz);
    }
  } else {
    interact(ERA, val);
  }
  return;
}

// interaction jump table - all entries default to badrdx
interactionFn interactions[16][16] = {
  [0 ... 15] = {[0 ... 15] = &badrdx}
};

void interact(Term neg, Term pos) {
  /*
  if (1) {
    if (termTag(neg) != ERA && termTag(pos) != NUL) {
      if (1) {
	// fprintf(stderr, "%ld: ", graphCount);
	// printRawTerm(neg);
	// fprintf(stderr, " - ");
	// printRawTerm(pos);
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

  // if (termLab(pos) == SUP && termLoc(pos) == 0x15a) {
  // printTerm("NEG", neg);
  // printTerm("POS", pos);
  // }
  // if (termLab(neg) == APP && termLoc(pos) == 0x2c) {
  // printTerm("NEG", neg);
  // printTerm("POS", pos);
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
  /*
  u64 waitingThreads = atomic_load_explicit(&waiting, memory_order_relaxed);
  if (waitingThreads > 0) {
    pthread_mutex_lock(&redexMutex);
    pthread_cond_signal(&redexCon);
    pthread_mutex_unlock(&redexMutex);
  }
  // */

  u64 *res = malloc(sizeof(u64));
  // *res = rdxCount;
  return res;
}

Term makeOp(Lab op, Term x, Term y) {
  Term t = makePair(OPX, op, y, SUB);
  Term ret = newTerm(VAR, 0, portLoc(2, t));
  swap(termLoc(ret), makePair(LAZ, 0, t, x));
  return ret;
}

// For testing/debugging only
void printRawTerm(Term t) {
  if (t == 0) {
    fprintf(stderr, "  FREE   ");
  } else {
    Tag tag = termTag(t);
    Lab lab = termLab(t);
    switch(termTag(t)) {
    case NUL:
    case ERA:
    case I60:
    case F60:
    case SUB:
      fprintf(stderr, "%s %x", tagStr(tag), lab);
      break;

    case VAL:
      fprintf(stderr, "%s %llx", tagStr(tag), t & ~VAL_MASK);
      break;

    case REF: {
      char *refName = "";
      for (unsigned i = 0; i <= refsCount; i++) {
	if (refNames[i].fn == (interactionFn)(t & ~TAG_MASK)) {
	  refName = refNames[i].name;
	  break;
	}
      }
      fprintf(stderr, "%s %llx %s", tagStr(tag), t & ~TAG_MASK, refName);
    }
      break;

      // case REF:
      // printf("REF %s", refName(t));
      // break;

    default:
      fprintf(stderr, "%s %x %.3x", tagStr(tag), lab, termLoc(t));
      break;
    }
  }
}

// Helper to print a term's details
void printTerm(const char* prefix, Term term) {
  fprintf(stderr, "%s:\n", prefix);
  fprintf(stderr, "  Tag: %s (%d, 0x%x)\n", tagStr(termTag(term)), termTag(term), termTag(term));
  Lab lab = termLab(term);
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
    fprintf(stderr, "  Val: %ld", getI60(term));
    break;

  case VAR:
    fprintf(stderr, "  Location: %.3x\n", termLoc(term));
    // If this is a pair, print its contents
    if (termLoc(term) >= 0) {
      Term first = get(portLoc(1, term));
      fprintf(stderr, "  term: ");
      printRawTerm(first);
      fprintf(stderr, "\n");
    }
    break;

  case SUP:
    if (lab == 1 || lab == 2) {
      fprintf(stderr, "  Location: %.3x\n", termLoc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
	Term first = get(portLoc(1, term));
	Term second = get(portLoc(2, term));
	fprintf(stderr, "  Refs: %d\n", (int)first);
	fprintf(stderr, "  Second term: ");
	printRawTerm(second);
	fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "  Location: %.3x\n", termLoc(term));
      fprintf(stderr, "  Label: %.3x\n", lab);
      // If this is a pair, print its contents
      if (termLoc(term) >= 0) {
	Term first = get(portLoc(1, term));
	Term second = get(portLoc(2, term));
	fprintf(stderr, "  First term: ");
	printRawTerm(first);
	fprintf(stderr, "\n");
	fprintf(stderr, "  Second term: ");
	printRawTerm(second);
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
      Term first = get(portLoc(1, term));
      Term second = get(portLoc(2, term));
      fprintf(stderr, "  First term: ");
      printRawTerm(first);
      fprintf(stderr, "\n");
      fprintf(stderr, "  Second term: ");
      printRawTerm(second);
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
void hvmInit(u64 size) {
  srand(time(NULL));

  dotFile = fopen("graphs.dot", "w");
  if (!dotFile) {
    BOOM( "Failed to open graphs.dot\n");
  }
  fprintf(dotFile, "graph grammar {\nranksep=0.1\n");

#ifdef NON_ATOMIC
  nodeBuff = (u64*)calloc(size, sizeof(a64));
#else
  nodeBuff = (a64*)calloc(size, sizeof(a64));
#endif
  if (!nodeBuff) {
    fprintf(stderr, "Failed to allocate memory\n");
    abort();
  }
  buffSize = size;

  interactions[APP][LAM] = &appLam;
  interactions[APP][NUL] = &negNul;
  interactions[OPX][NUL] = &negNul;
  interactions[OPY][NUL] = &negNul;
  interactions[SUB][NUL] = &subNul;
  interactions[ERA][NUL] = &eraLeaf;
  interactions[ERA][I60] = &eraLeaf;
  interactions[ERA][F60] = &eraLeaf;
  interactions[ERA][LAM] = &eraLam;
  // interactions[ERA][VAL] = &eraLeaf;  // TODO: special handling for VAL erasure
  interactions[ERA][SUP] = &eraSup;
  interactions[ERA][VAR] = &eraVar;
  interactions[DUP][NUL] = &dupLeaf;
  interactions[DUP][I60] = &dupLeaf;
  interactions[OPX][I60] = &opxNum;
  interactions[OPY][I60] = &opyNum;

  // Initialize mutex for thread-safe redex operations
  if (pthread_mutex_init(&redexMutex, NULL) != 0) {
    fprintf(stderr, "Failed to initialize mutex\n");
    abort();
  }

  // Initialize condition variable for redex signaling
  if (pthread_cond_init(&redexCond, NULL) != 0) {
    fprintf(stderr, "Failed to initialize condition variable\n");
    abort();
  }
}

// Free allocated memory
void hvmFree(void) {
  if (nodeBuff == NULL) {
    return;
  }

  free(nodeBuff);
  nodeBuff = NULL;

  fprintf(dotFile, "}\n");
  fclose(dotFile);
}

void hvmReset(void) {
  if (nodeBuff == NULL || nodeBuff == NULL) {
    fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvmInit first.\n");
    abort();
  }

  // Clear memory to prevent stale data
  // memset(nodeBuff, 0, buffSize * sizeof(Term));
  memset(nodeBuff, 0, buffSize * sizeof(Term));

  // Reset node index
  atomic_store_explicit(&buffEnd, 0, memory_order_relaxed);;

  // Initialize the free list (initially empty)
  freeList = EMPTY_FREE_LIST;
  atomic_store_explicit(&glblAlloced, 0, memory_order_relaxed);
  atomic_store_explicit(&rdxCount, 0, memory_order_relaxed);
  atomic_store_explicit(&waiting, 0, memory_order_relaxed);
  rdxCount = 0;
}

void checkBuff() {
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
  for (Location i = 0; i < buffEnd; i += 2) {
    Term t1 = buff[i];
    if (termTag(t1) != NUL || buff[i + 1] != 0) {
      leaks++;
    }
  }
  //graphDown("leaked", get(0xaf), 0, subGraphs++);
  if (leaks) {
    fprintf(stderr, "\nLeaked pairs!!\n");
    // printTerm("leaked", get(0x9c));
    pb();
    pr();
      // BOOM("Leak pairs");
  }
}

// For// Print contents of nodeBuff between start and end locations
void printBuff(Location start, Location end) {
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
    if ((termTag(t1) != NUL || termLab(t1) != 0xFF) || buff[i + 1] != 0) {
      fprintf(stderr," %.3x  ", i);
      printRawTerm(t1);
      fprintf(stderr,"  ");
      printRawTerm(buff[i + 1]);
      fprintf(stderr,"\n");
    }
  }
  fprintf(stderr, "\n");
}

void pb() {
  printBuff(0, buffEnd);
}

void pr() {
  fprintf(stderr, "Redexes: %d\n", pairs.count);
  for (int i = 0; i < pairs.count; i++) {
    fprintf(stderr, " %.3x  ", i);
    printRawTerm(pairs.rdxs[i][0]);
    fprintf(stderr, "  ");
    printRawTerm(pairs.rdxs[i][1]);
    fprintf(stderr, "\n");
  }
}

// Print the free list for debugging
void printFreeList(void) {
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
