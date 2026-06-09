#include <stdio.h>
#include "new.h"
#include "graph.h"

unsigned refsCount = 0;
refMap refNames[0];
char *dupLabels[] = {};

void testAppLam(void) {
  char msg[100];

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

  if (get(termLoc(result)) != laz) {
    sprintf(msg, "Result should point to LAZ %s", tagStr(termTag(get(termLoc(result)))));
    BOOM(msg);
  }

  freeLoc(termLoc(result));
  freePair(termLoc(laz));
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0 after cleanup, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
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
  take(portLoc(2, subTarget));  // free NUL at subTarget port 2
  freeLoc(subLoc);                  // free SUB at subTarget port 1 → pair freed
  freeLoc(portLoc(1, lam));         // free SUB at lam port 1 → pair freed

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0 after cleanup, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
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
  

  // Build LAM: port1=SUB, port2=I60(7)
  Term lam = makePair(LAM, 0, SUB, newI60(7));

  // Trigger ERA/LAM interaction
  interact(ERA, lam);

  // Post-checks: LAM rewired — port1 gets NUL, port2 freed, body erased
  Term port1 = take(portLoc(1, lam));

  if (termTag(port1) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }

  // LAM pair fully cleaned up — alloced should be back to initial
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/LAM with NUL in body port
void testEraLamNulBody(void) {
  char msg[100];

  // Build LAM: port1=SUB, port2=NUL
  Term lam = makePair(LAM, 0, SUB, NUL);

  interact(ERA, lam);

  Term port1 = take(portLoc(1, lam));

  if (termTag(port1) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/LAM with LAM in body port (nested LAM erased)
void testEraLamLamBody(void) {
  char msg[100];

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

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
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

  // Build SUB: port1=ERA, port2=I60(7) — label > 0 so ports are connected
  Term sub = makePair(SUB, 1, ERA, newI60(7));

  interact(sub, NUL);

  // SUB rewired — both ports freed by subNul
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

  // DUP rewired — both ports freed by dupLeaf
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

  interact(dup, newI60(87));

  // DUP rewired — both ports freed by dupLeaf
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUM interaction: OPX connects to I60
// After: OPY/NUM fires, result (# op a) moves to port 2
void testOpxNum(void) {
  char msg[100];
  

  // Build OPX: port1=I60(3) [a], port2=ERA [b]
  Term opx = makePair(OPX, OP_ADD, newI60(3), ERA);

  // Interact with NUM I60(5) [#]
  // OPX/NUM: swaps # into port1, creates OPY, fires OPY/NUM with a
  // OPY/NUM: computes # op a = 5 + 3 = 8, moves result to port2
  interact(opx, newI60(5));

  // After full reduction: port2 holds the result (5 + 3 = 8)
  Term result = take(portLoc(2, opx));
  if (termTag(result) != I60 || getI60(result) != 8) {
    sprintf(msg, "OPX port 2 should be I60(8), got tag %s val %ld",
            tagStr(termTag(result)), (long)getI60(result));
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
  

  // Build OPY: port1=I60(3) [#1], port2=ERA [b]
  Term opy = makePair(OPY, OP_ADD, newI60(3), ERA);

  // Interact with NUM I60(5) [#2]
  // OPY/NUM: computes #1 op #2 = 3 + 5 = 8, result goes to port 2 (b)
  interact(opy, newI60(5));

  // Port 1 was consumed by strictArgs (took #1)
  // Port 2 holds the result (3 + 5 = 8)
  Term port2 = take(portLoc(2, opy));
  if (termTag(port2) != I60 || getI60(port2) != 8) {
    sprintf(msg, "OPY port 2 should be I60(8), got tag %s val %ld",
            tagStr(termTag(port2)), (long)getI60(port2));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/SUP with LAM in ports — verifies ERA propagates into SUP's children
void testEraSupLam(void) {
  char msg[100];
  
  // Inner LAM: port1=SUB, port2=I60(99)
  Term innerLam = makePair(LAM, 0, SUB, newI60(99));

  // SUP: port1=innerLam, port2=I60(52)
  Term sup = makePair(SUP, 0, innerLam, newI60(52));

  interact(ERA, sup);

  // LAM port1 should be NUL
  Term port1 = take(portLoc(1, innerLam));
  if (termTag(port1) != NUL) {
    sprintf(msg, "LAM port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// ------------------------------------------------------------------
// negSup tests — APP/SUP, OPX/SUP, OPY/SUP wildcard handler
// ------------------------------------------------------------------

// Test OPX/SUP with x=NUL
// After (x=NUL): SUP aux port 1 carries NUL. * (OPX) principal connects directly to y.
// The negSup handler pushes a redex (OPX, I60(83)) to the stack.
void testNegSupXNul(void) {
  char msg[100];

  // SUP: port1=NUL (x), port2=I60(83) (y)
  Term sup = makePair(SUP, 0, NUL, newI60(83));

  // OPX: port1=I60(77) (a), port2=ERA (b)
  Term opx = makePair(OPX, OP_ADD, newI60(77), ERA);
  interact(opx, sup);

  // After (x=NUL case):
  // - OPX aux port 2 → y (I60(83)) — pushed as redex

  // Verify the redex (OPX, I60(83)) was pushed to the stack
  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex on stack");
  }
  if (pairs.count != 0) {
    sprintf(msg, "pairs.count should be 0 after pop, got %u", pairs.count);
    BOOM(msg);
  }
  if (termTag(neg) != OPX) {
    sprintf(msg, "redex neg should be OPX, got tag %s", tagStr(termTag(neg)));
    BOOM(msg);
  }
  if (termTag(pos) != I60 || getI60(pos) != 83) {
    sprintf(msg, "redex pos should be I60(83), got tag %s val %ld",
            tagStr(termTag(pos)), (long)getI60(pos));
    BOOM(msg);
  }

  // Verify OPX port 1 still has a (I60(77))
  Term opxPort1 = take(portLoc(1, opx));
  if (termTag(opxPort1) != I60 || getI60(opxPort1) != 77) {
    sprintf(msg, "OPX port 1 should be I60(77), got tag %s val %ld",
            tagStr(termTag(opxPort1)), (long)getI60(opxPort1));
    BOOM(msg);
  }

  // Verify SUP port 1 is still NUL
  Term supPort1 = take(portLoc(2, opx));
  if (termTag(supPort1) != ERA) {
    sprintf(msg, "OPX port 2 should be ERA, got tag %s", tagStr(termTag(supPort1)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test APP/SUP with y=NUL
// After (y=NUL): Both SUP ports are taken, non-null port's term pushed as redex with neg.
void testNegSupYNul(void) {
  char msg[100];

  // Inner LAM: port1=SUB, port2=I60(99)
  Term innerLam = makePair(LAM, 0, SUB, newI60(99));

  // SUP: port1=innerLam (x), port2=NUL (y)
  Term sup = makePair(SUP, 0, innerLam, NUL);

  // APP: port1=I60(77) (a), port2=ERA (b)
  Term app = makePair(APP, 0, newI60(77), ERA);
  interact(app, sup);

  // After (y=NUL case):
  // - Both SUP ports taken, SUP pair freed
  // - Redex (APP, innerLam) pushed to stack

  // Verify the redex (APP, LAM) was pushed to the stack
  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex on stack");
  }
  if (pairs.count != 0) {
    sprintf(msg, "pairs.count should be 0 after pop, got %u", pairs.count);
    BOOM(msg);
  }
  if (termTag(neg) != APP) {
    sprintf(msg, "redex neg should be APP, got tag %s", tagStr(termTag(neg)));
    BOOM(msg);
  }
  if (termTag(pos) != LAM) {
    sprintf(msg, "redex pos should be LAM, got tag %s", tagStr(termTag(pos)));
    BOOM(msg);
  }

  // Verify APP port 1 still has a (I60(77))
  Term appPort1 = take(portLoc(1, app));
  if (termTag(appPort1) != I60 || getI60(appPort1) != 77) {
    sprintf(msg, "APP port 1 should be I60(77), got tag %s val %ld",
            tagStr(termTag(appPort1)), (long)getI60(appPort1));
    BOOM(msg);
  }

  // Clean up APP port 2 (ERA)
  Term appPort2 = take(portLoc(2, app));
  if (termTag(appPort2) != ERA) {
    sprintf(msg, "APP port 2 should be ERA, got tag %s", tagStr(termTag(appPort2)));
    BOOM(msg);
  }

  // Clean up LAM ports (pos is the LAM term)
  // take() on SUB returns VAR and doesn't free — use get + freeLoc
  Term lamPort1 = get(portLoc(1, pos));
  if (termTag(lamPort1) != SUB) {
    sprintf(msg, "LAM port 1 should be SUB, got tag %s", tagStr(termTag(lamPort1)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, pos));
  Term lamPort2 = take(portLoc(2, pos));
  if (termTag(lamPort2) != I60 || getI60(lamPort2) != 99) {
    sprintf(msg, "LAM port 2 should be I60(99), got tag %s val %ld",
            tagStr(termTag(lamPort2)), (long)getI60(lamPort2));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test APP/SUP general case (both x and y are non-NUL)
// After: 6 new nodes created (dp1, cn1, lz1, cn2, lz2, dp2).
//         move() loses dp2 when old value is ERA, so APP port 2 is freed.
void testNegSupGeneral(void) {
  char msg[100];

  // SUP: port1=I60(83) (x), port2=I60(99) (y)
  Term sup = makePair(SUP, 0, newI60(83), newI60(99));

  // OPX: port1=I60(77) (a), port2=SUB (b) — SUB so dp2 stays at port2
  Term opx = makePair(OPX, 0, newI60(77), SUB);

  interact(opx, sup);

  // After general case:
  // - Original SUP freed (both ports taken)
  // - Original APP: port1 taken, port2 holds dp2 (SUB→dp2 swap returns SUB, no erasure)
  // - 6 new nodes created: dp1 (DUP), cn1 (APP), lz1 (LAZ), cn2 (APP), lz2 (LAZ), dp2 (SUP)
  // - glblAlloced == 7 (6 new + APP pair)
  if (glblAlloced != 7) {
    sprintf(msg, "glblAlloced should be 7, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }

  // dp2 should be at OPX port2
  Term dp2 = get(portLoc(2, opx));
  if (termTag(dp2) != SUP) {
    sprintf(msg, "APP port2 should be SUP, got tag %s", tagStr(termTag(dp2)));
    BOOM(msg);
  }

  // Clean up: erase dp2 and everything it references
  // dp2 = SUP(VAR→cn1_port2, VAR→cn2_port2)
  // Erase both SUP ports via eraVar → eraseLazy
  interact(ERA, newTerm(VAR, 0, portLoc(1, dp2)));
  interact(ERA, newTerm(VAR, 0, portLoc(2, dp2)));
  // OPX port2 still has dp2 (now erased), free it
  take(portLoc(2, opx));
  // glblAlloced should be 0
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0 after cleanup, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/SUP with x=NUL
void testNegSupOpxXNul(void) {
  char msg[100];

  // SUP: port1=NUL (x), port2=I60(83) (y)
  Term sup = makePair(SUP, 0, NUL, newI60(83));

  // OPX: port1=I60(77) (a), port2=ERA (b)
  Term opx = makePair(OPX, OP_ADD, newI60(77), ERA);
  interact(opx, sup);

  // After (x=NUL case):
  // - Both SUP ports taken, SUP pair freed
  // - Redex (OPX, I60(83)) pushed to stack

  // Verify the redex (OPX, I60(83)) was pushed
  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex on stack");
  }
  if (pairs.count != 0) {
    sprintf(msg, "pairs.count should be 0 after pop, got %u", pairs.count);
    BOOM(msg);
  }
  if (termTag(neg) != OPX) {
    sprintf(msg, "redex neg should be OPX, got tag %s", tagStr(termTag(neg)));
    BOOM(msg);
  }
  if (termTag(pos) != I60 || getI60(pos) != 83) {
    sprintf(msg, "redex pos should be I60(83), got tag %s val %ld",
            tagStr(termTag(pos)), (long)getI60(pos));
    BOOM(msg);
  }

  // Verify OPX port 1 still has a (I60(77))
  Term opxPort1 = take(portLoc(1, opx));
  if (termTag(opxPort1) != I60 || getI60(opxPort1) != 77) {
    sprintf(msg, "OPX port 1 should be I60(77), got tag %s val %ld",
            tagStr(termTag(opxPort1)), (long)getI60(opxPort1));
    BOOM(msg);
  }

  // Clean up OPX port 2 (ERA)
  Term opxPort2 = take(portLoc(2, opx));
  if (termTag(opxPort2) != ERA) {
    sprintf(msg, "OPX port 2 should be ERA, got tag %s", tagStr(termTag(opxPort2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPY/SUP with y=NUL
void testNegSupOpYYNul(void) {
  char msg[100];

  // SUP: port1=I60(83) (x), port2=NUL (y)
  Term sup = makePair(SUP, 0, newI60(83), NUL);

  // OPY: port1=I60(77) (a), port2=ERA (b)
  Term opy = makePair(OPY, OP_ADD, newI60(77), ERA);
  interact(opy, sup);

  // After (y=NUL case):
  // - Both SUP ports taken, SUP pair freed
  // - Redex (OPY, I60(83)) pushed to stack

  // Verify the redex (OPY, I60(83)) was pushed
  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex on stack");
  }
  if (pairs.count != 0) {
    sprintf(msg, "pairs.count should be 0 after pop, got %u", pairs.count);
    BOOM(msg);
  }
  if (termTag(neg) != OPY) {
    sprintf(msg, "redex neg should be OPY, got tag %s", tagStr(termTag(neg)));
    BOOM(msg);
  }
  if (termTag(pos) != I60 || getI60(pos) != 83) {
    sprintf(msg, "redex pos should be I60(83), got tag %s val %ld",
            tagStr(termTag(pos)), (long)getI60(pos));
    BOOM(msg);
  }

  // Verify OPY port 1 still has a (I60(77))
  Term opyPort1 = take(portLoc(1, opy));
  if (termTag(opyPort1) != I60 || getI60(opyPort1) != 77) {
    sprintf(msg, "OPY port 1 should be I60(77), got tag %s val %ld",
            tagStr(termTag(opyPort1)), (long)getI60(opyPort1));
    BOOM(msg);
  }

  // Clean up OPY port 2 (ERA)
  Term opyPort2 = take(portLoc(2, opy));
  if (termTag(opyPort2) != ERA) {
    sprintf(msg, "OPY port 2 should be ERA, got tag %s", tagStr(termTag(opyPort2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/NUL with SUB in ports — verifies SUB gets rewired to NUL
void testDupNulSub(void) {
  char msg[100];
  

  // DUP: port1=SUB, port2=SUB (both negative)
  Term dup = makePair(DUP, 0, SUB, SUB);

  interact(dup, NUL);

  // DUP port1 should be NUL
  Term port1 = take(portLoc(1, dup));
  if (termTag(port1) != NUL) {
    sprintf(msg, "DUP port 1 should be NUL, got tag %s", tagStr(termTag(port1)));
    BOOM(msg);
  }

  // DUP port2 should be NUL
  Term port2 = take(portLoc(2, dup));
  if (termTag(port2) != NUL) {
    sprintf(msg, "DUP port 2 should be NUL, got tag %s", tagStr(termTag(port2)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUM with subtraction
void testOpxNumSub(void) {
  char msg[100];
  

  // OPX: port1=I60(3) [a], port2=ERA
  // NUM: I60(10) [#]
  // Result: # - a = 10 - 3 = 7
  Term opx = makePair(OPX, OP_SUB, newI60(3), ERA);
  interact(opx, newI60(10));

  Term result = take(portLoc(2, opx));
  if (termTag(result) != I60 || getI60(result) != 7) {
    sprintf(msg, "OPX port 2 should be I60(7), got tag %s val %ld",
            tagStr(termTag(result)), (long)getI60(result));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUM with multiplication
void testOpxNumMul(void) {
  char msg[100];
  

  Term opx = makePair(OPX, OP_MUL, newI60(4), ERA);
  interact(opx, newI60(6));

  Term result = take(portLoc(2, opx));
  if (termTag(result) != I60 || getI60(result) != 24) {
    sprintf(msg, "OPX port 2 should be I60(24), got tag %s val %ld",
            tagStr(termTag(result)), (long)getI60(result));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUM with equality
void testOpxNumEq(void) {
  char msg[100];
  

  Term opx = makePair(OPX, OP_EQ, newI60(5), ERA);
  interact(opx, newI60(5));

  // 5 == 5 is true (1)
  Term result = take(portLoc(2, opx));
  if (termTag(result) != I60 || getI60(result) != 1) {
    sprintf(msg, "OPX port 2 should be I60(1), got tag %s val %ld",
            tagStr(termTag(result)), (long)getI60(result));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test OPX/NUL — verify pair is freed
void testOpxNulExplicit(void) {
  char msg[100];
  

  // OPX: port1=I60(7), port2=ERA
  Term opx = makePair(OPX, OP_ADD, newI60(7), ERA);

  interact(opx, NUL);

  // Pair should be freed by negNul (both ports handled)
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test cascading ERA through 3 levels of LAM
void testEraLamTriple(void) {
  char msg[100];
  

  // Level 3: LAM with I60(777)
  Term level3 = makePair(LAM, 0, SUB, newI60(777));

  // Level 2: LAM with level3 as body
  Term level2 = makePair(LAM, 0, SUB, level3);

  // Level 1: LAM with level2 as body
  Term level1 = makePair(LAM, 0, SUB, level2);

  // ERA connects to level1
  interact(ERA, level1);

  // All LAM port1 should be NUL
  Term p1 = take(portLoc(1, level1));
  Term p2 = take(portLoc(1, level2));
  Term p3 = take(portLoc(1, level3));

  if (termTag(p1) != NUL || termTag(p2) != NUL || termTag(p3) != NUL) {
    sprintf(msg, "All LAM ports should be NUL");
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/NUM with different values — verifies pair is freed
void testDupNumDifferent(void) {
  char msg[100];
  

  // DUP: port1=ERA, port2=ERA
  Term dup = makePair(DUP, 0, ERA, ERA);

  interact(dup, newI60(123));

  // Pair should be freed by dupLeaf
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test SUB/NUL with label=0 (SUB literal — ports not connected)
void testSubNulLiteral(void) {
  char msg[100];
  

  // SUB literal (label=0) — ports are not connected
  Term sub = SUB;

  interact(sub, NUL);

  // SUB literal should remain unchanged
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test multi-redex: outer APP/LAM pushes (innerAPP, innerLAM) as redex
void testMultiRedex(void) {
  char msg[100];
  

  // Inner LAM: port1=SUB, port2=I60(100)
  Term innerLam = makePair(LAM, 0, SUB, newI60(100));

  // Inner APP: port1=I60(7), port2=SUB
  Term innerApp = makePair(APP, 0, newI60(7), SUB);

  // Outer APP: port1=NUL, port2=innerApp
  Term outerApp = makePair(APP, 0, NUL, innerApp);

  // Outer LAM: port1=SUB, port2=innerLam
  Term outerLam = makePair(LAM, 0, SUB, innerLam);

  interact(outerApp, outerLam);

  // After outer APP/LAM:
  // - move(APP port2, innerLam) pushes redex(innerApp, innerLam)
  // - move(LAM port1, NUL) — SUB, no redex
  //
  // Pop and interact the redex
  Term neg, pos;
  if (!popRedex(&neg, &pos)) {
    BOOM("expected one redex");
  }

  // Verify redex terms
  if (termTag(neg) != APP || termTag(pos) != LAM) {
    BOOM("expected (APP, LAM) redex");
  }

  interact(neg, pos);

  // After inner APP/LAM:
  // - innerApp port2 = I60(100) (body)
  // - innerLam port1 = I60(7) (argument)

  Term innerPort1 = take(portLoc(1, innerLam));
  Term innerPort2 = take(portLoc(2, innerApp));

  if (termTag(innerPort1) != I60 || getI60(innerPort1) != 7) {
    sprintf(msg, "innerLam port 1 should be I60(7), got tag %s val %ld",
            tagStr(termTag(innerPort1)), (long)getI60(innerPort1));
    BOOM(msg);
  }
  if (termTag(innerPort2) != I60 || getI60(innerPort2) != 100) {
    sprintf(msg, "innerApp port 2 should be I60(100), got tag %s val %ld",
            tagStr(termTag(innerPort2)), (long)getI60(innerPort2));
    BOOM(msg);
  }

  // Clean up outer pairs
  // outerApp pair was freed by freePair during interact (both ports coalesced)
  // outerLam pair: port2 already freed, port1 has NUL
  take(portLoc(1, outerLam));  // NUL (port2 already VOID)

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/VAR → I60: follows VAR chain, erases the I60
void testEraVarI60(void) {
  char msg[100];
  

  // Create a location holding I60(74)
  Term holder = makePair(LAM, 0, SUB, newI60(74));
  Location i60Loc = portLoc(2, holder);

  // VAR pointing to that location
  Term var = newTerm(VAR, 0, i60Loc);

  // ERA interacts with VAR
  interact(ERA, var);

  // Clean up holder pair (port1 has SUB, freed by freeLoc)
  freeLoc(portLoc(1, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA → SUP: ERA connects to SUP principal, both ports get ERA
void testEraVarSup(void) {
  char msg[100];
  

  // SUP: port1=I60(7), port2=I60(8)
  Term sup = makePair(SUP, 0, newI60(7), newI60(8));

  // ERA interacts directly with SUP
  interact(ERA, sup);

  // eraVar frees the locations via take(), ports are freed (not ERA)
  // The SUP pair is freed by eraVar's take() calls (both ports freed -> freePair)
  // So glblAlloced should be 0 (SUP pair was the only allocation)
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test ERA/VAR with VAR chain: VAR → VAR → I60
void testEraVarChain(void) {
  char msg[100];
  

  // Create a location holding I60(55)
  Term holder = makePair(LAM, 0, SUB, newI60(55));
  Location i60Loc = portLoc(2, holder);

  // VAR1 → VAR2 → I60
  Term var1 = newTerm(VAR, 0, i60Loc);
  Term var2 = newTerm(VAR, 0, termLoc(var1));

  // ERA interacts with VAR2 (chain of 2)
  interact(ERA, var2);

  // Clean up: port1 should be SUB (use get+freeLoc for SUB)
  Term port1 = get(portLoc(1, holder));
  if (termTag(port1) != SUB) {
    sprintf(msg, "holder port 1 should be SUB, got tag %s",
            tagStr(termTag(port1)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testCascadingRedex(void) {
  char msg[100];
  

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

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test APP/NUL with LAM argument — exercises interact(ERA, LAM) → eraLam
void testAppNulLamArg(void) {
  char msg[100];

  // Inner LAM: port1=SUB, port2=I60(74)
  Term innerLam = makePair(LAM, 0, SUB, newI60(74));

  // APP: port1=innerLam, port2=ERA
  Term app = makePair(APP, 0, innerLam, ERA);

  interact(app, NUL);

  // APP port1 freed by take in negNul, port2 freed by swap with ERA
  // innerLam port1 rewired to NUL by eraLam, port2 freed by take
  Term innerPort1 = take(portLoc(1, innerLam));
  if (termTag(innerPort1) != NUL) {
    sprintf(msg, "innerLAM port 1 should be NUL, got tag %s", tagStr(termTag(innerPort1)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test SUB/NUL with LAM body — exercises interact(ERA, LAM) → eraLam
void testSubNulLamBody(void) {
  char msg[100];
  

  // Inner LAM: port1=SUB, port2=I60(99)
  Term innerLam = makePair(LAM, 0, SUB, newI60(99));

  // SUB: port1=ERA, port2=innerLam — label > 0 so ports are connected
  Term sub = makePair(SUB, 1, ERA, innerLam);

  interact(sub, NUL);

  // SUB port1 should be freed (was ERA, freed by swap)
  // SUB port2 should be freed (was LAM, erased by eraLam)
  // innerLam port1 should be NUL (eraLam rewired it)
  Term innerPort1 = take(portLoc(1, innerLam));
  if (termTag(innerPort1) != NUL) {
    sprintf(msg, "innerLAM port 1 should be NUL, got tag %s", tagStr(termTag(innerPort1)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test swap with SUB (deferred redex) — swap a SUB literal, verify no redex pushed
void testSwapSub(void) {
  char msg[100];
  

  // Create a SUB pair (deferred redex) holding APP and LAM
  Term innerApp = makePair(APP, 0, newI60(7), SUB);
  Term innerLam = makePair(LAM, 0, SUB, newI60(137));
  Term subPair = makePair(SUB, 1, innerApp, innerLam);

  // Swap NUL into the SUB pair's port1 location
  // The old value at port1 is innerApp (APP term)
  Term result = swap(portLoc(1, subPair), NUL);

  // swap should return innerApp (the old value) since it's not SUB/ERA
  if (termTag(result) != APP) {
    sprintf(msg, "swap should return APP, got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }

  // The SUB pair's port1 should now contain NUL
  Term locVal = get(portLoc(1, subPair));
  if (termTag(locVal) != NUL) {
    sprintf(msg, "SUB port1 should be NUL after swap, got tag %s", tagStr(termTag(locVal)));
    BOOM(msg);
  }

  // Clean up: use take() for non-SUB/LAZ, get+freeLoc for SUB
  // subPair: port1=NUL, port2=innerLam
  // innerLam: port1=SUB, port2=I60(137)
  // innerApp: port1=I60(7), port2=SUB
  // Free inner pairs first, then subPair
  Term p2_innerLam = take(portLoc(2, innerLam));  // should be I60
  if (termTag(p2_innerLam) != I60) {
    sprintf(msg, "innerLam port2 should be I60, got tag %s", tagStr(termTag(p2_innerLam)));
    BOOM(msg);
  }
  Term p1_innerLam = get(portLoc(1, innerLam));  // should be SUB
  if (termTag(p1_innerLam) != SUB) {
    sprintf(msg, "innerLam port1 should be SUB, got tag %s", tagStr(termTag(p1_innerLam)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, innerLam));
  Term p2_subPair = take(portLoc(2, subPair));  // should be innerLam (a LAM pair)
  if (termTag(p2_subPair) != LAM) {
    sprintf(msg, "subPair port2 should be LAM, got tag %s", tagStr(termTag(p2_subPair)));
    BOOM(msg);
  }
  Term p1_subPair = take(portLoc(1, subPair));  // should be NUL
  if (termTag(p1_subPair) != NUL) {
    sprintf(msg, "subPair port1 should be NUL, got tag %s", tagStr(termTag(p1_subPair)));
    BOOM(msg);
  }
  Term p2_innerApp = get(portLoc(2, innerApp));  // should be SUB
  if (termTag(p2_innerApp) != SUB) {
    sprintf(msg, "innerApp port2 should be SUB, got tag %s", tagStr(termTag(p2_innerApp)));
    BOOM(msg);
  }
  freeLoc(portLoc(2, innerApp));
  Term p1_innerApp = take(portLoc(1, innerApp));  // should be I60
  if (termTag(p1_innerApp) != I60) {
    sprintf(msg, "innerApp port1 should be I60, got tag %s", tagStr(termTag(p1_innerApp)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: lazy DUP cycle — context VAR → DUP, DUP ports → LAZ
void testIsCycleLazyDup(void) {
  char msg[100];

  // Build the lazy DUP cycle using swap to avoid makePair polarity issues
  // Step 1: Create DUP with SUB placeholders
  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  // Step 2: Create LAZ with SUB in port 1, VAR→dupLoc in port 2
  Term laz = makePair(LAZ, 0, SUB, newTerm(VAR, 0, dupLoc));
  Location lazLoc = termLoc(laz);

  // Step 3: Swap LAZ into DUP ports (bypass makePair polarity check)
  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  // Step 4: Swap DUP into LAZ port 1
  swap(portLoc(1, laz), dup);

  // Verify structure
  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != VAR) {
    sprintf(msg, "LAZ port 2 should be VAR, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }

  // Check: is there a cycle from context back to LAZ?
  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should detect lazy DUP cycle, got %d", result);
    BOOM(msg);
  }

  // Clean up: laz port1=DUP, port2=VAR; dup both ports=LAZ
  // take() frees non-SUB/LAZ locations; LAZ ports need explicit free
  take(portLoc(1, laz));  // DUP (freed)
  take(portLoc(2, laz));  // VAR (coalesces → frees LAZ pair)
  // DUP ports contain LAZ — take doesn't free LAZ locations
  // Free DUP pair explicitly (also clears both port locations)
  freePair(termLoc(dup) & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: no cycle — context is a leaf (I60)
void testIsCycleNoCycle(void) {
  char msg[100];

  Term laz = makePair(LAZ, 0, SUB, newI60(73));
  Location lazLoc = termLoc(laz);

  int result = isCycle(newI60(73), lazLoc);
  if (result != 0) {
    sprintf(msg, "isCycle should return 0 for leaf context, got %d", result);
    BOOM(msg);
  }

  // Clean up
  Term p1_laz = get(portLoc(1, laz));
  if (termTag(p1_laz) != SUB) {
    sprintf(msg, "LAZ port 1 should be SUB, got tag %s", tagStr(termTag(p1_laz)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, laz));
  Term p2_laz = get(portLoc(2, laz));
  if (termTag(p2_laz) != I60) {
    sprintf(msg, "LAZ port 2 should be I60, got tag %s", tagStr(termTag(p2_laz)));
    BOOM(msg);
  }
  freeLoc(portLoc(2, laz));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: VAR chain → LAZ directly (not via DUP)
void testIsCycleVarToLaz(void) {
  char msg[100];

  // Create LAZ: port 1 = SUB, port 2 = VAR → lazLoc
  Term laz = makePair(LAZ, 0, SUB, newTerm(VAR, 0, 0));
  Location lazLoc = termLoc(laz);

  // Replace port 2's VAR with VAR → lazLoc
  Term context = newTerm(VAR, 0, lazLoc);
  swap(portLoc(2, laz), context);

  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should detect VAR→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  // Clean up: laz port1=SUB, port2=VAR→lazLoc (self-ref cycle)
  // Swap port1 to NUL (clears SUB without freeing), then take(port2) which
  // follows VAR→lazLoc, gets LAZ (returns VAR), frees port2, coalesces pair
  swap(portLoc(1, laz), NUL);
  take(portLoc(2, laz));  // VAR→lazLoc, takes port2, coalesces (port1 is NUL/VOID)

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: VAR chain → I60 (no cycle)
void testIsCycleVarToI60(void) {
  char msg[100];

  // Create LAZ with I60 in port 2 (context is I60, no cycle)
  Term laz = makePair(LAZ, 0, SUB, newI60(99));
  Location lazLoc = termLoc(laz);

  // Context is I60 (leaf, no cycle possible)
  Term context = get(portLoc(2, laz));

  int result = isCycle(context, lazLoc);
  if (result != 0) {
    sprintf(msg, "isCycle should return 0 for I60, got %d", result);
    BOOM(msg);
  }

  // Clean up: use get+freeLoc for SUB (take doesn't free SUB locations)
  get(portLoc(1, laz));  // SUB
  freeLoc(portLoc(1, laz));  // free it
  take(portLoc(2, laz));  // I60 (coalesces LAZ pair)

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: VAR → SUP — DUP port - LAZ, cycle
void testIsCycleVarThruSup(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, newTerm(VAR, 0, portLoc(1, dup)), newI60(77));

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != SUP) {
    sprintf(msg, "LAZ port 2 should be SUP, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for LAZ→SUP→LAZ, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // VAR follows chain to SUP port 2 (frees it)
  take(portLoc(1, sup));  // NUL (SUP pair coalesced)
  take(portLoc(2, sup));
  freePair(dupLoc);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: SUP port1=VAR→DUP, port2=I60 — cycle
void testIsCycleVarThruSupI60(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, newTerm(VAR, 0, portLoc(1, dup)), newI60(77));

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != SUP) {
    sprintf(msg, "LAZ port 2 should be SUP, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for SUP→VAR→DUP→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // SUP
  take(portLoc(1, sup));  // VAR follows chain to DUP port 1
  take(portLoc(2, sup));  // I60 (SUP pair coalesced)
  freePair(dupLoc & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: LAM port1=APP, APP port1=VAR→DUP port1, LAM port2=I60 — cycle
void testIsCycleVarThruLamI60(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term var = newTerm(VAR, 0, portLoc(1, dup));
  Term app = makePair(APP, 0, var, ERA);
  Term lam = makePair(LAM, 0, app, newI60(77));

  Term laz = makePair(LAZ, 0, dup, lam);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != LAM) {
    sprintf(msg, "LAZ port 2 should be LAM, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for LAM→VAR→DUP→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // LAM
  take(portLoc(1, lam));  // APP
  take(portLoc(1, app));  // VAR follows chain to DUP port 1
  take(portLoc(2, app));  // ERA (APP pair coalesced)
  take(portLoc(2, lam));  // I60 (LAM pair coalesced)
  freePair(dupLoc & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: SUP port1=VAR→DUP, port2=NUL — cycle
void testIsCycleVarThruSupNul(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, newTerm(VAR, 0, portLoc(1, dup)), NUL);

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != SUP) {
    sprintf(msg, "LAZ port 2 should be SUP, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for SUP→VAR→DUP→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // SUP
  take(portLoc(1, sup));  // VAR follows chain to DUP port 1
  take(portLoc(2, sup));  // NUL (SUP pair coalesced)
  freePair(dupLoc & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: LAM port1=APP(NUL,ERA), port2=VAR→DUP port2 — cycle
void testIsCycleVarThruLamNulPort2(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term app = makePair(APP, 0, NUL, ERA);
  Term var = newTerm(VAR, 0, portLoc(2, dup));
  Term lam = makePair(LAM, 0, app, var);

  Term laz = makePair(LAZ, 0, dup, lam);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != LAM) {
    sprintf(msg, "LAZ port 2 should be LAM, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for LAM→VAR→DUP→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // LAM
  take(portLoc(1, lam));  // APP
  take(portLoc(2, lam));  // VAR follows chain to DUP port 2 (LAM pair coalesced)
  take(portLoc(1, app));  // NUL
  take(portLoc(2, app));  // ERA (APP pair coalesced)
  freePair(dupLoc & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test isCycle: SUP→LAM→APP→VAR→DUP port2 — cycle
void testIsCycleVarThruSupDupPort2(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term var = newTerm(VAR, 0, portLoc(2, dup));
  Term app = makePair(APP, 0, var, ERA);
  Term lam = makePair(LAM, 0, app, newI60(77));
  Term sup = makePair(SUP, 0, lam, NUL);

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  if (termTag(get(portLoc(1, laz))) != DUP) {
    sprintf(msg, "LAZ port 1 should be DUP, got %s", tagStr(termTag(get(portLoc(1, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, laz))) != SUP) {
    sprintf(msg, "LAZ port 2 should be SUP, got %s", tagStr(termTag(get(portLoc(2, laz)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(1, dup))) != LAZ) {
    sprintf(msg, "DUP port 1 should be LAZ, got %s", tagStr(termTag(get(portLoc(1, dup)))));
    BOOM(msg);
  }
  if (termTag(get(portLoc(2, dup))) != LAZ) {
    sprintf(msg, "DUP port 2 should be LAZ, got %s", tagStr(termTag(get(portLoc(2, dup)))));
    BOOM(msg);
  }


  int result = isCycle(get(portLoc(2, laz)), lazLoc);
  if (result != 1) {
    sprintf(msg, "isCycle should return 1 for SUP→LAM→APP→VAR→DUP→LAZ cycle, got %d", result);
    BOOM(msg);
  }

  take(portLoc(1, laz));  // DUP
  take(portLoc(2, laz));  // SUP
  take(portLoc(1, sup));  // LAM
  take(portLoc(2, sup));  // NUL (SUP pair coalesced)
  take(portLoc(1, lam));  // APP
  take(portLoc(2, lam));  // I60 (LAM pair coalesced)
  take(portLoc(1, app));  // VAR follows chain to DUP port 2
  take(portLoc(2, app));  // ERA (APP pair coalesced)
  freePair(dupLoc & 0xFFFFFFFE);

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test eraseLazy: cycle case — LAZ with DUP thunk, both DUP ports → LAZ, context cycles back
void testEraseLazyCycle1(void) {
  char msg[100];

  // Create DUP first
  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  // Create LAZ with DUP in port 1, VAR->dup in port 2
  Term context = newTerm(VAR, 0, dupLoc + 1);
  Term laz = makePair(LAZ, 0, dup, context);
  Location lazLoc = termLoc(laz);

  // Wire DUP ports to LAZ (lazy DUP pattern)
  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test eraseLazy: cycle case — LAZ with DUP thunk, both DUP ports → LAZ, context cycles back
void testEraseLazyCycle2(void) {
  char msg[100];

  // Create DUP first
  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  // Create LAZ with DUP in port 1, VAR->dup port 2 in port 2
  Term context = newTerm(VAR, 0, dupLoc + 1);
  Term laz = makePair(LAZ, 0, dup, context);
  Location lazLoc = termLoc(laz);

  // Wire DUP ports to LAZ (lazy DUP pattern)
  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testEraseLazyNoCycleDup1(void) {
  char msg[100];

  // Create DUP first
  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  // Create LAZ with DUP in port 1, I60 in port 2 (context, no cycle)
  Term laz = makePair(LAZ, 0, dup, newI60(88));
  Location lazLoc = termLoc(laz);

  // Wire DUP port 1 to LAZ, port 2 to LAZ
  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  interact(ERA, newTerm(VAR, 0, dupLoc));

  // Verify DUP port 2 has context (I60), port 2 freed
  Term p1_dup = take(portLoc(2, dup));
  if (termTag(p1_dup) != I60) {
    sprintf(msg, "DUP port 1 should be I60 (context wired), got %s", tagStr(termTag(p1_dup)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testEraseLazyNoCycleDup2(void) {
  char msg[100];

  // Create DUP first
  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  // Create LAZ with DUP in port 1, I60 in port 2 (context, no cycle)
  Term laz = makePair(LAZ, 0, dup, newI60(99));
  Location lazLoc = termLoc(laz);

  // Wire DUP port 1 to LAZ, port 2 to LAZ
  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  interact(ERA, newTerm(VAR, 0, dupLoc));

  // Verify DUP port 2 has context (I60), port 1 freed
  Term p2_dup = take(portLoc(2, dup));
  if (termTag(p2_dup) != I60) {
    sprintf(msg, "DUP port 2 should be I60 (context wired), got %s", tagStr(termTag(p2_dup)));
    BOOM(msg);
  }

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: SUP port1=VAR→DUP port2, port2=I60 — cycle
void testEraVarThruSupI60(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, newTerm(VAR, 0, portLoc(2, dup)), newI60(77));

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);


  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAM port1=APP(I60,ERA), port2=VAR→DUP port2 — cycle
void testEraVarThruLamI60(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term app = makePair(APP, 0, newI60(77), ERA);
  Term var = newTerm(VAR, 0, portLoc(2, dup));
  Term lam = makePair(LAM, 0, app, var);

  Term laz = makePair(LAZ, 0, dup, lam);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);


  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: SUP port1=NUL, port2=VAR→DUP port2 — cycle
void testEraVarThruSupNul(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, NUL, newTerm(VAR, 0, portLoc(2, dup)));

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);


  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAM port1=APP(NUL,ERA), port2=VAR→DUP port2 — cycle
void testEraVarThruLamNulPort2(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term app = makePair(APP, 0, NUL, ERA);
  Term var = newTerm(VAR, 0, portLoc(2, dup));
  Term lam = makePair(LAM, 0, app, var);

  Term laz = makePair(LAZ, 0, dup, lam);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);


  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: SUP port1=VAR→DUP port1, port2=I60 — VAR points to DUP port 1 (debug)
void testEraVarThruSupVarToDupPort1(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term sup = makePair(SUP, 0, newTerm(VAR, 0, portLoc(2, dup)), newI60(77));

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);

  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: SUP→LAM→APP→VAR→DUP port2 — cycle
void testEraVarThruSupDupPort2(void) {
  char msg[100];

  Term dup = makePair(DUP, 0, SUB, SUB);
  Location dupLoc = termLoc(dup);

  Term var = newTerm(VAR, 0, portLoc(2, dup));
  Term app = makePair(APP, 0, var, ERA);
  Term lam = makePair(LAM, 0, app, newI60(77));
  Term sup = makePair(SUP, 0, lam, NUL);

  Term laz = makePair(LAZ, 0, dup, sup);
  Location lazLoc = termLoc(laz);

  swap(portLoc(1, dup), laz);
  swap(portLoc(2, dup), laz);


  interact(ERA, newTerm(VAR, 0, dupLoc));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is I60
void testEraVarAppThunkI60(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term app = makePair(APP, 0, newI60(77), ERA);
  Term laz = makePair(LAZ, 0, app, newI60(88));

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is NUL
void testEraVarAppThunkNul(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term app = makePair(APP, 0, newI60(77), ERA);
  Term laz = makePair(LAZ, 0, app, NUL);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is SUP(I60,NUL)
void testEraVarAppThunkSupI60(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term app = makePair(APP, 0, newI60(77), ERA);
  Term sup = makePair(SUP, 0, newI60(88), NUL);
  Term laz = makePair(LAZ, 0, app, sup);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is SUP(NUL,I60)
void testEraVarAppThunkSupNul(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term app = makePair(APP, 0, newI60(77), ERA);
  Term sup = makePair(SUP, 0, NUL, newI60(88));
  Term laz = makePair(LAZ, 0, app, sup);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is LAM(APP(I60,ERA),I60)
void testEraVarAppThunkLamI60(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term innerApp = makePair(APP, 0, newI60(77), ERA);
  Term lam = makePair(LAM, 0, innerApp, newI60(88));
  Term app = makePair(APP, 0, newI60(99), ERA);
  Term laz = makePair(LAZ, 0, app, lam);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is LAM(APP(NUL,ERA),NUL)
void testEraVarAppThunkLamNul(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term innerApp = makePair(APP, 0, NUL, ERA);
  Term lam = makePair(LAM, 0, innerApp, NUL);
  Term app = makePair(APP, 0, newI60(77), ERA);
  Term laz = makePair(LAZ, 0, app, lam);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is SUP(I60,LAM(APP(NUL,ERA),I60))
void testEraVarAppThunkSupLamI60(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term innerApp = makePair(APP, 0, NUL, ERA);
  Term lam = makePair(LAM, 0, innerApp, newI60(88));
  Term sup = makePair(SUP, 0, newI60(77), lam);
  Term app = makePair(APP, 0, newI60(99), ERA);
  Term laz = makePair(LAZ, 0, app, sup);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// eraVar test: LAZ thunk is APP, context is SUP(NUL,LAM(APP(I60,ERA),NUL))
void testEraVarAppThunkSupLamNul(void) {
  char msg[100];

  Term holder = makePair(LAM, 0, SUB, newI60(0));
  Location holderLoc = portLoc(2, holder);

  Term innerApp = makePair(APP, 0, newI60(88), ERA);
  Term lam = makePair(LAM, 0, innerApp, NUL);
  Term sup = makePair(SUP, 0, NUL, lam);
  Term app = makePair(APP, 0, newI60(77), ERA);
  Term laz = makePair(LAZ, 0, app, sup);

  swap(holderLoc, laz);
  interact(ERA, newTerm(VAR, 0, holderLoc));

  swap(portLoc(1, holder), NUL);
  take(portLoc(1, holder));
  take(portLoc(2, holder));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// =============================================================================
// DUP/SUP Interaction Tests
// =============================================================================

// DUP/SUP annihilation: same labels, DUP aux ports wire to SUP aux ports
void testDupSupAnnihilation(void) {
  char msg[100];

  // DUP: label=0, aux ports=SUB, SUB
  Term dup = makePair(DUP, 0, SUB, SUB);

  // SUP: label=0 (same as DUP), aux ports=I60(7), I60(8)
  Term sup = makePair(SUP, 0, newI60(7), newI60(8));

  // Trigger DUP/SUP interaction — should annihilate
  interact(dup, sup);

  // DUP aux ports should now contain SUP's values
  Term p1 = take(portLoc(1, dup));
  if (termTag(p1) != I60 || getI60(p1) != 7) {
    sprintf(msg, "dup port1 should be I60(7), got tag %s val %ld",
            tagStr(termTag(p1)), (long)getI60(p1));
    BOOM(msg);
  }
  Term p2 = take(portLoc(2, dup));
  if (termTag(p2) != I60 || getI60(p2) != 8) {
    sprintf(msg, "dup port2 should be I60(8), got tag %s val %ld",
            tagStr(termTag(p2)), (long)getI60(p2));
    BOOM(msg);
  }

  // Both DUP and SUP pairs consumed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0 after annihilation, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// DUP/SUP commutation: different labels, DUP aux ports get new SUP nodes
void testDupSupCommutation(void) {
  char msg[100];

  // DUP: label=0, aux ports=SUB, SUB
  Term dup = makePair(DUP, 0, SUB, SUB);

  // SUP: label=1 (different from DUP), aux ports=I60(7), I60(8)
  Term sup = makePair(SUP, 1, newI60(7), newI60(8));

  // Trigger DUP/SUP interaction — should commute/expand
  interact(dup, sup);

  // DUP aux ports should contain new SUP nodes
  Term s1 = take(portLoc(1, dup));
  if (termTag(s1) != SUP) {
    sprintf(msg, "dup port1 should be SUP, got %s", tagStr(termTag(s1)));
    BOOM(msg);
  }
  Term s2 = take(portLoc(2, dup));
  if (termTag(s2) != SUP) {
    sprintf(msg, "dup port2 should be SUP, got %s", tagStr(termTag(s2)));
    BOOM(msg);
  }

  // s1: port1=I60(7), port2=I60(8) (makeLazyDup with I60 puts values directly)
  Term s1p1 = take(portLoc(1, s1));
  if (termTag(s1p1) != I60 || getI60(s1p1) != 7) {
    sprintf(msg, "sup1 port1 should be I60(7), got tag %s val %ld",
            tagStr(termTag(s1p1)), (long)getI60(s1p1));
    BOOM(msg);
  }
  Term s1p2 = take(portLoc(2, s1));
  if (termTag(s1p2) != I60 || getI60(s1p2) != 8) {
    sprintf(msg, "sup1 port2 should be I60(8), got tag %s val %ld",
            tagStr(termTag(s1p2)), (long)getI60(s1p2));
    BOOM(msg);
  }

  // s2: port1=I60(7), port2=I60(8)
  Term s2p1 = take(portLoc(1, s2));
  if (termTag(s2p1) != I60 || getI60(s2p1) != 7) {
    sprintf(msg, "sup2 port1 should be I60(7), got tag %s val %ld",
            tagStr(termTag(s2p1)), (long)getI60(s2p1));
    BOOM(msg);
  }
  Term s2p2 = take(portLoc(2, s2));
  if (termTag(s2p2) != I60 || getI60(s2p2) != 8) {
    sprintf(msg, "sup2 port2 should be I60(8), got tag %s val %ld",
            tagStr(termTag(s2p2)), (long)getI60(s2p2));
    BOOM(msg);
  }

  // All pairs consumed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// DUP/SUP commutation with identity LAMs in SUP ports
void testDupSupCommutationLam(void) {
  char msg[100];

  // DUP: label=0, aux ports=SUB, SUB
  Term dup = makePair(DUP, 0, SUB, SUB);

  // Identity LAM: port1=SUB, port2=VAR→port1
  Term lam1 = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lam1), newTerm(VAR, 0, portLoc(1, lam1)));
  Term lam2 = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lam2), newTerm(VAR, 0, portLoc(1, lam2)));

  // SUP: label=1 (different from DUP), aux ports=lam1, lam2
  Term sup = makePair(SUP, 1, lam1, lam2);

  // Trigger DUP/SUP interaction
  interact(dup, sup);

  // DUP aux ports should contain new SUP nodes
  Term s1 = take(portLoc(1, dup));
  if (termTag(s1) != SUP) {
    sprintf(msg, "dup port1 should be SUP, got %s", tagStr(termTag(s1)));
    BOOM(msg);
  }
  Term s2 = take(portLoc(2, dup));
  if (termTag(s2) != SUP) {
    sprintf(msg, "dup port2 should be SUP, got %s", tagStr(termTag(s2)));
    BOOM(msg);
  }

  // SUPs have VARs into LAZ/DUP chains (LAM is not a native value)
  Term s1p1 = take(portLoc(1, s1));
  if (termTag(s1p1) != VAR) {
    sprintf(msg, "sup1 port1 should be VAR, got %s", tagStr(termTag(s1p1)));
    BOOM(msg);
  }
  Term s1p2 = take(portLoc(2, s1));
  if (termTag(s1p2) != VAR) {
    sprintf(msg, "sup1 port2 should be VAR, got %s", tagStr(termTag(s1p2)));
    BOOM(msg);
  }
  Term s2p1 = take(portLoc(1, s2));
  if (termTag(s2p1) != VAR) {
    sprintf(msg, "sup2 port1 should be VAR, got %s", tagStr(termTag(s2p1)));
    BOOM(msg);
  }
  Term s2p2 = take(portLoc(2, s2));
  if (termTag(s2p2) != VAR) {
    sprintf(msg, "sup2 port2 should be VAR, got %s", tagStr(termTag(s2p2)));
    BOOM(msg);
  }

  // Follow VAR chains to DUP nodes and erase them
  interact(ERA, newTerm(VAR, 0, termLoc(s1p1)));
  interact(ERA, newTerm(VAR, 0, termLoc(s1p2)));
  interact(ERA, newTerm(VAR, 0, termLoc(s2p1)));
  interact(ERA, newTerm(VAR, 0, termLoc(s2p2)));

  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// =============================================================================
// DUP/LAM Interaction Tests
// =============================================================================
// DUP principal connects to LAM principal.
// After: a,b each connect to a LAM. LAM aux ports connect to SUP. SUP
// principal connects to x. LAZ aux ports connect to DUP and y.
//
// NOTE: DUP ports are initialized with ERA. When the handler moves new
// LAMs into the DUP's ports, swap encounters ERA which triggers
// interact(ERA, new_value) → eraLam erases the new LAM. The handler
// must handle old ERA values before moving new LAMs in.

void testDupLam(void) {
  char msg[100];

  // Original LAM: port1=SUB, port2=I60(88) (y)
  Term lam = makePair(LAM, 0, SUB, newI60(88));

  // DUP: port1=ERA, port2=ERA — old values that handler must handle
  Term dup = makePair(DUP, 0, ERA, ERA);

  // Trigger DUP/LAM interaction
  interact(dup, lam);

  // Original LAM pair freed by interaction — verify port1 is VOID
  if (take(portLoc(1, lam)) != NUL) {
    sprintf(msg, "original LAM port1 should be NUL after interaction");
    BOOM(msg);
  }
  // All nodes freed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/LAM with SUB in both DUP ports — full expansion path
void testDupLamWithSub(void) {
  char msg[100];

  // Original LAM: port1=SUB, port2=I60(88) (y)
  Term lam = makePair(LAM, 0, ERA, newI60(88));

  // DUP: port1=SUB, port2=SUB — old values that handler must handle
  Term dup = makePair(DUP, 0, SUB, SUB);

  // Trigger DUP/LAM interaction
  interact(dup, lam);

  // With SUB ports, handler takes full expansion path:
  // - Original LAM port1 gets SUP (moved via move(var, du1))
  // - Original LAM port2 freed by take(bod)
  // - DUP port1 = new LAM1, DUP port2 = new LAM2
  // Original LAM pair remains allocated (not freed like ERA case)

  // Original LAM port2 was freed by take(bod) in handler — not taken here
  // to avoid double-free. glblAlloced verifies all pairs freed.

  // DUP port1 should be new LAM1
  Term lam1 = take(portLoc(1, dup));
  if (termTag(lam1) != LAM) {
    sprintf(msg, "DUP port1 should be LAM, got %s", tagStr(termTag(lam1)));
    BOOM(msg);
  }

  // DUP port2 should be new LAM2
  Term lam2 = take(portLoc(2, dup));
  if (termTag(lam2) != LAM) {
    sprintf(msg, "DUP port2 should be LAM, got %s", tagStr(termTag(lam2)));
    BOOM(msg);
  }

  // Cleanup remaining pairs created by handler in correct order:
  // l1: port1=SUB, port2=VAR→dp port1
  // l2: port1=SUB, port2=VAR→dp port2
  // dp: port1=I60, port2=I60

  // Free lam1/lam2 port 1
  Term trm = take(portLoc(1, lam1));
  if (trm != ERA) {
    sprintf(msg, "LAM 1 port 1 should be ERA, got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }

  trm = take(portLoc(1, lam2));
  if (trm != ERA) {
    sprintf(msg, "LAM 22port 1 should be ERA, got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }

  // Free lam1/lam2 port2
  trm = take(portLoc(2, lam1));
  if (trm != newI60(88)) {
    sprintf(msg, "LAM 1 port 2 should be I60(88), got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }

  trm = take(portLoc(2, lam2));
  if (trm != newI60(88)) {
    sprintf(msg, "LAM 2 port 2 should be I60(88), got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }

  // All nodes freed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// Test DUP/LAM with SUB in both DUP ports and Identity LAM — full expansion path
void testDupIdentity(void) {
  char msg[100];

  // Original LAM: port1=SUB, port2=I60(88) (y)
  Term lam = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lam), newTerm(VAR, 0, portLoc(1, lam)));

  // DUP: port1=SUB, port2=SUB — old values that handler must handle
  Term dup = makePair(DUP, 0, SUB, SUB);

  // Trigger DUP/LAM interaction
  interact(dup, lam);

  // After interaction:
  // dup port1 = l1 (LAM), port2 = l2 (LAM)
  // l1 port1 = SUB, port2 = VAR→du2 port1
  // l2 port1 = SUB, port2 = VAR→du2 port2
  // du2 port1 = DUP (dp), port2 = VAR→original LAM port1
  // dp port1 = LAZ (du2), port2 = LAZ (du2)
  // original LAM port1 = sup (SUP), port2 = freed
  // sup port1 = VAR→l1 port1, port2 = VAR→l2 port1

  // --- Take l1 and l2 from dup ---
  Term lam1 = take(portLoc(1, dup));
  if (termTag(lam1) != LAM) {
    sprintf(msg, "dup port1 should be LAM, got %s", tagStr(termTag(lam1)));
    BOOM(msg);
  }
  Term lam2 = take(portLoc(2, dup));
  if (termTag(lam2) != LAM) {
    sprintf(msg, "dup port2 should be LAM, got %s", tagStr(termTag(lam2)));
    BOOM(msg);
  }

  // --- Take l1 port2 and l2 port2 (both VAR→du2) ---
  Term laz1 = take(portLoc(2, lam1));
  if (termTag(laz1) != VAR) {
    sprintf(msg, "l1 port2 should be VAR, got %s", tagStr(termTag(laz1)));
    BOOM(msg);
  }
  Term laz2 = take(portLoc(2, lam2));
  if (termTag(laz2) != VAR) {
    sprintf(msg, "l2 port2 should be VAR, got %s", tagStr(termTag(laz2)));
    BOOM(msg);
  }

  // Both point to the same LAZ (mask to even to compare pair locations)
  if ((termLoc(laz1) & 0xFFFFFFFE) != (termLoc(laz2) & 0xFFFFFFFE)) {
    BOOM("both LAMs should point to the same LAZ");
  }
  Location du2Loc = termLoc(laz1) & 0xFFFFFFFE;
  Term du2 = get(du2Loc);
  if (termTag(du2) != LAZ) {
    sprintf(msg, "du2 should be LAZ, got %s", tagStr(termTag(du2)));
    BOOM(msg);
  }

  // --- Verify original LAM port1 is SUP (take on du2 port2 will free it) ---
  Term supAtLamPort1 = get(portLoc(1, lam));
  if (termTag(supAtLamPort1) != SUP) {
    sprintf(msg, "original LAM port1 should be SUP, got %s", tagStr(termTag(supAtLamPort1)));
    BOOM(msg);
  }

  // --- Take du2 port1 (DUP) and port2 (VAR chain → SUP, frees LAM port1) ---
  Term dp = take(portLoc(1, du2));
  if (termTag(dp) != DUP) {
    sprintf(msg, "du2 port1 should be DUP, got %s", tagStr(termTag(dp)));
    BOOM(msg);
  }
  Term sup = take(portLoc(2, du2));
  if (termTag(sup) != SUP) {
    sprintf(msg, "du2 port2 should be SUP (via VAR chain), got %s", tagStr(termTag(sup)));
    BOOM(msg);
  }

  // --- Take sup's ports (both VAR→l1/l2 port1) ---
  Term varToL1 = take(portLoc(1, sup));
  if (termTag(varToL1) != VAR) {
    sprintf(msg, "sup port1 should be VAR, got %s", tagStr(termTag(varToL1)));
    BOOM(msg);
  }
  Term varToL2 = take(portLoc(2, sup));
  if (termTag(varToL2) != VAR) {
    sprintf(msg, "sup port2 should be VAR, got %s", tagStr(termTag(varToL2)));
    BOOM(msg);
  }

  // --- Take dp's ports (both LAZ→du2, take returns VAR without freeing) ---
  Term lazDp1 = take(portLoc(1, dp));
  if (termTag(lazDp1) != VAR) {
    sprintf(msg, "dp port1 take should return VAR, got %s", tagStr(termTag(lazDp1)));
    BOOM(msg);
  }
  Term lazDp2 = take(portLoc(2, dp));
  if (termTag(lazDp2) != VAR) {
    sprintf(msg, "dp port2 take should return VAR, got %s", tagStr(termTag(lazDp2)));
    BOOM(msg);
  }

  // --- Free dp's ports ---
  freeLoc(portLoc(1, dp));
  freeLoc(portLoc(2, dp));

  // --- Free l1 port1 (SUB) and l2 port1 (SUB) ---
  Term trm = get(portLoc(1, lam1));
  if (termTag(trm) != SUB) {
    sprintf(msg, "l1 port1 should be SUB, got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, lam1));

  trm = get(portLoc(1, lam2));
  if (termTag(trm) != SUB) {
    sprintf(msg, "l2 port1 should be SUB, got %s", tagStr(termTag(trm)));
    BOOM(msg);
  }
  freeLoc(portLoc(1, lam2));

  // All nodes freed
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

// =============================================================================
// Free/Alloc Pair Tests
// =============================================================================
void testAllocPairReturnsEven(void) {
  char msg[100];
  for (int i = 0; i < 50; i++) {
    Term p = makePair(SUP, 0, newI60(i), newI60(i + 1));
    Location loc = termLoc(p);
    if (loc & 0x1) {
      sprintf(msg, "allocPair returned odd location %u at iteration %d", loc, i);
      BOOM(msg);
    }
    freeLoc(portLoc(1, p));
    freeLoc(portLoc(2, p));
  }
  if (glblAlloced != 0) {
    sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testFreeLocCoalesces(void) {
  char msg[100];
  Term p = makePair(SUP, 0, newI60(1), newI60(2));
  freeLoc(portLoc(1, p));
  if (glblAlloced != 1) {
    sprintf(msg, "after freeing port1, glblAlloced should still be 1, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
  freeLoc(portLoc(2, p));
  if (glblAlloced != 0) {
    sprintf(msg, "after freeing port2, glblAlloced should be 0, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
}

void testFreeLocSingleCellDoesNotFreePair(void) {
  Term p = makePair(SUP, 0, newI60(1), newI60(2));
  freeLoc(portLoc(1, p));
  if (glblAlloced != 1) BOOM("freeLoc on port1 should not free the pair");
  freeLoc(portLoc(2, p));
  if (glblAlloced != 0) BOOM("freeLoc on port2 should free the pair");
}

void testInterleavedFreeLoc(void) {
  char msg[100];
  Term p1 = makePair(SUP, 0, newI60(1), newI60(2));
  Term p2 = makePair(SUP, 0, newI60(3), newI60(4));
  Term p3 = makePair(SUP, 0, newI60(5), newI60(6));
  freeLoc(portLoc(1, p1));
  freeLoc(portLoc(1, p2));
  if (glblAlloced != 3) {
    sprintf(msg, "glblAlloced should be 3, got %lld", (long long)glblAlloced);
    BOOM(msg);
  }
  freeLoc(portLoc(2, p1));
  if (glblAlloced != 2) BOOM("p1 freed, glblAlloced should be 2");
  freeLoc(portLoc(2, p2));
  if (glblAlloced != 1) BOOM("p2 freed, glblAlloced should be 1");
  freeLoc(portLoc(1, p3));
  freeLoc(portLoc(2, p3));
  if (glblAlloced != 0) BOOM("all freed, glblAlloced should be 0");
}

void testMakePairStoresTerms(void) {
  char msg[100];
  Term val1 = newI60(42), val2 = newI60(99);
  Term p = makePair(SUP, 5, val1, val2);
  if (get(portLoc(1, p)) != val1) BOOM("port 1 should store val1");
  if (get(portLoc(2, p)) != val2) BOOM("port 2 should store val2");
  freeLoc(portLoc(1, p)); freeLoc(portLoc(2, p));
  if (glblAlloced != 0) BOOM("glblAlloced should be 0");
}

void testStressAllocFree(void) {
  char msg[100];
  for (int cycle = 0; cycle < 20; cycle++) {
    Term pairs[100];
    for (int i = 0; i < 100; i++) {
      pairs[i] = makePair(SUP, 0, newI60(i + cycle * 100), newI60(i + cycle * 100 + 50));
    }
    if (glblAlloced != 100) {
      sprintf(msg, "cycle %d: glblAlloced should be 100, got %lld", cycle, (long long)glblAlloced);
      BOOM(msg);
    }
    for (int i = 0; i < 100; i++) {
      freeLoc(portLoc(1, pairs[i]));
      freeLoc(portLoc(2, pairs[i]));
    }
    if (glblAlloced != 0) {
      sprintf(msg, "cycle %d: glblAlloced should be 0, got %lld", cycle, (long long)glblAlloced);
      BOOM(msg);
    }
  }
}

void testFreeListEntryFormat(void) {
  char msg[100];
  Term p = makePair(SUP, 0, newI60(1), newI60(2));
  Location loc = termLoc(p);
  freeLoc(portLoc(1, p));
  freeLoc(portLoc(2, p));
  Term entry = get(loc);
  if (termTag(entry) != NUL) {
    sprintf(msg, "free list entry should have NUL tag, got %s", tagStr(termTag(entry)));
    BOOM(msg);
  }
  if (termLab(entry) != 0xFF) {
    sprintf(msg, "free list entry label should be 0xFF, got %u", termLab(entry));
    BOOM(msg);
  }
}
int main(int argc, char *argv[]) {
  hvmInit(1024);

  testAllocPairReturnsEven();
  testFreeLocCoalesces();
  testFreeLocSingleCellDoesNotFreePair();
  testInterleavedFreeLoc();
  testMakePairStoresTerms();
  testStressAllocFree();
  testFreeListEntryFormat();

  testAppLam();
  testMoveEra();
  testTakeVarChain();
  testTakeLaz();
  testTakeSub();
  testCascading();
  testMoveNul();
  testEraLam();
  testEraLamNulBody();
  testEraLamLamBody();
  testEraBoth();
  testAppNul();
  testOpxNul();
  testOpYNul();
  testSubNul();
  testEraSup();
  testDupNul();
  testDupNum();
  testOpxNum();
  testOpYNum();
  testEraSupLam();
  testNegSupXNul();
  testNegSupYNul();
  testNegSupGeneral();
  testNegSupOpxXNul();
  testNegSupOpYYNul();
  testDupNulSub();
  testOpxNumSub();
  testOpxNumMul();
  testOpxNumEq();
  testOpxNulExplicit();
  testEraLamTriple();
  testDupNumDifferent();
  testSubNulLiteral();
  testMultiRedex();
  testEraVarI60();
  testEraVarSup();
  testEraVarChain();
  testCascadingRedex();
  testAppNulLamArg();
  testSubNulLamBody();
  testSwapSub();
  testIsCycleLazyDup();
  testIsCycleNoCycle();
  testIsCycleVarToLaz();
  testIsCycleVarToI60();
  testIsCycleVarThruSup();
  testIsCycleVarThruSupI60();
  testIsCycleVarThruLamI60();
  testIsCycleVarThruSupNul();
  testIsCycleVarThruLamNulPort2();
  testIsCycleVarThruSupDupPort2();
  testEraseLazyCycle1();
  testEraseLazyCycle2();
  testEraseLazyNoCycleDup1();
  testEraseLazyNoCycleDup2();
  testEraVarThruSupI60();
  testEraVarThruLamI60();
  testEraVarThruSupNul();
  testEraVarThruLamNulPort2();
  testEraVarThruSupVarToDupPort1();
  testEraVarThruSupDupPort2();
  testEraVarAppThunkI60();
  testEraVarAppThunkNul();
  testEraVarAppThunkSupI60();
  testEraVarAppThunkSupNul();
  testEraVarAppThunkLamI60();
  testEraVarAppThunkLamNul();
  testEraVarAppThunkSupLamI60();
  testEraVarAppThunkSupLamNul();
  testDupSupAnnihilation();
  testDupSupCommutation();
  testDupSupCommutationLam();
  testDupLam();
  testDupLamWithSub();
  testDupIdentity();

  hvmFree();
  return 0;
}

