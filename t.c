
#define _XOPEN_SOURCE 600
#include "runtime3.h"
#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: these are only needed until core.toc codegen is complete
Value *noImpl1(FnArity *arity, Value *str_0) { abort(); }

Value *noImpl2(FnArity *arity, Value *str_0, Value *arg1) { abort(); }

Value *noImpl3(FnArity *arity, Value *str_0, Value *arg1, Value *arg2) {
  abort();
}
Term glblFn1;
void glblCFn0(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term x_1 = arityArgs.args[0];
    if (term_tag(x_1) == I60) {
      result = term_val((Term)stringValue("Integer"));
    } else {
      char *name = typeName(((Value *)x_1)->type);
      result = term_val((Term)stringValue(name));
    };
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn1 = new_ref(glblCFn0);
Term glbltype_name6;
void glblglbl5(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_5 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_5 = port(2, term_loc(seq_5));
  Term x_4 = term_new(VAR, 0, port(1, term_loc(seq_5)));
  // allocate args at core: 38
  // call default-type-name at core: 40
  Term Rslt_7Args = pair_make(APP, 0, x_4, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblFn1);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn1));
#endif

  // link args to body
  swapStore(r_seq_5, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_5);
  return;
}
Term glbltype_name6 = new_ref(glblglbl5);
Term glblProto3;
Term glblFn8;
void glblCFn7(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term x_1 = arityArgs.args[0];

    switch (term_tag(x_1)) {
    case I60:
      result = new_i60(IntegerType);
      break;

    case F60:
      result = new_i60(FloatType);
      break;

    case REF:
      BOOM("too tire");
      break;

    default:
      result = new_i60(((Value *)x_1)->type);
      dec_and_free(x_1, 1);
    }
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn8 = new_ref(glblCFn7);
Term glblProto14;
void glblSome17(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(43), callArgs);
  interact(callArgs, construct);
  return;
}
void glblNone11(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(0), args);
  callArgs = pair_make(APP, 0, new_i60(42), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal13 = new_ref(glblNone11);
Term glblVal13;
Term glblRslt12;
Term glblVal20 = new_ref(glblSome17);
Term glblVal20;
Term glblFld19;
void glblFldFn21(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld19 = new_ref(glblFldFn21);
Term glblProto16;
Term glblProto22;
Term glblProto24;
Term glblProto26;
Term glblProto28;
Term glblProto30;
Term glblProto32;
Term glblProto34;
Term glblProto36;
Term glblProto38;
Term glblProto40;
Term glblProto42;
Term glblProto44;
Term glblProto46;
Term glblProto48;
Term glblProto50;
Term glblProto52;
Term glblProto54;
Term glblProto56;
Term glblProto58;
Term glblProto60;
Term glblProto62;
Term glblProto64;
Term glblProto66;
Term glblProto68;
Term glblProto70;
Term glblProto72;
Term glblProto74;
Term glblProto76;
Term glblProto78;
Term glblProto80;
Term glblProto82;
Term glblProto84;
Term glblProto86;
Term glblProto88;
Term glblProto90;
Term glblProto92;
Term glblProto94;
Term glblProto96;
Term glblProto98;
Term glblProto100;
Term glblProto102;
Term glblProto104;
Term glblProto106;
Term glblProto108;
Term glblFn111;
void glblCFn110(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term str_1 = arityArgs.args[0];

    if (term_tag(str_1) != VAL) {
      fprintf(stdout, "\ninvalid type for 'abort': Integer or Float\n");
      abort();
    }
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      fprintf(stdout, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    } else {
      fprintf(stdout, "\ninvalid type for 'abort': %ld\n", s->type);
      abort();
    }
    abort();
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn111 = new_ref(glblCFn110);
Term glblFn114;
void glblCFn113(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term str_1 = arityArgs.args[0];

    if (term_tag(str_1) != VAL) {
      fprintf(stdout, "\ninvalid type for 'pr*': Integer or Float\n");
      abort();
    }
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      fprintf(stdout, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    } else {
      fprintf(stdout, "\ninvalid type for 'pr*': %ld\n", s->type);
      abort();
    }
    dec_and_free(str_1, 1);
    result = new_i60(0);
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn114 = new_ref(glblCFn113);
Term glblFn117;
void glblCFn116(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    fprintf(stderr, "'+' should be optimized away\n");
    abort();

    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn117 = new_ref(glblCFn116);
Term glblFn119;
void glblCFn118(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 255
  Term Rslt_4 = make_op(OP_ADD, new_i60(1), x_1);
  // link args to body
  swapStore(r_seq_2, Rslt_4);

  Term Rslt_4V = get(term_loc(Rslt_4));
  apps = take(port(1, term_loc(Rslt_4V)));
  lams = take(port(2, term_loc(Rslt_4V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_2);
  return;
}
Term glblFn119 = new_ref(glblCFn118);
Term glblFn123;
void glblCFn122(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    fprintf(stderr, "'-' should be optimized away\n");
    abort();

    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn123 = new_ref(glblCFn122);
Term glblFn126;
void glblCFn125(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 267
  Term Rslt_4 = make_op(OP_SUB, x_1, new_i60(1));
  // link args to body
  swapStore(r_seq_2, Rslt_4);

  Term Rslt_4V = get(term_loc(Rslt_4));
  apps = take(port(1, term_loc(Rslt_4V)));
  lams = take(port(2, term_loc(Rslt_4V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_2);
  return;
}
Term glblFn126 = new_ref(glblCFn125);
Term glblFn129;
void glblCFn128(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    fprintf(stderr, "'*' should be optimized away\n");
    abort();

    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn129 = new_ref(glblCFn128);
Term glblVal130 = (Term)&empty_vect_struct;
Term glblVal130;
Term glblFn133;
void glblCFn132(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term vect_1 = arityArgs.args[0];
    Term v_2 = arityArgs.args[1];

    result = term_val((Term)vectConj((Vector *)vect_1, v_2));
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn133 = new_ref(glblCFn132);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[5];
} str0 = {StringBufferType, REFS_STATIC, 0, 0, 4, "core"};
Term glblStr149 = term_new_(VAL, (Term)&str0);
Term glbl_EQ_148;
void glbl_EQ_147(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_8 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_8 = port(2, term_loc(seq_8));
  Term y_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
  seq_8 = pair_make(LAM, 0, SUB, seq_8);
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
  // allocate args at core: 286
  // call type-num at core: 287
  Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
  store_redex(Rslt_10Args, glblFn8);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn8));
#endif

  // call type-num at core: 287
  Term Rslt_11Args = pair_make(APP, 0, y_7, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn8);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn8));
#endif

  // call = at core: 287
  Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_10, Rslt_12Args);
  Rslt_12Args = pair_make(APP, 0, new_i60(287), Rslt_12Args);
  Rslt_12Args = pair_make(APP, 0, glblStr149, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblProto16);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblProto16));
#endif
  // link args to body
  swapStore(r_seq_8, Rslt_12);

  Term Rslt_12V = get(term_loc(Rslt_12));
  apps = take(port(1, term_loc(Rslt_12V)));
  lams = take(port(2, term_loc(Rslt_12V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_8);
  return;
}
Term glbl_EQ_148 = new_ref(glbl_EQ_147);
Term glblrecurse146;
void glblrecurse145(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_16 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_16 = port(2, term_loc(seq_16));
  Term f_15 = term_new(VAR, 0, port(1, term_loc(seq_16)));
  seq_16 = pair_make(LAM, 0, SUB, seq_16);
  Term v_14 = term_new(VAR, 0, port(1, term_loc(seq_16)));
  // allocate args at core: 288
  swapStore(term_loc(f_15), ERA);
  // link args to body
  swapStore(r_seq_16, v_14);

  V = get(r_seq_16);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_16, term_new(VAR, 0, vLoc));
  store_redex(args, seq_16);
  return;
}
Term glblrecurse146 = new_ref(glblrecurse145);
Term glbleither144;
void glbleither143(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_21 = port(2, term_loc(seq_21));
  Term cont_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term v_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  // allocate args at core: 290
  swapStore(term_loc(v_19), ERA);
  // link args to body
  swapStore(r_seq_21, cont_20);

  V = get(r_seq_21);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_21, term_new(VAR, 0, vLoc));
  store_redex(args, seq_21);
  return;
}
Term glbleither144 = new_ref(glbleither143);
Term glblcond142;
void glblcond141(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_27 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_27 = port(2, term_loc(seq_27));
  Term cont_26 = term_new(VAR, 0, port(1, term_loc(seq_27)));
  seq_27 = pair_make(LAM, 0, SUB, seq_27);
  Term x_25 = term_new(VAR, 0, port(1, term_loc(seq_27)));
  seq_27 = pair_make(LAM, 0, SUB, seq_27);
  Term v_24 = term_new(VAR, 0, port(1, term_loc(seq_27)));
  // allocate args at core: 292
  swapStore(term_loc(v_24), ERA);
  swapStore(term_loc(x_25), ERA);
  // link args to body
  swapStore(r_seq_27, cont_26);

  V = get(r_seq_27);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_27, term_new(VAR, 0, vLoc));
  store_redex(args, seq_27);
  return;
}
Term glblcond142 = new_ref(glblcond141);
Term glbland140;
void glbland139(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_32 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_32 = port(2, term_loc(seq_32));
  Term arg_31 = term_new(VAR, 0, port(1, term_loc(seq_32)));
  seq_32 = pair_make(LAM, 0, SUB, seq_32);
  Term x_30 = term_new(VAR, 0, port(1, term_loc(seq_32)));
  // allocate args at core: 294
  swapStore(term_loc(arg_31), ERA);
  // link args to body
  swapStore(r_seq_32, x_30);

  V = get(r_seq_32);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_32, term_new(VAR, 0, vLoc));
  store_redex(args, seq_32);
  return;
}
Term glbland140 = new_ref(glbland139);
Term glblor138;
void glblor137(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_37 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_37 = port(2, term_loc(seq_37));
  Term y_36 = term_new(VAR, 0, port(1, term_loc(seq_37)));
  seq_37 = pair_make(LAM, 0, SUB, seq_37);
  Term arg_35 = term_new(VAR, 0, port(1, term_loc(seq_37)));
  // allocate args at core: 296
  swapStore(term_loc(arg_35), ERA);
  // link args to body
  swapStore(r_seq_37, y_36);

  V = get(r_seq_37);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_37, term_new(VAR, 0, vLoc));
  store_redex(args, seq_37);
  return;
}
Term glblor138 = new_ref(glblor137);
Term glblmap136;
void glblmap135(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_42 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_42 = port(2, term_loc(seq_42));
  Term f_41 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  seq_42 = pair_make(LAM, 0, SUB, seq_42);
  Term x_40 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  // allocate args at core: 298
  swapStore(term_loc(f_41), ERA);
  // link args to body
  swapStore(r_seq_42, x_40);

  V = get(r_seq_42);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_42, term_new(VAR, 0, vLoc));
  store_redex(args, seq_42);
  return;
}
Term glblmap136 = new_ref(glblmap135);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str1 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\n"};
Term glblStr159 = term_new_(VAL, (Term)&str1);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str2 = {StringBufferType, REFS_STATIC, 0, 0, 1, " "};
Term glblStr160 = term_new_(VAL, (Term)&str2);
Term glblFn158;
void glblCFn157(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term vs_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 301
  Term seq_8 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_8 = port(2, term_loc(seq_8));
  Term val_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
  seq_8 = pair_make(LAM, 0, SUB, seq_8);
  Term v_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
  // allocate args at core: 304
  Term seq_15 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_15 = port(2, term_loc(seq_15));
  Term s_14 = term_new(VAR, 0, port(1, term_loc(seq_15)));
  seq_15 = pair_make(LAM, 0, SUB, seq_15);
  Term v_13 = term_new(VAR, 0, port(1, term_loc(seq_15)));
  // allocate args at core: 306
  Term glblVal130_2;
  glblVal130 = dupeArg(glblVal130, &glblVal130_2, 0);

  Term glblVal130_1;
  glblVal130 = dupeArg(glblVal130, &glblVal130_1, 0);

  // call conj at core: 303
  Term Rslt_4Args = pair_make(APP, 0, glblStr159, SUB);
  Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
  Rslt_4Args = pair_make(APP, 0, vs_1, Rslt_4Args);
  Rslt_4Args = pair_make(APP, 0, new_i60(303), Rslt_4Args);
  Rslt_4Args = pair_make(APP, 0, glblStr149, Rslt_4Args);
#ifdef STRICT
  store_redex(Rslt_4Args, glblProto64);
#else
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblProto64));
#endif
  // call str-vect at core: 305
  Term Rslt_10Args = pair_make(APP, 0, val_7, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, new_i60(305), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr149, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto106);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto106));
#endif
  // call conj at core: 305
  Term Rslt_11Args = pair_make(APP, 0, glblStr160, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_10, Rslt_11Args);
  Rslt_11Args = pair_make(APP, 0, new_i60(305), Rslt_11Args);
  Rslt_11Args = pair_make(APP, 0, glblStr149, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblProto64);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblProto64));
#endif
  // call pr* at core: 307
  Term Rslt_17Args = pair_make(APP, 0, s_14, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn114);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn114));
#endif

  // call conj at core: 307
  Term Rslt_18Args = pair_make(APP, 0, Rslt_17, SUB);
  Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
  Rslt_18Args = pair_make(APP, 0, v_13, Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, new_i60(307), Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, glblStr149, Rslt_18Args);
#ifdef STRICT
  store_redex(Rslt_18Args, glblProto64);
#else
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblProto64));
#endif
  // link args to body
  swapStore(r_seq_15, Rslt_18);

  Term Rslt_18V = get(term_loc(Rslt_18));
  apps = take(port(1, term_loc(Rslt_18V)));
  lams = take(port(2, term_loc(Rslt_18V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, apps, lams));
  // call reduce at core: 305
  Term Rslt_19Args = pair_make(APP, 0, seq_15, SUB);
  Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
  Rslt_19Args = pair_make(APP, 0, glblVal130_2, Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, Rslt_11, Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, new_i60(305), Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, glblStr149, Rslt_19Args);
#ifdef STRICT
  store_redex(Rslt_19Args, glblProto68);
#else
  swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblProto68));
#endif
  // call conj at core: 305
  Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
  Rslt_20Args = pair_make(APP, 0, v_6, Rslt_20Args);
  Rslt_20Args = pair_make(APP, 0, new_i60(305), Rslt_20Args);
  Rslt_20Args = pair_make(APP, 0, glblStr149, Rslt_20Args);
#ifdef STRICT
  store_redex(Rslt_20Args, glblProto64);
#else
  swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, glblProto64));
#endif
  // link args to body
  swapStore(r_seq_8, Rslt_20);

  Term Rslt_20V = get(term_loc(Rslt_20));
  apps = take(port(1, term_loc(Rslt_20V)));
  lams = take(port(2, term_loc(Rslt_20V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, apps, lams));
  // call reduce at core: 303
  Term Rslt_21Args = pair_make(APP, 0, seq_8, SUB);
  Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
  Rslt_21Args = pair_make(APP, 0, glblVal130_1, Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, Rslt_4, Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, new_i60(303), Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, glblStr149, Rslt_21Args);
#ifdef STRICT
  store_redex(Rslt_21Args, glblProto68);
#else
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblProto68));
#endif
  // link args to body
  swapStore(r_seq_2, Rslt_21);

  Term Rslt_21V = get(term_loc(Rslt_21));
  apps = take(port(1, term_loc(Rslt_21V)));
  lams = take(port(2, term_loc(Rslt_21V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_2);
  return;
}
Term glblFn158 = new_ref(glblCFn157);
Term glbl_EQ_186;
void glbl_EQ_185(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_25 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_25 = port(2, term_loc(seq_25));
  Term y_24 = term_new(VAR, 0, port(1, term_loc(seq_25)));
  seq_25 = pair_make(LAM, 0, SUB, seq_25);
  Term x_23 = term_new(VAR, 0, port(1, term_loc(seq_25)));
  // allocate args at core: 310

  Term x_23_1;
  x_23 = dupeArg(x_23, &x_23_1, 0);

  Term y_24_1;
  y_24 = dupeArg(y_24, &y_24_1, 0);
  // call type-num at core: 311
  Term Rslt_27Args = pair_make(APP, 0, x_23, SUB);
  Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
#ifdef STRICT
  store_redex(Rslt_27Args, glblFn8);
#else
  swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, glblFn8));
#endif

  // call type-num at core: 311
  Term Rslt_28Args = pair_make(APP, 0, y_24, SUB);
  Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
#ifdef STRICT
  store_redex(Rslt_28Args, glblFn8);
#else
  swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, glblFn8));
#endif

  // call = at core: 311
  Term Rslt_29Args = pair_make(APP, 0, Rslt_28, SUB);
  Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
  Rslt_29Args = pair_make(APP, 0, Rslt_27, Rslt_29Args);
  Rslt_29Args = pair_make(APP, 0, new_i60(311), Rslt_29Args);
  Rslt_29Args = pair_make(APP, 0, glblStr149, Rslt_29Args);
#ifdef STRICT
  store_redex(Rslt_29Args, glblProto16);
#else
  swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, glblProto16));
#endif
  // call .x at core: 312
  Term Rslt_30Args = pair_make(APP, 0, x_23_1, SUB);
  Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
  Rslt_30Args = pair_make(APP, 0, new_i60(312), Rslt_30Args);
  Rslt_30Args = pair_make(APP, 0, glblStr149, Rslt_30Args);
#ifdef STRICT
  store_redex(Rslt_30Args, glblProto14);
#else
  swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblProto14));
#endif
  // call .x at core: 312
  Term Rslt_31Args = pair_make(APP, 0, y_24_1, SUB);
  Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
  Rslt_31Args = pair_make(APP, 0, new_i60(312), Rslt_31Args);
  Rslt_31Args = pair_make(APP, 0, glblStr149, Rslt_31Args);
#ifdef STRICT
  store_redex(Rslt_31Args, glblProto14);
#else
  swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, glblProto14));
#endif
  // call = at core: 312
  Term Rslt_32Args = pair_make(APP, 0, Rslt_31, SUB);
  Term Rslt_32 = term_new(VAR, 0, port(2, term_loc(Rslt_32Args)));
  Rslt_32Args = pair_make(APP, 0, Rslt_30, Rslt_32Args);
  Rslt_32Args = pair_make(APP, 0, new_i60(312), Rslt_32Args);
  Rslt_32Args = pair_make(APP, 0, glblStr149, Rslt_32Args);
#ifdef STRICT
  store_redex(Rslt_32Args, glblProto16);
#else
  swapStore(term_loc(Rslt_32), pair_make(LAZ, 0, Rslt_32Args, glblProto16));
#endif
  // call and at core: 311
  Term Rslt_33Args = pair_make(APP, 0, Rslt_32, SUB);
  Term Rslt_33 = term_new(VAR, 0, port(2, term_loc(Rslt_33Args)));
  Rslt_33Args = pair_make(APP, 0, Rslt_29, Rslt_33Args);
  Rslt_33Args = pair_make(APP, 0, new_i60(311), Rslt_33Args);
  Rslt_33Args = pair_make(APP, 0, glblStr149, Rslt_33Args);
#ifdef STRICT
  store_redex(Rslt_33Args, glblProto24);
#else
  swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, Rslt_33Args, glblProto24));
#endif
  // link args to body
  swapStore(r_seq_25, Rslt_33);

  Term Rslt_33V = get(term_loc(Rslt_33));
  apps = take(port(1, term_loc(Rslt_33V)));
  lams = take(port(2, term_loc(Rslt_33V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_25);
  return;
}
Term glbl_EQ_186 = new_ref(glbl_EQ_185);
Term glblextract184;
void glblextract183(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_36 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_36 = port(2, term_loc(seq_36));
  Term v_35 = term_new(VAR, 0, port(1, term_loc(seq_36)));
  // allocate args at core: 313
  // call .x at core: 314
  Term Rslt_38Args = pair_make(APP, 0, v_35, SUB);
  Term Rslt_38 = term_new(VAR, 0, port(2, term_loc(Rslt_38Args)));
  Rslt_38Args = pair_make(APP, 0, new_i60(314), Rslt_38Args);
  Rslt_38Args = pair_make(APP, 0, glblStr149, Rslt_38Args);
#ifdef STRICT
  store_redex(Rslt_38Args, glblProto14);
#else
  swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, Rslt_38Args, glblProto14));
#endif
  // link args to body
  swapStore(r_seq_36, Rslt_38);

  Term Rslt_38V = get(term_loc(Rslt_38));
  apps = take(port(1, term_loc(Rslt_38V)));
  lams = take(port(2, term_loc(Rslt_38V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_36);
  return;
}
Term glblextract184 = new_ref(glblextract183);
Term glblrecurse182;
void glblrecurse181(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_42 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_42 = port(2, term_loc(seq_42));
  Term f_41 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  seq_42 = pair_make(LAM, 0, SUB, seq_42);
  Term v_40 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  // allocate args at core: 315
  Term glblVal20_1;
  glblVal20 = dupeArg(glblVal20, &glblVal20_1, 0);

  // call .x at core: 316
  Term Rslt_44Args = pair_make(APP, 0, v_40, SUB);
  Term Rslt_44 = term_new(VAR, 0, port(2, term_loc(Rslt_44Args)));
  Rslt_44Args = pair_make(APP, 0, new_i60(316), Rslt_44Args);
  Rslt_44Args = pair_make(APP, 0, glblStr149, Rslt_44Args);
#ifdef STRICT
  store_redex(Rslt_44Args, glblProto14);
#else
  swapStore(term_loc(Rslt_44), pair_make(LAZ, 0, Rslt_44Args, glblProto14));
#endif
  // call f at core: 315
  Term Rslt_45Args = pair_make(APP, 0, Rslt_44, SUB);
  Term Rslt_45 = term_new(VAR, 0, port(2, term_loc(Rslt_45Args)));
#ifdef STRICT
  store_redex(Rslt_45Args, f_41);
#else
  swapStore(term_loc(Rslt_45), pair_make(LAZ, 0, Rslt_45Args, f_41));
#endif

  // call Some at core: 316
  Term Rslt_46Args = pair_make(APP, 0, Rslt_45, SUB);
  Term Rslt_46 = term_new(VAR, 0, port(2, term_loc(Rslt_46Args)));
#ifdef STRICT
  store_redex(Rslt_46Args, glblVal20_1);
#else
  swapStore(term_loc(Rslt_46), pair_make(LAZ, 0, Rslt_46Args, glblVal20_1));
#endif

  // link args to body
  swapStore(r_seq_42, Rslt_46);

  Term Rslt_46V = get(term_loc(Rslt_46));
  apps = take(port(1, term_loc(Rslt_46V)));
  lams = take(port(2, term_loc(Rslt_46V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_46), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_42);
  return;
}
Term glblrecurse182 = new_ref(glblrecurse181);
Term glbleither180;
void glbleither179(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_50 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_50 = port(2, term_loc(seq_50));
  Term arg_49 = term_new(VAR, 0, port(1, term_loc(seq_50)));
  seq_50 = pair_make(LAM, 0, SUB, seq_50);
  Term v_48 = term_new(VAR, 0, port(1, term_loc(seq_50)));
  // allocate args at core: 317
  swapStore(term_loc(arg_49), ERA);
  // call .x at core: 318
  Term Rslt_52Args = pair_make(APP, 0, v_48, SUB);
  Term Rslt_52 = term_new(VAR, 0, port(2, term_loc(Rslt_52Args)));
  Rslt_52Args = pair_make(APP, 0, new_i60(318), Rslt_52Args);
  Rslt_52Args = pair_make(APP, 0, glblStr149, Rslt_52Args);
#ifdef STRICT
  store_redex(Rslt_52Args, glblProto14);
#else
  swapStore(term_loc(Rslt_52), pair_make(LAZ, 0, Rslt_52Args, glblProto14));
#endif
  // link args to body
  swapStore(r_seq_50, Rslt_52);

  Term Rslt_52V = get(term_loc(Rslt_52));
  apps = take(port(1, term_loc(Rslt_52V)));
  lams = take(port(2, term_loc(Rslt_52V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_52), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_50);
  return;
}
Term glbleither180 = new_ref(glbleither179);
Term glblcond178;
void glblcond177(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_57 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_57 = port(2, term_loc(seq_57));
  Term arg_56 = term_new(VAR, 0, port(1, term_loc(seq_57)));
  seq_57 = pair_make(LAM, 0, SUB, seq_57);
  Term clause_55 = term_new(VAR, 0, port(1, term_loc(seq_57)));
  seq_57 = pair_make(LAM, 0, SUB, seq_57);
  Term v_54 = term_new(VAR, 0, port(1, term_loc(seq_57)));
  // allocate args at core: 319
  swapStore(term_loc(v_54), ERA);
  swapStore(term_loc(arg_56), ERA);
  // link args to body
  swapStore(r_seq_57, clause_55);

  V = get(r_seq_57);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_57, term_new(VAR, 0, vLoc));
  store_redex(args, seq_57);
  return;
}
Term glblcond178 = new_ref(glblcond177);
Term glbland176;
void glbland175(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_62 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_62 = port(2, term_loc(seq_62));
  Term y_61 = term_new(VAR, 0, port(1, term_loc(seq_62)));
  seq_62 = pair_make(LAM, 0, SUB, seq_62);
  Term arg_60 = term_new(VAR, 0, port(1, term_loc(seq_62)));
  // allocate args at core: 321
  swapStore(term_loc(arg_60), ERA);
  // link args to body
  swapStore(r_seq_62, y_61);

  V = get(r_seq_62);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_62, term_new(VAR, 0, vLoc));
  store_redex(args, seq_62);
  return;
}
Term glbland176 = new_ref(glbland175);
Term glblor174;
void glblor173(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_67 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_67 = port(2, term_loc(seq_67));
  Term arg_66 = term_new(VAR, 0, port(1, term_loc(seq_67)));
  seq_67 = pair_make(LAM, 0, SUB, seq_67);
  Term q_65 = term_new(VAR, 0, port(1, term_loc(seq_67)));
  // allocate args at core: 323
  swapStore(term_loc(arg_66), ERA);
  // link args to body
  swapStore(r_seq_67, q_65);

  V = get(r_seq_67);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_67, term_new(VAR, 0, vLoc));
  store_redex(args, seq_67);
  return;
}
Term glblor174 = new_ref(glblor173);
Term glblmap172;
void glblmap171(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_72 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_72 = port(2, term_loc(seq_72));
  Term f_71 = term_new(VAR, 0, port(1, term_loc(seq_72)));
  seq_72 = pair_make(LAM, 0, SUB, seq_72);
  Term v_70 = term_new(VAR, 0, port(1, term_loc(seq_72)));
  // allocate args at core: 325
  Term glblVal20_2;
  glblVal20 = dupeArg(glblVal20, &glblVal20_2, 0);

  // call .x at core: 326
  Term Rslt_74Args = pair_make(APP, 0, v_70, SUB);
  Term Rslt_74 = term_new(VAR, 0, port(2, term_loc(Rslt_74Args)));
  Rslt_74Args = pair_make(APP, 0, new_i60(326), Rslt_74Args);
  Rslt_74Args = pair_make(APP, 0, glblStr149, Rslt_74Args);
#ifdef STRICT
  store_redex(Rslt_74Args, glblProto14);
#else
  swapStore(term_loc(Rslt_74), pair_make(LAZ, 0, Rslt_74Args, glblProto14));
#endif
  // call f at core: 325
  Term Rslt_75Args = pair_make(APP, 0, Rslt_74, SUB);
  Term Rslt_75 = term_new(VAR, 0, port(2, term_loc(Rslt_75Args)));
#ifdef STRICT
  store_redex(Rslt_75Args, f_71);
#else
  swapStore(term_loc(Rslt_75), pair_make(LAZ, 0, Rslt_75Args, f_71));
#endif

  // call Some at core: 326
  Term Rslt_76Args = pair_make(APP, 0, Rslt_75, SUB);
  Term Rslt_76 = term_new(VAR, 0, port(2, term_loc(Rslt_76Args)));
#ifdef STRICT
  store_redex(Rslt_76Args, glblVal20_2);
#else
  swapStore(term_loc(Rslt_76), pair_make(LAZ, 0, Rslt_76Args, glblVal20_2));
#endif

  // link args to body
  swapStore(r_seq_72, Rslt_76);

  Term Rslt_76V = get(term_loc(Rslt_76));
  apps = take(port(1, term_loc(Rslt_76V)));
  lams = take(port(2, term_loc(Rslt_76V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_76), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_72);
  return;
}
Term glblmap172 = new_ref(glblmap171);
Term glblflat_map170;
void glblflat_map169(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_80 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_80 = port(2, term_loc(seq_80));
  Term f_79 = term_new(VAR, 0, port(1, term_loc(seq_80)));
  seq_80 = pair_make(LAM, 0, SUB, seq_80);
  Term v_78 = term_new(VAR, 0, port(1, term_loc(seq_80)));
  // allocate args at core: 327
  // call .x at core: 328
  Term Rslt_82Args = pair_make(APP, 0, v_78, SUB);
  Term Rslt_82 = term_new(VAR, 0, port(2, term_loc(Rslt_82Args)));
  Rslt_82Args = pair_make(APP, 0, new_i60(328), Rslt_82Args);
  Rslt_82Args = pair_make(APP, 0, glblStr149, Rslt_82Args);
#ifdef STRICT
  store_redex(Rslt_82Args, glblProto14);
#else
  swapStore(term_loc(Rslt_82), pair_make(LAZ, 0, Rslt_82Args, glblProto14));
#endif
  // call f at core: 327
  Term Rslt_83Args = pair_make(APP, 0, Rslt_82, SUB);
  Term Rslt_83 = term_new(VAR, 0, port(2, term_loc(Rslt_83Args)));
#ifdef STRICT
  store_redex(Rslt_83Args, f_79);
#else
  swapStore(term_loc(Rslt_83), pair_make(LAZ, 0, Rslt_83Args, f_79));
#endif

  // link args to body
  swapStore(r_seq_80, Rslt_83);

  Term Rslt_83V = get(term_loc(Rslt_83));
  apps = take(port(1, term_loc(Rslt_83V)));
  lams = take(port(2, term_loc(Rslt_83V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_83), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_80);
  return;
}
Term glblflat_map170 = new_ref(glblflat_map169);
Term glblFn188;
void glblCFn187(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    result = term_val(integer_EQ(x_1, y_2));
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn188 = new_ref(glblCFn187);
Term glblFn190;
void glblCFn189(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    result = term_val(integer_LT(x_1, y_2));
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn190 = new_ref(glblCFn189);
Term glblFn192;
void glblCFn191(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term n_1 = arityArgs.args[0];

    if (term_tag(n_1) == VAL) {
      fprintf(stdout, "\ninvalid type for 'number-str': Value\n");
      abort();
    }
    result = number_str(n_1);

    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn192 = new_ref(glblCFn191);
Term glblFn194;
void glblCFn193(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 352

  Term x_1_1;
  x_1 = dupeArg(x_1, &x_1_1, 0);

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 0);
  // call int-< at core: 353
  Term Rslt_5Args = pair_make(APP, 0, y_2, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
  Rslt_5Args = pair_make(APP, 0, x_1, Rslt_5Args);
#ifdef STRICT
  store_redex(Rslt_5Args, glblFn190);
#else
  swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn190));
#endif

  // call int-= at core: 354
  Term Rslt_6Args = pair_make(APP, 0, y_2_1, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
  Rslt_6Args = pair_make(APP, 0, x_1_1, Rslt_6Args);
#ifdef STRICT
  store_redex(Rslt_6Args, glblFn188);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn188));
#endif

  // call or at core: 353
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
  Rslt_7Args = pair_make(APP, 0, Rslt_5, Rslt_7Args);
  Rslt_7Args = pair_make(APP, 0, new_i60(353), Rslt_7Args);
  Rslt_7Args = pair_make(APP, 0, glblStr149, Rslt_7Args);
#ifdef STRICT
  store_redex(Rslt_7Args, glblProto26);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblProto26));
#endif
  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblFn194 = new_ref(glblCFn193);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str3 = {StringBufferType, REFS_STATIC, 0, 0, 7, "Integer"};
Term glblStr227 = term_new_(VAL, (Term)&str3);
Term glbltype_name226;
void glbltype_name225(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term arg_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at core: 357
  swapStore(term_loc(arg_9), ERA);
  // link args to body
  swapStore(r_seq_10, glblStr227);

  V = get(r_seq_10);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_10, term_new(VAR, 0, vLoc));
  store_redex(args, seq_10);
  return;
}
Term glbltype_name226 = new_ref(glbltype_name225);
Term glblstr_vect224;
void glblstr_vect223(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_14 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_14 = port(2, term_loc(seq_14));
  Term n_13 = term_new(VAR, 0, port(1, term_loc(seq_14)));
  // allocate args at core: 360
  Term glblVal130_3;
  glblVal130 = dupeArg(glblVal130, &glblVal130_3, 0);

  // call number-str at core: 361
  Term Rslt_16Args = pair_make(APP, 0, n_13, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn192);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn192));
#endif

  // call vect-conj at core: 361
  Term Rslt_17Args = pair_make(APP, 0, Rslt_16, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, glblVal130_3, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_14, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_14);
  return;
}
Term glblstr_vect224 = new_ref(glblstr_vect223);
Term glbl_EQ_222;
void glbl_EQ_221(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_21 = port(2, term_loc(seq_21));
  Term y_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term x_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  // allocate args at core: 363
  // call int-= at core: 364
  Term Rslt_23Args = pair_make(APP, 0, y_20, SUB);
  Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
  Rslt_23Args = pair_make(APP, 0, x_19, Rslt_23Args);
#ifdef STRICT
  store_redex(Rslt_23Args, glblFn188);
#else
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblFn188));
#endif

  // link args to body
  swapStore(r_seq_21, Rslt_23);

  Term Rslt_23V = get(term_loc(Rslt_23));
  apps = take(port(1, term_loc(Rslt_23V)));
  lams = take(port(2, term_loc(Rslt_23V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_21);
  return;
}
Term glbl_EQ_222 = new_ref(glbl_EQ_221);
Term glbl_LT_220;
void glbl_LT_218(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_27 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_27 = port(2, term_loc(seq_27));
  Term y_26 = term_new(VAR, 0, port(1, term_loc(seq_27)));
  seq_27 = pair_make(LAM, 0, SUB, seq_27);
  Term x_25 = term_new(VAR, 0, port(1, term_loc(seq_27)));
  // allocate args at core: 366
  // call int-< at core: 367
  Term Rslt_29Args = pair_make(APP, 0, y_26, SUB);
  Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
  Rslt_29Args = pair_make(APP, 0, x_25, Rslt_29Args);
#ifdef STRICT
  store_redex(Rslt_29Args, glblFn190);
#else
  swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, glblFn190));
