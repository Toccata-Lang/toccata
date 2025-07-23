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

void leafFn(Term ref, Term args) {
  Term a0 = pair_make(APP, 0, NUL, SUB);
  Term b = term_new(VAR, 0, port(2, term_loc(a0)));
  Term l2 = pair_make(LAM, 0, a0, b);
  Term l1 = pair_make(LAM, 0, ERA, l2);
  Term l0 = pair_make(LAM, 0, SUB, l1);
  Pairs pairs;
  pairs.count = 0;
  swapStore(port(1, term_loc(a0)), term_new(VAR, 0, port(1, term_loc(l0))), &pairs);
  store_redex(&pairs, args, l0);
  link_redexes(&pairs);
  return;
}
Term leaf = new_ref(leafFn);

void nodeFn(Term ref, Term args) {
  Term l3 = pair_make(LAM, 0, ERA, NUL);
  Term l2 = pair_make(LAM, 0, SUB, l3);
  Term l1 = pair_make(LAM, 0, SUB, l2);
  Term rgt = term_new(VAR, 0, port(1, term_loc(l1)));
  Term l0 = pair_make(LAM, 0, SUB, l1);
  Term lft = term_new(VAR, 0, port(1, term_loc(l0)));
  Term a1 = pair_make(APP, 0, rgt, SUB);
  Term a0 = pair_make(APP, 0, lft, a1);
  Pairs pairs;
  pairs.count = 0;
  swapStore(port(2, term_loc(l3)), term_new(VAR, 0, port(2, term_loc(a1))), &pairs);
  swapStore(port(1, term_loc(l2)), a0, &pairs);
  store_redex(&pairs, args, l0);
  link_redexes(&pairs);
  return;
}
Term node = new_ref(nodeFn);

Term defer(Term ref, Term args, Pairs *pairs) {
  Location argLoc = port(1, term_loc(args));
  Term rTrm;
  Tag t;
  do {
    rTrm = get(argLoc);
    t = term_tag(rTrm);
    if (t == VAR)
      argLoc = term_loc(rTrm);
  } while(t == VAR);

  if (t == SUB) {
    if (rTrm != SUB) {
      BOOM("Should never happen");
      return VAR;
    } else {
      Term deferred = pair_make(SUB, 6, args, ref);
      Term newTrm = swapStore(argLoc, deferred, pairs);
      if (newTrm != SUB) {
	swapStore(argLoc, newTrm, pairs);
      }
      return VAR;
    }
  } else {
    Term rslt = take(port(1, term_loc(args)));
    return rslt;
  }
}

Term makeNode;
void makeFn(Term ref, Term args) {
  Pairs pairs;
  pairs.count = 0;
  Term hTrm = defer(ref, args, &pairs);
  switch(term_tag(hTrm)) {
  case VAR:
    break;

  case I60:
    if (1) {
      int h = get_i60(hTrm);
      Term a = take(port(2, term_loc(args)));
      if (h == 0) {
	store_redex(&pairs, a, leaf);
      } else {
	store_redex(&pairs, pair_make(APP, 0, new_i60(h - 1), a), makeNode);
      }
    }
    break;

  default:
    print_term("Bad argument to 'make'", hTrm);
    print_term("args", args);
    BOOM("log");
    break;
  }
  link_redexes(&pairs);
  return;
}
Term make = new_ref(makeFn);

void makeNodeFn(Term ref, Term args) {
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
  return;
}
Term makeNode = new_ref(makeNodeFn);

void sumLeafFn(Term ref, Term args) {
  Term l = pair_make(LAM, 0, SUB, NUL);
  Pairs pairs;
  pairs.count = 0;
  swapStore(port(2, term_loc(l)), term_new(VAR, 0, port(1, term_loc(l))), &pairs);
  store_redex(&pairs, args, l);
  link_redexes(&pairs);
  return;
}
Term sumLeaf = new_ref(sumLeafFn);

Term sum;

void sumNodeFn(Term ref, Term args) {
  Term s = pair_make(OPX, OP_ADD, NUL, SUB);
  Term l1 = pair_make(LAM, 0, SUB, term_new(VAR, 0, port(2, term_loc(s))));
  Term l0 = pair_make(LAM, 0, SUB, l1);

  Term sumLft = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l0))), SUB);
  Term sumRgt = pair_make(APP, 0, term_new(VAR, 0, port(1, term_loc(l1))), s);
  Pairs pairs;
  pairs.count = 0;
  swapStore(port(1, term_loc(s)), term_new(VAR, 0, port(2, term_loc(sumLft))), &pairs);
  store_redex(&pairs, sumRgt, sum);
  store_redex(&pairs, args, l0);
  store_redex(&pairs, sumLft, sum);
  link_redexes(&pairs);
  return;
}
Term sumNode = new_ref(sumNodeFn);

void sumFn(Term ref, Term args) {
  Term a1 = pair_make(APP, 0, sumLeaf, SUB);
  Term a0 = pair_make(APP, 0, sumNode, a1);
  Term l = pair_make(LAM, 0, a0, term_new(VAR, 0, port(2, term_loc(a1))));

  interact(args, l);
  return;
}
Term sum = new_ref(sumFn);

