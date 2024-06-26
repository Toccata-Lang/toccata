//#include <dlfcn.h>
// #include <inttypes.h>
// #include <math.h>
// #include <pthread.h>
// #include <stdatomic.h>
// #include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include "hvm.h"

#define MAG 0x00000141

// The Call Interaction.
bool CALL_main__C1(TM *tm, Port a, Port b);
bool CALL_sum(TM *tm, Port a, Port b);
bool CALL_main(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0 || !n1) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n1, new_pair(new_ref(CALL_main__C1),new_port(VAR,v0)));
  node_create(n0, new_pair(new_num(MAG),new_port(CON,n1)));
  link(tm, new_ref(CALL_sum), new_port(CON,n0));
  return TRUE;
}

bool CALL_down__C0(TM *tm, Port a, Port b);
bool CALL_down(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_down__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_flow(TM *tm, Port a, Port b);
bool CALL_down__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v7), k16);
  } else {
    k16 = new_port(VAR,v7);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v6), k15);
  } else {
    k15 = new_port(VAR,v6);
  }
  if (!k13) {
    node_create(n6, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n6), k12);
    } else {
      k12 = new_port(CON,n6);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k11;
    k19 = k11;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v5), k19);
  } else {
    k19 = new_port(VAR,v5);
  }
  if (k18 != NONE) {
    link(tm, new_port(VAR,v4), k18);
  } else {
    k18 = new_port(VAR,v4);
  }
  if (!k17) {
    node_create(n5, new_pair(k18,k19));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n5), k11);
    } else {
      k11 = new_port(DUP,n5);
    }
  }
  if (!k9) {
    node_create(n4, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n4), k8);
    } else {
      k8 = new_port(CON,n4);
    }
  }
  bool k20 = 0;
  Pair k21 = emptyPair;
  Port k22 = NONE;
  Port k23 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k20 = 1;
    k21 = node_take(k7);
    k22 = k21.fst;
    k23 = k21.snd;
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,v3), k23);
  } else {
    k23 = new_port(VAR,v3);
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v2), k22);
  } else {
    k22 = new_port(VAR,v2);
  }
  if (!k20) {
    node_create(n3, new_pair(k22,k23));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k24 = 0;
  Port k25 = NONE;
  Port k26 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = k3;
    k26 = k3;
  }
  if (k26 != NONE) {
    link(tm, new_port(VAR,v1), k26);
  } else {
    k26 = new_port(VAR,v1);
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v0), k25);
  } else {
    k25 = new_port(VAR,v0);
  }
  if (!k24) {
    node_create(n1, new_pair(k25,k26));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n9, new_pair(new_port(VAR,v2),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v4),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_flow), new_port(CON,n7));
  node_create(nc, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nb, new_pair(new_port(VAR,v5),new_port(CON,nc)));
  node_create(na, new_pair(new_port(VAR,v1),new_port(CON,nb)));
  link(tm, new_ref(CALL_flow), new_port(CON,na));
  return TRUE;
}

bool CALL_flow__C0(TM *tm, Port a, Port b);
bool CALL_flow(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_flow__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp(TM *tm, Port a, Port b);
bool CALL_flow__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v6), k12);
  } else {
    k12 = new_port(VAR,v6);
  }
  bool k13 = 0;
  Port k14 = NONE;
  Port k15 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = k11;
    k15 = k11;
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v5), k15);
  } else {
    k15 = new_port(VAR,v5);
  }
  if (k14 != NONE) {
    link(tm, new_port(VAR,v4), k14);
  } else {
    k14 = new_port(VAR,v4);
  }
  if (!k13) {
    node_create(n6, new_pair(k14,k15));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n6), k11);
    } else {
      k11 = new_port(DUP,n6);
    }
  }
  if (!k9) {
    node_create(n5, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n5), k8);
    } else {
      k8 = new_port(CON,n5);
    }
  }
  bool k16 = 0;
  Pair k17 = emptyPair;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k16 = 1;
    k17 = node_take(k7);
    k18 = k17.fst;
    k19 = k17.snd;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v3), k19);
  } else {
    k19 = new_port(VAR,v3);
  }
  if (k18 != NONE) {
    link(tm, new_port(VAR,v2), k18);
  } else {
    k18 = new_port(VAR,v2);
  }
  if (!k16) {
    node_create(n4, new_pair(k18,k19));
    if (k7 != NONE) {
      link(tm, new_port(CON,n4), k7);
    } else {
      k7 = new_port(CON,n4);
    }
  }
  if (!k5) {
    node_create(n3, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n3), k4);
    } else {
      k4 = new_port(CON,n3);
    }
  }
  bool k20 = 0;
  Port k21 = NONE;
  Port k22 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = k3;
    k22 = k3;
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v1), k22);
  } else {
    k22 = new_port(VAR,v1);
  }
  bool k23 = 0;
  Port k24 = NONE;
  // fast oper
  if (get_tag(k21) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k23 = 1;
    k24 = new_num(operate(k21, new_num(0x00000024)));
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,v0), k24);
  } else {
    k24 = new_port(VAR,v0);
  }
  if (!k23) {
    node_create(n2, new_pair(new_num(0x00000024),k24));
    if (k21 != NONE) {
      link(tm, new_port(OPR, n2), k21);
    } else {
      k21 = new_port(OPR, n2);
    }
  }
  if (!k20) {
    node_create(n1, new_pair(k21,k22));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n9, new_pair(new_port(VAR,v7),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v4),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_down), new_port(CON,n7));
  node_create(nd, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nc, new_pair(new_port(VAR,v2),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v5),new_port(CON,nc)));
  node_create(na, new_pair(new_port(VAR,v1),new_port(CON,nb)));
  link(tm, new_ref(CALL_warp), new_port(CON,na));
  return TRUE;
}

