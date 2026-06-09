
#define _XOPEN_SOURCE 600
#include "graph.h"
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
    if (termTag(x_1) == I60) {
      result = termVal((Term)stringValue("Integer"));
    } else {
      char *name = typeName(((Value *)x_1)->type);
      result = termVal((Term)stringValue(name));
    };
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn1 = newRef(glblCFn0);
Term glbltype_name6;
void glblglbl5(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_5 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_5 = portLoc(2, seq_5);
  Term x_4 = newTerm(VAR, 0, portLoc(1, seq_5));
  // allocate args at core: 39
  // call default-type-name at core: 41
  Term Rslt_7Args = makePair(APP, 0, x_4, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
#ifdef STRICT
  pushRedex(Rslt_7Args, glblFn1);
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, glblFn1));
#endif

  dupLabels[0] = "type-name";
  // link args to body
  swap(r_seq_5, Rslt_7);

  Term Rslt_7V = get(termLoc(Rslt_7));
  apps = take(portLoc(1, Rslt_7V));
  lams = take(portLoc(2, Rslt_7V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_7), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_5);
  return;
}
Term glbltype_name6 = newRef(glblglbl5);
Term glblProto3;
Term glblFn8;
void glblCFn7(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term x_1 = arityArgs.args[0];

    switch (termTag(x_1)) {
    case I60:
      result = newI60(IntegerType);
      break;

    case F60:
      result = newI60(FloatType);
      break;

    case REF:
      BOOM("too tire");
      break;

    default:
      result = newI60(((Value *)x_1)->type);
      dec_and_free(x_1, 1);
    }
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn8 = newRef(glblCFn7);
Term glblVal10 = newRef(graphFn);
;
Term glblVal10;
Term glblProto16;
void glblSome19(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(1), args);
  callArgs = makePair(APP, 0, newI60(43), callArgs);
  interact(callArgs, construct);
  return;
}
void glblNone12(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(0), args);
  callArgs = makePair(APP, 0, newI60(42), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal14 = newRef(glblNone12);
Term glblVal14;
Term glblRslt13;
Term glblVal22 = newRef(glblSome19);
Term glblVal22;
Term glblFld21;
void glblFldFn20(Term ref, Term args) {
  args = makePair(APP, 0, newI60(0), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld21 = newRef(glblFldFn20);
Term glblProto15;
Term glblProto23;
Term glblProto25;
Term glblProto27;
Term glblProto29;
Term glblProto31;
Term glblProto33;
Term glblProto35;
Term glblProto37;
Term glblProto39;
Term glblProto41;
Term glblProto43;
Term glblProto45;
Term glblProto47;
Term glblProto49;
Term glblProto51;
Term glblProto53;
Term glblProto55;
Term glblProto57;
Term glblProto59;
Term glblProto61;
Term glblProto63;
Term glblProto65;
Term glblProto67;
Term glblProto69;
Term glblProto71;
Term glblProto73;
Term glblProto75;
Term glblProto77;
Term glblProto79;
Term glblProto81;
Term glblProto83;
Term glblProto85;
Term glblProto87;
Term glblProto89;
Term glblProto91;
Term glblProto93;
Term glblProto95;
Term glblProto97;
Term glblProto99;
Term glblProto101;
Term glblProto103;
Term glblProto105;
Term glblProto107;
Term glblProto109;
Term glblFn112;
void glblCFn111(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term str_1 = arityArgs.args[0];

    if (termTag(str_1) != VAL) {
      fprintf(stderr, "\ninvalid type for 'abort': Integer or Float\n");
      abort();
    }
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      fprintf(stderr, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    } else {
      fprintf(stderr, "\ninvalid type for 'abort': %ld\n", s->type);
      abort();
    }
    abort();
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn112 = newRef(glblCFn111);
Term glblFn115;
void glblCFn114(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term str_1 = arityArgs.args[0];

    if (termTag(str_1) != VAL) {
      fprintf(stderr, "\ninvalid type for 'pr*': Integer or Float\n");
      abort();
    }
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      fprintf(stdout, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    } else if (s->type == SubStringType) {
      ReifiedVal *ss = (ReifiedVal *)str_1;
      String *parent = (String *)ss->impls[0];
      long start = getI60(ss->impls[1]);
      int len = (int)getI60(ss->impls[2]);
      fprintf(stdout, "%-.*s", len, &parent->buffer[start]);
    } else {
      fprintf(stdout, "\ninvalid type for 'pr*': %ld\n", s->type);
      abort();
    }
    dec_and_free(str_1, 1);
    result = newI60(0);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn115 = newRef(glblCFn114);
Term glblFn118;
void glblCFn117(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    if (termTag(x_1) != I60 && termTag(y_2) != I60) {
      BOOM("Invalid inputs to '+'");
    }
    result = newI60(getI60(x_1) + getI60(y_2));

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn118 = newRef(glblCFn117);
Term glblFn120;
void glblCFn119(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 9, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 265
  Term Rslt_4 = makeOp(OP_ADD, newI60(1), x_1);
  dupLabels[9] = "";
  // link args to body
  swap(r_seq_2, Rslt_4);

  Term Rslt_4V = get(termLoc(Rslt_4));
  apps = take(portLoc(1, Rslt_4V));
  lams = take(portLoc(2, Rslt_4V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_4), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn120 = newRef(glblCFn119);
Term glblFn124;
void glblCFn123(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    fprintf(stderr, "'-' should be optimized away\n");
    abort();

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn124 = newRef(glblCFn123);
Term glblFn127;
void glblCFn126(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 11, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 277
  Term Rslt_4 = makeOp(OP_SUB, x_1, newI60(1));
  dupLabels[11] = "";
  // link args to body
  swap(r_seq_2, Rslt_4);

  Term Rslt_4V = get(termLoc(Rslt_4));
  apps = take(portLoc(1, Rslt_4V));
  lams = take(portLoc(2, Rslt_4V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_4), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn127 = newRef(glblCFn126);
Term glblFn130;
void glblCFn129(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    fprintf(stderr, "'*' should be optimized away\n");
    abort();

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn130 = newRef(glblCFn129);
Term glblVal131 = (Term)&empty_vect_struct;
Term glblVal131;
Term glblFn134;
void glblCFn133(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term vect_1 = arityArgs.args[0];
    Term v_2 = arityArgs.args[1];

    result = termVal((Term)vectConj((Vector *)vect_1, v_2));
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn134 = newRef(glblCFn133);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[5];
} str0 = {StringBufferType, REFS_STATIC, 0, 0, 4, "core"};
Term glblStr151 = newTerm_(VAL, (Term)&str0);
void glblProto15_150(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(297), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
Term glbl_EQ_149;
void glbl_EQ_148(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_8 = makePair(LAM, 20, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term y_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 20, SUB, seq_8);
  Term x_6 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at core: 296
  // call type-num at core: 297
  Term Rslt_10Args = makePair(APP, 0, x_6, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
#ifdef STRICT
  pushRedex(Rslt_10Args, glblFn8);
#else
  swap(termLoc(Rslt_10), makePair(LAZ, 0, Rslt_10Args, glblFn8));
#endif

  // call type-num at core: 297
  Term Rslt_11Args = makePair(APP, 0, y_7, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
#ifdef STRICT
  pushRedex(Rslt_11Args, glblFn8);
#else
  swap(termLoc(Rslt_11), makePair(LAZ, 0, Rslt_11Args, glblFn8));
#endif

  // call = at core: 297
  Term Rslt_12Args = makePair(APP, 0, Rslt_11, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
  Rslt_12Args = makePair(APP, 0, Rslt_10, Rslt_12Args);
#ifdef STRICT
  pushRedex(Rslt_12Args, newRef(glblProto15_150));
#else
  swap(termLoc(Rslt_12),
       makePair(LAZ, 0, Rslt_12Args, newRef(glblProto15_150)));
#endif
  dupLabels[20] = "=";
  // link args to body
  swap(r_seq_8, Rslt_12);

  Term Rslt_12V = get(termLoc(Rslt_12));
  apps = take(portLoc(1, Rslt_12V));
  lams = take(portLoc(2, Rslt_12V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_12), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_8);
  return;
}
Term glbl_EQ_149 = newRef(glbl_EQ_148);
Term glblrecurse147;
void glblrecurse146(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_16 = makePair(LAM, 19, SUB, NUL);
  Location r_seq_16 = portLoc(2, seq_16);
  Term f_15 = newTerm(VAR, 0, portLoc(1, seq_16));
  seq_16 = makePair(LAM, 19, SUB, seq_16);
  Term v_14 = newTerm(VAR, 0, portLoc(1, seq_16));
  // allocate args at core: 298
  swap(termLoc(f_15), ERA);
  dupLabels[19] = "recurse";
  // link args to body
  swap(r_seq_16, v_14);

  V = get(r_seq_16);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 19, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_16, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_16);
  return;
}
Term glblrecurse147 = newRef(glblrecurse146);
Term glbleither145;
void glbleither144(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = makePair(LAM, 18, SUB, NUL);
  Location r_seq_21 = portLoc(2, seq_21);
  Term cont_20 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 18, SUB, seq_21);
  Term v_19 = newTerm(VAR, 0, portLoc(1, seq_21));
  // allocate args at core: 300
  swap(termLoc(v_19), ERA);
  dupLabels[18] = "either";
  // link args to body
  swap(r_seq_21, cont_20);

  V = get(r_seq_21);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 18, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_21, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_21);
  return;
}
Term glbleither145 = newRef(glbleither144);
Term glblcond143;
void glblcond142(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_27 = makePair(LAM, 17, SUB, NUL);
  Location r_seq_27 = portLoc(2, seq_27);
  Term cont_26 = newTerm(VAR, 0, portLoc(1, seq_27));
  seq_27 = makePair(LAM, 17, SUB, seq_27);
  Term x_25 = newTerm(VAR, 0, portLoc(1, seq_27));
  seq_27 = makePair(LAM, 17, SUB, seq_27);
  Term v_24 = newTerm(VAR, 0, portLoc(1, seq_27));
  // allocate args at core: 302
  swap(termLoc(v_24), ERA);
  swap(termLoc(x_25), ERA);
  dupLabels[17] = "cond";
  // link args to body
  swap(r_seq_27, cont_26);

  V = get(r_seq_27);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 17, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_27, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_27);
  return;
}
Term glblcond143 = newRef(glblcond142);
Term glbland141;
void glbland140(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_32 = makePair(LAM, 16, SUB, NUL);
  Location r_seq_32 = portLoc(2, seq_32);
  Term arg_31 = newTerm(VAR, 0, portLoc(1, seq_32));
  seq_32 = makePair(LAM, 16, SUB, seq_32);
  Term x_30 = newTerm(VAR, 0, portLoc(1, seq_32));
  // allocate args at core: 304
  swap(termLoc(arg_31), ERA);
  dupLabels[16] = "and";
  // link args to body
  swap(r_seq_32, x_30);

  V = get(r_seq_32);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 16, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_32, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_32);
  return;
}
Term glbland141 = newRef(glbland140);
Term glblor139;
void glblor138(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_37 = makePair(LAM, 15, SUB, NUL);
  Location r_seq_37 = portLoc(2, seq_37);
  Term y_36 = newTerm(VAR, 0, portLoc(1, seq_37));
  seq_37 = makePair(LAM, 15, SUB, seq_37);
  Term arg_35 = newTerm(VAR, 0, portLoc(1, seq_37));
  // allocate args at core: 306
  swap(termLoc(arg_35), ERA);
  dupLabels[15] = "or";
  // link args to body
  swap(r_seq_37, y_36);

  V = get(r_seq_37);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 15, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_37, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_37);
  return;
}
Term glblor139 = newRef(glblor138);
Term glblmap137;
void glblmap136(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_42 = makePair(LAM, 14, SUB, NUL);
  Location r_seq_42 = portLoc(2, seq_42);
  Term f_41 = newTerm(VAR, 0, portLoc(1, seq_42));
  seq_42 = makePair(LAM, 14, SUB, seq_42);
  Term x_40 = newTerm(VAR, 0, portLoc(1, seq_42));
  // allocate args at core: 308
  swap(termLoc(f_41), ERA);
  dupLabels[14] = "map";
  // link args to body
  swap(r_seq_42, x_40);

  V = get(r_seq_42);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 14, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_42, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_42);
  return;
}
Term glblmap137 = newRef(glblmap136);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str1 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\n"};
Term glblStr161 = newTerm_(VAL, (Term)&str1);
void glblProto65_167(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(313), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto107_169(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(315), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto107);
  }
}
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str2 = {StringBufferType, REFS_STATIC, 0, 0, 1, " "};
Term glblStr162 = newTerm_(VAL, (Term)&str2);
void glblProto65_171(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(315), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto65_173(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(317), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
Term glblc164;
void glblc163(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_15 = makePair(LAM, 22, SUB, NUL);
  Location r_seq_15 = portLoc(2, seq_15);
  Term s_14 = newTerm(VAR, 0, portLoc(1, seq_15));
  seq_15 = makePair(LAM, 22, SUB, seq_15);
  Term v_13 = newTerm(VAR, 0, portLoc(1, seq_15));
  // allocate args at core: 316
  // call pr* at core: 317
  Term Rslt_17Args = makePair(APP, 0, s_14, SUB);
  Term Rslt_17 = newTerm(VAR, 0, portLoc(2, Rslt_17Args));
#ifdef STRICT
  pushRedex(Rslt_17Args, glblFn115);
#else
  swap(termLoc(Rslt_17), makePair(LAZ, 0, Rslt_17Args, glblFn115));
#endif

  // call conj at core: 317
  Term Rslt_18Args = makePair(APP, 0, Rslt_17, SUB);
  Term Rslt_18 = newTerm(VAR, 0, portLoc(2, Rslt_18Args));
  Rslt_18Args = makePair(APP, 0, v_13, Rslt_18Args);
#ifdef STRICT
  pushRedex(Rslt_18Args, newRef(glblProto65_173));
#else
  swap(termLoc(Rslt_18),
       makePair(LAZ, 0, Rslt_18Args, newRef(glblProto65_173)));
#endif
  dupLabels[22] = "";
  // link args to body
  swap(r_seq_15, Rslt_18);

  Term Rslt_18V = get(termLoc(Rslt_18));
  apps = take(portLoc(1, Rslt_18V));
  lams = take(portLoc(2, Rslt_18V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_18), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_15);
  return;
}
Term glblc164 = newRef(glblc163);
void glblProto69_176(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(315), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto69);
  }
}
void glblProto65_178(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(315), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
Term glblc166;
void glblc165(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_8 = makePair(LAM, 23, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term val_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 23, SUB, seq_8);
  Term v_6 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at core: 314
  Term glblVal131_2;
  glblVal131 = dupeArg(glblVal131, &glblVal131_2, 0);

  // call str-vect at core: 315
  Term Rslt_10Args = makePair(APP, 0, val_7, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
#ifdef STRICT
  pushRedex(Rslt_10Args, newRef(glblProto107_169));
#else
  swap(termLoc(Rslt_10),
       makePair(LAZ, 0, Rslt_10Args, newRef(glblProto107_169)));
#endif
  // call conj at core: 315
  Term Rslt_11Args = makePair(APP, 0, glblStr162, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
  Rslt_11Args = makePair(APP, 0, Rslt_10, Rslt_11Args);
#ifdef STRICT
  pushRedex(Rslt_11Args, newRef(glblProto65_171));
#else
  swap(termLoc(Rslt_11),
       makePair(LAZ, 0, Rslt_11Args, newRef(glblProto65_171)));
#endif
  // call reduce at core: 315
  Term Rslt_19Args = makePair(APP, 0, glblc164, SUB);
  Term Rslt_19 = newTerm(VAR, 0, portLoc(2, Rslt_19Args));
  Rslt_19Args = makePair(APP, 0, glblVal131_2, Rslt_19Args);
  Rslt_19Args = makePair(APP, 0, Rslt_11, Rslt_19Args);
#ifdef STRICT
  pushRedex(Rslt_19Args, newRef(glblProto69_176));
#else
  swap(termLoc(Rslt_19),
       makePair(LAZ, 0, Rslt_19Args, newRef(glblProto69_176)));
#endif
  // call conj at core: 315
  Term Rslt_20Args = makePair(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = newTerm(VAR, 0, portLoc(2, Rslt_20Args));
  Rslt_20Args = makePair(APP, 0, v_6, Rslt_20Args);
#ifdef STRICT
  pushRedex(Rslt_20Args, newRef(glblProto65_178));
#else
  swap(termLoc(Rslt_20),
       makePair(LAZ, 0, Rslt_20Args, newRef(glblProto65_178)));
#endif
  dupLabels[23] = "";
  // link args to body
  swap(r_seq_8, Rslt_20);

  Term Rslt_20V = get(termLoc(Rslt_20));
  apps = take(portLoc(1, Rslt_20V));
  lams = take(portLoc(2, Rslt_20V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_20), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_8);
  return;
}
Term glblc166 = newRef(glblc165);
void glblProto69_181(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(313), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto69);
  }
}
Term glblFn160;
void glblCFn159(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 21, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term vs_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 311
  Term glblVal131_1;
  glblVal131 = dupeArg(glblVal131, &glblVal131_1, 0);

  // call conj at core: 313
  Term Rslt_4Args = makePair(APP, 0, glblStr161, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
  Rslt_4Args = makePair(APP, 0, vs_1, Rslt_4Args);
#ifdef STRICT
  pushRedex(Rslt_4Args, newRef(glblProto65_167));
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, newRef(glblProto65_167)));
#endif
  // call reduce at core: 313
  Term Rslt_21Args = makePair(APP, 0, glblc166, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
  Rslt_21Args = makePair(APP, 0, glblVal131_1, Rslt_21Args);
  Rslt_21Args = makePair(APP, 0, Rslt_4, Rslt_21Args);
#ifdef STRICT
  pushRedex(Rslt_21Args, newRef(glblProto69_181));
#else
  swap(termLoc(Rslt_21),
       makePair(LAZ, 0, Rslt_21Args, newRef(glblProto69_181)));
#endif
  dupLabels[21] = "";
  // link args to body
  swap(r_seq_2, Rslt_21);

  Term Rslt_21V = get(termLoc(Rslt_21));
  apps = take(portLoc(1, Rslt_21V));
  lams = take(portLoc(2, Rslt_21V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_21), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn160 = newRef(glblCFn159);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str3 = {StringBufferType, REFS_STATIC, 0, 0, 6, "(Some "};
Term glblStr207 = newTerm_(VAL, (Term)&str3);
void glblProto16_215(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(322), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto107_217(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(322), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto107);
  }
}
void glblProto35_219(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(322), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto35);
  }
}
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str4 = {StringBufferType, REFS_STATIC, 0, 0, 1, ")"};
Term glblStr206 = newTerm_(VAL, (Term)&str4);
Term glblstr_vect203;
void glblstr_vect202(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_24 = makePair(LAM, 33, SUB, NUL);
  Location r_seq_24 = portLoc(2, seq_24);
  Term s_23 = newTerm(VAR, 0, portLoc(1, seq_24));
  // allocate args at core: 320
  Term glblVal131_3;
  glblVal131 = dupeArg(glblVal131, &glblVal131_3, 0);

  // call vect-conj at core: 321
  Term Rslt_26Args = makePair(APP, 0, glblStr207, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
  Rslt_26Args = makePair(APP, 0, glblVal131_3, Rslt_26Args);
#ifdef STRICT
  pushRedex(Rslt_26Args, glblFn134);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, glblFn134));
#endif

  // call .x at core: 322
  Term Rslt_27Args = makePair(APP, 0, s_23, SUB);
  Term Rslt_27 = newTerm(VAR, 0, portLoc(2, Rslt_27Args));
#ifdef STRICT
  pushRedex(Rslt_27Args, newRef(glblProto16_215));
#else
  swap(termLoc(Rslt_27),
       makePair(LAZ, 0, Rslt_27Args, newRef(glblProto16_215)));
#endif
  // call str-vect at core: 322
  Term Rslt_28Args = makePair(APP, 0, Rslt_27, SUB);
  Term Rslt_28 = newTerm(VAR, 0, portLoc(2, Rslt_28Args));
#ifdef STRICT
  pushRedex(Rslt_28Args, newRef(glblProto107_217));
#else
  swap(termLoc(Rslt_28),
       makePair(LAZ, 0, Rslt_28Args, newRef(glblProto107_217)));
#endif
  // call comp at core: 322
  Term Rslt_29Args = makePair(APP, 0, Rslt_28, SUB);
  Term Rslt_29 = newTerm(VAR, 0, portLoc(2, Rslt_29Args));
  Rslt_29Args = makePair(APP, 0, Rslt_26, Rslt_29Args);
#ifdef STRICT
  pushRedex(Rslt_29Args, newRef(glblProto35_219));
#else
  swap(termLoc(Rslt_29),
       makePair(LAZ, 0, Rslt_29Args, newRef(glblProto35_219)));
#endif
  // call vect-conj at core: 323
  Term Rslt_30Args = makePair(APP, 0, glblStr206, SUB);
  Term Rslt_30 = newTerm(VAR, 0, portLoc(2, Rslt_30Args));
  Rslt_30Args = makePair(APP, 0, Rslt_29, Rslt_30Args);
#ifdef STRICT
  pushRedex(Rslt_30Args, glblFn134);
#else
  swap(termLoc(Rslt_30), makePair(LAZ, 0, Rslt_30Args, glblFn134));
#endif

  dupLabels[33] = "str-vect";
  // link args to body
  swap(r_seq_24, Rslt_30);

  Term Rslt_30V = get(termLoc(Rslt_30));
  apps = take(portLoc(1, Rslt_30V));
  lams = take(portLoc(2, Rslt_30V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_30), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_24);
  return;
}
Term glblstr_vect203 = newRef(glblstr_vect202);
void glblProto15_222(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(325), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto16_224(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(326), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto16_226(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(326), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto15_228(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(326), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto25_230(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(325), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
Term glbl_EQ_201;
void glbl_EQ_200(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_34 = makePair(LAM, 32, SUB, NUL);
  Location r_seq_34 = portLoc(2, seq_34);
  Term y_33 = newTerm(VAR, 0, portLoc(1, seq_34));
  seq_34 = makePair(LAM, 32, SUB, seq_34);
  Term x_32 = newTerm(VAR, 0, portLoc(1, seq_34));
  // allocate args at core: 324

  Term x_32_1;
  x_32 = dupeArg(x_32, &x_32_1, 32);

  Term y_33_1;
  y_33 = dupeArg(y_33, &y_33_1, 32);
  // call type-num at core: 325
  Term Rslt_36Args = makePair(APP, 0, x_32, SUB);
  Term Rslt_36 = newTerm(VAR, 0, portLoc(2, Rslt_36Args));
#ifdef STRICT
  pushRedex(Rslt_36Args, glblFn8);
#else
  swap(termLoc(Rslt_36), makePair(LAZ, 0, Rslt_36Args, glblFn8));
#endif

  // call type-num at core: 325
  Term Rslt_37Args = makePair(APP, 0, y_33, SUB);
  Term Rslt_37 = newTerm(VAR, 0, portLoc(2, Rslt_37Args));
#ifdef STRICT
  pushRedex(Rslt_37Args, glblFn8);
#else
  swap(termLoc(Rslt_37), makePair(LAZ, 0, Rslt_37Args, glblFn8));
#endif

  // call = at core: 325
  Term Rslt_38Args = makePair(APP, 0, Rslt_37, SUB);
  Term Rslt_38 = newTerm(VAR, 0, portLoc(2, Rslt_38Args));
  Rslt_38Args = makePair(APP, 0, Rslt_36, Rslt_38Args);
#ifdef STRICT
  pushRedex(Rslt_38Args, newRef(glblProto15_222));
#else
  swap(termLoc(Rslt_38),
       makePair(LAZ, 0, Rslt_38Args, newRef(glblProto15_222)));
#endif
  // call .x at core: 326
  Term Rslt_39Args = makePair(APP, 0, x_32_1, SUB);
  Term Rslt_39 = newTerm(VAR, 0, portLoc(2, Rslt_39Args));
#ifdef STRICT
  pushRedex(Rslt_39Args, newRef(glblProto16_224));
#else
  swap(termLoc(Rslt_39),
       makePair(LAZ, 0, Rslt_39Args, newRef(glblProto16_224)));
#endif
  // call .x at core: 326
  Term Rslt_40Args = makePair(APP, 0, y_33_1, SUB);
  Term Rslt_40 = newTerm(VAR, 0, portLoc(2, Rslt_40Args));
#ifdef STRICT
  pushRedex(Rslt_40Args, newRef(glblProto16_226));
#else
  swap(termLoc(Rslt_40),
       makePair(LAZ, 0, Rslt_40Args, newRef(glblProto16_226)));
#endif
  // call = at core: 326
  Term Rslt_41Args = makePair(APP, 0, Rslt_40, SUB);
  Term Rslt_41 = newTerm(VAR, 0, portLoc(2, Rslt_41Args));
  Rslt_41Args = makePair(APP, 0, Rslt_39, Rslt_41Args);
#ifdef STRICT
  pushRedex(Rslt_41Args, newRef(glblProto15_228));
#else
  swap(termLoc(Rslt_41),
       makePair(LAZ, 0, Rslt_41Args, newRef(glblProto15_228)));
#endif
  // call and at core: 325
  Term Rslt_42Args = makePair(APP, 0, Rslt_41, SUB);
  Term Rslt_42 = newTerm(VAR, 0, portLoc(2, Rslt_42Args));
  Rslt_42Args = makePair(APP, 0, Rslt_38, Rslt_42Args);
#ifdef STRICT
  pushRedex(Rslt_42Args, newRef(glblProto25_230));
#else
  swap(termLoc(Rslt_42),
       makePair(LAZ, 0, Rslt_42Args, newRef(glblProto25_230)));
#endif
  dupLabels[32] = "=";
  // link args to body
  swap(r_seq_34, Rslt_42);

  Term Rslt_42V = get(termLoc(Rslt_42));
  apps = take(portLoc(1, Rslt_42V));
  lams = take(portLoc(2, Rslt_42V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_42), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_34);
  return;
}
Term glbl_EQ_201 = newRef(glbl_EQ_200);
void glblProto16_233(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(328), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
Term glblextract199;
void glblextract198(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_45 = makePair(LAM, 31, SUB, NUL);
  Location r_seq_45 = portLoc(2, seq_45);
  Term v_44 = newTerm(VAR, 0, portLoc(1, seq_45));
  // allocate args at core: 327
  // call .x at core: 328
  Term Rslt_47Args = makePair(APP, 0, v_44, SUB);
  Term Rslt_47 = newTerm(VAR, 0, portLoc(2, Rslt_47Args));
#ifdef STRICT
  pushRedex(Rslt_47Args, newRef(glblProto16_233));
#else
  swap(termLoc(Rslt_47),
       makePair(LAZ, 0, Rslt_47Args, newRef(glblProto16_233)));
#endif
  dupLabels[31] = "extract";
  // link args to body
  swap(r_seq_45, Rslt_47);

  Term Rslt_47V = get(termLoc(Rslt_47));
  apps = take(portLoc(1, Rslt_47V));
  lams = take(portLoc(2, Rslt_47V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_47), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_45);
  return;
}
Term glblextract199 = newRef(glblextract198);
void glblProto16_236(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(330), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
Term glblrecurse197;
void glblrecurse196(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_51 = makePair(LAM, 30, SUB, NUL);
  Location r_seq_51 = portLoc(2, seq_51);
  Term f_50 = newTerm(VAR, 0, portLoc(1, seq_51));
  seq_51 = makePair(LAM, 30, SUB, seq_51);
  Term v_49 = newTerm(VAR, 0, portLoc(1, seq_51));
  // allocate args at core: 329
  Term glblVal22_1;
  glblVal22 = dupeArg(glblVal22, &glblVal22_1, 0);

  // call .x at core: 330
  Term Rslt_53Args = makePair(APP, 0, v_49, SUB);
  Term Rslt_53 = newTerm(VAR, 0, portLoc(2, Rslt_53Args));
#ifdef STRICT
  pushRedex(Rslt_53Args, newRef(glblProto16_236));
#else
  swap(termLoc(Rslt_53),
       makePair(LAZ, 0, Rslt_53Args, newRef(glblProto16_236)));
#endif
  // call f at core: 329
  Term Rslt_54Args = makePair(APP, 0, Rslt_53, SUB);
  Term Rslt_54 = newTerm(VAR, 0, portLoc(2, Rslt_54Args));
#ifdef STRICT
  pushRedex(Rslt_54Args, f_50);
#else
  swap(termLoc(Rslt_54), makePair(LAZ, 0, Rslt_54Args, f_50));
#endif

  // call Some at core: 330
  Term Rslt_55Args = makePair(APP, 0, Rslt_54, SUB);
  Term Rslt_55 = newTerm(VAR, 0, portLoc(2, Rslt_55Args));
#ifdef STRICT
  pushRedex(Rslt_55Args, glblVal22_1);
#else
  swap(termLoc(Rslt_55), makePair(LAZ, 0, Rslt_55Args, glblVal22_1));
#endif

  dupLabels[30] = "recurse";
  // link args to body
  swap(r_seq_51, Rslt_55);

  Term Rslt_55V = get(termLoc(Rslt_55));
  apps = take(portLoc(1, Rslt_55V));
  lams = take(portLoc(2, Rslt_55V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_55), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_51);
  return;
}
Term glblrecurse197 = newRef(glblrecurse196);
void glblProto16_239(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(332), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
Term glbleither195;
void glbleither194(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_59 = makePair(LAM, 29, SUB, NUL);
  Location r_seq_59 = portLoc(2, seq_59);
  Term arg_58 = newTerm(VAR, 0, portLoc(1, seq_59));
  seq_59 = makePair(LAM, 29, SUB, seq_59);
  Term v_57 = newTerm(VAR, 0, portLoc(1, seq_59));
  // allocate args at core: 331
  swap(termLoc(arg_58), ERA);
  // call .x at core: 332
  Term Rslt_61Args = makePair(APP, 0, v_57, SUB);
  Term Rslt_61 = newTerm(VAR, 0, portLoc(2, Rslt_61Args));
#ifdef STRICT
  pushRedex(Rslt_61Args, newRef(glblProto16_239));
#else
  swap(termLoc(Rslt_61),
       makePair(LAZ, 0, Rslt_61Args, newRef(glblProto16_239)));
#endif
  dupLabels[29] = "either";
  // link args to body
  swap(r_seq_59, Rslt_61);

  Term Rslt_61V = get(termLoc(Rslt_61));
  apps = take(portLoc(1, Rslt_61V));
  lams = take(portLoc(2, Rslt_61V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_61), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_59);
  return;
}
Term glbleither195 = newRef(glbleither194);
Term glblcond193;
void glblcond192(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_66 = makePair(LAM, 28, SUB, NUL);
  Location r_seq_66 = portLoc(2, seq_66);
  Term arg_65 = newTerm(VAR, 0, portLoc(1, seq_66));
  seq_66 = makePair(LAM, 28, SUB, seq_66);
  Term clause_64 = newTerm(VAR, 0, portLoc(1, seq_66));
  seq_66 = makePair(LAM, 28, SUB, seq_66);
  Term v_63 = newTerm(VAR, 0, portLoc(1, seq_66));
  // allocate args at core: 333
  swap(termLoc(v_63), ERA);
  swap(termLoc(arg_65), ERA);
  dupLabels[28] = "cond";
  // link args to body
  swap(r_seq_66, clause_64);

  V = get(r_seq_66);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 28, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_66, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_66);
  return;
}
Term glblcond193 = newRef(glblcond192);
Term glbland191;
void glbland190(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_71 = makePair(LAM, 27, SUB, NUL);
  Location r_seq_71 = portLoc(2, seq_71);
  Term y_70 = newTerm(VAR, 0, portLoc(1, seq_71));
  seq_71 = makePair(LAM, 27, SUB, seq_71);
  Term arg_69 = newTerm(VAR, 0, portLoc(1, seq_71));
  // allocate args at core: 335
  swap(termLoc(arg_69), ERA);
  dupLabels[27] = "and";
  // link args to body
  swap(r_seq_71, y_70);

  V = get(r_seq_71);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 27, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_71, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_71);
  return;
}
Term glbland191 = newRef(glbland190);
Term glblor189;
void glblor188(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_76 = makePair(LAM, 26, SUB, NUL);
  Location r_seq_76 = portLoc(2, seq_76);
  Term arg_75 = newTerm(VAR, 0, portLoc(1, seq_76));
  seq_76 = makePair(LAM, 26, SUB, seq_76);
  Term q_74 = newTerm(VAR, 0, portLoc(1, seq_76));
  // allocate args at core: 337
  swap(termLoc(arg_75), ERA);
  dupLabels[26] = "or";
  // link args to body
  swap(r_seq_76, q_74);

  V = get(r_seq_76);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 26, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_76, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_76);
  return;
}
Term glblor189 = newRef(glblor188);
void glblProto16_245(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(340), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
Term glblmap187;
void glblmap186(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_81 = makePair(LAM, 25, SUB, NUL);
  Location r_seq_81 = portLoc(2, seq_81);
  Term f_80 = newTerm(VAR, 0, portLoc(1, seq_81));
  seq_81 = makePair(LAM, 25, SUB, seq_81);
  Term v_79 = newTerm(VAR, 0, portLoc(1, seq_81));
  // allocate args at core: 339
  Term glblVal22_2;
  glblVal22 = dupeArg(glblVal22, &glblVal22_2, 0);

  // call .x at core: 340
  Term Rslt_83Args = makePair(APP, 0, v_79, SUB);
  Term Rslt_83 = newTerm(VAR, 0, portLoc(2, Rslt_83Args));
#ifdef STRICT
  pushRedex(Rslt_83Args, newRef(glblProto16_245));
#else
  swap(termLoc(Rslt_83),
       makePair(LAZ, 0, Rslt_83Args, newRef(glblProto16_245)));
#endif
  // call f at core: 339
  Term Rslt_84Args = makePair(APP, 0, Rslt_83, SUB);
  Term Rslt_84 = newTerm(VAR, 0, portLoc(2, Rslt_84Args));
#ifdef STRICT
  pushRedex(Rslt_84Args, f_80);
#else
  swap(termLoc(Rslt_84), makePair(LAZ, 0, Rslt_84Args, f_80));
#endif

  // call Some at core: 340
  Term Rslt_85Args = makePair(APP, 0, Rslt_84, SUB);
  Term Rslt_85 = newTerm(VAR, 0, portLoc(2, Rslt_85Args));
#ifdef STRICT
  pushRedex(Rslt_85Args, glblVal22_2);
#else
  swap(termLoc(Rslt_85), makePair(LAZ, 0, Rslt_85Args, glblVal22_2));
#endif

  dupLabels[25] = "map";
  // link args to body
  swap(r_seq_81, Rslt_85);

  Term Rslt_85V = get(termLoc(Rslt_85));
  apps = take(portLoc(1, Rslt_85V));
  lams = take(portLoc(2, Rslt_85V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_85), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_81);
  return;
}
Term glblmap187 = newRef(glblmap186);
void glblProto16_248(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(342), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
Term glblflat_map185;
void glblflat_map184(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_89 = makePair(LAM, 24, SUB, NUL);
  Location r_seq_89 = portLoc(2, seq_89);
  Term f_88 = newTerm(VAR, 0, portLoc(1, seq_89));
  seq_89 = makePair(LAM, 24, SUB, seq_89);
  Term v_87 = newTerm(VAR, 0, portLoc(1, seq_89));
  // allocate args at core: 341
  // call .x at core: 342
  Term Rslt_91Args = makePair(APP, 0, v_87, SUB);
  Term Rslt_91 = newTerm(VAR, 0, portLoc(2, Rslt_91Args));
#ifdef STRICT
  pushRedex(Rslt_91Args, newRef(glblProto16_248));
#else
  swap(termLoc(Rslt_91),
       makePair(LAZ, 0, Rslt_91Args, newRef(glblProto16_248)));
#endif
  // call f at core: 341
  Term Rslt_92Args = makePair(APP, 0, Rslt_91, SUB);
  Term Rslt_92 = newTerm(VAR, 0, portLoc(2, Rslt_92Args));
#ifdef STRICT
  pushRedex(Rslt_92Args, f_88);
#else
  swap(termLoc(Rslt_92), makePair(LAZ, 0, Rslt_92Args, f_88));
#endif

  dupLabels[24] = "flat-map";
  // link args to body
  swap(r_seq_89, Rslt_92);

  Term Rslt_92V = get(termLoc(Rslt_92));
  apps = take(portLoc(1, Rslt_92V));
  lams = take(portLoc(2, Rslt_92V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_92), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_89);
  return;
}
Term glblflat_map185 = newRef(glblflat_map184);
Term glblFn205;
void glblCFn204(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    result = termVal(integer_EQ(x_1, y_2));
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn205 = newRef(glblCFn204);
Term glblFn209;
void glblCFn208(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    result = termVal(integer_LT(x_1, y_2));
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn209 = newRef(glblCFn208);
Term glblFn211;
void glblCFn210(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term n_1 = arityArgs.args[0];

    if (termTag(n_1) == VAL) {
      fprintf(stderr, "\ninvalid type for 'number-str': Value\n");
      abort();
    }
    result = number_str(n_1);

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn211 = newRef(glblCFn210);
void glblProto27_254(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(367), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto27);
  }
}
Term glblFn213;
void glblCFn212(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 37, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term y_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 37, SUB, seq_3);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 366

  Term x_1_1;
  x_1 = dupeArg(x_1, &x_1_1, 37);

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 37);
  // call int-< at core: 367
  Term Rslt_5Args = makePair(APP, 0, y_2, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, x_1, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, glblFn209);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, glblFn209));
#endif

  // call int-= at core: 368
  Term Rslt_6Args = makePair(APP, 0, y_2_1, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
  Rslt_6Args = makePair(APP, 0, x_1_1, Rslt_6Args);
#ifdef STRICT
  pushRedex(Rslt_6Args, glblFn205);
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, glblFn205));
#endif

  // call or at core: 367
  Term Rslt_7Args = makePair(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
  Rslt_7Args = makePair(APP, 0, Rslt_5, Rslt_7Args);
#ifdef STRICT
  pushRedex(Rslt_7Args, newRef(glblProto27_254));
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, newRef(glblProto27_254)));
#endif
  dupLabels[37] = "";
  // link args to body
  swap(r_seq_3, Rslt_7);

  Term Rslt_7V = get(termLoc(Rslt_7));
  apps = take(portLoc(1, Rslt_7V));
  lams = take(portLoc(2, Rslt_7V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_7), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn213 = newRef(glblCFn212);
Term glblVal214 = newRef(intCond);
;
Term glblVal214;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str5 = {StringBufferType, REFS_STATIC, 0, 0, 7, "Integer"};
Term glblStr267 = newTerm_(VAL, (Term)&str5);
Term glbltype_name266;
void glbltype_name265(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 42, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term arg_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 373
  swap(termLoc(arg_1), ERA);
  dupLabels[42] = "type-name";
  // link args to body
  swap(r_seq_2, glblStr267);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 42, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glbltype_name266 = newRef(glbltype_name265);
Term glblstr_vect264;
void glblstr_vect263(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_6 = makePair(LAM, 41, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term n_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at core: 376
  Term glblVal131_4;
  glblVal131 = dupeArg(glblVal131, &glblVal131_4, 0);

  // call number-str at core: 377
  Term Rslt_8Args = makePair(APP, 0, n_5, SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, Rslt_8Args));
#ifdef STRICT
  pushRedex(Rslt_8Args, glblFn211);
#else
  swap(termLoc(Rslt_8), makePair(LAZ, 0, Rslt_8Args, glblFn211));
#endif

  // call vect-conj at core: 377
  Term Rslt_9Args = makePair(APP, 0, Rslt_8, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
  Rslt_9Args = makePair(APP, 0, glblVal131_4, Rslt_9Args);
#ifdef STRICT
  pushRedex(Rslt_9Args, glblFn134);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, glblFn134));
#endif

  dupLabels[41] = "str-vect";
  // link args to body
  swap(r_seq_6, Rslt_9);

  Term Rslt_9V = get(termLoc(Rslt_9));
  apps = take(portLoc(1, Rslt_9V));
  lams = take(portLoc(2, Rslt_9V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_9), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_6);
  return;
}
Term glblstr_vect264 = newRef(glblstr_vect263);
Term glbl_EQ_262;
void glbl_EQ_261(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_13 = makePair(LAM, 40, SUB, NUL);
  Location r_seq_13 = portLoc(2, seq_13);
  Term y_12 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 40, SUB, seq_13);
  Term x_11 = newTerm(VAR, 0, portLoc(1, seq_13));
  // allocate args at core: 379
  // call int-= at core: 380
  Term Rslt_15Args = makePair(APP, 0, y_12, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
  Rslt_15Args = makePair(APP, 0, x_11, Rslt_15Args);
#ifdef STRICT
  pushRedex(Rslt_15Args, glblFn205);
#else
  swap(termLoc(Rslt_15), makePair(LAZ, 0, Rslt_15Args, glblFn205));
#endif

  dupLabels[40] = "=";
  // link args to body
  swap(r_seq_13, Rslt_15);

  Term Rslt_15V = get(termLoc(Rslt_15));
  apps = take(portLoc(1, Rslt_15V));
  lams = take(portLoc(2, Rslt_15V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_15), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_13);
  return;
}
Term glbl_EQ_262 = newRef(glbl_EQ_261);
Term glbl_LT_260;
void glbl_LT_259(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_19 = makePair(LAM, 39, SUB, NUL);
  Location r_seq_19 = portLoc(2, seq_19);
  Term y_18 = newTerm(VAR, 0, portLoc(1, seq_19));
  seq_19 = makePair(LAM, 39, SUB, seq_19);
  Term x_17 = newTerm(VAR, 0, portLoc(1, seq_19));
  // allocate args at core: 382
  // call int-< at core: 383
  Term Rslt_21Args = makePair(APP, 0, y_18, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
  Rslt_21Args = makePair(APP, 0, x_17, Rslt_21Args);
#ifdef STRICT
  pushRedex(Rslt_21Args, glblFn209);
#else
  swap(termLoc(Rslt_21), makePair(LAZ, 0, Rslt_21Args, glblFn209));
#endif

  dupLabels[39] = "<";
  // link args to body
  swap(r_seq_19, Rslt_21);

  Term Rslt_21V = get(termLoc(Rslt_21));
  apps = take(portLoc(1, Rslt_21V));
  lams = take(portLoc(2, Rslt_21V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_21), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_19);
  return;
}
Term glbl_LT_260 = newRef(glbl_LT_259);
Term glblcond258;
void glblcond257(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_26 = makePair(LAM, 38, SUB, NUL);
  Location r_seq_26 = portLoc(2, seq_26);
  Term z_25 = newTerm(VAR, 0, portLoc(1, seq_26));
  seq_26 = makePair(LAM, 38, SUB, seq_26);
  Term non_z_24 = newTerm(VAR, 0, portLoc(1, seq_26));
  seq_26 = makePair(LAM, 38, SUB, seq_26);
  Term n_23 = newTerm(VAR, 0, portLoc(1, seq_26));
  // allocate args at core: 385
  Term glblVal214_1;
  glblVal214 = dupeArg(glblVal214, &glblVal214_1, 0);

  // call int-cond at core: 386
  Term Rslt_28Args = makePair(APP, 0, z_25, SUB);
  Term Rslt_28 = newTerm(VAR, 0, portLoc(2, Rslt_28Args));
  Rslt_28Args = makePair(APP, 0, non_z_24, Rslt_28Args);
  Rslt_28Args = makePair(APP, 0, n_23, Rslt_28Args);
#ifdef STRICT
  pushRedex(Rslt_28Args, glblVal214_1);
#else
  swap(termLoc(Rslt_28), makePair(LAZ, 0, Rslt_28Args, glblVal214_1));
#endif

  dupLabels[38] = "cond";
  // link args to body
  swap(r_seq_26, Rslt_28);

  Term Rslt_28V = get(termLoc(Rslt_28));
  apps = take(portLoc(1, Rslt_28V));
  lams = take(portLoc(2, Rslt_28V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_28), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_26);
  return;
}
Term glblcond258 = newRef(glblcond257);
void glblProto23_279(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(396), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto23);
  }
}
void glblProto29_281(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(396), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto29);
  }
}
Term glblFn269;
void glblCFn268(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 43, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term y_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 43, SUB, seq_3);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 392

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 43);
  // call < at core: 396
  Term Rslt_5Args = makePair(APP, 0, y_2, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, x_1, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, newRef(glblProto23_279));
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, newRef(glblProto23_279)));
#endif
  // call either at core: 396
  Term Rslt_6Args = makePair(APP, 0, y_2_1, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
  Rslt_6Args = makePair(APP, 0, Rslt_5, Rslt_6Args);
#ifdef STRICT
  pushRedex(Rslt_6Args, newRef(glblProto29_281));
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, newRef(glblProto29_281)));
#endif
  dupLabels[43] = "";
  // link args to body
  swap(r_seq_3, Rslt_6);

  Term Rslt_6V = get(termLoc(Rslt_6));
  apps = take(portLoc(1, Rslt_6V));
  lams = take(portLoc(2, Rslt_6V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_6), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn269 = newRef(glblCFn268);
Term glblFn271;
void glblCFn270(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term v_1 = arityArgs.args[0];

    Vector *v = (Vector *)((u64)v_1 & ~7);
    result = newI60(v->count);
    dec_and_free((Term)v, 1);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn271 = newRef(glblCFn270);
Term glblFn275;
void glblCFn274(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 45, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 406
  dupLabels[45] = "";
  // link args to body
  swap(r_seq_2, x_1);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 45, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblFn275 = newRef(glblCFn274);
void glblProto37_288(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(411), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto37);
  }
}
Term glblFn287;
void glblCFn286(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 46, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term f_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 46, SUB, seq_3);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 410

  Term f_2_1;
  f_2 = dupeArg(f_2, &f_2_1, 46);
  Term seq_7 = makePair(LAM, 47, SUB, NUL);
  Location r_seq_7 = portLoc(2, seq_7);
  Term v_6 = newTerm(VAR, 0, portLoc(1, seq_7));
  // allocate args at core: 411
  // call fold at core: 410
  Term Rslt_9Args = makePair(APP, 0, f_2_1, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
  Rslt_9Args = makePair(APP, 0, v_6, Rslt_9Args);
#ifdef STRICT
  pushRedex(Rslt_9Args, glblFn287);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, glblFn287));
#endif

  dupLabels[47] = "";
  // link args to body
  swap(r_seq_7, Rslt_9);

  Term Rslt_9V = get(termLoc(Rslt_9));
  apps = take(portLoc(1, Rslt_9V));
  lams = take(portLoc(2, Rslt_9V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_9), makePair(LAZ, 0, apps, lams));
  // call recurse at core: 411
  Term Rslt_10Args = makePair(APP, 0, seq_7, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
  Rslt_10Args = makePair(APP, 0, x_1, Rslt_10Args);
#ifdef STRICT
  pushRedex(Rslt_10Args, newRef(glblProto37_288));
#else
  swap(termLoc(Rslt_10),
       makePair(LAZ, 0, Rslt_10Args, newRef(glblProto37_288)));
#endif
  // call f at core: 410
  Term Rslt_12Args = makePair(APP, 0, Rslt_10, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
#ifdef STRICT
  pushRedex(Rslt_12Args, f_2);
#else
  swap(termLoc(Rslt_12), makePair(LAZ, 0, Rslt_12Args, f_2));
#endif

  dupLabels[46] = "";
  // link args to body
  swap(r_seq_3, Rslt_12);

  Term Rslt_12V = get(termLoc(Rslt_12));
  apps = take(portLoc(1, Rslt_12V));
  lams = take(portLoc(2, Rslt_12V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_12), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn287 = newRef(glblCFn286);
void glblLeaf295(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(0), args);
  callArgs = makePair(APP, 0, newI60(44), callArgs);
  interact(callArgs, construct);
  return;
}
void glblProto37_298(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(421), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto37);
  }
}
Term glblFn292;
void glblCFn291(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 48, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term f_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 48, SUB, seq_3);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 420

  Term f_2_1;
  f_2 = dupeArg(f_2, &f_2_1, 48);
  Term seq_8 = makePair(LAM, 50, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term v_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at core: 421
  // call f at core: 420
  Term Rslt_5Args = makePair(APP, 0, x_1, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
#ifdef STRICT
  pushRedex(Rslt_5Args, f_2);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, f_2));
#endif

  // call unfold at core: 420
  Term Rslt_10Args = makePair(APP, 0, f_2_1, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
  Rslt_10Args = makePair(APP, 0, v_7, Rslt_10Args);
#ifdef STRICT
  pushRedex(Rslt_10Args, glblFn292);
#else
  swap(termLoc(Rslt_10), makePair(LAZ, 0, Rslt_10Args, glblFn292));
#endif

  dupLabels[50] = "";
  // link args to body
  swap(r_seq_8, Rslt_10);

  Term Rslt_10V = get(termLoc(Rslt_10));
  apps = take(portLoc(1, Rslt_10V));
  lams = take(portLoc(2, Rslt_10V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_10), makePair(LAZ, 0, apps, lams));
  // call recurse at core: 421
  Term Rslt_11Args = makePair(APP, 0, seq_8, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
  Rslt_11Args = makePair(APP, 0, Rslt_5, Rslt_11Args);
#ifdef STRICT
  pushRedex(Rslt_11Args, newRef(glblProto37_298));
#else
  swap(termLoc(Rslt_11),
       makePair(LAZ, 0, Rslt_11Args, newRef(glblProto37_298)));
#endif
  dupLabels[48] = "";
  // link args to body
  swap(r_seq_3, Rslt_11);

  Term Rslt_11V = get(termLoc(Rslt_11));
  apps = take(portLoc(1, Rslt_11V));
  lams = take(portLoc(2, Rslt_11V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_11), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn292 = newRef(glblCFn291);
Term glblVal297 = newRef(glblLeaf295);
Term glblVal297;
Term glblRslt296;
Term glblFn294;
void glblCFn293(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term s_1 = arityArgs.args[0];

    result = strCount(s_1);
    dec_and_free(s_1, 1);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn294 = newRef(glblCFn293);
Term glblFn302;
void glblCFn301(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term len_1 = arityArgs.args[0];

    String *strVal = malloc_string(getI60(len_1));
    strVal->len = 0;
    strVal->buffer[0] = 0;
    dec_and_free(len_1, 1);
    result = termVal((Term)strVal);

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn302 = newRef(glblCFn301);
Term glblFn306;
void glblCFn305(Term ref, Term args) {
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
    result = termVal(dest_1);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn306 = newRef(glblCFn305);
Term glblProto307;
Term glblFn311;
void glblCFn310(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 4, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 4) {
    Term s_1 = arityArgs.args[0];
    Term start_2 = arityArgs.args[1];
    Term len_3 = arityArgs.args[2];
    Term tgt_4 = arityArgs.args[3];
    result = strEQ(s_1, start_2, len_3, tgt_4);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn311 = newRef(glblCFn310);
Term glblProto317;
Term glblProto315;
Term glblProto313;
void glblSubString327(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(3), args);
  callArgs = makePair(APP, 0, newI60(5), callArgs);
  interact(callArgs, construct);
  return;
}
Term glblVal334 = newRef(glblSubString327);
Term glblVal334;
Term glblstr_vect320;
void glblstr_vect319(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 54, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term s_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 473
  Term s_1Args = makePair(APP, 0, s_1, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, s_1Args));
  s_1Args = makePair(APP, 0, newI60(0), s_1Args);
  pushRedex(s_1Args, accessField);
  Term glblVal131_5;
  glblVal131 = dupeArg(glblVal131, &glblVal131_5, 0);

  // call vect-conj at core: 473
  Term Rslt_5Args = makePair(APP, 0, Rslt_4, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, glblVal131_5, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, glblFn134);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, glblFn134));
#endif

  dupLabels[54] = "str-vect";
  // link args to body
  swap(r_seq_2, Rslt_5);

  Term Rslt_5V = get(termLoc(Rslt_5));
  apps = take(portLoc(1, Rslt_5V));
  lams = take(portLoc(2, Rslt_5V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_5), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblstr_vect320 = newRef(glblstr_vect319);
Term glblcount322;
void glblcount321(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_8 = makePair(LAM, 55, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term arg_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at core: 474
  Term arg_7Args = makePair(APP, 0, arg_7, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, arg_7Args));
  arg_7Args = makePair(APP, 0, newI60(2), arg_7Args);
  pushRedex(arg_7Args, accessField);
  dupLabels[55] = "count";
  // link args to body
  swap(r_seq_8, Rslt_10);

  V = get(r_seq_8);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 55, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_8, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_8);
  return;
}
Term glblcount322 = newRef(glblcount321);
Term glbl_EQ_324;
void glbl_EQ_323(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_14 = makePair(LAM, 56, SUB, NUL);
  Location r_seq_14 = portLoc(2, seq_14);
  Term y_13 = newTerm(VAR, 0, portLoc(1, seq_14));
  seq_14 = makePair(LAM, 56, SUB, seq_14);
  Term x_12 = newTerm(VAR, 0, portLoc(1, seq_14));
  // allocate args at core: 476

  Term x_12_2;
  x_12 = dupeArg(x_12, &x_12_2, 56);

  Term x_12_1;
  x_12 = dupeArg(x_12, &x_12_1, 56);
  Term x_12_2Args = makePair(APP, 0, x_12_2, SUB);
  Term Rslt_18 = newTerm(VAR, 0, portLoc(2, x_12_2Args));
  x_12_2Args = makePair(APP, 0, newI60(2), x_12_2Args);
  pushRedex(x_12_2Args, accessField);
  Term x_12_1Args = makePair(APP, 0, x_12_1, SUB);
  Term Rslt_17 = newTerm(VAR, 0, portLoc(2, x_12_1Args));
  x_12_1Args = makePair(APP, 0, newI60(1), x_12_1Args);
  pushRedex(x_12_1Args, accessField);
  Term x_12Args = makePair(APP, 0, x_12, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, x_12Args));
  x_12Args = makePair(APP, 0, newI60(0), x_12Args);
  pushRedex(x_12Args, accessField);
  // call str= at core: 477
  Term Rslt_19Args = makePair(APP, 0, y_13, SUB);
  Term Rslt_19 = newTerm(VAR, 0, portLoc(2, Rslt_19Args));
  Rslt_19Args = makePair(APP, 0, Rslt_18, Rslt_19Args);
  Rslt_19Args = makePair(APP, 0, Rslt_17, Rslt_19Args);
  Rslt_19Args = makePair(APP, 0, Rslt_16, Rslt_19Args);
#ifdef STRICT
  pushRedex(Rslt_19Args, glblFn311);
#else
  swap(termLoc(Rslt_19), makePair(LAZ, 0, Rslt_19Args, glblFn311));
#endif

  dupLabels[56] = "=";
  // link args to body
  swap(r_seq_14, Rslt_19);

  Term Rslt_19V = get(termLoc(Rslt_19));
  apps = take(portLoc(1, Rslt_19V));
  lams = take(portLoc(2, Rslt_19V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_19), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_14);
  return;
}
Term glbl_EQ_324 = newRef(glbl_EQ_323);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[1];
} str6 = {StringBufferType, REFS_STATIC, 0, 0, 0, ""};
Term glblStr335 = newTerm_(VAL, (Term)&str6);
void glblProto31_339(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(481), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto31);
  }
}
void glblProto61_341(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(480), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
Term glblsubs326;
void glblsubs325(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_24 = makePair(LAM, 57, SUB, NUL);
  Location r_seq_24 = portLoc(2, seq_24);
  Term new_len_23 = newTerm(VAR, 0, portLoc(1, seq_24));
  seq_24 = makePair(LAM, 57, SUB, seq_24);
  Term new_start_22 = newTerm(VAR, 0, portLoc(1, seq_24));
  seq_24 = makePair(LAM, 57, SUB, seq_24);
  Term s_21 = newTerm(VAR, 0, portLoc(1, seq_24));
  // allocate args at core: 479

  Term s_21_4;
  s_21 = dupeArg(s_21, &s_21_4, 57);

  Term s_21_3;
  s_21 = dupeArg(s_21, &s_21_3, 57);

  Term s_21_2;
  s_21 = dupeArg(s_21, &s_21_2, 57);

  Term s_21_1;
  s_21 = dupeArg(s_21, &s_21_1, 57);
  Term s_21_4Args = makePair(APP, 0, s_21_4, SUB);
  Term Rslt_39 = newTerm(VAR, 0, portLoc(2, s_21_4Args));
  s_21_4Args = makePair(APP, 0, newI60(0), s_21_4Args);
  pushRedex(s_21_4Args, accessField);
  Term s_21_3Args = makePair(APP, 0, s_21_3, SUB);
  Term Rslt_37 = newTerm(VAR, 0, portLoc(2, s_21_3Args));
  s_21_3Args = makePair(APP, 0, newI60(2), s_21_3Args);
  pushRedex(s_21_3Args, accessField);
  Term s_21_2Args = makePair(APP, 0, s_21_2, SUB);
  Term Rslt_36 = newTerm(VAR, 0, portLoc(2, s_21_2Args));
  s_21_2Args = makePair(APP, 0, newI60(1), s_21_2Args);
  pushRedex(s_21_2Args, accessField);
  Term seq_28 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_28 = portLoc(2, seq_28);
  Term final_len_27 = newTerm(VAR, 0, portLoc(1, seq_28));
  // allocate args at core: 480

  Term final_len_27_1;
  final_len_27 = dupeArg(final_len_27, &final_len_27_1, 0);
  Term s_21_1Args = makePair(APP, 0, s_21_1, SUB);
  Term Rslt_32 = newTerm(VAR, 0, portLoc(2, s_21_1Args));
  s_21_1Args = makePair(APP, 0, newI60(1), s_21_1Args);
  pushRedex(s_21_1Args, accessField);
  Term s_21Args = makePair(APP, 0, s_21, SUB);
  Term Rslt_31 = newTerm(VAR, 0, portLoc(2, s_21Args));
  s_21Args = makePair(APP, 0, newI60(0), s_21Args);
  pushRedex(s_21Args, accessField);
  Term glblVal334_1;
  glblVal334 = dupeArg(glblVal334, &glblVal334_1, 0);

  // call <= at core: 481
  Term Rslt_30Args = makePair(APP, 0, newI60(0), SUB);
  Term Rslt_30 = newTerm(VAR, 0, portLoc(2, Rslt_30Args));
  Rslt_30Args = makePair(APP, 0, final_len_27, Rslt_30Args);
#ifdef STRICT
  pushRedex(Rslt_30Args, glblFn213);
#else
  swap(termLoc(Rslt_30), makePair(LAZ, 0, Rslt_30Args, glblFn213));
#endif

  Term Rslt_33 = makeOp(OP_ADD, Rslt_32, new_start_22);
  // call SubString at core: 483
  Term Rslt_34Args = makePair(APP, 0, final_len_27_1, SUB);
  Term Rslt_34 = newTerm(VAR, 0, portLoc(2, Rslt_34Args));
  Rslt_34Args = makePair(APP, 0, Rslt_33, Rslt_34Args);
  Rslt_34Args = makePair(APP, 0, Rslt_31, Rslt_34Args);
#ifdef STRICT
  pushRedex(Rslt_34Args, glblVal334_1);
#else
  swap(termLoc(Rslt_34), makePair(LAZ, 0, Rslt_34Args, glblVal334_1));
#endif

  // call cond at core: 481
  Term Rslt_35Args = makePair(APP, 0, Rslt_34, SUB);
  Term Rslt_35 = newTerm(VAR, 0, portLoc(2, Rslt_35Args));
  Rslt_35Args = makePair(APP, 0, glblStr335, Rslt_35Args);
  Rslt_35Args = makePair(APP, 0, Rslt_30, Rslt_35Args);
#ifdef STRICT
  pushRedex(Rslt_35Args, newRef(glblProto31_339));
#else
  swap(termLoc(Rslt_35),
       makePair(LAZ, 0, Rslt_35Args, newRef(glblProto31_339)));
#endif
  dupLabels[0] = "";
  // link args to body
  swap(r_seq_28, Rslt_35);

  Term Rslt_35V = get(termLoc(Rslt_35));
  apps = take(portLoc(1, Rslt_35V));
  lams = take(portLoc(2, Rslt_35V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_35), makePair(LAZ, 0, apps, lams));
  Term Rslt_38 = makeOp(OP_ADD, Rslt_36, Rslt_37);
  // call count at core: 480
  Term Rslt_40Args = makePair(APP, 0, Rslt_39, SUB);
  Term Rslt_40 = newTerm(VAR, 0, portLoc(2, Rslt_40Args));
#ifdef STRICT
  pushRedex(Rslt_40Args, newRef(glblProto61_341));
#else
  swap(termLoc(Rslt_40),
       makePair(LAZ, 0, Rslt_40Args, newRef(glblProto61_341)));
#endif
  Term Rslt_41 = makeOp(OP_SUB, Rslt_38, Rslt_40);
  // call min at core: 480
  Term Rslt_42Args = makePair(APP, 0, Rslt_41, SUB);
  Term Rslt_42 = newTerm(VAR, 0, portLoc(2, Rslt_42Args));
  Rslt_42Args = makePair(APP, 0, new_len_23, Rslt_42Args);
#ifdef STRICT
  pushRedex(Rslt_42Args, glblFn269);
#else
  swap(termLoc(Rslt_42), makePair(LAZ, 0, Rslt_42Args, glblFn269));
#endif

  // call  at core: 480
  Term Rslt_43Args = makePair(APP, 0, Rslt_42, SUB);
  Term Rslt_43 = newTerm(VAR, 0, portLoc(2, Rslt_43Args));
#ifdef STRICT
  pushRedex(Rslt_43Args, seq_28);
#else
  swap(termLoc(Rslt_43), makePair(LAZ, 0, Rslt_43Args, seq_28));
#endif

  dupLabels[57] = "subs";
  // link args to body
  swap(r_seq_24, Rslt_43);

  Term Rslt_43V = get(termLoc(Rslt_43));
  apps = take(portLoc(1, Rslt_43V));
  lams = take(portLoc(2, Rslt_43V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_43), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_24);
  return;
}
Term glblsubs326 = newRef(glblsubs325);
Term glblFld329;
void glblFldFn328(Term ref, Term args) {
  args = makePair(APP, 0, newI60(0), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld329 = newRef(glblFldFn328);
Term glblFld331;
void glblFldFn330(Term ref, Term args) {
  args = makePair(APP, 0, newI60(1), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld331 = newRef(glblFldFn330);
Term glblFld333;
void glblFldFn332(Term ref, Term args) {
  args = makePair(APP, 0, newI60(2), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld333 = newRef(glblFldFn332);
Term glblstr_vect351;
void glblstr_vect350(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_46 = makePair(LAM, 61, SUB, NUL);
  Location r_seq_46 = portLoc(2, seq_46);
  Term s_45 = newTerm(VAR, 0, portLoc(1, seq_46));
  // allocate args at core: 486
  Term glblVal131_6;
  glblVal131 = dupeArg(glblVal131, &glblVal131_6, 0);

  // call vect-conj at core: 487
  Term Rslt_48Args = makePair(APP, 0, s_45, SUB);
  Term Rslt_48 = newTerm(VAR, 0, portLoc(2, Rslt_48Args));
  Rslt_48Args = makePair(APP, 0, glblVal131_6, Rslt_48Args);
#ifdef STRICT
  pushRedex(Rslt_48Args, glblFn134);
#else
  swap(termLoc(Rslt_48), makePair(LAZ, 0, Rslt_48Args, glblFn134));
#endif

  dupLabels[61] = "str-vect";
  // link args to body
  swap(r_seq_46, Rslt_48);

  Term Rslt_48V = get(termLoc(Rslt_48));
  apps = take(portLoc(1, Rslt_48V));
  lams = take(portLoc(2, Rslt_48V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_48), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_46);
  return;
}
Term glblstr_vect351 = newRef(glblstr_vect350);
Term glblcount349;
void glblcount348(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_51 = makePair(LAM, 60, SUB, NUL);
  Location r_seq_51 = portLoc(2, seq_51);
  Term s_50 = newTerm(VAR, 0, portLoc(1, seq_51));
  // allocate args at core: 489
  // call str-count at core: 490
  Term Rslt_53Args = makePair(APP, 0, s_50, SUB);
  Term Rslt_53 = newTerm(VAR, 0, portLoc(2, Rslt_53Args));
#ifdef STRICT
  pushRedex(Rslt_53Args, glblFn294);
#else
  swap(termLoc(Rslt_53), makePair(LAZ, 0, Rslt_53Args, glblFn294));
#endif

  dupLabels[60] = "count";
  // link args to body
  swap(r_seq_51, Rslt_53);

  Term Rslt_53V = get(termLoc(Rslt_53));
  apps = take(portLoc(1, Rslt_53V));
  lams = take(portLoc(2, Rslt_53V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_53), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_51);
  return;
}
Term glblcount349 = newRef(glblcount348);
Term glbl_EQ_347;
void glbl_EQ_346(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_57 = makePair(LAM, 59, SUB, NUL);
  Location r_seq_57 = portLoc(2, seq_57);
  Term y_56 = newTerm(VAR, 0, portLoc(1, seq_57));
  seq_57 = makePair(LAM, 59, SUB, seq_57);
  Term x_55 = newTerm(VAR, 0, portLoc(1, seq_57));
  // allocate args at core: 492

  Term x_55_1;
  x_55 = dupeArg(x_55, &x_55_1, 59);
  // call str-count at core: 493
  Term Rslt_59Args = makePair(APP, 0, x_55_1, SUB);
  Term Rslt_59 = newTerm(VAR, 0, portLoc(2, Rslt_59Args));
#ifdef STRICT
  pushRedex(Rslt_59Args, glblFn294);
#else
  swap(termLoc(Rslt_59), makePair(LAZ, 0, Rslt_59Args, glblFn294));
#endif

  // call str= at core: 493
  Term Rslt_60Args = makePair(APP, 0, y_56, SUB);
  Term Rslt_60 = newTerm(VAR, 0, portLoc(2, Rslt_60Args));
  Rslt_60Args = makePair(APP, 0, Rslt_59, Rslt_60Args);
  Rslt_60Args = makePair(APP, 0, newI60(0), Rslt_60Args);
  Rslt_60Args = makePair(APP, 0, x_55, Rslt_60Args);
#ifdef STRICT
  pushRedex(Rslt_60Args, glblFn311);
#else
  swap(termLoc(Rslt_60), makePair(LAZ, 0, Rslt_60Args, glblFn311));
#endif

  dupLabels[59] = "=";
  // link args to body
  swap(r_seq_57, Rslt_60);

  Term Rslt_60V = get(termLoc(Rslt_60));
  apps = take(portLoc(1, Rslt_60V));
  lams = take(portLoc(2, Rslt_60V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_60), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_57);
  return;
}
Term glbl_EQ_347 = newRef(glbl_EQ_346);
void glblProto31_356(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(497), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto31);
  }
}
void glblProto61_358(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(496), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
Term glblsubs345;
void glblsubs344(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_65 = makePair(LAM, 58, SUB, NUL);
  Location r_seq_65 = portLoc(2, seq_65);
  Term len_64 = newTerm(VAR, 0, portLoc(1, seq_65));
  seq_65 = makePair(LAM, 58, SUB, seq_65);
  Term start_63 = newTerm(VAR, 0, portLoc(1, seq_65));
  seq_65 = makePair(LAM, 58, SUB, seq_65);
  Term s_62 = newTerm(VAR, 0, portLoc(1, seq_65));
  // allocate args at core: 495

  Term s_62_1;
  s_62 = dupeArg(s_62, &s_62_1, 58);

  Term start_63_1;
  start_63 = dupeArg(start_63, &start_63_1, 58);
  Term seq_69 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_69 = portLoc(2, seq_69);
  Term final_len_68 = newTerm(VAR, 0, portLoc(1, seq_69));
  // allocate args at core: 496

  Term final_len_68_1;
  final_len_68 = dupeArg(final_len_68, &final_len_68_1, 0);
  Term glblVal334_2;
  glblVal334 = dupeArg(glblVal334, &glblVal334_2, 0);

  // call <= at core: 497
  Term Rslt_71Args = makePair(APP, 0, newI60(0), SUB);
  Term Rslt_71 = newTerm(VAR, 0, portLoc(2, Rslt_71Args));
  Rslt_71Args = makePair(APP, 0, final_len_68, Rslt_71Args);
#ifdef STRICT
  pushRedex(Rslt_71Args, glblFn213);
#else
  swap(termLoc(Rslt_71), makePair(LAZ, 0, Rslt_71Args, glblFn213));
#endif

  // call SubString at core: 499
  Term Rslt_72Args = makePair(APP, 0, final_len_68_1, SUB);
  Term Rslt_72 = newTerm(VAR, 0, portLoc(2, Rslt_72Args));
  Rslt_72Args = makePair(APP, 0, start_63, Rslt_72Args);
  Rslt_72Args = makePair(APP, 0, s_62, Rslt_72Args);
#ifdef STRICT
  pushRedex(Rslt_72Args, glblVal334_2);
#else
  swap(termLoc(Rslt_72), makePair(LAZ, 0, Rslt_72Args, glblVal334_2));
#endif

  // call cond at core: 497
  Term Rslt_73Args = makePair(APP, 0, Rslt_72, SUB);
  Term Rslt_73 = newTerm(VAR, 0, portLoc(2, Rslt_73Args));
  Rslt_73Args = makePair(APP, 0, glblStr335, Rslt_73Args);
  Rslt_73Args = makePair(APP, 0, Rslt_71, Rslt_73Args);
#ifdef STRICT
  pushRedex(Rslt_73Args, newRef(glblProto31_356));
#else
  swap(termLoc(Rslt_73),
       makePair(LAZ, 0, Rslt_73Args, newRef(glblProto31_356)));
#endif
  dupLabels[0] = "";
  // link args to body
  swap(r_seq_69, Rslt_73);

  Term Rslt_73V = get(termLoc(Rslt_73));
  apps = take(portLoc(1, Rslt_73V));
  lams = take(portLoc(2, Rslt_73V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_73), makePair(LAZ, 0, apps, lams));
  // call count at core: 496
  Term Rslt_74Args = makePair(APP, 0, s_62_1, SUB);
  Term Rslt_74 = newTerm(VAR, 0, portLoc(2, Rslt_74Args));
#ifdef STRICT
  pushRedex(Rslt_74Args, newRef(glblProto61_358));
#else
  swap(termLoc(Rslt_74),
       makePair(LAZ, 0, Rslt_74Args, newRef(glblProto61_358)));
#endif
  Term Rslt_75 = makeOp(OP_SUB, Rslt_74, start_63_1);
  // call min at core: 496
  Term Rslt_76Args = makePair(APP, 0, len_64, SUB);
  Term Rslt_76 = newTerm(VAR, 0, portLoc(2, Rslt_76Args));
  Rslt_76Args = makePair(APP, 0, Rslt_75, Rslt_76Args);
#ifdef STRICT
  pushRedex(Rslt_76Args, glblFn269);
#else
  swap(termLoc(Rslt_76), makePair(LAZ, 0, Rslt_76Args, glblFn269));
#endif

  // call  at core: 496
  Term Rslt_77Args = makePair(APP, 0, Rslt_76, SUB);
  Term Rslt_77 = newTerm(VAR, 0, portLoc(2, Rslt_77Args));
#ifdef STRICT
  pushRedex(Rslt_77Args, seq_69);
#else
  swap(termLoc(Rslt_77), makePair(LAZ, 0, Rslt_77Args, seq_69));
#endif

  dupLabels[58] = "subs";
  // link args to body
  swap(r_seq_65, Rslt_77);

  Term Rslt_77V = get(termLoc(Rslt_77));
  apps = take(portLoc(1, Rslt_77V));
  lams = take(portLoc(2, Rslt_77V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_77), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_65);
  return;
}
Term glblsubs345 = newRef(glblsubs344);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str7 = {StringBufferType, REFS_STATIC, 0, 0, 6, "String"};
Term glblStr373 = newTerm_(VAL, (Term)&str7);
Term glbltype_name370;
void glbltype_name369(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_80 = makePair(LAM, 66, SUB, NUL);
  Location r_seq_80 = portLoc(2, seq_80);
  Term arg_79 = newTerm(VAR, 0, portLoc(1, seq_80));
  // allocate args at core: 510
  swap(termLoc(arg_79), ERA);
  dupLabels[66] = "type-name";
  // link args to body
  swap(r_seq_80, glblStr373);

  V = get(r_seq_80);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 66, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_80, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_80);
  return;
}
Term glbltype_name370 = newRef(glbltype_name369);
Term glbltype_name372;
void glbltype_name371(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_84 = makePair(LAM, 67, SUB, NUL);
  Location r_seq_84 = portLoc(2, seq_84);
  Term arg_83 = newTerm(VAR, 0, portLoc(1, seq_84));
  // allocate args at core: 510
  swap(termLoc(arg_83), ERA);
  dupLabels[67] = "type-name";
  // link args to body
  swap(r_seq_84, glblStr373);

  V = get(r_seq_84);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 67, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_84, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_84);
  return;
}
Term glbltype_name372 = newRef(glbltype_name371);
Term glblstr_vect366;
void glblstr_vect365(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_88 = makePair(LAM, 64, SUB, NUL);
  Location r_seq_88 = portLoc(2, seq_88);
  Term s_87 = newTerm(VAR, 0, portLoc(1, seq_88));
  // allocate args at core: 513
  Term glblVal131_7;
  glblVal131 = dupeArg(glblVal131, &glblVal131_7, 0);

  // call vect-conj at core: 514
  Term Rslt_90Args = makePair(APP, 0, s_87, SUB);
  Term Rslt_90 = newTerm(VAR, 0, portLoc(2, Rslt_90Args));
  Rslt_90Args = makePair(APP, 0, glblVal131_7, Rslt_90Args);
#ifdef STRICT
  pushRedex(Rslt_90Args, glblFn134);
#else
  swap(termLoc(Rslt_90), makePair(LAZ, 0, Rslt_90Args, glblFn134));
#endif

  dupLabels[64] = "str-vect";
  // link args to body
  swap(r_seq_88, Rslt_90);

  Term Rslt_90V = get(termLoc(Rslt_90));
  apps = take(portLoc(1, Rslt_90V));
  lams = take(portLoc(2, Rslt_90V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_90), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_88);
  return;
}
Term glblstr_vect366 = newRef(glblstr_vect365);
Term glblstr_vect368;
void glblstr_vect367(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_93 = makePair(LAM, 65, SUB, NUL);
  Location r_seq_93 = portLoc(2, seq_93);
  Term s_92 = newTerm(VAR, 0, portLoc(1, seq_93));
  // allocate args at core: 513
  Term glblVal131_8;
  glblVal131 = dupeArg(glblVal131, &glblVal131_8, 0);

  // call vect-conj at core: 514
  Term Rslt_95Args = makePair(APP, 0, s_92, SUB);
  Term Rslt_95 = newTerm(VAR, 0, portLoc(2, Rslt_95Args));
  Rslt_95Args = makePair(APP, 0, glblVal131_8, Rslt_95Args);
#ifdef STRICT
  pushRedex(Rslt_95Args, glblFn134);
#else
  swap(termLoc(Rslt_95), makePair(LAZ, 0, Rslt_95Args, glblFn134));
#endif

  dupLabels[65] = "str-vect";
  // link args to body
  swap(r_seq_93, Rslt_95);

  Term Rslt_95V = get(termLoc(Rslt_95));
  apps = take(portLoc(1, Rslt_95V));
  lams = take(portLoc(2, Rslt_95V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_95), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_93);
  return;
}
Term glblstr_vect368 = newRef(glblstr_vect367);
Term glblrecurse362;
void glblrecurse361(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_99 = makePair(LAM, 62, SUB, NUL);
  Location r_seq_99 = portLoc(2, seq_99);
  Term f_98 = newTerm(VAR, 0, portLoc(1, seq_99));
  seq_99 = makePair(LAM, 62, SUB, seq_99);
  Term s_97 = newTerm(VAR, 0, portLoc(1, seq_99));
  // allocate args at core: 516
  swap(termLoc(f_98), ERA);
  dupLabels[62] = "recurse";
  // link args to body
  swap(r_seq_99, s_97);

  V = get(r_seq_99);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 62, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_99, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_99);
  return;
}
Term glblrecurse362 = newRef(glblrecurse361);
Term glblrecurse364;
void glblrecurse363(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_104 = makePair(LAM, 63, SUB, NUL);
  Location r_seq_104 = portLoc(2, seq_104);
  Term f_103 = newTerm(VAR, 0, portLoc(1, seq_104));
  seq_104 = makePair(LAM, 63, SUB, seq_104);
  Term s_102 = newTerm(VAR, 0, portLoc(1, seq_104));
  // allocate args at core: 516
  swap(termLoc(f_103), ERA);
  dupLabels[63] = "recurse";
  // link args to body
  swap(r_seq_104, s_102);

  V = get(r_seq_104);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 63, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_104, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_104);
  return;
}
Term glblrecurse364 = newRef(glblrecurse363);
Term glblFn376;
void glblCFn375(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term v_1 = arityArgs.args[0];
    Term n_2 = arityArgs.args[1];

    result = termVal((Term)vectorGet(v_1, n_2));
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn376 = newRef(glblCFn375);
void glblProto15_386(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(620), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto65_388(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(622), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto31_390(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(620), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto31);
  }
}
Term glblFn385;
void glblCFn384(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_4 = makePair(LAM, 69, SUB, NUL);
  Location r_seq_4 = portLoc(2, seq_4);
  Term x_3 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 69, SUB, seq_4);
  Term n_2 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 69, SUB, seq_4);
  Term v_1 = newTerm(VAR, 0, portLoc(1, seq_4));
  // allocate args at core: 615

  Term v_1_1;
  v_1 = dupeArg(v_1, &v_1_1, 69);

  Term n_2_1;
  n_2 = dupeArg(n_2, &n_2_1, 69);

  Term x_3_2;
  x_3 = dupeArg(x_3, &x_3_2, 69);

  Term x_3_1;
  x_3 = dupeArg(x_3, &x_3_1, 69);
  // call = at core: 620
  Term Rslt_6Args = makePair(APP, 0, x_3, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
  Rslt_6Args = makePair(APP, 0, n_2, Rslt_6Args);
#ifdef STRICT
  pushRedex(Rslt_6Args, newRef(glblProto15_386));
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, newRef(glblProto15_386)));
#endif
  // call conj at core: 622
  Term Rslt_7Args = makePair(APP, 0, x_3_1, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
  Rslt_7Args = makePair(APP, 0, v_1_1, Rslt_7Args);
#ifdef STRICT
  pushRedex(Rslt_7Args, newRef(glblProto65_388));
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, newRef(glblProto65_388)));
#endif
  // call inc at core: 622
  Term Rslt_8Args = makePair(APP, 0, x_3_2, SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, Rslt_8Args));
#ifdef STRICT
  pushRedex(Rslt_8Args, glblFn120);
#else
  swap(termLoc(Rslt_8), makePair(LAZ, 0, Rslt_8Args, glblFn120));
#endif

  // call range* at core: 615
  Term Rslt_9Args = makePair(APP, 0, Rslt_8, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
  Rslt_9Args = makePair(APP, 0, n_2_1, Rslt_9Args);
  Rslt_9Args = makePair(APP, 0, Rslt_7, Rslt_9Args);
#ifdef STRICT
  pushRedex(Rslt_9Args, glblFn385);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, glblFn385));
#endif

  // call cond at core: 620
  Term Rslt_10Args = makePair(APP, 0, Rslt_9, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
  Rslt_10Args = makePair(APP, 0, v_1, Rslt_10Args);
  Rslt_10Args = makePair(APP, 0, Rslt_6, Rslt_10Args);
#ifdef STRICT
  pushRedex(Rslt_10Args, newRef(glblProto31_390));
#else
  swap(termLoc(Rslt_10),
       makePair(LAZ, 0, Rslt_10Args, newRef(glblProto31_390)));
#endif
  dupLabels[69] = "";
  // link args to body
  swap(r_seq_4, Rslt_10);

  Term Rslt_10V = get(termLoc(Rslt_10));
  apps = take(portLoc(1, Rslt_10V));
  lams = take(portLoc(2, Rslt_10V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_10), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_4);
  return;
}
Term glblFn385 = newRef(glblCFn384);
Term glblFn394;
void glblCFn393(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 70, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term n_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 624
  Term glblVal131_9;
  glblVal131 = dupeArg(glblVal131, &glblVal131_9, 0);

  // call range* at core: 627
  Term Rslt_4Args = makePair(APP, 0, newI60(0), SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
  Rslt_4Args = makePair(APP, 0, n_1, Rslt_4Args);
  Rslt_4Args = makePair(APP, 0, glblVal131_9, Rslt_4Args);
#ifdef STRICT
  pushRedex(Rslt_4Args, glblFn385);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, glblFn385));
#endif

  dupLabels[70] = "";
  // link args to body
  swap(r_seq_2, Rslt_4);

  Term Rslt_4V = get(termLoc(Rslt_4));
  apps = take(portLoc(1, Rslt_4V));
  lams = take(portLoc(2, Rslt_4V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_4), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn394 = newRef(glblCFn393);
Term glblProto396;
Term glblvect_reduce399;
void glblvect_reduce398(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_13 = makePair(LAM, 71, SUB, NUL);
  Location r_seq_13 = portLoc(2, seq_13);
  Term f_12 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 71, SUB, seq_13);
  Term result_11 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 71, SUB, seq_13);
  Term n_10 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 71, SUB, seq_13);
  Term vect_9 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 71, SUB, seq_13);
  Term el_8 = newTerm(VAR, 0, portLoc(1, seq_13));
  // allocate args at core: 638
  swap(termLoc(el_8), ERA);
  swap(termLoc(vect_9), ERA);
  swap(termLoc(n_10), ERA);
  swap(termLoc(f_12), ERA);
  dupLabels[71] = "vect-reduce";
  // link args to body
  swap(r_seq_13, result_11);

  V = get(r_seq_13);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 71, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_13, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_13);
  return;
}
Term glblvect_reduce399 = newRef(glblvect_reduce398);
void glblProto101_405(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(642), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto101);
  }
}
void glblProto16_407(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(642), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto396_409(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(642), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
Term glblvect_reduce402;
void glblvect_reduce401(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = makePair(LAM, 72, SUB, NUL);
  Location r_seq_21 = portLoc(2, seq_21);
  Term f_20 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 72, SUB, seq_21);
  Term result_19 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 72, SUB, seq_21);
  Term n_18 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 72, SUB, seq_21);
  Term vect_17 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 72, SUB, seq_21);
  Term el_16 = newTerm(VAR, 0, portLoc(1, seq_21));
  // allocate args at core: 641

  Term vect_17_1;
  vect_17 = dupeArg(vect_17, &vect_17_1, 72);

  Term n_18_1;
  n_18 = dupeArg(n_18, &n_18_1, 72);

  Term f_20_1;
  f_20 = dupeArg(f_20, &f_20_1, 72);
  // call get at core: 642
  Term Rslt_23Args = makePair(APP, 0, n_18, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
  Rslt_23Args = makePair(APP, 0, vect_17, Rslt_23Args);
#ifdef STRICT
  pushRedex(Rslt_23Args, newRef(glblProto101_405));
#else
  swap(termLoc(Rslt_23),
       makePair(LAZ, 0, Rslt_23Args, newRef(glblProto101_405)));
#endif
  // call inc at core: 642
  Term Rslt_24Args = makePair(APP, 0, n_18_1, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
#ifdef STRICT
  pushRedex(Rslt_24Args, glblFn120);
#else
  swap(termLoc(Rslt_24), makePair(LAZ, 0, Rslt_24Args, glblFn120));
#endif

  // call .x at core: 642
  Term Rslt_25Args = makePair(APP, 0, el_16, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
#ifdef STRICT
  pushRedex(Rslt_25Args, newRef(glblProto16_407));
#else
  swap(termLoc(Rslt_25),
       makePair(LAZ, 0, Rslt_25Args, newRef(glblProto16_407)));
#endif
  // call f at core: 641
  Term Rslt_26Args = makePair(APP, 0, Rslt_25, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
  Rslt_26Args = makePair(APP, 0, result_19, Rslt_26Args);
#ifdef STRICT
  pushRedex(Rslt_26Args, f_20);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, f_20));
#endif

  // call vect-reduce at core: 642
  Term Rslt_27Args = makePair(APP, 0, f_20_1, SUB);
  Term Rslt_27 = newTerm(VAR, 0, portLoc(2, Rslt_27Args));
  Rslt_27Args = makePair(APP, 0, Rslt_26, Rslt_27Args);
  Rslt_27Args = makePair(APP, 0, Rslt_24, Rslt_27Args);
  Rslt_27Args = makePair(APP, 0, vect_17_1, Rslt_27Args);
  Rslt_27Args = makePair(APP, 0, Rslt_23, Rslt_27Args);
#ifdef STRICT
  pushRedex(Rslt_27Args, newRef(glblProto396_409));
#else
  swap(termLoc(Rslt_27),
       makePair(LAZ, 0, Rslt_27Args, newRef(glblProto396_409)));
#endif
  dupLabels[72] = "vect-reduce";
  // link args to body
  swap(r_seq_21, Rslt_27);

  Term Rslt_27V = get(termLoc(Rslt_27));
  apps = take(portLoc(1, Rslt_27V));
  lams = take(portLoc(2, Rslt_27V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_27), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_21);
  return;
}
Term glblvect_reduce402 = newRef(glblvect_reduce401);
Term glblProto403;
Term glblvect_reverse413;
void glblvect_reverse412(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_11 = makePair(LAM, 73, SUB, NUL);
  Location r_seq_11 = portLoc(2, seq_11);
  Term result_10 = newTerm(VAR, 0, portLoc(1, seq_11));
  seq_11 = makePair(LAM, 73, SUB, seq_11);
  Term n_9 = newTerm(VAR, 0, portLoc(1, seq_11));
  seq_11 = makePair(LAM, 73, SUB, seq_11);
  Term vect_8 = newTerm(VAR, 0, portLoc(1, seq_11));
  seq_11 = makePair(LAM, 73, SUB, seq_11);
  Term el_7 = newTerm(VAR, 0, portLoc(1, seq_11));
  // allocate args at core: 652
  swap(termLoc(el_7), ERA);
  swap(termLoc(vect_8), ERA);
  swap(termLoc(n_9), ERA);
  dupLabels[73] = "vect-reverse";
  // link args to body
  swap(r_seq_11, result_10);

  V = get(r_seq_11);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 73, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_11, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_11);
  return;
}
Term glblvect_reverse413 = newRef(glblvect_reverse412);
void glblProto101_419(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(656), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto101);
  }
}
void glblProto25_421(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(656), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
void glblProto16_423(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(657), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto65_425(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(657), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto403_427(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(656), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto403);
  }
}
Term glblvect_reverse416;
void glblvect_reverse415(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_18 = makePair(LAM, 74, SUB, NUL);
  Location r_seq_18 = portLoc(2, seq_18);
  Term result_17 = newTerm(VAR, 0, portLoc(1, seq_18));
  seq_18 = makePair(LAM, 74, SUB, seq_18);
  Term n_16 = newTerm(VAR, 0, portLoc(1, seq_18));
  seq_18 = makePair(LAM, 74, SUB, seq_18);
  Term vect_15 = newTerm(VAR, 0, portLoc(1, seq_18));
  seq_18 = makePair(LAM, 74, SUB, seq_18);
  Term el_14 = newTerm(VAR, 0, portLoc(1, seq_18));
  // allocate args at core: 655

  Term vect_15_1;
  vect_15 = dupeArg(vect_15, &vect_15_1, 74);

  Term n_16_2;
  n_16 = dupeArg(n_16, &n_16_2, 74);

  Term n_16_1;
  n_16 = dupeArg(n_16, &n_16_1, 74);
  // call <= at core: 656
  Term Rslt_20Args = makePair(APP, 0, n_16, SUB);
  Term Rslt_20 = newTerm(VAR, 0, portLoc(2, Rslt_20Args));
  Rslt_20Args = makePair(APP, 0, newI60(0), Rslt_20Args);
#ifdef STRICT
  pushRedex(Rslt_20Args, glblFn213);
#else
  swap(termLoc(Rslt_20), makePair(LAZ, 0, Rslt_20Args, glblFn213));
#endif

  // call get at core: 656
  Term Rslt_21Args = makePair(APP, 0, n_16_1, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
  Rslt_21Args = makePair(APP, 0, vect_15, Rslt_21Args);
#ifdef STRICT
  pushRedex(Rslt_21Args, newRef(glblProto101_419));
#else
  swap(termLoc(Rslt_21),
       makePair(LAZ, 0, Rslt_21Args, newRef(glblProto101_419)));
#endif
  // call and at core: 656
  Term Rslt_22Args = makePair(APP, 0, Rslt_21, SUB);
  Term Rslt_22 = newTerm(VAR, 0, portLoc(2, Rslt_22Args));
  Rslt_22Args = makePair(APP, 0, Rslt_20, Rslt_22Args);
#ifdef STRICT
  pushRedex(Rslt_22Args, newRef(glblProto25_421));
#else
  swap(termLoc(Rslt_22),
       makePair(LAZ, 0, Rslt_22Args, newRef(glblProto25_421)));
#endif
  // call dec at core: 657
  Term Rslt_23Args = makePair(APP, 0, n_16_2, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
#ifdef STRICT
  pushRedex(Rslt_23Args, glblFn127);
#else
  swap(termLoc(Rslt_23), makePair(LAZ, 0, Rslt_23Args, glblFn127));
#endif

  // call .x at core: 657
  Term Rslt_24Args = makePair(APP, 0, el_14, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
#ifdef STRICT
  pushRedex(Rslt_24Args, newRef(glblProto16_423));
#else
  swap(termLoc(Rslt_24),
       makePair(LAZ, 0, Rslt_24Args, newRef(glblProto16_423)));
#endif
  // call conj at core: 657
  Term Rslt_25Args = makePair(APP, 0, Rslt_24, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
  Rslt_25Args = makePair(APP, 0, result_17, Rslt_25Args);
#ifdef STRICT
  pushRedex(Rslt_25Args, newRef(glblProto65_425));
#else
  swap(termLoc(Rslt_25),
       makePair(LAZ, 0, Rslt_25Args, newRef(glblProto65_425)));
#endif
  // call vect-reverse at core: 656
  Term Rslt_26Args = makePair(APP, 0, Rslt_25, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
  Rslt_26Args = makePair(APP, 0, Rslt_23, Rslt_26Args);
  Rslt_26Args = makePair(APP, 0, vect_15_1, Rslt_26Args);
  Rslt_26Args = makePair(APP, 0, Rslt_22, Rslt_26Args);
#ifdef STRICT
  pushRedex(Rslt_26Args, newRef(glblProto403_427));
#else
  swap(termLoc(Rslt_26),
       makePair(LAZ, 0, Rslt_26Args, newRef(glblProto403_427)));
#endif
  dupLabels[74] = "vect-reverse";
  // link args to body
  swap(r_seq_18, Rslt_26);

  Term Rslt_26V = get(termLoc(Rslt_26));
  apps = take(portLoc(1, Rslt_26V));
  lams = take(portLoc(2, Rslt_26V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_26), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_18);
  return;
}
Term glblvect_reverse416 = newRef(glblvect_reverse415);
Term glblProto417;
Term glblvect_sub431;
void glblvect_sub430(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_13 = makePair(LAM, 75, SUB, NUL);
  Location r_seq_13 = portLoc(2, seq_13);
  Term result_12 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 75, SUB, seq_13);
  Term max_n_11 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 75, SUB, seq_13);
  Term n_10 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 75, SUB, seq_13);
  Term vect_9 = newTerm(VAR, 0, portLoc(1, seq_13));
  seq_13 = makePair(LAM, 75, SUB, seq_13);
  Term el_8 = newTerm(VAR, 0, portLoc(1, seq_13));
  // allocate args at core: 666
  swap(termLoc(el_8), ERA);
  swap(termLoc(vect_9), ERA);
  swap(termLoc(n_10), ERA);
  swap(termLoc(max_n_11), ERA);
  dupLabels[75] = "vect-sub";
  // link args to body
  swap(r_seq_13, result_12);

  V = get(r_seq_13);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 75, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_13, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_13);
  return;
}
Term glblvect_sub431 = newRef(glblvect_sub430);
void glblProto23_435(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(670), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto23);
  }
}
void glblProto101_437(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(670), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto101);
  }
}
void glblProto25_439(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(670), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
void glblProto16_441(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(671), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto65_443(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(671), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto417_445(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(670), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto417);
  }
}
Term glblvect_sub434;
void glblvect_sub433(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_21 = makePair(LAM, 76, SUB, NUL);
  Location r_seq_21 = portLoc(2, seq_21);
  Term result_20 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 76, SUB, seq_21);
  Term max_n_19 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 76, SUB, seq_21);
  Term n_18 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 76, SUB, seq_21);
  Term vect_17 = newTerm(VAR, 0, portLoc(1, seq_21));
  seq_21 = makePair(LAM, 76, SUB, seq_21);
  Term el_16 = newTerm(VAR, 0, portLoc(1, seq_21));
  // allocate args at core: 669

  Term vect_17_1;
  vect_17 = dupeArg(vect_17, &vect_17_1, 76);

  Term n_18_2;
  n_18 = dupeArg(n_18, &n_18_2, 76);

  Term n_18_1;
  n_18 = dupeArg(n_18, &n_18_1, 76);

  Term max_n_19_1;
  max_n_19 = dupeArg(max_n_19, &max_n_19_1, 76);
  // call < at core: 670
  Term Rslt_23Args = makePair(APP, 0, max_n_19, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
  Rslt_23Args = makePair(APP, 0, n_18, Rslt_23Args);
#ifdef STRICT
  pushRedex(Rslt_23Args, newRef(glblProto23_435));
#else
  swap(termLoc(Rslt_23),
       makePair(LAZ, 0, Rslt_23Args, newRef(glblProto23_435)));
#endif
  // call get at core: 670
  Term Rslt_24Args = makePair(APP, 0, n_18_1, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
  Rslt_24Args = makePair(APP, 0, vect_17, Rslt_24Args);
#ifdef STRICT
  pushRedex(Rslt_24Args, newRef(glblProto101_437));
#else
  swap(termLoc(Rslt_24),
       makePair(LAZ, 0, Rslt_24Args, newRef(glblProto101_437)));
#endif
  // call and at core: 670
  Term Rslt_25Args = makePair(APP, 0, Rslt_24, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
  Rslt_25Args = makePair(APP, 0, Rslt_23, Rslt_25Args);
#ifdef STRICT
  pushRedex(Rslt_25Args, newRef(glblProto25_439));
#else
  swap(termLoc(Rslt_25),
       makePair(LAZ, 0, Rslt_25Args, newRef(glblProto25_439)));
#endif
  // call inc at core: 671
  Term Rslt_26Args = makePair(APP, 0, n_18_2, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
#ifdef STRICT
  pushRedex(Rslt_26Args, glblFn120);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, glblFn120));
#endif

  // call .x at core: 671
  Term Rslt_27Args = makePair(APP, 0, el_16, SUB);
  Term Rslt_27 = newTerm(VAR, 0, portLoc(2, Rslt_27Args));
#ifdef STRICT
  pushRedex(Rslt_27Args, newRef(glblProto16_441));
#else
  swap(termLoc(Rslt_27),
       makePair(LAZ, 0, Rslt_27Args, newRef(glblProto16_441)));
#endif
  // call conj at core: 671
  Term Rslt_28Args = makePair(APP, 0, Rslt_27, SUB);
  Term Rslt_28 = newTerm(VAR, 0, portLoc(2, Rslt_28Args));
  Rslt_28Args = makePair(APP, 0, result_20, Rslt_28Args);
#ifdef STRICT
  pushRedex(Rslt_28Args, newRef(glblProto65_443));
#else
  swap(termLoc(Rslt_28),
       makePair(LAZ, 0, Rslt_28Args, newRef(glblProto65_443)));
#endif
  // call vect-sub at core: 670
  Term Rslt_29Args = makePair(APP, 0, Rslt_28, SUB);
  Term Rslt_29 = newTerm(VAR, 0, portLoc(2, Rslt_29Args));
  Rslt_29Args = makePair(APP, 0, max_n_19_1, Rslt_29Args);
  Rslt_29Args = makePair(APP, 0, Rslt_26, Rslt_29Args);
  Rslt_29Args = makePair(APP, 0, vect_17_1, Rslt_29Args);
  Rslt_29Args = makePair(APP, 0, Rslt_25, Rslt_29Args);
#ifdef STRICT
  pushRedex(Rslt_29Args, newRef(glblProto417_445));
#else
  swap(termLoc(Rslt_29),
       makePair(LAZ, 0, Rslt_29Args, newRef(glblProto417_445)));
#endif
  dupLabels[76] = "vect-sub";
  // link args to body
  swap(r_seq_21, Rslt_29);

  Term Rslt_29V = get(termLoc(Rslt_29));
  apps = take(portLoc(1, Rslt_29V));
  lams = take(portLoc(2, Rslt_29V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_29), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_21);
  return;
}
Term glblvect_sub434 = newRef(glblvect_sub433);
void glblProto417_451(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(675), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto417);
  }
}
Term glblFn449;
void glblCFn448(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_4 = makePair(LAM, 77, SUB, NUL);
  Location r_seq_4 = portLoc(2, seq_4);
  Term len_3 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 77, SUB, seq_4);
  Term start_2 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 77, SUB, seq_4);
  Term v_1 = newTerm(VAR, 0, portLoc(1, seq_4));
  // allocate args at core: 673

  Term v_1_1;
  v_1 = dupeArg(v_1, &v_1_1, 77);

  Term start_2_2;
  start_2 = dupeArg(start_2, &start_2_2, 77);

  Term start_2_1;
  start_2 = dupeArg(start_2, &start_2_1, 77);
  Term glblVal131_10;
  glblVal131 = dupeArg(glblVal131, &glblVal131_10, 0);

  // call vect-get at core: 675
  Term Rslt_6Args = makePair(APP, 0, start_2, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
  Rslt_6Args = makePair(APP, 0, v_1, Rslt_6Args);
#ifdef STRICT
  pushRedex(Rslt_6Args, glblFn376);
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, glblFn376));
#endif

  // call inc at core: 675
  Term Rslt_7Args = makePair(APP, 0, start_2_1, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
#ifdef STRICT
  pushRedex(Rslt_7Args, glblFn120);
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, glblFn120));
#endif

  Term Rslt_8 = makeOp(OP_ADD, start_2_2, len_3);
  // call vect-sub at core: 675
  Term Rslt_9Args = makePair(APP, 0, glblVal131_10, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
  Rslt_9Args = makePair(APP, 0, Rslt_8, Rslt_9Args);
  Rslt_9Args = makePair(APP, 0, Rslt_7, Rslt_9Args);
  Rslt_9Args = makePair(APP, 0, v_1_1, Rslt_9Args);
  Rslt_9Args = makePair(APP, 0, Rslt_6, Rslt_9Args);
#ifdef STRICT
  pushRedex(Rslt_9Args, newRef(glblProto417_451));
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, newRef(glblProto417_451)));
#endif
  dupLabels[77] = "";
  // link args to body
  swap(r_seq_4, Rslt_9);

  Term Rslt_9V = get(termLoc(Rslt_9));
  apps = take(portLoc(1, Rslt_9V));
  lams = take(portLoc(2, Rslt_9V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_9), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_4);
  return;
}
Term glblFn449 = newRef(glblCFn448);
Term glblVal450 = newRef(vectMap);
Term glblVal450;
void glblProto73_458(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(684), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto73);
  }
}
void glblProto396_460(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(684), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
void glblProto73_462(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(681), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto73);
  }
}
void glblProto61_464(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(683), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
Term glblc457;
void glblc456(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_19 = makePair(LAM, 79, SUB, NUL);
  Location r_seq_19 = portLoc(2, seq_19);
  Term s_18 = newTerm(VAR, 0, portLoc(1, seq_19));
  seq_19 = makePair(LAM, 79, SUB, seq_19);
  Term len_17 = newTerm(VAR, 0, portLoc(1, seq_19));
  // allocate args at core: 682
  // call count at core: 683
  Term Rslt_21Args = makePair(APP, 0, s_18, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
#ifdef STRICT
  pushRedex(Rslt_21Args, newRef(glblProto61_464));
#else
  swap(termLoc(Rslt_21),
       makePair(LAZ, 0, Rslt_21Args, newRef(glblProto61_464)));
#endif
  Term Rslt_22 = makeOp(OP_ADD, len_17, Rslt_21);
  dupLabels[79] = "";
  // link args to body
  swap(r_seq_19, Rslt_22);

  Term Rslt_22V = get(termLoc(Rslt_22));
  apps = take(portLoc(1, Rslt_22V));
  lams = take(portLoc(2, Rslt_22V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_22), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_19);
  return;
}
Term glblc457 = newRef(glblc456);
void glblProto396_467(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(681), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
void glblProto43_469(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(680), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto43);
  }
}
void glblProto107_471(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(680), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto107);
  }
}
Term glblFn455;
void glblCFn454(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 78, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term vect_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 679
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term ss_vect_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at core: 680

  Term ss_vect_5_3;
  ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_3, 0);

  Term ss_vect_5_2;
  ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_2, 0);

  Term ss_vect_5_1;
  ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_1, 0);
  Term seq_10 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_10 = portLoc(2, seq_10);
  Term new_len_9 = newTerm(VAR, 0, portLoc(1, seq_10));
  // allocate args at core: 681
  // call first at core: 684
  Term Rslt_12Args = makePair(APP, 0, ss_vect_5, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
#ifdef STRICT
  pushRedex(Rslt_12Args, newRef(glblProto73_458));
#else
  swap(termLoc(Rslt_12),
       makePair(LAZ, 0, Rslt_12Args, newRef(glblProto73_458)));
#endif
  // call str-malloc at core: 684
  Term Rslt_13Args = makePair(APP, 0, new_len_9, SUB);
  Term Rslt_13 = newTerm(VAR, 0, portLoc(2, Rslt_13Args));
#ifdef STRICT
  pushRedex(Rslt_13Args, glblFn302);
#else
  swap(termLoc(Rslt_13), makePair(LAZ, 0, Rslt_13Args, glblFn302));
#endif

  // call vect-reduce at core: 684
  Term Rslt_14Args = makePair(APP, 0, glblFn306, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
  Rslt_14Args = makePair(APP, 0, Rslt_13, Rslt_14Args);
  Rslt_14Args = makePair(APP, 0, newI60(1), Rslt_14Args);
  Rslt_14Args = makePair(APP, 0, ss_vect_5_1, Rslt_14Args);
  Rslt_14Args = makePair(APP, 0, Rslt_12, Rslt_14Args);
#ifdef STRICT
  pushRedex(Rslt_14Args, newRef(glblProto396_460));
#else
  swap(termLoc(Rslt_14),
       makePair(LAZ, 0, Rslt_14Args, newRef(glblProto396_460)));
#endif
  dupLabels[0] = "";
  // link args to body
  swap(r_seq_10, Rslt_14);

  Term Rslt_14V = get(termLoc(Rslt_14));
  apps = take(portLoc(1, Rslt_14V));
  lams = take(portLoc(2, Rslt_14V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_14), makePair(LAZ, 0, apps, lams));
  // call first at core: 681
  Term Rslt_15Args = makePair(APP, 0, ss_vect_5_2, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
#ifdef STRICT
  pushRedex(Rslt_15Args, newRef(glblProto73_462));
#else
  swap(termLoc(Rslt_15),
       makePair(LAZ, 0, Rslt_15Args, newRef(glblProto73_462)));
#endif
  // call vect-reduce at core: 681
  Term Rslt_23Args = makePair(APP, 0, glblc457, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
  Rslt_23Args = makePair(APP, 0, newI60(0), Rslt_23Args);
  Rslt_23Args = makePair(APP, 0, newI60(1), Rslt_23Args);
  Rslt_23Args = makePair(APP, 0, ss_vect_5_3, Rslt_23Args);
  Rslt_23Args = makePair(APP, 0, Rslt_15, Rslt_23Args);
#ifdef STRICT
  pushRedex(Rslt_23Args, newRef(glblProto396_467));
#else
  swap(termLoc(Rslt_23),
       makePair(LAZ, 0, Rslt_23Args, newRef(glblProto396_467)));
#endif
  // call  at core: 681
  Term Rslt_24Args = makePair(APP, 0, Rslt_23, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
#ifdef STRICT
  pushRedex(Rslt_24Args, seq_10);
#else
  swap(termLoc(Rslt_24), makePair(LAZ, 0, Rslt_24Args, seq_10));
#endif

  dupLabels[0] = "";
  // link args to body
  swap(r_seq_6, Rslt_24);

  Term Rslt_24V = get(termLoc(Rslt_24));
  apps = take(portLoc(1, Rslt_24V));
  lams = take(portLoc(2, Rslt_24V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_24), makePair(LAZ, 0, apps, lams));
  // call flat-map at core: 680
  Term Rslt_25Args = makePair(APP, 0, newRef(glblProto107_471), SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
  Rslt_25Args = makePair(APP, 0, vect_1, Rslt_25Args);
#ifdef STRICT
  pushRedex(Rslt_25Args, newRef(glblProto43_469));
#else
  swap(termLoc(Rslt_25),
       makePair(LAZ, 0, Rslt_25Args, newRef(glblProto43_469)));
#endif
  // call  at core: 680
  Term Rslt_26Args = makePair(APP, 0, Rslt_25, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
#ifdef STRICT
  pushRedex(Rslt_26Args, seq_6);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, seq_6));
#endif

  dupLabels[78] = "";
  // link args to body
  swap(r_seq_2, Rslt_26);

  Term Rslt_26V = get(termLoc(Rslt_26));
  apps = take(portLoc(1, Rslt_26V));
  lams = take(portLoc(2, Rslt_26V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_26), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn455 = newRef(glblCFn454);
void glblProto23_478(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(699), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto23);
  }
}
void glblProto95_480(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto95);
  }
}
void glblProto16_482(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto95_484(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto95);
  }
}
void glblProto16_486(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto16);
  }
}
void glblProto15_488(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto25_490(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(701), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
void glblProto31_492(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(699), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto31);
  }
}
Term glblFn475;
void glblCFn474(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_4 = makePair(LAM, 80, SUB, NUL);
  Location r_seq_4 = portLoc(2, seq_4);
  Term start_3 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 80, SUB, seq_4);
  Term y_2 = newTerm(VAR, 0, portLoc(1, seq_4));
  seq_4 = makePair(LAM, 80, SUB, seq_4);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_4));
  // allocate args at core: 686

  Term x_1_2;
  x_1 = dupeArg(x_1, &x_1_2, 80);

  Term x_1_1;
  x_1 = dupeArg(x_1, &x_1_1, 80);

  Term y_2_1;
  y_2 = dupeArg(y_2, &y_2_1, 80);

  Term start_3_3;
  start_3 = dupeArg(start_3, &start_3_3, 80);

  Term start_3_2;
  start_3 = dupeArg(start_3, &start_3_2, 80);

  Term start_3_1;
  start_3 = dupeArg(start_3, &start_3_1, 80);
  Term glblVal22_3;
  glblVal22 = dupeArg(glblVal22, &glblVal22_3, 0);

  // call < at core: 699
  Term Rslt_6Args = makePair(APP, 0, newI60(0), SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
  Rslt_6Args = makePair(APP, 0, start_3, Rslt_6Args);
#ifdef STRICT
  pushRedex(Rslt_6Args, newRef(glblProto23_478));
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, newRef(glblProto23_478)));
#endif
  // call Some at core: 700
  Term Rslt_7Args = makePair(APP, 0, x_1, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
#ifdef STRICT
  pushRedex(Rslt_7Args, glblVal22_3);
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, glblVal22_3));
#endif

  // call nth at core: 701
  Term Rslt_8Args = makePair(APP, 0, start_3_1, SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, Rslt_8Args));
  Rslt_8Args = makePair(APP, 0, x_1_1, Rslt_8Args);
