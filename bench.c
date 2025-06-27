#include "new.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

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

bool makeLeafFn(Term ref, Term args) {
  term_link(args, leaf);
  return true;
}
Term makeLeaf = new_ref(makeLeafFn);

Term defer(Term ref, Term args) {
  Location argLoc = port(1, term_loc(args));
  Term rTrm = take(argLoc);
  if (term_tag(rTrm) == VAR) {
    set(argLoc, rTrm);
    Location varLoc = term_loc(rTrm);
    Term deferred = pair_make(SUB, 6, args, ref);
    Term newTrm = swap(varLoc, deferred);
    if (newTrm != SUB) {
      freeLoc(argLoc);
      freeLoc(varLoc);
      pair_free(deferred);
      return newTrm;
    }
  }
  return rTrm;
}

Term makeNode;
bool makeFn(Term ref, Term args) {
  Term hTrm = defer(ref, args);
  switch(term_tag(hTrm)) {
  case VAR:
    return true;

  case I60:
    if (1) {
      int h = get_i60(hTrm);
      Term a = take(port(2, term_loc(args)));
      if (h == 0)
	term_link(a, makeLeaf);
      else
	term_link(pair_make(APP, 0, new_i60(h - 1), a), makeNode);
    }
    break;

  default:
    print_term("Bad argument to 'make'", hTrm);
    print_term("args", args);
    abort();
    break;
  }
  return true;
}
Term make = new_ref(makeFn);

bool makeNodeFn(Term ref, Term args) {
  Term rgtN = pair_make(OPY, OP_ADD, new_i60(1), SUB);
  Term lftN = pair_make(DUP, 0, SUB, rgtN);
  Term dblN = pair_make(OPY, OP_MUL, new_i60(2), lftN);
  Location nLoc = term_loc(lftN);

  Term h = pair_make(DUP, 0, SUB, SUB);
  Location hLoc = term_loc(h);

  Term lftA1 = pair_make(APP, 0, term_new(VAR, 0, port(1, nLoc)), SUB);
  Term lftA0 = pair_make(APP, 0, term_new(VAR, 0, port(1, hLoc)), lftA1);
  Term lft = term_new(VAR, 0, port(2, term_loc(lftA1)));

  Term rgtA1 = pair_make(APP, 0, term_new(VAR, 0, port(2, term_loc(rgtN))), SUB);
  Term rgtA0 = pair_make(APP, 0, term_new(VAR, 0, port(2, hLoc)), rgtA1);
  Term rgt = term_new(VAR, 0, port(2, term_loc(rgtA1)));

  Term nA1 = pair_make(APP, 0, rgt, SUB);
  Term nA0 = pair_make(APP, 0, lft, nA1);
  
  Term l1 = pair_make(LAM, 0, dblN, term_new(VAR, 0, port(2, term_loc(nA1))));
  Term l0 = pair_make(LAM, 0, h, l1);

  Pairs pairs;
  pairs.count = 0;
  store_redex(&pairs, args, l0);
  store_redex(&pairs, lftA0, make);
  store_redex(&pairs, nA0, node);
  store_redex(&pairs, rgtA0, make);
  link_redexes(&pairs);
  return true;
}
Term makeNode = new_ref(makeNodeFn);

bool sumLeafFn(Term ref, Term args) {
  Term l = pair_make(LAM, 0, SUB, NUL);
  set(port(2, term_loc(l)), term_new(VAR, 0, port(1, term_loc(l))));
  term_link(args, l);
  return true;
}
Term sumLeaf = new_ref(sumLeafFn);

Term sum;

bool sumNodeFn(Term ref, Term args) {
  Term s = pair_make(OPX, OP_ADD, NUL, SUB);
  Term l1 = pair_make(LAM, 0, SUB, term_new(VAR, 0, port(2, term_loc(s))));
  Term l0 = pair_make(LAM, 0, SUB, l1);

  Term sumLft = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l0))), SUB);
  Term sumRgt = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l1))), s);
  set(port(1, term_loc(s)), term_new(VAR, 0, port(2, term_loc(sumLft))));
  Pairs pairs;
  pairs.count = 0;
  store_redex(&pairs, sumRgt, sum);
  store_redex(&pairs, args, l0);
  store_redex(&pairs, sumLft, sum);
  link_redexes(&pairs);
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

struct timeval startTime, endTime;
double elapsed;
int height;
unsigned long long expected;

bool endFn(Term ref, Term args) {
  Term rTrm = defer(ref, args);
  switch(term_tag(rTrm)) {
  case VAR:
    return true;
    break;

  case I60: 
    gettimeofday(&endTime, NULL);
    elapsed = (endTime.tv_sec - startTime.tv_sec) +
      (endTime.tv_usec - startTime.tv_usec) / 1000000.0;

    pair_free(term_loc(args));
    print_term("result", rTrm);
    printf("exptd: %llu\n", expected);
    printf("interactions: %lu\n", reduced);
    printf("MIPS: %f\n", reduced / elapsed / 1000000);
    printf("elapsed: %f\n", elapsed);
    printf("alloced pairs: %lu\n", alloced);
    if (expected != get_i60(rTrm)) {
      abort();
    }
    if (alloced != 0) {
      print_buff(0, 50);
      exit(1);
    }
#ifndef SINGLE_THREAD
    for (int i = 0; i < threadCount; i++) {
      push_redex(0, 0);
    }
#endif
    break;

  default:
    pthread_mutex_lock(&redex_mutex);
    print_term("bad result", rTrm);
    BOOM("in 'end'");
    break;
  }
  return true;
}
Term end = new_ref(endFn);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("\ncorrect command line is \"bench <height> <threads>\"\n");
    exit(1);
  }
  
  // Initialize the VM with some memory
  hvm_init(1024 * 1024 * 1024);

  height = atoi(argv[1]);
  if (height < 0) {
    printf("\nInvalid height: %d\n", height);
    exit(1);
  } else {
    printf("\nHeight: %d\n", height);
  }
  expected = ((1 << height) - 1) * (1 << height) / 2;

#ifndef SINGLE_THREAD
  threadCount = atoi(argv[2]);
  printf("Threads: %d\n", threadCount);
#else
  printf("Running single thread\n");
#endif

  for(int reps = 0; reps < 10000; reps++) {
    hvm_reset();
    gettimeofday(&startTime, NULL);

    Term a1 = pair_make(APP, 0, new_i60(0), SUB);
    Term a0 = pair_make(APP, 0, new_i60(height), a1);

    Term n = term_new(VAR, 0, port(2, term_loc(a1)));
    Term a = pair_make(APP, 0, n, SUB);
    Term a3 = pair_make(APP, 0, term_new(VAR, 0, port(2, term_loc(a))), SUB);

    Pairs pairs;
    pairs.count = 0;
    store_redex(&pairs, a0, make);
    store_redex(&pairs, a, sum);
    store_redex(&pairs, a3, end);
    link_redexes(&pairs);

    // normalize(NULL);

    spawn_threads();
    for(int i = 0; i < threadCount; i++) {
      pthread_join(threads[i], NULL);
    }
  }

  return 0;
}

char *refName(Term ref) {
  if (ref == leaf)
    return "leaf";
  else if (ref == node)
    return "node";
  else if (ref == makeLeaf)
    return "makeLeaf";
  else if (ref == make)
    return "make";
  else if (ref == makeNode)
    return "makeNode";
  else if (ref == sumLeaf)
    return "sumLeaf";
  else if (ref == sumNode)
    return "sumNode";
  else if (ref == sum)
    return "sum";
  else if (ref == end)
    return "end";
  else
    return "unknown";
}