bool CALL_gen__bend0(TM *tm, Port a, Port b);
bool CALL_gen(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  if (k4 != NONE) {
    link(tm, new_port(VAR,v1), k4);
  } else {
    k4 = new_port(VAR,v1);
  }
  if (k3 != NONE) {
    link(tm, new_port(VAR,v0), k3);
  } else {
    k3 = new_port(VAR,v0);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n2, new_pair(new_num(0x00000001),new_port(VAR,v1)));
  node_create(n1, new_pair(new_port(VAR,v0),new_port(CON,n2)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,n1));
  return TRUE;
}

bool CALL_gen__bend0__C0(TM *tm, Port a, Port b);
bool CALL_gen__bend0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  if (k4 != NONE) {
    link(tm, new_port(VAR,v2), k4);
  } else {
    k4 = new_port(VAR,v2);
  }
  bool k5 = 0;
  Port k6 = NONE;
  Port k7 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k5 = 1;
    k6 = k3;
    k7 = k3;
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  node_create(n5, new_pair(new_port(VAR,v0),new_port(VAR,v0)));
  node_create(n4, new_pair(erase,new_port(CON,n5)));
  node_create(n3, new_pair(new_port(CON,n4),new_ref(CALL_gen__bend0__C0)));
  node_create(n6, new_pair(new_port(VAR,v1),new_port(VAR,v2)));
  node_create(n2, new_pair(new_port(CON,n3),new_port(CON,n6)));
  if (k6 != NONE) {
    link(tm, new_port(SWI,n2), k6);
  } else {
    k6 = new_port(SWI,n2);
  }
  if (!k5) {
    node_create(n1, new_pair(k6,k7));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_gen__bend0__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v5), k16);
  } else {
    k16 = new_port(VAR,v5);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v4), k15);
  } else {
    k15 = new_port(VAR,v4);
  }
  if (!k13) {
    node_create(na, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,na), k12);
    } else {
      k12 = new_port(CON,na);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k11) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k11;
    k19 = k11;
  }
  bool k20 = 0;
  Port k21 = NONE;
  // fast oper
  if (get_tag(k19) == NUM && get_tag(new_num(0x00000047)) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = new_num(operate(k19, new_num(0x00000047)));
  }
  if (k21 != NONE) {
    link(tm, new_port(VAR,v3), k21);
  } else {
    k21 = new_port(VAR,v3);
  }
  if (!k20) {
    node_create(n9, new_pair(new_num(0x00000047),k21));
    if (k19 != NONE) {
      link(tm, new_port(OPR, n9), k19);
    } else {
      k19 = new_port(OPR, n9);
    }
  }
  bool k22 = 0;
  Port k23 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x00000047)) == NUM) {
    tm->itrs += 1;
    k22 = 1;
    k23 = new_num(operate(k18, new_num(0x00000047)));
  }
  bool k24 = 0;
  Port k25 = NONE;
  // fast oper
  if (get_tag(k23) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = new_num(operate(k23, new_num(0x00000024)));
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v2), k25);
  } else {
    k25 = new_port(VAR,v2);
  }
  if (!k24) {
    node_create(n8, new_pair(new_num(0x00000024),k25));
    if (k23 != NONE) {
      link(tm, new_port(OPR, n8), k23);
    } else {
      k23 = new_port(OPR, n8);
    }
  }
  if (!k22) {
    node_create(n7, new_pair(new_num(0x00000047),k23));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n7), k18);
    } else {
      k18 = new_port(OPR, n7);
    }
  }
  if (!k17) {
    node_create(n6, new_pair(k18,k19));
    if (k11 != NONE) {
      link(tm, new_port(DUP,n6), k11);
    } else {
      k11 = new_port(DUP,n6);
    }
  }
  if (!k9) {
    node_create(n5, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n5), k8);
    } else {
      k8 = new_port(CON,n5);
    }
  }
  bool k26 = 0;
  Port k27 = NONE;
  Port k28 = NONE;
  // fast copy
  if (get_tag(k7) == NUM) {
    tm->itrs += 1;
    k26 = 1;
    k27 = k7;
    k28 = k7;
  }
  bool k29 = 0;
  Port k30 = NONE;
  // fast oper
  if (get_tag(k28) == NUM && get_tag(new_num(0x00000026)) == NUM) {
    tm->itrs += 1;
    k29 = 1;
    k30 = new_num(operate(k28, new_num(0x00000026)));
  }
  if (k30 != NONE) {
    link(tm, new_port(VAR,v1), k30);
  } else {
    k30 = new_port(VAR,v1);
  }
  if (!k29) {
    node_create(n4, new_pair(new_num(0x00000026),k30));
    if (k28 != NONE) {
      link(tm, new_port(OPR, n4), k28);
    } else {
      k28 = new_port(OPR, n4);
    }
  }
  bool k31 = 0;
  Port k32 = NONE;
  // fast oper
  if (get_tag(k27) == NUM && get_tag(new_num(0x00000026)) == NUM) {
    tm->itrs += 1;
    k31 = 1;
    k32 = new_num(operate(k27, new_num(0x00000026)));
  }
  if (k32 != NONE) {
    link(tm, new_port(VAR,v0), k32);
  } else {
    k32 = new_port(VAR,v0);
  }
  if (!k31) {
    node_create(n3, new_pair(new_num(0x00000026),k32));
    if (k27 != NONE) {
      link(tm, new_port(OPR, n3), k27);
    } else {
      k27 = new_port(OPR, n3);
    }
  }
  if (!k26) {
    node_create(n2, new_pair(k27,k28));
    if (k7 != NONE) {
      link(tm, new_port(DUP,n2), k7);
    } else {
      k7 = new_port(DUP,n2);
    }
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  // fast void
  if (get_tag(k3) == ERA || get_tag(k3) == NUM) {
    tm->itrs += 1;
  } else {
    if (k3 != NONE) {
      link(tm, erase, k3);
    } else {
      k3 = erase;
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nc, new_pair(new_port(VAR,v2),new_port(VAR,v4)));
  node_create(nb, new_pair(new_port(VAR,v0),new_port(CON,nc)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,nb));
  node_create(ne, new_pair(new_port(VAR,v3),new_port(VAR,v5)));
  node_create(nd, new_pair(new_port(VAR,v1),new_port(CON,ne)));
  link(tm, new_ref(CALL_gen__bend0), new_port(CON,nd));
  return TRUE;
}

bool CALL_main__C0(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n0, new_pair(new_num(MAG),new_port(VAR,v0)));
  link(tm, new_ref(CALL_gen), new_port(CON,n0));
  return TRUE;
}