#ifdef STRICT
  pushRedex(Rslt_8Args, newRef(glblProto95_480));
#else
  swap(termLoc(Rslt_8), makePair(LAZ, 0, Rslt_8Args, newRef(glblProto95_480)));
#endif
  // call .x at core: 701
  Term Rslt_9Args = makePair(APP, 0, Rslt_8, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
#ifdef STRICT
  pushRedex(Rslt_9Args, newRef(glblProto16_482));
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, newRef(glblProto16_482)));
#endif
  // call nth at core: 701
  Term Rslt_10Args = makePair(APP, 0, start_3_2, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
  Rslt_10Args = makePair(APP, 0, y_2, Rslt_10Args);
#ifdef STRICT
  pushRedex(Rslt_10Args, newRef(glblProto95_484));
#else
  swap(termLoc(Rslt_10),
       makePair(LAZ, 0, Rslt_10Args, newRef(glblProto95_484)));
#endif
  // call .x at core: 701
  Term Rslt_11Args = makePair(APP, 0, Rslt_10, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
#ifdef STRICT
  pushRedex(Rslt_11Args, newRef(glblProto16_486));
#else
  swap(termLoc(Rslt_11),
       makePair(LAZ, 0, Rslt_11Args, newRef(glblProto16_486)));
#endif
  // call = at core: 701
  Term Rslt_12Args = makePair(APP, 0, Rslt_11, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
  Rslt_12Args = makePair(APP, 0, Rslt_9, Rslt_12Args);
#ifdef STRICT
  pushRedex(Rslt_12Args, newRef(glblProto15_488));
#else
  swap(termLoc(Rslt_12),
       makePair(LAZ, 0, Rslt_12Args, newRef(glblProto15_488)));
#endif
  // call dec at core: 702
  Term Rslt_13Args = makePair(APP, 0, start_3_3, SUB);
  Term Rslt_13 = newTerm(VAR, 0, portLoc(2, Rslt_13Args));
#ifdef STRICT
  pushRedex(Rslt_13Args, glblFn127);
#else
  swap(termLoc(Rslt_13), makePair(LAZ, 0, Rslt_13Args, glblFn127));
#endif

  // call vect-= at core: 686
  Term Rslt_14Args = makePair(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
  Rslt_14Args = makePair(APP, 0, y_2_1, Rslt_14Args);
  Rslt_14Args = makePair(APP, 0, x_1_2, Rslt_14Args);
#ifdef STRICT
  pushRedex(Rslt_14Args, glblFn475);
#else
  swap(termLoc(Rslt_14), makePair(LAZ, 0, Rslt_14Args, glblFn475));
#endif

  // call and at core: 701
  Term Rslt_15Args = makePair(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
  Rslt_15Args = makePair(APP, 0, Rslt_12, Rslt_15Args);
#ifdef STRICT
  pushRedex(Rslt_15Args, newRef(glblProto25_490));
#else
  swap(termLoc(Rslt_15),
       makePair(LAZ, 0, Rslt_15Args, newRef(glblProto25_490)));
#endif
  // call cond at core: 699
  Term Rslt_16Args = makePair(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, Rslt_16Args));
  Rslt_16Args = makePair(APP, 0, Rslt_7, Rslt_16Args);
  Rslt_16Args = makePair(APP, 0, Rslt_6, Rslt_16Args);
#ifdef STRICT
  pushRedex(Rslt_16Args, newRef(glblProto31_492));
#else
  swap(termLoc(Rslt_16),
       makePair(LAZ, 0, Rslt_16Args, newRef(glblProto31_492)));
#endif
  dupLabels[80] = "";
  // link args to body
  swap(r_seq_4, Rslt_16);

  Term Rslt_16V = get(termLoc(Rslt_16));
  apps = take(portLoc(1, Rslt_16V));
  lams = take(portLoc(2, Rslt_16V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_16), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_4);
  return;
}
Term glblFn475 = newRef(glblCFn474);
Term glblFn477;
void glblCFn476(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 3) {
    Term v_1 = arityArgs.args[0];
    Term n_2 = arityArgs.args[1];
    Term x_3 = arityArgs.args[2];

    result = vectStore((Vector *)v_1, getI60(n_2), (Term)x_3);
    dec_and_free(n_2, 1);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn477 = newRef(glblCFn476);
void glblProto15_552(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(716), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto15_557(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(717), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto15);
  }
}
void glblProto61_559(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(718), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
void glblProto25_561(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(717), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
void glblProto25_563(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(716), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
Term glbl_EQ_539;
void glbl_EQ_538(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_9 = makePair(LAM, 103, SUB, NUL);
  Location r_seq_9 = portLoc(2, seq_9);
  Term y_8 = newTerm(VAR, 0, portLoc(1, seq_9));
  seq_9 = makePair(LAM, 103, SUB, seq_9);
  Term x_7 = newTerm(VAR, 0, portLoc(1, seq_9));
  // allocate args at core: 714

  Term x_7_3;
  x_7 = dupeArg(x_7, &x_7_3, 103);

  Term x_7_2;
  x_7 = dupeArg(x_7, &x_7_2, 103);

  Term x_7_1;
  x_7 = dupeArg(x_7, &x_7_1, 103);

  Term y_8_2;
  y_8 = dupeArg(y_8, &y_8_2, 103);

  Term y_8_1;
  y_8 = dupeArg(y_8, &y_8_1, 103);
  // call type-num at core: 716
  Term Rslt_11Args = makePair(APP, 0, x_7, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
#ifdef STRICT
  pushRedex(Rslt_11Args, glblFn8);
#else
  swap(termLoc(Rslt_11), makePair(LAZ, 0, Rslt_11Args, glblFn8));
#endif

  // call type-num at core: 716
  Term Rslt_12Args = makePair(APP, 0, y_8, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
#ifdef STRICT
  pushRedex(Rslt_12Args, glblFn8);
#else
  swap(termLoc(Rslt_12), makePair(LAZ, 0, Rslt_12Args, glblFn8));
#endif

  // call = at core: 716
  Term Rslt_13Args = makePair(APP, 0, Rslt_12, SUB);
  Term Rslt_13 = newTerm(VAR, 0, portLoc(2, Rslt_13Args));
  Rslt_13Args = makePair(APP, 0, Rslt_11, Rslt_13Args);
#ifdef STRICT
  pushRedex(Rslt_13Args, newRef(glblProto15_552));
#else
  swap(termLoc(Rslt_13),
       makePair(LAZ, 0, Rslt_13Args, newRef(glblProto15_552)));
#endif
  // call vect-count at core: 717
  Term Rslt_14Args = makePair(APP, 0, x_7_1, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
#ifdef STRICT
  pushRedex(Rslt_14Args, glblFn271);
#else
  swap(termLoc(Rslt_14), makePair(LAZ, 0, Rslt_14Args, glblFn271));
#endif

  // call vect-count at core: 717
  Term Rslt_15Args = makePair(APP, 0, y_8_1, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
#ifdef STRICT
  pushRedex(Rslt_15Args, glblFn271);
#else
  swap(termLoc(Rslt_15), makePair(LAZ, 0, Rslt_15Args, glblFn271));
#endif

  // call = at core: 717
  Term Rslt_16Args = makePair(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, Rslt_16Args));
  Rslt_16Args = makePair(APP, 0, Rslt_14, Rslt_16Args);
#ifdef STRICT
  pushRedex(Rslt_16Args, newRef(glblProto15_557));
#else
  swap(termLoc(Rslt_16),
       makePair(LAZ, 0, Rslt_16Args, newRef(glblProto15_557)));
#endif
  // call count at core: 718
  Term Rslt_17Args = makePair(APP, 0, x_7_3, SUB);
  Term Rslt_17 = newTerm(VAR, 0, portLoc(2, Rslt_17Args));
#ifdef STRICT
  pushRedex(Rslt_17Args, newRef(glblProto61_559));
#else
  swap(termLoc(Rslt_17),
       makePair(LAZ, 0, Rslt_17Args, newRef(glblProto61_559)));
#endif
  // call dec at core: 718
  Term Rslt_18Args = makePair(APP, 0, Rslt_17, SUB);
  Term Rslt_18 = newTerm(VAR, 0, portLoc(2, Rslt_18Args));
#ifdef STRICT
  pushRedex(Rslt_18Args, glblFn127);
#else
  swap(termLoc(Rslt_18), makePair(LAZ, 0, Rslt_18Args, glblFn127));
#endif

  // call vect-= at core: 718
  Term Rslt_19Args = makePair(APP, 0, Rslt_18, SUB);
  Term Rslt_19 = newTerm(VAR, 0, portLoc(2, Rslt_19Args));
  Rslt_19Args = makePair(APP, 0, y_8_2, Rslt_19Args);
  Rslt_19Args = makePair(APP, 0, x_7_2, Rslt_19Args);
#ifdef STRICT
  pushRedex(Rslt_19Args, glblFn475);
#else
  swap(termLoc(Rslt_19), makePair(LAZ, 0, Rslt_19Args, glblFn475));
#endif

  // call and at core: 717
  Term Rslt_20Args = makePair(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = newTerm(VAR, 0, portLoc(2, Rslt_20Args));
  Rslt_20Args = makePair(APP, 0, Rslt_16, Rslt_20Args);
#ifdef STRICT
  pushRedex(Rslt_20Args, newRef(glblProto25_561));
#else
  swap(termLoc(Rslt_20),
       makePair(LAZ, 0, Rslt_20Args, newRef(glblProto25_561)));
#endif
  // call and at core: 716
  Term Rslt_21Args = makePair(APP, 0, Rslt_20, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
  Rslt_21Args = makePair(APP, 0, Rslt_13, Rslt_21Args);
#ifdef STRICT
  pushRedex(Rslt_21Args, newRef(glblProto25_563));
#else
  swap(termLoc(Rslt_21),
       makePair(LAZ, 0, Rslt_21Args, newRef(glblProto25_563)));
#endif
  dupLabels[103] = "=";
  // link args to body
  swap(r_seq_9, Rslt_21);

  Term Rslt_21V = get(termLoc(Rslt_21));
  apps = take(portLoc(1, Rslt_21V));
  lams = take(portLoc(2, Rslt_21V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_21), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_9);
  return;
}
Term glbl_EQ_539 = newRef(glbl_EQ_538);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str8 = {StringBufferType, REFS_STATIC, 0, 0, 6, "Vector"};
Term glblStr540 = newTerm_(VAL, (Term)&str8);
Term glbltype_name537;
void glbltype_name536(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_24 = makePair(LAM, 102, SUB, NUL);
  Location r_seq_24 = portLoc(2, seq_24);
  Term arg_23 = newTerm(VAR, 0, portLoc(1, seq_24));
  // allocate args at core: 720
  swap(termLoc(arg_23), ERA);
  dupLabels[102] = "type-name";
  // link args to body
  swap(r_seq_24, glblStr540);

  V = get(r_seq_24);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 102, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_24, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_24);
  return;
}
Term glbltype_name537 = newRef(glbltype_name536);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str9 = {StringBufferType, REFS_STATIC, 0, 0, 1, "["};
Term glblStr542 = newTerm_(VAL, (Term)&str9);
void glblProto57_567(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(724), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto57);
  }
}
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str10 = {StringBufferType, REFS_STATIC, 0, 0, 1, "]"};
Term glblStr541 = newTerm_(VAL, (Term)&str10);
Term glblstr_vect535;
void glblstr_vect534(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_28 = makePair(LAM, 101, SUB, NUL);
  Location r_seq_28 = portLoc(2, seq_28);
  Term v_27 = newTerm(VAR, 0, portLoc(1, seq_28));
  // allocate args at core: 723
  Term glblVal131_11;
  glblVal131 = dupeArg(glblVal131, &glblVal131_11, 0);

  // call vect-conj at core: 724
  Term Rslt_30Args = makePair(APP, 0, glblStr542, SUB);
  Term Rslt_30 = newTerm(VAR, 0, portLoc(2, Rslt_30Args));
  Rslt_30Args = makePair(APP, 0, glblVal131_11, Rslt_30Args);
#ifdef STRICT
  pushRedex(Rslt_30Args, glblFn134);
#else
  swap(termLoc(Rslt_30), makePair(LAZ, 0, Rslt_30Args, glblFn134));
#endif

  // call interpose at core: 724
  Term Rslt_31Args = makePair(APP, 0, glblStr162, SUB);
  Term Rslt_31 = newTerm(VAR, 0, portLoc(2, Rslt_31Args));
  Rslt_31Args = makePair(APP, 0, v_27, Rslt_31Args);
#ifdef STRICT
  pushRedex(Rslt_31Args, newRef(glblProto57_567));
#else
  swap(termLoc(Rslt_31),
       makePair(LAZ, 0, Rslt_31Args, newRef(glblProto57_567)));
#endif
  // call to-str at core: 724
  Term Rslt_32Args = makePair(APP, 0, Rslt_31, SUB);
  Term Rslt_32 = newTerm(VAR, 0, portLoc(2, Rslt_32Args));
#ifdef STRICT
  pushRedex(Rslt_32Args, glblFn455);
#else
  swap(termLoc(Rslt_32), makePair(LAZ, 0, Rslt_32Args, glblFn455));
#endif

  // call vect-conj at core: 724
  Term Rslt_33Args = makePair(APP, 0, Rslt_32, SUB);
  Term Rslt_33 = newTerm(VAR, 0, portLoc(2, Rslt_33Args));
  Rslt_33Args = makePair(APP, 0, Rslt_30, Rslt_33Args);
#ifdef STRICT
  pushRedex(Rslt_33Args, glblFn134);
#else
  swap(termLoc(Rslt_33), makePair(LAZ, 0, Rslt_33Args, glblFn134));
#endif

  // call vect-conj at core: 724
  Term Rslt_34Args = makePair(APP, 0, glblStr541, SUB);
  Term Rslt_34 = newTerm(VAR, 0, portLoc(2, Rslt_34Args));
  Rslt_34Args = makePair(APP, 0, Rslt_33, Rslt_34Args);
#ifdef STRICT
  pushRedex(Rslt_34Args, glblFn134);
#else
  swap(termLoc(Rslt_34), makePair(LAZ, 0, Rslt_34Args, glblFn134));
#endif

  dupLabels[101] = "str-vect";
  // link args to body
  swap(r_seq_28, Rslt_34);

  Term Rslt_34V = get(termLoc(Rslt_34));
  apps = take(portLoc(1, Rslt_34V));
  lams = take(portLoc(2, Rslt_34V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_34), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_28);
  return;
}
Term glblstr_vect535 = newRef(glblstr_vect534);
Term glblcount533;
void glblcount532(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_37 = makePair(LAM, 100, SUB, NUL);
  Location r_seq_37 = portLoc(2, seq_37);
  Term l_36 = newTerm(VAR, 0, portLoc(1, seq_37));
  // allocate args at core: 726
  // call vect-count at core: 727
  Term Rslt_39Args = makePair(APP, 0, l_36, SUB);
  Term Rslt_39 = newTerm(VAR, 0, portLoc(2, Rslt_39Args));
#ifdef STRICT
  pushRedex(Rslt_39Args, glblFn271);
#else
  swap(termLoc(Rslt_39), makePair(LAZ, 0, Rslt_39Args, glblFn271));
#endif

  dupLabels[100] = "count";
  // link args to body
  swap(r_seq_37, Rslt_39);

  Term Rslt_39V = get(termLoc(Rslt_39));
  apps = take(portLoc(1, Rslt_39V));
  lams = take(portLoc(2, Rslt_39V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_39), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_37);
  return;
}
Term glblcount533 = newRef(glblcount532);
Term glblget531;
void glblget530(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_43 = makePair(LAM, 99, SUB, NUL);
  Location r_seq_43 = portLoc(2, seq_43);
  Term n_42 = newTerm(VAR, 0, portLoc(1, seq_43));
  seq_43 = makePair(LAM, 99, SUB, seq_43);
  Term v_41 = newTerm(VAR, 0, portLoc(1, seq_43));
  // allocate args at core: 729
  // call vect-get at core: 730
  Term Rslt_45Args = makePair(APP, 0, n_42, SUB);
  Term Rslt_45 = newTerm(VAR, 0, portLoc(2, Rslt_45Args));
  Rslt_45Args = makePair(APP, 0, v_41, Rslt_45Args);
#ifdef STRICT
  pushRedex(Rslt_45Args, glblFn376);
#else
  swap(termLoc(Rslt_45), makePair(LAZ, 0, Rslt_45Args, glblFn376));
#endif

  dupLabels[99] = "get";
  // link args to body
  swap(r_seq_43, Rslt_45);

  Term Rslt_45V = get(termLoc(Rslt_45));
  apps = take(portLoc(1, Rslt_45V));
  lams = take(portLoc(2, Rslt_45V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_45), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_43);
  return;
}
Term glblget531 = newRef(glblget530);
Term glblnth529;
void glblnth528(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_49 = makePair(LAM, 98, SUB, NUL);
  Location r_seq_49 = portLoc(2, seq_49);
  Term n_48 = newTerm(VAR, 0, portLoc(1, seq_49));
  seq_49 = makePair(LAM, 98, SUB, seq_49);
  Term v_47 = newTerm(VAR, 0, portLoc(1, seq_49));
  // allocate args at core: 732
  // call vect-get at core: 733
  Term Rslt_51Args = makePair(APP, 0, n_48, SUB);
  Term Rslt_51 = newTerm(VAR, 0, portLoc(2, Rslt_51Args));
  Rslt_51Args = makePair(APP, 0, v_47, Rslt_51Args);
#ifdef STRICT
  pushRedex(Rslt_51Args, glblFn376);
#else
  swap(termLoc(Rslt_51), makePair(LAZ, 0, Rslt_51Args, glblFn376));
#endif

  dupLabels[98] = "nth";
  // link args to body
  swap(r_seq_49, Rslt_51);

  Term Rslt_51V = get(termLoc(Rslt_51));
  apps = take(portLoc(1, Rslt_51V));
  lams = take(portLoc(2, Rslt_51V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_51), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_49);
  return;
}
Term glblnth529 = newRef(glblnth528);
void glblProto61_573(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(736), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
void glblProto25_576(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(736), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto25);
  }
}
Term glblempty_QM_527;
void glblempty_QM_526(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_54 = makePair(LAM, 97, SUB, NUL);
  Location r_seq_54 = portLoc(2, seq_54);
  Term v_53 = newTerm(VAR, 0, portLoc(1, seq_54));
  // allocate args at core: 735
  Term glblVal131_12;
  glblVal131 = dupeArg(glblVal131, &glblVal131_12, 0);

  Term glblVal22_4;
  glblVal22 = dupeArg(glblVal22, &glblVal22_4, 0);

  // call count at core: 736
  Term Rslt_56Args = makePair(APP, 0, v_53, SUB);
  Term Rslt_56 = newTerm(VAR, 0, portLoc(2, Rslt_56Args));
#ifdef STRICT
  pushRedex(Rslt_56Args, newRef(glblProto61_573));
#else
  swap(termLoc(Rslt_56),
       makePair(LAZ, 0, Rslt_56Args, newRef(glblProto61_573)));
#endif
  // call int-= at core: 736
  Term Rslt_57Args = makePair(APP, 0, Rslt_56, SUB);
  Term Rslt_57 = newTerm(VAR, 0, portLoc(2, Rslt_57Args));
  Rslt_57Args = makePair(APP, 0, newI60(0), Rslt_57Args);
#ifdef STRICT
  pushRedex(Rslt_57Args, glblFn205);
#else
  swap(termLoc(Rslt_57), makePair(LAZ, 0, Rslt_57Args, glblFn205));
#endif

  // call Some at core: 737
  Term Rslt_58Args = makePair(APP, 0, glblVal131_12, SUB);
  Term Rslt_58 = newTerm(VAR, 0, portLoc(2, Rslt_58Args));
#ifdef STRICT
  pushRedex(Rslt_58Args, glblVal22_4);
#else
  swap(termLoc(Rslt_58), makePair(LAZ, 0, Rslt_58Args, glblVal22_4));
#endif

  // call and at core: 736
  Term Rslt_59Args = makePair(APP, 0, Rslt_58, SUB);
  Term Rslt_59 = newTerm(VAR, 0, portLoc(2, Rslt_59Args));
  Rslt_59Args = makePair(APP, 0, Rslt_57, Rslt_59Args);
#ifdef STRICT
  pushRedex(Rslt_59Args, newRef(glblProto25_576));
#else
  swap(termLoc(Rslt_59),
       makePair(LAZ, 0, Rslt_59Args, newRef(glblProto25_576)));
#endif
  dupLabels[97] = "empty?";
  // link args to body
  swap(r_seq_54, Rslt_59);

  Term Rslt_59V = get(termLoc(Rslt_59));
  apps = take(portLoc(1, Rslt_59V));
  lams = take(portLoc(2, Rslt_59V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_59), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_54);
  return;
}
Term glblempty_QM_527 = newRef(glblempty_QM_526);
Term glblempty525;
void glblempty524(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_62 = makePair(LAM, 96, SUB, NUL);
  Location r_seq_62 = portLoc(2, seq_62);
  Term coll_61 = newTerm(VAR, 0, portLoc(1, seq_62));
  // allocate args at core: 739
  swap(termLoc(coll_61), ERA);
  Term glblVal131_13;
  glblVal131 = dupeArg(glblVal131, &glblVal131_13, 0);

  dupLabels[96] = "empty";
  // link args to body
  swap(r_seq_62, glblVal131_13);

  V = get(r_seq_62);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 96, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_62, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_62);
  return;
}
Term glblempty525 = newRef(glblempty524);
void glblProto69_583(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(743), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto69);
  }
}
Term glblcomp523;
void glblcomp522(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_67 = makePair(LAM, 95, SUB, NUL);
  Location r_seq_67 = portLoc(2, seq_67);
  Term v2_66 = newTerm(VAR, 0, portLoc(1, seq_67));
  seq_67 = makePair(LAM, 95, SUB, seq_67);
  Term v1_65 = newTerm(VAR, 0, portLoc(1, seq_67));
  // allocate args at core: 742
  // call reduce at core: 743
  Term Rslt_69Args = makePair(APP, 0, glblFn134, SUB);
  Term Rslt_69 = newTerm(VAR, 0, portLoc(2, Rslt_69Args));
  Rslt_69Args = makePair(APP, 0, v1_65, Rslt_69Args);
  Rslt_69Args = makePair(APP, 0, v2_66, Rslt_69Args);
#ifdef STRICT
  pushRedex(Rslt_69Args, newRef(glblProto69_583));
#else
  swap(termLoc(Rslt_69),
       makePair(LAZ, 0, Rslt_69Args, newRef(glblProto69_583)));
#endif
  dupLabels[95] = "comp";
  // link args to body
  swap(r_seq_67, Rslt_69);

  Term Rslt_69V = get(termLoc(Rslt_69));
  apps = take(portLoc(1, Rslt_69V));
  lams = take(portLoc(2, Rslt_69V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_69), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_67);
  return;
}
Term glblcomp523 = newRef(glblcomp522);
void glblProto101_589(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(746), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto101);
  }
}
Term glblfirst521;
void glblfirst520(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_72 = makePair(LAM, 94, SUB, NUL);
  Location r_seq_72 = portLoc(2, seq_72);
  Term v_71 = newTerm(VAR, 0, portLoc(1, seq_72));
  // allocate args at core: 745
  // call get at core: 746
  Term Rslt_74Args = makePair(APP, 0, newI60(0), SUB);
  Term Rslt_74 = newTerm(VAR, 0, portLoc(2, Rslt_74Args));
  Rslt_74Args = makePair(APP, 0, v_71, Rslt_74Args);
#ifdef STRICT
  pushRedex(Rslt_74Args, newRef(glblProto101_589));
#else
  swap(termLoc(Rslt_74),
       makePair(LAZ, 0, Rslt_74Args, newRef(glblProto101_589)));
#endif
  dupLabels[94] = "first";
  // link args to body
  swap(r_seq_72, Rslt_74);

  Term Rslt_74V = get(termLoc(Rslt_74));
  apps = take(portLoc(1, Rslt_74V));
  lams = take(portLoc(2, Rslt_74V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_74), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_72);
  return;
}
Term glblfirst521 = newRef(glblfirst520);
void glblProto61_596(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(749), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
void glblProto101_601(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(749), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto101);
  }
}
Term glbllast519;
void glbllast518(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_77 = makePair(LAM, 93, SUB, NUL);
  Location r_seq_77 = portLoc(2, seq_77);
  Term v_76 = newTerm(VAR, 0, portLoc(1, seq_77));
  // allocate args at core: 748

  Term v_76_1;
  v_76 = dupeArg(v_76, &v_76_1, 93);
  // call count at core: 749
  Term Rslt_79Args = makePair(APP, 0, v_76_1, SUB);
  Term Rslt_79 = newTerm(VAR, 0, portLoc(2, Rslt_79Args));
#ifdef STRICT
  pushRedex(Rslt_79Args, newRef(glblProto61_596));
#else
  swap(termLoc(Rslt_79),
       makePair(LAZ, 0, Rslt_79Args, newRef(glblProto61_596)));
#endif
  // call dec at core: 749
  Term Rslt_80Args = makePair(APP, 0, Rslt_79, SUB);
  Term Rslt_80 = newTerm(VAR, 0, portLoc(2, Rslt_80Args));
#ifdef STRICT
  pushRedex(Rslt_80Args, glblFn127);
#else
  swap(termLoc(Rslt_80), makePair(LAZ, 0, Rslt_80Args, glblFn127));
#endif

  // call get at core: 749
  Term Rslt_81Args = makePair(APP, 0, Rslt_80, SUB);
  Term Rslt_81 = newTerm(VAR, 0, portLoc(2, Rslt_81Args));
  Rslt_81Args = makePair(APP, 0, v_76, Rslt_81Args);
#ifdef STRICT
  pushRedex(Rslt_81Args, newRef(glblProto101_601));
#else
  swap(termLoc(Rslt_81),
       makePair(LAZ, 0, Rslt_81Args, newRef(glblProto101_601)));
#endif
  dupLabels[93] = "last";
  // link args to body
  swap(r_seq_77, Rslt_81);

  Term Rslt_81V = get(termLoc(Rslt_81));
  apps = take(portLoc(1, Rslt_81V));
  lams = take(portLoc(2, Rslt_81V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_81), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_77);
  return;
}
Term glbllast519 = newRef(glbllast518);
void glblProto61_606(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(752), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
Term glblrest517;
void glblrest516(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_84 = makePair(LAM, 92, SUB, NUL);
  Location r_seq_84 = portLoc(2, seq_84);
  Term v_83 = newTerm(VAR, 0, portLoc(1, seq_84));
  // allocate args at core: 751

  Term v_83_1;
  v_83 = dupeArg(v_83, &v_83_1, 92);
  // call count at core: 752
  Term Rslt_86Args = makePair(APP, 0, v_83_1, SUB);
  Term Rslt_86 = newTerm(VAR, 0, portLoc(2, Rslt_86Args));
#ifdef STRICT
  pushRedex(Rslt_86Args, newRef(glblProto61_606));
#else
  swap(termLoc(Rslt_86),
       makePair(LAZ, 0, Rslt_86Args, newRef(glblProto61_606)));
#endif
  // call dec at core: 752
  Term Rslt_87Args = makePair(APP, 0, Rslt_86, SUB);
  Term Rslt_87 = newTerm(VAR, 0, portLoc(2, Rslt_87Args));
#ifdef STRICT
  pushRedex(Rslt_87Args, glblFn127);
#else
  swap(termLoc(Rslt_87), makePair(LAZ, 0, Rslt_87Args, glblFn127));
#endif

  // call subvec at core: 752
  Term Rslt_88Args = makePair(APP, 0, Rslt_87, SUB);
  Term Rslt_88 = newTerm(VAR, 0, portLoc(2, Rslt_88Args));
  Rslt_88Args = makePair(APP, 0, newI60(1), Rslt_88Args);
  Rslt_88Args = makePair(APP, 0, v_83, Rslt_88Args);
#ifdef STRICT
  pushRedex(Rslt_88Args, glblFn449);
#else
  swap(termLoc(Rslt_88), makePair(LAZ, 0, Rslt_88Args, glblFn449));
#endif

  dupLabels[92] = "rest";
  // link args to body
  swap(r_seq_84, Rslt_88);

  Term Rslt_88V = get(termLoc(Rslt_88));
  apps = take(portLoc(1, Rslt_88V));
  lams = take(portLoc(2, Rslt_88V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_88), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_84);
  return;
}
Term glblrest517 = newRef(glblrest516);
Term glblconj515;
void glblconj514(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_92 = makePair(LAM, 91, SUB, NUL);
  Location r_seq_92 = portLoc(2, seq_92);
  Term v_91 = newTerm(VAR, 0, portLoc(1, seq_92));
  seq_92 = makePair(LAM, 91, SUB, seq_92);
  Term vect_90 = newTerm(VAR, 0, portLoc(1, seq_92));
  // allocate args at core: 754
  // call vect-conj at core: 755
  Term Rslt_94Args = makePair(APP, 0, v_91, SUB);
  Term Rslt_94 = newTerm(VAR, 0, portLoc(2, Rslt_94Args));
  Rslt_94Args = makePair(APP, 0, vect_90, Rslt_94Args);
#ifdef STRICT
  pushRedex(Rslt_94Args, glblFn134);
#else
  swap(termLoc(Rslt_94), makePair(LAZ, 0, Rslt_94Args, glblFn134));
#endif

  dupLabels[91] = "conj";
  // link args to body
  swap(r_seq_92, Rslt_94);

  Term Rslt_94V = get(termLoc(Rslt_94));
  apps = take(portLoc(1, Rslt_94V));
  lams = take(portLoc(2, Rslt_94V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_94), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_92);
  return;
}
Term glblconj515 = newRef(glblconj514);
Term glblstore513;
void glblstore512(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_99 = makePair(LAM, 90, SUB, NUL);
  Location r_seq_99 = portLoc(2, seq_99);
  Term v_98 = newTerm(VAR, 0, portLoc(1, seq_99));
  seq_99 = makePair(LAM, 90, SUB, seq_99);
  Term n_97 = newTerm(VAR, 0, portLoc(1, seq_99));
  seq_99 = makePair(LAM, 90, SUB, seq_99);
  Term vect_96 = newTerm(VAR, 0, portLoc(1, seq_99));
  // allocate args at core: 757
  // call vect-store at core: 758
  Term Rslt_101Args = makePair(APP, 0, v_98, SUB);
  Term Rslt_101 = newTerm(VAR, 0, portLoc(2, Rslt_101Args));
  Rslt_101Args = makePair(APP, 0, n_97, Rslt_101Args);
  Rslt_101Args = makePair(APP, 0, vect_96, Rslt_101Args);
#ifdef STRICT
  pushRedex(Rslt_101Args, glblFn477);
#else
  swap(termLoc(Rslt_101), makePair(LAZ, 0, Rslt_101Args, glblFn477));
#endif

  dupLabels[90] = "store";
  // link args to body
  swap(r_seq_99, Rslt_101);

  Term Rslt_101V = get(termLoc(Rslt_101));
  apps = take(portLoc(1, Rslt_101V));
  lams = take(portLoc(2, Rslt_101V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_101), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_99);
  return;
}
Term glblstore513 = newRef(glblstore512);
void glblProto73_611(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(761), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto73);
  }
}
void glblProto396_613(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(761), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
Term glblreduce511;
void glblreduce510(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_106 = makePair(LAM, 89, SUB, NUL);
  Location r_seq_106 = portLoc(2, seq_106);
  Term f_105 = newTerm(VAR, 0, portLoc(1, seq_106));
  seq_106 = makePair(LAM, 89, SUB, seq_106);
  Term result_104 = newTerm(VAR, 0, portLoc(1, seq_106));
  seq_106 = makePair(LAM, 89, SUB, seq_106);
  Term v_103 = newTerm(VAR, 0, portLoc(1, seq_106));
  // allocate args at core: 760

  Term v_103_1;
  v_103 = dupeArg(v_103, &v_103_1, 89);
  // call first at core: 761
  Term Rslt_108Args = makePair(APP, 0, v_103, SUB);
  Term Rslt_108 = newTerm(VAR, 0, portLoc(2, Rslt_108Args));
#ifdef STRICT
  pushRedex(Rslt_108Args, newRef(glblProto73_611));
#else
  swap(termLoc(Rslt_108),
       makePair(LAZ, 0, Rslt_108Args, newRef(glblProto73_611)));
#endif
  // call vect-reduce at core: 761
  Term Rslt_109Args = makePair(APP, 0, f_105, SUB);
  Term Rslt_109 = newTerm(VAR, 0, portLoc(2, Rslt_109Args));
  Rslt_109Args = makePair(APP, 0, result_104, Rslt_109Args);
  Rslt_109Args = makePair(APP, 0, newI60(1), Rslt_109Args);
  Rslt_109Args = makePair(APP, 0, v_103_1, Rslt_109Args);
  Rslt_109Args = makePair(APP, 0, Rslt_108, Rslt_109Args);
#ifdef STRICT
  pushRedex(Rslt_109Args, newRef(glblProto396_613));
#else
  swap(termLoc(Rslt_109),
       makePair(LAZ, 0, Rslt_109Args, newRef(glblProto396_613)));
#endif
  dupLabels[89] = "reduce";
  // link args to body
  swap(r_seq_106, Rslt_109);

  Term Rslt_109V = get(termLoc(Rslt_109));
  apps = take(portLoc(1, Rslt_109V));
  lams = take(portLoc(2, Rslt_109V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_109), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_106);
  return;
}
Term glblreduce511 = newRef(glblreduce510);
void glblProto77_616(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(764), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto77);
  }
}
void glblProto61_618(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(764), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
void glblProto403_620(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(764), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto403);
  }
}
Term glblreverse509;
void glblreverse508(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_112 = makePair(LAM, 88, SUB, NUL);
  Location r_seq_112 = portLoc(2, seq_112);
  Term v_111 = newTerm(VAR, 0, portLoc(1, seq_112));
  // allocate args at core: 763

  Term v_111_2;
  v_111 = dupeArg(v_111, &v_111_2, 88);

  Term v_111_1;
  v_111 = dupeArg(v_111, &v_111_1, 88);
  Term glblVal131_14;
  glblVal131 = dupeArg(glblVal131, &glblVal131_14, 0);

  // call last at core: 764
  Term Rslt_114Args = makePair(APP, 0, v_111, SUB);
  Term Rslt_114 = newTerm(VAR, 0, portLoc(2, Rslt_114Args));
#ifdef STRICT
  pushRedex(Rslt_114Args, newRef(glblProto77_616));
#else
  swap(termLoc(Rslt_114),
       makePair(LAZ, 0, Rslt_114Args, newRef(glblProto77_616)));
#endif
  // call count at core: 764
  Term Rslt_115Args = makePair(APP, 0, v_111_2, SUB);
  Term Rslt_115 = newTerm(VAR, 0, portLoc(2, Rslt_115Args));
#ifdef STRICT
  pushRedex(Rslt_115Args, newRef(glblProto61_618));
#else
  swap(termLoc(Rslt_115),
       makePair(LAZ, 0, Rslt_115Args, newRef(glblProto61_618)));
#endif
  Term Rslt_116 = makeOp(OP_SUB, Rslt_115, newI60(2));
  // call vect-reverse at core: 764
  Term Rslt_117Args = makePair(APP, 0, glblVal131_14, SUB);
  Term Rslt_117 = newTerm(VAR, 0, portLoc(2, Rslt_117Args));
  Rslt_117Args = makePair(APP, 0, Rslt_116, Rslt_117Args);
  Rslt_117Args = makePair(APP, 0, v_111_1, Rslt_117Args);
  Rslt_117Args = makePair(APP, 0, Rslt_114, Rslt_117Args);
#ifdef STRICT
  pushRedex(Rslt_117Args, newRef(glblProto403_620));
#else
  swap(termLoc(Rslt_117),
       makePair(LAZ, 0, Rslt_117Args, newRef(glblProto403_620)));
#endif
  dupLabels[88] = "reverse";
  // link args to body
  swap(r_seq_112, Rslt_117);

  Term Rslt_117V = get(termLoc(Rslt_117));
  apps = take(portLoc(1, Rslt_117V));
  lams = take(portLoc(2, Rslt_117V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_117), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_112);
  return;
}
Term glblreverse509 = newRef(glblreverse508);
Term glblmap507;
void glblmap506(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_121 = makePair(LAM, 87, SUB, NUL);
  Location r_seq_121 = portLoc(2, seq_121);
  Term f_120 = newTerm(VAR, 0, portLoc(1, seq_121));
  seq_121 = makePair(LAM, 87, SUB, seq_121);
  Term v_119 = newTerm(VAR, 0, portLoc(1, seq_121));
  // allocate args at core: 766
  Term glblVal450_1;
  glblVal450 = dupeArg(glblVal450, &glblVal450_1, 0);

  // call vect-map at core: 768
  Term Rslt_123Args = makePair(APP, 0, f_120, SUB);
  Term Rslt_123 = newTerm(VAR, 0, portLoc(2, Rslt_123Args));
  Rslt_123Args = makePair(APP, 0, v_119, Rslt_123Args);
#ifdef STRICT
  pushRedex(Rslt_123Args, glblVal450_1);
#else
  swap(termLoc(Rslt_123), makePair(LAZ, 0, Rslt_123Args, glblVal450_1));
#endif

  dupLabels[87] = "map";
  // link args to body
  swap(r_seq_121, Rslt_123);

  Term Rslt_123V = get(termLoc(Rslt_123));
  apps = take(portLoc(1, Rslt_123V));
  lams = take(portLoc(2, Rslt_123V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_123), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_121);
  return;
}
Term glblmap507 = newRef(glblmap506);
Term glblwrap505;
void glblwrap504(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_127 = makePair(LAM, 86, SUB, NUL);
  Location r_seq_127 = portLoc(2, seq_127);
  Term v_126 = newTerm(VAR, 0, portLoc(1, seq_127));
  seq_127 = makePair(LAM, 86, SUB, seq_127);
  Term arg_125 = newTerm(VAR, 0, portLoc(1, seq_127));
  // allocate args at core: 770
  swap(termLoc(arg_125), ERA);
  Term glblVal131_15;
  glblVal131 = dupeArg(glblVal131, &glblVal131_15, 0);

  // call vect-conj at core: 771
  Term Rslt_129Args = makePair(APP, 0, v_126, SUB);
  Term Rslt_129 = newTerm(VAR, 0, portLoc(2, Rslt_129Args));
  Rslt_129Args = makePair(APP, 0, glblVal131_15, Rslt_129Args);
#ifdef STRICT
  pushRedex(Rslt_129Args, glblFn134);
#else
  swap(termLoc(Rslt_129), makePair(LAZ, 0, Rslt_129Args, glblFn134));
#endif

  dupLabels[86] = "wrap";
  // link args to body
  swap(r_seq_127, Rslt_129);

  Term Rslt_129V = get(termLoc(Rslt_129));
  apps = take(portLoc(1, Rslt_129V));
  lams = take(portLoc(2, Rslt_129V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_129), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_127);
  return;
}
Term glblwrap505 = newRef(glblwrap504);
void glblProto45_625(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(774), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto45);
  }
}
Term glblflat_map503;
void glblflat_map502(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_133 = makePair(LAM, 85, SUB, NUL);
  Location r_seq_133 = portLoc(2, seq_133);
  Term f_132 = newTerm(VAR, 0, portLoc(1, seq_133));
  seq_133 = makePair(LAM, 85, SUB, seq_133);
  Term v_131 = newTerm(VAR, 0, portLoc(1, seq_133));
  // allocate args at core: 773
  Term glblVal450_2;
  glblVal450 = dupeArg(glblVal450, &glblVal450_2, 0);

  // call vect-map at core: 774
  Term Rslt_135Args = makePair(APP, 0, f_132, SUB);
  Term Rslt_135 = newTerm(VAR, 0, portLoc(2, Rslt_135Args));
  Rslt_135Args = makePair(APP, 0, v_131, Rslt_135Args);
#ifdef STRICT
  pushRedex(Rslt_135Args, glblVal450_2);
#else
  swap(termLoc(Rslt_135), makePair(LAZ, 0, Rslt_135Args, glblVal450_2));
#endif

  // call flatten at core: 774
  Term Rslt_136Args = makePair(APP, 0, Rslt_135, SUB);
  Term Rslt_136 = newTerm(VAR, 0, portLoc(2, Rslt_136Args));
#ifdef STRICT
  pushRedex(Rslt_136Args, newRef(glblProto45_625));
#else
  swap(termLoc(Rslt_136),
       makePair(LAZ, 0, Rslt_136Args, newRef(glblProto45_625)));
#endif
  dupLabels[85] = "flat-map";
  // link args to body
  swap(r_seq_133, Rslt_136);

  Term Rslt_136V = get(termLoc(Rslt_136));
  apps = take(portLoc(1, Rslt_136V));
  lams = take(portLoc(2, Rslt_136V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_136), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_133);
  return;
}
Term glblflat_map503 = newRef(glblflat_map502);
void glblProto73_628(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(777), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto73);
  }
}
void glblProto396_630(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(777), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
void glblProto35_632(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(777), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto35);
  }
}
Term glblflatten501;
void glblflatten500(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_139 = makePair(LAM, 84, SUB, NUL);
  Location r_seq_139 = portLoc(2, seq_139);
  Term v_138 = newTerm(VAR, 0, portLoc(1, seq_139));
  // allocate args at core: 776

  Term v_138_1;
  v_138 = dupeArg(v_138, &v_138_1, 84);
  Term glblVal131_16;
  glblVal131 = dupeArg(glblVal131, &glblVal131_16, 0);

  // call first at core: 777
  Term Rslt_141Args = makePair(APP, 0, v_138, SUB);
  Term Rslt_141 = newTerm(VAR, 0, portLoc(2, Rslt_141Args));
#ifdef STRICT
  pushRedex(Rslt_141Args, newRef(glblProto73_628));
#else
  swap(termLoc(Rslt_141),
       makePair(LAZ, 0, Rslt_141Args, newRef(glblProto73_628)));
#endif
  // call vect-reduce at core: 777
  Term Rslt_142Args = makePair(APP, 0, newRef(glblProto35_632), SUB);
  Term Rslt_142 = newTerm(VAR, 0, portLoc(2, Rslt_142Args));
  Rslt_142Args = makePair(APP, 0, glblVal131_16, Rslt_142Args);
  Rslt_142Args = makePair(APP, 0, newI60(1), Rslt_142Args);
  Rslt_142Args = makePair(APP, 0, v_138_1, Rslt_142Args);
  Rslt_142Args = makePair(APP, 0, Rslt_141, Rslt_142Args);
#ifdef STRICT
  pushRedex(Rslt_142Args, newRef(glblProto396_630));
#else
  swap(termLoc(Rslt_142),
       makePair(LAZ, 0, Rslt_142Args, newRef(glblProto396_630)));
#endif
  dupLabels[84] = "flatten";
  // link args to body
  swap(r_seq_139, Rslt_142);

  Term Rslt_142V = get(termLoc(Rslt_142));
  apps = take(portLoc(1, Rslt_142V));
  lams = take(portLoc(2, Rslt_142V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_142), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_139);
  return;
}
Term glblflatten501 = newRef(glblflatten500);
Term glblProto317;
Term glblProto579;
Term glblProto575;
void glblCons592(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(3), args);
  callArgs = makePair(APP, 0, newI60(45), callArgs);
  interact(callArgs, construct);
  return;
}
void glblEndOfList553(Term ref, Term args) {
  Term callArgs = makePair(APP, 0, newI60(0), args);
  callArgs = makePair(APP, 0, newI60(46), callArgs);
  interact(callArgs, construct);
  return;
}
void glblProto73_635(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(780), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto73);
  }
}
void glblProto65_637(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(782), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto65);
  }
}
void glblProto31_639(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(781), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto31);
  }
}
void glblProto396_641(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(780), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto396);
  }
}
Term glblfilter499;
void glblfilter498(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_146 = makePair(LAM, 83, SUB, NUL);
  Location r_seq_146 = portLoc(2, seq_146);
  Term pred_145 = newTerm(VAR, 0, portLoc(1, seq_146));
  seq_146 = makePair(LAM, 83, SUB, seq_146);
  Term v_144 = newTerm(VAR, 0, portLoc(1, seq_146));
  // allocate args at core: 779

  Term v_144_1;
  v_144 = dupeArg(v_144, &v_144_1, 83);
  Term seq_152 = makePair(LAM, 106, SUB, NUL);
  Location r_seq_152 = portLoc(2, seq_152);
  Term x_151 = newTerm(VAR, 0, portLoc(1, seq_152));
  seq_152 = makePair(LAM, 106, SUB, seq_152);
  Term result_150 = newTerm(VAR, 0, portLoc(1, seq_152));
  // allocate args at core: 780

  Term result_150_1;
  result_150 = dupeArg(result_150, &result_150_1, 106);

  Term x_151_1;
  x_151 = dupeArg(x_151, &x_151_1, 106);
  Term glblVal131_17;
  glblVal131 = dupeArg(glblVal131, &glblVal131_17, 0);

  // call first at core: 780
  Term Rslt_148Args = makePair(APP, 0, v_144, SUB);
  Term Rslt_148 = newTerm(VAR, 0, portLoc(2, Rslt_148Args));
#ifdef STRICT
  pushRedex(Rslt_148Args, newRef(glblProto73_635));
#else
  swap(termLoc(Rslt_148),
       makePair(LAZ, 0, Rslt_148Args, newRef(glblProto73_635)));
#endif
  // call pred at core: 779
  Term Rslt_154Args = makePair(APP, 0, x_151, SUB);
  Term Rslt_154 = newTerm(VAR, 0, portLoc(2, Rslt_154Args));
#ifdef STRICT
  pushRedex(Rslt_154Args, pred_145);
#else
  swap(termLoc(Rslt_154), makePair(LAZ, 0, Rslt_154Args, pred_145));
#endif

  // call conj at core: 782
  Term Rslt_155Args = makePair(APP, 0, x_151_1, SUB);
  Term Rslt_155 = newTerm(VAR, 0, portLoc(2, Rslt_155Args));
  Rslt_155Args = makePair(APP, 0, result_150, Rslt_155Args);
#ifdef STRICT
  pushRedex(Rslt_155Args, newRef(glblProto65_637));
#else
  swap(termLoc(Rslt_155),
       makePair(LAZ, 0, Rslt_155Args, newRef(glblProto65_637)));
#endif
  // call cond at core: 781
  Term Rslt_156Args = makePair(APP, 0, result_150_1, SUB);
  Term Rslt_156 = newTerm(VAR, 0, portLoc(2, Rslt_156Args));
  Rslt_156Args = makePair(APP, 0, Rslt_155, Rslt_156Args);
  Rslt_156Args = makePair(APP, 0, Rslt_154, Rslt_156Args);
#ifdef STRICT
  pushRedex(Rslt_156Args, newRef(glblProto31_639));
#else
  swap(termLoc(Rslt_156),
       makePair(LAZ, 0, Rslt_156Args, newRef(glblProto31_639)));
#endif
  dupLabels[106] = "";
  // link args to body
  swap(r_seq_152, Rslt_156);

  Term Rslt_156V = get(termLoc(Rslt_156));
  apps = take(portLoc(1, Rslt_156V));
  lams = take(portLoc(2, Rslt_156V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_156), makePair(LAZ, 0, apps, lams));
  // call vect-reduce at core: 780
  Term Rslt_157Args = makePair(APP, 0, seq_152, SUB);
  Term Rslt_157 = newTerm(VAR, 0, portLoc(2, Rslt_157Args));
  Rslt_157Args = makePair(APP, 0, glblVal131_17, Rslt_157Args);
  Rslt_157Args = makePair(APP, 0, newI60(1), Rslt_157Args);
  Rslt_157Args = makePair(APP, 0, v_144_1, Rslt_157Args);
  Rslt_157Args = makePair(APP, 0, Rslt_148, Rslt_157Args);
#ifdef STRICT
  pushRedex(Rslt_157Args, newRef(glblProto396_641));
#else
  swap(termLoc(Rslt_157),
       makePair(LAZ, 0, Rslt_157Args, newRef(glblProto396_641)));
#endif
  dupLabels[83] = "filter";
  // link args to body
  swap(r_seq_146, Rslt_157);

  Term Rslt_157V = get(termLoc(Rslt_157));
  apps = take(portLoc(1, Rslt_157V));
  lams = take(portLoc(2, Rslt_157V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_157), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_146);
  return;
}
Term glblfilter499 = newRef(glblfilter498);
void glblProto43_644(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(788), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto43);
  }
}
void glblProto75_646(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(789), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto75);
  }
}
Term glblinterpose497;
void glblinterpose496(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_162 = makePair(LAM, 82, SUB, NUL);
  Location r_seq_162 = portLoc(2, seq_162);
  Term sep_161 = newTerm(VAR, 0, portLoc(1, seq_162));
  seq_162 = makePair(LAM, 82, SUB, seq_162);
  Term coll_160 = newTerm(VAR, 0, portLoc(1, seq_162));
  // allocate args at core: 785
  Term seq_166 = makePair(LAM, 107, SUB, NUL);
  Location r_seq_166 = portLoc(2, seq_166);
  Term x_165 = newTerm(VAR, 0, portLoc(1, seq_166));
  // allocate args at core: 788
  Term glblVal131_18;
  glblVal131 = dupeArg(glblVal131, &glblVal131_18, 0);

  // call vect-conj at core: 788
  Term Rslt_168Args = makePair(APP, 0, sep_161, SUB);
  Term Rslt_168 = newTerm(VAR, 0, portLoc(2, Rslt_168Args));
  Rslt_168Args = makePair(APP, 0, glblVal131_18, Rslt_168Args);
#ifdef STRICT
  pushRedex(Rslt_168Args, glblFn134);
#else
  swap(termLoc(Rslt_168), makePair(LAZ, 0, Rslt_168Args, glblFn134));
#endif

  // call vect-conj at core: 788
  Term Rslt_169Args = makePair(APP, 0, x_165, SUB);
  Term Rslt_169 = newTerm(VAR, 0, portLoc(2, Rslt_169Args));
  Rslt_169Args = makePair(APP, 0, Rslt_168, Rslt_169Args);
#ifdef STRICT
  pushRedex(Rslt_169Args, glblFn134);
#else
  swap(termLoc(Rslt_169), makePair(LAZ, 0, Rslt_169Args, glblFn134));
#endif

  dupLabels[107] = "";
  // link args to body
  swap(r_seq_166, Rslt_169);

  Term Rslt_169V = get(termLoc(Rslt_169));
  apps = take(portLoc(1, Rslt_169V));
  lams = take(portLoc(2, Rslt_169V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_169), makePair(LAZ, 0, apps, lams));
  // call flat-map at core: 788
  Term Rslt_170Args = makePair(APP, 0, seq_166, SUB);
  Term Rslt_170 = newTerm(VAR, 0, portLoc(2, Rslt_170Args));
  Rslt_170Args = makePair(APP, 0, coll_160, Rslt_170Args);
#ifdef STRICT
  pushRedex(Rslt_170Args, newRef(glblProto43_644));
#else
  swap(termLoc(Rslt_170),
       makePair(LAZ, 0, Rslt_170Args, newRef(glblProto43_644)));
#endif
  // call rest at core: 789
  Term Rslt_172Args = makePair(APP, 0, Rslt_170, SUB);
  Term Rslt_172 = newTerm(VAR, 0, portLoc(2, Rslt_172Args));
#ifdef STRICT
  pushRedex(Rslt_172Args, newRef(glblProto75_646));
#else
  swap(termLoc(Rslt_172),
       makePair(LAZ, 0, Rslt_172Args, newRef(glblProto75_646)));
#endif
  dupLabels[82] = "interpose";
  // link args to body
  swap(r_seq_162, Rslt_172);

  Term Rslt_172V = get(termLoc(Rslt_172));
  apps = take(portLoc(1, Rslt_172V));
  lams = take(portLoc(2, Rslt_172V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_172), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_162);
  return;
}
Term glblinterpose497 = newRef(glblinterpose496);
Term glblVal556 = newRef(glblEndOfList553);
Term glblVal556;
Term glblRslt555;
Term glblrecurse549;
void glblrecurse548(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 108, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term f_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 108, SUB, seq_3);
  Term l_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 799
  swap(termLoc(f_2), ERA);
  dupLabels[108] = "recurse";
  // link args to body
  swap(r_seq_3, l_1);

  V = get(r_seq_3);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 108, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_3, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_3);
  return;
}
Term glblrecurse549 = newRef(glblrecurse548);
Term glblcount551;
void glblcount550(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_7 = makePair(LAM, 109, SUB, NUL);
  Location r_seq_7 = portLoc(2, seq_7);
  Term l_6 = newTerm(VAR, 0, portLoc(1, seq_7));
  // allocate args at core: 805
  swap(termLoc(l_6), ERA);
  dupLabels[109] = "count";
  // link args to body
  swap(r_seq_7, newI60(0));

  V = get(r_seq_7);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 109, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_7, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_7);
  return;
}
Term glblcount551 = newRef(glblcount550);
Term glblVal605 = newRef(glblCons592);
Term glblVal605;
Term glblrecurse586;
void glblrecurse585(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 110, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term f_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 110, SUB, seq_3);
  Term arg_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 812

  Term arg_1_2;
  arg_1 = dupeArg(arg_1, &arg_1_2, 110);

  Term arg_1_1;
  arg_1 = dupeArg(arg_1, &arg_1_1, 110);
  Term arg_1_2Args = makePair(APP, 0, arg_1_2, SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, arg_1_2Args));
  arg_1_2Args = makePair(APP, 0, newI60(2), arg_1_2Args);
  pushRedex(arg_1_2Args, accessField);
  Term arg_1_1Args = makePair(APP, 0, arg_1_1, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, arg_1_1Args));
  arg_1_1Args = makePair(APP, 0, newI60(1), arg_1_1Args);
  pushRedex(arg_1_1Args, accessField);
  Term arg_1Args = makePair(APP, 0, arg_1, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, arg_1Args));
  arg_1Args = makePair(APP, 0, newI60(0), arg_1Args);
  pushRedex(arg_1Args, accessField);
  Term glblVal605_1;
  glblVal605 = dupeArg(glblVal605, &glblVal605_1, 0);

  // call f at core: 812
  Term Rslt_7Args = makePair(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
#ifdef STRICT
  pushRedex(Rslt_7Args, f_2);
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, f_2));
#endif

  // call Cons at core: 813
  Term Rslt_9Args = makePair(APP, 0, Rslt_8, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
  Rslt_9Args = makePair(APP, 0, Rslt_7, Rslt_9Args);
  Rslt_9Args = makePair(APP, 0, Rslt_5, Rslt_9Args);
#ifdef STRICT
  pushRedex(Rslt_9Args, glblVal605_1);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, glblVal605_1));