#endif

  // link args to body
  swapStore(r_seq_27, Rslt_29);

  Term Rslt_29V = get(term_loc(Rslt_29));
  apps = take(port(1, term_loc(Rslt_29V)));
  lams = take(port(2, term_loc(Rslt_29V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_27);
  return;
}
Term glbl_LT_220 = new_ref(glbl_LT_218);
Term glblFn229;
void glblCFn228(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 373

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 0);
  // call < at core: 377
  Term Rslt_5Args = pair_make(APP, 0, y_2, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
  Rslt_5Args = pair_make(APP, 0, x_1, Rslt_5Args);
  Rslt_5Args = pair_make(APP, 0, new_i60(377), Rslt_5Args);
  Rslt_5Args = pair_make(APP, 0, glblStr149, Rslt_5Args);
#ifdef STRICT
  store_redex(Rslt_5Args, glblProto22);
#else
  swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblProto22));
#endif
  // call either at core: 377
  Term Rslt_6Args = pair_make(APP, 0, y_2_1, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
  Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, new_i60(377), Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, glblStr149, Rslt_6Args);
#ifdef STRICT
  store_redex(Rslt_6Args, glblProto28);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblProto28));
#endif
  // link args to body
  swapStore(r_seq_3, Rslt_6);

  Term Rslt_6V = get(term_loc(Rslt_6));
  apps = take(port(1, term_loc(Rslt_6V)));
  lams = take(port(2, term_loc(Rslt_6V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblFn229 = new_ref(glblCFn228);
Term glblFn235;
void glblCFn234(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term v_1 = arityArgs.args[0];

    Vector *v = (Vector *)((u64)v_1 & ~7);
    result = new_i60(v->count);
    dec_and_free((Term)v, 1);
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn235 = new_ref(glblCFn234);
Term glblFn241;
void glblCFn240(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 387
  // link args to body
  swapStore(r_seq_2, x_1);

  V = get(r_seq_2);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_2, term_new(VAR, 0, vLoc));
  store_redex(args, seq_2);
  return;
}
Term glblFn241 = new_ref(glblCFn240);
Term glblFn244;
void glblCFn243(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 391

  Term f_2_1;
  f_2 = dupeArg(f_2, &f_2_1, 0);
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term v_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at core: 392
  // call fold at core: 391
  Term Rslt_9Args = pair_make(APP, 0, f_2_1, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, v_6, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn244);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn244));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_9);

  Term Rslt_9V = get(term_loc(Rslt_9));
  apps = take(port(1, term_loc(Rslt_9V)));
  lams = take(port(2, term_loc(Rslt_9V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, apps, lams));
  // call recurse at core: 392
  Term Rslt_10Args = pair_make(APP, 0, seq_7, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, x_1, Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, new_i60(392), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr149, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto36);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto36));
#endif
  // call f at core: 391
  Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
  store_redex(Rslt_11Args, f_2);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, f_2));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_11);

  Term Rslt_11V = get(term_loc(Rslt_11));
  apps = take(port(1, term_loc(Rslt_11V)));
  lams = take(port(2, term_loc(Rslt_11V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblFn244 = new_ref(glblCFn243);
void glblLeaf251(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(0), args);
  callArgs = pair_make(APP, 0, new_i60(44), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblFn248;
void glblCFn247(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 401

  Term f_2_1;
  f_2 = dupeArg(f_2, &f_2_1, 0);
  Term seq_8 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_8 = port(2, term_loc(seq_8));
  Term v_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
  // allocate args at core: 402
  // call f at core: 401
  Term Rslt_5Args = pair_make(APP, 0, x_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
  store_redex(Rslt_5Args, f_2);
#else
  swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, f_2));
#endif

  // call unfold at core: 401
  Term Rslt_10Args = pair_make(APP, 0, f_2_1, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, v_7, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblFn248);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn248));
#endif

  // link args to body
  swapStore(r_seq_8, Rslt_10);

  Term Rslt_10V = get(term_loc(Rslt_10));
  apps = take(port(1, term_loc(Rslt_10V)));
  lams = take(port(2, term_loc(Rslt_10V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, apps, lams));
  // call recurse at core: 402
  Term Rslt_11Args = pair_make(APP, 0, seq_8, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_5, Rslt_11Args);
  Rslt_11Args = pair_make(APP, 0, new_i60(402), Rslt_11Args);
  Rslt_11Args = pair_make(APP, 0, glblStr149, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblProto36);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblProto36));
#endif
  // link args to body
  swapStore(r_seq_3, Rslt_11);

  Term Rslt_11V = get(term_loc(Rslt_11));
  apps = take(port(1, term_loc(Rslt_11V)));
  lams = take(port(2, term_loc(Rslt_11V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblFn248 = new_ref(glblCFn247);
Term glblVal253 = new_ref(glblLeaf251);
Term glblVal253;
Term glblRslt252;
Term glblFn250;
void glblCFn249(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term s_1 = arityArgs.args[0];

    result = strCount(s_1);
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn250 = new_ref(glblCFn249);
Term glblFn258;
void glblCFn257(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term len_1 = arityArgs.args[0];

    String *strVal = malloc_string(get_i60(len_1));
    strVal->len = 0;
    strVal->buffer[0] = 0;
    dec_and_free(len_1, 1);
    result = term_val((Term)strVal);

    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn258 = new_ref(glblCFn257);
Term glblFn261;
void glblCFn260(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term dest_1 = arityArgs.args[0];
    Term src_2 = arityArgs.args[1];

    String *s_1 = (String *)dest_1;
    String *s_2 = (String *)src_2;
    strncat(s_1->buffer, s_2->buffer, s_2->len);
    s_1->len += s_2->len;
    dec_and_free(src_2, 1);
    result = term_val(dest_1);
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn261 = new_ref(glblCFn260);
Term glblProto262;
Term glblProto269;
Term glblProto267;
Term glblProto265;
void glblSubString275(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(45), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal279 = new_ref(glblSubString275);
Term glblVal279;
Term glblcount272;
void glblcount271(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term arg_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 443
  Term arg_1Args = pair_make(APP, 0, arg_1, SUB);
  Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(arg_1Args)));
  arg_1Args = pair_make(APP, 0, new_i60(2), arg_1Args);
  store_redex(arg_1Args, accessField);
  // link args to body
  swapStore(r_seq_2, Rslt_4);

  V = get(r_seq_2);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_2, term_new(VAR, 0, vLoc));
  store_redex(args, seq_2);
  return;
}
Term glblcount272 = new_ref(glblcount271);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[16];
} str4 = {StringBufferType, REFS_STATIC, 0, 0, 15, "SubString subs\n"};
Term glblStr280 = term_new_(VAL, (Term)&str4);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[1];
} str5 = {StringBufferType, REFS_STATIC, 0, 0, 0, ""};
Term glblStr281 = term_new_(VAL, (Term)&str5);
Term glblsubs274;
void glblsubs273(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_9 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_9 = port(2, term_loc(seq_9));
  Term new_len_8 = term_new(VAR, 0, port(1, term_loc(seq_9)));
  seq_9 = pair_make(LAM, 0, SUB, seq_9);
  Term new_start_7 = term_new(VAR, 0, port(1, term_loc(seq_9)));
  seq_9 = pair_make(LAM, 0, SUB, seq_9);
  Term s_6 = term_new(VAR, 0, port(1, term_loc(seq_9)));
  // allocate args at core: 446

  Term s_6_4;
  s_6 = dupeArg(s_6, &s_6_4, 0);

  Term s_6_3;
  s_6 = dupeArg(s_6, &s_6_3, 0);

  Term s_6_2;
  s_6 = dupeArg(s_6, &s_6_2, 0);

  Term s_6_1;
  s_6 = dupeArg(s_6, &s_6_1, 0);
  Term s_6_4Args = pair_make(APP, 0, s_6_4, SUB);
  Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(s_6_4Args)));
  s_6_4Args = pair_make(APP, 0, new_i60(0), s_6_4Args);
  store_redex(s_6_4Args, accessField);
  Term s_6_3Args = pair_make(APP, 0, s_6_3, SUB);
  Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(s_6_3Args)));
  s_6_3Args = pair_make(APP, 0, new_i60(2), s_6_3Args);
  store_redex(s_6_3Args, accessField);
  Term s_6_2Args = pair_make(APP, 0, s_6_2, SUB);
  Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(s_6_2Args)));
  s_6_2Args = pair_make(APP, 0, new_i60(1), s_6_2Args);
  store_redex(s_6_2Args, accessField);
  Term seq_14 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_14 = port(2, term_loc(seq_14));
  Term final_len_13 = term_new(VAR, 0, port(1, term_loc(seq_14)));
  // allocate args at core: 448

  Term final_len_13_2;
  final_len_13 = dupeArg(final_len_13, &final_len_13_2, 0);

  Term final_len_13_1;
  final_len_13 = dupeArg(final_len_13, &final_len_13_1, 0);
  Term s_6_1Args = pair_make(APP, 0, s_6_1, SUB);
  Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(s_6_1Args)));
  s_6_1Args = pair_make(APP, 0, new_i60(1), s_6_1Args);
  store_redex(s_6_1Args, accessField);
  Term s_6Args = pair_make(APP, 0, s_6, SUB);
  Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(s_6Args)));
  s_6Args = pair_make(APP, 0, new_i60(0), s_6Args);
  store_redex(s_6Args, accessField);
  Term glblVal279_1;
  glblVal279 = dupeArg(glblVal279, &glblVal279_1, 0);

  // call pr* at core: 447
  Term Rslt_11Args = pair_make(APP, 0, glblStr280, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn114);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn114));
#endif

  // call < at core: 449
  Term Rslt_16Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, final_len_13, Rslt_16Args);
  Rslt_16Args = pair_make(APP, 0, new_i60(449), Rslt_16Args);
  Rslt_16Args = pair_make(APP, 0, glblStr149, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblProto22);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblProto22));
#endif
  // call = at core: 450
  Term Rslt_17Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, final_len_13_1, Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, new_i60(450), Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, glblStr149, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblProto16);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblProto16));
#endif
  // call or at core: 449
  Term Rslt_18Args = pair_make(APP, 0, Rslt_17, SUB);
  Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
  Rslt_18Args = pair_make(APP, 0, Rslt_16, Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, new_i60(449), Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, glblStr149, Rslt_18Args);
#ifdef STRICT
  store_redex(Rslt_18Args, glblProto26);
#else
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblProto26));
#endif
  Term Rslt_21 = make_op(OP_ADD, Rslt_20, new_start_7);
  // call SubString at core: 453
  Term Rslt_22Args = pair_make(APP, 0, final_len_13_2, SUB);
  Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
  Rslt_22Args = pair_make(APP, 0, Rslt_21, Rslt_22Args);
  Rslt_22Args = pair_make(APP, 0, Rslt_19, Rslt_22Args);
#ifdef STRICT
  store_redex(Rslt_22Args, glblVal279_1);
#else
  swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, glblVal279_1));
#endif

  // call cond at core: 449
  Term Rslt_23Args = pair_make(APP, 0, Rslt_22, SUB);
  Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
  Rslt_23Args = pair_make(APP, 0, glblStr281, Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, Rslt_18, Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, new_i60(449), Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, glblStr149, Rslt_23Args);
#ifdef STRICT
  store_redex(Rslt_23Args, glblProto30);
#else
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblProto30));
#endif
  // link args to body
  swapStore(r_seq_14, Rslt_23);

  Term Rslt_23V = get(term_loc(Rslt_23));
  apps = take(port(1, term_loc(Rslt_23V)));
  lams = take(port(2, term_loc(Rslt_23V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, apps, lams));
  Term Rslt_26 = make_op(OP_ADD, Rslt_24, Rslt_25);
  // call count at core: 448
  Term Rslt_28Args = pair_make(APP, 0, Rslt_27, SUB);
  Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
  Rslt_28Args = pair_make(APP, 0, new_i60(448), Rslt_28Args);
  Rslt_28Args = pair_make(APP, 0, glblStr149, Rslt_28Args);
#ifdef STRICT
  store_redex(Rslt_28Args, glblProto60);
#else
  swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, glblProto60));
#endif
  Term Rslt_29 = make_op(OP_SUB, Rslt_26, Rslt_28);
  // call min at core: 448
  Term Rslt_30Args = pair_make(APP, 0, Rslt_29, SUB);
  Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
  Rslt_30Args = pair_make(APP, 0, new_len_8, Rslt_30Args);
#ifdef STRICT
  store_redex(Rslt_30Args, glblFn229);
#else
  swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblFn229));
#endif

  // call  at core: 448
  Term Rslt_31Args = pair_make(APP, 0, Rslt_30, SUB);
  Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
#ifdef STRICT
  store_redex(Rslt_31Args, seq_14);
#else
  swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, seq_14));
#endif

  // link args to body
  swapStore(r_seq_9, Rslt_31);

  Term Rslt_31V = get(term_loc(Rslt_31));
  apps = take(port(1, term_loc(Rslt_31V)));
  lams = take(port(2, term_loc(Rslt_31V)));

  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_11, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_9);
  return;
}
Term glblsubs274 = new_ref(glblsubs273);
Term glblFld276;
void glblFldFn289(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld276 = new_ref(glblFldFn289);
Term glblFld277;
void glblFldFn290(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld277 = new_ref(glblFldFn290);
Term glblFld278;
void glblFldFn291(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld278 = new_ref(glblFldFn291);
Term glblcount295;
void glblcount294(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_34 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_34 = port(2, term_loc(seq_34));
  Term s_33 = term_new(VAR, 0, port(1, term_loc(seq_34)));
  // allocate args at core: 456
  swapStore(term_loc(s_33), ERA);
  // link args to body
  swapStore(r_seq_34, new_i60(77));

  V = get(r_seq_34);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_34, term_new(VAR, 0, vLoc));
  store_redex(args, seq_34);
  return;
}
Term glblcount295 = new_ref(glblcount294);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[19];
} str6 = {StringBufferType, REFS_STATIC, 0, 0, 18, "StringBuffer subs\n"};
Term glblStr298 = term_new_(VAL, (Term)&str6);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[14];
} str7 = {StringBufferType, REFS_STATIC, 0, 0, 13, "empty string\n"};
Term glblStr300 = term_new_(VAL, (Term)&str7);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[18];
} str8 = {StringBufferType, REFS_STATIC, 0, 0, 17, "non-empty string\n"};
Term glblStr301 = term_new_(VAL, (Term)&str8);
Term glblsubs293;
void glblsubs292(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_40 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_40 = port(2, term_loc(seq_40));
  Term len_39 = term_new(VAR, 0, port(1, term_loc(seq_40)));
  seq_40 = pair_make(LAM, 0, SUB, seq_40);
  Term start_38 = term_new(VAR, 0, port(1, term_loc(seq_40)));
  seq_40 = pair_make(LAM, 0, SUB, seq_40);
  Term s_37 = term_new(VAR, 0, port(1, term_loc(seq_40)));
  // allocate args at core: 459

  Term s_37_1;
  s_37 = dupeArg(s_37, &s_37_1, 0);

  Term start_38_1;
  start_38 = dupeArg(start_38, &start_38_1, 0);

  Term len_39_1;
  len_39 = dupeArg(len_39, &len_39_1, 0);
  Term seq_45 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_45 = port(2, term_loc(seq_45));
  Term new_len_44 = term_new(VAR, 0, port(1, term_loc(seq_45)));
  // allocate args at core: 461

  Term new_len_44_3;
  new_len_44 = dupeArg(new_len_44, &new_len_44_3, 0);

  Term new_len_44_2;
  new_len_44 = dupeArg(new_len_44, &new_len_44_2, 0);

  Term new_len_44_1;
  new_len_44 = dupeArg(new_len_44, &new_len_44_1, 0);
  Term seq_58 = pair_make(LAM, 0, ERA, NUL);
  Location r_seq_58 = port(2, term_loc(seq_58));
  // allocate args at : 0
  Term glblVal279_2;
  glblVal279 = dupeArg(glblVal279, &glblVal279_2, 0);

  Term seq_53 = pair_make(LAM, 0, ERA, NUL);
  Location r_seq_53 = port(2, term_loc(seq_53));
  // allocate args at : 0
  // call pr* at core: 460
  Term Rslt_42Args = pair_make(APP, 0, glblStr298, SUB);
  Term Rslt_42 = term_new(VAR, 0, port(2, term_loc(Rslt_42Args)));
#ifdef STRICT
  store_redex(Rslt_42Args, glblFn114);
#else
  swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, Rslt_42Args, glblFn114));
#endif

  // call number-str at core: 462
  Term Rslt_47Args = pair_make(APP, 0, new_len_44, SUB);
  Term Rslt_47 = term_new(VAR, 0, port(2, term_loc(Rslt_47Args)));
#ifdef STRICT
  store_redex(Rslt_47Args, glblFn192);
#else
  swapStore(term_loc(Rslt_47), pair_make(LAZ, 0, Rslt_47Args, glblFn192));
#endif

  // call pr* at core: 462
  Term Rslt_48Args = pair_make(APP, 0, Rslt_47, SUB);
  Term Rslt_48 = term_new(VAR, 0, port(2, term_loc(Rslt_48Args)));
#ifdef STRICT
  store_redex(Rslt_48Args, glblFn114);
#else
  swapStore(term_loc(Rslt_48), pair_make(LAZ, 0, Rslt_48Args, glblFn114));
#endif

  // call < at core: 463
  Term Rslt_49Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_49 = term_new(VAR, 0, port(2, term_loc(Rslt_49Args)));
  Rslt_49Args = pair_make(APP, 0, new_len_44_1, Rslt_49Args);
  Rslt_49Args = pair_make(APP, 0, new_i60(463), Rslt_49Args);
  Rslt_49Args = pair_make(APP, 0, glblStr149, Rslt_49Args);
#ifdef STRICT
  store_redex(Rslt_49Args, glblProto22);
#else
  swapStore(term_loc(Rslt_49), pair_make(LAZ, 0, Rslt_49Args, glblProto22));
#endif
  // call = at core: 464
  Term Rslt_50Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_50 = term_new(VAR, 0, port(2, term_loc(Rslt_50Args)));
  Rslt_50Args = pair_make(APP, 0, new_len_44_2, Rslt_50Args);
  Rslt_50Args = pair_make(APP, 0, new_i60(464), Rslt_50Args);
  Rslt_50Args = pair_make(APP, 0, glblStr149, Rslt_50Args);
#ifdef STRICT
  store_redex(Rslt_50Args, glblProto16);
#else
  swapStore(term_loc(Rslt_50), pair_make(LAZ, 0, Rslt_50Args, glblProto16));
#endif
  // call or at core: 463
  Term Rslt_51Args = pair_make(APP, 0, Rslt_50, SUB);
  Term Rslt_51 = term_new(VAR, 0, port(2, term_loc(Rslt_51Args)));
  Rslt_51Args = pair_make(APP, 0, Rslt_49, Rslt_51Args);
  Rslt_51Args = pair_make(APP, 0, new_i60(463), Rslt_51Args);
  Rslt_51Args = pair_make(APP, 0, glblStr149, Rslt_51Args);
#ifdef STRICT
  store_redex(Rslt_51Args, glblProto26);
#else
  swapStore(term_loc(Rslt_51), pair_make(LAZ, 0, Rslt_51Args, glblProto26));
#endif
  // call pr* at core: 466
  Term Rslt_55Args = pair_make(APP, 0, glblStr300, SUB);
  Term Rslt_55 = term_new(VAR, 0, port(2, term_loc(Rslt_55Args)));
#ifdef STRICT
  store_redex(Rslt_55Args, glblFn114);
#else
  swapStore(term_loc(Rslt_55), pair_make(LAZ, 0, Rslt_55Args, glblFn114));
#endif

  // link args to body
  swapStore(r_seq_53, glblStr281);

  V = get(r_seq_53);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_55, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_53, term_new(VAR, 0, vLoc));
  // call  at : 0
  Term Rslt_56Args = pair_make(APP, 0, NUL, SUB);
  Term Rslt_56 = term_new(VAR, 0, port(2, term_loc(Rslt_56Args)));
#ifdef STRICT
  store_redex(Rslt_56Args, seq_53);
#else
  swapStore(term_loc(Rslt_56), pair_make(LAZ, 0, Rslt_56Args, seq_53));
#endif

  // call pr* at core: 470
  Term Rslt_60Args = pair_make(APP, 0, glblStr301, SUB);
  Term Rslt_60 = term_new(VAR, 0, port(2, term_loc(Rslt_60Args)));
#ifdef STRICT
  store_redex(Rslt_60Args, glblFn114);
#else
  swapStore(term_loc(Rslt_60), pair_make(LAZ, 0, Rslt_60Args, glblFn114));
#endif

  // call SubString at core: 471
  Term Rslt_61Args = pair_make(APP, 0, new_len_44_3, SUB);
  Term Rslt_61 = term_new(VAR, 0, port(2, term_loc(Rslt_61Args)));
  Rslt_61Args = pair_make(APP, 0, start_38, Rslt_61Args);
  Rslt_61Args = pair_make(APP, 0, s_37, Rslt_61Args);
#ifdef STRICT
  store_redex(Rslt_61Args, glblVal279_2);
#else
  swapStore(term_loc(Rslt_61), pair_make(LAZ, 0, Rslt_61Args, glblVal279_2));
#endif

  // link args to body
  swapStore(r_seq_58, Rslt_61);

  Term Rslt_61V = get(term_loc(Rslt_61));
  apps = take(port(1, term_loc(Rslt_61V)));
  lams = take(port(2, term_loc(Rslt_61V)));

  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_60, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(term_loc(Rslt_61), pair_make(LAZ, 0, apps, lams));
  // call  at : 0
  Term Rslt_62Args = pair_make(APP, 0, NUL, SUB);
  Term Rslt_62 = term_new(VAR, 0, port(2, term_loc(Rslt_62Args)));
#ifdef STRICT
  store_redex(Rslt_62Args, seq_58);
#else
  swapStore(term_loc(Rslt_62), pair_make(LAZ, 0, Rslt_62Args, seq_58));
#endif

  // call cond at core: 463
  Term Rslt_63Args = pair_make(APP, 0, Rslt_62, SUB);
  Term Rslt_63 = term_new(VAR, 0, port(2, term_loc(Rslt_63Args)));
  Rslt_63Args = pair_make(APP, 0, Rslt_56, Rslt_63Args);
  Rslt_63Args = pair_make(APP, 0, Rslt_51, Rslt_63Args);
  Rslt_63Args = pair_make(APP, 0, new_i60(463), Rslt_63Args);
  Rslt_63Args = pair_make(APP, 0, glblStr149, Rslt_63Args);
#ifdef STRICT
  store_redex(Rslt_63Args, glblProto30);
#else
  swapStore(term_loc(Rslt_63), pair_make(LAZ, 0, Rslt_63Args, glblProto30));
#endif
  // link args to body
  swapStore(r_seq_45, Rslt_63);

  Term Rslt_63V = get(term_loc(Rslt_63));
  apps = take(port(1, term_loc(Rslt_63V)));
  lams = take(port(2, term_loc(Rslt_63V)));

  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_48, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(term_loc(Rslt_63), pair_make(LAZ, 0, apps, lams));
  Term Rslt_64 = make_op(OP_ADD, start_38_1, len_39_1);
  // call count at core: 461
  Term Rslt_65Args = pair_make(APP, 0, s_37_1, SUB);
  Term Rslt_65 = term_new(VAR, 0, port(2, term_loc(Rslt_65Args)));
  Rslt_65Args = pair_make(APP, 0, new_i60(461), Rslt_65Args);
  Rslt_65Args = pair_make(APP, 0, glblStr149, Rslt_65Args);
#ifdef STRICT
  store_redex(Rslt_65Args, glblProto60);
#else
  swapStore(term_loc(Rslt_65), pair_make(LAZ, 0, Rslt_65Args, glblProto60));
#endif
  Term Rslt_66 = make_op(OP_SUB, Rslt_64, Rslt_65);
  // call min at core: 461
  Term Rslt_67Args = pair_make(APP, 0, Rslt_66, SUB);
  Term Rslt_67 = term_new(VAR, 0, port(2, term_loc(Rslt_67Args)));
  Rslt_67Args = pair_make(APP, 0, len_39, Rslt_67Args);
#ifdef STRICT
  store_redex(Rslt_67Args, glblFn229);
#else
  swapStore(term_loc(Rslt_67), pair_make(LAZ, 0, Rslt_67Args, glblFn229));
#endif

  // call  at core: 461
  Term Rslt_68Args = pair_make(APP, 0, Rslt_67, SUB);
  Term Rslt_68 = term_new(VAR, 0, port(2, term_loc(Rslt_68Args)));
#ifdef STRICT
  store_redex(Rslt_68Args, seq_45);
#else
  swapStore(term_loc(Rslt_68), pair_make(LAZ, 0, Rslt_68Args, seq_45));
#endif

  // link args to body
  swapStore(r_seq_40, Rslt_68);

  Term Rslt_68V = get(term_loc(Rslt_68));
  apps = take(port(1, term_loc(Rslt_68V)));
  lams = take(port(2, term_loc(Rslt_68V)));

  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_42, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(term_loc(Rslt_68), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_40);
  return;
}
Term glblsubs293 = new_ref(glblsubs292);
Term glblFn297;
void glblCFn296(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];
    result = strEQ(x_1, y_2);
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn297 = new_ref(glblCFn296);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str9 = {StringBufferType, REFS_STATIC, 0, 0, 6, "String"};
Term glblStr326 = term_new_(VAL, (Term)&str9);
Term glbltype_name323;
void glbltype_name322(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term arg_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at core: 485
  swapStore(term_loc(arg_6), ERA);
  // link args to body
  swapStore(r_seq_7, glblStr326);

  V = get(r_seq_7);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_7, term_new(VAR, 0, vLoc));
  store_redex(args, seq_7);
  return;
}
Term glbltype_name323 = new_ref(glbltype_name322);
Term glbltype_name325;
void glbltype_name324(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_11 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_11 = port(2, term_loc(seq_11));
  Term arg_10 = term_new(VAR, 0, port(1, term_loc(seq_11)));
  // allocate args at core: 485
  swapStore(term_loc(arg_10), ERA);
  // link args to body
  swapStore(r_seq_11, glblStr326);

  V = get(r_seq_11);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_11, term_new(VAR, 0, vLoc));
  store_redex(args, seq_11);
  return;
}
Term glbltype_name325 = new_ref(glbltype_name324);
Term glblstr_vect319;
void glblstr_vect318(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_15 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_15 = port(2, term_loc(seq_15));
  Term s_14 = term_new(VAR, 0, port(1, term_loc(seq_15)));
  // allocate args at core: 488
  Term glblVal130_4;
  glblVal130 = dupeArg(glblVal130, &glblVal130_4, 0);

  // call vect-conj at core: 489
  Term Rslt_17Args = pair_make(APP, 0, s_14, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, glblVal130_4, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_15, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_15);
  return;
}
Term glblstr_vect319 = new_ref(glblstr_vect318);
Term glblstr_vect321;
void glblstr_vect320(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_20 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_20 = port(2, term_loc(seq_20));
  Term s_19 = term_new(VAR, 0, port(1, term_loc(seq_20)));
  // allocate args at core: 488
  Term glblVal130_5;
  glblVal130 = dupeArg(glblVal130, &glblVal130_5, 0);

  // call vect-conj at core: 489
  Term Rslt_22Args = pair_make(APP, 0, s_19, SUB);
  Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
  Rslt_22Args = pair_make(APP, 0, glblVal130_5, Rslt_22Args);
#ifdef STRICT
  store_redex(Rslt_22Args, glblFn133);
#else
  swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_20, Rslt_22);

  Term Rslt_22V = get(term_loc(Rslt_22));
  apps = take(port(1, term_loc(Rslt_22V)));
  lams = take(port(2, term_loc(Rslt_22V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_20);
  return;
}
Term glblstr_vect321 = new_ref(glblstr_vect320);
Term glblrecurse315;
void glblrecurse314(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_26 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_26 = port(2, term_loc(seq_26));
  Term f_25 = term_new(VAR, 0, port(1, term_loc(seq_26)));
  seq_26 = pair_make(LAM, 0, SUB, seq_26);
  Term s_24 = term_new(VAR, 0, port(1, term_loc(seq_26)));
  // allocate args at core: 491
  swapStore(term_loc(f_25), ERA);
  // link args to body
  swapStore(r_seq_26, s_24);

  V = get(r_seq_26);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_26, term_new(VAR, 0, vLoc));
  store_redex(args, seq_26);
  return;
}
Term glblrecurse315 = new_ref(glblrecurse314);
Term glblrecurse317;
void glblrecurse316(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_31 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_31 = port(2, term_loc(seq_31));
  Term f_30 = term_new(VAR, 0, port(1, term_loc(seq_31)));
  seq_31 = pair_make(LAM, 0, SUB, seq_31);
  Term s_29 = term_new(VAR, 0, port(1, term_loc(seq_31)));
  // allocate args at core: 491
  swapStore(term_loc(f_30), ERA);
  // link args to body
  swapStore(r_seq_31, s_29);

  V = get(r_seq_31);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_31, term_new(VAR, 0, vLoc));
  store_redex(args, seq_31);
  return;
}
Term glblrecurse317 = new_ref(glblrecurse316);
Term glbl_EQ_311;
void glbl_EQ_310(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_36 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_36 = port(2, term_loc(seq_36));
  Term y_35 = term_new(VAR, 0, port(1, term_loc(seq_36)));
  seq_36 = pair_make(LAM, 0, SUB, seq_36);
  Term x_34 = term_new(VAR, 0, port(1, term_loc(seq_36)));
  // allocate args at core: 494
  // call str-eq at core: 495
  Term Rslt_38Args = pair_make(APP, 0, y_35, SUB);
  Term Rslt_38 = term_new(VAR, 0, port(2, term_loc(Rslt_38Args)));
  Rslt_38Args = pair_make(APP, 0, x_34, Rslt_38Args);
#ifdef STRICT
  store_redex(Rslt_38Args, glblFn297);
#else
  swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, Rslt_38Args, glblFn297));
#endif

  // link args to body
  swapStore(r_seq_36, Rslt_38);

  Term Rslt_38V = get(term_loc(Rslt_38));
  apps = take(port(1, term_loc(Rslt_38V)));
  lams = take(port(2, term_loc(Rslt_38V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_36);
  return;
}
Term glbl_EQ_311 = new_ref(glbl_EQ_310);
Term glbl_EQ_313;
void glbl_EQ_312(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_42 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_42 = port(2, term_loc(seq_42));
  Term y_41 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  seq_42 = pair_make(LAM, 0, SUB, seq_42);
  Term x_40 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  // allocate args at core: 494
  // call str-eq at core: 495
  Term Rslt_44Args = pair_make(APP, 0, y_41, SUB);
  Term Rslt_44 = term_new(VAR, 0, port(2, term_loc(Rslt_44Args)));
  Rslt_44Args = pair_make(APP, 0, x_40, Rslt_44Args);
#ifdef STRICT
  store_redex(Rslt_44Args, glblFn297);
#else
  swapStore(term_loc(Rslt_44), pair_make(LAZ, 0, Rslt_44Args, glblFn297));
#endif

  // link args to body
  swapStore(r_seq_42, Rslt_44);

  Term Rslt_44V = get(term_loc(Rslt_44));
  apps = take(port(1, term_loc(Rslt_44V)));
  lams = take(port(2, term_loc(Rslt_44V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_44), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_42);
  return;
}
Term glbl_EQ_313 = new_ref(glbl_EQ_312);
Term glblFn328;
void glblCFn327(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term v_1 = arityArgs.args[0];
    Term n_2 = arityArgs.args[1];

    result = term_val((Term)vectorGet(v_1, n_2));
    moveStore(port(2, term_loc(args)), result);
  }
  return;
}
Term glblFn328 = new_ref(glblCFn327);
Term glblProto330;
Term glblvect_reduce342;
void glblvect_reduce341(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_13 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_13 = port(2, term_loc(seq_13));
  Term f_12 = term_new(VAR, 0, port(1, term_loc(seq_13)));
  seq_13 = pair_make(LAM, 0, SUB, seq_13);
  Term result_11 = term_new(VAR, 0, port(1, term_loc(seq_13)));
  seq_13 = pair_make(LAM, 0, SUB, seq_13);
  Term n_10 = term_new(VAR, 0, port(1, term_loc(seq_13)));
  seq_13 = pair_make(LAM, 0, SUB, seq_13);
  Term vect_9 = term_new(VAR, 0, port(1, term_loc(seq_13)));
  seq_13 = pair_make(LAM, 0, SUB, seq_13);
  Term el_8 = term_new(VAR, 0, port(1, term_loc(seq_13)));
  // allocate args at core: 601
  swapStore(term_loc(el_8), ERA);
  swapStore(term_loc(vect_9), ERA);
  swapStore(term_loc(n_10), ERA);
  swapStore(term_loc(f_12), ERA);
  // link args to body
  swapStore(r_seq_13, result_11);

  V = get(r_seq_13);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_13, term_new(VAR, 0, vLoc));
  store_redex(args, seq_13);
  return;
}
Term glblvect_reduce342 = new_ref(glblvect_reduce341);
Term glblvect_reduce344;
void glblvect_reduce343(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_21 = port(2, term_loc(seq_21));
  Term f_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term result_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term n_18 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term vect_17 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  seq_21 = pair_make(LAM, 0, SUB, seq_21);
  Term el_16 = term_new(VAR, 0, port(1, term_loc(seq_21)));
  // allocate args at core: 604

  Term vect_17_1;
  vect_17 = dupeArg(vect_17, &vect_17_1, 0);

  Term n_18_1;
  n_18 = dupeArg(n_18, &n_18_1, 0);

  Term f_20_1;
  f_20 = dupeArg(f_20, &f_20_1, 0);
  // call get at core: 605
  Term Rslt_23Args = pair_make(APP, 0, n_18, SUB);
  Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
  Rslt_23Args = pair_make(APP, 0, vect_17, Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, new_i60(605), Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, glblStr149, Rslt_23Args);
#ifdef STRICT
  store_redex(Rslt_23Args, glblProto100);
#else
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblProto100));
#endif
  // call inc at core: 605
  Term Rslt_24Args = pair_make(APP, 0, n_18_1, SUB);
  Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
  store_redex(Rslt_24Args, glblFn119);
#else
  swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, glblFn119));
#endif

  // call .x at core: 605
  Term Rslt_25Args = pair_make(APP, 0, el_16, SUB);
  Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
  Rslt_25Args = pair_make(APP, 0, new_i60(605), Rslt_25Args);
  Rslt_25Args = pair_make(APP, 0, glblStr149, Rslt_25Args);
#ifdef STRICT
  store_redex(Rslt_25Args, glblProto14);
#else
  swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, glblProto14));
#endif
  // call f at core: 604
  Term Rslt_26Args = pair_make(APP, 0, Rslt_25, SUB);
  Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
  Rslt_26Args = pair_make(APP, 0, result_19, Rslt_26Args);
#ifdef STRICT
  store_redex(Rslt_26Args, f_20);
#else
  swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, f_20));
#endif

  // call vect-reduce at core: 605
  Term Rslt_27Args = pair_make(APP, 0, f_20_1, SUB);
  Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
  Rslt_27Args = pair_make(APP, 0, Rslt_26, Rslt_27Args);
  Rslt_27Args = pair_make(APP, 0, Rslt_24, Rslt_27Args);
  Rslt_27Args = pair_make(APP, 0, vect_17_1, Rslt_27Args);
  Rslt_27Args = pair_make(APP, 0, Rslt_23, Rslt_27Args);
  Rslt_27Args = pair_make(APP, 0, new_i60(605), Rslt_27Args);
  Rslt_27Args = pair_make(APP, 0, glblStr149, Rslt_27Args);