bool CALL_sort(TM *tm, Port a, Port b);
bool CALL_main__C1(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  if (b != NONE) {
    link(tm, new_port(VAR,v0), b);
  } else {
    b = new_port(VAR,v0);
  }
  node_create(n2, new_pair(new_ref(CALL_main__C0),new_port(VAR,v0)));
  node_create(n1, new_pair(new_num(0x00000001),new_port(CON,n2)));
  node_create(n0, new_pair(new_num(MAG),new_port(CON,n1)));
  link(tm, new_ref(CALL_sort), new_port(CON,n0));
  return TRUE;
}

bool CALL_sort__C0(TM *tm, Port a, Port b);
bool CALL_sort(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n8, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n8), k8);
    } else {
      k8 = new_port(CON,n8);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n7, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n7), k4);
    } else {
      k4 = new_port(CON,n7);
    }
  }
  node_create(n4, new_pair(erase,new_port(VAR,v0)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n2, new_pair(new_port(CON,n3),new_ref(CALL_sort__C0)));
  node_create(n6, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n5, new_pair(new_port(VAR,v1),new_port(CON,n6)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n5)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_sort__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port v8 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  Port nf = node_alloc(tm, &nl);
  Port n10 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !v8 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne || !nf || !n10) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  vars_create(v8, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v6), k12);
  } else {
    k12 = new_port(VAR,v6);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v5), k11);
  } else {
    k11 = new_port(VAR,v5);
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k7);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v4), k16);
  } else {
    k16 = new_port(VAR,v4);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v3), k15);
  } else {
    k15 = new_port(VAR,v3);
  }
  if (!k13) {
    node_create(n5, new_pair(k15,k16));
    if (k7 != NONE) {
      link(tm, new_port(CON,n5), k7);
    } else {
      k7 = new_port(CON,n5);
    }
  }
  if (!k5) {
    node_create(n4, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n4), k4);
    } else {
      k4 = new_port(CON,n4);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k3;
    k19 = k3;
  }
  bool k20 = 0;
  Port k21 = NONE;
  Port k22 = NONE;
  // fast copy
  if (get_tag(k19) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = k19;
    k22 = k19;
  }
  if (k22 != NONE) {
    link(tm, new_port(VAR,v2), k22);
  } else {
    k22 = new_port(VAR,v2);
  }
  if (k21 != NONE) {
    link(tm, new_port(VAR,v1), k21);
  } else {
    k21 = new_port(VAR,v1);
  }
  if (!k20) {
    node_create(n3, new_pair(k21,k22));
    if (k19 != NONE) {
      link(tm, new_port(DUP,n3), k19);
    } else {
      k19 = new_port(DUP,n3);
    }
  }
  bool k23 = 0;
  Port k24 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x00000024)) == NUM) {
    tm->itrs += 1;
    k23 = 1;
    k24 = new_num(operate(k18, new_num(0x00000024)));
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,v0), k24);
  } else {
    k24 = new_port(VAR,v0);
  }
  if (!k23) {
    node_create(n2, new_pair(new_num(0x00000024),k24));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n2), k18);
    } else {
      k18 = new_port(OPR, n2);
    }
  }
  if (!k17) {
    node_create(n1, new_pair(k18,k19));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(na, new_pair(new_port(VAR,v7),new_port(VAR,v8)));
  node_create(n9, new_pair(new_port(CON,na),new_port(VAR,v6)));
  node_create(n8, new_pair(new_port(VAR,v5),new_port(CON,n9)));
  node_create(n7, new_pair(new_port(VAR,v0),new_port(CON,n8)));
  link(tm, new_ref(CALL_flow), new_port(CON,n7));
  node_create(nd, new_pair(new_port(VAR,v3),new_port(VAR,v7)));
  node_create(nc, new_pair(new_num(0x00000001),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v1),new_port(CON,nc)));
  link(tm, new_ref(CALL_sort), new_port(CON,nb));
  node_create(n10, new_pair(new_port(VAR,v4),new_port(VAR,v8)));
  node_create(nf, new_pair(new_num(0x00000021),new_port(CON,n10)));
  node_create(ne, new_pair(new_port(VAR,v2),new_port(CON,nf)));
  link(tm, new_ref(CALL_sort), new_port(CON,ne));
  return TRUE;
}