#endif

  dupLabels[110] = "recurse";
  // link args to body
  swap(r_seq_3, Rslt_9);

  Term Rslt_9V = get(termLoc(Rslt_9));
  apps = take(portLoc(1, Rslt_9V));
  lams = take(portLoc(2, Rslt_9V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_9), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblrecurse586 = newRef(glblrecurse585);
void glblProto317_652(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(819), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto317);
  }
}
Term glblcount590;
void glblcount588(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_12 = makePair(LAM, 111, SUB, NUL);
  Location r_seq_12 = portLoc(2, seq_12);
  Term l_11 = newTerm(VAR, 0, portLoc(1, seq_12));
  // allocate args at core: 818
  // call .len at core: 819
  Term Rslt_14Args = makePair(APP, 0, l_11, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
#ifdef STRICT
  pushRedex(Rslt_14Args, newRef(glblProto317_652));
#else
  swap(termLoc(Rslt_14),
       makePair(LAZ, 0, Rslt_14Args, newRef(glblProto317_652)));
#endif
  dupLabels[111] = "count";
  // link args to body
  swap(r_seq_12, Rslt_14);

  Term Rslt_14V = get(termLoc(Rslt_14));
  apps = take(portLoc(1, Rslt_14V));
  lams = take(portLoc(2, Rslt_14V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_14), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_12);
  return;
}
Term glblcount590 = newRef(glblcount588);
Term glblFld595;
void glblFldFn593(Term ref, Term args) {
  args = makePair(APP, 0, newI60(0), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld595 = newRef(glblFldFn593);
Term glblFld599;
void glblFldFn597(Term ref, Term args) {
  args = makePair(APP, 0, newI60(1), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld599 = newRef(glblFldFn597);
Term glblFld602;
void glblFldFn600(Term ref, Term args) {
  args = makePair(APP, 0, newI60(2), args);
  pushRedex(args, accessField);
  return;
}
Term glblFld602 = newRef(glblFldFn600);
void glblProto61_655(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(823), args);
    args = makePair(APP, 0, glblStr151, args);
    interact(args, glblProto61);
  }
}
Term glblFn545;
void glblCFn544(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 104, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term l_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 104, SUB, seq_3);
  Term v_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at core: 821

  Term l_2_1;
  l_2 = dupeArg(l_2, &l_2_1, 104);
  Term glblVal605_2;
  glblVal605 = dupeArg(glblVal605, &glblVal605_2, 0);

  // call count at core: 823
  Term Rslt_5Args = makePair(APP, 0, l_2_1, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
#ifdef STRICT
  pushRedex(Rslt_5Args, newRef(glblProto61_655));
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, newRef(glblProto61_655)));
#endif
  // call inc at core: 823
  Term Rslt_6Args = makePair(APP, 0, Rslt_5, SUB);
  Term Rslt_6 = newTerm(VAR, 0, portLoc(2, Rslt_6Args));
#ifdef STRICT
  pushRedex(Rslt_6Args, glblFn120);
#else
  swap(termLoc(Rslt_6), makePair(LAZ, 0, Rslt_6Args, glblFn120));
#endif

  // call Cons at core: 823
  Term Rslt_7Args = makePair(APP, 0, Rslt_6, SUB);
  Term Rslt_7 = newTerm(VAR, 0, portLoc(2, Rslt_7Args));
  Rslt_7Args = makePair(APP, 0, l_2, Rslt_7Args);
  Rslt_7Args = makePair(APP, 0, v_1, Rslt_7Args);
#ifdef STRICT
  pushRedex(Rslt_7Args, glblVal605_2);
#else
  swap(termLoc(Rslt_7), makePair(LAZ, 0, Rslt_7Args, glblVal605_2));
#endif

  dupLabels[104] = "";
  // link args to body
  swap(r_seq_3, Rslt_7);

  Term Rslt_7V = get(termLoc(Rslt_7));
  apps = take(portLoc(1, Rslt_7V));
  lams = take(portLoc(2, Rslt_7V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_7), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn545 = newRef(glblCFn544);
Term glblFn547;
void glblCFn546(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 105, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term vs_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at core: 825
  // call to-str at core: 829
  Term Rslt_4Args = makePair(APP, 0, vs_1, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
#ifdef STRICT
  pushRedex(Rslt_4Args, glblFn455);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, glblFn455));
#endif

  dupLabels[105] = "";
  // link args to body
  swap(r_seq_2, Rslt_4);

  Term Rslt_4V = get(termLoc(Rslt_4));
  apps = take(portLoc(1, Rslt_4V));
  lams = take(portLoc(2, Rslt_4V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_4), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn547 = newRef(glblCFn546);
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
Term glblFn660;
void glblCFn659(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term str_1 = arityArgs.args[0];

    if (termTag(str_1) != VAL) {
      fprintf(stderr, "\ninvalid type for 'pr*': Integer or Float\n");
      abort();
    }
    char dest[100];
    int len;
    char *src;
    int num = 0;
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      src = ((String *)s)->buffer;
      len = (int)((String *)s)->len;
    } else if (s->type == SubStringType) {
      ReifiedVal *ss = (ReifiedVal *)str_1;
      String *parent = (String *)ss->impls[0];
      long start = getI60(ss->impls[1]);
      len = (int)getI60(ss->impls[2]);
      src = &parent->buffer[start];
    } else {
      fprintf(stdout, "\ninvalid type for 'pr*': %ld\n", s->type);
      abort();
    }
    strncpy(dest, src, len);
    dest[len] = 0;
    sscanf(dest, "%d", &num);
    dec_and_free(str_1, 1);
    result = newI60(num);
    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn660 = newRef(glblCFn659);
Term glblFn663;
void glblCFn662(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term y_2 = arityArgs.args[1];

    unsigned x = (unsigned)getI60(x_1);
    unsigned y = (unsigned)getI60(y_2);
    result = newI60(x & y);

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn663 = newRef(glblCFn662);
Term glblFn666;
void glblCFn664(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 1) {
    Term x_1 = arityArgs.args[0];

    unsigned x = (unsigned)getI60(x_1);
    result = newI60(x >> 1);

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn666 = newRef(glblCFn664);
Term glblFn668;
void glblCFn667(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
  if (arityArgs.count == 2) {
    Term x_1 = arityArgs.args[0];
    Term n_2 = arityArgs.args[1];

    unsigned x = (unsigned)getI60(x_1);
    unsigned n = (unsigned)getI60(n_2);
    result = newI60(x >> n);

    move(portLoc(2, args), result);
  }
  return;
}
Term glblFn668 = newRef(glblCFn667);
Term glblVal670;
Term glblVal672;
Term glblFn674;
void glblCFn673(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 116, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term p_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 56
  swap(termLoc(p_1), ERA);
  // call pr* at sidequest.toc: 57
  Term Rslt_4Args = makePair(APP, 0, glblStr161, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
#ifdef STRICT
  pushRedex(Rslt_4Args, glblFn115);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, glblFn115));
#endif

  dupLabels[116] = "";
  // link args to body
  swap(r_seq_2, Rslt_4);

  Term Rslt_4V = get(termLoc(Rslt_4));
  apps = take(portLoc(1, Rslt_4V));
  lams = take(portLoc(2, Rslt_4V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_4), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn674 = newRef(glblCFn673);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str11 = {StringBufferType, REFS_STATIC, 0, 0, 1, "0"};
Term glblStr679 = newTerm_(VAL, (Term)&str11);
Term glblVal670;
void glblCFn677(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 117, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term p_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 59
  Term glblVal672_1;
  glblVal672 = dupeArg(glblVal672, &glblVal672_1, 0);

  // call pr* at sidequest.toc: 60
  Term Rslt_4Args = makePair(APP, 0, glblStr679, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
#ifdef STRICT
  pushRedex(Rslt_4Args, glblFn115);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, glblFn115));
#endif

  // call p at sidequest.toc: 59
  Term Rslt_5Args = makePair(APP, 0, glblFn674, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, glblVal672_1, Rslt_5Args);
  Rslt_5Args = makePair(APP, 0, glblVal670, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, p_1);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, p_1));
#endif

  dupLabels[117] = "";
  // link args to body
  swap(r_seq_2, Rslt_5);

  Term Rslt_5V = get(termLoc(Rslt_5));
  apps = take(portLoc(1, Rslt_5V));
  lams = take(portLoc(2, Rslt_5V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_4, apps);
  lams = makePair(LAM, 117, ERA, lams);

  swap(termLoc(Rslt_5), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblVal670 = newRef(glblCFn677);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[2];
} str12 = {StringBufferType, REFS_STATIC, 0, 0, 1, "1"};
Term glblStr683 = newTerm_(VAL, (Term)&str12);
Term glblVal672;
void glblCFn681(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 118, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term p_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 63
  // call pr* at sidequest.toc: 64
  Term Rslt_4Args = makePair(APP, 0, glblStr683, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
#ifdef STRICT
  pushRedex(Rslt_4Args, glblFn115);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, glblFn115));
#endif

  // call p at sidequest.toc: 63
  Term Rslt_5Args = makePair(APP, 0, glblFn674, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, glblVal672, Rslt_5Args);
  Rslt_5Args = makePair(APP, 0, glblVal670, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, p_1);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, p_1));
#endif

  dupLabels[118] = "";
  // link args to body
  swap(r_seq_2, Rslt_5);

  Term Rslt_5V = get(termLoc(Rslt_5));
  apps = take(portLoc(1, Rslt_5V));
  lams = take(portLoc(2, Rslt_5V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_4, apps);
  lams = makePair(LAM, 118, ERA, lams);

  swap(termLoc(Rslt_5), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblVal672 = newRef(glblCFn681);
Term glblFn686;
void glblCFn685(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 119, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term bits_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 67
  // call bits at sidequest.toc: 67
  Term Rslt_4Args = makePair(APP, 0, glblFn674, SUB);
  Term Rslt_4 = newTerm(VAR, 0, portLoc(2, Rslt_4Args));
  Rslt_4Args = makePair(APP, 0, glblVal672, Rslt_4Args);
  Rslt_4Args = makePair(APP, 0, glblVal670, Rslt_4Args);
#ifdef STRICT
  pushRedex(Rslt_4Args, bits_1);
#else
  swap(termLoc(Rslt_4), makePair(LAZ, 0, Rslt_4Args, bits_1));
#endif

  // call  at sidequest.toc: 67
  Term Rslt_5Args = makePair(APP, 0, NUL, SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
#ifdef STRICT
  pushRedex(Rslt_5Args, Rslt_4);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, Rslt_4));
#endif

  dupLabels[119] = "";
  // link args to body
  swap(r_seq_2, Rslt_5);

  Term Rslt_5V = get(termLoc(Rslt_5));
  apps = take(portLoc(1, Rslt_5V));
  lams = take(portLoc(2, Rslt_5V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_5), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn686 = newRef(glblCFn685);
Term glblhbin_done691;
void glblhbin_done690(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_46 = makePair(LAM, 123, SUB, NUL);
  Location r_seq_46 = portLoc(2, seq_46);
  Term n_45 = newTerm(VAR, 0, portLoc(1, seq_46));
  // allocate args at sidequest.toc: 83
  swap(termLoc(n_45), ERA);
  Term seq_52 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_52 = portLoc(2, seq_52);
  Term e_51 = newTerm(VAR, 0, portLoc(1, seq_52));
  seq_52 = makePair(LAM, 0, SUB, seq_52);
  Term i_50 = newTerm(VAR, 0, portLoc(1, seq_52));
  seq_52 = makePair(LAM, 0, SUB, seq_52);
  Term o_49 = newTerm(VAR, 0, portLoc(1, seq_52));
  // allocate args at sidequest.toc: 84
  swap(termLoc(o_49), ERA);
  swap(termLoc(i_50), ERA);
  dupLabels[0] = "hbin-e";
  // link args to body
  swap(r_seq_52, e_51);

  V = get(r_seq_52);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_52, newTerm(VAR, 0, vLoc));
  dupLabels[123] = "hbin-done";
  // link args to body
  swap(r_seq_46, seq_52);

  V = get(r_seq_46);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 123, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_46, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_46);
  return;
}
Term glblhbin_done691 = newRef(glblhbin_done690);
Term glblFn689;
void glblCFn688(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 120, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term N_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 70

  Term N_1_1;
  N_1 = dupeArg(N_1, &N_1_1, 120);
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term l_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at sidequest.toc: 72

  Term l_5_1;
  l_5 = dupeArg(l_5, &l_5_1, 0);
  Term seq_10 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_10 = portLoc(2, seq_10);
  Term n_9 = newTerm(VAR, 0, portLoc(1, seq_10));
  // allocate args at sidequest.toc: 73

  Term n_9_2;
  n_9 = dupeArg(n_9, &n_9_2, 0);

  Term n_9_1;
  n_9 = dupeArg(n_9, &n_9_1, 0);
  Term seq_14 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_14 = portLoc(2, seq_14);
  Term x_13 = newTerm(VAR, 0, portLoc(1, seq_14));
  // allocate args at sidequest.toc: 74
  Term seq_31 = makePair(LAM, 122, SUB, NUL);
  Location r_seq_31 = portLoc(2, seq_31);
  Term e_30 = newTerm(VAR, 0, portLoc(1, seq_31));
  seq_31 = makePair(LAM, 122, SUB, seq_31);
  Term i_29 = newTerm(VAR, 0, portLoc(1, seq_31));
  seq_31 = makePair(LAM, 122, SUB, seq_31);
  Term o_28 = newTerm(VAR, 0, portLoc(1, seq_31));
  // allocate args at sidequest.toc: 78
  swap(termLoc(i_29), ERA);
  swap(termLoc(e_30), ERA);
  Term seq_20 = makePair(LAM, 121, SUB, NUL);
  Location r_seq_20 = portLoc(2, seq_20);
  Term e_19 = newTerm(VAR, 0, portLoc(1, seq_20));
  seq_20 = makePair(LAM, 121, SUB, seq_20);
  Term i_18 = newTerm(VAR, 0, portLoc(1, seq_20));
  seq_20 = makePair(LAM, 121, SUB, seq_20);
  Term o_17 = newTerm(VAR, 0, portLoc(1, seq_20));
  // allocate args at sidequest.toc: 76
  swap(termLoc(o_17), ERA);
  swap(termLoc(e_19), ERA);
  Term glblVal214_3;
  glblVal214 = dupeArg(glblVal214, &glblVal214_3, 0);

  Term glblVal214_2;
  glblVal214 = dupeArg(glblVal214, &glblVal214_2, 0);

  Term Rslt_22 = makeOp(OP_SUB, l_5, newI60(1));
  // call hbin at sidequest.toc: 70
  Term Rslt_23Args = makePair(APP, 0, Rslt_22, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
#ifdef STRICT
  pushRedex(Rslt_23Args, glblFn689);
#else
  swap(termLoc(Rslt_23), makePair(LAZ, 0, Rslt_23Args, glblFn689));
#endif

  // call /2 at sidequest.toc: 77
  Term Rslt_24Args = makePair(APP, 0, n_9, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
#ifdef STRICT
  pushRedex(Rslt_24Args, glblFn666);
#else
  swap(termLoc(Rslt_24), makePair(LAZ, 0, Rslt_24Args, glblFn666));
#endif

  // call  at sidequest.toc: 77
  Term Rslt_25Args = makePair(APP, 0, Rslt_24, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
#ifdef STRICT
  pushRedex(Rslt_25Args, Rslt_23);
#else
  swap(termLoc(Rslt_25), makePair(LAZ, 0, Rslt_25Args, Rslt_23));
#endif

  // call i at sidequest.toc: 76
  Term Rslt_26Args = makePair(APP, 0, Rslt_25, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
#ifdef STRICT
  pushRedex(Rslt_26Args, i_18);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, i_18));
#endif

  dupLabels[121] = "hbin-1";
  // link args to body
  swap(r_seq_20, Rslt_26);

  Term Rslt_26V = get(termLoc(Rslt_26));
  apps = take(portLoc(1, Rslt_26V));
  lams = take(portLoc(2, Rslt_26V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_26), makePair(LAZ, 0, apps, lams));
  Term Rslt_33 = makeOp(OP_SUB, l_5_1, newI60(1));
  // call hbin at sidequest.toc: 70
  Term Rslt_34Args = makePair(APP, 0, Rslt_33, SUB);
  Term Rslt_34 = newTerm(VAR, 0, portLoc(2, Rslt_34Args));
#ifdef STRICT
  pushRedex(Rslt_34Args, glblFn689);
#else
  swap(termLoc(Rslt_34), makePair(LAZ, 0, Rslt_34Args, glblFn689));
#endif

  // call /2 at sidequest.toc: 79
  Term Rslt_35Args = makePair(APP, 0, n_9_1, SUB);
  Term Rslt_35 = newTerm(VAR, 0, portLoc(2, Rslt_35Args));
#ifdef STRICT
  pushRedex(Rslt_35Args, glblFn666);
#else
  swap(termLoc(Rslt_35), makePair(LAZ, 0, Rslt_35Args, glblFn666));
#endif

  // call  at sidequest.toc: 79
  Term Rslt_36Args = makePair(APP, 0, Rslt_35, SUB);
  Term Rslt_36 = newTerm(VAR, 0, portLoc(2, Rslt_36Args));
#ifdef STRICT
  pushRedex(Rslt_36Args, Rslt_34);
#else
  swap(termLoc(Rslt_36), makePair(LAZ, 0, Rslt_36Args, Rslt_34));
#endif

  // call o at sidequest.toc: 78
  Term Rslt_37Args = makePair(APP, 0, Rslt_36, SUB);
  Term Rslt_37 = newTerm(VAR, 0, portLoc(2, Rslt_37Args));
#ifdef STRICT
  pushRedex(Rslt_37Args, o_28);
#else
  swap(termLoc(Rslt_37), makePair(LAZ, 0, Rslt_37Args, o_28));
#endif

  dupLabels[122] = "hbin-0";
  // link args to body
  swap(r_seq_31, Rslt_37);

  Term Rslt_37V = get(termLoc(Rslt_37));
  apps = take(portLoc(1, Rslt_37V));
  lams = take(portLoc(2, Rslt_37V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_37), makePair(LAZ, 0, apps, lams));
  // call int-cond at sidequest.toc: 75
  Term Rslt_38Args = makePair(APP, 0, seq_31, SUB);
  Term Rslt_38 = newTerm(VAR, 0, portLoc(2, Rslt_38Args));
  Rslt_38Args = makePair(APP, 0, seq_20, Rslt_38Args);
  Rslt_38Args = makePair(APP, 0, x_13, Rslt_38Args);
#ifdef STRICT
  pushRedex(Rslt_38Args, glblVal214_3);
#else
  swap(termLoc(Rslt_38), makePair(LAZ, 0, Rslt_38Args, glblVal214_3));
#endif

  dupLabels[0] = "hbin-cond";
  // link args to body
  swap(r_seq_14, Rslt_38);

  Term Rslt_38V = get(termLoc(Rslt_38));
  apps = take(portLoc(1, Rslt_38V));
  lams = take(portLoc(2, Rslt_38V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_38), makePair(LAZ, 0, apps, lams));
  // call bit-and at sidequest.toc: 80
  Term Rslt_41Args = makePair(APP, 0, newI60(1), SUB);
  Term Rslt_41 = newTerm(VAR, 0, portLoc(2, Rslt_41Args));
  Rslt_41Args = makePair(APP, 0, n_9_2, Rslt_41Args);
#ifdef STRICT
  pushRedex(Rslt_41Args, glblFn663);
#else
  swap(termLoc(Rslt_41), makePair(LAZ, 0, Rslt_41Args, glblFn663));
#endif

  // call hbin-cond at sidequest.toc: 80
  Term Rslt_42Args = makePair(APP, 0, Rslt_41, SUB);
  Term Rslt_42 = newTerm(VAR, 0, portLoc(2, Rslt_42Args));
#ifdef STRICT
  pushRedex(Rslt_42Args, seq_14);
#else
  swap(termLoc(Rslt_42), makePair(LAZ, 0, Rslt_42Args, seq_14));
#endif

  dupLabels[0] = "hbin-n";
  // link args to body
  swap(r_seq_10, Rslt_42);

  Term Rslt_42V = get(termLoc(Rslt_42));
  apps = take(portLoc(1, Rslt_42V));
  lams = take(portLoc(2, Rslt_42V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_42), makePair(LAZ, 0, apps, lams));
  dupLabels[0] = "hbin-step";
  // link args to body
  swap(r_seq_6, seq_10);

  V = get(r_seq_6);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_6, newTerm(VAR, 0, vLoc));
  // call hbin-step at sidequest.toc: 70
  Term Rslt_43Args = makePair(APP, 0, N_1_1, SUB);
  Term Rslt_43 = newTerm(VAR, 0, portLoc(2, Rslt_43Args));
#ifdef STRICT
  pushRedex(Rslt_43Args, seq_6);
#else
  swap(termLoc(Rslt_43), makePair(LAZ, 0, Rslt_43Args, seq_6));
#endif

  // call int-cond at sidequest.toc: 71
  Term Rslt_54Args = makePair(APP, 0, glblhbin_done691, SUB);
  Term Rslt_54 = newTerm(VAR, 0, portLoc(2, Rslt_54Args));
  Rslt_54Args = makePair(APP, 0, Rslt_43, Rslt_54Args);
  Rslt_54Args = makePair(APP, 0, N_1, Rslt_54Args);
#ifdef STRICT
  pushRedex(Rslt_54Args, glblVal214_2);
#else
  swap(termLoc(Rslt_54), makePair(LAZ, 0, Rslt_54Args, glblVal214_2));
#endif

  dupLabels[120] = "";
  // link args to body
  swap(r_seq_2, Rslt_54);

  Term Rslt_54V = get(termLoc(Rslt_54));
  apps = take(portLoc(1, Rslt_54V));
  lams = take(portLoc(2, Rslt_54V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_54), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn689 = newRef(glblCFn688);
Term glblbin_0698;
void glblbin_0697(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_33 = makePair(LAM, 127, SUB, NUL);
  Location r_seq_33 = portLoc(2, seq_33);
  Term e_32 = newTerm(VAR, 0, portLoc(1, seq_33));
  seq_33 = makePair(LAM, 127, SUB, seq_33);
  Term i_31 = newTerm(VAR, 0, portLoc(1, seq_33));
  seq_33 = makePair(LAM, 127, SUB, seq_33);
  Term o_30 = newTerm(VAR, 0, portLoc(1, seq_33));
  // allocate args at sidequest.toc: 100
  swap(termLoc(o_30), ERA);
  swap(termLoc(i_31), ERA);
  dupLabels[127] = "bin-0";
  // link args to body
  swap(r_seq_33, e_32);

  V = get(r_seq_33);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 127, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_33, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_33);
  return;
}
Term glblbin_0698 = newRef(glblbin_0697);
Term glblFn695;
void glblCFn694(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 124, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term v_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 124, SUB, seq_3);
  Term bits_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at sidequest.toc: 87

  Term bits_1_2;
  bits_1 = dupeArg(bits_1, &bits_1_2, 124);

  Term bits_1_1;
  bits_1 = dupeArg(bits_1, &bits_1_1, 124);

  Term v_2_2;
  v_2 = dupeArg(v_2, &v_2_2, 124);

  Term v_2_1;
  v_2 = dupeArg(v_2, &v_2_1, 124);
  Term seq_20 = makePair(LAM, 126, SUB, NUL);
  Location r_seq_20 = portLoc(2, seq_20);
  Term e_19 = newTerm(VAR, 0, portLoc(1, seq_20));
  seq_20 = makePair(LAM, 126, SUB, seq_20);
  Term i_18 = newTerm(VAR, 0, portLoc(1, seq_20));
  seq_20 = makePair(LAM, 126, SUB, seq_20);
  Term o_17 = newTerm(VAR, 0, portLoc(1, seq_20));
  // allocate args at sidequest.toc: 96
  swap(termLoc(i_18), ERA);
  swap(termLoc(e_19), ERA);
  Term seq_10 = makePair(LAM, 125, SUB, NUL);
  Location r_seq_10 = portLoc(2, seq_10);
  Term e_9 = newTerm(VAR, 0, portLoc(1, seq_10));
  seq_10 = makePair(LAM, 125, SUB, seq_10);
  Term i_8 = newTerm(VAR, 0, portLoc(1, seq_10));
  seq_10 = makePair(LAM, 125, SUB, seq_10);
  Term o_7 = newTerm(VAR, 0, portLoc(1, seq_10));
  // allocate args at sidequest.toc: 91
  swap(termLoc(o_7), ERA);
  swap(termLoc(e_9), ERA);
  Term glblVal214_5;
  glblVal214 = dupeArg(glblVal214, &glblVal214_5, 0);

  Term glblVal214_4;
  glblVal214 = dupeArg(glblVal214, &glblVal214_4, 0);

  // call bit-and at sidequest.toc: 89
  Term Rslt_5Args = makePair(APP, 0, newI60(1), SUB);
  Term Rslt_5 = newTerm(VAR, 0, portLoc(2, Rslt_5Args));
  Rslt_5Args = makePair(APP, 0, v_2, Rslt_5Args);
#ifdef STRICT
  pushRedex(Rslt_5Args, glblFn663);
#else
  swap(termLoc(Rslt_5), makePair(LAZ, 0, Rslt_5Args, glblFn663));
#endif

  // call dec at sidequest.toc: 93
  Term Rslt_12Args = makePair(APP, 0, bits_1_1, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
#ifdef STRICT
  pushRedex(Rslt_12Args, glblFn127);
#else
  swap(termLoc(Rslt_12), makePair(LAZ, 0, Rslt_12Args, glblFn127));
#endif

  // call /2 at sidequest.toc: 93
  Term Rslt_13Args = makePair(APP, 0, v_2_1, SUB);
  Term Rslt_13 = newTerm(VAR, 0, portLoc(2, Rslt_13Args));
#ifdef STRICT
  pushRedex(Rslt_13Args, glblFn666);
#else
  swap(termLoc(Rslt_13), makePair(LAZ, 0, Rslt_13Args, glblFn666));
#endif

  // call bin at sidequest.toc: 87
  Term Rslt_14Args = makePair(APP, 0, Rslt_13, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
  Rslt_14Args = makePair(APP, 0, Rslt_12, Rslt_14Args);
#ifdef STRICT
  pushRedex(Rslt_14Args, glblFn695);
#else
  swap(termLoc(Rslt_14), makePair(LAZ, 0, Rslt_14Args, glblFn695));
#endif

  // call i at sidequest.toc: 91
  Term Rslt_15Args = makePair(APP, 0, Rslt_14, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
#ifdef STRICT
  pushRedex(Rslt_15Args, i_8);
#else
  swap(termLoc(Rslt_15), makePair(LAZ, 0, Rslt_15Args, i_8));
#endif

  dupLabels[125] = "bin-1-1";
  // link args to body
  swap(r_seq_10, Rslt_15);

  Term Rslt_15V = get(termLoc(Rslt_15));
  apps = take(portLoc(1, Rslt_15V));
  lams = take(portLoc(2, Rslt_15V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_15), makePair(LAZ, 0, apps, lams));
  // call dec at sidequest.toc: 98
  Term Rslt_22Args = makePair(APP, 0, bits_1_2, SUB);
  Term Rslt_22 = newTerm(VAR, 0, portLoc(2, Rslt_22Args));
#ifdef STRICT
  pushRedex(Rslt_22Args, glblFn127);
#else
  swap(termLoc(Rslt_22), makePair(LAZ, 0, Rslt_22Args, glblFn127));
#endif

  // call /2 at sidequest.toc: 98
  Term Rslt_23Args = makePair(APP, 0, v_2_2, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
#ifdef STRICT
  pushRedex(Rslt_23Args, glblFn666);
#else
  swap(termLoc(Rslt_23), makePair(LAZ, 0, Rslt_23Args, glblFn666));
#endif

  // call bin at sidequest.toc: 87
  Term Rslt_24Args = makePair(APP, 0, Rslt_23, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
  Rslt_24Args = makePair(APP, 0, Rslt_22, Rslt_24Args);
#ifdef STRICT
  pushRedex(Rslt_24Args, glblFn695);
#else
  swap(termLoc(Rslt_24), makePair(LAZ, 0, Rslt_24Args, glblFn695));
#endif

  // call o at sidequest.toc: 96
  Term Rslt_25Args = makePair(APP, 0, Rslt_24, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
#ifdef STRICT
  pushRedex(Rslt_25Args, o_17);
#else
  swap(termLoc(Rslt_25), makePair(LAZ, 0, Rslt_25Args, o_17));
#endif

  dupLabels[126] = "bin-1-1";
  // link args to body
  swap(r_seq_20, Rslt_25);

  Term Rslt_25V = get(termLoc(Rslt_25));
  apps = take(portLoc(1, Rslt_25V));
  lams = take(portLoc(2, Rslt_25V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_25), makePair(LAZ, 0, apps, lams));
  // call int-cond at sidequest.toc: 89
  Term Rslt_26Args = makePair(APP, 0, seq_20, SUB);
  Term Rslt_26 = newTerm(VAR, 0, portLoc(2, Rslt_26Args));
  Rslt_26Args = makePair(APP, 0, seq_10, Rslt_26Args);
  Rslt_26Args = makePair(APP, 0, Rslt_5, Rslt_26Args);
#ifdef STRICT
  pushRedex(Rslt_26Args, glblVal214_5);
#else
  swap(termLoc(Rslt_26), makePair(LAZ, 0, Rslt_26Args, glblVal214_5));
#endif

  // call int-cond at sidequest.toc: 88
  Term Rslt_35Args = makePair(APP, 0, glblbin_0698, SUB);
  Term Rslt_35 = newTerm(VAR, 0, portLoc(2, Rslt_35Args));
  Rslt_35Args = makePair(APP, 0, Rslt_26, Rslt_35Args);
  Rslt_35Args = makePair(APP, 0, bits_1, Rslt_35Args);
#ifdef STRICT
  pushRedex(Rslt_35Args, glblVal214_4);
#else
  swap(termLoc(Rslt_35), makePair(LAZ, 0, Rslt_35Args, glblVal214_4));
#endif

  dupLabels[124] = "";
  // link args to body
  swap(r_seq_3, Rslt_35);

  Term Rslt_35V = get(termLoc(Rslt_35));
  apps = take(portLoc(1, Rslt_35V));
  lams = take(portLoc(2, Rslt_35V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_35), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblFn695 = newRef(glblCFn694);
Term glblVal696;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str13 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr1 0 bit\n"};
Term glblStr704 = newTerm_(VAL, (Term)&str13);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str14 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr1 1 bit\n"};
Term glblStr705 = newTerm_(VAL, (Term)&str14);
Term glblVal696;
void glblCFn701(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 128, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 105
  Term seq_8 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term e_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 0, SUB, seq_8);
  Term i_6 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 0, SUB, seq_8);
  Term o_5 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at sidequest.toc: 106

  Term o_5_1;
  o_5 = dupeArg(o_5, &o_5_1, 0);
  swap(termLoc(i_6), ERA);
  Term seq_19 = makePair(LAM, 130, SUB, NUL);
  Location r_seq_19 = portLoc(2, seq_19);
  Term p_18 = newTerm(VAR, 0, portLoc(1, seq_19));
  // allocate args at sidequest.toc: 110
  Term seq_12 = makePair(LAM, 129, SUB, NUL);
  Location r_seq_12 = portLoc(2, seq_12);
  Term p_11 = newTerm(VAR, 0, portLoc(1, seq_12));
  // allocate args at sidequest.toc: 107
  // call pr* at sidequest.toc: 108
  Term Rslt_14Args = makePair(APP, 0, glblStr704, SUB);
  Term Rslt_14 = newTerm(VAR, 0, portLoc(2, Rslt_14Args));
#ifdef STRICT
  pushRedex(Rslt_14Args, glblFn115);
#else
  swap(termLoc(Rslt_14), makePair(LAZ, 0, Rslt_14Args, glblFn115));
#endif

  // call clr1 at sidequest.toc: 105
  Term Rslt_15Args = makePair(APP, 0, p_11, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
#ifdef STRICT
  pushRedex(Rslt_15Args, glblVal696);
#else
  swap(termLoc(Rslt_15), makePair(LAZ, 0, Rslt_15Args, glblVal696));
#endif

  // call o at sidequest.toc: 106
  Term Rslt_16Args = makePair(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, Rslt_16Args));
#ifdef STRICT
  pushRedex(Rslt_16Args, o_5);
#else
  swap(termLoc(Rslt_16), makePair(LAZ, 0, Rslt_16Args, o_5));
#endif

  dupLabels[129] = "clr1-0";
  // link args to body
  swap(r_seq_12, Rslt_16);

  Term Rslt_16V = get(termLoc(Rslt_16));
  apps = take(portLoc(1, Rslt_16V));
  lams = take(portLoc(2, Rslt_16V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_14, apps);
  lams = makePair(LAM, 129, ERA, lams);

  swap(termLoc(Rslt_16), makePair(LAZ, 0, apps, lams));
  // call pr* at sidequest.toc: 111
  Term Rslt_21Args = makePair(APP, 0, glblStr705, SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
#ifdef STRICT
  pushRedex(Rslt_21Args, glblFn115);
#else
  swap(termLoc(Rslt_21), makePair(LAZ, 0, Rslt_21Args, glblFn115));
#endif

  // call clr1 at sidequest.toc: 105
  Term Rslt_22Args = makePair(APP, 0, p_18, SUB);
  Term Rslt_22 = newTerm(VAR, 0, portLoc(2, Rslt_22Args));
#ifdef STRICT
  pushRedex(Rslt_22Args, glblVal696);
#else
  swap(termLoc(Rslt_22), makePair(LAZ, 0, Rslt_22Args, glblVal696));
#endif

  // call o at sidequest.toc: 106
  Term Rslt_23Args = makePair(APP, 0, Rslt_22, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
#ifdef STRICT
  pushRedex(Rslt_23Args, o_5_1);
#else
  swap(termLoc(Rslt_23), makePair(LAZ, 0, Rslt_23Args, o_5_1));
#endif

  dupLabels[130] = "clr1-1";
  // link args to body
  swap(r_seq_19, Rslt_23);

  Term Rslt_23V = get(termLoc(Rslt_23));
  apps = take(portLoc(1, Rslt_23V));
  lams = take(portLoc(2, Rslt_23V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_21, apps);
  lams = makePair(LAM, 130, ERA, lams);

  swap(termLoc(Rslt_23), makePair(LAZ, 0, apps, lams));
  // call x at sidequest.toc: 105
  Term Rslt_24Args = makePair(APP, 0, e_7, SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
  Rslt_24Args = makePair(APP, 0, seq_19, Rslt_24Args);
  Rslt_24Args = makePair(APP, 0, seq_12, Rslt_24Args);
#ifdef STRICT
  pushRedex(Rslt_24Args, x_1);
#else
  swap(termLoc(Rslt_24), makePair(LAZ, 0, Rslt_24Args, x_1));
#endif

  dupLabels[0] = "clr1-f";
  // link args to body
  swap(r_seq_8, Rslt_24);

  Term Rslt_24V = get(termLoc(Rslt_24));
  apps = take(portLoc(1, Rslt_24V));
  lams = take(portLoc(2, Rslt_24V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_24), makePair(LAZ, 0, apps, lams));
  dupLabels[128] = "";
  // link args to body
  swap(r_seq_2, seq_8);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 128, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblVal696 = newRef(glblCFn701);
Term glblVal703;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str15 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr2 0 bit\n"};
Term glblStr710 = newTerm_(VAL, (Term)&str15);
Term glblclr2_0713;
void glblclr2_0712(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_12 = makePair(LAM, 132, SUB, NUL);
  Location r_seq_12 = portLoc(2, seq_12);
  Term p_11 = newTerm(VAR, 0, portLoc(1, seq_12));
  // allocate args at sidequest.toc: 119
  Term seq_16 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_16 = portLoc(2, seq_16);
  Term o_15 = newTerm(VAR, 0, portLoc(1, seq_16));
  // allocate args at sidequest.toc: 120
  // call pr* at sidequest.toc: 121
  Term Rslt_18Args = makePair(APP, 0, glblStr710, SUB);
  Term Rslt_18 = newTerm(VAR, 0, portLoc(2, Rslt_18Args));
#ifdef STRICT
  pushRedex(Rslt_18Args, glblFn115);
#else
  swap(termLoc(Rslt_18), makePair(LAZ, 0, Rslt_18Args, glblFn115));
#endif

  // call clr2 at sidequest.toc: 117
  Term Rslt_19Args = makePair(APP, 0, p_11, SUB);
  Term Rslt_19 = newTerm(VAR, 0, portLoc(2, Rslt_19Args));
#ifdef STRICT
  pushRedex(Rslt_19Args, glblVal703);
#else
  swap(termLoc(Rslt_19), makePair(LAZ, 0, Rslt_19Args, glblVal703));
#endif

  // call o at sidequest.toc: 120
  Term Rslt_20Args = makePair(APP, 0, Rslt_19, SUB);
  Term Rslt_20 = newTerm(VAR, 0, portLoc(2, Rslt_20Args));
#ifdef STRICT
  pushRedex(Rslt_20Args, o_15);
#else
  swap(termLoc(Rslt_20), makePair(LAZ, 0, Rslt_20Args, o_15));
#endif

  dupLabels[0] = "clr2-0-c";
  // link args to body
  swap(r_seq_16, Rslt_20);

  Term Rslt_20V = get(termLoc(Rslt_20));
  apps = take(portLoc(1, Rslt_20V));
  lams = take(portLoc(2, Rslt_20V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_18, apps);
  lams = makePair(LAM, 0, ERA, lams);

  swap(termLoc(Rslt_20), makePair(LAZ, 0, apps, lams));
  dupLabels[132] = "clr2-0";
  // link args to body
  swap(r_seq_12, seq_16);

  V = get(r_seq_12);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 132, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_12, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_12);
  return;
}
Term glblclr2_0713 = newRef(glblclr2_0712);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[12];
} str16 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr2 1 bit\n"};
Term glblStr711 = newTerm_(VAL, (Term)&str16);
Term glblclr2_1715;
void glblclr2_1714(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_23 = makePair(LAM, 133, SUB, NUL);
  Location r_seq_23 = portLoc(2, seq_23);
  Term p_22 = newTerm(VAR, 0, portLoc(1, seq_23));
  // allocate args at sidequest.toc: 123
  Term seq_27 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_27 = portLoc(2, seq_27);
  Term o_26 = newTerm(VAR, 0, portLoc(1, seq_27));
  // allocate args at sidequest.toc: 124
  // call pr* at sidequest.toc: 125
  Term Rslt_29Args = makePair(APP, 0, glblStr711, SUB);
  Term Rslt_29 = newTerm(VAR, 0, portLoc(2, Rslt_29Args));
#ifdef STRICT
  pushRedex(Rslt_29Args, glblFn115);
#else
  swap(termLoc(Rslt_29), makePair(LAZ, 0, Rslt_29Args, glblFn115));
#endif

  // call clr2 at sidequest.toc: 117
  Term Rslt_30Args = makePair(APP, 0, p_22, SUB);
  Term Rslt_30 = newTerm(VAR, 0, portLoc(2, Rslt_30Args));
#ifdef STRICT
  pushRedex(Rslt_30Args, glblVal703);
#else
  swap(termLoc(Rslt_30), makePair(LAZ, 0, Rslt_30Args, glblVal703));
#endif

  // call o at sidequest.toc: 124
  Term Rslt_31Args = makePair(APP, 0, Rslt_30, SUB);
  Term Rslt_31 = newTerm(VAR, 0, portLoc(2, Rslt_31Args));
#ifdef STRICT
  pushRedex(Rslt_31Args, o_26);
#else
  swap(termLoc(Rslt_31), makePair(LAZ, 0, Rslt_31Args, o_26));
#endif

  dupLabels[0] = "clr2-1-c";
  // link args to body
  swap(r_seq_27, Rslt_31);

  Term Rslt_31V = get(termLoc(Rslt_31));
  apps = take(portLoc(1, Rslt_31V));
  lams = take(portLoc(2, Rslt_31V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_29, apps);
  lams = makePair(LAM, 0, ERA, lams);

  swap(termLoc(Rslt_31), makePair(LAZ, 0, apps, lams));
  dupLabels[133] = "clr2-1";
  // link args to body
  swap(r_seq_23, seq_27);

  V = get(r_seq_23);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 133, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_23, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_23);
  return;
}
Term glblclr2_1715 = newRef(glblclr2_1714);
Term glblVal703;
void glblCFn707(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 131, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 117
  Term seq_8 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term e_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 0, SUB, seq_8);
  Term i_6 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 0, SUB, seq_8);
  Term o_5 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at sidequest.toc: 118
  swap(termLoc(i_6), ERA);
  Term seq_34 = makePair(LAM, 134, SUB, NUL);
  Location r_seq_34 = portLoc(2, seq_34);
  Term o_33 = newTerm(VAR, 0, portLoc(1, seq_34));
  // allocate args at sidequest.toc: 127
  swap(termLoc(o_33), ERA);
  dupLabels[134] = "clr-2-done";
  // link args to body
  swap(r_seq_34, e_7);

  V = get(r_seq_34);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 134, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_34, newTerm(VAR, 0, vLoc));
  // call x at sidequest.toc: 117
  Term Rslt_36Args = makePair(APP, 0, seq_34, SUB);
  Term Rslt_36 = newTerm(VAR, 0, portLoc(2, Rslt_36Args));
  Rslt_36Args = makePair(APP, 0, glblclr2_1715, Rslt_36Args);
  Rslt_36Args = makePair(APP, 0, glblclr2_0713, Rslt_36Args);
#ifdef STRICT
  pushRedex(Rslt_36Args, x_1);
#else
  swap(termLoc(Rslt_36), makePair(LAZ, 0, Rslt_36Args, x_1));
#endif

  // call  at sidequest.toc: 118
  Term Rslt_38Args = makePair(APP, 0, o_5, SUB);
  Term Rslt_38 = newTerm(VAR, 0, portLoc(2, Rslt_38Args));
#ifdef STRICT
  pushRedex(Rslt_38Args, Rslt_36);
#else
  swap(termLoc(Rslt_38), makePair(LAZ, 0, Rslt_38Args, Rslt_36));
#endif

  dupLabels[0] = "clr2-f";
  // link args to body
  swap(r_seq_8, Rslt_38);

  Term Rslt_38V = get(termLoc(Rslt_38));
  apps = take(portLoc(1, Rslt_38V));
  lams = take(portLoc(2, Rslt_38V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_38), makePair(LAZ, 0, apps, lams));
  dupLabels[131] = "";
  // link args to body
  swap(r_seq_2, seq_8);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 131, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblVal703 = newRef(glblCFn707);
Term glblVal709;
Term glblrep_O724;
void glblrep_O723(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_8 = makePair(LAM, 139, SUB, NUL);
  Location r_seq_8 = portLoc(2, seq_8);
  Term x_7 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 139, SUB, seq_8);
  Term f_6 = newTerm(VAR, 0, portLoc(1, seq_8));
  seq_8 = makePair(LAM, 139, SUB, seq_8);
  Term p_5 = newTerm(VAR, 0, portLoc(1, seq_8));
  // allocate args at sidequest.toc: 155

  Term f_6_1;
  f_6 = dupeArg(f_6, &f_6_1, 139);
  Term seq_13 = makePair(LAM, 136, SUB, NUL);
  Location r_seq_13 = portLoc(2, seq_13);
  Term k_12 = newTerm(VAR, 0, portLoc(1, seq_13));
  // allocate args at sidequest.toc: 158
  // call rep at sidequest.toc: 154
  Term Rslt_10Args = makePair(APP, 0, p_5, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
#ifdef STRICT
  pushRedex(Rslt_10Args, glblVal709);
#else
  swap(termLoc(Rslt_10), makePair(LAZ, 0, Rslt_10Args, glblVal709));
#endif

  // call f at sidequest.toc: 155
  Term Rslt_15Args = makePair(APP, 0, k_12, SUB);
  Term Rslt_15 = newTerm(VAR, 0, portLoc(2, Rslt_15Args));
#ifdef STRICT
  pushRedex(Rslt_15Args, f_6_1);
#else
  swap(termLoc(Rslt_15), makePair(LAZ, 0, Rslt_15Args, f_6_1));
#endif

  // call f at sidequest.toc: 155
  Term Rslt_16Args = makePair(APP, 0, Rslt_15, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, Rslt_16Args));
#ifdef STRICT
  pushRedex(Rslt_16Args, f_6);
#else
  swap(termLoc(Rslt_16), makePair(LAZ, 0, Rslt_16Args, f_6));
#endif

  dupLabels[136] = "rep-0-c";
  // link args to body
  swap(r_seq_13, Rslt_16);

  Term Rslt_16V = get(termLoc(Rslt_16));
  apps = take(portLoc(1, Rslt_16V));
  lams = take(portLoc(2, Rslt_16V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_16), makePair(LAZ, 0, apps, lams));
  // call  at sidequest.toc: 158
  Term Rslt_17Args = makePair(APP, 0, x_7, SUB);
  Term Rslt_17 = newTerm(VAR, 0, portLoc(2, Rslt_17Args));
  Rslt_17Args = makePair(APP, 0, seq_13, Rslt_17Args);
#ifdef STRICT
  pushRedex(Rslt_17Args, Rslt_10);
#else
  swap(termLoc(Rslt_17), makePair(LAZ, 0, Rslt_17Args, Rslt_10));
#endif

  dupLabels[139] = "rep-O";
  // link args to body
  swap(r_seq_8, Rslt_17);

  Term Rslt_17V = get(termLoc(Rslt_17));
  apps = take(portLoc(1, Rslt_17V));
  lams = take(portLoc(2, Rslt_17V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_17), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_8);
  return;
}
Term glblrep_O724 = newRef(glblrep_O723);
Term glblrep_I726;
void glblrep_I725(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_23 = makePair(LAM, 140, SUB, NUL);
  Location r_seq_23 = portLoc(2, seq_23);
  Term x_22 = newTerm(VAR, 0, portLoc(1, seq_23));
  seq_23 = makePair(LAM, 140, SUB, seq_23);
  Term f_21 = newTerm(VAR, 0, portLoc(1, seq_23));
  seq_23 = makePair(LAM, 140, SUB, seq_23);
  Term p_20 = newTerm(VAR, 0, portLoc(1, seq_23));
  // allocate args at sidequest.toc: 162

  Term f_21_2;
  f_21 = dupeArg(f_21, &f_21_2, 140);

  Term f_21_1;
  f_21 = dupeArg(f_21, &f_21_1, 140);
  Term seq_28 = makePair(LAM, 138, SUB, NUL);
  Location r_seq_28 = portLoc(2, seq_28);
  Term k_27 = newTerm(VAR, 0, portLoc(1, seq_28));
  // allocate args at sidequest.toc: 165
  // call rep at sidequest.toc: 154
  Term Rslt_25Args = makePair(APP, 0, p_20, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
#ifdef STRICT
  pushRedex(Rslt_25Args, glblVal709);
#else
  swap(termLoc(Rslt_25), makePair(LAZ, 0, Rslt_25Args, glblVal709));
#endif

  // call f at sidequest.toc: 162
  Term Rslt_30Args = makePair(APP, 0, k_27, SUB);
  Term Rslt_30 = newTerm(VAR, 0, portLoc(2, Rslt_30Args));
#ifdef STRICT
  pushRedex(Rslt_30Args, f_21_1);
#else
  swap(termLoc(Rslt_30), makePair(LAZ, 0, Rslt_30Args, f_21_1));
#endif

  // call f at sidequest.toc: 162
  Term Rslt_31Args = makePair(APP, 0, Rslt_30, SUB);
  Term Rslt_31 = newTerm(VAR, 0, portLoc(2, Rslt_31Args));
#ifdef STRICT
  pushRedex(Rslt_31Args, f_21);
#else
  swap(termLoc(Rslt_31), makePair(LAZ, 0, Rslt_31Args, f_21));
#endif

  dupLabels[138] = "rep-1-c";
  // link args to body
  swap(r_seq_28, Rslt_31);

  Term Rslt_31V = get(termLoc(Rslt_31));
  apps = take(portLoc(1, Rslt_31V));
  lams = take(portLoc(2, Rslt_31V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_31), makePair(LAZ, 0, apps, lams));
  // call f at sidequest.toc: 162
  Term Rslt_32Args = makePair(APP, 0, x_22, SUB);
  Term Rslt_32 = newTerm(VAR, 0, portLoc(2, Rslt_32Args));
#ifdef STRICT
  pushRedex(Rslt_32Args, f_21_2);
#else
  swap(termLoc(Rslt_32), makePair(LAZ, 0, Rslt_32Args, f_21_2));
#endif

  // call  at sidequest.toc: 165
  Term Rslt_33Args = makePair(APP, 0, Rslt_32, SUB);
  Term Rslt_33 = newTerm(VAR, 0, portLoc(2, Rslt_33Args));
  Rslt_33Args = makePair(APP, 0, seq_28, Rslt_33Args);
#ifdef STRICT
  pushRedex(Rslt_33Args, Rslt_25);
#else
  swap(termLoc(Rslt_33), makePair(LAZ, 0, Rslt_33Args, Rslt_25));
#endif

  dupLabels[140] = "rep-I";
  // link args to body
  swap(r_seq_23, Rslt_33);

  Term Rslt_33V = get(termLoc(Rslt_33));
  apps = take(portLoc(1, Rslt_33V));
  lams = take(portLoc(2, Rslt_33V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_33), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_23);
  return;
}
Term glblrep_I726 = newRef(glblrep_I725);
Term glblrep_E728;
void glblrep_E727(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_38 = makePair(LAM, 141, SUB, NUL);
  Location r_seq_38 = portLoc(2, seq_38);
  Term x_37 = newTerm(VAR, 0, portLoc(1, seq_38));
  seq_38 = makePair(LAM, 141, SUB, seq_38);
  Term f_36 = newTerm(VAR, 0, portLoc(1, seq_38));
  // allocate args at sidequest.toc: 169
  swap(termLoc(f_36), ERA);
  dupLabels[141] = "rep-E";
  // link args to body
  swap(r_seq_38, x_37);

  V = get(r_seq_38);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 141, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_38, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_38);
  return;
}
Term glblrep_E728 = newRef(glblrep_E727);
Term glblVal709;
void glblCFn719(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 135, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term xs_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 154
  // call xs at sidequest.toc: 154
  Term Rslt_40Args = makePair(APP, 0, glblrep_E728, SUB);
  Term Rslt_40 = newTerm(VAR, 0, portLoc(2, Rslt_40Args));
  Rslt_40Args = makePair(APP, 0, glblrep_I726, Rslt_40Args);
  Rslt_40Args = makePair(APP, 0, glblrep_O724, Rslt_40Args);
#ifdef STRICT
  pushRedex(Rslt_40Args, xs_1);
#else
  swap(termLoc(Rslt_40), makePair(LAZ, 0, Rslt_40Args, xs_1));
#endif

  dupLabels[135] = "";
  // link args to body
  swap(r_seq_2, Rslt_40);

  Term Rslt_40V = get(termLoc(Rslt_40));
  apps = take(portLoc(1, Rslt_40V));
  lams = take(portLoc(2, Rslt_40V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_40), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblVal709 = newRef(glblCFn719);
Term glblFn722;
void glblCFn721(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 137, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term b_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 172
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term t_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at sidequest.toc: 173
  swap(termLoc(t_5), ERA);
  Term seq_10 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_10 = portLoc(2, seq_10);
  Term f_9 = newTerm(VAR, 0, portLoc(1, seq_10));
  // allocate args at sidequest.toc: 174

  Term f_9_1;
  f_9 = dupeArg(f_9, &f_9_1, 0);
  // call b at sidequest.toc: 172
  Term Rslt_12Args = makePair(APP, 0, f_9_1, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
  Rslt_12Args = makePair(APP, 0, f_9, Rslt_12Args);
#ifdef STRICT
  pushRedex(Rslt_12Args, b_1);
#else
  swap(termLoc(Rslt_12), makePair(LAZ, 0, Rslt_12Args, b_1));
#endif

  dupLabels[0] = "";
  // link args to body
  swap(r_seq_10, Rslt_12);

  Term Rslt_12V = get(termLoc(Rslt_12));
  apps = take(portLoc(1, Rslt_12V));
  lams = take(portLoc(2, Rslt_12V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_12), makePair(LAZ, 0, apps, lams));
  dupLabels[0] = "";
  // link args to body
  swap(r_seq_6, seq_10);

  V = get(r_seq_6);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_6, newTerm(VAR, 0, vLoc));
  dupLabels[137] = "";
  // link args to body
  swap(r_seq_2, seq_6);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 137, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblFn722 = newRef(glblCFn721);
Term glblFn734;
void glblCFn733(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 142, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term b_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 177
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term t_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at sidequest.toc: 178
  swap(termLoc(t_5), ERA);
  Term seq_10 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_10 = portLoc(2, seq_10);
  Term f_9 = newTerm(VAR, 0, portLoc(1, seq_10));
  // allocate args at sidequest.toc: 179

  Term f_9_2;
  f_9 = dupeArg(f_9, &f_9_2, 0);

  Term f_9_1;
  f_9 = dupeArg(f_9, &f_9_1, 0);
  // call f at sidequest.toc: 179
  Term Rslt_12Args = makePair(APP, 0, f_9_1, SUB);
  Term Rslt_12 = newTerm(VAR, 0, portLoc(2, Rslt_12Args));
#ifdef STRICT
  pushRedex(Rslt_12Args, f_9);
#else
  swap(termLoc(Rslt_12), makePair(LAZ, 0, Rslt_12Args, f_9));
#endif

  // call b at sidequest.toc: 177
  Term Rslt_13Args = makePair(APP, 0, f_9_2, SUB);
  Term Rslt_13 = newTerm(VAR, 0, portLoc(2, Rslt_13Args));
  Rslt_13Args = makePair(APP, 0, Rslt_12, Rslt_13Args);
#ifdef STRICT
  pushRedex(Rslt_13Args, b_1);
#else
  swap(termLoc(Rslt_13), makePair(LAZ, 0, Rslt_13Args, b_1));
#endif

  dupLabels[0] = "";
  // link args to body
  swap(r_seq_10, Rslt_13);

  Term Rslt_13V = get(termLoc(Rslt_13));
  apps = take(portLoc(1, Rslt_13V));
  lams = take(portLoc(2, Rslt_13V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_13), makePair(LAZ, 0, apps, lams));
  dupLabels[0] = "";
  // link args to body
  swap(r_seq_6, seq_10);

  V = get(r_seq_6);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 0, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_6, newTerm(VAR, 0, vLoc));
  dupLabels[142] = "";
  // link args to body
  swap(r_seq_2, seq_6);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 142, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblFn734 = newRef(glblCFn733);
Term glblVal735;
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[7];
} str17 = {StringBufferType, REFS_STATIC, 0, 0, 6, "bad-f\n"};
Term glblStr740 = newTerm_(VAL, (Term)&str17);
Term glblVal735;
void glblCFn738(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_3 = makePair(LAM, 143, SUB, NUL);
  Location r_seq_3 = portLoc(2, seq_3);
  Term o_2 = newTerm(VAR, 0, portLoc(1, seq_3));
  seq_3 = makePair(LAM, 143, SUB, seq_3);
  Term x_1 = newTerm(VAR, 0, portLoc(1, seq_3));
  // allocate args at sidequest.toc: 183
  Term seq_7 = makePair(LAM, 144, SUB, NUL);
  Location r_seq_7 = portLoc(2, seq_7);
  Term p_6 = newTerm(VAR, 0, portLoc(1, seq_7));
  // allocate args at sidequest.toc: 184
  swap(termLoc(p_6), ERA);
  // call pr* at sidequest.toc: 185
  Term Rslt_9Args = makePair(APP, 0, glblStr740, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
#ifdef STRICT
  pushRedex(Rslt_9Args, glblFn115);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, glblFn115));
#endif

  // call o at sidequest.toc: 183
  Term Rslt_10Args = makePair(APP, 0, NUL, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
#ifdef STRICT
  pushRedex(Rslt_10Args, o_2);
#else
  swap(termLoc(Rslt_10), makePair(LAZ, 0, Rslt_10Args, o_2));
#endif

  dupLabels[144] = "bad-f";
  // link args to body
  swap(r_seq_7, Rslt_10);

  Term Rslt_10V = get(termLoc(Rslt_10));
  apps = take(portLoc(1, Rslt_10V));
  lams = take(portLoc(2, Rslt_10V));

  vLoc = portLoc(2, apps);
  apps = makePair(APP, 0, Rslt_9, apps);
  lams = makePair(LAM, 144, ERA, lams);

  swap(termLoc(Rslt_10), makePair(LAZ, 0, apps, lams));
  // call x at sidequest.toc: 183
  Term Rslt_11Args = makePair(APP, 0, seq_7, SUB);
  Term Rslt_11 = newTerm(VAR, 0, portLoc(2, Rslt_11Args));
#ifdef STRICT
  pushRedex(Rslt_11Args, x_1);
#else
  swap(termLoc(Rslt_11), makePair(LAZ, 0, Rslt_11Args, x_1));
#endif

  dupLabels[143] = "";
  // link args to body
  swap(r_seq_3, Rslt_11);

  Term Rslt_11V = get(termLoc(Rslt_11));
  apps = take(portLoc(1, Rslt_11V));
  lams = take(portLoc(2, Rslt_11V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_11), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_3);
  return;
}
Term glblVal735 = newRef(glblCFn738);
Term glblFn743;
void glblCFn742(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 145, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term b_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 188
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term f_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at sidequest.toc: 189
  // call b at sidequest.toc: 188
  Term Rslt_8Args = makePair(APP, 0, f_5, SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, Rslt_8Args));
#ifdef STRICT
  pushRedex(Rslt_8Args, b_1);
#else
  swap(termLoc(Rslt_8), makePair(LAZ, 0, Rslt_8Args, b_1));
#endif

  dupLabels[0] = "";
  // link args to body
  swap(r_seq_6, Rslt_8);

  Term Rslt_8V = get(termLoc(Rslt_8));
  apps = take(portLoc(1, Rslt_8V));
  lams = take(portLoc(2, Rslt_8V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_8), makePair(LAZ, 0, apps, lams));
  dupLabels[145] = "";
  // link args to body
  swap(r_seq_2, seq_6);

  V = get(r_seq_2);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 145, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_2, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_2);
  return;
}
Term glblFn743 = newRef(glblCFn742);
Term glblinput748;
void glblinput747(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_14 = makePair(LAM, 147, SUB, NUL);
  Location r_seq_14 = portLoc(2, seq_14);
  Term f_13 = newTerm(VAR, 0, portLoc(1, seq_14));
  seq_14 = makePair(LAM, 147, SUB, seq_14);
  Term t_12 = newTerm(VAR, 0, portLoc(1, seq_14));
  // allocate args at sidequest.toc: 197
  swap(termLoc(f_13), ERA);
  dupLabels[147] = "input";
  // link args to body
  swap(r_seq_14, t_12);

  V = get(r_seq_14);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 147, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_14, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_14);
  return;
}
Term glblinput748 = newRef(glblinput747);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[8];
} str18 = {StringBufferType, REFS_STATIC, 0, 0, 7, "boogity"};
Term glblStr749 = newTerm_(VAL, (Term)&str18);
Term glblfalse751;
void glblfalse750(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_19 = makePair(LAM, 148, SUB, NUL);
  Location r_seq_19 = portLoc(2, seq_19);
  Term f_18 = newTerm(VAR, 0, portLoc(1, seq_19));
  // allocate args at sidequest.toc: 200
  dupLabels[148] = "false";
  // link args to body
  swap(r_seq_19, f_18);

  V = get(r_seq_19);
  apps = makePair(APP, 0, V, SUB);
  lams = makePair(LAM, 148, SUB, NUL);
  swap(portLoc(2, lams), newTerm(VAR, 0, portLoc(1, lams)));
  vLoc = portLoc(2, apps);
  swap(vLoc, makePair(LAZ, 0, apps, lams));
  swap(r_seq_19, newTerm(VAR, 0, vLoc));
  pushRedex(args, seq_19);
  return;
}
Term glblfalse751 = newRef(glblfalse750);
struct {
  TYPE_SIZE type;
  REFS_SIZE refs;
  int64_t *hash;
  Value *parent;
  int64_t len;
  char buffer[14];
} str19 = {StringBufferType, REFS_STATIC, 0, 0, 13, "sidequest.toc"};
Term glblStr755 = newTerm_(VAL, (Term)&str19);
void glblProto95_754(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(193), args);
    args = makePair(APP, 0, glblStr755, args);
    interact(args, glblProto95);
  }
}
void glblProto39_756(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(193), args);
    args = makePair(APP, 0, glblStr755, args);
    interact(args, glblProto39);
  }
}
void glblProto29_758(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
    swap(portLoc(1, args), arityArgs.args[0]);
    args = makePair(APP, 0, newI60(193), args);
    args = makePair(APP, 0, glblStr755, args);
    interact(args, glblProto29);
  }
}
Term glblFn746;
void glblCFn745(Term ref, Term args) {
  Term apps, lams, V;
  Location vLoc;
  Term seq_2 = makePair(LAM, 146, SUB, NUL);
  Location r_seq_2 = portLoc(2, seq_2);
  Term args_1 = newTerm(VAR, 0, portLoc(1, seq_2));
  // allocate args at sidequest.toc: 192
  Term seq_6 = makePair(LAM, 0, SUB, NUL);
  Location r_seq_6 = portLoc(2, seq_6);
  Term reps_5 = newTerm(VAR, 0, portLoc(1, seq_6));
  // allocate args at sidequest.toc: 193
  // call hbin at sidequest.toc: 195
  Term Rslt_8Args = makePair(APP, 0, newI60(8), SUB);
  Term Rslt_8 = newTerm(VAR, 0, portLoc(2, Rslt_8Args));
#ifdef STRICT
  pushRedex(Rslt_8Args, glblFn689);
#else
  swap(termLoc(Rslt_8), makePair(LAZ, 0, Rslt_8Args, glblFn689));
#endif

  // call  at sidequest.toc: 193
  Term Rslt_9Args = makePair(APP, 0, reps_5, SUB);
  Term Rslt_9 = newTerm(VAR, 0, portLoc(2, Rslt_9Args));
#ifdef STRICT
  pushRedex(Rslt_9Args, Rslt_8);
#else
  swap(termLoc(Rslt_9), makePair(LAZ, 0, Rslt_9Args, Rslt_8));
#endif

  // call rep at sidequest.toc: 195
  Term Rslt_10Args = makePair(APP, 0, Rslt_9, SUB);
  Term Rslt_10 = newTerm(VAR, 0, portLoc(2, Rslt_10Args));
#ifdef STRICT
  pushRedex(Rslt_10Args, glblVal709);
#else
  swap(termLoc(Rslt_10), makePair(LAZ, 0, Rslt_10Args, glblVal709));
#endif

  // call  at sidequest.toc: 196
  Term Rslt_16Args = makePair(APP, 0, glblinput748, SUB);
  Term Rslt_16 = newTerm(VAR, 0, portLoc(2, Rslt_16Args));
  Rslt_16Args = makePair(APP, 0, glblFn722, Rslt_16Args);
#ifdef STRICT
  pushRedex(Rslt_16Args, Rslt_10);
#else
  swap(termLoc(Rslt_16), makePair(LAZ, 0, Rslt_16Args, Rslt_10));
#endif

  // call  at sidequest.toc: 199
  Term Rslt_21Args = makePair(APP, 0, newI60(95), SUB);
  Term Rslt_21 = newTerm(VAR, 0, portLoc(2, Rslt_21Args));
  Rslt_21Args = makePair(APP, 0, glblfalse751, Rslt_21Args);
  Rslt_21Args = makePair(APP, 0, glblStr749, Rslt_21Args);
#ifdef STRICT
  pushRedex(Rslt_21Args, Rslt_16);
#else
  swap(termLoc(Rslt_21), makePair(LAZ, 0, Rslt_21Args, Rslt_16));
#endif

  dupLabels[0] = "";
  // link args to body
  swap(r_seq_6, Rslt_21);

  Term Rslt_21V = get(termLoc(Rslt_21));
  apps = take(portLoc(1, Rslt_21V));
  lams = take(portLoc(2, Rslt_21V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_21), makePair(LAZ, 0, apps, lams));
  // call nth at sidequest.toc: 193
  Term Rslt_22Args = makePair(APP, 0, newI60(1), SUB);
  Term Rslt_22 = newTerm(VAR, 0, portLoc(2, Rslt_22Args));
  Rslt_22Args = makePair(APP, 0, args_1, Rslt_22Args);
#ifdef STRICT
  pushRedex(Rslt_22Args, newRef(glblProto95_754));
#else
  swap(termLoc(Rslt_22),
       makePair(LAZ, 0, Rslt_22Args, newRef(glblProto95_754)));
#endif
  // call map at sidequest.toc: 193
  Term Rslt_23Args = makePair(APP, 0, glblFn660, SUB);
  Term Rslt_23 = newTerm(VAR, 0, portLoc(2, Rslt_23Args));
  Rslt_23Args = makePair(APP, 0, Rslt_22, Rslt_23Args);
#ifdef STRICT
  pushRedex(Rslt_23Args, newRef(glblProto39_756));
#else
  swap(termLoc(Rslt_23),
       makePair(LAZ, 0, Rslt_23Args, newRef(glblProto39_756)));
#endif
  // call either at sidequest.toc: 193
  Term Rslt_24Args = makePair(APP, 0, newI60(4), SUB);
  Term Rslt_24 = newTerm(VAR, 0, portLoc(2, Rslt_24Args));
  Rslt_24Args = makePair(APP, 0, Rslt_23, Rslt_24Args);
#ifdef STRICT
  pushRedex(Rslt_24Args, newRef(glblProto29_758));
#else
  swap(termLoc(Rslt_24),
       makePair(LAZ, 0, Rslt_24Args, newRef(glblProto29_758)));
#endif
  // call  at sidequest.toc: 193
  Term Rslt_25Args = makePair(APP, 0, Rslt_24, SUB);
  Term Rslt_25 = newTerm(VAR, 0, portLoc(2, Rslt_25Args));
#ifdef STRICT
  pushRedex(Rslt_25Args, seq_6);
#else
  swap(termLoc(Rslt_25), makePair(LAZ, 0, Rslt_25Args, seq_6));
#endif

  dupLabels[146] = "";
  // link args to body
  swap(r_seq_2, Rslt_25);

  Term Rslt_25V = get(termLoc(Rslt_25));
  apps = take(portLoc(1, Rslt_25V));
  lams = take(portLoc(2, Rslt_25V));

  vLoc = portLoc(2, apps);
  swap(termLoc(Rslt_25), makePair(LAZ, 0, apps, lams));
  pushRedex(args, seq_2);
  return;
}
Term glblFn746 = newRef(glblCFn745);
Term glblProto51;
void glblCFn52(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'wrap' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblwrap505);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'wrap' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto51 = newRef(glblCFn52);
Term glblProto69;
void glblCFn70(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'reduce' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblreduce511);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'reduce' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto69 = newRef(glblCFn70);
Term glblProto105;
void glblCFn106(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vals' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'vals' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto105 = newRef(glblCFn106);
Term glblProto63;
void glblCFn64(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'empty' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblempty525);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'empty' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto63 = newRef(glblCFn64);
Term glblProto81;
void glblCFn82(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'split' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'split' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto81 = newRef(glblCFn82);
Term glblProto89;
void glblCFn90(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'drop-while' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'drop-while' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto89 = newRef(glblCFn90);
Term glblProto37;
void glblCFn38(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'recurse' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glblrecurse364);
        break;
      case 5:
        pushRedex(args, glblrecurse362);
        break;
      case 46:
        pushRedex(args, glblrecurse549);
        break;
      case 43:
        pushRedex(args, glblrecurse197);
        break;
      case 45:
        pushRedex(args, glblrecurse586);
        break;
      case 42:
        pushRedex(args, glblrecurse147);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'recurse' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto37 = newRef(glblCFn38);
Term glblProto313;
void glblFldFn314(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.s' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 5:
        pushRedex(args, glblFld329);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.s' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto313 = newRef(glblFldFn314);
Term glblProto575;
void glblFldFn577(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.head' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 45:
        pushRedex(args, glblFld595);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.head' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto575 = newRef(glblFldFn577);
Term glblProto29;
void glblCFn30(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'either' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glbleither195);
        break;
      case 42:
        pushRedex(args, glbleither145);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'either' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto29 = newRef(glblCFn30);
Term glblProto93;
void glblCFn94(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'reverse' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblreverse509);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'reverse' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto93 = newRef(glblCFn94);
Term glblProto15;
void glblCFn17(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      pushRedex(args, glbl_EQ_262);
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glbl_EQ_347);
        break;
      case 5:
        pushRedex(args, glbl_EQ_324);
        break;
      case 43:
        pushRedex(args, glbl_EQ_201);
        break;
      case 8:
        pushRedex(args, glbl_EQ_539);
        break;
      case 42:
        pushRedex(args, glbl_EQ_149);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '=' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto15 = newRef(glblCFn17);
Term glblProto95;
void glblCFn96(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'nth' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblnth529);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'nth' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto95 = newRef(glblCFn96);
Term glblProto3;
void glblCFn4(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      pushRedex(args, glbltype_name266);
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glbltype_name372);
        break;
      case 5:
        pushRedex(args, glbltype_name370);
        break;
      case 8:
        pushRedex(args, glbltype_name537);
        break;
      default: {
        pushRedex(args, glbltype_name6);
      }
      }
    }
  }
  return;
}
Term glblProto3 = newRef(glblCFn4);
Term glblProto25;
void glblCFn26(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'and' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glbland191);
        break;
      case 42:
        pushRedex(args, glbland141);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'and' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto25 = newRef(glblCFn26);
Term glblProto307;
void glblCFn308(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'subs' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glblsubs345);
        break;
      case 5:
        pushRedex(args, glblsubs326);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'subs' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto307 = newRef(glblCFn308);
Term glblProto579;
void glblFldFn581(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.tail' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 45:
        pushRedex(args, glblFld599);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.tail' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto579 = newRef(glblFldFn581);
Term glblProto91;
void glblCFn92(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'take-while' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'take-while' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto91 = newRef(glblCFn92);
Term glblProto403;
void glblCFn404(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vect-reverse' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblvect_reverse416);
        break;
      case 42:
        pushRedex(args, glblvect_reverse413);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'vect-reverse' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto403 = newRef(glblCFn404);
Term glblProto45;
void glblCFn46(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'flatten' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblflatten501);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'flatten' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto45 = newRef(glblCFn46);
Term glblProto23;
void glblCFn24(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      pushRedex(args, glbl_LT_260);
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of '<' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto23 = newRef(glblCFn24);
Term glblProto87;
void glblCFn88(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'drop' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'drop' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto87 = newRef(glblCFn88);
Term glblProto39;
void glblCFn40(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'map' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblmap187);
        break;
      case 8:
        pushRedex(args, glblmap507);
        break;
      case 42:
        pushRedex(args, glblmap137);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'map' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto39 = newRef(glblCFn40);
Term glblProto41;
void glblCFn42(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'map!' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'map!' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto41 = newRef(glblCFn42);
Term glblProto65;
void glblCFn66(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'conj' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblconj515);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'conj' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto65 = newRef(glblCFn66);
Term glblProto73;
void glblCFn74(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'first' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblfirst521);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'first' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto73 = newRef(glblCFn74);
Term glblProto79;
void glblCFn80(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'butlast' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'butlast' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto79 = newRef(glblCFn80);
Term glblProto396;
void glblCFn397(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vect-reduce' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblvect_reduce402);
        break;
      case 42:
        pushRedex(args, glblvect_reduce399);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'vect-reduce' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto396 = newRef(glblCFn397);
Term glblProto33;
void glblCFn34(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'zero' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'zero' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto33 = newRef(glblCFn34);
Term glblProto43;
void glblCFn44(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'flat-map' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblflat_map185);
        break;
      case 8:
        pushRedex(args, glblflat_map503);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'flat-map' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto43 = newRef(glblCFn44);
Term glblProto71;
void glblCFn72(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vec' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'vec' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto71 = newRef(glblCFn72);
Term glblProto315;
void glblFldFn316(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.start' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 5:
        pushRedex(args, glblFld331);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.start' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto315 = newRef(glblFldFn316);
Term glblProto417;
void glblCFn418(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'vect-sub' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblvect_sub434);
        break;
      case 42:
        pushRedex(args, glblvect_sub431);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'vect-sub' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto417 = newRef(glblCFn418);
Term glblProto75;
void glblCFn76(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'rest' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblrest517);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'rest' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto75 = newRef(glblCFn76);
Term glblProto47;
void glblCFn48(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'extend' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'extend' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto47 = newRef(glblCFn48);
Term glblProto83;
void glblCFn84(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'split-with' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'split-with' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto83 = newRef(glblCFn84);
Term glblProto59;
void glblCFn60(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'empty?' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblempty_QM_527);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'empty?' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto59 = newRef(glblCFn60);
Term glblProto67;
void glblCFn68(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'filter' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblfilter499);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'filter' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto67 = newRef(glblCFn68);
Term glblProto97;
void glblCFn98(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'store' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblstore513);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'store' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto97 = newRef(glblCFn98);
Term glblProto85;
void glblCFn86(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'take' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'take' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto85 = newRef(glblCFn86);
Term glblProto61;
void glblCFn62(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'count' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glblcount349);
        break;
      case 5:
        pushRedex(args, glblcount322);
        break;
      case 46:
        pushRedex(args, glblcount551);
        break;
      case 8:
        pushRedex(args, glblcount533);
        break;
      case 45:
        pushRedex(args, glblcount590);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'count' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto61 = newRef(glblCFn62);
Term glblProto16;
void glblFldFn18(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.x' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblFld21);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.x' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto16 = newRef(glblFldFn18);
Term glblProto107;
void glblCFn108(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      pushRedex(args, glblstr_vect264);
    } else {
      switch (dispVal->type) {
      case 2:
        pushRedex(args, glblstr_vect368);
        break;
      case 5:
        pushRedex(args, glblstr_vect366);
        break;
      case 43:
        pushRedex(args, glblstr_vect203);
        break;
      case 8:
        pushRedex(args, glblstr_vect535);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'str-vect' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto107 = newRef(glblCFn108);
Term glblProto57;
void glblCFn58(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'interpose' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblinterpose497);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'interpose' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto57 = newRef(glblCFn58);
Term glblProto35;
void glblCFn36(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'comp' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblcomp523);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'comp' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto35 = newRef(glblCFn36);
Term glblProto103;
void glblCFn104(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'keys' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'keys' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto103 = newRef(glblCFn104);
Term glblProto55;
void glblCFn56(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'apply' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'apply' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto55 = newRef(glblCFn56);
Term glblProto27;
void glblCFn28(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'or' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblor189);
        break;
      case 42:
        pushRedex(args, glblor139);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'or' found for type %s (%ld) called from "
                "%.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto27 = newRef(glblCFn28);
Term glblProto31;
void glblCFn32(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      pushRedex(args, glblcond258);
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblcond193);
        break;
      case 42:
        pushRedex(args, glblcond143);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'cond' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto31 = newRef(glblCFn32);
Term glblProto77;
void glblCFn78(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'last' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glbllast519);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'last' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto77 = newRef(glblCFn78);
Term glblProto317;
void glblFldFn318(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of '.len' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 5:
        pushRedex(args, glblFld333);
        break;
      case 45:
        pushRedex(args, glblFld602);
        break;
      default: {
        fprintf(stderr,
                "No implementation of '.len' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto317 = newRef(glblFldFn318);
Term glblProto99;
void glblCFn100(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'assoc' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'assoc' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto99 = newRef(glblCFn100);
Term glblProto53;
void glblCFn54(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'extract' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 43:
        pushRedex(args, glblextract199);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'extract' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto53 = newRef(glblCFn54);
Term glblProto49;
void glblCFn50(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'duplicate' found for integers called from "
              "%.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'duplicate' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto49 = newRef(glblCFn50);
Term glblProto109;
void glblCFn110(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'free-resource' found for integers called "
              "from %.*s: %ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      default: {
        fprintf(stderr,
                "No implementation of 'free-resource' found for type %s (%ld) "
                "called from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto109 = newRef(glblCFn110);
Term glblProto101;
void glblCFn102(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swap(termLoc(args), (Term)dispVal);
    if (termTag((Term)dispVal) == I60) {
      fprintf(stderr,
              "No implementation of 'get' found for integers called from %.*s: "
              "%ld\n",
              (int)fileStr->len, fileStr->buffer, getI60(arityArgs.args[1]));
      BOOM("Dispatch failed");
    } else {
      switch (dispVal->type) {
      case 8:
        pushRedex(args, glblget531);
        break;
      default: {
        fprintf(stderr,
                "No implementation of 'get' found for type %s (%ld) called "
                "from %.*s: %ld\n",
                typeName(dispVal->type), dispVal->type, (int)fileStr->len,
                fileStr->buffer, getI60(arityArgs.args[1]));
        BOOM("Dispatch failed");
      }
      }
    }
  }
  return;
}
Term glblProto101 = newRef(glblCFn102);
char *dupLabels[154];
unsigned refsCount = 205;
unsigned refNameCount = 205;
refMap refNames[205] = {{intCond, "intCond"},
                        {vectMap, "vectMap"},
                        {vectConjFn, "vectConj"},
                        {graphFn, "graph"},
                        {accessFieldFn, "accessField"},
                        {hvmVectFn, "hvmVect"},
                        {constructFn, "construct"},
                        {glblCFn114, "pr*"},
                        {glblCFn52, "wrap"},
                        {glblwrap504, "wrap 8"},
                        {glblCFn204, "int-="},
                        {glblCFn70, "reduce"},
                        {glblreduce510, "reduce 8"},
                        {glblCFn106, "vals"},
                        {glblCFn64, "empty"},
                        {glblempty524, "empty 8"},
                        {glblCFn82, "split"},
                        {glblCFn90, "drop-while"},
                        {glblCFn454, "to-str"},
                        {glblCFn38, "recurse"},
                        {glblrecurse363, "recurse 2"},
                        {glblrecurse361, "recurse 5"},
                        {glblrecurse548, "recurse 46"},
                        {glblrecurse196, "recurse 43"},
                        {glblrecurse585, "recurse 45"},
                        {glblrecurse146, "recurse 42"},
                        {glblFldFn314, ".s"},
                        {glblFldFn328, ".s 5"},
                        {glblCFn129, "*"},
                        {glblFldFn577, ".head"},
                        {glblFldFn593, ".head 45"},
                        {glblCFn30, "either"},
                        {glbleither194, "either 43"},
                        {glbleither144, "either 42"},
                        {glblCFn212, "<="},
                        {glblCFn94, "reverse"},
                        {glblreverse508, "reverse 8"},
                        {glblCFn17, "="},
                        {glbl_EQ_346, "= 2"},
                        {glbl_EQ_323, "= 5"},
                        {glbl_EQ_200, "= 43"},
                        {glbl_EQ_538, "= 8"},
                        {glbl_EQ_148, "= 42"},
                        {glbl_EQ_261, "= 1"},
                        {glblCFn270, "vect-count"},
                        {glblCons592, "Cons"},
                        {glblCFn96, "nth"},
                        {glblnth528, "nth 8"},
                        {glblCFn117, "+"},
                        {glblCFn268, "min"},
                        {glblCFn4, "type-name"},
                        {glbltype_name371, "type-name 2"},
                        {glbltype_name369, "type-name 5"},
                        {glbltype_name536, "type-name 8"},
                        {glbltype_name265, "type-name 1"},
                        {glblglbl5, "type-name 0"},
                        {glblCFn375, "vect-get"},
                        {glblCFn546, "str*"},
                        {glblCFn26, "and"},
                        {glbland190, "and 43"},
                        {glbland140, "and 42"},
                        {glblc165, "core: 313"},
                        {glblCFn544, "cons"},
                        {glblCFn308, "subs"},
                        {glblsubs344, "subs 2"},
                        {glblsubs325, "subs 5"},
                        {glblFldFn581, ".tail"},
                        {glblFldFn597, ".tail 45"},
                        {glblCFn384, "range*"},
                        {glblCFn92, "take-while"},
                        {glblCFn393, "range"},
                        {glblCFn404, "vect-reverse"},
                        {glblvect_reverse415, "vect-reverse 43"},
                        {glblvect_reverse412, "vect-reverse 42"},
                        {glblCFn7, "type-num"},
                        {glblCFn111, "abort"},
                        {glblCFn46, "flatten"},
                        {glblflatten500, "flatten 8"},
                        {glblCFn24, "<"},
                        {glbl_LT_259, "< 1"},
                        {glblCFn88, "drop"},
                        {glblCFn310, "str="},
                        {glblCFn40, "map"},
                        {glblmap186, "map 43"},
                        {glblmap506, "map 8"},
                        {glblmap136, "map 42"},
                        {glblCFn286, "fold"},
                        {glblCFn42, "map!"},
                        {glblCFn66, "conj"},
                        {glblconj514, "conj 8"},
                        {glblCFn74, "first"},
                        {glblfirst520, "first 8"},
                        {glblCFn80, "butlast"},
                        {glblCFn301, "str-malloc"},
                        {glblCFn397, "vect-reduce"},
                        {glblvect_reduce401, "vect-reduce 43"},
                        {glblvect_reduce398, "vect-reduce 42"},
                        {glblCFn305, "str-append"},
                        {glblCFn34, "zero"},
                        {glblCFn44, "flat-map"},
                        {glblflat_map184, "flat-map 43"},
                        {glblflat_map502, "flat-map 8"},
                        {glblCFn72, "vec"},
                        {glblFldFn316, ".start"},
                        {glblFldFn330, ".start 5"},
                        {glblCFn418, "vect-sub"},
                        {glblvect_sub433, "vect-sub 43"},
                        {glblvect_sub430, "vect-sub 42"},
                        {glblCFn210, "number-str"},
                        {glblCFn291, "unfold"},
                        {glblCFn76, "rest"},
                        {glblrest516, "rest 8"},
                        {glblCFn48, "extend"},
                        {glblCFn159, "println*"},
                        {glblCFn474, "vect-="},
                        {glblCFn84, "split-with"},
                        {glblCFn60, "empty?"},
                        {glblempty_QM_526, "empty? 8"},
                        {glblCFn68, "filter"},
                        {glblfilter498, "filter 8"},
                        {glblCFn476, "vect-store"},
                        {glblCFn98, "store"},
                        {glblstore512, "store 8"},
                        {glblCFn86, "take"},
                        {glblCFn62, "count"},
                        {glblcount348, "count 2"},
                        {glblcount321, "count 5"},
                        {glblcount550, "count 46"},
                        {glblcount532, "count 8"},
                        {glblcount588, "count 45"},
                        {glblFldFn18, ".x"},
                        {glblFldFn20, ".x 43"},
                        {glblSome19, "Some"},
                        {glblCFn108, "str-vect"},
                        {glblstr_vect367, "str-vect 2"},
                        {glblstr_vect365, "str-vect 5"},
                        {glblstr_vect202, "str-vect 43"},
                        {glblstr_vect534, "str-vect 8"},
                        {glblstr_vect263, "str-vect 1"},
                        {glblCFn119, "inc"},
                        {glblCFn58, "interpose"},
                        {glblinterpose496, "interpose 8"},
                        {glblCFn123, "-"},
                        {glblCFn448, "subvec"},
                        {glblCFn126, "dec"},
                        {glblCFn133, "vect-conj"},
                        {glblCFn36, "comp"},
                        {glblcomp522, "comp 8"},
                        {glblCFn0, "default-type-name"},
                        {glblCFn104, "keys"},
                        {glblCFn56, "apply"},
                        {glblCFn28, "or"},
                        {glblor188, "or 43"},
                        {glblor138, "or 42"},
                        {glblc456, "core: 681"},
                        {glblCFn32, "cond"},
                        {glblcond192, "cond 43"},
                        {glblcond142, "cond 42"},
                        {glblcond257, "cond 1"},
                        {glblCFn78, "last"},
                        {glbllast518, "last 8"},
                        {glblFldFn318, ".len"},
                        {glblFldFn332, ".len 5"},
                        {glblFldFn600, ".len 45"},
                        {glblCFn100, "assoc"},
                        {glblc163, "core: 315"},
                        {glblCFn293, "str-count"},
                        {glblCFn54, "extract"},
                        {glblextract198, "extract 43"},
                        {glblCFn50, "duplicate"},
                        {glblCFn274, "identity"},
                        {glblSubString327, "SubString"},
                        {glblCFn110, "free-resource"},
                        {glblCFn102, "get"},
                        {glblget530, "get 8"},
                        {glblCFn208, "int-<"},
                        {glblCFn677, "pr0"},
                        {glblCFn685, "output"},
                        {glblrep_E727, "rep-E"},
                        {glblCFn667, ">>"},
                        {glblhbin_done690, "hbin-done"},
                        {glblCFn688, "hbin"},
                        {glblCFn701, "clr1"},
                        {glblCFn664, "/2"},
                        {glblbin_0697, "bin-0"},
                        {glblCFn662, "bit-and"},
                        {glblCFn721, "simplest"},
                        {glblrep_I725, "rep-I"},
                        {glblCFn659, "str-to-int"},
                        {glblCFn673, "prLF"},
                        {glblCFn707, "clr2"},
                        {glblfalse750, "false"},
                        {glblCFn733, "counter"},
                        {glblclr2_1714, "clr2-1"},
                        {glblCFn745, "main"},
                        {glblclr2_0712, "clr2-0"},
                        {glblCFn742, "bad-simplest"},
                        {glblCFn681, "pr1"},
                        {glblrep_O723, "rep-O"},
                        {glblinput747, "input"},
                        {glblCFn738, "bad-clr"},
                        {glblCFn719, "rep"},
                        {glblCFn694, "bin"}};
typeNameMap typeNames[21] = {
    {1, "Integer"},    {2, "String"},     {8, "Vector"},
    {9, "VectorNode"}, {11, "HashMap"},   {12, "HashMap"},
    {13, "HashMap"},   {14, "HashMap"},   {11, "BitmapIndexedNode"},
    {45, "Cons"},      {18, "Opaque"},    {3, "FnArity"},
    {12, "ArrayNode"}, {43, "Some"},      {13, "HashCollisionNode"},
    {42, "None"},      {46, "EndOfList"}, {44, "Leaf"},
    {5, "SubString"}};
void normGlobals() {
  Term apps, lams, V;
  Location vLoc;
  {
    Term glblVal14_1;
    glblVal14 = dupeArg(glblVal14, &glblVal14_1, 0);

    // call None at core: 66
    Term glblRslt13Args = makePair(APP, 0, NUL, SUB);
    glblRslt13 = newTerm(VAR, 0, portLoc(2, glblRslt13Args));
    pushRedex(glblRslt13Args, glblVal14_1);
  }
  {
    Term glblVal297_1;
    glblVal297 = dupeArg(glblVal297, &glblVal297_1, 0);

    // call Leaf at core: 431
    Term glblRslt296Args = makePair(APP, 0, NUL, SUB);
    glblRslt296 = newTerm(VAR, 0, portLoc(2, glblRslt296Args));
    pushRedex(glblRslt296Args, glblVal297_1);
  }
  {
    Term glblVal556_1;
    glblVal556 = dupeArg(glblVal556, &glblVal556_1, 0);

    // call EndOfList at core: 798
    Term glblRslt555Args = makePair(APP, 0, NUL, SUB);
    glblRslt555 = newTerm(VAR, 0, portLoc(2, glblRslt555Args));
    pushRedex(glblRslt555Args, glblVal556_1);
  }
  normalize(NULL);
  if (termTag(glblVal605) == VAR)
    glblVal605 = take(termLoc(glblVal605));
  if (termTag(glblRslt555) == VAR)
    glblRslt555 = take(termLoc(glblRslt555));
  if (termTag(glblVal556) == VAR)
    glblVal556 = take(termLoc(glblVal556));
  if (termTag(glblVal450) == VAR)
    glblVal450 = take(termLoc(glblVal450));
  if (termTag(glblVal334) == VAR)
    glblVal334 = take(termLoc(glblVal334));
  if (termTag(glblRslt296) == VAR)
    glblRslt296 = take(termLoc(glblRslt296));
  if (termTag(glblVal297) == VAR)
    glblVal297 = take(termLoc(glblVal297));
  if (termTag(glblVal214) == VAR)
    glblVal214 = take(termLoc(glblVal214));
  if (termTag(glblVal22) == VAR)
    glblVal22 = take(termLoc(glblVal22));
  if (termTag(glblRslt13) == VAR)
    glblRslt13 = take(termLoc(glblRslt13));
  if (termTag(glblVal14) == VAR)
    glblVal14 = take(termLoc(glblVal14));
  if (termTag(glblVal10) == VAR)
    glblVal10 = take(termLoc(glblVal10));
}
void freeGlobals() {
  freeGlobal(glblVal605);
  freeGlobal(glblRslt555);
  freeGlobal(glblVal556);
  freeGlobal(glblVal450);
  freeGlobal(glblVal334);
  freeGlobal(glblRslt296);
  freeGlobal(glblVal297);
  freeGlobal(glblVal214);
  freeGlobal(glblVal22);
  freeGlobal(glblRslt13);
  freeGlobal(glblVal14);
  freeGlobal(glblVal10);
  normalize(NULL);
}
Term mainFn = newRef(glblCFn745);
int typeCount = 47;
