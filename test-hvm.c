#include <stdio.h>
#include "new.h"
#include "graph.h"

unsigned refsCount = 0;
refMap refNames[0];

void test_app_lam(void) {
  char msg[100];

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }

  Term lam = makePair(LAM, 0, SUB, newI60(7));
  Term app = makePair(APP, 0, newI60(7), SUB);

  interact(app, lam);

  // Take the rewired results
  Term result1 = take(portLoc(2, app));
  Term result2 = take(portLoc(1, lam));

  if (termTag(result1) != I60 || getI60(result1) != 7) {
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

void test_move_era(void) {
  char msg[100];

  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, newI60(7), ERA);

  interact(app, lam);

  // move(APP port 2, NUL) encounters ERA at port 2
  // swap frees the ERA location and calls interact(ERA, NUL)
  // Then move(LAM port 1, I60(7)) wires the argument into LAM
  // Verify the successful wiring
  Term lam_port1 = take(portLoc(1, lam));
  if (termTag(lam_port1) != I60 || getI60(lam_port1) != 7) {
    sprintf(msg, "LAM port 1 should be I60(7), got tag %s", tagStr(termTag(lam_port1)));
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
void test_take_var_chain(void) {
  char msg[100];

  // Create a separate LAM whose port 2 holds I60(7) — this is the VAR target
  Term target = makePair(LAM, 0, SUB, newI60(7));
  Location targetLoc = portLoc(2, target);

  // Create the LAM for the interaction
  Term lam = makePair(LAM, 0, SUB, NUL);

  // Create APP with VAR pointing to target's port 2
  Term varTerm = newTerm(VAR, 0, targetLoc);
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
}

// Test take with LAZ — returns VAR, doesn't free
void test_take_laz(void) {
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
}

// Test take with SUB — returns VAR, doesn't free
void test_take_sub(void) {
  char msg[100];

  // Create a separate LAM holding SUB in port 1
  Term subTarget = makePair(LAM, 0, SUB, NUL);
  Location subLoc = portLoc(1, subTarget);

  // APP with VAR pointing to SUB location
  Term varTerm = newTerm(VAR, 0, subLoc);
  Term lam = makePair(LAM, 0, SUB, NUL);
  Term app = makePair(APP, 0, varTerm, ERA);

  interact(app, lam);

  // take followed VAR to subLoc, found SUB, returned VAR
  Term result = take(portLoc(1, lam));
  if (termTag(result) != VAR) {
    sprintf(msg, "LAM port 1 should be VAR, got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }
}

// Test cascading: inner LAM rewired into APP's ERA port triggers interact
void test_cascading(void) {
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
void test_move_nul(void) {
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

void test_era_both(void) {
  char msg[100];

  // Both negative ports are ERA — both moves trigger interact(ERA, leaf)
  Term lam = makePair(LAM, 0, ERA, newI60(7));
  Term app = makePair(APP, 0, newI60(42), ERA);

  interact(app, lam);

  // Both ERA locations freed by swap, both pairs completed by take
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

int main(int argc, char *argv[]) {
  hvmInit(1024);

  test_app_lam();
  test_move_era();
  test_era_both();
  test_take_var_chain();
  test_take_laz();
  test_take_sub();
  test_cascading();
  test_move_nul();

  hvmFree();
  return 0;
}