bool CALL_sum__C0(TM *tm, Port a, Port b);
bool CALL_sum(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2 || !n3) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k5 = NONE;
  Port k3 = NONE;
  Port k4 = NONE;
  //fast switch
  if (get_tag(b) == CON) {
    k2 = node_load(b);
    k5 = enter(k2.fst);
    if (get_tag(k5) == NUM) {
      tm->itrs += 3;
      vars_take(v1);
      k1 = 1;
      if (get_u24(get_num(k5)) == 0) {
        node_take(b);
        k3 = k2.snd;
        k4 = erase;
      } else {
        node_store(b, new_pair(new_num(new_u24(get_u24(get_num(k5))-1)), k2.snd));
        k3 = erase;
        k4 = b;
      }
    } else {
      node_store(b, new_pair(k5,k2.snd));
    }
  }
  bool k6 = 0;
  Pair k7 = emptyPair;
  Port k8 = NONE;
  Port k9 = NONE;
  // fast anni
  if (get_tag(k3) == CON && !isEmpty(node_load(k3))) {
    tm->itrs += 1;
    k6 = 1;
    k7 = node_take(k3);
    k8 = k7.fst;
    k9 = k7.snd;
  }
  if (k9 != NONE) {
    link(tm, new_port(VAR,v0), k9);
  } else {
    k9 = new_port(VAR,v0);
  }
  if (k8 != NONE) {
    link(tm, new_port(VAR,v0), k8);
  } else {
    k8 = new_port(VAR,v0);
  }
  if (!k6) {
    node_create(n3, new_pair(k8,k9));
    if (k3 != NONE) {
      link(tm, new_port(CON,n3), k3);
    } else {
      k3 = new_port(CON,n3);
    }
  }
  if (k4 != NONE) {
    link(tm, new_ref(CALL_sum__C0), k4);
  } else {
    k4 = new_ref(CALL_sum__C0);
  }
  if (!k1) {
    node_create(n0, new_pair(new_port(SWI,n1),new_port(VAR,v1)));
    node_create(n1, new_pair(new_port(CON,n2),new_port(VAR,v1)));
    node_create(n2, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON, n0), b);
    } else {
      b = new_port(CON, n0);
    }
  }
  return TRUE;
}

bool CALL_sum__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  if (k8 != NONE) {
    link(tm, new_port(VAR,v4), k8);
  } else {
    k8 = new_port(VAR,v4);
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k7);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v3), k12);
  } else {
    k12 = new_port(VAR,v3);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v2), k11);
  } else {
    k11 = new_port(VAR,v2);
  }
  if (!k9) {
    node_create(n3, new_pair(k11,k12));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k13 = 0;
  Port k14 = NONE;
  Port k15 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = k3;
    k15 = k3;
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v1), k15);
  } else {
    k15 = new_port(VAR,v1);
  }
  if (k14 != NONE) {
    link(tm, new_port(VAR,v0), k14);
  } else {
    k14 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n1, new_pair(k14,k15));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(n7, new_pair(new_port(VAR,v5),new_port(VAR,v4)));
  node_create(n6, new_pair(new_num(0x00000080),new_port(OPR,n7)));
  node_create(n5, new_pair(new_port(VAR,v2),new_port(OPR,n6)));
  node_create(n4, new_pair(new_port(VAR,v0),new_port(CON,n5)));
  link(tm, new_ref(CALL_sum), new_port(CON,n4));
  node_create(n9, new_pair(new_port(VAR,v3),new_port(VAR,v5)));
  node_create(n8, new_pair(new_port(VAR,v1),new_port(CON,n9)));
  link(tm, new_ref(CALL_sum), new_port(CON,n8));
  return TRUE;
}

