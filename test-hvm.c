#include <stdio.h>
#include "new.h"

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

  hvmFree();
  return 0;
}