struct timeval startTime, endTime;
double elapsed;
int height;
u64 expected;

void endFn(Term ref, Term args) {
  Pairs pairs;
  pairs.count = 0;
  Term rTrm = defer(ref, args, &pairs);
  link_redexes(&pairs);
  switch(term_tag(rTrm)) {
  case VAR:
    return;
    break;

  case I60: 
    gettimeofday(&endTime, NULL);
    elapsed = (endTime.tv_sec - startTime.tv_sec) +
      (endTime.tv_usec - startTime.tv_usec) / 1000000.0;

    pair_free(term_loc(args));
    print_term("result", rTrm);
    printf("exptd: %lu\n", expected);
    if (expected != get_i60(rTrm)) {
      abort();
    }
    /*
    u64 allocCount = atomic_load_explicit(&alloced, memory_order_relaxed);
    if (allocCount != 0) {
      printf("alloced pairs: %lu\n", allocCount);
      // print_free_list();
      exit(1);
    }
    // */
#ifndef SINGLE_THREAD
    u64 currTop;
    do {
      currTop = atomic_exchange_explicit(&RBAG_END, LOCK_REDEX_STACK, memory_order_relaxed);
      switch (currTop) {
      case LOCK_REDEX_STACK:
	break;

      default:
	if (1) {
#ifdef SAFETY
	  // Check if there's space in the bag
	  if (currTop + threadCount - 1 > RBAG_SIZE) {
	    fprintf(stderr, "Error: Redex bag is full. RBAG_END=%lu, RBAG_SIZE=%lu\n",
		    currTop, RBAG_SIZE);
	    abort();
	  }
#endif
	  u64 newTop = currTop;
	  for (int i = 0; i < threadCount; i++, newTop += 2) {
	    RBAG_BUFF[newTop] = 0;
	    RBAG_BUFF[newTop + 1] = 0;
	  }

	  if (currTop == 0) {
	    pthread_mutex_lock(&redex_mutex);
	    pthread_cond_signal(&redex_cond);
	    pthread_mutex_unlock(&redex_mutex);
	  }
	  atomic_store_explicit(&RBAG_END, newTop, memory_order_relaxed);
	}
      }
    } while (currTop == LOCK_REDEX_STACK);
#endif
    break;

  default:
    print_term("bad result", rTrm);
    BOOM("in 'end'");
    break;
  }
  return;
}
Term end = new_ref(endFn);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("\ncorrect command line is \"bench <height> <threads>\"\n");
    exit(1);
  }
  
  // Initialize the VM with some memory
  hvm_init((u64)3 * (u64)(1 << 30));

  height = atoi(argv[1]);
  if (height < 0) {
    printf("\nInvalid height: %d\n", height);
    exit(1);
  } else {
    printf("\nHeight: %d\n", height);
  }
  i64 wtf = 1 << height;
  expected = (wtf - 1) * wtf / 2;

#ifndef SINGLE_THREAD
  threadCount = atoi(argv[2]);
  printf("Threads: %d\n", threadCount);
#else
  printf("Running single thread\n");
#endif

  for(int reps = 0; reps < 1; reps++) {
    hvm_reset();

    gettimeofday(&startTime, NULL);

    Term a1 = pair_make(APP, 0, new_i60(0), SUB);
    Term a0 = pair_make(APP, 0, new_i60(height), a1);

    Term n = term_new(VAR, 0, port(2, term_loc(a1)));
    Term a = pair_make(APP, 0, n, SUB);
    Term a3 = pair_make(APP, 0, term_new(VAR, 0, port(2, term_loc(a))), SUB);

    RBAG_BUFF[0] = a0;
    RBAG_BUFF[1] = make;
    RBAG_BUFF[2] = a;
    RBAG_BUFF[3] = sum;
    RBAG_BUFF[4] = a3;
    RBAG_BUFF[5] = end;
    atomic_store_explicit(&RBAG_END, 6, memory_order_relaxed);

    spawn_threads();
    u64 *res;
    u64 interactions = rdxCount;
    for(int i = 0; i < threadCount; i++) {
      pthread_join(threads[i], (void **)&res);
      interactions += *res;
      printf("rdxCount: %lu\n", *res);
    }
    u64 rdxs = atomic_load_explicit(&reduced, memory_order_relaxed);
    printf("interactions: %lu %lu\n", rdxs, interactions);
    printf("MIPS: %f\n", interactions / elapsed / 1000000);
    printf("elapsed: %f\n", elapsed);
    printf("Heap needed: %lu\n", atomic_load_explicit(&RNOD_END, memory_order_relaxed));
    i64 gAlloced = atomic_load_explicit(&glblAlloced, memory_order_relaxed);
    i64 rnod = atomic_load_explicit(&RNOD_END, memory_order_relaxed);
    if (glblAlloced != 0) {
      printf("final alloced: %ld\n", gAlloced);
      printf("RNOD_END: %ld\n", rnod);
      print_free_list();
      print_buff(0, rnod); 
      BOOM("");
    }
  }

  return 0;
}

char *refName(Term ref) {
  if (ref == leaf)
    return "leaf";
  else if (ref == node)
    return "node";
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
