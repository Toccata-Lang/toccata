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

  // Clean up target pair — free port 1 first so freeLoc triggers freePair
  freeLoc(portLoc(1, target));
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

  interact(app, lam);

  // take followed VAR to subLoc, found SUB, returned VAR
  Term result = get(portLoc(1, lam));
  if (result != newTerm(VAR, 0, portLoc(1, subTarget))) {
    sprintf(msg, "LAM port 1 should be VAR pointing to subTarget\n");
    BOOM(msg);
  }

  // Clean up: SUB locations aren't freed by take (returns VAR without freeing)
  // Note: APP pair already freed by swap handling ERA during interact
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

  // Clean up: innerLam's port 1 still has NUL after take
  // Note: APP pair already freed by swap handling ERA during interact
  take(portLoc(1, innerLam));  // free NUL at innerLam port 2
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
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
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/LAM interaction: ERA → x, NUL → y
// When ERA connects to LAM principal port, LAM's aux ports are rewired:
// port 1 gets ERA, port 2 gets NUL. LAM's body is erased.
void testEraLam(void) {
  char msg[100];
  u64 initialAlloced = glblAlloced;

  // Build LAM: port1=SUB, port2=I60(7)
  Term lam = makePair(LAM, 0, SUB, newI60(7));

  // Trigger ERA/LAM interaction
  interact(ERA, lam);

  // Post-checks: LAM rewired — port1 gets NUL, port2 freed, body erased
  Term port1 = take(portLoc(1, lam));
  Term port2 = take(portLoc(2, lam));

  if (termTag(port1) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (port2 != 0) {
    sprintf(msg, "LAM port 2 should be VOID, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  // LAM pair fully cleaned up — alloced should be back to initial
  if (glblAlloced != initialAlloced) {
    sprintf(msg, "glblAlloced should be %lld, got %lld", (long long)initialAlloced, (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/LAM with NUL in body port
void testEraLamNulBody(void) {
  char msg[100];
  u64 initialAlloced = glblAlloced;

  // Build LAM: port1=SUB, port2=NUL
  Term lam = makePair(LAM, 0, SUB, NUL);

  interact(ERA, lam);

  Term port1 = take(portLoc(1, lam));
  Term port2 = take(portLoc(2, lam));

  if (termTag(port1) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (port2 != 0) {
    sprintf(msg, "LAM port 2 should be VOID, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != initialAlloced) {
    sprintf(msg, "glblAlloced should be %lld, got %lld", (long long)initialAlloced, (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/LAM with LAM in body port (nested LAM erased)
void testEraLamLamBody(void) {
  char msg[100];
  u64 initialAlloced = glblAlloced;

  // Inner LAM: port1=SUB, port2=I60(137)
  Term innerLam = makePair(LAM, 0, SUB, newI60(137));

  // Outer LAM: port1=SUB, port2=innerLam
  Term lam = makePair(LAM, 0, SUB, innerLam);

  interact(ERA, lam);

  if (take(portLoc(1, lam)) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL");
    BOOM(msg);
  }

  if (take(portLoc(1, innerLam)) != NUL) {
    sprintf(msg, "Inner LAM port 1 should be NUL");
    BOOM(msg);
  }

  if (glblAlloced != initialAlloced) {
    sprintf(msg, "glblAlloced should be %lld, got %lld", (long long)initialAlloced, (long long)glblAlloced);
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

// Test cascading: outer APP/outer LAM interaction creates a redex of inner APP/inner LAM
// Test APP/NUL interaction: APP principal connects to NUL
// After: port1 → NUL, port2 → ERA
void testAppNul(void) {
  char msg[100];

  // Build APP: port1=I60(7), port2=ERA
  Term app = makePair(APP, 0, newI60(7), ERA);

  // Trigger APP/NUL interaction
  interact(app, NUL);

  // Post-checks: APP rewired — both ports freed
  // (negNul frees both ports via take + move/swap)

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUL interaction
void testOpxNul(void) {
  char msg[100];

  // Build OPX: port1=I60(7), port2=ERA
  Term opx = makePair(OPX, 0, newI60(7), ERA);

  interact(opx, NUL);

  // OPX rewired — both ports freed by negNul
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPY/NUL interaction
void testOpYNul(void) {
  char msg[100];

  // Build OPY: port1=I60(7), port2=ERA
  Term opy = makePair(OPY, 0, newI60(7), ERA);

  interact(opy, NUL);

  // OPY rewired — both ports freed by negNul
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test SUB/NUL interaction: circle node connects to NUL
// After: port1 → NUL, port2 → ERA
void testSubNul(void) {
  char msg[100];

  // Build SUB: port1=ERA, port2=I60(7)
  Term sub = makePair(SUB, 0, ERA, newI60(7));

  interact(sub, NUL);

  Term port1 = take(portLoc(1, sub));
  Term port2 = take(portLoc(2, sub));

  if (termTag(port1) != NUL) {
    sprintf(msg, "SUB port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != ERA) {
    sprintf(msg, "SUB port 2 should be ERA, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/SUP interaction: ERA connects to SUP principal
// After: two ERAs connect to SUP's ports
void testEraSup(void) {
  char msg[100];

  // Build SUP: port1=I60(7), port2=I60(8)
  Term sup = makePair(SUP, 0, newI60(7), newI60(8));

  interact(ERA, sup);

  Term port1 = take(portLoc(1, sup));
  Term port2 = take(portLoc(2, sup));

  if (termTag(port1) != ERA) {
    sprintf(msg, "SUP port 1 should be ERA, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != ERA) {
    sprintf(msg, "SUP port 2 should be ERA, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/NUL interaction: DUP principal connects to NUL
// After: both DUP aux ports connect to NUL
void testDupNul(void) {
  char msg[100];

  // Build DUP: port1=ERA, port2=ERA
  Term dup = makePair(DUP, 0, ERA, ERA);

  interact(dup, NUL);

  Term port1 = take(portLoc(1, dup));
  Term port2 = take(portLoc(2, dup));

  if (termTag(port1) != NUL) {
    sprintf(msg, "DUP port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != NUL) {
    sprintf(msg, "DUP port 2 should be NUL, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/NUM interaction: DUP connects to I60
// After: both ports get the number
void testDupNum(void) {
  char msg[100];

  // Build DUP: port1=ERA, port2=ERA
  Term dup = makePair(DUP, 0, ERA, ERA);

  interact(dup, newI60(42));

  Term port1 = take(portLoc(1, dup));
  Term port2 = take(portLoc(2, dup));

  if (termTag(port1) != I60 || getI60(port1) != 42) {
    sprintf(msg, "DUP port 1 should be I60(42), got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != I60 || getI60(port2) != 42) {
    sprintf(msg, "DUP port 2 should be I60(42), got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUM interaction: OPX connects to I60
// After: # to port1, OPY to port2
void testOpxNum(void) {
  char msg[100];

  // Build OPX: port1=I60(3), port2=ERA
  Term opx = makePair(OPX, OP_ADD, newI60(3), ERA);

  interact(opx, newI60(5));

  Term port1 = take(portLoc(1, opx));
  Term port2 = take(portLoc(2, opx));

  if (termTag(port1) != I60 || getI60(port1) != 5) {
    sprintf(msg, "OPX port 1 should be I60(5), got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != OPY) {
    sprintf(msg, "OPX port 2 should be OPY, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  // Verify OPY: port1=#, port2=b (b was ERA)
  Term opyPort1 = take(portLoc(1, port2));
  Term opyPort2 = take(portLoc(2, port2));
  if (termTag(opyPort1) != I60 || getI60(opyPort1) != 5) {
    sprintf(msg, "OPY port 1 should be I60(5), got tag %s", tagStr(termTag(opyPort1)));
    BOOM(msg);
  }
  // opyPort2 was ERA (b), take returns ERA
  if (termTag(opyPort2) != ERA) {
    sprintf(msg, "OPY port 2 should be ERA, got tag %s", tagStr(termTag(opyPort2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPY/NUM interaction: OPY connects to I60
// After: result (#1 op #2) in both ports
void testOpYNum(void) {
  char msg[100];

  // Build OPY: port1=I60(3), port2=ERA
  Term opy = makePair(OPY, OP_ADD, newI60(3), ERA);

  interact(opy, newI60(5));

  Term port1 = take(portLoc(1, opy));
  Term port2 = take(portLoc(2, opy));

  if (termTag(port1) != I60 || getI60(port1) != 8) {
    sprintf(msg, "OPY port 1 should be I60(8), got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }
  if (termTag(port2) != I60 || getI60(port2) != 8) {
    sprintf(msg, "OPY port 2 should be I60(8), got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testCascadingRedex(void) {
  char msg[100];
  u64 initialAlloced = glblAlloced;

  // Inner APP: port1=I60(7), port2=SUB
  Term innerApp = makePair(APP, 0, newI60(7), SUB);

  // Inner LAM: port1=SUB, port2=I60(137)
  Term innerLam = makePair(LAM, 0, SUB, newI60(137));

  // Outer APP: port1=NUL, port2=innerApp
  // APP port2 accepts negative terms, so innerApp goes directly there
  Term outerApp = makePair(APP, 0, NUL, innerApp);

  // Outer LAM: port1=SUB, port2=innerLam
  Term outerLam = makePair(LAM, 0, SUB, innerLam);

  interact(outerApp, outerLam);

  // After APP/LAM interaction (appLam):
  // 1. take(APP port1) takes NUL, frees port1 loc, returns NUL
  // 2. take(LAM port2) takes innerLam, frees LAM port2 loc, returns innerLam
  // 3. move(APP port2, innerLam) — old value=innerApp(APP,neg)
  //    swap returns innerApp, move pushes redex(innerApp, innerLam)
  // 4. move(LAM port1, NUL) — old value=SUB, no redex pushed
  //
  // Result: one redex on stack = (innerAPP, innerLAM)

  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex on stack");
  }
  if (pairs.count != 0) {
    sprintf(msg, "pairs.count should be 0 after pop, got %u", pairs.count);
    BOOM(msg);
  }

  // Verify the popped redex terms
  if (termTag(neg) != APP) {
    sprintf(msg, "redex neg should be APP, got tag %s", tagStr(termTag(neg)));
    BOOM(msg);
  }
  if (termTag(pos) != LAM) {
    sprintf(msg, "redex pos should be LAM, got tag %s", tagStr(termTag(pos)));
    BOOM(msg);
  }

  // Interact the redex — another APP/LAM beta reduction
  interact(neg, pos);

  // After second interact:
  // innerApp: port1=VOID, port2=I60(137) (move swapped body in)
  // innerLam: port1=I60(7) (move swapped arg in), port2=VOID
  // outerApp: both ports VOID (pair already freed by move during first interact)
  // outerLam: port1=NUL, port2=VOID
  //
  // Clean up using take — each take frees its location and coalesces into freePair
  // VOID ports are not taken — glblAlloced verifies all pairs are freed
  Term r;
  // innerApp port2: has I60(137), take frees location
  r = take(portLoc(2, innerApp));
  if (termTag(r) != I60 || getI60(r) != 137) {
    sprintf(msg, "innerApp port2 should be I60(137), got tag %s", tagStr(termTag(r)));
    BOOM(msg);
  }
  // innerLam port1: has I60(7), take frees location
  r = take(portLoc(1, innerLam));
  if (termTag(r) != I60 || getI60(r) != 7) {
    sprintf(msg, "innerLam port1 should be I60(7), got tag %s", tagStr(termTag(r)));
    BOOM(msg);
  }
  // outerLam port1: has NUL, take frees location
  r = take(portLoc(1, outerLam));
  if (termTag(r) != NUL) { sprintf(msg, "outerLam port1 should be NUL"); BOOM(msg); }

  if (glblAlloced != initialAlloced) {
    sprintf(msg, "glblAlloced should be %lld, got %lld", (long long)initialAlloced, (long long)glblAlloced);
    BOOM(msg);
  }
}

int main(int argc, char *argv[]) {
  hvmInit(1024);

  // testAppLam();
  // testMoveEra();
  // testEraBoth();
  // testTakeVarChain();
  // // testTakeLaz();
  // testTakeSub();
  // testCascading();
  // testMoveNul();
  // testEraLam();
  // testEraLamNulBody();
  // testEraLamLamBody();
  // testCascadingRedex();
  // testAppNul();
  // testOpxNul();
  testOpYNul();
  // testSubNul();
  // testEraSup();
  // testDupNul();
  // testDupNum();
  // testOpxNum();
  // testOpYNum();

  hvmFree();
  return 0;
}