#ifdef STRICT
  store_redex(Rslt_27Args, glblProto330);
#else
  swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, glblProto330));
#endif
  // link args to body
  swapStore(r_seq_21, Rslt_27);

  Term Rslt_27V = get(term_loc(Rslt_27));
  apps = take(port(1, term_loc(Rslt_27V)));
  lams = take(port(2, term_loc(Rslt_27V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_21);
  return;
}
Term glblvect_reduce344 = new_ref(glblvect_reduce343);
Term glblFn351;
void glblCFn350(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_5 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_5 = port(2, term_loc(seq_5));
  Term result_4 = term_new(VAR, 0, port(1, term_loc(seq_5)));
  seq_5 = pair_make(LAM, 0, SUB, seq_5);
  Term max_index_3 = term_new(VAR, 0, port(1, term_loc(seq_5)));
  seq_5 = pair_make(LAM, 0, SUB, seq_5);
  Term curr_index_2 = term_new(VAR, 0, port(1, term_loc(seq_5)));
  seq_5 = pair_make(LAM, 0, SUB, seq_5);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_5)));
  // allocate args at core: 607

  Term v_1_1;
  v_1 = dupeArg(v_1, &v_1_1, 0);

  Term curr_index_2_2;
  curr_index_2 = dupeArg(curr_index_2, &curr_index_2_2, 0);

  Term curr_index_2_1;
  curr_index_2 = dupeArg(curr_index_2, &curr_index_2_1, 0);

  Term max_index_3_1;
  max_index_3 = dupeArg(max_index_3, &max_index_3_1, 0);

  Term result_4_1;
  result_4 = dupeArg(result_4, &result_4_1, 0);
  Term seq_11 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_11 = port(2, term_loc(seq_11));
  Term x_10 = term_new(VAR, 0, port(1, term_loc(seq_11)));
  // allocate args at core: 614
  // call <= at core: 612
  Term Rslt_7Args = pair_make(APP, 0, max_index_3, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
  Rslt_7Args = pair_make(APP, 0, curr_index_2, Rslt_7Args);
#ifdef STRICT
  store_redex(Rslt_7Args, glblFn194);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn194));
#endif

  // call vect-get at core: 613
  Term Rslt_8Args = pair_make(APP, 0, curr_index_2_1, SUB);
  Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
  Rslt_8Args = pair_make(APP, 0, v_1, Rslt_8Args);
#ifdef STRICT
  store_redex(Rslt_8Args, glblFn328);
#else
  swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn328));
#endif

  // call inc at core: 615
  Term Rslt_13Args = pair_make(APP, 0, curr_index_2_2, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
#ifdef STRICT
  store_redex(Rslt_13Args, glblFn119);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn119));
#endif

  // call vect-conj at core: 615
  Term Rslt_14Args = pair_make(APP, 0, x_10, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, result_4, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblFn133);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn133));
#endif

  // call subvec* at core: 607
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
  Rslt_15Args = pair_make(APP, 0, max_index_3_1, Rslt_15Args);
  Rslt_15Args = pair_make(APP, 0, Rslt_13, Rslt_15Args);
  Rslt_15Args = pair_make(APP, 0, v_1_1, Rslt_15Args);
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn351);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn351));
#endif

  // link args to body
  swapStore(r_seq_11, Rslt_15);

  Term Rslt_15V = get(term_loc(Rslt_15));
  apps = take(port(1, term_loc(Rslt_15V)));
  lams = take(port(2, term_loc(Rslt_15V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, apps, lams));
  // call map at core: 613
  Term Rslt_16Args = pair_make(APP, 0, seq_11, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_8, Rslt_16Args);
  Rslt_16Args = pair_make(APP, 0, new_i60(613), Rslt_16Args);
  Rslt_16Args = pair_make(APP, 0, glblStr149, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblProto38);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblProto38));
#endif
  // call and at core: 612
  Term Rslt_17Args = pair_make(APP, 0, Rslt_16, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_7, Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, new_i60(612), Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, glblStr149, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblProto24);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblProto24));
#endif
  // call either at core: 612
  Term Rslt_18Args = pair_make(APP, 0, result_4_1, SUB);
  Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
  Rslt_18Args = pair_make(APP, 0, Rslt_17, Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, new_i60(612), Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, glblStr149, Rslt_18Args);
#ifdef STRICT
  store_redex(Rslt_18Args, glblProto28);
#else
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblProto28));
#endif
  // link args to body
  swapStore(r_seq_5, Rslt_18);

  Term Rslt_18V = get(term_loc(Rslt_18));
  apps = take(port(1, term_loc(Rslt_18V)));
  lams = take(port(2, term_loc(Rslt_18V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_5);
  return;
}
Term glblFn351 = new_ref(glblCFn350);
Term glblFn353;
void glblCFn352(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_4 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_4 = port(2, term_loc(seq_4));
  Term len_3 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  seq_4 = pair_make(LAM, 0, SUB, seq_4);
  Term start_2 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  seq_4 = pair_make(LAM, 0, SUB, seq_4);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  // allocate args at core: 618

  Term start_2_1;
  start_2 = dupeArg(start_2, &start_2_1, 0);
  Term glblVal130_6;
  glblVal130 = dupeArg(glblVal130, &glblVal130_6, 0);

  Term Rslt_6 = make_op(OP_ADD, start_2_1, len_3);
  // call dec at core: 620
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblFn126);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn126));
#endif

  // call subvec* at core: 620
  Term Rslt_8Args = pair_make(APP, 0, glblVal130_6, SUB);
  Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
  Rslt_8Args = pair_make(APP, 0, Rslt_7, Rslt_8Args);
  Rslt_8Args = pair_make(APP, 0, start_2, Rslt_8Args);
  Rslt_8Args = pair_make(APP, 0, v_1, Rslt_8Args);
#ifdef STRICT
  store_redex(Rslt_8Args, glblFn351);
#else
  swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn351));
#endif

  // link args to body
  swapStore(r_seq_4, Rslt_8);

  Term Rslt_8V = get(term_loc(Rslt_8));
  apps = take(port(1, term_loc(Rslt_8V)));
  lams = take(port(2, term_loc(Rslt_8V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_4);
  return;
}
Term glblFn353 = new_ref(glblCFn352);
Term glblVal354 = new_ref(vectMap);
Term glblVal354;
void glblProto106_366(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(625), args);
  args = pair_make(APP, 0, glblStr149, args);
  interact(args, glblProto106);
}
Term glblFn361;
void glblCFn360(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term vect_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 624
  Term seq_6 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_6 = port(2, term_loc(seq_6));
  Term ss_vect_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
  // allocate args at core: 625

  Term ss_vect_5_1;
  ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_1, 0);
  Term seq_17 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_17 = port(2, term_loc(seq_17));
  Term s_16 = term_new(VAR, 0, port(1, term_loc(seq_17)));
  seq_17 = pair_make(LAM, 0, SUB, seq_17);
  Term len_15 = term_new(VAR, 0, port(1, term_loc(seq_17)));
  // allocate args at core: 626
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term new_len_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at core: 626
  // call str-malloc at core: 628
  Term Rslt_12Args = pair_make(APP, 0, new_len_9, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn258);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn258));
#endif

  // call reduce at core: 628
  Term Rslt_13Args = pair_make(APP, 0, glblFn261, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, Rslt_12, Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, ss_vect_5, Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, new_i60(628), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr149, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto68);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto68));
#endif
  // link args to body
  swapStore(r_seq_10, Rslt_13);

  Term Rslt_13V = get(term_loc(Rslt_13));
  apps = take(port(1, term_loc(Rslt_13V)));
  lams = take(port(2, term_loc(Rslt_13V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, apps, lams));
  // call count at core: 627
  Term Rslt_19Args = pair_make(APP, 0, s_16, SUB);
  Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
  Rslt_19Args = pair_make(APP, 0, new_i60(627), Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, glblStr149, Rslt_19Args);
#ifdef STRICT
  store_redex(Rslt_19Args, glblProto60);
#else
  swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblProto60));
#endif
  Term Rslt_20 = make_op(OP_ADD, len_15, Rslt_19);
  // link args to body
  swapStore(r_seq_17, Rslt_20);

  Term Rslt_20V = get(term_loc(Rslt_20));
  apps = take(port(1, term_loc(Rslt_20V)));
  lams = take(port(2, term_loc(Rslt_20V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, apps, lams));
  // call reduce at core: 626
  Term Rslt_21Args = pair_make(APP, 0, seq_17, SUB);
  Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
  Rslt_21Args = pair_make(APP, 0, new_i60(0), Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, ss_vect_5_1, Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, new_i60(626), Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, glblStr149, Rslt_21Args);
#ifdef STRICT
  store_redex(Rslt_21Args, glblProto68);
#else
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblProto68));
#endif
  // call  at core: 626
  Term Rslt_22Args = pair_make(APP, 0, Rslt_21, SUB);
  Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
#ifdef STRICT
  store_redex(Rslt_22Args, seq_10);
#else
  swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, seq_10));
#endif

  // link args to body
  swapStore(r_seq_6, Rslt_22);

  Term Rslt_22V = get(term_loc(Rslt_22));
  apps = take(port(1, term_loc(Rslt_22V)));
  lams = take(port(2, term_loc(Rslt_22V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, apps, lams));
  // call flat-map at core: 625
  Term Rslt_23Args = pair_make(APP, 0, new_ref(glblProto106_366), SUB);
  Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
  Rslt_23Args = pair_make(APP, 0, vect_1, Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, new_i60(625), Rslt_23Args);
  Rslt_23Args = pair_make(APP, 0, glblStr149, Rslt_23Args);
#ifdef STRICT
  store_redex(Rslt_23Args, glblProto42);
#else
  swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblProto42));
#endif
  // call  at core: 625
  Term Rslt_24Args = pair_make(APP, 0, Rslt_23, SUB);
  Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
  store_redex(Rslt_24Args, seq_6);
#else
  swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, seq_6));
#endif

  // link args to body
  swapStore(r_seq_2, Rslt_24);

  Term Rslt_24V = get(term_loc(Rslt_24));
  apps = take(port(1, term_loc(Rslt_24V)));
  lams = take(port(2, term_loc(Rslt_24V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_2);
  return;
}
Term glblFn361 = new_ref(glblCFn360);
Term glblFn370;
void glblCFn369(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_4 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_4 = port(2, term_loc(seq_4));
  Term start_3 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  seq_4 = pair_make(LAM, 0, SUB, seq_4);
  Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  seq_4 = pair_make(LAM, 0, SUB, seq_4);
  Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_4)));
  // allocate args at core: 630

  Term x_1_3;
  x_1 = dupeArg(x_1, &x_1_3, 0);

  Term x_1_2;
  x_1 = dupeArg(x_1, &x_1_2, 0);

  Term x_1_1;
  x_1 = dupeArg(x_1, &x_1_1, 0);

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 0);

  Term start_3_3;
  start_3 = dupeArg(start_3, &start_3_3, 0);

  Term start_3_2;
  start_3 = dupeArg(start_3, &start_3_2, 0);

  Term start_3_1;
  start_3 = dupeArg(start_3, &start_3_1, 0);
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term arg_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at core: 635
  swapStore(term_loc(arg_9), ERA);
  // call count at core: 634
  Term Rslt_6Args = pair_make(APP, 0, x_1, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
  Rslt_6Args = pair_make(APP, 0, new_i60(634), Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, glblStr149, Rslt_6Args);
#ifdef STRICT
  store_redex(Rslt_6Args, glblProto60);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblProto60));
#endif
  // call = at core: 634
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
  Rslt_7Args = pair_make(APP, 0, start_3, Rslt_7Args);
  Rslt_7Args = pair_make(APP, 0, new_i60(634), Rslt_7Args);
  Rslt_7Args = pair_make(APP, 0, glblStr149, Rslt_7Args);
#ifdef STRICT
  store_redex(Rslt_7Args, glblProto16);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblProto16));
#endif
  // link args to body
  swapStore(r_seq_10, x_1_1);

  V = get(r_seq_10);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_10, term_new(VAR, 0, vLoc));
  // call map at core: 634
  Term Rslt_12Args = pair_make(APP, 0, seq_10, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_7, Rslt_12Args);
  Rslt_12Args = pair_make(APP, 0, new_i60(634), Rslt_12Args);
  Rslt_12Args = pair_make(APP, 0, glblStr149, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblProto38);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblProto38));
#endif
  // call nth at core: 637
  Term Rslt_13Args = pair_make(APP, 0, start_3_1, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, x_1_2, Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, new_i60(637), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr149, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto94);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto94));
#endif
  // call extract at core: 637
  Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, new_i60(637), Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, glblStr149, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblProto52);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblProto52));
#endif
  // call nth at core: 637
  Term Rslt_15Args = pair_make(APP, 0, start_3_2, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
  Rslt_15Args = pair_make(APP, 0, y_2, Rslt_15Args);
  Rslt_15Args = pair_make(APP, 0, new_i60(637), Rslt_15Args);
  Rslt_15Args = pair_make(APP, 0, glblStr149, Rslt_15Args);
#ifdef STRICT
  store_redex(Rslt_15Args, glblProto94);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblProto94));
#endif
  // call extract at core: 637
  Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, new_i60(637), Rslt_16Args);
  Rslt_16Args = pair_make(APP, 0, glblStr149, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblProto52);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblProto52));
#endif
  // call = at core: 637
  Term Rslt_17Args = pair_make(APP, 0, Rslt_16, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_14, Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, new_i60(637), Rslt_17Args);
  Rslt_17Args = pair_make(APP, 0, glblStr149, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblProto16);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblProto16));
#endif
  // call inc at core: 638
  Term Rslt_18Args = pair_make(APP, 0, start_3_3, SUB);
  Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
#ifdef STRICT
  store_redex(Rslt_18Args, glblFn119);
#else
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblFn119));
#endif

  // call vect-= at core: 630
  Term Rslt_19Args = pair_make(APP, 0, Rslt_18, SUB);
  Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
  Rslt_19Args = pair_make(APP, 0, y_2_1, Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, x_1_3, Rslt_19Args);
#ifdef STRICT
  store_redex(Rslt_19Args, glblFn370);
#else
  swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblFn370));
#endif

  // call and at core: 637
  Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
  Rslt_20Args = pair_make(APP, 0, Rslt_17, Rslt_20Args);
  Rslt_20Args = pair_make(APP, 0, new_i60(637), Rslt_20Args);
  Rslt_20Args = pair_make(APP, 0, glblStr149, Rslt_20Args);
#ifdef STRICT
  store_redex(Rslt_20Args, glblProto24);
#else
  swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, glblProto24));
#endif
  // call or at core: 634
  Term Rslt_21Args = pair_make(APP, 0, Rslt_20, SUB);
  Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
  Rslt_21Args = pair_make(APP, 0, Rslt_12, Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, new_i60(634), Rslt_21Args);
  Rslt_21Args = pair_make(APP, 0, glblStr149, Rslt_21Args);
#ifdef STRICT
  store_redex(Rslt_21Args, glblProto26);
#else
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblProto26));
#endif
  // link args to body
  swapStore(r_seq_4, Rslt_21);

  Term Rslt_21V = get(term_loc(Rslt_21));
  apps = take(port(1, term_loc(Rslt_21V)));
  lams = take(port(2, term_loc(Rslt_21V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_4);
  return;
}
Term glblFn370 = new_ref(glblCFn369);
Term glbl_EQ_419;
void glbl_EQ_418(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_25 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_25 = port(2, term_loc(seq_25));
  Term y_24 = term_new(VAR, 0, port(1, term_loc(seq_25)));
  seq_25 = pair_make(LAM, 0, SUB, seq_25);
  Term x_23 = term_new(VAR, 0, port(1, term_loc(seq_25)));
  // allocate args at core: 641

  Term x_23_2;
  x_23 = dupeArg(x_23, &x_23_2, 0);

  Term x_23_1;
  x_23 = dupeArg(x_23, &x_23_1, 0);

  Term y_24_2;
  y_24 = dupeArg(y_24, &y_24_2, 0);

  Term y_24_1;
  y_24 = dupeArg(y_24, &y_24_1, 0);
  // call type-num at core: 642
  Term Rslt_27Args = pair_make(APP, 0, x_23, SUB);
  Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
#ifdef STRICT
  store_redex(Rslt_27Args, glblFn8);
#else
  swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, glblFn8));
#endif

  // call type-num at core: 642
  Term Rslt_28Args = pair_make(APP, 0, y_24, SUB);
  Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
#ifdef STRICT
  store_redex(Rslt_28Args, glblFn8);
#else
  swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, glblFn8));
#endif

  // call = at core: 642
  Term Rslt_29Args = pair_make(APP, 0, Rslt_28, SUB);
  Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
  Rslt_29Args = pair_make(APP, 0, Rslt_27, Rslt_29Args);
  Rslt_29Args = pair_make(APP, 0, new_i60(642), Rslt_29Args);
  Rslt_29Args = pair_make(APP, 0, glblStr149, Rslt_29Args);
#ifdef STRICT
  store_redex(Rslt_29Args, glblProto16);
#else
  swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, glblProto16));
#endif
  // call count at core: 643
  Term Rslt_30Args = pair_make(APP, 0, x_23_1, SUB);
  Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
  Rslt_30Args = pair_make(APP, 0, new_i60(643), Rslt_30Args);
  Rslt_30Args = pair_make(APP, 0, glblStr149, Rslt_30Args);
#ifdef STRICT
  store_redex(Rslt_30Args, glblProto60);
#else
  swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblProto60));
#endif
  // call count at core: 643
  Term Rslt_31Args = pair_make(APP, 0, y_24_1, SUB);
  Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
  Rslt_31Args = pair_make(APP, 0, new_i60(643), Rslt_31Args);
  Rslt_31Args = pair_make(APP, 0, glblStr149, Rslt_31Args);
#ifdef STRICT
  store_redex(Rslt_31Args, glblProto60);
#else
  swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, glblProto60));
#endif
  // call = at core: 643
  Term Rslt_32Args = pair_make(APP, 0, Rslt_31, SUB);
  Term Rslt_32 = term_new(VAR, 0, port(2, term_loc(Rslt_32Args)));
  Rslt_32Args = pair_make(APP, 0, Rslt_30, Rslt_32Args);
  Rslt_32Args = pair_make(APP, 0, new_i60(643), Rslt_32Args);
  Rslt_32Args = pair_make(APP, 0, glblStr149, Rslt_32Args);
#ifdef STRICT
  store_redex(Rslt_32Args, glblProto16);
#else
  swapStore(term_loc(Rslt_32), pair_make(LAZ, 0, Rslt_32Args, glblProto16));
#endif
  // call vect-= at core: 644
  Term Rslt_33Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_33 = term_new(VAR, 0, port(2, term_loc(Rslt_33Args)));
  Rslt_33Args = pair_make(APP, 0, y_24_2, Rslt_33Args);
  Rslt_33Args = pair_make(APP, 0, x_23_2, Rslt_33Args);
#ifdef STRICT
  store_redex(Rslt_33Args, glblFn370);
#else
  swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, Rslt_33Args, glblFn370));
#endif

  // call and at core: 643
  Term Rslt_34Args = pair_make(APP, 0, Rslt_33, SUB);
  Term Rslt_34 = term_new(VAR, 0, port(2, term_loc(Rslt_34Args)));
  Rslt_34Args = pair_make(APP, 0, Rslt_32, Rslt_34Args);
  Rslt_34Args = pair_make(APP, 0, new_i60(643), Rslt_34Args);
  Rslt_34Args = pair_make(APP, 0, glblStr149, Rslt_34Args);
#ifdef STRICT
  store_redex(Rslt_34Args, glblProto24);
#else
  swapStore(term_loc(Rslt_34), pair_make(LAZ, 0, Rslt_34Args, glblProto24));
#endif
  // call and at core: 642
  Term Rslt_35Args = pair_make(APP, 0, Rslt_34, SUB);
  Term Rslt_35 = term_new(VAR, 0, port(2, term_loc(Rslt_35Args)));
  Rslt_35Args = pair_make(APP, 0, Rslt_29, Rslt_35Args);
  Rslt_35Args = pair_make(APP, 0, new_i60(642), Rslt_35Args);
  Rslt_35Args = pair_make(APP, 0, glblStr149, Rslt_35Args);
#ifdef STRICT
  store_redex(Rslt_35Args, glblProto24);
#else
  swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, Rslt_35Args, glblProto24));
