#ifndef NEW_H
#define NEW_H

// #define CHECK_MEM_LEAK 1
// #define STATS 1
// #define NON_ATOMIC 1
// #define SAFETY 1
// #define STRICT 1

#define HEIGHT 20
extern unsigned hght;

#include <stdatomic.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

// Type definitions
typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint32_t Location;
typedef uint32_t Lab;
typedef atomic_uint_least64_t a64;

// Bit layout constants
#define TAG_SIZE 4
#define LAB_SIZE 28
#define LOC_SIZE 32
#define TAG_MASK 0xFULL
#define LAB_MASK 0xFFFFFFFULL
#define LOC_MASK 0xFFFFFFFFULL
#define VAL_MASK 0x7ULL
#define VOID 0

// Term is a 64-bit value:
// - Highest 32 bits: Location
// - Next 28 bits: Label
// - Lowest 4 bits: Tag
typedef u64 Term;

// Define empty free list marker
#define EMPTY_FREE_LIST 0xFFFFFFFE
#define LOCK_FREE_LIST 0xFFFFFFFF

#define LOCK_REDEX_STACK 0xFFFFFFFF
extern a64 waiting;

// Global VM state
extern unsigned nodeCount;
extern a64 glblAlloced;
extern a64 rdxCount;
extern int threadCount;
extern pthread_mutex_t redexMutex; // Mutex for thread-safe redex operations
extern pthread_cond_t redexCond; // Condition variable for signaling when redex is available
void printFreeList(void);   // For debugging
void printRawTerm(Term t);
void printTerm(const char* prefix, Term term);
void printBuff(Location start, Location end);
void pb();
void pr();
void checkBuff();
char *refName(Term ref);

// Tags for different term types
#define VAL 0x00 // positive native value
#define VAR 0x01 // positive variable
#define SUB 0x02 // negative possible deferred redex {- +}
#define NUL 0x03 // positive eraser
#define ERA 0x04 // negative eraser
#define LAM 0x05 // positive constructor {- +}
#define APP 0x06 // negative constructor {+ -}
#define REF 0x07 // positive reference
#define VL1 0x08 // positive native value alias
#define SUP 0x09 // positive duplicator {+ +}
#define DUP 0x0a // negative duplicator {- -}
#define OPX 0x0b // negative operation {+ -}
#define OPY 0x0c // negative operation {+ -}
#define I60 0x0d // positive 56 bit int
#define F60 0x0e // positive 56 bit float
#define LAZ 0x0f // positive lazy node {- +}
typedef u32 Tag; // Tag is now just an unsigned integer

// Operators
#define OP_ADD 0x00
#define OP_SUB 0x01
#define OP_MUL 0x02
#define OP_DIV 0x03
#define OP_MOD 0x04
#define OP_EQ  0x05
#define OP_NE  0x06
#define OP_LT  0x07
#define OP_GT  0x08
#define OP_LTE 0x09
#define OP_GTE 0x0A
#define OP_AND 0x0B
#define OP_OR  0x0C
#define OP_XOR 0x0D
#define OP_LSH 0x0E
#define OP_RSH 0x0F


#define MAX_ARGS 15
typedef struct {
  int count;
  Term args[MAX_ARGS + 2];
} NativeArgs;

#define LOCAL_PAIRS_SIZE 16000
typedef struct {
  unsigned count;
  Term rdxs[LOCAL_PAIRS_SIZE][2];
} Pairs;

// creating number terms
#define newI60(x) (((i64)(x) << TAG_SIZE) | I60)
#define getI60(x) (i64)((i64)(x) >> TAG_SIZE)
#define getU64(x) (i64)((u64)(x) >> TAG_SIZE)
#define newNum(type, x) (((u64)(x) << TAG_SIZE) | type)

// Function declarations
void hvmInit(u64 size);
void hvmFree(void);
void hvmReset(void);
void initFreeList(u64 start, u64 end);
Location allocPair(void);
void freePair(Location loc);
Term makePair(Tag tag, Lab lab, Term fst, Term snd);
const char* tagStr(Tag tag);
Term termNew(Tag tag, Lab lab, Location loc);
Term termVal(Term val);
Tag termTag(Term term);
Lab termLab(Term term);
Location termLoc(Term term);
Location portLoc(u64 n, Term trm);
// Check term polarity
bool isPositive(Term term);
bool isNegative(Term term);

// this abuses the compile time functionality
// to create a Port value
#define termNew_(tag, x) (((u64)x + tag))

// Get term at location
Term get(Location loc);
Term take(Location loc);
void freeLoc(Location loc);
void interact(Term neg, Term pos);
void pushRedex(Term neg, Term pos);
bool popRedex(Term* neg, Term* pos);
Term swap(Location loc, Term term);
void move(Location neg_loc, Term pos);
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct);
Term dupeArg(Term arg, Term *dupedArg, unsigned dupLabel);
int decSubRefs(Location sup_loc);
Term makeOp(Lab op, Term x, Term y);

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void *normalize(void *v);
void forceLazy(Term z);
void spawn_threads();
extern pthread_t threads[];

typedef void (*interactionFn)(Term a, Term b);
#define newRef(x) (((u64)x + REF))

extern unsigned refsCount;
typedef struct {
  interactionFn fn;
  char *name;} refMap;
extern refMap refNames[];

// Create a REF term with a specific interaction function
Term refMake(interactionFn fn);

u64 time64();
void *boom(char *msg, char *file, int line);
#define BOOM(msg) boom(msg, __FILE__, __LINE__)
#endif // NEW_H

void intCond(Term ref, Term args);
extern char *dupLabels[];

char hasLocation(Term tree);
int eraseCycle(Term tree, Location tgtLoc);
