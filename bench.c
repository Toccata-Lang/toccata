#include "new.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Print contents of BUFF between start and end locations
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

  default:
    printf("  Location: %u\n", term_loc(term));
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


bool nodeFn(Term ref, Term args) {
  Term t3 = pair_make(LAM, 0, ERA, NUL);
  Term t2 = pair_make(LAM, 0, SUB, t3);
  Term t1 = pair_make(LAM, 0, SUB, t2);
  Term t0 = pair_make(LAM, 0, SUB, t1);
  Term a1 = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(t1))), SUB);
  Term a0 = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(t0))), a1);
  set(port(2, term_loc(t3)), term_new(VAR, 0, port(2, term_loc(a1))));

  term_link(args, t0);
  return true;
}
Term node = new_ref(nodeFn);

bool sumLeafFn(Term ref, Term args) {
  Term l = pair_make(LAM, 0, SUB, NUL);
  set(port(2, term_loc(l)), term_new(VAR, 0, port(1, term_loc(l))));
  term_link(args, l);
  return true;
}
Term sumLeaf = new_ref(sumLeafFn);

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

int main() {
  // Initialize the VM with some memory
  hvm_init(1024 * 1024);
  hvm_reset();
    
  Term s = pair_make(APP, 0, sumLeaf, SUB);
  Term lv = pair_make(APP, 0, new_i60(0), s);
  Term h = pair_make(APP, 0, new_i60(20), lv);
  term_link(h, leaf);
  normalize();
  print_buff(0, 20);
  print_term("result", take(port(2, term_loc(s))));
  print_buff(0, 20);

  printf("alloced pairs: %d\n", alloced);

  return 0;
}