#endif
  // link args to body
  swapStore(r_seq_25, Rslt_35);

  Term Rslt_35V = get(term_loc(Rslt_35));
  apps = take(port(1, term_loc(Rslt_35V)));
  lams = take(port(2, term_loc(Rslt_35V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_25);
  return;
}
Term glbl_EQ_419 = new_ref(glbl_EQ_418);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str10 = {StringBufferType, REFS_STATIC, 0, 0, 6, "Vector"};
Term glblStr420 = term_new_(VAL, (Term)&str10);
Term glbltype_name417;
void glbltype_name416(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_38 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_38 = port(2, term_loc(seq_38));
  Term arg_37 = term_new(VAR, 0, port(1, term_loc(seq_38)));
  // allocate args at core: 645
  swapStore(term_loc(arg_37), ERA);
  // link args to body
  swapStore(r_seq_38, glblStr420);

  V = get(r_seq_38);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_38, term_new(VAR, 0, vLoc));
  store_redex(args, seq_38);
  return;
}
Term glbltype_name417 = new_ref(glbltype_name416);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str11 = {StringBufferType, REFS_STATIC, 0, 0, 1, "["};
Term glblStr422 = term_new_(VAL, (Term)&str11);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str12 = {StringBufferType, REFS_STATIC, 0, 0, 1, "]"};
Term glblStr421 = term_new_(VAL, (Term)&str12);
Term glblstr_vect415;
void glblstr_vect414(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_42 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_42 = port(2, term_loc(seq_42));
  Term v_41 = term_new(VAR, 0, port(1, term_loc(seq_42)));
  // allocate args at core: 648
  Term glblVal130_7;
  glblVal130 = dupeArg(glblVal130, &glblVal130_7, 0);

  // call vect-conj at core: 649
  Term Rslt_44Args = pair_make(APP, 0, glblStr422, SUB);
  Term Rslt_44 = term_new(VAR, 0, port(2, term_loc(Rslt_44Args)));
  Rslt_44Args = pair_make(APP, 0, glblVal130_7, Rslt_44Args);
#ifdef STRICT
  store_redex(Rslt_44Args, glblFn133);
#else
  swapStore(term_loc(Rslt_44), pair_make(LAZ, 0, Rslt_44Args, glblFn133));
#endif

  // call interpose at core: 649
  Term Rslt_45Args = pair_make(APP, 0, glblStr160, SUB);
  Term Rslt_45 = term_new(VAR, 0, port(2, term_loc(Rslt_45Args)));
  Rslt_45Args = pair_make(APP, 0, v_41, Rslt_45Args);
  Rslt_45Args = pair_make(APP, 0, new_i60(649), Rslt_45Args);
  Rslt_45Args = pair_make(APP, 0, glblStr149, Rslt_45Args);
#ifdef STRICT
  store_redex(Rslt_45Args, glblProto56);
#else
  swapStore(term_loc(Rslt_45), pair_make(LAZ, 0, Rslt_45Args, glblProto56));
#endif
  // call to-str at core: 649
  Term Rslt_46Args = pair_make(APP, 0, Rslt_45, SUB);
  Term Rslt_46 = term_new(VAR, 0, port(2, term_loc(Rslt_46Args)));
#ifdef STRICT
  store_redex(Rslt_46Args, glblFn361);
#else
  swapStore(term_loc(Rslt_46), pair_make(LAZ, 0, Rslt_46Args, glblFn361));
#endif

  // call vect-conj at core: 649
  Term Rslt_47Args = pair_make(APP, 0, Rslt_46, SUB);
  Term Rslt_47 = term_new(VAR, 0, port(2, term_loc(Rslt_47Args)));
  Rslt_47Args = pair_make(APP, 0, Rslt_44, Rslt_47Args);
#ifdef STRICT
  store_redex(Rslt_47Args, glblFn133);
#else
  swapStore(term_loc(Rslt_47), pair_make(LAZ, 0, Rslt_47Args, glblFn133));
#endif

  // call vect-conj at core: 649
  Term Rslt_48Args = pair_make(APP, 0, glblStr421, SUB);
  Term Rslt_48 = term_new(VAR, 0, port(2, term_loc(Rslt_48Args)));
  Rslt_48Args = pair_make(APP, 0, Rslt_47, Rslt_48Args);
#ifdef STRICT
  store_redex(Rslt_48Args, glblFn133);
#else
  swapStore(term_loc(Rslt_48), pair_make(LAZ, 0, Rslt_48Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_42, Rslt_48);

  Term Rslt_48V = get(term_loc(Rslt_48));
  apps = take(port(1, term_loc(Rslt_48V)));
  lams = take(port(2, term_loc(Rslt_48V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_48), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_42);
  return;
}
Term glblstr_vect415 = new_ref(glblstr_vect414);
Term glblcount413;
void glblcount412(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_51 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_51 = port(2, term_loc(seq_51));
  Term l_50 = term_new(VAR, 0, port(1, term_loc(seq_51)));
  // allocate args at core: 651
  // call vect-count at core: 652
  Term Rslt_53Args = pair_make(APP, 0, l_50, SUB);
  Term Rslt_53 = term_new(VAR, 0, port(2, term_loc(Rslt_53Args)));
#ifdef STRICT
  store_redex(Rslt_53Args, glblFn235);
#else
  swapStore(term_loc(Rslt_53), pair_make(LAZ, 0, Rslt_53Args, glblFn235));
#endif

  // link args to body
  swapStore(r_seq_51, Rslt_53);

  Term Rslt_53V = get(term_loc(Rslt_53));
  apps = take(port(1, term_loc(Rslt_53V)));
  lams = take(port(2, term_loc(Rslt_53V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_53), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_51);
  return;
}
Term glblcount413 = new_ref(glblcount412);
Term glblget411;
void glblget410(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_57 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_57 = port(2, term_loc(seq_57));
  Term n_56 = term_new(VAR, 0, port(1, term_loc(seq_57)));
  seq_57 = pair_make(LAM, 0, SUB, seq_57);
  Term v_55 = term_new(VAR, 0, port(1, term_loc(seq_57)));
  // allocate args at core: 654
  // call vect-get at core: 655
  Term Rslt_59Args = pair_make(APP, 0, n_56, SUB);
  Term Rslt_59 = term_new(VAR, 0, port(2, term_loc(Rslt_59Args)));
  Rslt_59Args = pair_make(APP, 0, v_55, Rslt_59Args);
#ifdef STRICT
  store_redex(Rslt_59Args, glblFn328);
#else
  swapStore(term_loc(Rslt_59), pair_make(LAZ, 0, Rslt_59Args, glblFn328));
#endif

  // link args to body
  swapStore(r_seq_57, Rslt_59);

  Term Rslt_59V = get(term_loc(Rslt_59));
  apps = take(port(1, term_loc(Rslt_59V)));
  lams = take(port(2, term_loc(Rslt_59V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_59), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_57);
  return;
}
Term glblget411 = new_ref(glblget410);
Term glblnth409;
void glblnth408(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_63 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_63 = port(2, term_loc(seq_63));
  Term n_62 = term_new(VAR, 0, port(1, term_loc(seq_63)));
  seq_63 = pair_make(LAM, 0, SUB, seq_63);
  Term v_61 = term_new(VAR, 0, port(1, term_loc(seq_63)));
  // allocate args at core: 657
  // call vect-get at core: 658
  Term Rslt_65Args = pair_make(APP, 0, n_62, SUB);
  Term Rslt_65 = term_new(VAR, 0, port(2, term_loc(Rslt_65Args)));
  Rslt_65Args = pair_make(APP, 0, v_61, Rslt_65Args);
#ifdef STRICT
  store_redex(Rslt_65Args, glblFn328);
#else
  swapStore(term_loc(Rslt_65), pair_make(LAZ, 0, Rslt_65Args, glblFn328));
#endif

  // link args to body
  swapStore(r_seq_63, Rslt_65);

  Term Rslt_65V = get(term_loc(Rslt_65));
  apps = take(port(1, term_loc(Rslt_65V)));
  lams = take(port(2, term_loc(Rslt_65V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_65), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_63);
  return;
}
Term glblnth409 = new_ref(glblnth408);
Term glblempty_QM_407;
void glblempty_QM_406(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_68 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_68 = port(2, term_loc(seq_68));
  Term v_67 = term_new(VAR, 0, port(1, term_loc(seq_68)));
  // allocate args at core: 660
  Term glblVal130_8;
  glblVal130 = dupeArg(glblVal130, &glblVal130_8, 0);

  Term glblVal20_3;
  glblVal20 = dupeArg(glblVal20, &glblVal20_3, 0);

  // call count at core: 661
  Term Rslt_70Args = pair_make(APP, 0, v_67, SUB);
  Term Rslt_70 = term_new(VAR, 0, port(2, term_loc(Rslt_70Args)));
  Rslt_70Args = pair_make(APP, 0, new_i60(661), Rslt_70Args);
  Rslt_70Args = pair_make(APP, 0, glblStr149, Rslt_70Args);
#ifdef STRICT
  store_redex(Rslt_70Args, glblProto60);
#else
  swapStore(term_loc(Rslt_70), pair_make(LAZ, 0, Rslt_70Args, glblProto60));
#endif
  // call int-= at core: 661
  Term Rslt_71Args = pair_make(APP, 0, Rslt_70, SUB);
  Term Rslt_71 = term_new(VAR, 0, port(2, term_loc(Rslt_71Args)));
  Rslt_71Args = pair_make(APP, 0, new_i60(0), Rslt_71Args);
#ifdef STRICT
  store_redex(Rslt_71Args, glblFn188);
#else
  swapStore(term_loc(Rslt_71), pair_make(LAZ, 0, Rslt_71Args, glblFn188));
#endif

  // call Some at core: 662
  Term Rslt_72Args = pair_make(APP, 0, glblVal130_8, SUB);
  Term Rslt_72 = term_new(VAR, 0, port(2, term_loc(Rslt_72Args)));
#ifdef STRICT
  store_redex(Rslt_72Args, glblVal20_3);
#else
  swapStore(term_loc(Rslt_72), pair_make(LAZ, 0, Rslt_72Args, glblVal20_3));
#endif

  // call and at core: 661
  Term Rslt_73Args = pair_make(APP, 0, Rslt_72, SUB);
  Term Rslt_73 = term_new(VAR, 0, port(2, term_loc(Rslt_73Args)));
  Rslt_73Args = pair_make(APP, 0, Rslt_71, Rslt_73Args);
  Rslt_73Args = pair_make(APP, 0, new_i60(661), Rslt_73Args);
  Rslt_73Args = pair_make(APP, 0, glblStr149, Rslt_73Args);
#ifdef STRICT
  store_redex(Rslt_73Args, glblProto24);
#else
  swapStore(term_loc(Rslt_73), pair_make(LAZ, 0, Rslt_73Args, glblProto24));
#endif
  // link args to body
  swapStore(r_seq_68, Rslt_73);

  Term Rslt_73V = get(term_loc(Rslt_73));
  apps = take(port(1, term_loc(Rslt_73V)));
  lams = take(port(2, term_loc(Rslt_73V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_73), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_68);
  return;
}
Term glblempty_QM_407 = new_ref(glblempty_QM_406);
Term glblempty405;
void glblempty404(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_76 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_76 = port(2, term_loc(seq_76));
  Term coll_75 = term_new(VAR, 0, port(1, term_loc(seq_76)));
  // allocate args at core: 664
  swapStore(term_loc(coll_75), ERA);
  Term glblVal130_9;
  glblVal130 = dupeArg(glblVal130, &glblVal130_9, 0);

  // link args to body
  swapStore(r_seq_76, glblVal130_9);

  V = get(r_seq_76);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_76, term_new(VAR, 0, vLoc));
  store_redex(args, seq_76);
  return;
}
Term glblempty405 = new_ref(glblempty404);
Term glblcomp403;
void glblcomp402(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_81 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_81 = port(2, term_loc(seq_81));
  Term v2_80 = term_new(VAR, 0, port(1, term_loc(seq_81)));
  seq_81 = pair_make(LAM, 0, SUB, seq_81);
  Term v1_79 = term_new(VAR, 0, port(1, term_loc(seq_81)));
  // allocate args at core: 667
  // call reduce at core: 668
  Term Rslt_83Args = pair_make(APP, 0, glblFn133, SUB);
  Term Rslt_83 = term_new(VAR, 0, port(2, term_loc(Rslt_83Args)));
  Rslt_83Args = pair_make(APP, 0, v1_79, Rslt_83Args);
  Rslt_83Args = pair_make(APP, 0, v2_80, Rslt_83Args);
  Rslt_83Args = pair_make(APP, 0, new_i60(668), Rslt_83Args);
  Rslt_83Args = pair_make(APP, 0, glblStr149, Rslt_83Args);
#ifdef STRICT
  store_redex(Rslt_83Args, glblProto68);
#else
  swapStore(term_loc(Rslt_83), pair_make(LAZ, 0, Rslt_83Args, glblProto68));
#endif
  // link args to body
  swapStore(r_seq_81, Rslt_83);

  Term Rslt_83V = get(term_loc(Rslt_83));
  apps = take(port(1, term_loc(Rslt_83V)));
  lams = take(port(2, term_loc(Rslt_83V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_83), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_81);
  return;
}
Term glblcomp403 = new_ref(glblcomp402);
Term glblfirst401;
void glblfirst400(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_86 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_86 = port(2, term_loc(seq_86));
  Term v_85 = term_new(VAR, 0, port(1, term_loc(seq_86)));
  // allocate args at core: 670
  // call get at core: 671
  Term Rslt_88Args = pair_make(APP, 0, new_i60(0), SUB);
  Term Rslt_88 = term_new(VAR, 0, port(2, term_loc(Rslt_88Args)));
  Rslt_88Args = pair_make(APP, 0, v_85, Rslt_88Args);
  Rslt_88Args = pair_make(APP, 0, new_i60(671), Rslt_88Args);
  Rslt_88Args = pair_make(APP, 0, glblStr149, Rslt_88Args);
#ifdef STRICT
  store_redex(Rslt_88Args, glblProto100);
#else
  swapStore(term_loc(Rslt_88), pair_make(LAZ, 0, Rslt_88Args, glblProto100));
#endif
  // link args to body
  swapStore(r_seq_86, Rslt_88);

  Term Rslt_88V = get(term_loc(Rslt_88));
  apps = take(port(1, term_loc(Rslt_88V)));
  lams = take(port(2, term_loc(Rslt_88V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_88), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_86);
  return;
}
Term glblfirst401 = new_ref(glblfirst400);
Term glbllast399;
void glbllast398(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_91 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_91 = port(2, term_loc(seq_91));
  Term v_90 = term_new(VAR, 0, port(1, term_loc(seq_91)));
  // allocate args at core: 673

  Term v_90_1;
  v_90 = dupeArg(v_90, &v_90_1, 0);
  // call count at core: 674
  Term Rslt_93Args = pair_make(APP, 0, v_90_1, SUB);
  Term Rslt_93 = term_new(VAR, 0, port(2, term_loc(Rslt_93Args)));
  Rslt_93Args = pair_make(APP, 0, new_i60(674), Rslt_93Args);
  Rslt_93Args = pair_make(APP, 0, glblStr149, Rslt_93Args);
#ifdef STRICT
  store_redex(Rslt_93Args, glblProto60);
#else
  swapStore(term_loc(Rslt_93), pair_make(LAZ, 0, Rslt_93Args, glblProto60));
#endif
  // call dec at core: 674
  Term Rslt_94Args = pair_make(APP, 0, Rslt_93, SUB);
  Term Rslt_94 = term_new(VAR, 0, port(2, term_loc(Rslt_94Args)));
#ifdef STRICT
  store_redex(Rslt_94Args, glblFn126);
#else
  swapStore(term_loc(Rslt_94), pair_make(LAZ, 0, Rslt_94Args, glblFn126));
#endif

  // call get at core: 674
  Term Rslt_95Args = pair_make(APP, 0, Rslt_94, SUB);
  Term Rslt_95 = term_new(VAR, 0, port(2, term_loc(Rslt_95Args)));
  Rslt_95Args = pair_make(APP, 0, v_90, Rslt_95Args);
  Rslt_95Args = pair_make(APP, 0, new_i60(674), Rslt_95Args);
  Rslt_95Args = pair_make(APP, 0, glblStr149, Rslt_95Args);
#ifdef STRICT
  store_redex(Rslt_95Args, glblProto100);
#else
  swapStore(term_loc(Rslt_95), pair_make(LAZ, 0, Rslt_95Args, glblProto100));
#endif
  // link args to body
  swapStore(r_seq_91, Rslt_95);

  Term Rslt_95V = get(term_loc(Rslt_95));
  apps = take(port(1, term_loc(Rslt_95V)));
  lams = take(port(2, term_loc(Rslt_95V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_95), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_91);
  return;
}
Term glbllast399 = new_ref(glbllast398);
Term glblrest397;
void glblrest396(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_98 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_98 = port(2, term_loc(seq_98));
  Term v_97 = term_new(VAR, 0, port(1, term_loc(seq_98)));
  // allocate args at core: 676

  Term v_97_1;
  v_97 = dupeArg(v_97, &v_97_1, 0);
  Term glblVal130_10;
  glblVal130 = dupeArg(glblVal130, &glblVal130_10, 0);

  // call count at core: 677
  Term Rslt_100Args = pair_make(APP, 0, v_97_1, SUB);
  Term Rslt_100 = term_new(VAR, 0, port(2, term_loc(Rslt_100Args)));
  Rslt_100Args = pair_make(APP, 0, new_i60(677), Rslt_100Args);
  Rslt_100Args = pair_make(APP, 0, glblStr149, Rslt_100Args);
#ifdef STRICT
  store_redex(Rslt_100Args, glblProto60);
#else
  swapStore(term_loc(Rslt_100), pair_make(LAZ, 0, Rslt_100Args, glblProto60));
#endif
  // call dec at core: 677
  Term Rslt_101Args = pair_make(APP, 0, Rslt_100, SUB);
  Term Rslt_101 = term_new(VAR, 0, port(2, term_loc(Rslt_101Args)));
#ifdef STRICT
  store_redex(Rslt_101Args, glblFn126);
#else
  swapStore(term_loc(Rslt_101), pair_make(LAZ, 0, Rslt_101Args, glblFn126));
#endif

  // call subvec* at core: 677
  Term Rslt_102Args = pair_make(APP, 0, glblVal130_10, SUB);
  Term Rslt_102 = term_new(VAR, 0, port(2, term_loc(Rslt_102Args)));
  Rslt_102Args = pair_make(APP, 0, Rslt_101, Rslt_102Args);
  Rslt_102Args = pair_make(APP, 0, new_i60(1), Rslt_102Args);
  Rslt_102Args = pair_make(APP, 0, v_97, Rslt_102Args);
#ifdef STRICT
  store_redex(Rslt_102Args, glblFn351);
#else
  swapStore(term_loc(Rslt_102), pair_make(LAZ, 0, Rslt_102Args, glblFn351));
#endif

  // link args to body
  swapStore(r_seq_98, Rslt_102);

  Term Rslt_102V = get(term_loc(Rslt_102));
  apps = take(port(1, term_loc(Rslt_102V)));
  lams = take(port(2, term_loc(Rslt_102V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_102), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_98);
  return;
}
Term glblrest397 = new_ref(glblrest396);
Term glblconj395;
void glblconj394(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_106 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_106 = port(2, term_loc(seq_106));
  Term v_105 = term_new(VAR, 0, port(1, term_loc(seq_106)));
  seq_106 = pair_make(LAM, 0, SUB, seq_106);
  Term vect_104 = term_new(VAR, 0, port(1, term_loc(seq_106)));
  // allocate args at core: 679
  // call vect-conj at core: 680
  Term Rslt_108Args = pair_make(APP, 0, v_105, SUB);
  Term Rslt_108 = term_new(VAR, 0, port(2, term_loc(Rslt_108Args)));
  Rslt_108Args = pair_make(APP, 0, vect_104, Rslt_108Args);
#ifdef STRICT
  store_redex(Rslt_108Args, glblFn133);
#else
  swapStore(term_loc(Rslt_108), pair_make(LAZ, 0, Rslt_108Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_106, Rslt_108);

  Term Rslt_108V = get(term_loc(Rslt_108));
  apps = take(port(1, term_loc(Rslt_108V)));
  lams = take(port(2, term_loc(Rslt_108V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_108), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_106);
  return;
}
Term glblconj395 = new_ref(glblconj394);
Term glblreduce393;
void glblreduce392(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_113 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_113 = port(2, term_loc(seq_113));
  Term f_112 = term_new(VAR, 0, port(1, term_loc(seq_113)));
  seq_113 = pair_make(LAM, 0, SUB, seq_113);
  Term result_111 = term_new(VAR, 0, port(1, term_loc(seq_113)));
  seq_113 = pair_make(LAM, 0, SUB, seq_113);
  Term v_110 = term_new(VAR, 0, port(1, term_loc(seq_113)));
  // allocate args at core: 682

  Term v_110_1;
  v_110 = dupeArg(v_110, &v_110_1, 0);
  // call first at core: 683
  Term Rslt_115Args = pair_make(APP, 0, v_110, SUB);
  Term Rslt_115 = term_new(VAR, 0, port(2, term_loc(Rslt_115Args)));
  Rslt_115Args = pair_make(APP, 0, new_i60(683), Rslt_115Args);
  Rslt_115Args = pair_make(APP, 0, glblStr149, Rslt_115Args);
#ifdef STRICT
  store_redex(Rslt_115Args, glblProto72);
#else
  swapStore(term_loc(Rslt_115), pair_make(LAZ, 0, Rslt_115Args, glblProto72));
#endif
  // call vect-reduce at core: 683
  Term Rslt_116Args = pair_make(APP, 0, f_112, SUB);
  Term Rslt_116 = term_new(VAR, 0, port(2, term_loc(Rslt_116Args)));
  Rslt_116Args = pair_make(APP, 0, result_111, Rslt_116Args);
  Rslt_116Args = pair_make(APP, 0, new_i60(1), Rslt_116Args);
  Rslt_116Args = pair_make(APP, 0, v_110_1, Rslt_116Args);
  Rslt_116Args = pair_make(APP, 0, Rslt_115, Rslt_116Args);
  Rslt_116Args = pair_make(APP, 0, new_i60(683), Rslt_116Args);
  Rslt_116Args = pair_make(APP, 0, glblStr149, Rslt_116Args);
#ifdef STRICT
  store_redex(Rslt_116Args, glblProto330);
#else
  swapStore(term_loc(Rslt_116), pair_make(LAZ, 0, Rslt_116Args, glblProto330));
#endif
  // link args to body
  swapStore(r_seq_113, Rslt_116);

  Term Rslt_116V = get(term_loc(Rslt_116));
  apps = take(port(1, term_loc(Rslt_116V)));
  lams = take(port(2, term_loc(Rslt_116V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_116), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_113);
  return;
}
Term glblreduce393 = new_ref(glblreduce392);
Term glblmap391;
void glblmap390(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_120 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_120 = port(2, term_loc(seq_120));
  Term f_119 = term_new(VAR, 0, port(1, term_loc(seq_120)));
  seq_120 = pair_make(LAM, 0, SUB, seq_120);
  Term v_118 = term_new(VAR, 0, port(1, term_loc(seq_120)));
  // allocate args at core: 685
  Term glblVal354_1;
  glblVal354 = dupeArg(glblVal354, &glblVal354_1, 0);

  // call vect-map at core: 687
  Term Rslt_122Args = pair_make(APP, 0, f_119, SUB);
  Term Rslt_122 = term_new(VAR, 0, port(2, term_loc(Rslt_122Args)));
  Rslt_122Args = pair_make(APP, 0, v_118, Rslt_122Args);
#ifdef STRICT
  store_redex(Rslt_122Args, glblVal354_1);
#else
  swapStore(term_loc(Rslt_122), pair_make(LAZ, 0, Rslt_122Args, glblVal354_1));
#endif

  // link args to body
  swapStore(r_seq_120, Rslt_122);

  Term Rslt_122V = get(term_loc(Rslt_122));
  apps = take(port(1, term_loc(Rslt_122V)));
  lams = take(port(2, term_loc(Rslt_122V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_122), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_120);
  return;
}
Term glblmap391 = new_ref(glblmap390);
Term glblwrap389;
void glblwrap388(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_126 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_126 = port(2, term_loc(seq_126));
  Term v_125 = term_new(VAR, 0, port(1, term_loc(seq_126)));
  seq_126 = pair_make(LAM, 0, SUB, seq_126);
  Term arg_124 = term_new(VAR, 0, port(1, term_loc(seq_126)));
  // allocate args at core: 689
  swapStore(term_loc(arg_124), ERA);
  Term glblVal130_11;
  glblVal130 = dupeArg(glblVal130, &glblVal130_11, 0);

  // call vect-conj at core: 690
  Term Rslt_128Args = pair_make(APP, 0, v_125, SUB);
  Term Rslt_128 = term_new(VAR, 0, port(2, term_loc(Rslt_128Args)));
  Rslt_128Args = pair_make(APP, 0, glblVal130_11, Rslt_128Args);
#ifdef STRICT
  store_redex(Rslt_128Args, glblFn133);
#else
  swapStore(term_loc(Rslt_128), pair_make(LAZ, 0, Rslt_128Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_126, Rslt_128);

  Term Rslt_128V = get(term_loc(Rslt_128));
  apps = take(port(1, term_loc(Rslt_128V)));
  lams = take(port(2, term_loc(Rslt_128V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_128), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_126);
  return;
}
Term glblwrap389 = new_ref(glblwrap388);
Term glblflat_map387;
void glblflat_map386(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_132 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_132 = port(2, term_loc(seq_132));
  Term f_131 = term_new(VAR, 0, port(1, term_loc(seq_132)));
  seq_132 = pair_make(LAM, 0, SUB, seq_132);
  Term v_130 = term_new(VAR, 0, port(1, term_loc(seq_132)));
  // allocate args at core: 692
  // call map at core: 693
  Term Rslt_134Args = pair_make(APP, 0, f_131, SUB);
  Term Rslt_134 = term_new(VAR, 0, port(2, term_loc(Rslt_134Args)));
  Rslt_134Args = pair_make(APP, 0, v_130, Rslt_134Args);
  Rslt_134Args = pair_make(APP, 0, new_i60(693), Rslt_134Args);
  Rslt_134Args = pair_make(APP, 0, glblStr149, Rslt_134Args);
#ifdef STRICT
  store_redex(Rslt_134Args, glblProto38);
#else
  swapStore(term_loc(Rslt_134), pair_make(LAZ, 0, Rslt_134Args, glblProto38));
#endif
  // call flatten at core: 693
  Term Rslt_135Args = pair_make(APP, 0, Rslt_134, SUB);
  Term Rslt_135 = term_new(VAR, 0, port(2, term_loc(Rslt_135Args)));
  Rslt_135Args = pair_make(APP, 0, new_i60(693), Rslt_135Args);
  Rslt_135Args = pair_make(APP, 0, glblStr149, Rslt_135Args);
#ifdef STRICT
  store_redex(Rslt_135Args, glblProto44);
#else
  swapStore(term_loc(Rslt_135), pair_make(LAZ, 0, Rslt_135Args, glblProto44));
#endif
  // link args to body
  swapStore(r_seq_132, Rslt_135);

  Term Rslt_135V = get(term_loc(Rslt_135));
  apps = take(port(1, term_loc(Rslt_135V)));
  lams = take(port(2, term_loc(Rslt_135V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_135), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_132);
  return;
}
Term glblflat_map387 = new_ref(glblflat_map386);
void glblProto34_484(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(696), args);
  args = pair_make(APP, 0, glblStr149, args);
  interact(args, glblProto34);
}
Term glblflatten385;
void glblflatten384(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_138 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_138 = port(2, term_loc(seq_138));
  Term v_137 = term_new(VAR, 0, port(1, term_loc(seq_138)));
  // allocate args at core: 695
  Term glblVal130_12;
  glblVal130 = dupeArg(glblVal130, &glblVal130_12, 0);

  // call reduce at core: 696
  Term Rslt_140Args = pair_make(APP, 0, new_ref(glblProto34_484), SUB);
  Term Rslt_140 = term_new(VAR, 0, port(2, term_loc(Rslt_140Args)));
  Rslt_140Args = pair_make(APP, 0, glblVal130_12, Rslt_140Args);
  Rslt_140Args = pair_make(APP, 0, v_137, Rslt_140Args);
  Rslt_140Args = pair_make(APP, 0, new_i60(696), Rslt_140Args);
  Rslt_140Args = pair_make(APP, 0, glblStr149, Rslt_140Args);
#ifdef STRICT
  store_redex(Rslt_140Args, glblProto68);
#else
  swapStore(term_loc(Rslt_140), pair_make(LAZ, 0, Rslt_140Args, glblProto68));
#endif
  // link args to body
  swapStore(r_seq_138, Rslt_140);

  Term Rslt_140V = get(term_loc(Rslt_140));
  apps = take(port(1, term_loc(Rslt_140V)));
  lams = take(port(2, term_loc(Rslt_140V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_140), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_138);
  return;
}
Term glblflatten385 = new_ref(glblflatten384);
Term glblProto269;
Term glblProto445;
Term glblProto443;
void glblCons453(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(46), callArgs);
  interact(callArgs, construct);
  return;
}
void glblEndOfList434(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(0), args);
  callArgs = pair_make(APP, 0, new_i60(47), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblinterpose383;
void glblinterpose382(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_144 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_144 = port(2, term_loc(seq_144));
  Term sep_143 = term_new(VAR, 0, port(1, term_loc(seq_144)));
  seq_144 = pair_make(LAM, 0, SUB, seq_144);
  Term coll_142 = term_new(VAR, 0, port(1, term_loc(seq_144)));
  // allocate args at core: 698
  Term seq_148 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_148 = port(2, term_loc(seq_148));
  Term x_147 = term_new(VAR, 0, port(1, term_loc(seq_148)));
  // allocate args at core: 701
  Term glblVal130_13;
  glblVal130 = dupeArg(glblVal130, &glblVal130_13, 0);

  // call vect-conj at core: 701
  Term Rslt_150Args = pair_make(APP, 0, sep_143, SUB);
  Term Rslt_150 = term_new(VAR, 0, port(2, term_loc(Rslt_150Args)));
  Rslt_150Args = pair_make(APP, 0, glblVal130_13, Rslt_150Args);
#ifdef STRICT
  store_redex(Rslt_150Args, glblFn133);
#else
  swapStore(term_loc(Rslt_150), pair_make(LAZ, 0, Rslt_150Args, glblFn133));
#endif

  // call vect-conj at core: 701
  Term Rslt_151Args = pair_make(APP, 0, x_147, SUB);
  Term Rslt_151 = term_new(VAR, 0, port(2, term_loc(Rslt_151Args)));
  Rslt_151Args = pair_make(APP, 0, Rslt_150, Rslt_151Args);
#ifdef STRICT
  store_redex(Rslt_151Args, glblFn133);
#else
  swapStore(term_loc(Rslt_151), pair_make(LAZ, 0, Rslt_151Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_148, Rslt_151);

  Term Rslt_151V = get(term_loc(Rslt_151));
  apps = take(port(1, term_loc(Rslt_151V)));
  lams = take(port(2, term_loc(Rslt_151V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_151), pair_make(LAZ, 0, apps, lams));
  // call flat-map at core: 701
  Term Rslt_152Args = pair_make(APP, 0, seq_148, SUB);
  Term Rslt_152 = term_new(VAR, 0, port(2, term_loc(Rslt_152Args)));
  Rslt_152Args = pair_make(APP, 0, coll_142, Rslt_152Args);
  Rslt_152Args = pair_make(APP, 0, new_i60(701), Rslt_152Args);
  Rslt_152Args = pair_make(APP, 0, glblStr149, Rslt_152Args);
#ifdef STRICT
  store_redex(Rslt_152Args, glblProto42);
#else
  swapStore(term_loc(Rslt_152), pair_make(LAZ, 0, Rslt_152Args, glblProto42));
#endif
  // call rest at core: 702
  Term Rslt_153Args = pair_make(APP, 0, Rslt_152, SUB);
  Term Rslt_153 = term_new(VAR, 0, port(2, term_loc(Rslt_153Args)));
  Rslt_153Args = pair_make(APP, 0, new_i60(702), Rslt_153Args);
  Rslt_153Args = pair_make(APP, 0, glblStr149, Rslt_153Args);
#ifdef STRICT
  store_redex(Rslt_153Args, glblProto74);
#else
  swapStore(term_loc(Rslt_153), pair_make(LAZ, 0, Rslt_153Args, glblProto74));
#endif
  // link args to body
  swapStore(r_seq_144, Rslt_153);

  Term Rslt_153V = get(term_loc(Rslt_153));
  apps = take(port(1, term_loc(Rslt_153V)));
  lams = take(port(2, term_loc(Rslt_153V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_153), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_144);
  return;
}
Term glblinterpose383 = new_ref(glblinterpose382);
Term glblVal437 = new_ref(glblEndOfList434);
Term glblVal437;
Term glblRslt436;
Term glblrecurse430;
void glblrecurse429(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term l_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 712
  swapStore(term_loc(f_2), ERA);
  // link args to body
  swapStore(r_seq_3, l_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse430 = new_ref(glblrecurse429);
Term glblcount433;
void glblcount431(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term l_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at core: 718
  swapStore(term_loc(l_6), ERA);
  // link args to body
  swapStore(r_seq_7, new_i60(0));

  V = get(r_seq_7);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_7, term_new(VAR, 0, vLoc));
  store_redex(args, seq_7);
  return;
}
Term glblcount433 = new_ref(glblcount431);
Term glblVal460 = new_ref(glblCons453);
Term glblVal460;
Term glblrecurse449;
void glblrecurse448(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term arg_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 725

  Term arg_1_2;
  arg_1 = dupeArg(arg_1, &arg_1_2, 0);

  Term arg_1_1;
  arg_1 = dupeArg(arg_1, &arg_1_1, 0);
  Term arg_1_2Args = pair_make(APP, 0, arg_1_2, SUB);
  Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(arg_1_2Args)));
  arg_1_2Args = pair_make(APP, 0, new_i60(2), arg_1_2Args);
  store_redex(arg_1_2Args, accessField);
  Term arg_1_1Args = pair_make(APP, 0, arg_1_1, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(arg_1_1Args)));
  arg_1_1Args = pair_make(APP, 0, new_i60(1), arg_1_1Args);
  store_redex(arg_1_1Args, accessField);
  Term arg_1Args = pair_make(APP, 0, arg_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(arg_1Args)));
  arg_1Args = pair_make(APP, 0, new_i60(0), arg_1Args);
  store_redex(arg_1Args, accessField);
  Term glblVal460_1;
  glblVal460 = dupeArg(glblVal460, &glblVal460_1, 0);

  // call f at core: 725
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, f_2);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, f_2));
#endif

  // call Cons at core: 726
  Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, Rslt_7, Rslt_9Args);
  Rslt_9Args = pair_make(APP, 0, Rslt_5, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblVal460_1);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblVal460_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_9);

  Term Rslt_9V = get(term_loc(Rslt_9));
  apps = take(port(1, term_loc(Rslt_9V)));
  lams = take(port(2, term_loc(Rslt_9V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse449 = new_ref(glblrecurse448);
Term glblcount452;
void glblcount451(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_12 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_12 = port(2, term_loc(seq_12));
  Term l_11 = term_new(VAR, 0, port(1, term_loc(seq_12)));
  // allocate args at core: 731
  // call .len at core: 732
  Term Rslt_14Args = pair_make(APP, 0, l_11, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, new_i60(732), Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, glblStr149, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblProto269);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblProto269));
#endif
  // link args to body
  swapStore(r_seq_12, Rslt_14);

  Term Rslt_14V = get(term_loc(Rslt_14));
  apps = take(port(1, term_loc(Rslt_14V)));
  lams = take(port(2, term_loc(Rslt_14V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_12);
  return;
}
Term glblcount452 = new_ref(glblcount451);
Term glblFld455;
void glblFldFn495(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld455 = new_ref(glblFldFn495);
Term glblFld456;
void glblFldFn496(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld456 = new_ref(glblFldFn496);
Term glblFld457;
void glblFldFn497(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld457 = new_ref(glblFldFn497);
Term glblFn425;
void glblCFn424(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term l_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at core: 734

  Term l_2_1;
  l_2 = dupeArg(l_2, &l_2_1, 0);
  Term glblVal460_2;
  glblVal460 = dupeArg(glblVal460, &glblVal460_2, 0);

  // call count at core: 736
  Term Rslt_5Args = pair_make(APP, 0, l_2_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
  Rslt_5Args = pair_make(APP, 0, new_i60(736), Rslt_5Args);
  Rslt_5Args = pair_make(APP, 0, glblStr149, Rslt_5Args);
#ifdef STRICT
  store_redex(Rslt_5Args, glblProto60);
#else
  swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblProto60));
#endif
  // call inc at core: 736
  Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
  store_redex(Rslt_6Args, glblFn119);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn119));
#endif

  // call Cons at core: 736
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
  Rslt_7Args = pair_make(APP, 0, l_2, Rslt_7Args);
  Rslt_7Args = pair_make(APP, 0, v_1, Rslt_7Args);
#ifdef STRICT
  store_redex(Rslt_7Args, glblVal460_2);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal460_2));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblFn425 = new_ref(glblCFn424);
Term glblFn427;
void glblCFn426(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term vs_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at core: 738
  // call to-str at core: 742
  Term Rslt_4Args = pair_make(APP, 0, vs_1, SUB);
  Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
  store_redex(Rslt_4Args, glblFn361);
#else
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn361));
#endif

  // link args to body
  swapStore(r_seq_2, Rslt_4);

  Term Rslt_4V = get(term_loc(Rslt_4));
  apps = take(port(1, term_loc(Rslt_4V)));
  lams = take(port(2, term_loc(Rslt_4V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_2);
  return;
}
Term glblFn427 = new_ref(glblCFn426);
Value *(*type_name)(FnArity *, Value *) = noImpl1;
Value *(*zero)(FnArity *, Value *) = noImpl1;
Value *(*invoke0Args)(FnArity *, Value *) = noImpl1;
Value *(*invoke1Arg)(FnArity *, Value *, Value *) = noImpl2;
Value *(*invoke2Args)(FnArity *, Value *, Value *, Value *) = noImpl3;
Value *(*equalSTAR)(FnArity *, Value *, Value *) = noImpl2;
Value *(*hashSeq)(FnArity *, Value *, Value *) = noImpl2;
Value *(*count)(FnArity *, Value *) = noImpl1;
Value *(*vals)(FnArity *, Value *) = noImpl1;
Value *(*first)(FnArity *, Value *) = noImpl1;
Value *(*rest)(FnArity *, Value *) = noImpl1;
Value *(*seq)(FnArity *, Value *) = noImpl1;
Value *(*sha1)(FnArity *, Value *) = noImpl1;
Value *(*hasField)(FnArity *, Value *, Value *) = noImpl2;
Value *(*showFn)(FnArity *, Value *) = noImpl1;
Value *(*newHashSet)(FnArity *, Value *) = noImpl1;
Value *(*fn_apply)(FnArity *, Value *, Value *) = noImpl2;
Value *(*prValue)(FnArity *, Value *) = noImpl1;
Term glblProto503;
Term glblProto501;
void glblLocation505(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(49), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal508 = new_ref(glblLocation505);
Term glblVal508;
Term glblFld506;
void glblFldFn509(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld506 = new_ref(glblFldFn509);
Term glblFld507;
void glblFldFn510(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld507 = new_ref(glblFldFn510);
Term glblProto511;
void glblModule513(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(50), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal515 = new_ref(glblModule513);
Term glblVal515;
Term glblFld514;
void glblFldFn516(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld514 = new_ref(glblFldFn516);
Term glblProto521;
Term glblProto519;
Term glblProto517;
void glblConstructor523(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(55), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal527 = new_ref(glblConstructor523);
Term glblVal527;
Term glblFld524;
void glblFldFn528(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld524 = new_ref(glblFldFn528);
Term glblFld525;
void glblFldFn529(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld525 = new_ref(glblFldFn529);
Term glblFld526;
void glblFldFn530(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld526 = new_ref(glblFldFn530);
Term glblProto602;
void glblBlockComment605(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(56), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto594;
Term glblProto592;
Term glblProto590;
void glblMatch596(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(57), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto533;
Term glblProto584;
void glblFieldGetter586(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(58), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto578;
Term glblProto576;
void glblTypeConstraint580(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(59), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto571;
void glblSuperposition573(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(60), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto564;
Term glblProto562;
Term glblProto517;
void glblFn566(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(61), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto556;
Term glblProto554;
void glblCall558(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(62), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto533;
Term glblProto540;
void glblStringLit550(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(63), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto533;
Term glblProto540;
void glblFloatLit546(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(64), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto533;
Term glblProto540;
void glblIntegerLit542(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(65), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto533;
Term glblProto517;
Term glblProto531;
void glblSymbol535(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(66), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal539 = new_ref(glblSymbol535);
Term glblVal539;
Term glblFld536;
void glblFldFn609(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld536 = new_ref(glblFldFn609);
Term glblFld537;
void glblFldFn610(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld537 = new_ref(glblFldFn610);
Term glblFld538;
void glblFldFn611(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld538 = new_ref(glblFldFn611);
Term glblVal545 = new_ref(glblIntegerLit542);
Term glblVal545;
Term glblFld543;
void glblFldFn612(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld543 = new_ref(glblFldFn612);
Term glblFld544;
void glblFldFn613(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld544 = new_ref(glblFldFn613);
Term glblVal549 = new_ref(glblFloatLit546);
Term glblVal549;
Term glblFld547;
void glblFldFn614(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld547 = new_ref(glblFldFn614);
Term glblFld548;
void glblFldFn615(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld548 = new_ref(glblFldFn615);
Term glblVal553 = new_ref(glblStringLit550);
Term glblVal553;
Term glblFld551;
void glblFldFn616(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld551 = new_ref(glblFldFn616);
Term glblFld552;
void glblFldFn617(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld552 = new_ref(glblFldFn617);
Term glblVal561 = new_ref(glblCall558);
Term glblVal561;
Term glblFld559;
void glblFldFn618(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld559 = new_ref(glblFldFn618);
Term glblFld560;
void glblFldFn619(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld560 = new_ref(glblFldFn619);
Term glblVal570 = new_ref(glblFn566);
Term glblVal570;
Term glblFld567;
void glblFldFn620(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld567 = new_ref(glblFldFn620);
Term glblFld568;
void glblFldFn621(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld568 = new_ref(glblFldFn621);
Term glblFld569;
void glblFldFn622(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld569 = new_ref(glblFldFn622);
Term glblVal575 = new_ref(glblSuperposition573);
Term glblVal575;
Term glblFld574;
void glblFldFn623(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld574 = new_ref(glblFldFn623);
Term glblVal583 = new_ref(glblTypeConstraint580);
Term glblVal583;
Term glblFld581;
void glblFldFn626(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld581 = new_ref(glblFldFn626);
Term glblFld582;
void glblFldFn627(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld582 = new_ref(glblFldFn627);
Term glblVal589 = new_ref(glblFieldGetter586);
Term glblVal589;
Term glblFld587;
void glblFldFn628(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld587 = new_ref(glblFldFn628);
Term glblFld588;
void glblFldFn629(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld588 = new_ref(glblFldFn629);
Term glblVal601 = new_ref(glblMatch596);
Term glblVal601;
Term glblFld597;
void glblFldFn630(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld597 = new_ref(glblFldFn630);
Term glblFld598;
void glblFldFn631(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld598 = new_ref(glblFldFn631);
Term glblFld599;
void glblFldFn632(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld599 = new_ref(glblFldFn632);
Term glblVal607 = new_ref(glblBlockComment605);
Term glblVal607;
Term glblFld606;
void glblFldFn633(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld606 = new_ref(glblFldFn633);
Term glblProto676;
void glblInline678(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(67), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto667;
Term glblProto665;
void glblAddNs670(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(68), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto656;
Term glblProto639;
void glblExtendType659(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(69), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto564;
Term glblProto562;
Term glblProto517;
void glblDefp650(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(3), args);
  callArgs = pair_make(APP, 0, new_i60(70), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto641;
Term glblProto639;
void glblDefType644(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(71), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto624;
Term glblProto517;
void glblDefinition634(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(72), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal637 = new_ref(glblDefinition634);
Term glblVal637;
Term glblFld635;
void glblFldFn682(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld635 = new_ref(glblFldFn682);
Term glblFld636;
void glblFldFn683(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld636 = new_ref(glblFldFn683);
Term glblVal647 = new_ref(glblDefType644);
Term glblVal647;
Term glblFld645;
void glblFldFn686(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld645 = new_ref(glblFldFn686);
Term glblFld646;
void glblFldFn687(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld646 = new_ref(glblFldFn687);
Term glblVal654 = new_ref(glblDefp650);
Term glblVal654;
Term glblFld651;
void glblFldFn690(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld651 = new_ref(glblFldFn690);
Term glblFld652;
void glblFldFn691(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld652 = new_ref(glblFldFn691);
Term glblFld653;
void glblFldFn694(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(2), args);
  store_redex(args, accessField);
  return;
}
Term glblFld653 = new_ref(glblFldFn694);
Term glblVal663 = new_ref(glblExtendType659);
Term glblVal663;
Term glblFld660;
void glblFldFn697(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld660 = new_ref(glblFldFn697);
Term glblFld661;
void glblFldFn698(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld661 = new_ref(glblFldFn698);
Term glblVal674 = new_ref(glblAddNs670);
Term glblVal674;
Term glblFld671;
void glblFldFn700(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld671 = new_ref(glblFldFn700);
Term glblFld673;
void glblFldFn701(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld673 = new_ref(glblFldFn701);
Term glblVal681 = new_ref(glblInline678);
Term glblVal681;
Term glblFld679;
void glblFldFn702(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld679 = new_ref(glblFldFn702);
Term glblProto688;
Term glblProto684;
void glblParserState692(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(48), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal699 = new_ref(glblParserState692);
Term glblVal699;
Term glblFld693;
void glblFldFn703(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld693 = new_ref(glblFldFn703);
Term glblFld695;
void glblFldFn704(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld695 = new_ref(glblFldFn704);
Term glblProto709;
Term glblProto719;
void glblParserError721(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(51), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto709;
void glblParserIgnore715(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(52), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto709;
Term glblProto540;
void glblParserMatch711(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(53), callArgs);
  interact(callArgs, construct);
  return;
}
void glblParserFail705(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(0), args);
  callArgs = pair_make(APP, 0, new_i60(54), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal707 = new_ref(glblParserFail705);
Term glblVal707;
Term glblRslt706;
Term glblVal714 = new_ref(glblParserMatch711);
Term glblVal714;
Term glblFld712;
void glblFldFn725(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld712 = new_ref(glblFldFn725);
Term glblFld713;
void glblFldFn726(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld713 = new_ref(glblFldFn726);
Term glblVal717 = new_ref(glblParserIgnore715);
Term glblVal717;
Term glblFld716;
void glblFldFn728(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld716 = new_ref(glblFldFn728);
Term glblVal724 = new_ref(glblParserError721);
Term glblVal724;
Term glblFld722;
void glblFldFn729(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld722 = new_ref(glblFldFn729);
Term glblFld723;
void glblFldFn730(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld723 = new_ref(glblFldFn730);
Term glblProto853;
void glblRecur859(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(73), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto731;
Term glblProto517;
void glblRule845(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(74), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto731;
void glblMany830(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(75), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto804;
void glblAny820(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(76), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto804;
void glblAll810(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(77), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto719;
void glblError797(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(78), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto780;
void glblNotChar787(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(79), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto540;
void glblAlwaysSucceed774(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(80), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto752;
Term glblProto750;
void glblCharRange759(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(2), args);
  callArgs = pair_make(APP, 0, new_i60(81), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblProto731;
void glblIgnore738(Term ref, Term args) {
  Term callArgs = pair_make(APP, 0, new_i60(1), args);
  callArgs = pair_make(APP, 0, new_i60(82), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal740 = new_ref(glblIgnore738);
Term glblVal740;
Term glblrecurse734;
void glblrecurse733(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 17
  Term v_1Args = pair_make(APP, 0, v_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(v_1Args)));
  v_1Args = pair_make(APP, 0, new_i60(0), v_1Args);
  store_redex(v_1Args, accessField);
  Term glblVal740_1;
  glblVal740 = dupeArg(glblVal740, &glblVal740_1, 0);

  // call f at intrp-rdr.toc: 17
  Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
  store_redex(Rslt_6Args, f_2);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, f_2));
#endif

  // call Ignore at intrp-rdr.toc: 18
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblVal740_1);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal740_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse734 = new_ref(glblrecurse733);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[9];
} str13 = {StringBufferType, REFS_STATIC, 0, 0, 8, "(Ignore "};
Term glblStr746 = term_new_(VAL, (Term)&str13);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[14];
} str14 = {StringBufferType, REFS_STATIC, 0, 0, 13, "intrp-rdr.toc"};
Term glblStr869 = term_new_(VAL, (Term)&str14);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str15 = {StringBufferType, REFS_STATIC, 0, 0, 1, ")"};
Term glblStr745 = term_new_(VAL, (Term)&str15);
Term glblstr_vect737;
void glblstr_vect735(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term x_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at intrp-rdr.toc: 19
  Term glblVal130_15;
  glblVal130 = dupeArg(glblVal130, &glblVal130_15, 0);

  Term glblVal130_14;
  glblVal130 = dupeArg(glblVal130, &glblVal130_14, 0);

  // call vect-conj at intrp-rdr.toc: 20
  Term Rslt_12Args = pair_make(APP, 0, glblStr746, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, glblVal130_14, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // call .parser at intrp-rdr.toc: 20
  Term Rslt_13Args = pair_make(APP, 0, x_9, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, new_i60(20), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr869, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto731);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto731));
#endif
  // call vect-conj at intrp-rdr.toc: 20
  Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, glblVal130_15, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblFn133);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn133));
#endif

  // call str* at intrp-rdr.toc: 20
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn427);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn427));
#endif

  // call vect-conj at intrp-rdr.toc: 20
  Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_12, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn133);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 20
  Term Rslt_17Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_16, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_10, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_10);
  return;
}
Term glblstr_vect737 = new_ref(glblstr_vect735);
Term glblFld739;
void glblFldFn871(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld739 = new_ref(glblFldFn871);
Term glblVal762 = new_ref(glblCharRange759);
Term glblVal762;
Term glblrecurse755;
void glblrecurse754(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 24
  swapStore(term_loc(f_2), ERA);
  // link args to body
  swapStore(r_seq_3, v_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse755 = new_ref(glblrecurse754);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str16 = {StringBufferType, REFS_STATIC, 0, 0, 11, "(CharRange "};
Term glblStr766 = term_new_(VAL, (Term)&str16);
Term glblstr_vect757;
void glblstr_vect756(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at intrp-rdr.toc: 26

  Term x_6_1;
  x_6 = dupeArg(x_6, &x_6_1, 0);
  Term glblVal130_16;
  glblVal130 = dupeArg(glblVal130, &glblVal130_16, 0);

  // call vect-conj at intrp-rdr.toc: 27
  Term Rslt_9Args = pair_make(APP, 0, glblStr766, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, glblVal130_16, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn133);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

  // call .lower at intrp-rdr.toc: 27
  Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, new_i60(27), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr869, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto750);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto750));
#endif
  // call vect-conj at intrp-rdr.toc: 27
  Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_9, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn133);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 27
  Term Rslt_12Args = pair_make(APP, 0, glblStr160, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_11, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // call .upper at intrp-rdr.toc: 27
  Term Rslt_13Args = pair_make(APP, 0, x_6_1, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, new_i60(27), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr869, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto752);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto752));
#endif
  // call vect-conj at intrp-rdr.toc: 27
  Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, Rslt_12, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblFn133);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 27
  Term Rslt_15Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
  Rslt_15Args = pair_make(APP, 0, Rslt_14, Rslt_15Args);
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn133);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_15);

  Term Rslt_15V = get(term_loc(Rslt_15));
  apps = take(port(1, term_loc(Rslt_15V)));
  lams = take(port(2, term_loc(Rslt_15V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_7);
  return;
}
Term glblstr_vect757 = new_ref(glblstr_vect756);
Term glblFld760;
void glblFldFn880(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld760 = new_ref(glblFldFn880);
Term glblFld761;
void glblFldFn881(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld761 = new_ref(glblFldFn881);
Term glblVal776 = new_ref(glblAlwaysSucceed774);
Term glblVal776;
Term glblrecurse771;
void glblrecurse770(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 29
  swapStore(term_loc(f_2), ERA);
  // link args to body
  swapStore(r_seq_3, v_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse771 = new_ref(glblrecurse770);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[15];
} str17 = {StringBufferType, REFS_STATIC, 0, 0, 14, "(AlwaysSucceed"};
Term glblStr779 = term_new_(VAL, (Term)&str17);
Term glblstr_vect773;
void glblstr_vect772(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at intrp-rdr.toc: 31
  Term glblVal130_17;
  glblVal130 = dupeArg(glblVal130, &glblVal130_17, 0);

  // call vect-conj at intrp-rdr.toc: 32
  Term Rslt_9Args = pair_make(APP, 0, glblStr779, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, glblVal130_17, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn133);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

  // call .value at intrp-rdr.toc: 32
  Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, new_i60(32), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr869, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto540);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto540));
#endif
  // call vect-conj at intrp-rdr.toc: 32
  Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_9, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn133);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 32
  Term Rslt_12Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_11, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_12);

  Term Rslt_12V = get(term_loc(Rslt_12));
  apps = take(port(1, term_loc(Rslt_12V)));
  lams = take(port(2, term_loc(Rslt_12V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_7);
  return;
}
Term glblstr_vect773 = new_ref(glblstr_vect772);
Term glblFld775;
void glblFldFn885(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld775 = new_ref(glblFldFn885);
Term glblVal789 = new_ref(glblNotChar787);
Term glblVal789;
Term glblrecurse784;
void glblrecurse783(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 35
  swapStore(term_loc(f_2), ERA);
  // link args to body
  swapStore(r_seq_3, v_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse784 = new_ref(glblrecurse783);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[10];
} str18 = {StringBufferType, REFS_STATIC, 0, 0, 9, "(NotChar "};
Term glblStr792 = term_new_(VAL, (Term)&str18);
Term glblstr_vect786;
void glblstr_vect785(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at intrp-rdr.toc: 37
  Term glblVal130_18;
  glblVal130 = dupeArg(glblVal130, &glblVal130_18, 0);

  // call vect-conj at intrp-rdr.toc: 38
  Term Rslt_9Args = pair_make(APP, 0, glblStr792, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, glblVal130_18, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn133);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

  // call .char at intrp-rdr.toc: 38
  Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, new_i60(38), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr869, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto780);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto780));
#endif
  // call vect-conj at intrp-rdr.toc: 38
  Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_9, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn133);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 38
  Term Rslt_12Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_11, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_12);

  Term Rslt_12V = get(term_loc(Rslt_12));
  apps = take(port(1, term_loc(Rslt_12V)));
  lams = take(port(2, term_loc(Rslt_12V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_7);
  return;
}
Term glblstr_vect786 = new_ref(glblstr_vect785);
Term glblFld788;
void glblFldFn891(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld788 = new_ref(glblFldFn891);
Term glblVal799 = new_ref(glblError797);
Term glblVal799;
Term glblrecurse794;
void glblrecurse793(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 41
  swapStore(term_loc(f_2), ERA);
  // link args to body
  swapStore(r_seq_3, v_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse794 = new_ref(glblrecurse793);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str19 = {StringBufferType, REFS_STATIC, 0, 0, 7, "(Error "};
Term glblStr802 = term_new_(VAL, (Term)&str19);
Term glblstr_vect796;
void glblstr_vect795(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at intrp-rdr.toc: 43
  Term glblVal130_19;
  glblVal130 = dupeArg(glblVal130, &glblVal130_19, 0);

  // call vect-conj at intrp-rdr.toc: 44
  Term Rslt_9Args = pair_make(APP, 0, glblStr802, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, glblVal130_19, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn133);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

  // call .msg at intrp-rdr.toc: 44
  Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
  Rslt_10Args = pair_make(APP, 0, new_i60(44), Rslt_10Args);
  Rslt_10Args = pair_make(APP, 0, glblStr869, Rslt_10Args);
#ifdef STRICT
  store_redex(Rslt_10Args, glblProto719);
#else
  swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblProto719));
#endif
  // call vect-conj at intrp-rdr.toc: 44
  Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
  Rslt_11Args = pair_make(APP, 0, Rslt_9, Rslt_11Args);
#ifdef STRICT
  store_redex(Rslt_11Args, glblFn133);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 44
  Term Rslt_12Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, Rslt_11, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_12);

  Term Rslt_12V = get(term_loc(Rslt_12));
  apps = take(port(1, term_loc(Rslt_12V)));
  lams = take(port(2, term_loc(Rslt_12V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_7);
  return;
}
Term glblstr_vect796 = new_ref(glblstr_vect795);
Term glblFld798;
void glblFldFn896(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld798 = new_ref(glblFldFn896);
Term glblVal812 = new_ref(glblAll810);
Term glblVal812;
Term glblrecurse807;
void glblrecurse806(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 47
  Term v_1Args = pair_make(APP, 0, v_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(v_1Args)));
  v_1Args = pair_make(APP, 0, new_i60(0), v_1Args);
  store_redex(v_1Args, accessField);
  Term glblVal812_1;
  glblVal812 = dupeArg(glblVal812, &glblVal812_1, 0);

  // call map at intrp-rdr.toc: 48
  Term Rslt_6Args = pair_make(APP, 0, f_2, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
  Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, new_i60(48), Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, glblStr869, Rslt_6Args);
#ifdef STRICT
  store_redex(Rslt_6Args, glblProto38);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblProto38));
#endif
  // call All at intrp-rdr.toc: 48
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblVal812_1);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal812_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse807 = new_ref(glblrecurse806);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[6];
} str20 = {StringBufferType, REFS_STATIC, 0, 0, 5, "(All "};
Term glblStr815 = term_new_(VAL, (Term)&str20);
void glblProto106_903(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(50), args);
  args = pair_make(APP, 0, glblStr869, args);
  interact(args, glblProto106);
}
Term glblstr_vect809;
void glblstr_vect808(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term x_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at intrp-rdr.toc: 49
  Term glblVal130_20;
  glblVal130 = dupeArg(glblVal130, &glblVal130_20, 0);

  // call vect-conj at intrp-rdr.toc: 50
  Term Rslt_12Args = pair_make(APP, 0, glblStr815, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, glblVal130_20, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // call .parsers at intrp-rdr.toc: 50
  Term Rslt_13Args = pair_make(APP, 0, x_9, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, new_i60(50), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr869, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto804);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto804));
#endif
  // call flat-map at intrp-rdr.toc: 50
  Term Rslt_14Args = pair_make(APP, 0, new_ref(glblProto106_903), SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, Rslt_13, Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, new_i60(50), Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, glblStr869, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblProto42);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblProto42));
#endif
  // call to-str at intrp-rdr.toc: 50
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn361);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn361));
#endif

  // call vect-conj at intrp-rdr.toc: 50
  Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_12, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn133);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 50
  Term Rslt_17Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_16, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_10, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_10);
  return;
}
Term glblstr_vect809 = new_ref(glblstr_vect808);
Term glblFld811;
void glblFldFn906(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld811 = new_ref(glblFldFn906);
Term glblVal822 = new_ref(glblAny820);
Term glblVal822;
Term glblrecurse817;
void glblrecurse816(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 53
  Term v_1Args = pair_make(APP, 0, v_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(v_1Args)));
  v_1Args = pair_make(APP, 0, new_i60(0), v_1Args);
  store_redex(v_1Args, accessField);
  Term glblVal822_1;
  glblVal822 = dupeArg(glblVal822, &glblVal822_1, 0);

  // call map at intrp-rdr.toc: 54
  Term Rslt_6Args = pair_make(APP, 0, f_2, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
  Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, new_i60(54), Rslt_6Args);
  Rslt_6Args = pair_make(APP, 0, glblStr869, Rslt_6Args);
#ifdef STRICT
  store_redex(Rslt_6Args, glblProto38);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblProto38));
#endif
  // call Any at intrp-rdr.toc: 54
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblVal822_1);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal822_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse817 = new_ref(glblrecurse816);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[6];
} str21 = {StringBufferType, REFS_STATIC, 0, 0, 5, "(Any "};
Term glblStr825 = term_new_(VAL, (Term)&str21);
void glblProto106_912(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(56), args);
  args = pair_make(APP, 0, glblStr869, args);
  interact(args, glblProto106);
}
Term glblstr_vect819;
void glblstr_vect818(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term x_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at intrp-rdr.toc: 55
  Term glblVal130_21;
  glblVal130 = dupeArg(glblVal130, &glblVal130_21, 0);

  // call vect-conj at intrp-rdr.toc: 56
  Term Rslt_12Args = pair_make(APP, 0, glblStr825, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, glblVal130_21, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // call .parsers at intrp-rdr.toc: 56
  Term Rslt_13Args = pair_make(APP, 0, x_9, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, new_i60(56), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr869, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto804);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto804));
#endif
  // call flat-map at intrp-rdr.toc: 56
  Term Rslt_14Args = pair_make(APP, 0, new_ref(glblProto106_912), SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, Rslt_13, Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, new_i60(56), Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, glblStr869, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblProto42);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblProto42));
#endif
  // call to-str at intrp-rdr.toc: 56
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn361);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn361));
#endif

  // call vect-conj at intrp-rdr.toc: 56
  Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_12, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn133);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 56
  Term Rslt_17Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_16, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_10, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_10);
  return;
}
Term glblstr_vect819 = new_ref(glblstr_vect818);
Term glblFld821;
void glblFldFn915(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld821 = new_ref(glblFldFn915);
Term glblVal832 = new_ref(glblMany830);
Term glblVal832;
Term glblrecurse827;
void glblrecurse826(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 59
  Term v_1Args = pair_make(APP, 0, v_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(v_1Args)));
  v_1Args = pair_make(APP, 0, new_i60(0), v_1Args);
  store_redex(v_1Args, accessField);
  Term glblVal832_1;
  glblVal832 = dupeArg(glblVal832, &glblVal832_1, 0);

  // call f at intrp-rdr.toc: 59
  Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
  store_redex(Rslt_6Args, f_2);
#else
  swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, f_2));
#endif

  // call Many at intrp-rdr.toc: 60
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, glblVal832_1);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal832_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_7);

  Term Rslt_7V = get(term_loc(Rslt_7));
  apps = take(port(1, term_loc(Rslt_7V)));
  lams = take(port(2, term_loc(Rslt_7V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse827 = new_ref(glblrecurse826);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str22 = {StringBufferType, REFS_STATIC, 0, 0, 6, "(Many "};
Term glblStr837 = term_new_(VAL, (Term)&str22);
Term glblstr_vect829;
void glblstr_vect828(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_10 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_10 = port(2, term_loc(seq_10));
  Term x_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
  // allocate args at intrp-rdr.toc: 61
  Term glblVal130_22;
  glblVal130 = dupeArg(glblVal130, &glblVal130_22, 0);

  // call vect-conj at intrp-rdr.toc: 62
  Term Rslt_12Args = pair_make(APP, 0, glblStr837, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, glblVal130_22, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblFn133);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

  // call .parser at intrp-rdr.toc: 62
  Term Rslt_13Args = pair_make(APP, 0, x_9, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, new_i60(62), Rslt_13Args);
  Rslt_13Args = pair_make(APP, 0, glblStr869, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblProto731);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblProto731));
#endif
  // call str-vect at intrp-rdr.toc: 62
  Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, new_i60(62), Rslt_14Args);
  Rslt_14Args = pair_make(APP, 0, glblStr869, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblProto106);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblProto106));
#endif
  // call first at intrp-rdr.toc: 62
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
  Rslt_15Args = pair_make(APP, 0, new_i60(62), Rslt_15Args);
  Rslt_15Args = pair_make(APP, 0, glblStr869, Rslt_15Args);
#ifdef STRICT
  store_redex(Rslt_15Args, glblProto72);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblProto72));
#endif
  // call vect-conj at intrp-rdr.toc: 62
  Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_12, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn133);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 62
  Term Rslt_17Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
  Rslt_17Args = pair_make(APP, 0, Rslt_16, Rslt_17Args);
#ifdef STRICT
  store_redex(Rslt_17Args, glblFn133);
#else
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_10, Rslt_17);

  Term Rslt_17V = get(term_loc(Rslt_17));
  apps = take(port(1, term_loc(Rslt_17V)));
  lams = take(port(2, term_loc(Rslt_17V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_10);
  return;
}
Term glblstr_vect829 = new_ref(glblstr_vect828);
Term glblFld831;
void glblFldFn921(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld831 = new_ref(glblFldFn921);
Term glblVal848 = new_ref(glblRule845);
Term glblVal848;
Term glblrecurse842;
void glblrecurse841(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 66

  Term v_1_1;
  v_1 = dupeArg(v_1, &v_1_1, 0);
  Term v_1_1Args = pair_make(APP, 0, v_1_1, SUB);
  Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(v_1_1Args)));
  v_1_1Args = pair_make(APP, 0, new_i60(1), v_1_1Args);
  store_redex(v_1_1Args, accessField);
  Term v_1Args = pair_make(APP, 0, v_1, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(v_1Args)));
  v_1Args = pair_make(APP, 0, new_i60(0), v_1Args);
  store_redex(v_1Args, accessField);
  Term glblVal848_1;
  glblVal848 = dupeArg(glblVal848, &glblVal848_1, 0);

  // call f at intrp-rdr.toc: 66
  Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
  store_redex(Rslt_7Args, f_2);
#else
  swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, f_2));
#endif

  // call Rule at intrp-rdr.toc: 67
  Term Rslt_8Args = pair_make(APP, 0, Rslt_7, SUB);
  Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
  Rslt_8Args = pair_make(APP, 0, Rslt_5, Rslt_8Args);
#ifdef STRICT
  store_redex(Rslt_8Args, glblVal848_1);
#else
  swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblVal848_1));
#endif

  // link args to body
  swapStore(r_seq_3, Rslt_8);

  Term Rslt_8V = get(term_loc(Rslt_8));
  apps = take(port(1, term_loc(Rslt_8V)));
  lams = take(port(2, term_loc(Rslt_8V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse842 = new_ref(glblrecurse841);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str23 = {StringBufferType, REFS_STATIC, 0, 0, 6, "(Rule "};
Term glblStr852 = term_new_(VAL, (Term)&str23);
Term glblstr_vect844;
void glblstr_vect843(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_11 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_11 = port(2, term_loc(seq_11));
  Term x_10 = term_new(VAR, 0, port(1, term_loc(seq_11)));
  // allocate args at intrp-rdr.toc: 68

  Term x_10_1;
  x_10 = dupeArg(x_10, &x_10_1, 0);
  Term x_10_1Args = pair_make(APP, 0, x_10_1, SUB);
  Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(x_10_1Args)));
  x_10_1Args = pair_make(APP, 0, new_i60(1), x_10_1Args);
  store_redex(x_10_1Args, accessField);
  Term x_10Args = pair_make(APP, 0, x_10, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(x_10Args)));
  x_10Args = pair_make(APP, 0, new_i60(0), x_10Args);
  store_redex(x_10Args, accessField);
  Term glblVal130_23;
  glblVal130 = dupeArg(glblVal130, &glblVal130_23, 0);

  // call vect-conj at intrp-rdr.toc: 69
  Term Rslt_13Args = pair_make(APP, 0, glblStr852, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, glblVal130_23, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblFn133);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 69
  Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
  Rslt_15Args = pair_make(APP, 0, Rslt_13, Rslt_15Args);
#ifdef STRICT
  store_redex(Rslt_15Args, glblFn133);
#else
  swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 69
  Term Rslt_16Args = pair_make(APP, 0, glblStr160, SUB);
  Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
  Rslt_16Args = pair_make(APP, 0, Rslt_15, Rslt_16Args);
#ifdef STRICT
  store_redex(Rslt_16Args, glblFn133);
#else
  swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, glblFn133));
#endif

  // call str-vect at intrp-rdr.toc: 69
  Term Rslt_18Args = pair_make(APP, 0, Rslt_17, SUB);
  Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
  Rslt_18Args = pair_make(APP, 0, new_i60(69), Rslt_18Args);
  Rslt_18Args = pair_make(APP, 0, glblStr869, Rslt_18Args);
#ifdef STRICT
  store_redex(Rslt_18Args, glblProto106);
#else
  swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblProto106));
#endif
  // call first at intrp-rdr.toc: 69
  Term Rslt_19Args = pair_make(APP, 0, Rslt_18, SUB);
  Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
  Rslt_19Args = pair_make(APP, 0, new_i60(69), Rslt_19Args);
  Rslt_19Args = pair_make(APP, 0, glblStr869, Rslt_19Args);
#ifdef STRICT
  store_redex(Rslt_19Args, glblProto72);
#else
  swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblProto72));
#endif
  // call vect-conj at intrp-rdr.toc: 69
  Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
  Rslt_20Args = pair_make(APP, 0, Rslt_16, Rslt_20Args);
#ifdef STRICT
  store_redex(Rslt_20Args, glblFn133);
#else
  swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 69
  Term Rslt_21Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
  Rslt_21Args = pair_make(APP, 0, Rslt_20, Rslt_21Args);
#ifdef STRICT
  store_redex(Rslt_21Args, glblFn133);
#else
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_11, Rslt_21);

  Term Rslt_21V = get(term_loc(Rslt_21));
  apps = take(port(1, term_loc(Rslt_21V)));
  lams = take(port(2, term_loc(Rslt_21V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_11);
  return;
}
Term glblstr_vect844 = new_ref(glblstr_vect843);
Term glblFld846;
void glblFldFn926(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld846 = new_ref(glblFldFn926);
Term glblFld847;
void glblFldFn927(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(1), args);
  store_redex(args, accessField);
  return;
}
Term glblFld847 = new_ref(glblFldFn927);
Term glblVal861 = new_ref(glblRecur859);
Term glblVal861;
Term glblrecurse856;
void glblrecurse855(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_3 = port(2, term_loc(seq_3));
  Term rec_f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  seq_3 = pair_make(LAM, 0, SUB, seq_3);
  Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
  // allocate args at intrp-rdr.toc: 72
  swapStore(term_loc(rec_f_2), ERA);
  // link args to body
  swapStore(r_seq_3, v_1);

  V = get(r_seq_3);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_3, term_new(VAR, 0, vLoc));
  store_redex(args, seq_3);
  return;
}
Term glblrecurse856 = new_ref(glblrecurse855);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str24 = {StringBufferType, REFS_STATIC, 0, 0, 7, "(Recur "};
Term glblStr863 = term_new_(VAL, (Term)&str24);
Term glblstr_vect858;
void glblstr_vect857(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_7 = port(2, term_loc(seq_7));
  Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
  // allocate args at intrp-rdr.toc: 76
  Term x_6Args = pair_make(APP, 0, x_6, SUB);
  Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(x_6Args)));
  x_6Args = pair_make(APP, 0, new_i60(0), x_6Args);
  store_redex(x_6Args, accessField);
  Term glblVal130_24;
  glblVal130 = dupeArg(glblVal130, &glblVal130_24, 0);

  // call vect-conj at intrp-rdr.toc: 77
  Term Rslt_9Args = pair_make(APP, 0, glblStr863, SUB);
  Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
  Rslt_9Args = pair_make(APP, 0, glblVal130_24, Rslt_9Args);
#ifdef STRICT
  store_redex(Rslt_9Args, glblFn133);
#else
  swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

  // call  at : 0
  Term Rslt_11Args = pair_make(APP, 0, NUL, SUB);
  Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
  store_redex(Rslt_11Args, Rslt_10);
#else
  swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, Rslt_10));
#endif

  // call .name at intrp-rdr.toc: 77
  Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
  Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
  Rslt_12Args = pair_make(APP, 0, new_i60(77), Rslt_12Args);
  Rslt_12Args = pair_make(APP, 0, glblStr869, Rslt_12Args);
#ifdef STRICT
  store_redex(Rslt_12Args, glblProto517);
#else
  swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblProto517));
#endif
  // call vect-conj at intrp-rdr.toc: 77
  Term Rslt_13Args = pair_make(APP, 0, Rslt_12, SUB);
  Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
  Rslt_13Args = pair_make(APP, 0, Rslt_9, Rslt_13Args);
#ifdef STRICT
  store_redex(Rslt_13Args, glblFn133);
#else
  swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn133));
#endif

  // call vect-conj at intrp-rdr.toc: 77
  Term Rslt_14Args = pair_make(APP, 0, glblStr745, SUB);
  Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
  Rslt_14Args = pair_make(APP, 0, Rslt_13, Rslt_14Args);
#ifdef STRICT
  store_redex(Rslt_14Args, glblFn133);
#else
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn133));
#endif

  // link args to body
  swapStore(r_seq_7, Rslt_14);

  Term Rslt_14V = get(term_loc(Rslt_14));
  apps = take(port(1, term_loc(Rslt_14V)));
  lams = take(port(2, term_loc(Rslt_14V)));

  vLoc = port(2, term_loc(apps));
  swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, apps, lams));
  store_redex(args, seq_7);
  return;
}
Term glblstr_vect858 = new_ref(glblstr_vect857);
Term glblFld860;
void glblFldFn931(Term ref, Term args) {
  args = pair_make(APP, 0, new_i60(0), args);
  store_redex(args, accessField);
  return;
}
Term glblFld860 = new_ref(glblFldFn931);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str25 = {StringBufferType, REFS_STATIC, 0, 0, 6, "digits"};
Term glblStr864 = term_new_(VAL, (Term)&str25);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str26 = {StringBufferType, REFS_STATIC, 0, 0, 1, "0"};
Term glblStr866 = term_new_(VAL, (Term)&str26);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str27 = {StringBufferType, REFS_STATIC, 0, 0, 1, "9"};
Term glblStr865 = term_new_(VAL, (Term)&str27);
Term glblRslt600;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str28 = {StringBufferType, REFS_STATIC, 0, 0, 1, "A"};
Term glblStr878 = term_new_(VAL, (Term)&str28);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str29 = {StringBufferType, REFS_STATIC, 0, 0, 1, "Z"};
Term glblStr877 = term_new_(VAL, (Term)&str29);
Term glblRslt604;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str30 = {StringBufferType, REFS_STATIC, 0, 0, 1, "a"};
Term glblStr887 = term_new_(VAL, (Term)&str30);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str31 = {StringBufferType, REFS_STATIC, 0, 0, 1, "z"};
Term glblStr886 = term_new_(VAL, (Term)&str31);
Term glblRslt608;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str32 = {StringBufferType, REFS_STATIC, 0, 0, 1, ";"};
Term glblStr894 = term_new_(VAL, (Term)&str32);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str33 = {StringBufferType, REFS_STATIC, 0, 0, 7, "comment"};
Term glblStr897 = term_new_(VAL, (Term)&str33);
Term glblRslt638;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str34 = {StringBufferType, REFS_STATIC, 0, 0, 1, ","};
Term glblStr935 = term_new_(VAL, (Term)&str34);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str35 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\t"};
Term glblStr933 = term_new_(VAL, (Term)&str35);
Term glblRslt643;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[11];
} str36 = {StringBufferType, REFS_STATIC, 0, 0, 10, "whitespace"};
Term glblStr936 = term_new_(VAL, (Term)&str36);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str37 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\r"};
Term glblStr937 = term_new_(VAL, (Term)&str37);
Term glblRslt648;
Term glblRslt649;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[4];
} str38 = {StringBufferType, REFS_STATIC, 0, 0, 3, "def"};
Term glblStr941 = term_new_(VAL, (Term)&str38);
Term glblRslt655;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[9];
} str39 = {StringBufferType, REFS_STATIC, 0, 0, 8, "defmacro"};
Term glblStr943 = term_new_(VAL, (Term)&str39);
Term glblRslt658;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[5];
} str40 = {StringBufferType, REFS_STATIC, 0, 0, 4, "defn"};
Term glblStr946 = term_new_(VAL, (Term)&str40);
Term glblRslt662;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str41 = {StringBufferType, REFS_STATIC, 0, 0, 7, "deftype"};
Term glblStr947 = term_new_(VAL, (Term)&str41);
Term glblRslt664;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[5];
} str42 = {StringBufferType, REFS_STATIC, 0, 0, 4, "defp"};
Term glblStr948 = term_new_(VAL, (Term)&str42);
Term glblRslt669;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str43 = {StringBufferType, REFS_STATIC, 0, 0, 11, "extend-type"};
Term glblStr949 = term_new_(VAL, (Term)&str43);
Term glblRslt672;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str44 = {StringBufferType, REFS_STATIC, 0, 0, 6, "add-ns"};
Term glblStr950 = term_new_(VAL, (Term)&str44);
Term glblRslt675;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str45 = {StringBufferType, REFS_STATIC, 0, 0, 6, "inline"};
Term glblStr951 = term_new_(VAL, (Term)&str45);
Term glblRslt680;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[5];
} str46 = {StringBufferType, REFS_STATIC, 0, 0, 4, "main"};
Term glblStr952 = term_new_(VAL, (Term)&str46);
Term glblRslt696;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[3];
} str47 = {StringBufferType, REFS_STATIC, 0, 0, 2, "fn"};
Term glblStr953 = term_new_(VAL, (Term)&str47);
Term glblRslt708;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[4];
} str48 = {StringBufferType, REFS_STATIC, 0, 0, 3, "let"};
Term glblStr954 = term_new_(VAL, (Term)&str48);
Term glblRslt718;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[3];
} str49 = {StringBufferType, REFS_STATIC, 0, 0, 2, "do"};
Term glblStr955 = term_new_(VAL, (Term)&str49);
Term glblRslt727;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str50 = {StringBufferType, REFS_STATIC, 0, 0, 1, "|"};
Term glblStr956 = term_new_(VAL, (Term)&str50);
Term glblRslt736;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[3];
} str51 = {StringBufferType, REFS_STATIC, 0, 0, 2, "->"};
Term glblStr957 = term_new_(VAL, (Term)&str51);
Term glblRslt741;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[6];
} str52 = {StringBufferType, REFS_STATIC, 0, 0, 5, "match"};
Term glblStr958 = term_new_(VAL, (Term)&str52);
Term glblRslt742;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str53 = {StringBufferType, REFS_STATIC, 0, 0, 1, "!"};
Term glblStr959 = term_new_(VAL, (Term)&str53);
Term glblRslt743;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str54 = {StringBufferType, REFS_STATIC, 0, 0, 6, "_FILE_"};
Term glblStr960 = term_new_(VAL, (Term)&str54);
Term glblRslt744;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str55 = {StringBufferType, REFS_STATIC, 0, 0, 6, "_LINE_"};
Term glblStr961 = term_new_(VAL, (Term)&str55);
Term glblRslt747;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str56 = {StringBufferType, REFS_STATIC, 0, 0, 1, "("};
Term glblStr962 = term_new_(VAL, (Term)&str56);
Term glblRslt748;
Term glblRslt749;
Term glblRslt758;
Term glblRslt763;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str57 = {StringBufferType, REFS_STATIC, 0, 0, 1, "{"};
Term glblStr966 = term_new_(VAL, (Term)&str57);
Term glblRslt767;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str58 = {StringBufferType, REFS_STATIC, 0, 0, 1, "}"};
Term glblStr967 = term_new_(VAL, (Term)&str58);
Term glblRslt768;
Term glblRslt769;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str59 = {StringBufferType, REFS_STATIC, 0, 0, 1, "'"};
Term glblStr969 = term_new_(VAL, (Term)&str59);
Term glblRslt777;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str60 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\""};
Term glblStr970 = term_new_(VAL, (Term)&str60);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str61 = {StringBufferType, REFS_STATIC, 0, 0, 7, "integer"};
Term glblStr971 = term_new_(VAL, (Term)&str61);
Term glblRslt782;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[6];
} str62 = {StringBufferType, REFS_STATIC, 0, 0, 5, "alpha"};
Term glblStr972 = term_new_(VAL, (Term)&str62);
Term glblRslt791;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[13];
} str63 = {StringBufferType, REFS_STATIC, 0, 0, 12, "symbol-start"};
Term glblStr973 = term_new_(VAL, (Term)&str63);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str64 = {StringBufferType, REFS_STATIC, 0, 0, 1, "."};
Term glblStr982 = term_new_(VAL, (Term)&str64);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str65 = {StringBufferType, REFS_STATIC, 0, 0, 1, "_"};
Term glblStr981 = term_new_(VAL, (Term)&str65);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str66 = {StringBufferType, REFS_STATIC, 0, 0, 1, "<"};
Term glblStr980 = term_new_(VAL, (Term)&str66);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str67 = {StringBufferType, REFS_STATIC, 0, 0, 1, ">"};
Term glblStr979 = term_new_(VAL, (Term)&str67);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str68 = {StringBufferType, REFS_STATIC, 0, 0, 1, "="};
Term glblStr978 = term_new_(VAL, (Term)&str68);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str69 = {StringBufferType, REFS_STATIC, 0, 0, 1, "+"};
Term glblStr977 = term_new_(VAL, (Term)&str69);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str70 = {StringBufferType, REFS_STATIC, 0, 0, 1, "-"};
Term glblStr976 = term_new_(VAL, (Term)&str70);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str71 = {StringBufferType, REFS_STATIC, 0, 0, 1, "*"};
Term glblStr975 = term_new_(VAL, (Term)&str71);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str72 = {StringBufferType, REFS_STATIC, 0, 0, 1, "/"};
Term glblStr974 = term_new_(VAL, (Term)&str72);
Term glblRslt800;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[15];
} str73 = {StringBufferType, REFS_STATIC, 0, 0, 14, "rest-of-symbol"};
Term glblStr983 = term_new_(VAL, (Term)&str73);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str74 = {StringBufferType, REFS_STATIC, 0, 0, 1, "?"};
Term glblStr985 = term_new_(VAL, (Term)&str74);
Term glblRslt803;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str75 = {StringBufferType, REFS_STATIC, 0, 0, 6, "symbol"};
Term glblStr994 = term_new_(VAL, (Term)&str75);
Term glblRslt813;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[6];
} str76 = {StringBufferType, REFS_STATIC, 0, 0, 5, "float"};
Term glblStr995 = term_new_(VAL, (Term)&str76);
Term glblRslt823;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[13];
} str77 = {StringBufferType, REFS_STATIC, 0, 0, 12, "escaped-char"};
Term glblStr997 = term_new_(VAL, (Term)&str77);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str78 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\\"};
Term glblStr998 = term_new_(VAL, (Term)&str78);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str79 = {StringBufferType, REFS_STATIC, 0, 0, 1, "n"};
Term glblStr1001 = term_new_(VAL, (Term)&str79);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str80 = {StringBufferType, REFS_STATIC, 0, 0, 1, "r"};
Term glblStr1000 = term_new_(VAL, (Term)&str80);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str81 = {StringBufferType, REFS_STATIC, 0, 0, 1, "t"};
Term glblStr999 = term_new_(VAL, (Term)&str81);
Term glblRslt833;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[21];
} str82 = {StringBufferType, REFS_STATIC, 0, 0, 20, "double-quoted-string"};
Term glblStr1004 = term_new_(VAL, (Term)&str82);
Term glblRslt834;
Term glblVal835;
Term glblVal838;
Term glblVal839;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[18];
} str83 = {StringBufferType, REFS_STATIC, 0, 0, 17, "vector-expression"};
Term glblStr1006 = term_new_(VAL, (Term)&str83);
Term glblRslt840;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[20];
} str84 = {StringBufferType, REFS_STATIC, 0, 0, 19, "hash-map-expression"};
Term glblStr1007 = term_new_(VAL, (Term)&str84);
Term glblRslt849;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[11];
} str85 = {StringBufferType, REFS_STATIC, 0, 0, 10, "expression"};
Term glblStr1008 = term_new_(VAL, (Term)&str85);
Term glblRslt867;
Term glblVal835;
void glblCFn873(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_1 = pair_make(LAM, 0, ERA, NUL);
  Location r_seq_1 = port(2, term_loc(seq_1));
  // allocate args at : 0
  Term glblRslt867_1;
  glblRslt867 = dupeArg(glblRslt867, &glblRslt867_1, 0);

  // link args to body
  swapStore(r_seq_1, glblRslt867_1);

  V = get(r_seq_1);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_1, term_new(VAR, 0, vLoc));
  store_redex(args, seq_1);
  return;
}
Term glblVal835 = new_ref(glblCFn873);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[15];
} str86 = {StringBufferType, REFS_STATIC, 0, 0, 14, "min-constraint"};
Term glblStr1009 = term_new_(VAL, (Term)&str86);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[4];
} str87 = {StringBufferType, REFS_STATIC, 0, 0, 3, "min"};
Term glblStr1010 = term_new_(VAL, (Term)&str87);
Term glblRslt907;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[15];
} str88 = {StringBufferType, REFS_STATIC, 0, 0, 14, "max-constraint"};
Term glblStr1012 = term_new_(VAL, (Term)&str88);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[4];
} str89 = {StringBufferType, REFS_STATIC, 0, 0, 3, "max"};
Term glblStr1013 = term_new_(VAL, (Term)&str89);
Term glblRslt932;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[22];
} str90 = {StringBufferType, REFS_STATIC, 0, 0, 21, "min-length-constraint"};
Term glblStr1014 = term_new_(VAL, (Term)&str90);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[11];
} str91 = {StringBufferType, REFS_STATIC, 0, 0, 10, "min-length"};
Term glblStr1015 = term_new_(VAL, (Term)&str91);
Term glblRslt939;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[22];
} str92 = {StringBufferType, REFS_STATIC, 0, 0, 21, "max-length-constraint"};
Term glblStr1016 = term_new_(VAL, (Term)&str92);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[11];
} str93 = {StringBufferType, REFS_STATIC, 0, 0, 10, "max-length"};
Term glblStr1017 = term_new_(VAL, (Term)&str93);
Term glblRslt940;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[18];
} str94 = {StringBufferType, REFS_STATIC, 0, 0, 17, "length-constraint"};
Term glblStr1018 = term_new_(VAL, (Term)&str94);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str95 = {StringBufferType, REFS_STATIC, 0, 0, 6, "length"};
Term glblStr1019 = term_new_(VAL, (Term)&str95);
Term glblRslt942;
Term glblFn945;
void glblCFn944(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = pair_make(LAM, 0, SUB, NUL);
  Location r_seq_2 = port(2, term_loc(seq_2));
  Term arg_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
  // allocate args at intrp-rdr.toc: 193
  swapStore(term_loc(arg_1), ERA);
  Term glblRslt867_2;
  glblRslt867 = dupeArg(glblRslt867, &glblRslt867_2, 0);

  Term glblVal130_76;
  glblVal130 = dupeArg(glblVal130, &glblVal130_76, 0);

  // call vect-conj at intrp-rdr.toc: 194
  Term Rslt_4Args = pair_make(APP, 0, glblRslt867_2, SUB);
  Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
  Rslt_4Args = pair_make(APP, 0, glblVal130_76, Rslt_4Args);
#ifdef STRICT
  store_redex(Rslt_4Args, glblFn133);
#else
  swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

  // call println* at intrp-rdr.toc: 194
  Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
  Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
  store_redex(Rslt_5Args, glblFn158);
#else
  swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn158));