bool CALL_swap__C0(TM *tm, Port a, Port b);
bool CALL_swap__C1(TM *tm, Port a, Port b);
bool CALL_swap(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v2), k12);
  } else {
    k12 = new_port(VAR,v2);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v0), k11);
  } else {
    k11 = new_port(VAR,v0);
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  if (!k5) {
    node_create(n5, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n5), k4);
    } else {
      k4 = new_port(CON,n5);
    }
  }
  node_create(n2, new_pair(new_ref(CALL_swap__C0),new_ref(CALL_swap__C1)));
  node_create(n4, new_pair(new_port(VAR,v1),new_port(VAR,v2)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n3)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_swap__C0(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v0), k12);
  } else {
    k12 = new_port(VAR,v0);
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n2, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n2), k8);
    } else {
      k8 = new_port(CON,n2);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v1), k7);
  } else {
    k7 = new_port(VAR,v1);
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  if (k3 != NONE) {
    link(tm, new_port(VAR,v0), k3);
  } else {
    k3 = new_port(VAR,v0);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_swap__C1(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !n0 || !n1 || !n2 || !n3) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v1), k16);
  } else {
    k16 = new_port(VAR,v1);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v0), k15);
  } else {
    k15 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n3, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n3), k12);
    } else {
      k12 = new_port(CON,n3);
    }
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n2, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n2), k8);
    } else {
      k8 = new_port(CON,n2);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v0), k7);
  } else {
    k7 = new_port(VAR,v0);
  }
  if (!k5) {
    node_create(n1, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n1), k4);
    } else {
      k4 = new_port(CON,n1);
    }
  }
  // fast void
  if (get_tag(k3) == ERA || get_tag(k3) == NUM) {
    tm->itrs += 1;
  } else {
    if (k3 != NONE) {
      link(tm, erase, k3);
    } else {
      k3 = erase;
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp__C0(TM *tm, Port a, Port b);
bool CALL_warp__C1(TM *tm, Port a, Port b);
bool CALL_warp(TM *tm, Port a, Port b) {
  if (get_tag(b) == DUP) {
    return ERAS(tm, a, b);
  }
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v3), k16);
  } else {
    k16 = new_port(VAR,v3);
  }
  if (k15 != NONE) {
    link(tm, new_port(VAR,v0), k15);
  } else {
    k15 = new_port(VAR,v0);
  }
  if (!k13) {
    node_create(n8, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n8), k12);
    } else {
      k12 = new_port(CON,n8);
    }
  }
  if (k11 != NONE) {
    link(tm, new_port(VAR,v1), k11);
  } else {
    k11 = new_port(VAR,v1);
  }
  if (!k9) {
    node_create(n7, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n7), k8);
    } else {
      k8 = new_port(CON,n7);
    }
  }
  if (k7 != NONE) {
    link(tm, new_port(VAR,v2), k7);
  } else {
    k7 = new_port(VAR,v2);
  }
  if (!k5) {
    node_create(n6, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n6), k4);
    } else {
      k4 = new_port(CON,n6);
    }
  }
  node_create(n2, new_pair(new_ref(CALL_warp__C0),new_ref(CALL_warp__C1)));
  node_create(n5, new_pair(new_port(VAR,v2),new_port(VAR,v3)));
  node_create(n4, new_pair(new_port(VAR,v1),new_port(CON,n5)));
  node_create(n3, new_pair(new_port(VAR,v0),new_port(CON,n4)));
  node_create(n1, new_pair(new_port(CON,n2),new_port(CON,n3)));
  if (k3 != NONE) {
    link(tm, new_port(SWI,n1), k3);
  } else {
    k3 = new_port(SWI,n1);
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  return TRUE;
}

