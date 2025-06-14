#include "new.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Print contents of RBAG_BUFF between start and end locations
void print_rbag(Location start, Location end) {
  Term* buff = get_rbag_buff();
  if (!buff) {
    printf("RBAG_BUFF is not initialized\n");
    return;
  }
  if (start >= end) {
    printf("Invalid range: start=%u end=%u\n", start, end);
    return;
  }
  printf("RBAG contents from %u to %u:\n", start, end);
  for (Location i = start; i < end; i += 2) {
    printf(" %.3x  ", i);
    print_raw_term(buff[i]);
    printf("  ");
    print_raw_term(buff[i + 1]);
    printf("\n");
  }
  printf("\n");
}

bool leafFn(Term ref, Term args) {
  Term a0 = pair_make(APP, 0, NUL, SUB);
  Term b = term_new(VAR, 0, port(2, term_loc(a0)));
  Term l2 = pair_make(LAM, 0, a0, b);
  Term l1 = pair_make(LAM, 0, ERA, l2);
  Term l0 = pair_make(LAM, 0, SUB, l1);
  set(port(1, term_loc(a0)), term_new(VAR, 0, port(1, term_loc(l0))));
  term_link(args, l0);
  return true;
}
Term leaf = new_ref(leafFn);

bool makeLeafFn(Term ref, Term args) {
  term_link(args, leaf);
  return true;
}
Term makeLeaf = new_ref(makeLeafFn);

bool sumLeafFn(Term ref, Term args) {
  Term l = pair_make(LAM, 0, SUB, NUL);
  set(port(2, term_loc(l)), term_new(VAR, 0, port(1, term_loc(l))));
  term_link(args, l);
  return true;
}
Term sumLeaf = new_ref(sumLeafFn);

bool nodeFn(Term ref, Term args) {
  Term l3 = pair_make(LAM, 0, ERA, NUL);
  Term l2 = pair_make(LAM, 0, SUB, l3);
  Term l1 = pair_make(LAM, 0, SUB, l2);
  Term rgt = term_new(VAR, 0, port(1, term_loc(l1)));
  Term l0 = pair_make(LAM, 0, SUB, l1);
  Term lft = term_new(VAR, 0, port(1, term_loc(l0)));
  Term a1 = pair_make(APP, 0, rgt, SUB);
  Term a0 = pair_make(APP, 0, lft, a1);
  set(port(2, term_loc(l3)), term_new(VAR, 0, port(2, term_loc(a1))));
  set(port(1, term_loc(l2)), a0);

  term_link(args, l0);
  return true;
}
Term node = new_ref(nodeFn);

Term sum;

bool sumNodeFn(Term ref, Term args) {
  Term s = pair_make(OPX, OP_ADD, NUL, SUB);
  Term l1 = pair_make(LAM, 0, SUB, term_new(VAR, 0, port(2, term_loc(s))));
  Term l0 = pair_make(LAM, 0, SUB, l1);

  Term sumLft = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l0))), SUB);
  Term sumRgt = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l1))), s);
  set(port(1, term_loc(s)), term_new(VAR, 0, port(2, term_loc(sumLft))));
  term_link(sumLft, sum);
  term_link(sumRgt, sum);
  term_link(args, l0);
  return true;
}
Term sumNode = new_ref(sumNodeFn);

bool sumFn(Term ref, Term args) {
  Term a1 = pair_make(APP, 0, sumLeaf, SUB);
  Term a0 = pair_make(APP, 0, sumNode, a1);
  Term l = pair_make(LAM, 0, a0, term_new(VAR, 0, port(2, term_loc(a1))));

  term_link(args, l);
  return true;
}
Term sum = new_ref(sumFn);

int main() {
  // Initialize the VM with some memory
  hvm_init(1024 * 1024);
  hvm_reset();
    
  Term l1 = pair_make(APP, 0, new_i60(20), SUB);
  term_link(l1, leaf);

  Term l2 = pair_make(APP, 0, new_i60(21), SUB);
  term_link(l2, leaf);

  Term lft = term_new(VAR, 0, port(2, term_loc(l1)));
  Term rgt = term_new(VAR, 0, port(2, term_loc(l2)));
  Term a1 = pair_make(APP, 0, rgt, SUB);
  Term a0 = pair_make(APP, 0, lft, a1);
  term_link(a0, node);

  Term n = term_new(VAR, 0, port(2, term_loc(a1)));
  Term a = pair_make(APP, 0, n, SUB);
  term_link(a, sum);
  normalize();
  print_term("result", take(port(2, term_loc(a))));
  // print_term("result", take(n));

  printf("alloced pairs: %d\n", alloced);

  return 0;
}