#endif

  // link args to body
  swapStore(r_seq_2, new_i60(0));

  V = get(r_seq_2);
  apps = pair_make(APP, 0, V, SUB);
  lams = pair_make(LAM, 0, SUB, NUL);
  swapStore(port(2, term_loc(lams)), term_new(VAR, 0, port(1, term_loc(lams))));
  vLoc = port(2, term_loc(apps));
  apps = pair_make(APP, 0, Rslt_5, apps);
  lams = pair_make(LAM, 0, sideEffects, lams);

  swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
  swapStore(r_seq_2, term_new(VAR, 0, vLoc));
  store_redex(args, seq_2);
  return;
}
Term glblFn945 = new_ref(glblCFn944);
Term glblProto50;
void glblCFn51(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'wrap' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblwrap389);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'wrap' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto50 = new_ref(glblCFn51);
Term glblProto641;
void glblFldFn642(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.constructors' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 71:
        store_redex(args, glblFld646);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.constructors' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto641 = new_ref(glblFldFn642);
Term glblProto68;
void glblCFn69(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'reduce' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblreduce393);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'reduce' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto68 = new_ref(glblCFn69);
Term glblProto104;
void glblCFn105(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vals' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'vals' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto104 = new_ref(glblCFn105);
Term glblProto62;
void glblCFn63(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'empty' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblempty405);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'empty' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto62 = new_ref(glblCFn63);
Term glblProto624;
void glblFldFn625(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.expressions' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 72:
        store_redex(args, glblFld636);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.expressions' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto624 = new_ref(glblFldFn625);
Term glblProto80;
void glblCFn81(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'split' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'split' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto80 = new_ref(glblCFn81);
Term glblProto88;
void glblCFn89(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'drop-while' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'drop-while' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto88 = new_ref(glblCFn89);
Term glblProto36;
void glblCFn37(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'recurse' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glblrecurse317);
        break;
      case 46:
        store_redex(args, glblrecurse449);
        break;
      case 79:
        store_redex(args, glblrecurse784);
        break;
      case 43:
        store_redex(args, glblrecurse182);
        break;
      case 75:
        store_redex(args, glblrecurse827);
        break;
      case 80:
        store_redex(args, glblrecurse771);
        break;
      case 73:
        store_redex(args, glblrecurse856);
        break;
      case 78:
        store_redex(args, glblrecurse794);
        break;
      case 45:
        store_redex(args, glblrecurse315);
        break;
      case 74:
        store_redex(args, glblrecurse842);
        break;
      case 42:
        store_redex(args, glblrecurse146);
        break;
      case 77:
        store_redex(args, glblrecurse807);
        break;
      case 82:
        store_redex(args, glblrecurse734);
        break;
      case 76:
        store_redex(args, glblrecurse817);
        break;
      case 47:
        store_redex(args, glblrecurse430);
        break;
      case 81:
        store_redex(args, glblrecurse755);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'recurse' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto36 = new_ref(glblCFn37);
Term glblProto501;
void glblFldFn502(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.file' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 49:
        store_redex(args, glblFld506);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.file' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto501 = new_ref(glblFldFn502);
Term glblProto265;
void glblFldFn266(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.s' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 45:
        store_redex(args, glblFld276);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.s' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto265 = new_ref(glblFldFn266);
Term glblProto656;
void glblFldFn657(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.methods' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 69:
        store_redex(args, glblFld661);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.methods' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto656 = new_ref(glblFldFn657);
Term glblProto443;
void glblFldFn444(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.head' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 46:
        store_redex(args, glblFld455);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.head' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto443 = new_ref(glblFldFn444);
Term glblProto564;
void glblFldFn565(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.body' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 61:
        store_redex(args, glblFld569);
        break;
      case 70:
        store_redex(args, glblFld653);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.body' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto564 = new_ref(glblFldFn565);
Term glblProto28;
void glblCFn29(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'either' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glbleither180);
        break;
      case 42:
        store_redex(args, glbleither144);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'either' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto28 = new_ref(glblCFn29);
Term glblProto684;
void glblFldFn685(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.input' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 48:
        store_redex(args, glblFld693);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.input' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto684 = new_ref(glblFldFn685);
Term glblProto92;
void glblCFn93(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'reverse' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'reverse' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto92 = new_ref(glblCFn93);
Term glblProto16;
void glblCFn18(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      store_redex(args, glbl_EQ_222);
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glbl_EQ_313);
        break;
      case 43:
        store_redex(args, glbl_EQ_186);
        break;
      case 8:
        store_redex(args, glbl_EQ_419);
        break;
      case 45:
        store_redex(args, glbl_EQ_311);
        break;
      case 42:
        store_redex(args, glbl_EQ_148);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '=' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto16 = new_ref(glblCFn18);
Term glblProto584;
void glblFldFn585(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.field-name' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 58:
        store_redex(args, glblFld587);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.field-name' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto584 = new_ref(glblFldFn585);
Term glblProto531;
void glblFldFn532(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.ns' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 66:
        store_redex(args, glblFld536);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.ns' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto531 = new_ref(glblFldFn532);
Term glblProto94;
void glblCFn95(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'nth' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblnth409);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'nth' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto94 = new_ref(glblCFn95);
Term glblProto592;
void glblFldFn593(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.cases' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 57:
        store_redex(args, glblFld598);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.cases' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto592 = new_ref(glblFldFn593);
Term glblProto3;
void glblCFn4(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      store_redex(args, glbltype_name226);
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glbltype_name325);
        break;
      case 8:
        store_redex(args, glbltype_name417);
        break;
      case 45:
        store_redex(args, glbltype_name323);
        break;
      default: {
        store_redex(args, glbltype_name6);
      }
      }
    }
  }
  return;
}
Term glblProto3 = new_ref(glblCFn4);
Term glblProto521;
void glblFldFn522(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.implementations' found for integers "
              "called from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 55:
        store_redex(args, glblFld526);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.implementations' found for type %s "
                "(%ld) called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto521 = new_ref(glblFldFn522);
Term glblProto594;
void glblFldFn595(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.default-case' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 57:
        store_redex(args, glblFld599);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.default-case' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto594 = new_ref(glblFldFn595);
Term glblProto804;
void glblFldFn805(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.parsers' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 77:
        store_redex(args, glblFld811);
        break;
      case 76:
        store_redex(args, glblFld821);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.parsers' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto804 = new_ref(glblFldFn805);
Term glblProto24;
void glblCFn25(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'and' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glbland176);
        break;
      case 42:
        store_redex(args, glbland140);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'and' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto24 = new_ref(glblCFn25);
Term glblProto503;
void glblFldFn504(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.line' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 49:
        store_redex(args, glblFld507);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.line' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto503 = new_ref(glblFldFn504);
Term glblProto665;
void glblFldFn666(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.namespace' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 68:
        store_redex(args, glblFld671);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.namespace' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto665 = new_ref(glblFldFn666);
Term glblProto262;
void glblCFn263(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'subs' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glblsubs293);
        break;
      case 45:
        store_redex(args, glblsubs274);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'subs' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto262 = new_ref(glblCFn263);
Term glblProto445;
void glblFldFn447(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.tail' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 46:
        store_redex(args, glblFld456);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.tail' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto445 = new_ref(glblFldFn447);
Term glblProto90;
void glblCFn91(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'take-while' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'take-while' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto90 = new_ref(glblCFn91);
Term glblProto556;
void glblFldFn557(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.operands' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 62:
        store_redex(args, glblFld560);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.operands' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto556 = new_ref(glblFldFn557);
Term glblProto590;
void glblFldFn591(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.expr' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 57:
        store_redex(args, glblFld597);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.expr' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto590 = new_ref(glblFldFn591);
Term glblProto533;
void glblFldFn534(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.loc' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 63:
        store_redex(args, glblFld552);
        break;
      case 66:
        store_redex(args, glblFld538);
        break;
      case 65:
        store_redex(args, glblFld544);
        break;
      case 58:
        store_redex(args, glblFld588);
        break;
      case 64:
        store_redex(args, glblFld548);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.loc' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto533 = new_ref(glblFldFn534);
Term glblProto639;
void glblFldFn640(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.type-name' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 71:
        store_redex(args, glblFld645);
        break;
      case 69:
        store_redex(args, glblFld660);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.type-name' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto639 = new_ref(glblFldFn640);
Term glblProto44;
void glblCFn45(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'flatten' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblflatten385);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'flatten' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto44 = new_ref(glblCFn45);
Term glblProto22;
void glblCFn23(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      store_redex(args, glbl_LT_220);
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of '<' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto22 = new_ref(glblCFn23);
Term glblProto731;
void glblFldFn732(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.parser' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 75:
        store_redex(args, glblFld831);
        break;
      case 74:
        store_redex(args, glblFld847);
        break;
      case 82:
        store_redex(args, glblFld739);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.parser' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto731 = new_ref(glblFldFn732);
Term glblProto86;
void glblCFn87(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'drop' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'drop' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto86 = new_ref(glblCFn87);
Term glblProto38;
void glblCFn39(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'map' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblmap172);
        break;
      case 8:
        store_redex(args, glblmap391);
        break;
      case 42:
        store_redex(args, glblmap136);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'map' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto38 = new_ref(glblCFn39);
Term glblProto719;
void glblFldFn720(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.msg' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 78:
        store_redex(args, glblFld798);
        break;
      case 51:
        store_redex(args, glblFld722);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.msg' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto719 = new_ref(glblFldFn720);
Term glblProto40;
void glblCFn41(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'map!' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'map!' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto40 = new_ref(glblCFn41);
Term glblProto64;
void glblCFn65(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'conj' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblconj395);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'conj' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto64 = new_ref(glblCFn65);
Term glblProto72;
void glblCFn73(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'first' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblfirst401);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'first' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto72 = new_ref(glblCFn73);
Term glblProto709;
void glblFldFn710(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.state' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 52:
        store_redex(args, glblFld716);
        break;
      case 51:
        store_redex(args, glblFld723);
        break;
      case 53:
        store_redex(args, glblFld713);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.state' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto709 = new_ref(glblFldFn710);
Term glblProto78;
void glblCFn79(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'butlast' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'butlast' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto78 = new_ref(glblCFn79);
Term glblProto511;
void glblFldFn512(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.path' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 50:
        store_redex(args, glblFld514);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.path' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto511 = new_ref(glblFldFn512);
Term glblProto330;
void glblCFn331(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vect-reduce' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblvect_reduce344);
        break;
      case 42:
        store_redex(args, glblvect_reduce342);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'vect-reduce' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto330 = new_ref(glblCFn331);
Term glblProto750;
void glblFldFn751(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.lower' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 81:
        store_redex(args, glblFld760);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.lower' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto750 = new_ref(glblFldFn751);
Term glblProto32;
void glblCFn33(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'zero' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'zero' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto32 = new_ref(glblCFn33);
Term glblProto42;
void glblCFn43(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'flat-map' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblflat_map170);
        break;
      case 8:
        store_redex(args, glblflat_map387);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'flat-map' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto42 = new_ref(glblCFn43);
Term glblProto70;
void glblCFn71(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vec' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'vec' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto70 = new_ref(glblCFn71);
Term glblProto267;
void glblFldFn268(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.start' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 45:
        store_redex(args, glblFld277);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.start' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto267 = new_ref(glblFldFn268);
Term glblProto752;
void glblFldFn753(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.upper' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 81:
        store_redex(args, glblFld761);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.upper' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto752 = new_ref(glblFldFn753);
Term glblProto74;
void glblCFn75(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'rest' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblrest397);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'rest' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto74 = new_ref(glblCFn75);
Term glblProto46;
void glblCFn47(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'extend' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'extend' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto46 = new_ref(glblCFn47);
Term glblProto780;
void glblFldFn781(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.char' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 79:
        store_redex(args, glblFld788);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.char' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto780 = new_ref(glblFldFn781);
Term glblProto82;
void glblCFn83(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'split-with' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'split-with' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto82 = new_ref(glblCFn83);
Term glblProto554;
void glblFldFn555(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.operator' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 62:
        store_redex(args, glblFld559);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.operator' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto554 = new_ref(glblFldFn555);
Term glblProto58;
void glblCFn59(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'empty?' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblempty_QM_407);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'empty?' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto58 = new_ref(glblCFn59);
Term glblProto66;
void glblCFn67(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'filter' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'filter' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto66 = new_ref(glblCFn67);
Term glblProto96;
void glblCFn97(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'store' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'store' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto96 = new_ref(glblCFn97);
Term glblProto688;
void glblFldFn689(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.values' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 48:
        store_redex(args, glblFld695);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.values' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto688 = new_ref(glblFldFn689);
Term glblProto562;
void glblFldFn563(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.parameter-list' found for integers "
              "called from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 61:
        store_redex(args, glblFld568);
        break;
      case 70:
        store_redex(args, glblFld652);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.parameter-list' found for type %s "
                "(%ld) called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto562 = new_ref(glblFldFn563);
Term glblProto84;
void glblCFn85(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'take' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'take' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto84 = new_ref(glblCFn85);
Term glblProto60;
void glblCFn61(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'count' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glblcount295);
        break;
      case 46:
        store_redex(args, glblcount452);
        break;
      case 8:
        store_redex(args, glblcount413);
        break;
      case 45:
        store_redex(args, glblcount272);
        break;
      case 47:
        store_redex(args, glblcount433);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'count' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto60 = new_ref(glblCFn61);
Term glblProto14;
void glblFldFn15(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.x' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblFld19);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.x' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto14 = new_ref(glblFldFn15);
Term glblProto540;
void glblFldFn541(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.value' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 63:
        store_redex(args, glblFld551);
        break;
      case 80:
        store_redex(args, glblFld775);
        break;
      case 65:
        store_redex(args, glblFld543);
        break;
      case 53:
        store_redex(args, glblFld712);
        break;
      case 64:
        store_redex(args, glblFld547);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.value' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto540 = new_ref(glblFldFn541);
Term glblProto667;
void glblFldFn668(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.module-path' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 68:
        store_redex(args, glblFld673);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.module-path' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto667 = new_ref(glblFldFn668);
Term glblProto106;
void glblCFn107(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      store_redex(args, glblstr_vect224);
    } else {
      switch (dispVal->type) {
      case 2:
        store_redex(args, glblstr_vect321);
        break;
      case 79:
        store_redex(args, glblstr_vect786);
        break;
      case 8:
        store_redex(args, glblstr_vect415);
        break;
      case 75:
        store_redex(args, glblstr_vect829);
        break;
      case 80:
        store_redex(args, glblstr_vect773);
        break;
      case 73:
        store_redex(args, glblstr_vect858);
        break;
      case 78:
        store_redex(args, glblstr_vect796);
        break;
      case 45:
        store_redex(args, glblstr_vect319);
        break;
      case 74:
        store_redex(args, glblstr_vect844);
        break;
      case 77:
        store_redex(args, glblstr_vect809);
        break;
      case 82:
        store_redex(args, glblstr_vect737);
        break;
      case 76:
        store_redex(args, glblstr_vect819);
        break;
      case 81:
        store_redex(args, glblstr_vect757);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'str-vect' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto106 = new_ref(glblCFn107);
Term glblProto519;
void glblFldFn520(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.field-list' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 55:
        store_redex(args, glblFld525);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.field-list' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto519 = new_ref(glblFldFn520);
Term glblProto56;
void glblCFn57(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'interpose' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblinterpose383);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'interpose' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto56 = new_ref(glblCFn57);
Term glblProto853;
void glblFldFn854(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.f' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 73:
        store_redex(args, glblFld860);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.f' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto853 = new_ref(glblFldFn854);
Term glblProto576;
void glblFldFn577(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.symbol' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 59:
        store_redex(args, glblFld581);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.symbol' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto576 = new_ref(glblFldFn577);
Term glblProto676;
void glblFldFn677(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.c-code' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 67:
        store_redex(args, glblFld679);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.c-code' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto676 = new_ref(glblFldFn677);
Term glblProto34;
void glblCFn35(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'comp' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblcomp403);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'comp' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto34 = new_ref(glblCFn35);
Term glblProto578;
void glblFldFn579(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.type-expr' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 59:
        store_redex(args, glblFld582);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.type-expr' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto578 = new_ref(glblFldFn579);
Term glblProto102;
void glblCFn103(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'keys' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'keys' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto102 = new_ref(glblCFn103);
Term glblProto54;
void glblCFn55(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'apply' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'apply' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto54 = new_ref(glblCFn55);
Term glblProto26;
void glblCFn27(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'or' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblor174);
        break;
      case 42:
        store_redex(args, glblor138);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'or' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto26 = new_ref(glblCFn27);
Term glblProto571;
void glblFldFn572(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.alternatives' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 60:
        store_redex(args, glblFld574);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.alternatives' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto571 = new_ref(glblFldFn572);
Term glblProto30;
void glblCFn31(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'cond' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblcond178);
        break;
      case 42:
        store_redex(args, glblcond142);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'cond' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto30 = new_ref(glblCFn31);
Term glblProto602;
void glblFldFn603(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.lines' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 56:
        store_redex(args, glblFld606);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.lines' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto602 = new_ref(glblFldFn603);
Term glblProto76;
void glblCFn77(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'last' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glbllast399);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'last' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto76 = new_ref(glblCFn77);
Term glblProto269;
void glblFldFn270(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.len' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 46:
        store_redex(args, glblFld457);
        break;
      case 45:
        store_redex(args, glblFld278);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.len' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto269 = new_ref(glblFldFn270);
Term glblProto98;
void glblCFn99(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'assoc' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'assoc' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto98 = new_ref(glblCFn99);
Term glblProto52;
void glblCFn53(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'extract' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 43:
        store_redex(args, glblextract184);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'extract' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto52 = new_ref(glblCFn53);
Term glblProto48;
void glblCFn49(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'duplicate' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'duplicate' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto48 = new_ref(glblCFn49);
Term glblProto517;
void glblFldFn518(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.name' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 61:
        store_redex(args, glblFld567);
        break;
      case 72:
        store_redex(args, glblFld635);
        break;
      case 66:
        store_redex(args, glblFld537);
        break;
      case 70:
        store_redex(args, glblFld651);
        break;
      case 55:
        store_redex(args, glblFld524);
        break;
      case 74:
        store_redex(args, glblFld846);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.name' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto517 = new_ref(glblFldFn518);
Term glblProto108;
void glblCFn109(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'free-resource' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'free-resource' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto108 = new_ref(glblCFn109);
Term glblProto100;
void glblCFn101(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'get' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
      abort();
    } else {
      switch (dispVal->type) {
      case 8:
        store_redex(args, glblget411);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'get' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, get_i60(arityArgs.args[1]));
        abort();
      }
      }
    }
  }
  return;
}
Term glblProto100 = new_ref(glblCFn101);
unsigned refsCount = 162;
unsigned refNameCount = 162;
refMap refNames[167] = {{vectConjFn, "vectConj"},
                        {accessFieldFn, "accessField"},
                        {hvmVectFn, "hvmVect"},
                        {constructFn, "construct"},
                        {glblCFn113, "pr*"},
                        {glblCFn51, "wrap"},
                        {glblCFn187, "int-="},
                        {glblFldFn642, ".constructors"},
                        {glblCFn69, "reduce"},
                        {glblCFn105, "vals"},
                        {glblCFn63, "empty"},
                        {glblFldFn625, ".expressions"},
                        {glblCFn81, "split"},
                        {glblCFn89, "drop-while"},
                        {glblCFn360, "to-str"},
                        {glblCFn37, "recurse"},
                        {glblFldFn502, ".file"},
                        {glblFldFn266, ".s"},
                        {glblCFn128, "*"},
                        {glblFldFn657, ".methods"},
                        {glblFldFn444, ".head"},
                        {glblFldFn565, ".body"},
                        {glblCFn29, "either"},
                        {glblFldFn685, ".input"},
                        {glblCFn193, "<="},
                        {glblCFn93, "reverse"},
                        {glblCFn18, "="},
                        {glblCFn234, "vect-count"},
                        {glblFldFn585, ".field-name"},
                        {glblFldFn532, ".ns"},
                        {glblCons453, "Cons"},
                        {glblCFn95, "nth"},
                        {glblCFn116, "+"},
                        {glblCFn228, "min"},
                        {glblFldFn593, ".cases"},
                        {glblCFn4, "type-name"},
                        {glblFldFn522, ".implementations"},
                        {glblCFn327, "vect-get"},
                        {glblFldFn595, ".default-case"},
                        {glblCFn426, "str*"},
                        {glblFldFn805, ".parsers"},
                        {glblCFn25, "and"},
                        {glblFldFn504, ".line"},
                        {glblFldFn666, ".namespace"},
                        {glblCFn424, "cons"},
                        {glblCFn263, "subs"},
                        {glblFldFn447, ".tail"},
                        {glblCFn91, "take-while"},
                        {glblFldFn557, ".operands"},
                        {glblCFn7, "type-num"},
                        {glblFldFn591, ".expr"},
                        {glblFldFn534, ".loc"},
                        {glblCFn110, "abort"},
                        {glblFldFn640, ".type-name"},
                        {glblCFn45, "flatten"},
                        {glblCFn23, "<"},
                        {glblFldFn732, ".parser"},
                        {glblCFn87, "drop"},
                        {glblCFn39, "map"},
                        {glblCFn243, "fold"},
                        {glblFldFn720, ".msg"},
                        {glblCFn41, "map!"},
                        {glblCFn65, "conj"},
                        {glblCFn73, "first"},
                        {glblFldFn710, ".state"},
                        {glblCFn79, "butlast"},
                        {glblFldFn512, ".path"},
                        {glblCFn257, "str-malloc"},
                        {glblCFn331, "vect-reduce"},
                        {glblCFn296, "str-eq"},
                        {glblFldFn751, ".lower"},
                        {glblCFn260, "str-append"},
                        {glblCFn33, "zero"},
                        {glblCFn43, "flat-map"},
                        {glblCFn71, "vec"},
                        {glblFldFn268, ".start"},
                        {glblCFn191, "number-str"},
                        {glblCFn247, "unfold"},
                        {glblFldFn753, ".upper"},
                        {glblCFn75, "rest"},
                        {glblCFn47, "extend"},
                        {glblCFn157, "println*"},
                        {glblCFn369, "vect-="},
                        {glblFldFn781, ".char"},
                        {glblCFn83, "split-with"},
                        {glblFldFn555, ".operator"},
                        {glblCFn59, "empty?"},
                        {glblCFn67, "filter"},
                        {glblCFn97, "store"},
                        {glblFldFn689, ".values"},
                        {glblFldFn563, ".parameter-list"},
                        {glblCFn85, "take"},
                        {glblCFn350, "subvec*"},
                        {glblCFn61, "count"},
                        {glblFldFn15, ".x"},
                        {glblFldFn541, ".value"},
                        {glblFldFn668, ".module-path"},
                        {glblSome17, "Some"},
                        {glblCFn107, "str-vect"},
                        {glblFldFn520, ".field-list"},
                        {glblCFn118, "inc"},
                        {glblCFn57, "interpose"},
                        {glblCFn122, "-"},
                        {glblFldFn854, ".f"},
                        {glblFldFn577, ".symbol"},
                        {glblCFn352, "subvec"},
                        {glblFldFn677, ".c-code"},
                        {glblCFn125, "dec"},
                        {glblCFn132, "vect-conj"},
                        {glblCFn35, "comp"},
                        {glblFldFn579, ".type-expr"},
                        {glblCFn0, "default-type-name"},
                        {glblCFn103, "keys"},
                        {glblCFn55, "apply"},
                        {glblCFn27, "or"},
                        {glblFldFn572, ".alternatives"},
                        {glblCFn31, "cond"},
                        {glblFldFn603, ".lines"},
                        {glblCFn77, "last"},
                        {glblFldFn270, ".len"},
                        {glblCFn99, "assoc"},
                        {glblCFn249, "str-count"},
                        {glblCFn53, "extract"},
                        {glblCFn49, "duplicate"},
                        {glblCFn240, "identity"},
                        {glblSubString275, "SubString"},
                        {glblFldFn518, ".name"},
                        {glblCFn109, "free-resource"},
                        {glblCFn101, "get"},
                        {glblCFn189, "int-<"},
                        {glblRecur859, "Recur"},
                        {glblMany830, "Many"},
                        {glblParserError721, "ParserError"},
                        {glblParserIgnore715, "ParserIgnore"},
                        {glblRule845, "Rule"},
                        {glblIgnore738, "Ignore"},
                        {glblAny820, "Any"},
                        {glblCharRange759, "CharRange"},
                        {glblParserMatch711, "ParserMatch"},
                        {glblAlwaysSucceed774, "AlwaysSucceed"},
                        {glblCFn944, "main"},
                        {glblAll810, "All"},
                        {glblCFn873, "sub-expression"},
                        {glblNotChar787, "NotChar"},
                        {glblParserState692, "ParserState"},
                        {glblError797, "Error"},
                        {glblSymbol535, "Symbol"},
                        {glblDefType644, "DefType"},
                        {glblStringLit550, "StringLit"},
                        {glblDefp650, "Defp"},
                        {glblFieldGetter586, "FieldGetter"},
                        {glblIntegerLit542, "IntegerLit"},
                        {glblConstructor523, "Constructor"},
                        {glblSuperposition573, "Superposition"},
                        {glblDefinition634, "Definition"},
                        {glblTypeConstraint580, "TypeConstraint"},
                        {glblExtendType659, "ExtendType"},
                        {glblLocation505, "Location"},
                        {glblFloatLit546, "FloatLit"},
                        {glblBlockComment605, "BlockComment"},
                        {glblFn566, "Fn"},
                        {glblInline678, "Inline"},
                        {glblAddNs670, "AddNs"},
                        {glblMatch596, "Match"},
                        {glblCall558, "Call"},
                        {glblModule513, "Module"}};
typeNameMap typeNames[56] = {
    {1, "Integer"},      {2, "String"},          {8, "Vector"},
    {9, "VectorNode"},   {11, "HashMap"},        {12, "HashMap"},
    {13, "HashMap"},     {14, "HashMap"},        {11, "BitmapIndexedNode"},
    {46, "Cons"},        {18, "Opaque"},         {3, "FnArity"},
    {12, "ArrayNode"},   {43, "Some"},           {13, "HashCollisionNode"},
    {42, "None"},        {47, "EndOfList"},      {44, "Leaf"},
    {45, "SubString"},   {73, "Recur"},          {75, "Many"},
    {51, "ParserError"}, {52, "ParserIgnore"},   {54, "ParserFail"},
    {74, "Rule"},        {82, "Ignore"},         {76, "Any"},
    {81, "CharRange"},   {53, "ParserMatch"},    {80, "AlwaysSucceed"},
    {77, "All"},         {79, "NotChar"},        {48, "ParserState"},
    {78, "Error"},       {66, "Symbol"},         {71, "DefType"},
    {63, "StringLit"},   {70, "Defp"},           {58, "FieldGetter"},
    {65, "IntegerLit"},  {55, "Constructor"},    {60, "Superposition"},
    {72, "Definition"},  {59, "TypeConstraint"}, {69, "ExtendType"},
    {49, "Location"},    {64, "FloatLit"},       {56, "BlockComment"},
    {61, "Fn"},          {67, "Inline"},         {68, "AddNs"},
    {57, "Match"},       {62, "Call"},           {50, "Module"}};
void normGlobals() {
  Term apps, lams, V;
  Location vLoc;
  {
    Term glblVal13_1;
    glblVal13 = dupeArg(glblVal13, &glblVal13_1, 0);

    // call None at core: 63
    Term glblRslt12Args = pair_make(APP, 0, NUL, SUB);
    glblRslt12 = term_new(VAR, 0, port(2, term_loc(glblRslt12Args)));
    store_redex(glblRslt12Args, glblVal13_1);
  }
  {
    Term glblVal253_1;
    glblVal253 = dupeArg(glblVal253, &glblVal253_1, 0);

    // call Leaf at core: 412
    Term glblRslt252Args = pair_make(APP, 0, NUL, SUB);
    glblRslt252 = term_new(VAR, 0, port(2, term_loc(glblRslt252Args)));
    store_redex(glblRslt252Args, glblVal253_1);
  }
  {
    Term glblVal437_1;
    glblVal437 = dupeArg(glblVal437, &glblVal437_1, 0);

    // call EndOfList at core: 711
    Term glblRslt436Args = pair_make(APP, 0, NUL, SUB);
    glblRslt436 = term_new(VAR, 0, port(2, term_loc(glblRslt436Args)));
    store_redex(glblRslt436Args, glblVal437_1);
  }
  {
    Term glblVal707_1;
    glblVal707 = dupeArg(glblVal707, &glblVal707_1, 0);

    // call ParserFail at intrp-rdr.toc: 9
    Term glblRslt706Args = pair_make(APP, 0, NUL, SUB);
    glblRslt706 = term_new(VAR, 0, port(2, term_loc(glblRslt706Args)));
    store_redex(glblRslt706Args, glblVal707_1);
  }
  {
    Term glblVal762_1;
    glblVal762 = dupeArg(glblVal762, &glblVal762_1, 0);

    Term glblVal848_2;
    glblVal848 = dupeArg(glblVal848, &glblVal848_2, 0);

    // call CharRange at intrp-rdr.toc: 79
    Term Rslt_0Args = pair_make(APP, 0, glblStr865, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblStr866, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblVal762_1);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblVal762_1));
#endif

    // call Rule at intrp-rdr.toc: 79
    Term glblRslt600Args = pair_make(APP, 0, Rslt_0, SUB);
    glblRslt600 = term_new(VAR, 0, port(2, term_loc(glblRslt600Args)));
    glblRslt600Args = pair_make(APP, 0, glblStr864, glblRslt600Args);
    store_redex(glblRslt600Args, glblVal848_2);
  }
  {
    Term glblVal762_2;
    glblVal762 = dupeArg(glblVal762, &glblVal762_2, 0);

    // call CharRange at intrp-rdr.toc: 81
    Term glblRslt604Args = pair_make(APP, 0, glblStr877, SUB);
    glblRslt604 = term_new(VAR, 0, port(2, term_loc(glblRslt604Args)));
    glblRslt604Args = pair_make(APP, 0, glblStr878, glblRslt604Args);
    store_redex(glblRslt604Args, glblVal762_2);
  }
  {
    Term glblVal762_3;
    glblVal762 = dupeArg(glblVal762, &glblVal762_3, 0);

    // call CharRange at intrp-rdr.toc: 83
    Term glblRslt608Args = pair_make(APP, 0, glblStr886, SUB);
    glblRslt608 = term_new(VAR, 0, port(2, term_loc(glblRslt608Args)));
    glblRslt608Args = pair_make(APP, 0, glblStr887, glblRslt608Args);
    store_redex(glblRslt608Args, glblVal762_3);
  }
  {
    Term glblVal789_1;
    glblVal789 = dupeArg(glblVal789, &glblVal789_1, 0);

    Term glblVal832_2;
    glblVal832 = dupeArg(glblVal832, &glblVal832_2, 0);

    Term glblStr894_1;
    glblStr894 = dupeArg(glblStr894, &glblStr894_1, 0);

    Term glblVal130_25;
    glblVal130 = dupeArg(glblVal130, &glblVal130_25, 0);

    Term glblVal812_2;
    glblVal812 = dupeArg(glblVal812, &glblVal812_2, 0);

    Term glblVal848_3;
    glblVal848 = dupeArg(glblVal848, &glblVal848_3, 0);

    // call vect-conj at intrp-rdr.toc: 87
    Term Rslt_0Args = pair_make(APP, 0, glblStr894_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_25, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call NotChar at intrp-rdr.toc: 87
    Term Rslt_1Args = pair_make(APP, 0, glblStr159, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal789_1);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal789_1));
#endif

    // call Many at intrp-rdr.toc: 87
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
#ifdef STRICT
    store_redex(Rslt_2Args, glblVal832_2);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblVal832_2));
#endif

    // call vect-conj at intrp-rdr.toc: 87
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_0, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 87
    Term Rslt_4Args = pair_make(APP, 0, Rslt_3, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
    store_redex(Rslt_4Args, glblVal812_2);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblVal812_2));
#endif

    // call Rule at intrp-rdr.toc: 87
    Term glblRslt638Args = pair_make(APP, 0, Rslt_4, SUB);
    glblRslt638 = term_new(VAR, 0, port(2, term_loc(glblRslt638Args)));
    glblRslt638Args = pair_make(APP, 0, glblStr897, glblRslt638Args);
    store_redex(glblRslt638Args, glblVal848_3);
  }
  {
    Term glblVal130_26;
    glblVal130 = dupeArg(glblVal130, &glblVal130_26, 0);

    Term glblVal822_2;
    glblVal822 = dupeArg(glblVal822, &glblVal822_2, 0);

    // call vect-conj at intrp-rdr.toc: 90
    Term Rslt_0Args = pair_make(APP, 0, glblStr935, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_26, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 90
    Term Rslt_1Args = pair_make(APP, 0, glblStr160, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 90
    Term Rslt_2Args = pair_make(APP, 0, glblStr933, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 90
    Term glblRslt643Args = pair_make(APP, 0, Rslt_2, SUB);
    glblRslt643 = term_new(VAR, 0, port(2, term_loc(glblRslt643Args)));
    store_redex(glblRslt643Args, glblVal822_2);
  }
  {
    Term glblRslt638_1;
    glblRslt638 = dupeArg(glblRslt638, &glblRslt638_1, 0);

    Term glblRslt643_1;
    glblRslt643 = dupeArg(glblRslt643, &glblRslt643_1, 0);

    Term glblVal130_27;
    glblVal130 = dupeArg(glblVal130, &glblVal130_27, 0);

    Term glblVal822_3;
    glblVal822 = dupeArg(glblVal822, &glblVal822_3, 0);

    Term glblVal832_3;
    glblVal832 = dupeArg(glblVal832, &glblVal832_3, 0);

    Term glblVal848_4;
    glblVal848 = dupeArg(glblVal848, &glblVal848_4, 0);

    // call vect-conj at intrp-rdr.toc: 91
    Term Rslt_0Args = pair_make(APP, 0, glblRslt643_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_27, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 91
    Term Rslt_1Args = pair_make(APP, 0, glblStr159, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 91
    Term Rslt_2Args = pair_make(APP, 0, glblStr937, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 91
    Term Rslt_3Args = pair_make(APP, 0, glblRslt638_1, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 91
    Term Rslt_4Args = pair_make(APP, 0, Rslt_3, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
    store_redex(Rslt_4Args, glblVal822_3);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblVal822_3));
#endif

    // call Many at intrp-rdr.toc: 91
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal832_3);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal832_3));
#endif

    // call Rule at intrp-rdr.toc: 91
    Term glblRslt648Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt648 = term_new(VAR, 0, port(2, term_loc(glblRslt648Args)));
    glblRslt648Args = pair_make(APP, 0, glblStr936, glblRslt648Args);
    store_redex(glblRslt648Args, glblVal848_4);
  }
  {
    Term glblRslt648_1;
    glblRslt648 = dupeArg(glblRslt648, &glblRslt648_1, 0);

    Term glblVal740_2;
    glblVal740 = dupeArg(glblVal740, &glblVal740_2, 0);

    // call Ignore at intrp-rdr.toc: 92
    Term glblRslt649Args = pair_make(APP, 0, glblRslt648_1, SUB);
    glblRslt649 = term_new(VAR, 0, port(2, term_loc(glblRslt649Args)));
    store_redex(glblRslt649Args, glblVal740_2);
  }
  {
    Term glblRslt649_1;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_1, 0);

    Term glblVal130_28;
    glblVal130 = dupeArg(glblVal130, &glblVal130_28, 0);

    Term glblVal812_3;
    glblVal812 = dupeArg(glblVal812, &glblVal812_3, 0);

    // call vect-conj at intrp-rdr.toc: 95
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_28, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 95
    Term Rslt_1Args = pair_make(APP, 0, glblStr941, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 95
    Term glblRslt655Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt655 = term_new(VAR, 0, port(2, term_loc(glblRslt655Args)));
    store_redex(glblRslt655Args, glblVal812_3);
  }
  {
    Term glblRslt649_2;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_2, 0);

    Term glblVal130_29;
    glblVal130 = dupeArg(glblVal130, &glblVal130_29, 0);

    Term glblVal812_4;
    glblVal812 = dupeArg(glblVal812, &glblVal812_4, 0);

    // call vect-conj at intrp-rdr.toc: 96
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_2, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_29, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 96
    Term Rslt_1Args = pair_make(APP, 0, glblStr943, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 96
    Term glblRslt658Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt658 = term_new(VAR, 0, port(2, term_loc(glblRslt658Args)));
    store_redex(glblRslt658Args, glblVal812_4);
  }
  {
    Term glblRslt649_3;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_3, 0);

    Term glblVal130_30;
    glblVal130 = dupeArg(glblVal130, &glblVal130_30, 0);

    Term glblVal812_5;
    glblVal812 = dupeArg(glblVal812, &glblVal812_5, 0);

    // call vect-conj at intrp-rdr.toc: 97
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_3, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_30, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 97
    Term Rslt_1Args = pair_make(APP, 0, glblStr946, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 97
    Term glblRslt662Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt662 = term_new(VAR, 0, port(2, term_loc(glblRslt662Args)));
    store_redex(glblRslt662Args, glblVal812_5);
  }
  {
    Term glblRslt649_4;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_4, 0);

    Term glblVal130_31;
    glblVal130 = dupeArg(glblVal130, &glblVal130_31, 0);

    Term glblVal812_6;
    glblVal812 = dupeArg(glblVal812, &glblVal812_6, 0);

    // call vect-conj at intrp-rdr.toc: 98
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_4, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_31, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 98
    Term Rslt_1Args = pair_make(APP, 0, glblStr947, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 98
    Term glblRslt664Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt664 = term_new(VAR, 0, port(2, term_loc(glblRslt664Args)));
    store_redex(glblRslt664Args, glblVal812_6);
  }
  {
    Term glblRslt649_5;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_5, 0);

    Term glblVal130_32;
    glblVal130 = dupeArg(glblVal130, &glblVal130_32, 0);

    Term glblVal812_7;
    glblVal812 = dupeArg(glblVal812, &glblVal812_7, 0);

    // call vect-conj at intrp-rdr.toc: 99
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_5, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_32, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 99
    Term Rslt_1Args = pair_make(APP, 0, glblStr948, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 99
    Term glblRslt669Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt669 = term_new(VAR, 0, port(2, term_loc(glblRslt669Args)));
    store_redex(glblRslt669Args, glblVal812_7);
  }
  {
    Term glblRslt649_6;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_6, 0);

    Term glblVal130_33;
    glblVal130 = dupeArg(glblVal130, &glblVal130_33, 0);

    Term glblVal812_8;
    glblVal812 = dupeArg(glblVal812, &glblVal812_8, 0);

    // call vect-conj at intrp-rdr.toc: 100
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_6, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_33, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 100
    Term Rslt_1Args = pair_make(APP, 0, glblStr949, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 100
    Term glblRslt672Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt672 = term_new(VAR, 0, port(2, term_loc(glblRslt672Args)));
    store_redex(glblRslt672Args, glblVal812_8);
  }
  {
    Term glblRslt649_7;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_7, 0);

    Term glblVal130_34;
    glblVal130 = dupeArg(glblVal130, &glblVal130_34, 0);

    Term glblVal812_9;
    glblVal812 = dupeArg(glblVal812, &glblVal812_9, 0);

    // call vect-conj at intrp-rdr.toc: 101
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_7, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_34, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 101
    Term Rslt_1Args = pair_make(APP, 0, glblStr950, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 101
    Term glblRslt675Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt675 = term_new(VAR, 0, port(2, term_loc(glblRslt675Args)));
    store_redex(glblRslt675Args, glblVal812_9);
  }
  {
    Term glblRslt649_8;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_8, 0);

    Term glblVal130_35;
    glblVal130 = dupeArg(glblVal130, &glblVal130_35, 0);

    Term glblVal812_10;
    glblVal812 = dupeArg(glblVal812, &glblVal812_10, 0);

    // call vect-conj at intrp-rdr.toc: 102
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_8, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_35, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 102
    Term Rslt_1Args = pair_make(APP, 0, glblStr951, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 102
    Term glblRslt680Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt680 = term_new(VAR, 0, port(2, term_loc(glblRslt680Args)));
    store_redex(glblRslt680Args, glblVal812_10);
  }
  {
    Term glblRslt649_9;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_9, 0);

    Term glblVal130_36;
    glblVal130 = dupeArg(glblVal130, &glblVal130_36, 0);

    Term glblVal812_11;
    glblVal812 = dupeArg(glblVal812, &glblVal812_11, 0);

    // call vect-conj at intrp-rdr.toc: 103
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_9, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_36, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 103
    Term Rslt_1Args = pair_make(APP, 0, glblStr952, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 103
    Term glblRslt696Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt696 = term_new(VAR, 0, port(2, term_loc(glblRslt696Args)));
    store_redex(glblRslt696Args, glblVal812_11);
  }
  {
    Term glblRslt649_10;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_10, 0);

    Term glblVal130_37;
    glblVal130 = dupeArg(glblVal130, &glblVal130_37, 0);

    Term glblVal812_12;
    glblVal812 = dupeArg(glblVal812, &glblVal812_12, 0);

    // call vect-conj at intrp-rdr.toc: 106
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_10, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_37, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 106
    Term Rslt_1Args = pair_make(APP, 0, glblStr953, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 106
    Term glblRslt708Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt708 = term_new(VAR, 0, port(2, term_loc(glblRslt708Args)));
    store_redex(glblRslt708Args, glblVal812_12);
  }
  {
    Term glblRslt649_11;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_11, 0);

    Term glblVal130_38;
    glblVal130 = dupeArg(glblVal130, &glblVal130_38, 0);

    Term glblVal812_13;
    glblVal812 = dupeArg(glblVal812, &glblVal812_13, 0);

    // call vect-conj at intrp-rdr.toc: 107
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_11, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_38, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 107
    Term Rslt_1Args = pair_make(APP, 0, glblStr954, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 107
    Term glblRslt718Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt718 = term_new(VAR, 0, port(2, term_loc(glblRslt718Args)));
    store_redex(glblRslt718Args, glblVal812_13);
  }
  {
    Term glblRslt649_12;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_12, 0);

    Term glblVal130_39;
    glblVal130 = dupeArg(glblVal130, &glblVal130_39, 0);

    Term glblVal812_14;
    glblVal812 = dupeArg(glblVal812, &glblVal812_14, 0);

    // call vect-conj at intrp-rdr.toc: 108
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_12, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_39, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 108
    Term Rslt_1Args = pair_make(APP, 0, glblStr955, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 108
    Term glblRslt727Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt727 = term_new(VAR, 0, port(2, term_loc(glblRslt727Args)));
    store_redex(glblRslt727Args, glblVal812_14);
  }
  {
    Term glblRslt649_13;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_13, 0);

    Term glblVal130_40;
    glblVal130 = dupeArg(glblVal130, &glblVal130_40, 0);

    Term glblVal812_15;
    glblVal812 = dupeArg(glblVal812, &glblVal812_15, 0);

    // call vect-conj at intrp-rdr.toc: 109
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_13, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_40, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 109
    Term Rslt_1Args = pair_make(APP, 0, glblStr956, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 109
    Term glblRslt736Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt736 = term_new(VAR, 0, port(2, term_loc(glblRslt736Args)));
    store_redex(glblRslt736Args, glblVal812_15);
  }
  {
    Term glblRslt649_14;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_14, 0);

    Term glblVal130_41;
    glblVal130 = dupeArg(glblVal130, &glblVal130_41, 0);

    Term glblVal812_16;
    glblVal812 = dupeArg(glblVal812, &glblVal812_16, 0);

    // call vect-conj at intrp-rdr.toc: 110
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_14, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_41, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 110
    Term Rslt_1Args = pair_make(APP, 0, glblStr957, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 110
    Term glblRslt741Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt741 = term_new(VAR, 0, port(2, term_loc(glblRslt741Args)));
    store_redex(glblRslt741Args, glblVal812_16);
  }
  {
    Term glblRslt649_15;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_15, 0);

    Term glblVal130_42;
    glblVal130 = dupeArg(glblVal130, &glblVal130_42, 0);

    Term glblVal812_17;
    glblVal812 = dupeArg(glblVal812, &glblVal812_17, 0);

    // call vect-conj at intrp-rdr.toc: 111
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_15, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_42, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 111
    Term Rslt_1Args = pair_make(APP, 0, glblStr958, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 111
    Term glblRslt742Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt742 = term_new(VAR, 0, port(2, term_loc(glblRslt742Args)));
    store_redex(glblRslt742Args, glblVal812_17);
  }
  {
    Term glblRslt649_16;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_16, 0);

    Term glblVal130_43;
    glblVal130 = dupeArg(glblVal130, &glblVal130_43, 0);

    Term glblVal812_18;
    glblVal812 = dupeArg(glblVal812, &glblVal812_18, 0);

    // call vect-conj at intrp-rdr.toc: 114
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_16, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_43, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 114
    Term Rslt_1Args = pair_make(APP, 0, glblStr959, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 114
    Term glblRslt743Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt743 = term_new(VAR, 0, port(2, term_loc(glblRslt743Args)));
    store_redex(glblRslt743Args, glblVal812_18);
  }
  {
    Term glblRslt649_17;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_17, 0);

    Term glblVal130_44;
    glblVal130 = dupeArg(glblVal130, &glblVal130_44, 0);

    Term glblVal812_19;
    glblVal812 = dupeArg(glblVal812, &glblVal812_19, 0);

    // call vect-conj at intrp-rdr.toc: 117
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_17, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_44, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 117
    Term Rslt_1Args = pair_make(APP, 0, glblStr960, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 117
    Term glblRslt744Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt744 = term_new(VAR, 0, port(2, term_loc(glblRslt744Args)));
    store_redex(glblRslt744Args, glblVal812_19);
  }
  {
    Term glblRslt649_18;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_18, 0);

    Term glblVal130_45;
    glblVal130 = dupeArg(glblVal130, &glblVal130_45, 0);

    Term glblVal812_20;
    glblVal812 = dupeArg(glblVal812, &glblVal812_20, 0);

    // call vect-conj at intrp-rdr.toc: 118
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_18, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_45, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 118
    Term Rslt_1Args = pair_make(APP, 0, glblStr961, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 118
    Term glblRslt747Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt747 = term_new(VAR, 0, port(2, term_loc(glblRslt747Args)));
    store_redex(glblRslt747Args, glblVal812_20);
  }
  {
    Term glblRslt649_19;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_19, 0);

    Term glblVal130_46;
    glblVal130 = dupeArg(glblVal130, &glblVal130_46, 0);

    Term glblVal812_21;
    glblVal812 = dupeArg(glblVal812, &glblVal812_21, 0);

    // call vect-conj at intrp-rdr.toc: 121
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_19, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_46, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 121
    Term Rslt_1Args = pair_make(APP, 0, glblStr962, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 121
    Term glblRslt748Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt748 = term_new(VAR, 0, port(2, term_loc(glblRslt748Args)));
    store_redex(glblRslt748Args, glblVal812_21);
  }
  {
    Term glblRslt649_20;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_20, 0);

    Term glblVal130_47;
    glblVal130 = dupeArg(glblVal130, &glblVal130_47, 0);

    Term glblVal812_22;
    glblVal812 = dupeArg(glblVal812, &glblVal812_22, 0);

    // call vect-conj at intrp-rdr.toc: 122
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_20, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_47, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 122
    Term Rslt_1Args = pair_make(APP, 0, glblStr745, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 122
    Term glblRslt749Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt749 = term_new(VAR, 0, port(2, term_loc(glblRslt749Args)));
    store_redex(glblRslt749Args, glblVal812_22);
  }
  {
    Term glblRslt649_21;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_21, 0);

    Term glblVal130_48;
    glblVal130 = dupeArg(glblVal130, &glblVal130_48, 0);

    Term glblVal812_23;
    glblVal812 = dupeArg(glblVal812, &glblVal812_23, 0);

    // call vect-conj at intrp-rdr.toc: 123
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_21, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_48, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 123
    Term Rslt_1Args = pair_make(APP, 0, glblStr422, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 123
    Term glblRslt758Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt758 = term_new(VAR, 0, port(2, term_loc(glblRslt758Args)));
    store_redex(glblRslt758Args, glblVal812_23);
  }
  {
    Term glblRslt649_22;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_22, 0);

    Term glblVal130_49;
    glblVal130 = dupeArg(glblVal130, &glblVal130_49, 0);

    Term glblVal812_24;
    glblVal812 = dupeArg(glblVal812, &glblVal812_24, 0);

    // call vect-conj at intrp-rdr.toc: 124
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_22, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_49, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 124
    Term Rslt_1Args = pair_make(APP, 0, glblStr421, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 124
    Term glblRslt763Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt763 = term_new(VAR, 0, port(2, term_loc(glblRslt763Args)));
    store_redex(glblRslt763Args, glblVal812_24);
  }
  {
    Term glblRslt649_23;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_23, 0);

    Term glblVal130_50;
    glblVal130 = dupeArg(glblVal130, &glblVal130_50, 0);

    Term glblVal812_25;
    glblVal812 = dupeArg(glblVal812, &glblVal812_25, 0);

    // call vect-conj at intrp-rdr.toc: 125
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_23, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_50, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 125
    Term Rslt_1Args = pair_make(APP, 0, glblStr966, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 125
    Term glblRslt767Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt767 = term_new(VAR, 0, port(2, term_loc(glblRslt767Args)));
    store_redex(glblRslt767Args, glblVal812_25);
  }
  {
    Term glblRslt649_24;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_24, 0);

    Term glblVal130_51;
    glblVal130 = dupeArg(glblVal130, &glblVal130_51, 0);

    Term glblVal812_26;
    glblVal812 = dupeArg(glblVal812, &glblVal812_26, 0);

    // call vect-conj at intrp-rdr.toc: 126
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_24, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_51, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 126
    Term Rslt_1Args = pair_make(APP, 0, glblStr967, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 126
    Term glblRslt768Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt768 = term_new(VAR, 0, port(2, term_loc(glblRslt768Args)));
    store_redex(glblRslt768Args, glblVal812_26);
  }
  {
    Term glblRslt649_25;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_25, 0);

    Term glblVal130_52;
    glblVal130 = dupeArg(glblVal130, &glblVal130_52, 0);

    Term glblVal812_27;
    glblVal812 = dupeArg(glblVal812, &glblVal812_27, 0);

    // call vect-conj at intrp-rdr.toc: 129
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_25, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_52, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 129
    Term Rslt_1Args = pair_make(APP, 0, glblStr894, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 129
    Term glblRslt769Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt769 = term_new(VAR, 0, port(2, term_loc(glblRslt769Args)));
    store_redex(glblRslt769Args, glblVal812_27);
  }
  {
    Term glblRslt649_26;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_26, 0);

    Term glblVal130_53;
    glblVal130 = dupeArg(glblVal130, &glblVal130_53, 0);

    Term glblVal812_28;
    glblVal812 = dupeArg(glblVal812, &glblVal812_28, 0);

    // call vect-conj at intrp-rdr.toc: 132
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_26, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_53, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 132
    Term Rslt_1Args = pair_make(APP, 0, glblStr969, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 132
    Term glblRslt777Args = pair_make(APP, 0, Rslt_1, SUB);
    glblRslt777 = term_new(VAR, 0, port(2, term_loc(glblRslt777Args)));
    store_redex(glblRslt777Args, glblVal812_28);
  }
  {
    Term glblRslt600_1;
    glblRslt600 = dupeArg(glblRslt600, &glblRslt600_1, 0);

    Term glblVal832_4;
    glblVal832 = dupeArg(glblVal832, &glblVal832_4, 0);

    Term glblRslt649_27;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_27, 0);

    Term glblVal130_54;
    glblVal130 = dupeArg(glblVal130, &glblVal130_54, 0);

    Term glblVal812_29;
    glblVal812 = dupeArg(glblVal812, &glblVal812_29, 0);

    Term glblVal848_5;
    glblVal848 = dupeArg(glblVal848, &glblVal848_5, 0);

    // call vect-conj at intrp-rdr.toc: 136
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_27, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_54, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call Many at intrp-rdr.toc: 136
    Term Rslt_1Args = pair_make(APP, 0, glblRslt600_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal832_4);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal832_4));
#endif

    // call vect-conj at intrp-rdr.toc: 136
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_0, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 136
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
#ifdef STRICT
    store_redex(Rslt_3Args, glblVal812_29);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblVal812_29));
#endif

    // call Rule at intrp-rdr.toc: 136
    Term glblRslt782Args = pair_make(APP, 0, Rslt_3, SUB);
    glblRslt782 = term_new(VAR, 0, port(2, term_loc(glblRslt782Args)));
    glblRslt782Args = pair_make(APP, 0, glblStr971, glblRslt782Args);
    store_redex(glblRslt782Args, glblVal848_5);
  }
  {
    Term glblRslt604_1;
    glblRslt604 = dupeArg(glblRslt604, &glblRslt604_1, 0);

    Term glblRslt608_1;
    glblRslt608 = dupeArg(glblRslt608, &glblRslt608_1, 0);

    Term glblVal130_55;
    glblVal130 = dupeArg(glblVal130, &glblVal130_55, 0);

    Term glblVal822_4;
    glblVal822 = dupeArg(glblVal822, &glblVal822_4, 0);

    Term glblVal848_6;
    glblVal848 = dupeArg(glblVal848, &glblVal848_6, 0);

    // call vect-conj at intrp-rdr.toc: 137
    Term Rslt_0Args = pair_make(APP, 0, glblRslt608_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_55, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 137
    Term Rslt_1Args = pair_make(APP, 0, glblRslt604_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 137
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
#ifdef STRICT
    store_redex(Rslt_2Args, glblVal822_4);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblVal822_4));
#endif

    // call Rule at intrp-rdr.toc: 137
    Term glblRslt791Args = pair_make(APP, 0, Rslt_2, SUB);
    glblRslt791 = term_new(VAR, 0, port(2, term_loc(glblRslt791Args)));
    glblRslt791Args = pair_make(APP, 0, glblStr972, glblRslt791Args);
    store_redex(glblRslt791Args, glblVal848_6);
  }
  {
    Term glblRslt791_1;
    glblRslt791 = dupeArg(glblRslt791, &glblRslt791_1, 0);

    Term glblVal130_57;
    glblVal130 = dupeArg(glblVal130, &glblVal130_57, 0);

    Term glblVal822_5;
    glblVal822 = dupeArg(glblVal822, &glblVal822_5, 0);

    Term glblRslt649_28;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_28, 0);

    Term glblVal130_56;
    glblVal130 = dupeArg(glblVal130, &glblVal130_56, 0);

    Term glblVal812_30;
    glblVal812 = dupeArg(glblVal812, &glblVal812_30, 0);

    Term glblVal848_7;
    glblVal848 = dupeArg(glblVal848, &glblVal848_7, 0);

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_28, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_56, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_1Args = pair_make(APP, 0, glblRslt791_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, glblVal130_57, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_2Args = pair_make(APP, 0, glblStr982, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_3Args = pair_make(APP, 0, glblStr981, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_4Args = pair_make(APP, 0, glblStr980, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_5Args = pair_make(APP, 0, glblStr979, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
    Rslt_5Args = pair_make(APP, 0, Rslt_4, Rslt_5Args);
#ifdef STRICT
    store_redex(Rslt_5Args, glblFn133);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_6Args = pair_make(APP, 0, glblStr978, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
    Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
#ifdef STRICT
    store_redex(Rslt_6Args, glblFn133);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_7Args = pair_make(APP, 0, glblStr977, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_6, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_8Args = pair_make(APP, 0, glblStr976, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
    Rslt_8Args = pair_make(APP, 0, Rslt_7, Rslt_8Args);
#ifdef STRICT
    store_redex(Rslt_8Args, glblFn133);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_9Args = pair_make(APP, 0, glblStr975, SUB);
    Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
    Rslt_9Args = pair_make(APP, 0, Rslt_8, Rslt_9Args);
#ifdef STRICT
    store_redex(Rslt_9Args, glblFn133);
#else
    swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_10Args = pair_make(APP, 0, glblStr974, SUB);
    Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
    Rslt_10Args = pair_make(APP, 0, Rslt_9, Rslt_10Args);
#ifdef STRICT
    store_redex(Rslt_10Args, glblFn133);
#else
    swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 140
    Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
    Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
    store_redex(Rslt_11Args, glblVal822_5);
#else
    swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblVal822_5));
#endif

    // call vect-conj at intrp-rdr.toc: 140
    Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
    Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
    Rslt_12Args = pair_make(APP, 0, Rslt_0, Rslt_12Args);
#ifdef STRICT
    store_redex(Rslt_12Args, glblFn133);
#else
    swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 140
    Term Rslt_13Args = pair_make(APP, 0, Rslt_12, SUB);
    Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
#ifdef STRICT
    store_redex(Rslt_13Args, glblVal812_30);
#else
    swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblVal812_30));
#endif

    // call Rule at intrp-rdr.toc: 139
    Term glblRslt800Args = pair_make(APP, 0, Rslt_13, SUB);
    glblRslt800 = term_new(VAR, 0, port(2, term_loc(glblRslt800Args)));
    glblRslt800Args = pair_make(APP, 0, glblStr973, glblRslt800Args);
    store_redex(glblRslt800Args, glblVal848_7);
  }
  {
    Term glblRslt600_2;
    glblRslt600 = dupeArg(glblRslt600, &glblRslt600_2, 0);

    Term glblRslt791_2;
    glblRslt791 = dupeArg(glblRslt791, &glblRslt791_2, 0);

    Term glblVal130_58;
    glblVal130 = dupeArg(glblVal130, &glblVal130_58, 0);

    Term glblVal822_6;
    glblVal822 = dupeArg(glblVal822, &glblVal822_6, 0);

    Term glblVal848_8;
    glblVal848 = dupeArg(glblVal848, &glblVal848_8, 0);

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_0Args = pair_make(APP, 0, glblRslt791_2, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_58, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_1Args = pair_make(APP, 0, glblRslt600_2, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_2Args = pair_make(APP, 0, glblStr982, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_3Args = pair_make(APP, 0, glblStr981, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_4Args = pair_make(APP, 0, glblStr980, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_5Args = pair_make(APP, 0, glblStr979, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
    Rslt_5Args = pair_make(APP, 0, Rslt_4, Rslt_5Args);
#ifdef STRICT
    store_redex(Rslt_5Args, glblFn133);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_6Args = pair_make(APP, 0, glblStr978, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
    Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
#ifdef STRICT
    store_redex(Rslt_6Args, glblFn133);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_7Args = pair_make(APP, 0, glblStr977, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_6, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_8Args = pair_make(APP, 0, glblStr976, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
    Rslt_8Args = pair_make(APP, 0, Rslt_7, Rslt_8Args);
#ifdef STRICT
    store_redex(Rslt_8Args, glblFn133);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_9Args = pair_make(APP, 0, glblStr975, SUB);
    Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
    Rslt_9Args = pair_make(APP, 0, Rslt_8, Rslt_9Args);
#ifdef STRICT
    store_redex(Rslt_9Args, glblFn133);
#else
    swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_10Args = pair_make(APP, 0, glblStr985, SUB);
    Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
    Rslt_10Args = pair_make(APP, 0, Rslt_9, Rslt_10Args);
#ifdef STRICT
    store_redex(Rslt_10Args, glblFn133);
#else
    swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 143
    Term Rslt_11Args = pair_make(APP, 0, glblStr959, SUB);
    Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
    Rslt_11Args = pair_make(APP, 0, Rslt_10, Rslt_11Args);
#ifdef STRICT
    store_redex(Rslt_11Args, glblFn133);
#else
    swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 143
    Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
    Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
    store_redex(Rslt_12Args, glblVal822_6);
#else
    swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblVal822_6));
#endif

    // call Rule at intrp-rdr.toc: 142
    Term glblRslt803Args = pair_make(APP, 0, Rslt_12, SUB);
    glblRslt803 = term_new(VAR, 0, port(2, term_loc(glblRslt803Args)));
    glblRslt803Args = pair_make(APP, 0, glblStr983, glblRslt803Args);
    store_redex(glblRslt803Args, glblVal848_8);
  }
  {
    Term glblRslt803_1;
    glblRslt803 = dupeArg(glblRslt803, &glblRslt803_1, 0);

    Term glblVal832_5;
    glblVal832 = dupeArg(glblVal832, &glblVal832_5, 0);

    Term glblRslt800_1;
    glblRslt800 = dupeArg(glblRslt800, &glblRslt800_1, 0);

    Term glblVal130_59;
    glblVal130 = dupeArg(glblVal130, &glblVal130_59, 0);

    Term glblVal812_31;
    glblVal812 = dupeArg(glblVal812, &glblVal812_31, 0);

    Term glblVal848_9;
    glblVal848 = dupeArg(glblVal848, &glblVal848_9, 0);

    // call vect-conj at intrp-rdr.toc: 145
    Term Rslt_0Args = pair_make(APP, 0, glblRslt800_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_59, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call Many at intrp-rdr.toc: 145
    Term Rslt_1Args = pair_make(APP, 0, glblRslt803_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal832_5);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal832_5));
#endif

    // call vect-conj at intrp-rdr.toc: 145
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_0, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 145
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
#ifdef STRICT
    store_redex(Rslt_3Args, glblVal812_31);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblVal812_31));
#endif

    // call Rule at intrp-rdr.toc: 145
    Term glblRslt813Args = pair_make(APP, 0, Rslt_3, SUB);
    glblRslt813 = term_new(VAR, 0, port(2, term_loc(glblRslt813Args)));
    glblRslt813Args = pair_make(APP, 0, glblStr994, glblRslt813Args);
    store_redex(glblRslt813Args, glblVal848_9);
  }
  {
    Term glblRslt600_4;
    glblRslt600 = dupeArg(glblRslt600, &glblRslt600_4, 0);

    Term glblVal832_7;
    glblVal832 = dupeArg(glblVal832, &glblVal832_7, 0);

    Term glblRslt600_3;
    glblRslt600 = dupeArg(glblRslt600, &glblRslt600_3, 0);

    Term glblVal832_6;
    glblVal832 = dupeArg(glblVal832, &glblVal832_6, 0);

    Term glblRslt649_29;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_29, 0);

    Term glblVal130_61;
    glblVal130 = dupeArg(glblVal130, &glblVal130_61, 0);

    Term glblVal812_33;
    glblVal812 = dupeArg(glblVal812, &glblVal812_33, 0);

    Term glblVal130_60;
    glblVal130 = dupeArg(glblVal130, &glblVal130_60, 0);

    Term glblVal812_32;
    glblVal812 = dupeArg(glblVal812, &glblVal812_32, 0);

    Term glblVal848_10;
    glblVal848 = dupeArg(glblVal848, &glblVal848_10, 0);

    // call vect-conj at intrp-rdr.toc: 148
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_29, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_61, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call Many at intrp-rdr.toc: 148
    Term Rslt_1Args = pair_make(APP, 0, glblRslt600_3, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal832_6);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal832_6));
#endif

    // call vect-conj at intrp-rdr.toc: 148
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_0, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 148
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
#ifdef STRICT
    store_redex(Rslt_3Args, glblVal812_33);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblVal812_33));
#endif

    // call vect-conj at intrp-rdr.toc: 148
    Term Rslt_4Args = pair_make(APP, 0, Rslt_3, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, glblVal130_60, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 148
    Term Rslt_5Args = pair_make(APP, 0, glblStr982, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
    Rslt_5Args = pair_make(APP, 0, Rslt_4, Rslt_5Args);
#ifdef STRICT
    store_redex(Rslt_5Args, glblFn133);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn133));
#endif

    // call Many at intrp-rdr.toc: 148
    Term Rslt_6Args = pair_make(APP, 0, glblRslt600_4, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
    store_redex(Rslt_6Args, glblVal832_7);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblVal832_7));
#endif

    // call vect-conj at intrp-rdr.toc: 148
    Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_5, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 148
    Term Rslt_8Args = pair_make(APP, 0, Rslt_7, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
    store_redex(Rslt_8Args, glblVal812_32);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblVal812_32));
#endif

    // call Rule at intrp-rdr.toc: 148
    Term glblRslt823Args = pair_make(APP, 0, Rslt_8, SUB);
    glblRslt823 = term_new(VAR, 0, port(2, term_loc(glblRslt823Args)));
    glblRslt823Args = pair_make(APP, 0, glblStr995, glblRslt823Args);
    store_redex(glblRslt823Args, glblVal848_10);
  }
  {
    Term glblVal130_63;
    glblVal130 = dupeArg(glblVal130, &glblVal130_63, 0);

    Term glblVal822_7;
    glblVal822 = dupeArg(glblVal822, &glblVal822_7, 0);

    Term glblVal130_62;
    glblVal130 = dupeArg(glblVal130, &glblVal130_62, 0);

    Term glblVal812_34;
    glblVal812 = dupeArg(glblVal812, &glblVal812_34, 0);

    Term glblVal848_11;
    glblVal848 = dupeArg(glblVal848, &glblVal848_11, 0);

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_0Args = pair_make(APP, 0, glblStr998, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_62, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_1Args = pair_make(APP, 0, glblStr998, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, glblVal130_63, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_2Args = pair_make(APP, 0, glblStr970, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_3Args = pair_make(APP, 0, glblStr1001, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_4Args = pair_make(APP, 0, glblStr1000, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_5Args = pair_make(APP, 0, glblStr999, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
    Rslt_5Args = pair_make(APP, 0, Rslt_4, Rslt_5Args);
#ifdef STRICT
    store_redex(Rslt_5Args, glblFn133);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 151
    Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
    store_redex(Rslt_6Args, glblVal822_7);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblVal822_7));
#endif

    // call vect-conj at intrp-rdr.toc: 151
    Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_0, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 151
    Term Rslt_8Args = pair_make(APP, 0, Rslt_7, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
    store_redex(Rslt_8Args, glblVal812_34);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblVal812_34));
#endif

    // call Rule at intrp-rdr.toc: 151
    Term glblRslt833Args = pair_make(APP, 0, Rslt_8, SUB);
    glblRslt833 = term_new(VAR, 0, port(2, term_loc(glblRslt833Args)));
    glblRslt833Args = pair_make(APP, 0, glblStr997, glblRslt833Args);
    store_redex(glblRslt833Args, glblVal848_11);
  }
  {
    Term glblStr970_2;
    glblStr970 = dupeArg(glblStr970, &glblStr970_2, 0);

    Term glblRslt833_1;
    glblRslt833 = dupeArg(glblRslt833, &glblRslt833_1, 0);

    Term glblVal789_2;
    glblVal789 = dupeArg(glblVal789, &glblVal789_2, 0);

    Term glblVal130_65;
    glblVal130 = dupeArg(glblVal130, &glblVal130_65, 0);

    Term glblVal822_8;
    glblVal822 = dupeArg(glblVal822, &glblVal822_8, 0);

    Term glblVal832_8;
    glblVal832 = dupeArg(glblVal832, &glblVal832_8, 0);

    Term glblStr970_1;
    glblStr970 = dupeArg(glblStr970, &glblStr970_1, 0);

    Term glblRslt649_30;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_30, 0);

    Term glblVal130_64;
    glblVal130 = dupeArg(glblVal130, &glblVal130_64, 0);

    Term glblVal812_35;
    glblVal812 = dupeArg(glblVal812, &glblVal812_35, 0);

    Term glblVal848_12;
    glblVal848 = dupeArg(glblVal848, &glblVal848_12, 0);

    // call vect-conj at intrp-rdr.toc: 152
    Term Rslt_0Args = pair_make(APP, 0, glblRslt649_30, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_64, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 152
    Term Rslt_1Args = pair_make(APP, 0, glblStr970_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call NotChar at intrp-rdr.toc: 154
    Term Rslt_2Args = pair_make(APP, 0, glblStr970, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
#ifdef STRICT
    store_redex(Rslt_2Args, glblVal789_2);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblVal789_2));
#endif

    // call vect-conj at intrp-rdr.toc: 154
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, glblVal130_65, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 154
    Term Rslt_4Args = pair_make(APP, 0, glblRslt833_1, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 154
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal822_8);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal822_8));
#endif

    // call Many at intrp-rdr.toc: 154
    Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
    store_redex(Rslt_6Args, glblVal832_8);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblVal832_8));
#endif

    // call vect-conj at intrp-rdr.toc: 152
    Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_1, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 152
    Term Rslt_8Args = pair_make(APP, 0, glblStr970_2, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
    Rslt_8Args = pair_make(APP, 0, Rslt_7, Rslt_8Args);
#ifdef STRICT
    store_redex(Rslt_8Args, glblFn133);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 152
    Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
    Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
#ifdef STRICT
    store_redex(Rslt_9Args, glblVal812_35);
#else
    swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblVal812_35));
#endif

    // call Rule at intrp-rdr.toc: 152
    Term glblRslt834Args = pair_make(APP, 0, Rslt_9, SUB);
    glblRslt834 = term_new(VAR, 0, port(2, term_loc(glblRslt834Args)));
    glblRslt834Args = pair_make(APP, 0, glblStr1004, glblRslt834Args);
    store_redex(glblRslt834Args, glblVal848_12);
  }
  {
    Term glblRslt763_1;
    glblRslt763 = dupeArg(glblRslt763, &glblRslt763_1, 0);

    Term glblVal835_1;
    glblVal835 = dupeArg(glblVal835, &glblVal835_1, 0);

    Term glblVal861_1;
    glblVal861 = dupeArg(glblVal861, &glblVal861_1, 0);

    Term glblVal832_9;
    glblVal832 = dupeArg(glblVal832, &glblVal832_9, 0);

    Term glblRslt758_1;
    glblRslt758 = dupeArg(glblRslt758, &glblRslt758_1, 0);

    Term glblVal130_66;
    glblVal130 = dupeArg(glblVal130, &glblVal130_66, 0);

    Term glblVal812_36;
    glblVal812 = dupeArg(glblVal812, &glblVal812_36, 0);

    Term glblVal848_13;
    glblVal848 = dupeArg(glblVal848, &glblVal848_13, 0);

    // call vect-conj at intrp-rdr.toc: 164
    Term Rslt_0Args = pair_make(APP, 0, glblRslt758_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_66, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call Recur at intrp-rdr.toc: 164
    Term Rslt_1Args = pair_make(APP, 0, glblVal835_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal861_1);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal861_1));
#endif

    // call Many at intrp-rdr.toc: 164
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
#ifdef STRICT
    store_redex(Rslt_2Args, glblVal832_9);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblVal832_9));
#endif

    // call vect-conj at intrp-rdr.toc: 164
    Term Rslt_3Args = pair_make(APP, 0, Rslt_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_0, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 164
    Term Rslt_4Args = pair_make(APP, 0, glblRslt763_1, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 164
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_36);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_36));
#endif

    // call Rule at intrp-rdr.toc: 163
    Term glblRslt840Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt840 = term_new(VAR, 0, port(2, term_loc(glblRslt840Args)));
    glblRslt840Args = pair_make(APP, 0, glblStr1006, glblRslt840Args);
    store_redex(glblRslt840Args, glblVal848_13);
  }
  {
    Term glblRslt768_1;
    glblRslt768 = dupeArg(glblRslt768, &glblRslt768_1, 0);

    Term glblVal835_3;
    glblVal835 = dupeArg(glblVal835, &glblVal835_3, 0);

    Term glblVal861_3;
    glblVal861 = dupeArg(glblVal861, &glblVal861_3, 0);

    Term glblVal835_2;
    glblVal835 = dupeArg(glblVal835, &glblVal835_2, 0);

    Term glblVal861_2;
    glblVal861 = dupeArg(glblVal861, &glblVal861_2, 0);

    Term glblVal130_68;
    glblVal130 = dupeArg(glblVal130, &glblVal130_68, 0);

    Term glblVal812_38;
    glblVal812 = dupeArg(glblVal812, &glblVal812_38, 0);

    Term glblVal832_10;
    glblVal832 = dupeArg(glblVal832, &glblVal832_10, 0);

    Term glblRslt767_1;
    glblRslt767 = dupeArg(glblRslt767, &glblRslt767_1, 0);

    Term glblVal130_67;
    glblVal130 = dupeArg(glblVal130, &glblVal130_67, 0);

    Term glblVal812_37;
    glblVal812 = dupeArg(glblVal812, &glblVal812_37, 0);

    Term glblVal848_14;
    glblVal848 = dupeArg(glblVal848, &glblVal848_14, 0);

    // call vect-conj at intrp-rdr.toc: 168
    Term Rslt_0Args = pair_make(APP, 0, glblRslt767_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_67, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call Recur at intrp-rdr.toc: 168
    Term Rslt_1Args = pair_make(APP, 0, glblVal835_2, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
#ifdef STRICT
    store_redex(Rslt_1Args, glblVal861_2);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblVal861_2));
#endif

    // call vect-conj at intrp-rdr.toc: 168
    Term Rslt_2Args = pair_make(APP, 0, Rslt_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, glblVal130_68, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call Recur at intrp-rdr.toc: 168
    Term Rslt_3Args = pair_make(APP, 0, glblVal835_3, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
#ifdef STRICT
    store_redex(Rslt_3Args, glblVal861_3);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblVal861_3));
#endif

    // call vect-conj at intrp-rdr.toc: 168
    Term Rslt_4Args = pair_make(APP, 0, Rslt_3, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_2, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 168
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_38);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_38));
#endif

    // call Many at intrp-rdr.toc: 168
    Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
    store_redex(Rslt_6Args, glblVal832_10);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblVal832_10));
#endif

    // call vect-conj at intrp-rdr.toc: 168
    Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
    Rslt_7Args = pair_make(APP, 0, Rslt_0, Rslt_7Args);
#ifdef STRICT
    store_redex(Rslt_7Args, glblFn133);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 168
    Term Rslt_8Args = pair_make(APP, 0, glblRslt768_1, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
    Rslt_8Args = pair_make(APP, 0, Rslt_7, Rslt_8Args);
#ifdef STRICT
    store_redex(Rslt_8Args, glblFn133);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 168
    Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
    Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
#ifdef STRICT
    store_redex(Rslt_9Args, glblVal812_37);
#else
    swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblVal812_37));
#endif

    // call Rule at intrp-rdr.toc: 167
    Term glblRslt849Args = pair_make(APP, 0, Rslt_9, SUB);
    glblRslt849 = term_new(VAR, 0, port(2, term_loc(glblRslt849Args)));
    glblRslt849Args = pair_make(APP, 0, glblStr1007, glblRslt849Args);
    store_redex(glblRslt849Args, glblVal848_14);
  }
  {
    Term glblRslt849_1;
    glblRslt849 = dupeArg(glblRslt849, &glblRslt849_1, 0);

    Term glblRslt840_1;
    glblRslt840 = dupeArg(glblRslt840, &glblRslt840_1, 0);

    Term glblRslt749_1;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_1, 0);

    Term glblVal835_5;
    glblVal835 = dupeArg(glblVal835, &glblVal835_5, 0);

    Term glblVal861_5;
    glblVal861 = dupeArg(glblVal861, &glblVal861_5, 0);

    Term glblVal832_11;
    glblVal832 = dupeArg(glblVal832, &glblVal832_11, 0);

    Term glblVal835_4;
    glblVal835 = dupeArg(glblVal835, &glblVal835_4, 0);

    Term glblVal861_4;
    glblVal861 = dupeArg(glblVal861, &glblVal861_4, 0);

    Term glblRslt748_1;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_1, 0);

    Term glblVal130_70;
    glblVal130 = dupeArg(glblVal130, &glblVal130_70, 0);

    Term glblVal812_39;
    glblVal812 = dupeArg(glblVal812, &glblVal812_39, 0);

    Term glblRslt813_1;
    glblRslt813 = dupeArg(glblRslt813, &glblRslt813_1, 0);

    Term glblRslt834_1;
    glblRslt834 = dupeArg(glblRslt834, &glblRslt834_1, 0);

    Term glblRslt823_1;
    glblRslt823 = dupeArg(glblRslt823, &glblRslt823_1, 0);

    Term glblRslt782_1;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_1, 0);

    Term glblVal130_69;
    glblVal130 = dupeArg(glblVal130, &glblVal130_69, 0);

    Term glblVal822_9;
    glblVal822 = dupeArg(glblVal822, &glblVal822_9, 0);

    Term glblVal848_15;
    glblVal848 = dupeArg(glblVal848, &glblVal848_15, 0);

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_0Args = pair_make(APP, 0, glblRslt782_1, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_69, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_1Args = pair_make(APP, 0, glblRslt823_1, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_2Args = pair_make(APP, 0, glblRslt834_1, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_3Args = pair_make(APP, 0, glblRslt813_1, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 177
    Term Rslt_4Args = pair_make(APP, 0, glblRslt748_1, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, glblVal130_70, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call Recur at intrp-rdr.toc: 178
    Term Rslt_5Args = pair_make(APP, 0, glblVal835_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal861_4);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal861_4));
#endif

    // call vect-conj at intrp-rdr.toc: 177
    Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
    Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
    Rslt_6Args = pair_make(APP, 0, Rslt_4, Rslt_6Args);
#ifdef STRICT
    store_redex(Rslt_6Args, glblFn133);
#else
    swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn133));
#endif

    // call Recur at intrp-rdr.toc: 179
    Term Rslt_7Args = pair_make(APP, 0, glblVal835_5, SUB);
    Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
    store_redex(Rslt_7Args, glblVal861_5);
#else
    swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal861_5));
#endif

    // call Many at intrp-rdr.toc: 179
    Term Rslt_8Args = pair_make(APP, 0, Rslt_7, SUB);
    Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
    store_redex(Rslt_8Args, glblVal832_11);
#else
    swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblVal832_11));
#endif

    // call vect-conj at intrp-rdr.toc: 177
    Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
    Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
    Rslt_9Args = pair_make(APP, 0, Rslt_6, Rslt_9Args);
#ifdef STRICT
    store_redex(Rslt_9Args, glblFn133);
#else
    swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 177
    Term Rslt_10Args = pair_make(APP, 0, glblRslt749_1, SUB);
    Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
    Rslt_10Args = pair_make(APP, 0, Rslt_9, Rslt_10Args);
#ifdef STRICT
    store_redex(Rslt_10Args, glblFn133);
#else
    swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 177
    Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
    Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
    store_redex(Rslt_11Args, glblVal812_39);
#else
    swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblVal812_39));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
    Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
    Rslt_12Args = pair_make(APP, 0, Rslt_3, Rslt_12Args);
#ifdef STRICT
    store_redex(Rslt_12Args, glblFn133);
#else
    swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_13Args = pair_make(APP, 0, glblRslt840_1, SUB);
    Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
    Rslt_13Args = pair_make(APP, 0, Rslt_12, Rslt_13Args);
#ifdef STRICT
    store_redex(Rslt_13Args, glblFn133);
#else
    swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 173
    Term Rslt_14Args = pair_make(APP, 0, glblRslt849_1, SUB);
    Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
    Rslt_14Args = pair_make(APP, 0, Rslt_13, Rslt_14Args);
#ifdef STRICT
    store_redex(Rslt_14Args, glblFn133);
#else
    swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn133));
#endif

    // call Any at intrp-rdr.toc: 173
    Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
    Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
    store_redex(Rslt_15Args, glblVal822_9);
#else
    swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblVal822_9));
#endif

    // call Rule at intrp-rdr.toc: 172
    Term glblRslt867Args = pair_make(APP, 0, Rslt_15, SUB);
    glblRslt867 = term_new(VAR, 0, port(2, term_loc(glblRslt867Args)));
    glblRslt867Args = pair_make(APP, 0, glblStr1008, glblRslt867Args);
    store_redex(glblRslt867Args, glblVal848_15);
  }
  {
    Term glblRslt749_2;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_2, 0);

    Term glblRslt782_2;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_2, 0);

    Term glblRslt649_31;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_31, 0);

    Term glblRslt748_2;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_2, 0);

    Term glblVal130_71;
    glblVal130 = dupeArg(glblVal130, &glblVal130_71, 0);

    Term glblVal812_40;
    glblVal812 = dupeArg(glblVal812, &glblVal812_40, 0);

    Term glblVal848_16;
    glblVal848 = dupeArg(glblVal848, &glblVal848_16, 0);

    // call vect-conj at intrp-rdr.toc: 187
    Term Rslt_0Args = pair_make(APP, 0, glblRslt748_2, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_71, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 187
    Term Rslt_1Args = pair_make(APP, 0, glblRslt649_31, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 187
    Term Rslt_2Args = pair_make(APP, 0, glblStr1010, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 187
    Term Rslt_3Args = pair_make(APP, 0, glblRslt782_2, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 187
    Term Rslt_4Args = pair_make(APP, 0, glblRslt749_2, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 187
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_40);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_40));
#endif

    // call Rule at intrp-rdr.toc: 187
    Term glblRslt907Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt907 = term_new(VAR, 0, port(2, term_loc(glblRslt907Args)));
    glblRslt907Args = pair_make(APP, 0, glblStr1009, glblRslt907Args);
    store_redex(glblRslt907Args, glblVal848_16);
  }
  {
    Term glblRslt749_3;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_3, 0);

    Term glblRslt782_3;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_3, 0);

    Term glblRslt649_32;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_32, 0);

    Term glblRslt748_3;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_3, 0);

    Term glblVal130_72;
    glblVal130 = dupeArg(glblVal130, &glblVal130_72, 0);

    Term glblVal812_41;
    glblVal812 = dupeArg(glblVal812, &glblVal812_41, 0);

    Term glblVal848_17;
    glblVal848 = dupeArg(glblVal848, &glblVal848_17, 0);

    // call vect-conj at intrp-rdr.toc: 188
    Term Rslt_0Args = pair_make(APP, 0, glblRslt748_3, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_72, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 188
    Term Rslt_1Args = pair_make(APP, 0, glblRslt649_32, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 188
    Term Rslt_2Args = pair_make(APP, 0, glblStr1013, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 188
    Term Rslt_3Args = pair_make(APP, 0, glblRslt782_3, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 188
    Term Rslt_4Args = pair_make(APP, 0, glblRslt749_3, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 188
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_41);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_41));
#endif

    // call Rule at intrp-rdr.toc: 188
    Term glblRslt932Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt932 = term_new(VAR, 0, port(2, term_loc(glblRslt932Args)));
    glblRslt932Args = pair_make(APP, 0, glblStr1012, glblRslt932Args);
    store_redex(glblRslt932Args, glblVal848_17);
  }
  {
    Term glblRslt749_4;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_4, 0);

    Term glblRslt782_4;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_4, 0);

    Term glblRslt649_33;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_33, 0);

    Term glblRslt748_4;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_4, 0);

    Term glblVal130_73;
    glblVal130 = dupeArg(glblVal130, &glblVal130_73, 0);

    Term glblVal812_42;
    glblVal812 = dupeArg(glblVal812, &glblVal812_42, 0);

    Term glblVal848_18;
    glblVal848 = dupeArg(glblVal848, &glblVal848_18, 0);

    // call vect-conj at intrp-rdr.toc: 189
    Term Rslt_0Args = pair_make(APP, 0, glblRslt748_4, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_73, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 189
    Term Rslt_1Args = pair_make(APP, 0, glblRslt649_33, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 189
    Term Rslt_2Args = pair_make(APP, 0, glblStr1015, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 189
    Term Rslt_3Args = pair_make(APP, 0, glblRslt782_4, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 189
    Term Rslt_4Args = pair_make(APP, 0, glblRslt749_4, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 189
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_42);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_42));
#endif

    // call Rule at intrp-rdr.toc: 189
    Term glblRslt939Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt939 = term_new(VAR, 0, port(2, term_loc(glblRslt939Args)));
    glblRslt939Args = pair_make(APP, 0, glblStr1014, glblRslt939Args);
    store_redex(glblRslt939Args, glblVal848_18);
  }
  {
    Term glblRslt749_5;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_5, 0);

    Term glblRslt782_5;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_5, 0);

    Term glblRslt649_34;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_34, 0);

    Term glblRslt748_5;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_5, 0);

    Term glblVal130_74;
    glblVal130 = dupeArg(glblVal130, &glblVal130_74, 0);

    Term glblVal812_43;
    glblVal812 = dupeArg(glblVal812, &glblVal812_43, 0);

    Term glblVal848_19;
    glblVal848 = dupeArg(glblVal848, &glblVal848_19, 0);

    // call vect-conj at intrp-rdr.toc: 190
    Term Rslt_0Args = pair_make(APP, 0, glblRslt748_5, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_74, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 190
    Term Rslt_1Args = pair_make(APP, 0, glblRslt649_34, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 190
    Term Rslt_2Args = pair_make(APP, 0, glblStr1017, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 190
    Term Rslt_3Args = pair_make(APP, 0, glblRslt782_5, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 190
    Term Rslt_4Args = pair_make(APP, 0, glblRslt749_5, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 190
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_43);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_43));
#endif

    // call Rule at intrp-rdr.toc: 190
    Term glblRslt940Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt940 = term_new(VAR, 0, port(2, term_loc(glblRslt940Args)));
    glblRslt940Args = pair_make(APP, 0, glblStr1016, glblRslt940Args);
    store_redex(glblRslt940Args, glblVal848_19);
  }
  {
    Term glblRslt749_6;
    glblRslt749 = dupeArg(glblRslt749, &glblRslt749_6, 0);

    Term glblRslt782_6;
    glblRslt782 = dupeArg(glblRslt782, &glblRslt782_6, 0);

    Term glblRslt649_35;
    glblRslt649 = dupeArg(glblRslt649, &glblRslt649_35, 0);

    Term glblRslt748_6;
    glblRslt748 = dupeArg(glblRslt748, &glblRslt748_6, 0);

    Term glblVal130_75;
    glblVal130 = dupeArg(glblVal130, &glblVal130_75, 0);

    Term glblVal812_44;
    glblVal812 = dupeArg(glblVal812, &glblVal812_44, 0);

    Term glblVal848_20;
    glblVal848 = dupeArg(glblVal848, &glblVal848_20, 0);

    // call vect-conj at intrp-rdr.toc: 191
    Term Rslt_0Args = pair_make(APP, 0, glblRslt748_6, SUB);
    Term Rslt_0 = term_new(VAR, 0, port(2, term_loc(Rslt_0Args)));
    Rslt_0Args = pair_make(APP, 0, glblVal130_75, Rslt_0Args);
#ifdef STRICT
    store_redex(Rslt_0Args, glblFn133);
#else
    swapStore(term_loc(Rslt_0), pair_make(LAZ, 0, Rslt_0Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 191
    Term Rslt_1Args = pair_make(APP, 0, glblRslt649_35, SUB);
    Term Rslt_1 = term_new(VAR, 0, port(2, term_loc(Rslt_1Args)));
    Rslt_1Args = pair_make(APP, 0, Rslt_0, Rslt_1Args);
#ifdef STRICT
    store_redex(Rslt_1Args, glblFn133);
#else
    swapStore(term_loc(Rslt_1), pair_make(LAZ, 0, Rslt_1Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 191
    Term Rslt_2Args = pair_make(APP, 0, glblStr1019, SUB);
    Term Rslt_2 = term_new(VAR, 0, port(2, term_loc(Rslt_2Args)));
    Rslt_2Args = pair_make(APP, 0, Rslt_1, Rslt_2Args);
#ifdef STRICT
    store_redex(Rslt_2Args, glblFn133);
#else
    swapStore(term_loc(Rslt_2), pair_make(LAZ, 0, Rslt_2Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 191
    Term Rslt_3Args = pair_make(APP, 0, glblRslt782_6, SUB);
    Term Rslt_3 = term_new(VAR, 0, port(2, term_loc(Rslt_3Args)));
    Rslt_3Args = pair_make(APP, 0, Rslt_2, Rslt_3Args);
#ifdef STRICT
    store_redex(Rslt_3Args, glblFn133);
#else
    swapStore(term_loc(Rslt_3), pair_make(LAZ, 0, Rslt_3Args, glblFn133));
#endif

    // call vect-conj at intrp-rdr.toc: 191
    Term Rslt_4Args = pair_make(APP, 0, glblRslt749_6, SUB);
    Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
    Rslt_4Args = pair_make(APP, 0, Rslt_3, Rslt_4Args);
#ifdef STRICT
    store_redex(Rslt_4Args, glblFn133);
#else
    swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn133));
#endif

    // call All at intrp-rdr.toc: 191
    Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
    Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
    store_redex(Rslt_5Args, glblVal812_44);
#else
    swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblVal812_44));
#endif

    // call Rule at intrp-rdr.toc: 191
    Term glblRslt942Args = pair_make(APP, 0, Rslt_5, SUB);
    glblRslt942 = term_new(VAR, 0, port(2, term_loc(glblRslt942Args)));
    glblRslt942Args = pair_make(APP, 0, glblStr1018, glblRslt942Args);
    store_redex(glblRslt942Args, glblVal848_20);
  }
  normalize(NULL);
  if (term_tag(glblVal460) == VAR)
    glblVal460 = take(term_loc(glblVal460));
  if (term_tag(glblRslt436) == VAR)
    glblRslt436 = take(term_loc(glblRslt436));
  if (term_tag(glblVal437) == VAR)
    glblVal437 = take(term_loc(glblVal437));
  if (term_tag(glblVal354) == VAR)
    glblVal354 = take(term_loc(glblVal354));
  if (term_tag(glblVal279) == VAR)
    glblVal279 = take(term_loc(glblVal279));
  if (term_tag(glblRslt252) == VAR)
    glblRslt252 = take(term_loc(glblRslt252));
  if (term_tag(glblVal253) == VAR)
    glblVal253 = take(term_loc(glblVal253));
  if (term_tag(glblVal20) == VAR)
    glblVal20 = take(term_loc(glblVal20));
  if (term_tag(glblRslt12) == VAR)
    glblRslt12 = take(term_loc(glblRslt12));
  if (term_tag(glblVal13) == VAR)
    glblVal13 = take(term_loc(glblVal13));
  if (term_tag(glblRslt942) == VAR)
    glblRslt942 = take(term_loc(glblRslt942));
  if (term_tag(glblRslt940) == VAR)
    glblRslt940 = take(term_loc(glblRslt940));
  if (term_tag(glblRslt939) == VAR)
    glblRslt939 = take(term_loc(glblRslt939));
  if (term_tag(glblRslt932) == VAR)
    glblRslt932 = take(term_loc(glblRslt932));
  if (term_tag(glblRslt907) == VAR)
    glblRslt907 = take(term_loc(glblRslt907));
  if (term_tag(glblRslt867) == VAR)
    glblRslt867 = take(term_loc(glblRslt867));
  if (term_tag(glblRslt849) == VAR)
    glblRslt849 = take(term_loc(glblRslt849));
  if (term_tag(glblRslt840) == VAR)
    glblRslt840 = take(term_loc(glblRslt840));
  if (term_tag(glblRslt834) == VAR)
    glblRslt834 = take(term_loc(glblRslt834));
  if (term_tag(glblRslt833) == VAR)
    glblRslt833 = take(term_loc(glblRslt833));
  if (term_tag(glblRslt823) == VAR)
    glblRslt823 = take(term_loc(glblRslt823));
  if (term_tag(glblRslt813) == VAR)
    glblRslt813 = take(term_loc(glblRslt813));
  if (term_tag(glblRslt803) == VAR)
    glblRslt803 = take(term_loc(glblRslt803));
  if (term_tag(glblRslt800) == VAR)
    glblRslt800 = take(term_loc(glblRslt800));
  if (term_tag(glblRslt791) == VAR)
    glblRslt791 = take(term_loc(glblRslt791));
  if (term_tag(glblRslt782) == VAR)
    glblRslt782 = take(term_loc(glblRslt782));
  if (term_tag(glblRslt777) == VAR)
    glblRslt777 = take(term_loc(glblRslt777));
  if (term_tag(glblRslt769) == VAR)
    glblRslt769 = take(term_loc(glblRslt769));
  if (term_tag(glblRslt768) == VAR)
    glblRslt768 = take(term_loc(glblRslt768));
  if (term_tag(glblRslt767) == VAR)
    glblRslt767 = take(term_loc(glblRslt767));
  if (term_tag(glblRslt763) == VAR)
    glblRslt763 = take(term_loc(glblRslt763));
  if (term_tag(glblRslt758) == VAR)
    glblRslt758 = take(term_loc(glblRslt758));
  if (term_tag(glblRslt749) == VAR)
    glblRslt749 = take(term_loc(glblRslt749));
  if (term_tag(glblRslt748) == VAR)
    glblRslt748 = take(term_loc(glblRslt748));
  if (term_tag(glblRslt747) == VAR)
    glblRslt747 = take(term_loc(glblRslt747));
  if (term_tag(glblRslt744) == VAR)
    glblRslt744 = take(term_loc(glblRslt744));
  if (term_tag(glblRslt743) == VAR)
    glblRslt743 = take(term_loc(glblRslt743));
  if (term_tag(glblRslt742) == VAR)
    glblRslt742 = take(term_loc(glblRslt742));
  if (term_tag(glblRslt741) == VAR)
    glblRslt741 = take(term_loc(glblRslt741));
  if (term_tag(glblRslt736) == VAR)
    glblRslt736 = take(term_loc(glblRslt736));
  if (term_tag(glblRslt727) == VAR)
    glblRslt727 = take(term_loc(glblRslt727));
  if (term_tag(glblRslt718) == VAR)
    glblRslt718 = take(term_loc(glblRslt718));
  if (term_tag(glblRslt708) == VAR)
    glblRslt708 = take(term_loc(glblRslt708));
  if (term_tag(glblRslt696) == VAR)
    glblRslt696 = take(term_loc(glblRslt696));
  if (term_tag(glblRslt680) == VAR)
    glblRslt680 = take(term_loc(glblRslt680));
  if (term_tag(glblRslt675) == VAR)
    glblRslt675 = take(term_loc(glblRslt675));
  if (term_tag(glblRslt672) == VAR)
    glblRslt672 = take(term_loc(glblRslt672));
  if (term_tag(glblRslt669) == VAR)
    glblRslt669 = take(term_loc(glblRslt669));
  if (term_tag(glblRslt664) == VAR)
    glblRslt664 = take(term_loc(glblRslt664));
  if (term_tag(glblRslt662) == VAR)
    glblRslt662 = take(term_loc(glblRslt662));
  if (term_tag(glblRslt658) == VAR)
    glblRslt658 = take(term_loc(glblRslt658));
  if (term_tag(glblRslt655) == VAR)
    glblRslt655 = take(term_loc(glblRslt655));
  if (term_tag(glblRslt649) == VAR)
    glblRslt649 = take(term_loc(glblRslt649));
  if (term_tag(glblRslt648) == VAR)
    glblRslt648 = take(term_loc(glblRslt648));
  if (term_tag(glblRslt643) == VAR)
    glblRslt643 = take(term_loc(glblRslt643));
  if (term_tag(glblRslt638) == VAR)
    glblRslt638 = take(term_loc(glblRslt638));
  if (term_tag(glblRslt608) == VAR)
    glblRslt608 = take(term_loc(glblRslt608));
  if (term_tag(glblRslt604) == VAR)
    glblRslt604 = take(term_loc(glblRslt604));
  if (term_tag(glblRslt600) == VAR)
    glblRslt600 = take(term_loc(glblRslt600));
  if (term_tag(glblVal861) == VAR)
    glblVal861 = take(term_loc(glblVal861));
  if (term_tag(glblVal848) == VAR)
    glblVal848 = take(term_loc(glblVal848));
  if (term_tag(glblVal832) == VAR)
    glblVal832 = take(term_loc(glblVal832));
  if (term_tag(glblVal822) == VAR)
    glblVal822 = take(term_loc(glblVal822));
  if (term_tag(glblVal812) == VAR)
    glblVal812 = take(term_loc(glblVal812));
  if (term_tag(glblVal799) == VAR)
    glblVal799 = take(term_loc(glblVal799));
  if (term_tag(glblVal789) == VAR)
    glblVal789 = take(term_loc(glblVal789));
  if (term_tag(glblVal776) == VAR)
    glblVal776 = take(term_loc(glblVal776));
  if (term_tag(glblVal762) == VAR)
    glblVal762 = take(term_loc(glblVal762));
  if (term_tag(glblVal740) == VAR)
    glblVal740 = take(term_loc(glblVal740));
  if (term_tag(glblVal724) == VAR)
    glblVal724 = take(term_loc(glblVal724));
  if (term_tag(glblVal717) == VAR)
    glblVal717 = take(term_loc(glblVal717));
  if (term_tag(glblVal714) == VAR)
    glblVal714 = take(term_loc(glblVal714));
  if (term_tag(glblRslt706) == VAR)
    glblRslt706 = take(term_loc(glblRslt706));
  if (term_tag(glblVal707) == VAR)
    glblVal707 = take(term_loc(glblVal707));
  if (term_tag(glblVal699) == VAR)
    glblVal699 = take(term_loc(glblVal699));
  if (term_tag(glblVal681) == VAR)
    glblVal681 = take(term_loc(glblVal681));
  if (term_tag(glblVal674) == VAR)
    glblVal674 = take(term_loc(glblVal674));
  if (term_tag(glblVal663) == VAR)
    glblVal663 = take(term_loc(glblVal663));
  if (term_tag(glblVal654) == VAR)
    glblVal654 = take(term_loc(glblVal654));
  if (term_tag(glblVal647) == VAR)
    glblVal647 = take(term_loc(glblVal647));
  if (term_tag(glblVal637) == VAR)
    glblVal637 = take(term_loc(glblVal637));
  if (term_tag(glblVal607) == VAR)
    glblVal607 = take(term_loc(glblVal607));
  if (term_tag(glblVal601) == VAR)
    glblVal601 = take(term_loc(glblVal601));
  if (term_tag(glblVal589) == VAR)
    glblVal589 = take(term_loc(glblVal589));
  if (term_tag(glblVal583) == VAR)
    glblVal583 = take(term_loc(glblVal583));
  if (term_tag(glblVal575) == VAR)
    glblVal575 = take(term_loc(glblVal575));
  if (term_tag(glblVal570) == VAR)
    glblVal570 = take(term_loc(glblVal570));
  if (term_tag(glblVal561) == VAR)
    glblVal561 = take(term_loc(glblVal561));
  if (term_tag(glblVal553) == VAR)
    glblVal553 = take(term_loc(glblVal553));
  if (term_tag(glblVal549) == VAR)
    glblVal549 = take(term_loc(glblVal549));
  if (term_tag(glblVal545) == VAR)
    glblVal545 = take(term_loc(glblVal545));
  if (term_tag(glblVal539) == VAR)
    glblVal539 = take(term_loc(glblVal539));
  if (term_tag(glblVal527) == VAR)
    glblVal527 = take(term_loc(glblVal527));
  if (term_tag(glblVal515) == VAR)
    glblVal515 = take(term_loc(glblVal515));
  if (term_tag(glblVal508) == VAR)
    glblVal508 = take(term_loc(glblVal508));
}
void freeGlobals() {
  freeGlobal(glblVal460);
  freeGlobal(glblRslt436);
  freeGlobal(glblVal437);
  freeGlobal(glblVal354);
  freeGlobal(glblVal279);
  freeGlobal(glblRslt252);
  freeGlobal(glblVal253);
  freeGlobal(glblVal20);
  freeGlobal(glblRslt12);
  freeGlobal(glblVal13);
  freeGlobal(glblRslt942);
  freeGlobal(glblRslt940);
  freeGlobal(glblRslt939);
  freeGlobal(glblRslt932);
  freeGlobal(glblRslt907);
  freeGlobal(glblRslt867);
  freeGlobal(glblRslt849);
  freeGlobal(glblRslt840);
  freeGlobal(glblRslt834);
  freeGlobal(glblRslt833);
  freeGlobal(glblRslt823);
  freeGlobal(glblRslt813);
  freeGlobal(glblRslt803);
  freeGlobal(glblRslt800);
  freeGlobal(glblRslt791);
  freeGlobal(glblRslt782);
  freeGlobal(glblRslt777);
  freeGlobal(glblRslt769);
  freeGlobal(glblRslt768);
  freeGlobal(glblRslt767);
  freeGlobal(glblRslt763);
  freeGlobal(glblRslt758);
  freeGlobal(glblRslt749);
  freeGlobal(glblRslt748);
  freeGlobal(glblRslt747);
  freeGlobal(glblRslt744);
  freeGlobal(glblRslt743);
  freeGlobal(glblRslt742);
  freeGlobal(glblRslt741);
  freeGlobal(glblRslt736);
  freeGlobal(glblRslt727);
  freeGlobal(glblRslt718);
  freeGlobal(glblRslt708);
  freeGlobal(glblRslt696);
  freeGlobal(glblRslt680);
  freeGlobal(glblRslt675);
  freeGlobal(glblRslt672);
  freeGlobal(glblRslt669);
  freeGlobal(glblRslt664);
  freeGlobal(glblRslt662);
  freeGlobal(glblRslt658);
  freeGlobal(glblRslt655);
  freeGlobal(glblRslt649);
  freeGlobal(glblRslt648);
  freeGlobal(glblRslt643);
  freeGlobal(glblRslt638);
  freeGlobal(glblRslt608);
  freeGlobal(glblRslt604);
  freeGlobal(glblRslt600);
  freeGlobal(glblVal861);
  freeGlobal(glblVal848);
  freeGlobal(glblVal832);
  freeGlobal(glblVal822);
  freeGlobal(glblVal812);
  freeGlobal(glblVal799);
  freeGlobal(glblVal789);
  freeGlobal(glblVal776);
  freeGlobal(glblVal762);
  freeGlobal(glblVal740);
  freeGlobal(glblVal724);
  freeGlobal(glblVal717);
  freeGlobal(glblVal714);
  freeGlobal(glblRslt706);
  freeGlobal(glblVal707);
  freeGlobal(glblVal699);
  freeGlobal(glblVal681);
  freeGlobal(glblVal674);
  freeGlobal(glblVal663);
  freeGlobal(glblVal654);
  freeGlobal(glblVal647);
  freeGlobal(glblVal637);
  freeGlobal(glblVal607);
  freeGlobal(glblVal601);
  freeGlobal(glblVal589);
  freeGlobal(glblVal583);
  freeGlobal(glblVal575);
  freeGlobal(glblVal570);
  freeGlobal(glblVal561);
  freeGlobal(glblVal553);
  freeGlobal(glblVal549);
  freeGlobal(glblVal545);
  freeGlobal(glblVal539);
  freeGlobal(glblVal527);
  freeGlobal(glblVal515);
  freeGlobal(glblVal508);
  normalize(NULL);
  check_buff();
}
Term mainFn = new_ref(glblCFn944);
int typeCount = 83;
