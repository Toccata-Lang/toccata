#include <stdio.h>
#include "new.h"
#include "graph.h"

unsigned refsCount = 0;
refMap refNames[0];

void testAppLam(void) {
  char msg[100];

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }

  Term lam = makePair(LAM, 0, SUB, newI60(8));
  Term app = makePair(APP, 0, newI60(7), SUB);

  interact(app, lam);

  // Take the rewired results
  Term result1 = take(portLoc(2, app));
  Term result2 = take(portLoc(1, lam));

  if (termTag(result1) != I60 || getI60(result1) != 8) {
    sprintf(msg, "result1 should be I60(7), got tag %s", tagStr(termTag(result1)));
    BOOM(msg);
  }
  if (termTag(result2) != I60 || getI60(result2) != 7) {
    sprintf(msg, "result2 should be I60(7), got tag %s", tagStr(termTag(result2)));
    BOOM(msg);
  }

  // All nodes should be freed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testMoveEra(void) {
  char msg[100];

  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, newI60(7), ERA);

  interact(app, lam);

  // move(APP port 2, NUL) encounters ERA at port 2
  // swap frees the ERA location and calls interact(ERA, NUL)
  // Then move(LAM port 1, I60(7)) wires the argument into LAM
  // Verify the successful wiring
  Term lamPort1 = take(portLoc(1, lam));
  if (termTag(lamPort1) != I60 || getI60(lamPort1) != 7) {
    sprintf(msg, "LAM port 1 should be I60(7), got tag %s", tagStr(termTag(lamPort1)));
    BOOM(msg);
  }

  // All nodes freed: ERA location freed by swap, APP pair freed by take+swap,
  // LAM pair freed by take (completes the pair with B+1 already freed)
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test take following VAR chains through APP/LAM
void testTakeVarChain(void) {
  char msg[100];

  // Create a separate LAM whose port 2 holds I60(7) — this is the VAR target
  Term target = makePair(LAM, 0, SUB, newI60(7));
  Location targetLoc = portLoc(2, target);

  // Create the LAM for the interaction
  Term lam = makePair(LAM, 0, SUB, NUL);

  // Create APP with VAR pointing to target's port 2
  Term varTerm = newTerm(VAR, 0, targetLoc);
  varTerm = newTerm(VAR, 0, termLoc(varTerm));
  Term app = makePair(APP, 0, varTerm, ERA);

  interact(app, lam);

  // take(APP port 1) followed VAR to targetLoc, got I60(7)
  // move placed I60(7) into LAM port 1
  Term result = take(portLoc(1, lam));
  if (termTag(result) != I60 || getI60(result) != 7) {
    sprintf(msg, "LAM port 1 should be I60(7), got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }
  // APP pair and LAM pair freed, but target pair still has SUB in port 1
  if (glblAlloced != 1) {
    sprintf(msg, "glblAlloced should be 1, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }

  // Clean up target pair
  take(portLoc(2, target));
  freePair(termLoc(target));
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0 after cleanup, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test take with LAZ — returns VAR, doesn't free
void testTakeLaz(void) {
  char msg[100];

  // Create a LAZ term to put in APP port 1
  Term laz = makePair(LAZ, 0, SUB, newI60(7));

  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, laz, ERA);

  interact(app, lam);

  // take(APP port 1) found LAZ, returned VAR
  Term result = take(portLoc(1, lam));
  if (termTag(result) != VAR) {
    sprintf(msg, "LAM port 1 should be VAR, got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }

  // NOTE: Buffer is not clean after this test. 'take' on LAZ returns a VAR
  // without freeing the location (by design — LAZ nodes are shared). The
  // locations left behind will be cleaned up when ERA/VAR and ERA/LAZ
  // interactions are implemented.
}

// Test take with SUB — returns VAR, doesn't free
void testTakeSub(void) {
  char msg[100];

  // Create a separate LAM holding SUB in port 1
  Term subTarget = makePair(LAM, 0, SUB, NUL);
  Location subLoc = portLoc(1, subTarget);

  // APP with VAR pointing to SUB location
  Term varTerm = newTerm(VAR, 0, subLoc);
  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, varTerm, ERA);

  graphDown("app", app, 0, subGraphs++);
  graphDown("lam", lam, nodeCount, subGraphs++);
  interact(app, lam);
  pb();

  // take followed VAR to subLoc, found SUB, returned VAR
  Term result = take(portLoc(1, lam));
  if (termTag(result) != VAR) {
    sprintf(msg, "LAM port 1 should be VAR, got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }

  // Clean up: SUB locations aren't freed by take (returns VAR without freeing)
  freeLoc(portLoc(2, subTarget));  // free NUL at subTarget port 2
  freeLoc(subLoc);                  // free SUB at subTarget port 1 → pair freed
  freeLoc(portLoc(1, lam));         // free SUB at lam port 1 → pair freed
}

// Test cascading: inner LAM rewired into APP's ERA port triggers interact
void testCascading(void) {
  char msg[100];

  // Inner LAM: port1=SUB, port2=NUL
  Term innerLam = makePair(LAM, 0, SUB, NUL);

  // Outer LAM: port1=SUB, port2=innerLam
  Term outerLam = makePair(LAM, 0, SUB, innerLam);

  // APP: port1=I60(7), port2=ERA
  Term app = makePair(APP, 0, newI60(7), ERA);

  interact(app, outerLam);

  // After APP/LAM:
  // - take(APP port 1) gets I60(7)
  // - take(outerLAM port 2) gets innerLam
  // - move(APP port 2, innerLam) → ERA triggers interact(ERA, innerLam)
  // - move(outerLAM port 1, I60(7)) → SUB, no special handling
  // Result: outerLAM port 1 = I60(7)

  Term result = take(portLoc(1, outerLam));
  if (termTag(result) != I60 || getI60(result) != 7) {
    sprintf(msg, "outerLAM port 1 should be I60(7), got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }
}

// Test move with NUL as the positive term
// move(ERA, NUL) triggers interact(ERA, NUL) via swap's ERA handling
void testMoveNul(void) {
  char msg[100];

  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, newI60(7), ERA);

  interact(app, lam);

  Term result = take(portLoc(1, lam));
  if (termTag(result) != I60 || getI60(result) != 7) {
    sprintf(msg, "LAM port 1 should be I60(7), got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }
}

void testEraBoth(void) {
  char msg[100];

  // Both negative ports are ERA — both moves trigger interact(ERA, leaf)
  Term lam = makePair(LAM, 0, ERA, newI60(7));
  Term app = makePair(APP, 0, newI60(8), ERA);

  interact(app, lam);

  // Both ERA locations freed by swap, both pairs completed by take
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

int main(int argc, char *argv[]) {
  hvmInit(1024);

  testAppLam();
  testMoveEra();
  testEraBoth();
  testTakeVarChain();
  testTakeLaz();
  testTakeSub();
  testCascading();
  testMoveNul();

  hvmFree();
  return 0;
}