bool CALL_warp__C0(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  if (k12 != NONE) {
    link(tm, new_port(VAR,v5), k12);
  } else {
    k12 = new_port(VAR,v5);
  }
  bool k13 = 0;
  Port k14 = NONE;
  // fast oper
  if (get_tag(k11) == NUM && get_tag(new_num(0x00000080)) == NUM) {
    tm->itrs += 1;
    k13 = 1;
    k14 = new_num(operate(k11, new_num(0x00000080)));
  }
  bool k15 = 0;
  Port k16 = NONE;
  // fast oper
  if (get_tag(k14) == NUM && get_tag(new_port(VAR,v2)) == NUM) {
    tm->itrs += 1;
    k15 = 1;
    k16 = new_num(operate(k14, new_port(VAR,v2)));
  }
  if (k16 != NONE) {
    link(tm, new_port(VAR,v4), k16);
  } else {
    k16 = new_port(VAR,v4);
  }
  if (!k15) {
    node_create(n8, new_pair(new_port(VAR,v2),k16));
    if (k14 != NONE) {
      link(tm, new_port(OPR, n8), k14);
    } else {
      k14 = new_port(OPR, n8);
    }
  }
  if (!k13) {
    node_create(n7, new_pair(new_num(0x00000080),k14));
    if (k11 != NONE) {
      link(tm, new_port(OPR, n7), k11);
    } else {
      k11 = new_port(OPR, n7);
    }
  }
  if (!k9) {
    node_create(n6, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n6), k8);
    } else {
      k8 = new_port(CON,n6);
    }
  }
  bool k17 = 0;
  Port k18 = NONE;
  Port k19 = NONE;
  // fast copy
  if (get_tag(k7) == NUM) {
    tm->itrs += 1;
    k17 = 1;
    k18 = k7;
    k19 = k7;
  }
  if (k19 != NONE) {
    link(tm, new_port(VAR,v3), k19);
  } else {
    k19 = new_port(VAR,v3);
  }
  bool k20 = 0;
  Port k21 = NONE;
  // fast oper
  if (get_tag(k18) == NUM && get_tag(new_num(0x000001e0)) == NUM) {
    tm->itrs += 1;
    k20 = 1;
    k21 = new_num(operate(k18, new_num(0x000001e0)));
  }
  bool k22 = 0;
  Port k23 = NONE;
  // fast oper
  if (get_tag(k21) == NUM && get_tag(new_port(VAR,v0)) == NUM) {
    tm->itrs += 1;
    k22 = 1;
    k23 = new_num(operate(k21, new_port(VAR,v0)));
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,v2), k23);
  } else {
    k23 = new_port(VAR,v2);
  }
  if (!k22) {
    node_create(n5, new_pair(new_port(VAR,v0),k23));
    if (k21 != NONE) {
      link(tm, new_port(OPR, n5), k21);
    } else {
      k21 = new_port(OPR, n5);
    }
  }
  if (!k20) {
    node_create(n4, new_pair(new_num(0x000001e0),k21));
    if (k18 != NONE) {
      link(tm, new_port(OPR, n4), k18);
    } else {
      k18 = new_port(OPR, n4);
    }
  }
  if (!k17) {
    node_create(n3, new_pair(k18,k19));
    if (k7 != NONE) {
      link(tm, new_port(DUP,n3), k7);
    } else {
      k7 = new_port(DUP,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k24 = 0;
  Port k25 = NONE;
  Port k26 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k24 = 1;
    k25 = k3;
    k26 = k3;
  }
  if (k26 != NONE) {
    link(tm, new_port(VAR,v1), k26);
  } else {
    k26 = new_port(VAR,v1);
  }
  if (k25 != NONE) {
    link(tm, new_port(VAR,v0), k25);
  } else {
    k25 = new_port(VAR,v0);
  }
  if (!k24) {
    node_create(n1, new_pair(k25,k26));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nb, new_pair(new_port(VAR,v1),new_port(VAR,v5)));
  node_create(na, new_pair(new_port(VAR,v3),new_port(CON,nb)));
  node_create(n9, new_pair(new_port(VAR,v4),new_port(CON,na)));
  link(tm, new_ref(CALL_swap), new_port(CON,n9));
  return TRUE;
}

bool CALL_warp__C1(TM *tm, Port a, Port b) {
  u32 vl = 0;
  u32 nl = 0;
  Port v0 = vars_alloc(tm, &vl);
  Port v1 = vars_alloc(tm, &vl);
  Port v2 = vars_alloc(tm, &vl);
  Port v3 = vars_alloc(tm, &vl);
  Port v4 = vars_alloc(tm, &vl);
  Port v5 = vars_alloc(tm, &vl);
  Port v6 = vars_alloc(tm, &vl);
  Port v7 = vars_alloc(tm, &vl);
  Port v8 = vars_alloc(tm, &vl);
  Port v9 = vars_alloc(tm, &vl);
  Port va = vars_alloc(tm, &vl);
  Port vb = vars_alloc(tm, &vl);
  Port n0 = node_alloc(tm, &nl);
  Port n1 = node_alloc(tm, &nl);
  Port n2 = node_alloc(tm, &nl);
  Port n3 = node_alloc(tm, &nl);
  Port n4 = node_alloc(tm, &nl);
  Port n5 = node_alloc(tm, &nl);
  Port n6 = node_alloc(tm, &nl);
  Port n7 = node_alloc(tm, &nl);
  Port n8 = node_alloc(tm, &nl);
  Port n9 = node_alloc(tm, &nl);
  Port na = node_alloc(tm, &nl);
  Port nb = node_alloc(tm, &nl);
  Port nc = node_alloc(tm, &nl);
  Port nd = node_alloc(tm, &nl);
  Port ne = node_alloc(tm, &nl);
  Port nf = node_alloc(tm, &nl);
  Port n10 = node_alloc(tm, &nl);
  Port n11 = node_alloc(tm, &nl);
  Port n12 = node_alloc(tm, &nl);
  Port n13 = node_alloc(tm, &nl);
  Port n14 = node_alloc(tm, &nl);
  if (0 || !v0 || !v1 || !v2 || !v3 || !v4 || !v5 || !v6 || !v7 || !v8 || !v9 || !va || !vb || !n0 || !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n9 || !na || !nb || !nc || !nd || !ne || !nf || !n10 || !n11 || !n12 || !n13 || !n14) {
    return FALSE;
  }
  vars_create(v0, NONE);
  vars_create(v1, NONE);
  vars_create(v2, NONE);
  vars_create(v3, NONE);
  vars_create(v4, NONE);
  vars_create(v5, NONE);
  vars_create(v6, NONE);
  vars_create(v7, NONE);
  vars_create(v8, NONE);
  vars_create(v9, NONE);
  vars_create(va, NONE);
  vars_create(vb, NONE);
  bool k1 = 0;
  Pair k2 = emptyPair;
  Port k3 = NONE;
  Port k4 = NONE;
  // fast anni
  if (get_tag(b) == CON && !isEmpty(node_load(b))) {
    tm->itrs += 1;
    k1 = 1;
    k2 = node_take(b);
    k3 = k2.fst;
    k4 = k2.snd;
  }
  bool k5 = 0;
  Pair k6 = emptyPair;
  Port k7 = NONE;
  Port k8 = NONE;
  // fast anni
  if (get_tag(k4) == CON && !isEmpty(node_load(k4))) {
    tm->itrs += 1;
    k5 = 1;
    k6 = node_take(k4);
    k7 = k6.fst;
    k8 = k6.snd;
  }
  bool k9 = 0;
  Pair k10 = emptyPair;
  Port k11 = NONE;
  Port k12 = NONE;
  // fast anni
  if (get_tag(k8) == CON && !isEmpty(node_load(k8))) {
    tm->itrs += 1;
    k9 = 1;
    k10 = node_take(k8);
    k11 = k10.fst;
    k12 = k10.snd;
  }
  bool k13 = 0;
  Pair k14 = emptyPair;
  Port k15 = NONE;
  Port k16 = NONE;
  // fast anni
  if (get_tag(k12) == CON && !isEmpty(node_load(k12))) {
    tm->itrs += 1;
    k13 = 1;
    k14 = node_take(k12);
    k15 = k14.fst;
    k16 = k14.snd;
  }
  bool k17 = 0;
  Pair k18 = emptyPair;
  Port k19 = NONE;
  Port k20 = NONE;
  // fast anni
  if (get_tag(k16) == CON && !isEmpty(node_load(k16))) {
    tm->itrs += 1;
    k17 = 1;
    k18 = node_take(k16);
    k19 = k18.fst;
    k20 = k18.snd;
  }
  bool k21 = 0;
  Pair k22 = emptyPair;
  Port k23 = NONE;
  Port k24 = NONE;
  // fast anni
  if (get_tag(k20) == CON && !isEmpty(node_load(k20))) {
    tm->itrs += 1;
    k21 = 1;
    k22 = node_take(k20);
    k23 = k22.fst;
    k24 = k22.snd;
  }
  if (k24 != NONE) {
    link(tm, new_port(VAR,vb), k24);
  } else {
    k24 = new_port(VAR,vb);
  }
  if (k23 != NONE) {
    link(tm, new_port(VAR,va), k23);
  } else {
    k23 = new_port(VAR,va);
  }
  if (!k21) {
    node_create(na, new_pair(k23,k24));
    if (k20 != NONE) {
      link(tm, new_port(CON,na), k20);
    } else {
      k20 = new_port(CON,na);
    }
  }
  bool k25 = 0;
  Pair k26 = emptyPair;
  Port k27 = NONE;
  Port k28 = NONE;
  // fast anni
  if (get_tag(k19) == CON && !isEmpty(node_load(k19))) {
    tm->itrs += 1;
    k25 = 1;
    k26 = node_take(k19);
    k27 = k26.fst;
    k28 = k26.snd;
  }
  if (k28 != NONE) {
    link(tm, new_port(VAR,v9), k28);
  } else {
    k28 = new_port(VAR,v9);
  }
  if (k27 != NONE) {
    link(tm, new_port(VAR,v8), k27);
  } else {
    k27 = new_port(VAR,v8);
  }
  if (!k25) {
    node_create(n9, new_pair(k27,k28));
    if (k19 != NONE) {
      link(tm, new_port(CON,n9), k19);
    } else {
      k19 = new_port(CON,n9);
    }
  }
  if (!k17) {
    node_create(n8, new_pair(k19,k20));
    if (k16 != NONE) {
      link(tm, new_port(CON,n8), k16);
    } else {
      k16 = new_port(CON,n8);
    }
  }
  bool k29 = 0;
  Port k30 = NONE;
  Port k31 = NONE;
  // fast copy
  if (get_tag(k15) == NUM) {
    tm->itrs += 1;
    k29 = 1;
    k30 = k15;
    k31 = k15;
  }
  if (k31 != NONE) {
    link(tm, new_port(VAR,v7), k31);
  } else {
    k31 = new_port(VAR,v7);
  }
  if (k30 != NONE) {
    link(tm, new_port(VAR,v6), k30);
  } else {
    k30 = new_port(VAR,v6);
  }
  if (!k29) {
    node_create(n7, new_pair(k30,k31));
    if (k15 != NONE) {
      link(tm, new_port(DUP,n7), k15);
    } else {
      k15 = new_port(DUP,n7);
    }
  }
  if (!k13) {
    node_create(n6, new_pair(k15,k16));
    if (k12 != NONE) {
      link(tm, new_port(CON,n6), k12);
    } else {
      k12 = new_port(CON,n6);
    }
  }
  bool k32 = 0;
  Pair k33 = emptyPair;
  Port k34 = NONE;
  Port k35 = NONE;
  // fast anni
  if (get_tag(k11) == CON && !isEmpty(node_load(k11))) {
    tm->itrs += 1;
    k32 = 1;
    k33 = node_take(k11);
    k34 = k33.fst;
    k35 = k33.snd;
  }
  if (k35 != NONE) {
    link(tm, new_port(VAR,v5), k35);
  } else {
    k35 = new_port(VAR,v5);
  }
  if (k34 != NONE) {
    link(tm, new_port(VAR,v4), k34);
  } else {
    k34 = new_port(VAR,v4);
  }
  if (!k32) {
    node_create(n5, new_pair(k34,k35));
    if (k11 != NONE) {
      link(tm, new_port(CON,n5), k11);
    } else {
      k11 = new_port(CON,n5);
    }
  }
  if (!k9) {
    node_create(n4, new_pair(k11,k12));
    if (k8 != NONE) {
      link(tm, new_port(CON,n4), k8);
    } else {
      k8 = new_port(CON,n4);
    }
  }
  bool k36 = 0;
  Pair k37 = emptyPair;
  Port k38 = NONE;
  Port k39 = NONE;
  // fast anni
  if (get_tag(k7) == CON && !isEmpty(node_load(k7))) {
    tm->itrs += 1;
    k36 = 1;
    k37 = node_take(k7);
    k38 = k37.fst;
    k39 = k37.snd;
  }
  if (k39 != NONE) {
    link(tm, new_port(VAR,v3), k39);
  } else {
    k39 = new_port(VAR,v3);
  }
  if (k38 != NONE) {
    link(tm, new_port(VAR,v2), k38);
  } else {
    k38 = new_port(VAR,v2);
  }
  if (!k36) {
    node_create(n3, new_pair(k38,k39));
    if (k7 != NONE) {
      link(tm, new_port(CON,n3), k7);
    } else {
      k7 = new_port(CON,n3);
    }
  }
  if (!k5) {
    node_create(n2, new_pair(k7,k8));
    if (k4 != NONE) {
      link(tm, new_port(CON,n2), k4);
    } else {
      k4 = new_port(CON,n2);
    }
  }
  bool k40 = 0;
  Port k41 = NONE;
  Port k42 = NONE;
  // fast copy
  if (get_tag(k3) == NUM) {
    tm->itrs += 1;
    k40 = 1;
    k41 = k3;
    k42 = k3;
  }
  if (k42 != NONE) {
    link(tm, new_port(VAR,v1), k42);
  } else {
    k42 = new_port(VAR,v1);
  }
  if (k41 != NONE) {
    link(tm, new_port(VAR,v0), k41);
  } else {
    k41 = new_port(VAR,v0);
  }
  if (!k40) {
    node_create(n1, new_pair(k41,k42));
    if (k3 != NONE) {
      link(tm, new_port(DUP,n1), k3);
    } else {
      k3 = new_port(DUP,n1);
    }
  }
  if (!k1) {
    node_create(n0, new_pair(k3,k4));
    if (b != NONE) {
      link(tm, new_port(CON,n0), b);
    } else {
      b = new_port(CON,n0);
    }
  }
  node_create(nf, new_pair(new_port(VAR,v9),new_port(VAR,vb)));
  node_create(ne, new_pair(new_port(VAR,v3),new_port(CON,nf)));
  node_create(nd, new_pair(new_port(VAR,v5),new_port(CON,ne)));
  node_create(nc, new_pair(new_port(VAR,v7),new_port(CON,nd)));
  node_create(nb, new_pair(new_port(VAR,v1),new_port(CON,nc)));
  link(tm, new_ref(CALL_warp), new_port(CON,nb));
  node_create(n14, new_pair(new_port(VAR,v8),new_port(VAR,va)));
  node_create(n13, new_pair(new_port(VAR,v2),new_port(CON,n14)));
  node_create(n12, new_pair(new_port(VAR,v4),new_port(CON,n13)));
  node_create(n11, new_pair(new_port(VAR,v6),new_port(CON,n12)));
  node_create(n10, new_pair(new_port(VAR,v0),new_port(CON,n11)));
  link(tm, new_ref(CALL_warp), new_port(CON,n10));
  return TRUE;
}

int main() {
  printf("Port size: %lu\n", sizeof(Port));
  hvm_c(CALL_main);
  return 0;
}
