
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdatomic.h>
#include "runtime3.h"

// TODO: these are only needed until core.toc codegen is complete
Value *noImpl1(FnArity *arity, Value *str_0) {
abort();
}

Value *noImpl2(FnArity *arity, Value *str_0, Value *arg1) {
abort();
}

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

dupLabels[0] = "type-name";
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

    switch(term_tag(x_1)) {
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
Term glblVal9 = new_ref(graphFn);;
Term glblVal9;
Term glblProto17;
void glblSome19(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(1), args);
callArgs = pair_make(APP, 0, new_i60(43), callArgs);
interact(callArgs, construct);
return;
}
void glblNone12(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(0), args);
callArgs = pair_make(APP, 0, new_i60(42), callArgs);
interact(callArgs, construct);
return;
}
Term glblVal14 = new_ref(glblNone12);
Term glblVal14;
Term glblRslt13;
Term glblVal22 = new_ref(glblSome19);
Term glblVal22;
Term glblFld21;
void glblFldFn20(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(0), args);
store_redex(args, accessField);
return;
}
Term glblFld21 = new_ref(glblFldFn20);
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

    if (term_tag(str_1) != VAL) {
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
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn112 = new_ref(glblCFn111);
Term glblFn115;
void glblCFn114(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 1) {
  Term str_1 = arityArgs.args[0];

    if (term_tag(str_1) != VAL) {
      fprintf(stderr, "\ninvalid type for 'pr*': Integer or Float\n");
      abort();
    }
    Value *s = (Value *)((u64)str_1 & ~7);
    if (s->type == StringBufferType) {
      fprintf(stdout, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    } else if (s->type == SubStringType) {
      ReifiedVal *ss = (ReifiedVal *)str_1;
      String *parent = (String *)ss->impls[0];
      long start = get_i60(ss->impls[1]);
      int len = (int)get_i60(ss->impls[2]);
      fprintf(stdout, "%-.*s", len, &parent->buffer[start]);
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
Term glblFn115 = new_ref(glblCFn114);
Term glblFn118;
void glblCFn117(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 2) {
  Term x_1 = arityArgs.args[0];
  Term y_2 = arityArgs.args[1];

    if (term_tag(x_1) != I60 && term_tag(y_2) != I60) {
      BOOM("Invalid inputs to '+'");
    }
    result = new_i60(get_i60(x_1) + get_i60(y_2));

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn118 = new_ref(glblCFn117);
Term glblFn121;
void glblCFn120(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 9, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 264
Term Rslt_4 = make_op(OP_ADD, new_i60(1), x_1);
dupLabels[9] = "";
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
Term glblFn121 = new_ref(glblCFn120);
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

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn124 = new_ref(glblCFn123);
Term glblFn127;
void glblCFn126(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 11, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 276
Term Rslt_4 = make_op(OP_SUB, x_1, new_i60(1));
dupLabels[11] = "";
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
Term glblFn127 = new_ref(glblCFn126);
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

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn130 = new_ref(glblCFn129);
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

  result = term_val((Term)vectConj((Vector *)vect_1, v_2));
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn134 = new_ref(glblCFn133);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[5];
} str0 = {StringBufferType, REFS_STATIC, 0, 0, 4, "core"};
Term glblStr151 = term_new_(VAL, (Term)&str0);
void glblProto15_150(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(296), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
Term glbl_EQ_149;
void glbl_EQ_148(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_8 = pair_make(LAM, 20, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term y_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 20, SUB, seq_8);
Term x_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at core: 295
// call type-num at core: 296
Term Rslt_10Args = pair_make(APP, 0, x_6, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
store_redex(Rslt_10Args, glblFn8);
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn8));
#endif

// call type-num at core: 296
Term Rslt_11Args = pair_make(APP, 0, y_7, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
store_redex(Rslt_11Args, glblFn8);
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn8));
#endif

// call = at core: 296
Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
Rslt_12Args = pair_make(APP, 0, Rslt_10, Rslt_12Args);
#ifdef STRICT
store_redex(Rslt_12Args, new_ref(glblProto15_150));
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, new_ref(glblProto15_150)));
#endif
dupLabels[20] = "=";
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
Term glbl_EQ_149 = new_ref(glbl_EQ_148);
Term glblrecurse147;
void glblrecurse146(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_16 = pair_make(LAM, 19, SUB, NUL);
Location r_seq_16 = port(2, term_loc(seq_16));
Term f_15 = term_new(VAR, 0, port(1, term_loc(seq_16)));
seq_16 = pair_make(LAM, 19, SUB, seq_16);
Term v_14 = term_new(VAR, 0, port(1, term_loc(seq_16)));
// allocate args at core: 297
swapStore(term_loc(f_15), ERA);
dupLabels[19] = "recurse";
// link args to body
swapStore(r_seq_16, v_14);

V = get(r_seq_16);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 19, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_16, term_new(VAR, 0, vLoc));
store_redex(args, seq_16);
return;
}
Term glblrecurse147 = new_ref(glblrecurse146);
Term glbleither145;
void glbleither144(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_21 = pair_make(LAM, 18, SUB, NUL);
Location r_seq_21 = port(2, term_loc(seq_21));
Term cont_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 18, SUB, seq_21);
Term v_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
// allocate args at core: 299
swapStore(term_loc(v_19), ERA);
dupLabels[18] = "either";
// link args to body
swapStore(r_seq_21, cont_20);

V = get(r_seq_21);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 18, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_21, term_new(VAR, 0, vLoc));
store_redex(args, seq_21);
return;
}
Term glbleither145 = new_ref(glbleither144);
Term glblcond143;
void glblcond142(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_27 = pair_make(LAM, 17, SUB, NUL);
Location r_seq_27 = port(2, term_loc(seq_27));
Term cont_26 = term_new(VAR, 0, port(1, term_loc(seq_27)));
seq_27 = pair_make(LAM, 17, SUB, seq_27);
Term x_25 = term_new(VAR, 0, port(1, term_loc(seq_27)));
seq_27 = pair_make(LAM, 17, SUB, seq_27);
Term v_24 = term_new(VAR, 0, port(1, term_loc(seq_27)));
// allocate args at core: 301
swapStore(term_loc(v_24), ERA);
swapStore(term_loc(x_25), ERA);
dupLabels[17] = "cond";
// link args to body
swapStore(r_seq_27, cont_26);

V = get(r_seq_27);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 17, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_27, term_new(VAR, 0, vLoc));
store_redex(args, seq_27);
return;
}
Term glblcond143 = new_ref(glblcond142);
Term glbland141;
void glbland140(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_32 = pair_make(LAM, 16, SUB, NUL);
Location r_seq_32 = port(2, term_loc(seq_32));
Term arg_31 = term_new(VAR, 0, port(1, term_loc(seq_32)));
seq_32 = pair_make(LAM, 16, SUB, seq_32);
Term x_30 = term_new(VAR, 0, port(1, term_loc(seq_32)));
// allocate args at core: 303
swapStore(term_loc(arg_31), ERA);
dupLabels[16] = "and";
// link args to body
swapStore(r_seq_32, x_30);

V = get(r_seq_32);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 16, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_32, term_new(VAR, 0, vLoc));
store_redex(args, seq_32);
return;
}
Term glbland141 = new_ref(glbland140);
Term glblor139;
void glblor138(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_37 = pair_make(LAM, 15, SUB, NUL);
Location r_seq_37 = port(2, term_loc(seq_37));
Term y_36 = term_new(VAR, 0, port(1, term_loc(seq_37)));
seq_37 = pair_make(LAM, 15, SUB, seq_37);
Term arg_35 = term_new(VAR, 0, port(1, term_loc(seq_37)));
// allocate args at core: 305
swapStore(term_loc(arg_35), ERA);
dupLabels[15] = "or";
// link args to body
swapStore(r_seq_37, y_36);

V = get(r_seq_37);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 15, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_37, term_new(VAR, 0, vLoc));
store_redex(args, seq_37);
return;
}
Term glblor139 = new_ref(glblor138);
Term glblmap137;
void glblmap136(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_42 = pair_make(LAM, 14, SUB, NUL);
Location r_seq_42 = port(2, term_loc(seq_42));
Term f_41 = term_new(VAR, 0, port(1, term_loc(seq_42)));
seq_42 = pair_make(LAM, 14, SUB, seq_42);
Term x_40 = term_new(VAR, 0, port(1, term_loc(seq_42)));
// allocate args at core: 307
swapStore(term_loc(f_41), ERA);
dupLabels[14] = "map";
// link args to body
swapStore(r_seq_42, x_40);

V = get(r_seq_42);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 14, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_42, term_new(VAR, 0, vLoc));
store_redex(args, seq_42);
return;
}
Term glblmap137 = new_ref(glblmap136);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str1 = {StringBufferType, REFS_STATIC, 0, 0, 1, "\n"};
Term glblStr161 = term_new_(VAL, (Term)&str1);
void glblProto65_167(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(312), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto107_169(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(314), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto107);
}
}
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str2 = {StringBufferType, REFS_STATIC, 0, 0, 1, " "};
Term glblStr162 = term_new_(VAL, (Term)&str2);
void glblProto65_171(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(314), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto65_173(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(316), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
Term glblc164;
void glblc163(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_15 = pair_make(LAM, 22, SUB, NUL);
Location r_seq_15 = port(2, term_loc(seq_15));
Term s_14 = term_new(VAR, 0, port(1, term_loc(seq_15)));
seq_15 = pair_make(LAM, 22, SUB, seq_15);
Term v_13 = term_new(VAR, 0, port(1, term_loc(seq_15)));
// allocate args at core: 315
// call pr* at core: 316
Term Rslt_17Args = pair_make(APP, 0, s_14, SUB);
Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
#ifdef STRICT
store_redex(Rslt_17Args, glblFn115);
#else
swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, glblFn115));
#endif

// call conj at core: 316
Term Rslt_18Args = pair_make(APP, 0, Rslt_17, SUB);
Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
Rslt_18Args = pair_make(APP, 0, v_13, Rslt_18Args);
#ifdef STRICT
store_redex(Rslt_18Args, new_ref(glblProto65_173));
#else
swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, new_ref(glblProto65_173)));
#endif
dupLabels[22] = "";
// link args to body
swapStore(r_seq_15, Rslt_18);

Term Rslt_18V = get(term_loc(Rslt_18));
apps = take(port(1, term_loc(Rslt_18V)));
lams = take(port(2, term_loc(Rslt_18V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_15);
return;
}
Term glblc164 = new_ref(glblc163);
void glblProto69_176(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(314), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto69);
}
}
void glblProto65_178(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(314), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
Term glblc166;
void glblc165(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_8 = pair_make(LAM, 23, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term val_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 23, SUB, seq_8);
Term v_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at core: 313
Term glblVal131_2;
glblVal131 = dupeArg(glblVal131, &glblVal131_2, 0);

// call str-vect at core: 314
Term Rslt_10Args = pair_make(APP, 0, val_7, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
store_redex(Rslt_10Args, new_ref(glblProto107_169));
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, new_ref(glblProto107_169)));
#endif
// call conj at core: 314
Term Rslt_11Args = pair_make(APP, 0, glblStr162, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
Rslt_11Args = pair_make(APP, 0, Rslt_10, Rslt_11Args);
#ifdef STRICT
store_redex(Rslt_11Args, new_ref(glblProto65_171));
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, new_ref(glblProto65_171)));
#endif
// call reduce at core: 314
Term Rslt_19Args = pair_make(APP, 0, glblc164, SUB);
Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
Rslt_19Args = pair_make(APP, 0, glblVal131_2, Rslt_19Args);
Rslt_19Args = pair_make(APP, 0, Rslt_11, Rslt_19Args);
#ifdef STRICT
store_redex(Rslt_19Args, new_ref(glblProto69_176));
#else
swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, new_ref(glblProto69_176)));
#endif
// call conj at core: 314
Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
Rslt_20Args = pair_make(APP, 0, v_6, Rslt_20Args);
#ifdef STRICT
store_redex(Rslt_20Args, new_ref(glblProto65_178));
#else
swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, new_ref(glblProto65_178)));
#endif
dupLabels[23] = "";
// link args to body
swapStore(r_seq_8, Rslt_20);

Term Rslt_20V = get(term_loc(Rslt_20));
apps = take(port(1, term_loc(Rslt_20V)));
lams = take(port(2, term_loc(Rslt_20V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_8);
return;
}
Term glblc166 = new_ref(glblc165);
void glblProto69_181(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(312), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto69);
}
}
Term glblFn160;
void glblCFn159(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 21, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term vs_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 310
Term glblVal131_1;
glblVal131 = dupeArg(glblVal131, &glblVal131_1, 0);

// call conj at core: 312
Term Rslt_4Args = pair_make(APP, 0, glblStr161, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
Rslt_4Args = pair_make(APP, 0, vs_1, Rslt_4Args);
#ifdef STRICT
store_redex(Rslt_4Args, new_ref(glblProto65_167));
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, new_ref(glblProto65_167)));
#endif
// call reduce at core: 312
Term Rslt_21Args = pair_make(APP, 0, glblc166, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
Rslt_21Args = pair_make(APP, 0, glblVal131_1, Rslt_21Args);
Rslt_21Args = pair_make(APP, 0, Rslt_4, Rslt_21Args);
#ifdef STRICT
store_redex(Rslt_21Args, new_ref(glblProto69_181));
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, new_ref(glblProto69_181)));
#endif
dupLabels[21] = "";
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
Term glblFn160 = new_ref(glblCFn159);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[7];
} str3 = {StringBufferType, REFS_STATIC, 0, 0, 6, "(Some "};
Term glblStr207 = term_new_(VAL, (Term)&str3);
void glblProto17_215(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(321), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto107_217(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(321), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto107);
}
}
void glblProto35_219(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(321), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto35);
}
}
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str4 = {StringBufferType, REFS_STATIC, 0, 0, 1, ")"};
Term glblStr206 = term_new_(VAL, (Term)&str4);
Term glblstr_vect203;
void glblstr_vect202(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_24 = pair_make(LAM, 33, SUB, NUL);
Location r_seq_24 = port(2, term_loc(seq_24));
Term s_23 = term_new(VAR, 0, port(1, term_loc(seq_24)));
// allocate args at core: 319
Term glblVal131_3;
glblVal131 = dupeArg(glblVal131, &glblVal131_3, 0);

// call vect-conj at core: 320
Term Rslt_26Args = pair_make(APP, 0, glblStr207, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
Rslt_26Args = pair_make(APP, 0, glblVal131_3, Rslt_26Args);
#ifdef STRICT
store_redex(Rslt_26Args, glblFn134);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, glblFn134));
#endif

// call .x at core: 321
Term Rslt_27Args = pair_make(APP, 0, s_23, SUB);
Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
#ifdef STRICT
store_redex(Rslt_27Args, new_ref(glblProto17_215));
#else
swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, new_ref(glblProto17_215)));
#endif
// call str-vect at core: 321
Term Rslt_28Args = pair_make(APP, 0, Rslt_27, SUB);
Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
#ifdef STRICT
store_redex(Rslt_28Args, new_ref(glblProto107_217));
#else
swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, new_ref(glblProto107_217)));
#endif
// call comp at core: 321
Term Rslt_29Args = pair_make(APP, 0, Rslt_28, SUB);
Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
Rslt_29Args = pair_make(APP, 0, Rslt_26, Rslt_29Args);
#ifdef STRICT
store_redex(Rslt_29Args, new_ref(glblProto35_219));
#else
swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, new_ref(glblProto35_219)));
#endif
// call vect-conj at core: 322
Term Rslt_30Args = pair_make(APP, 0, glblStr206, SUB);
Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
Rslt_30Args = pair_make(APP, 0, Rslt_29, Rslt_30Args);
#ifdef STRICT
store_redex(Rslt_30Args, glblFn134);
#else
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblFn134));
#endif

dupLabels[33] = "str-vect";
// link args to body
swapStore(r_seq_24, Rslt_30);

Term Rslt_30V = get(term_loc(Rslt_30));
apps = take(port(1, term_loc(Rslt_30V)));
lams = take(port(2, term_loc(Rslt_30V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_24);
return;
}
Term glblstr_vect203 = new_ref(glblstr_vect202);
void glblProto15_222(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(324), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto17_224(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(325), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto17_226(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(325), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto15_228(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(325), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto25_230(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(324), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
Term glbl_EQ_201;
void glbl_EQ_200(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_34 = pair_make(LAM, 32, SUB, NUL);
Location r_seq_34 = port(2, term_loc(seq_34));
Term y_33 = term_new(VAR, 0, port(1, term_loc(seq_34)));
seq_34 = pair_make(LAM, 32, SUB, seq_34);
Term x_32 = term_new(VAR, 0, port(1, term_loc(seq_34)));
// allocate args at core: 323

Term x_32_1;
x_32 = dupeArg(x_32, &x_32_1, 32);

Term y_33_1;
y_33 = dupeArg(y_33, &y_33_1, 32);
// call type-num at core: 324
Term Rslt_36Args = pair_make(APP, 0, x_32, SUB);
Term Rslt_36 = term_new(VAR, 0, port(2, term_loc(Rslt_36Args)));
#ifdef STRICT
store_redex(Rslt_36Args, glblFn8);
#else
swapStore(term_loc(Rslt_36), pair_make(LAZ, 0, Rslt_36Args, glblFn8));
#endif

// call type-num at core: 324
Term Rslt_37Args = pair_make(APP, 0, y_33, SUB);
Term Rslt_37 = term_new(VAR, 0, port(2, term_loc(Rslt_37Args)));
#ifdef STRICT
store_redex(Rslt_37Args, glblFn8);
#else
swapStore(term_loc(Rslt_37), pair_make(LAZ, 0, Rslt_37Args, glblFn8));
#endif

// call = at core: 324
Term Rslt_38Args = pair_make(APP, 0, Rslt_37, SUB);
Term Rslt_38 = term_new(VAR, 0, port(2, term_loc(Rslt_38Args)));
Rslt_38Args = pair_make(APP, 0, Rslt_36, Rslt_38Args);
#ifdef STRICT
store_redex(Rslt_38Args, new_ref(glblProto15_222));
#else
swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, Rslt_38Args, new_ref(glblProto15_222)));
#endif
// call .x at core: 325
Term Rslt_39Args = pair_make(APP, 0, x_32_1, SUB);
Term Rslt_39 = term_new(VAR, 0, port(2, term_loc(Rslt_39Args)));
#ifdef STRICT
store_redex(Rslt_39Args, new_ref(glblProto17_224));
#else
swapStore(term_loc(Rslt_39), pair_make(LAZ, 0, Rslt_39Args, new_ref(glblProto17_224)));
#endif
// call .x at core: 325
Term Rslt_40Args = pair_make(APP, 0, y_33_1, SUB);
Term Rslt_40 = term_new(VAR, 0, port(2, term_loc(Rslt_40Args)));
#ifdef STRICT
store_redex(Rslt_40Args, new_ref(glblProto17_226));
#else
swapStore(term_loc(Rslt_40), pair_make(LAZ, 0, Rslt_40Args, new_ref(glblProto17_226)));
#endif
// call = at core: 325
Term Rslt_41Args = pair_make(APP, 0, Rslt_40, SUB);
Term Rslt_41 = term_new(VAR, 0, port(2, term_loc(Rslt_41Args)));
Rslt_41Args = pair_make(APP, 0, Rslt_39, Rslt_41Args);
#ifdef STRICT
store_redex(Rslt_41Args, new_ref(glblProto15_228));
#else
swapStore(term_loc(Rslt_41), pair_make(LAZ, 0, Rslt_41Args, new_ref(glblProto15_228)));
#endif
// call and at core: 324
Term Rslt_42Args = pair_make(APP, 0, Rslt_41, SUB);
Term Rslt_42 = term_new(VAR, 0, port(2, term_loc(Rslt_42Args)));
Rslt_42Args = pair_make(APP, 0, Rslt_38, Rslt_42Args);
#ifdef STRICT
store_redex(Rslt_42Args, new_ref(glblProto25_230));
#else
swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, Rslt_42Args, new_ref(glblProto25_230)));
#endif
dupLabels[32] = "=";
// link args to body
swapStore(r_seq_34, Rslt_42);

Term Rslt_42V = get(term_loc(Rslt_42));
apps = take(port(1, term_loc(Rslt_42V)));
lams = take(port(2, term_loc(Rslt_42V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_34);
return;
}
Term glbl_EQ_201 = new_ref(glbl_EQ_200);
void glblProto17_233(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(327), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
Term glblextract199;
void glblextract198(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_45 = pair_make(LAM, 31, SUB, NUL);
Location r_seq_45 = port(2, term_loc(seq_45));
Term v_44 = term_new(VAR, 0, port(1, term_loc(seq_45)));
// allocate args at core: 326
// call .x at core: 327
Term Rslt_47Args = pair_make(APP, 0, v_44, SUB);
Term Rslt_47 = term_new(VAR, 0, port(2, term_loc(Rslt_47Args)));
#ifdef STRICT
store_redex(Rslt_47Args, new_ref(glblProto17_233));
#else
swapStore(term_loc(Rslt_47), pair_make(LAZ, 0, Rslt_47Args, new_ref(glblProto17_233)));
#endif
dupLabels[31] = "extract";
// link args to body
swapStore(r_seq_45, Rslt_47);

Term Rslt_47V = get(term_loc(Rslt_47));
apps = take(port(1, term_loc(Rslt_47V)));
lams = take(port(2, term_loc(Rslt_47V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_47), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_45);
return;
}
Term glblextract199 = new_ref(glblextract198);
void glblProto17_236(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(329), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
Term glblrecurse197;
void glblrecurse196(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_51 = pair_make(LAM, 30, SUB, NUL);
Location r_seq_51 = port(2, term_loc(seq_51));
Term f_50 = term_new(VAR, 0, port(1, term_loc(seq_51)));
seq_51 = pair_make(LAM, 30, SUB, seq_51);
Term v_49 = term_new(VAR, 0, port(1, term_loc(seq_51)));
// allocate args at core: 328
Term glblVal22_1;
glblVal22 = dupeArg(glblVal22, &glblVal22_1, 0);

// call .x at core: 329
Term Rslt_53Args = pair_make(APP, 0, v_49, SUB);
Term Rslt_53 = term_new(VAR, 0, port(2, term_loc(Rslt_53Args)));
#ifdef STRICT
store_redex(Rslt_53Args, new_ref(glblProto17_236));
#else
swapStore(term_loc(Rslt_53), pair_make(LAZ, 0, Rslt_53Args, new_ref(glblProto17_236)));
#endif
// call f at core: 328
Term Rslt_54Args = pair_make(APP, 0, Rslt_53, SUB);
Term Rslt_54 = term_new(VAR, 0, port(2, term_loc(Rslt_54Args)));
#ifdef STRICT
store_redex(Rslt_54Args, f_50);
#else
swapStore(term_loc(Rslt_54), pair_make(LAZ, 0, Rslt_54Args, f_50));
#endif

// call Some at core: 329
Term Rslt_55Args = pair_make(APP, 0, Rslt_54, SUB);
Term Rslt_55 = term_new(VAR, 0, port(2, term_loc(Rslt_55Args)));
#ifdef STRICT
store_redex(Rslt_55Args, glblVal22_1);
#else
swapStore(term_loc(Rslt_55), pair_make(LAZ, 0, Rslt_55Args, glblVal22_1));
#endif

dupLabels[30] = "recurse";
// link args to body
swapStore(r_seq_51, Rslt_55);

Term Rslt_55V = get(term_loc(Rslt_55));
apps = take(port(1, term_loc(Rslt_55V)));
lams = take(port(2, term_loc(Rslt_55V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_55), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_51);
return;
}
Term glblrecurse197 = new_ref(glblrecurse196);
void glblProto17_239(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(331), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
Term glbleither195;
void glbleither194(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_59 = pair_make(LAM, 29, SUB, NUL);
Location r_seq_59 = port(2, term_loc(seq_59));
Term arg_58 = term_new(VAR, 0, port(1, term_loc(seq_59)));
seq_59 = pair_make(LAM, 29, SUB, seq_59);
Term v_57 = term_new(VAR, 0, port(1, term_loc(seq_59)));
// allocate args at core: 330
swapStore(term_loc(arg_58), ERA);
// call .x at core: 331
Term Rslt_61Args = pair_make(APP, 0, v_57, SUB);
Term Rslt_61 = term_new(VAR, 0, port(2, term_loc(Rslt_61Args)));
#ifdef STRICT
store_redex(Rslt_61Args, new_ref(glblProto17_239));
#else
swapStore(term_loc(Rslt_61), pair_make(LAZ, 0, Rslt_61Args, new_ref(glblProto17_239)));
#endif
dupLabels[29] = "either";
// link args to body
swapStore(r_seq_59, Rslt_61);

Term Rslt_61V = get(term_loc(Rslt_61));
apps = take(port(1, term_loc(Rslt_61V)));
lams = take(port(2, term_loc(Rslt_61V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_61), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_59);
return;
}
Term glbleither195 = new_ref(glbleither194);
Term glblcond193;
void glblcond192(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_66 = pair_make(LAM, 28, SUB, NUL);
Location r_seq_66 = port(2, term_loc(seq_66));
Term arg_65 = term_new(VAR, 0, port(1, term_loc(seq_66)));
seq_66 = pair_make(LAM, 28, SUB, seq_66);
Term clause_64 = term_new(VAR, 0, port(1, term_loc(seq_66)));
seq_66 = pair_make(LAM, 28, SUB, seq_66);
Term v_63 = term_new(VAR, 0, port(1, term_loc(seq_66)));
// allocate args at core: 332
swapStore(term_loc(v_63), ERA);
swapStore(term_loc(arg_65), ERA);
dupLabels[28] = "cond";
// link args to body
swapStore(r_seq_66, clause_64);

V = get(r_seq_66);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 28, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_66, term_new(VAR, 0, vLoc));
store_redex(args, seq_66);
return;
}
Term glblcond193 = new_ref(glblcond192);
Term glbland191;
void glbland190(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_71 = pair_make(LAM, 27, SUB, NUL);
Location r_seq_71 = port(2, term_loc(seq_71));
Term y_70 = term_new(VAR, 0, port(1, term_loc(seq_71)));
seq_71 = pair_make(LAM, 27, SUB, seq_71);
Term arg_69 = term_new(VAR, 0, port(1, term_loc(seq_71)));
// allocate args at core: 334
swapStore(term_loc(arg_69), ERA);
dupLabels[27] = "and";
// link args to body
swapStore(r_seq_71, y_70);

V = get(r_seq_71);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 27, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_71, term_new(VAR, 0, vLoc));
store_redex(args, seq_71);
return;
}
Term glbland191 = new_ref(glbland190);
Term glblor189;
void glblor188(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_76 = pair_make(LAM, 26, SUB, NUL);
Location r_seq_76 = port(2, term_loc(seq_76));
Term arg_75 = term_new(VAR, 0, port(1, term_loc(seq_76)));
seq_76 = pair_make(LAM, 26, SUB, seq_76);
Term q_74 = term_new(VAR, 0, port(1, term_loc(seq_76)));
// allocate args at core: 336
swapStore(term_loc(arg_75), ERA);
dupLabels[26] = "or";
// link args to body
swapStore(r_seq_76, q_74);

V = get(r_seq_76);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 26, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_76, term_new(VAR, 0, vLoc));
store_redex(args, seq_76);
return;
}
Term glblor189 = new_ref(glblor188);
void glblProto17_245(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(339), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
Term glblmap187;
void glblmap186(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_81 = pair_make(LAM, 25, SUB, NUL);
Location r_seq_81 = port(2, term_loc(seq_81));
Term f_80 = term_new(VAR, 0, port(1, term_loc(seq_81)));
seq_81 = pair_make(LAM, 25, SUB, seq_81);
Term v_79 = term_new(VAR, 0, port(1, term_loc(seq_81)));
// allocate args at core: 338
Term glblVal22_2;
glblVal22 = dupeArg(glblVal22, &glblVal22_2, 0);

// call .x at core: 339
Term Rslt_83Args = pair_make(APP, 0, v_79, SUB);
Term Rslt_83 = term_new(VAR, 0, port(2, term_loc(Rslt_83Args)));
#ifdef STRICT
store_redex(Rslt_83Args, new_ref(glblProto17_245));
#else
swapStore(term_loc(Rslt_83), pair_make(LAZ, 0, Rslt_83Args, new_ref(glblProto17_245)));
#endif
// call f at core: 338
Term Rslt_84Args = pair_make(APP, 0, Rslt_83, SUB);
Term Rslt_84 = term_new(VAR, 0, port(2, term_loc(Rslt_84Args)));
#ifdef STRICT
store_redex(Rslt_84Args, f_80);
#else
swapStore(term_loc(Rslt_84), pair_make(LAZ, 0, Rslt_84Args, f_80));
#endif

// call Some at core: 339
Term Rslt_85Args = pair_make(APP, 0, Rslt_84, SUB);
Term Rslt_85 = term_new(VAR, 0, port(2, term_loc(Rslt_85Args)));
#ifdef STRICT
store_redex(Rslt_85Args, glblVal22_2);
#else
swapStore(term_loc(Rslt_85), pair_make(LAZ, 0, Rslt_85Args, glblVal22_2));
#endif

dupLabels[25] = "map";
// link args to body
swapStore(r_seq_81, Rslt_85);

Term Rslt_85V = get(term_loc(Rslt_85));
apps = take(port(1, term_loc(Rslt_85V)));
lams = take(port(2, term_loc(Rslt_85V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_85), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_81);
return;
}
Term glblmap187 = new_ref(glblmap186);
void glblProto17_248(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(341), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
Term glblflat_map185;
void glblflat_map184(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_89 = pair_make(LAM, 24, SUB, NUL);
Location r_seq_89 = port(2, term_loc(seq_89));
Term f_88 = term_new(VAR, 0, port(1, term_loc(seq_89)));
seq_89 = pair_make(LAM, 24, SUB, seq_89);
Term v_87 = term_new(VAR, 0, port(1, term_loc(seq_89)));
// allocate args at core: 340
// call .x at core: 341
Term Rslt_91Args = pair_make(APP, 0, v_87, SUB);
Term Rslt_91 = term_new(VAR, 0, port(2, term_loc(Rslt_91Args)));
#ifdef STRICT
store_redex(Rslt_91Args, new_ref(glblProto17_248));
#else
swapStore(term_loc(Rslt_91), pair_make(LAZ, 0, Rslt_91Args, new_ref(glblProto17_248)));
#endif
// call f at core: 340
Term Rslt_92Args = pair_make(APP, 0, Rslt_91, SUB);
Term Rslt_92 = term_new(VAR, 0, port(2, term_loc(Rslt_92Args)));
#ifdef STRICT
store_redex(Rslt_92Args, f_88);
#else
swapStore(term_loc(Rslt_92), pair_make(LAZ, 0, Rslt_92Args, f_88));
#endif

dupLabels[24] = "flat-map";
// link args to body
swapStore(r_seq_89, Rslt_92);

Term Rslt_92V = get(term_loc(Rslt_92));
apps = take(port(1, term_loc(Rslt_92V)));
lams = take(port(2, term_loc(Rslt_92V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_92), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_89);
return;
}
Term glblflat_map185 = new_ref(glblflat_map184);
Term glblFn205;
void glblCFn204(Term ref, Term args) {
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
Term glblFn205 = new_ref(glblCFn204);
Term glblFn209;
void glblCFn208(Term ref, Term args) {
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
Term glblFn209 = new_ref(glblCFn208);
Term glblFn211;
void glblCFn210(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 1) {
  Term n_1 = arityArgs.args[0];

    if (term_tag(n_1) == VAL) {
      fprintf(stderr, "\ninvalid type for 'number-str': Value\n");
      abort();
    }
    result = number_str(n_1);

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn211 = new_ref(glblCFn210);
void glblProto27_254(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(366), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto27);
}
}
Term glblFn213;
void glblCFn212(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 37, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 37, SUB, seq_3);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 365

Term x_1_1;
x_1 = dupeArg(x_1, &x_1_1, 37);

Term y_2_1;
y_2 = dupeArg(y_2, &y_2_1, 37);
// call int-< at core: 366
Term Rslt_5Args = pair_make(APP, 0, y_2, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, x_1, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, glblFn209);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn209));
#endif

// call int-= at core: 367
Term Rslt_6Args = pair_make(APP, 0, y_2_1, SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
Rslt_6Args = pair_make(APP, 0, x_1_1, Rslt_6Args);
#ifdef STRICT
store_redex(Rslt_6Args, glblFn205);
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn205));
#endif

// call or at core: 366
Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
Rslt_7Args = pair_make(APP, 0, Rslt_5, Rslt_7Args);
#ifdef STRICT
store_redex(Rslt_7Args, new_ref(glblProto27_254));
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, new_ref(glblProto27_254)));
#endif
dupLabels[37] = "";
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
Term glblFn213 = new_ref(glblCFn212);
Term glblVal214 = new_ref(intCond);;
Term glblVal214;
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[8];
} str5 = {StringBufferType, REFS_STATIC, 0, 0, 7, "Integer"};
Term glblStr267 = term_new_(VAL, (Term)&str5);
Term glbltype_name266;
void glbltype_name265(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 42, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term arg_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 372
swapStore(term_loc(arg_1), ERA);
dupLabels[42] = "type-name";
// link args to body
swapStore(r_seq_2, glblStr267);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 42, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glbltype_name266 = new_ref(glbltype_name265);
Term glblstr_vect264;
void glblstr_vect263(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_6 = pair_make(LAM, 41, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term n_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at core: 375
Term glblVal131_4;
glblVal131 = dupeArg(glblVal131, &glblVal131_4, 0);

// call number-str at core: 376
Term Rslt_8Args = pair_make(APP, 0, n_5, SUB);
Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
store_redex(Rslt_8Args, glblFn211);
#else
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn211));
#endif

// call vect-conj at core: 376
Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
Rslt_9Args = pair_make(APP, 0, glblVal131_4, Rslt_9Args);
#ifdef STRICT
store_redex(Rslt_9Args, glblFn134);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn134));
#endif

dupLabels[41] = "str-vect";
// link args to body
swapStore(r_seq_6, Rslt_9);

Term Rslt_9V = get(term_loc(Rslt_9));
apps = take(port(1, term_loc(Rslt_9V)));
lams = take(port(2, term_loc(Rslt_9V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_6);
return;
}
Term glblstr_vect264 = new_ref(glblstr_vect263);
Term glbl_EQ_262;
void glbl_EQ_261(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_13 = pair_make(LAM, 40, SUB, NUL);
Location r_seq_13 = port(2, term_loc(seq_13));
Term y_12 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 40, SUB, seq_13);
Term x_11 = term_new(VAR, 0, port(1, term_loc(seq_13)));
// allocate args at core: 378
// call int-= at core: 379
Term Rslt_15Args = pair_make(APP, 0, y_12, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
Rslt_15Args = pair_make(APP, 0, x_11, Rslt_15Args);
#ifdef STRICT
store_redex(Rslt_15Args, glblFn205);
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn205));
#endif

dupLabels[40] = "=";
// link args to body
swapStore(r_seq_13, Rslt_15);

Term Rslt_15V = get(term_loc(Rslt_15));
apps = take(port(1, term_loc(Rslt_15V)));
lams = take(port(2, term_loc(Rslt_15V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_13);
return;
}
Term glbl_EQ_262 = new_ref(glbl_EQ_261);
Term glbl_LT_260;
void glbl_LT_259(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_19 = pair_make(LAM, 39, SUB, NUL);
Location r_seq_19 = port(2, term_loc(seq_19));
Term y_18 = term_new(VAR, 0, port(1, term_loc(seq_19)));
seq_19 = pair_make(LAM, 39, SUB, seq_19);
Term x_17 = term_new(VAR, 0, port(1, term_loc(seq_19)));
// allocate args at core: 381
// call int-< at core: 382
Term Rslt_21Args = pair_make(APP, 0, y_18, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
Rslt_21Args = pair_make(APP, 0, x_17, Rslt_21Args);
#ifdef STRICT
store_redex(Rslt_21Args, glblFn209);
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblFn209));
#endif

dupLabels[39] = "<";
// link args to body
swapStore(r_seq_19, Rslt_21);

Term Rslt_21V = get(term_loc(Rslt_21));
apps = take(port(1, term_loc(Rslt_21V)));
lams = take(port(2, term_loc(Rslt_21V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_19);
return;
}
Term glbl_LT_260 = new_ref(glbl_LT_259);
Term glblcond258;
void glblcond257(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_26 = pair_make(LAM, 38, SUB, NUL);
Location r_seq_26 = port(2, term_loc(seq_26));
Term z_25 = term_new(VAR, 0, port(1, term_loc(seq_26)));
seq_26 = pair_make(LAM, 38, SUB, seq_26);
Term non_z_24 = term_new(VAR, 0, port(1, term_loc(seq_26)));
seq_26 = pair_make(LAM, 38, SUB, seq_26);
Term n_23 = term_new(VAR, 0, port(1, term_loc(seq_26)));
// allocate args at core: 384
Term glblVal214_1;
glblVal214 = dupeArg(glblVal214, &glblVal214_1, 0);

// call int-cond at core: 385
Term Rslt_28Args = pair_make(APP, 0, z_25, SUB);
Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
Rslt_28Args = pair_make(APP, 0, non_z_24, Rslt_28Args);
Rslt_28Args = pair_make(APP, 0, n_23, Rslt_28Args);
#ifdef STRICT
store_redex(Rslt_28Args, glblVal214_1);
#else
swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, glblVal214_1));
#endif

dupLabels[38] = "cond";
// link args to body
swapStore(r_seq_26, Rslt_28);

Term Rslt_28V = get(term_loc(Rslt_28));
apps = take(port(1, term_loc(Rslt_28V)));
lams = take(port(2, term_loc(Rslt_28V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_26);
return;
}
Term glblcond258 = new_ref(glblcond257);
void glblProto23_279(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(395), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto23);
}
}
void glblProto29_281(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(395), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto29);
}
}
Term glblFn269;
void glblCFn268(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 43, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 43, SUB, seq_3);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 391

Term y_2_1;
y_2 = dupeArg(y_2, &y_2_1, 43);
// call < at core: 395
Term Rslt_5Args = pair_make(APP, 0, y_2, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, x_1, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, new_ref(glblProto23_279));
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, new_ref(glblProto23_279)));
#endif
// call either at core: 395
Term Rslt_6Args = pair_make(APP, 0, y_2_1, SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
Rslt_6Args = pair_make(APP, 0, Rslt_5, Rslt_6Args);
#ifdef STRICT
store_redex(Rslt_6Args, new_ref(glblProto29_281));
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, new_ref(glblProto29_281)));
#endif
dupLabels[43] = "";
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
Term glblFn269 = new_ref(glblCFn268);
Term glblFn272;
void glblCFn271(Term ref, Term args) {
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
Term glblFn272 = new_ref(glblCFn271);
Term glblFn278;
void glblCFn277(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 45, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 405
dupLabels[45] = "";
// link args to body
swapStore(r_seq_2, x_1);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 45, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblFn278 = new_ref(glblCFn277);
void glblProto37_288(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(410), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto37);
}
}
Term glblFn287;
void glblCFn286(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 46, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 46, SUB, seq_3);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 409

Term f_2_1;
f_2 = dupeArg(f_2, &f_2_1, 46);
Term seq_7 = pair_make(LAM, 47, SUB, NUL);
Location r_seq_7 = port(2, term_loc(seq_7));
Term v_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
// allocate args at core: 410
// call fold at core: 409
Term Rslt_9Args = pair_make(APP, 0, f_2_1, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
Rslt_9Args = pair_make(APP, 0, v_6, Rslt_9Args);
#ifdef STRICT
store_redex(Rslt_9Args, glblFn287);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn287));
#endif

dupLabels[47] = "";
// link args to body
swapStore(r_seq_7, Rslt_9);

Term Rslt_9V = get(term_loc(Rslt_9));
apps = take(port(1, term_loc(Rslt_9V)));
lams = take(port(2, term_loc(Rslt_9V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, apps, lams));
// call recurse at core: 410
Term Rslt_10Args = pair_make(APP, 0, seq_7, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
Rslt_10Args = pair_make(APP, 0, x_1, Rslt_10Args);
#ifdef STRICT
store_redex(Rslt_10Args, new_ref(glblProto37_288));
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, new_ref(glblProto37_288)));
#endif
// call f at core: 409
Term Rslt_12Args = pair_make(APP, 0, Rslt_10, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
store_redex(Rslt_12Args, f_2);
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, f_2));
#endif

dupLabels[46] = "";
// link args to body
swapStore(r_seq_3, Rslt_12);

Term Rslt_12V = get(term_loc(Rslt_12));
apps = take(port(1, term_loc(Rslt_12V)));
lams = take(port(2, term_loc(Rslt_12V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_3);
return;
}
Term glblFn287 = new_ref(glblCFn286);
void glblLeaf295(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(0), args);
callArgs = pair_make(APP, 0, new_i60(44), callArgs);
interact(callArgs, construct);
return;
}
void glblProto37_298(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(420), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto37);
}
}
Term glblFn292;
void glblCFn291(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 48, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 48, SUB, seq_3);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 419

Term f_2_1;
f_2 = dupeArg(f_2, &f_2_1, 48);
Term seq_8 = pair_make(LAM, 49, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term v_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at core: 420
// call f at core: 419
Term Rslt_5Args = pair_make(APP, 0, x_1, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
store_redex(Rslt_5Args, f_2);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, f_2));
#endif

// call unfold at core: 419
Term Rslt_10Args = pair_make(APP, 0, f_2_1, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
Rslt_10Args = pair_make(APP, 0, v_7, Rslt_10Args);
#ifdef STRICT
store_redex(Rslt_10Args, glblFn292);
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblFn292));
#endif

dupLabels[49] = "";
// link args to body
swapStore(r_seq_8, Rslt_10);

Term Rslt_10V = get(term_loc(Rslt_10));
apps = take(port(1, term_loc(Rslt_10V)));
lams = take(port(2, term_loc(Rslt_10V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, apps, lams));
// call recurse at core: 420
Term Rslt_11Args = pair_make(APP, 0, seq_8, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
Rslt_11Args = pair_make(APP, 0, Rslt_5, Rslt_11Args);
#ifdef STRICT
store_redex(Rslt_11Args, new_ref(glblProto37_298));
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, new_ref(glblProto37_298)));
#endif
dupLabels[48] = "";
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
Term glblFn292 = new_ref(glblCFn291);
Term glblVal297 = new_ref(glblLeaf295);
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
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn294 = new_ref(glblCFn293);
Term glblFn303;
void glblCFn302(Term ref, Term args) {
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
Term glblFn303 = new_ref(glblCFn302);
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
  result = term_val(dest_1);
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn306 = new_ref(glblCFn305);
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
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn311 = new_ref(glblCFn310);
Term glblProto317;
Term glblProto315;
Term glblProto313;
void glblSubString327(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(3), args);
callArgs = pair_make(APP, 0, new_i60(5), callArgs);
interact(callArgs, construct);
return;
}
Term glblVal334 = new_ref(glblSubString327);
Term glblVal334;
Term glblstr_vect320;
void glblstr_vect319(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 54, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term s_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 472
Term s_1Args = pair_make(APP, 0, s_1, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(s_1Args)));
s_1Args = pair_make(APP, 0, new_i60(0), s_1Args);
store_redex(s_1Args, accessField);
Term glblVal131_5;
glblVal131 = dupeArg(glblVal131, &glblVal131_5, 0);

// call vect-conj at core: 472
Term Rslt_5Args = pair_make(APP, 0, Rslt_4, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, glblVal131_5, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, glblFn134);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn134));
#endif

dupLabels[54] = "str-vect";
// link args to body
swapStore(r_seq_2, Rslt_5);

Term Rslt_5V = get(term_loc(Rslt_5));
apps = take(port(1, term_loc(Rslt_5V)));
lams = take(port(2, term_loc(Rslt_5V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblstr_vect320 = new_ref(glblstr_vect319);
Term glblcount322;
void glblcount321(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_8 = pair_make(LAM, 55, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term arg_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at core: 473
Term arg_7Args = pair_make(APP, 0, arg_7, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(arg_7Args)));
arg_7Args = pair_make(APP, 0, new_i60(2), arg_7Args);
store_redex(arg_7Args, accessField);
dupLabels[55] = "count";
// link args to body
swapStore(r_seq_8, Rslt_10);

V = get(r_seq_8);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 55, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_8, term_new(VAR, 0, vLoc));
store_redex(args, seq_8);
return;
}
Term glblcount322 = new_ref(glblcount321);
Term glbl_EQ_324;
void glbl_EQ_323(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_14 = pair_make(LAM, 56, SUB, NUL);
Location r_seq_14 = port(2, term_loc(seq_14));
Term y_13 = term_new(VAR, 0, port(1, term_loc(seq_14)));
seq_14 = pair_make(LAM, 56, SUB, seq_14);
Term x_12 = term_new(VAR, 0, port(1, term_loc(seq_14)));
// allocate args at core: 475

Term x_12_2;
x_12 = dupeArg(x_12, &x_12_2, 56);

Term x_12_1;
x_12 = dupeArg(x_12, &x_12_1, 56);
Term x_12_2Args = pair_make(APP, 0, x_12_2, SUB);
Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(x_12_2Args)));
x_12_2Args = pair_make(APP, 0, new_i60(2), x_12_2Args);
store_redex(x_12_2Args, accessField);
Term x_12_1Args = pair_make(APP, 0, x_12_1, SUB);
Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(x_12_1Args)));
x_12_1Args = pair_make(APP, 0, new_i60(1), x_12_1Args);
store_redex(x_12_1Args, accessField);
Term x_12Args = pair_make(APP, 0, x_12, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(x_12Args)));
x_12Args = pair_make(APP, 0, new_i60(0), x_12Args);
store_redex(x_12Args, accessField);
// call str= at core: 476
Term Rslt_19Args = pair_make(APP, 0, y_13, SUB);
Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
Rslt_19Args = pair_make(APP, 0, Rslt_18, Rslt_19Args);
Rslt_19Args = pair_make(APP, 0, Rslt_17, Rslt_19Args);
Rslt_19Args = pair_make(APP, 0, Rslt_16, Rslt_19Args);
#ifdef STRICT
store_redex(Rslt_19Args, glblFn311);
#else
swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblFn311));
#endif

dupLabels[56] = "=";
// link args to body
swapStore(r_seq_14, Rslt_19);

Term Rslt_19V = get(term_loc(Rslt_19));
apps = take(port(1, term_loc(Rslt_19V)));
lams = take(port(2, term_loc(Rslt_19V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_14);
return;
}
Term glbl_EQ_324 = new_ref(glbl_EQ_323);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[1];
} str6 = {StringBufferType, REFS_STATIC, 0, 0, 0, ""};
Term glblStr335 = term_new_(VAL, (Term)&str6);
void glblProto31_339(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(480), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto31);
}
}
void glblProto61_341(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(479), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
Term glblsubs326;
void glblsubs325(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_24 = pair_make(LAM, 57, SUB, NUL);
Location r_seq_24 = port(2, term_loc(seq_24));
Term new_len_23 = term_new(VAR, 0, port(1, term_loc(seq_24)));
seq_24 = pair_make(LAM, 57, SUB, seq_24);
Term new_start_22 = term_new(VAR, 0, port(1, term_loc(seq_24)));
seq_24 = pair_make(LAM, 57, SUB, seq_24);
Term s_21 = term_new(VAR, 0, port(1, term_loc(seq_24)));
// allocate args at core: 478

Term s_21_4;
s_21 = dupeArg(s_21, &s_21_4, 57);

Term s_21_3;
s_21 = dupeArg(s_21, &s_21_3, 57);

Term s_21_2;
s_21 = dupeArg(s_21, &s_21_2, 57);

Term s_21_1;
s_21 = dupeArg(s_21, &s_21_1, 57);
Term s_21_4Args = pair_make(APP, 0, s_21_4, SUB);
Term Rslt_39 = term_new(VAR, 0, port(2, term_loc(s_21_4Args)));
s_21_4Args = pair_make(APP, 0, new_i60(0), s_21_4Args);
store_redex(s_21_4Args, accessField);
Term s_21_3Args = pair_make(APP, 0, s_21_3, SUB);
Term Rslt_37 = term_new(VAR, 0, port(2, term_loc(s_21_3Args)));
s_21_3Args = pair_make(APP, 0, new_i60(2), s_21_3Args);
store_redex(s_21_3Args, accessField);
Term s_21_2Args = pair_make(APP, 0, s_21_2, SUB);
Term Rslt_36 = term_new(VAR, 0, port(2, term_loc(s_21_2Args)));
s_21_2Args = pair_make(APP, 0, new_i60(1), s_21_2Args);
store_redex(s_21_2Args, accessField);
Term seq_28 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_28 = port(2, term_loc(seq_28));
Term final_len_27 = term_new(VAR, 0, port(1, term_loc(seq_28)));
// allocate args at core: 479

Term final_len_27_1;
final_len_27 = dupeArg(final_len_27, &final_len_27_1, 0);
Term s_21_1Args = pair_make(APP, 0, s_21_1, SUB);
Term Rslt_32 = term_new(VAR, 0, port(2, term_loc(s_21_1Args)));
s_21_1Args = pair_make(APP, 0, new_i60(1), s_21_1Args);
store_redex(s_21_1Args, accessField);
Term s_21Args = pair_make(APP, 0, s_21, SUB);
Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(s_21Args)));
s_21Args = pair_make(APP, 0, new_i60(0), s_21Args);
store_redex(s_21Args, accessField);
Term glblVal334_1;
glblVal334 = dupeArg(glblVal334, &glblVal334_1, 0);

// call <= at core: 480
Term Rslt_30Args = pair_make(APP, 0, new_i60(0), SUB);
Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
Rslt_30Args = pair_make(APP, 0, final_len_27, Rslt_30Args);
#ifdef STRICT
store_redex(Rslt_30Args, glblFn213);
#else
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblFn213));
#endif

Term Rslt_33 = make_op(OP_ADD, Rslt_32, new_start_22);
// call SubString at core: 482
Term Rslt_34Args = pair_make(APP, 0, final_len_27_1, SUB);
Term Rslt_34 = term_new(VAR, 0, port(2, term_loc(Rslt_34Args)));
Rslt_34Args = pair_make(APP, 0, Rslt_33, Rslt_34Args);
Rslt_34Args = pair_make(APP, 0, Rslt_31, Rslt_34Args);
#ifdef STRICT
store_redex(Rslt_34Args, glblVal334_1);
#else
swapStore(term_loc(Rslt_34), pair_make(LAZ, 0, Rslt_34Args, glblVal334_1));
#endif

// call cond at core: 480
Term Rslt_35Args = pair_make(APP, 0, Rslt_34, SUB);
Term Rslt_35 = term_new(VAR, 0, port(2, term_loc(Rslt_35Args)));
Rslt_35Args = pair_make(APP, 0, glblStr335, Rslt_35Args);
Rslt_35Args = pair_make(APP, 0, Rslt_30, Rslt_35Args);
#ifdef STRICT
store_redex(Rslt_35Args, new_ref(glblProto31_339));
#else
swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, Rslt_35Args, new_ref(glblProto31_339)));
#endif
dupLabels[0] = "";
// link args to body
swapStore(r_seq_28, Rslt_35);

Term Rslt_35V = get(term_loc(Rslt_35));
apps = take(port(1, term_loc(Rslt_35V)));
lams = take(port(2, term_loc(Rslt_35V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, apps, lams));
Term Rslt_38 = make_op(OP_ADD, Rslt_36, Rslt_37);
// call count at core: 479
Term Rslt_40Args = pair_make(APP, 0, Rslt_39, SUB);
Term Rslt_40 = term_new(VAR, 0, port(2, term_loc(Rslt_40Args)));
#ifdef STRICT
store_redex(Rslt_40Args, new_ref(glblProto61_341));
#else
swapStore(term_loc(Rslt_40), pair_make(LAZ, 0, Rslt_40Args, new_ref(glblProto61_341)));
#endif
Term Rslt_41 = make_op(OP_SUB, Rslt_38, Rslt_40);
// call min at core: 479
Term Rslt_42Args = pair_make(APP, 0, Rslt_41, SUB);
Term Rslt_42 = term_new(VAR, 0, port(2, term_loc(Rslt_42Args)));
Rslt_42Args = pair_make(APP, 0, new_len_23, Rslt_42Args);
#ifdef STRICT
store_redex(Rslt_42Args, glblFn269);
#else
swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, Rslt_42Args, glblFn269));
#endif

// call  at core: 479
Term Rslt_43Args = pair_make(APP, 0, Rslt_42, SUB);
Term Rslt_43 = term_new(VAR, 0, port(2, term_loc(Rslt_43Args)));
#ifdef STRICT
store_redex(Rslt_43Args, seq_28);
#else
swapStore(term_loc(Rslt_43), pair_make(LAZ, 0, Rslt_43Args, seq_28));
#endif

dupLabels[57] = "subs";
// link args to body
swapStore(r_seq_24, Rslt_43);

Term Rslt_43V = get(term_loc(Rslt_43));
apps = take(port(1, term_loc(Rslt_43V)));
lams = take(port(2, term_loc(Rslt_43V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_43), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_24);
return;
}
Term glblsubs326 = new_ref(glblsubs325);
Term glblFld329;
void glblFldFn328(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(0), args);
store_redex(args, accessField);
return;
}
Term glblFld329 = new_ref(glblFldFn328);
Term glblFld331;
void glblFldFn330(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(1), args);
store_redex(args, accessField);
return;
}
Term glblFld331 = new_ref(glblFldFn330);
Term glblFld333;
void glblFldFn332(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(2), args);
store_redex(args, accessField);
return;
}
Term glblFld333 = new_ref(glblFldFn332);
Term glblstr_vect351;
void glblstr_vect350(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_46 = pair_make(LAM, 61, SUB, NUL);
Location r_seq_46 = port(2, term_loc(seq_46));
Term s_45 = term_new(VAR, 0, port(1, term_loc(seq_46)));
// allocate args at core: 485
Term glblVal131_6;
glblVal131 = dupeArg(glblVal131, &glblVal131_6, 0);

// call vect-conj at core: 486
Term Rslt_48Args = pair_make(APP, 0, s_45, SUB);
Term Rslt_48 = term_new(VAR, 0, port(2, term_loc(Rslt_48Args)));
Rslt_48Args = pair_make(APP, 0, glblVal131_6, Rslt_48Args);
#ifdef STRICT
store_redex(Rslt_48Args, glblFn134);
#else
swapStore(term_loc(Rslt_48), pair_make(LAZ, 0, Rslt_48Args, glblFn134));
#endif

dupLabels[61] = "str-vect";
// link args to body
swapStore(r_seq_46, Rslt_48);

Term Rslt_48V = get(term_loc(Rslt_48));
apps = take(port(1, term_loc(Rslt_48V)));
lams = take(port(2, term_loc(Rslt_48V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_48), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_46);
return;
}
Term glblstr_vect351 = new_ref(glblstr_vect350);
Term glblcount349;
void glblcount348(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_51 = pair_make(LAM, 60, SUB, NUL);
Location r_seq_51 = port(2, term_loc(seq_51));
Term s_50 = term_new(VAR, 0, port(1, term_loc(seq_51)));
// allocate args at core: 488
// call str-count at core: 489
Term Rslt_53Args = pair_make(APP, 0, s_50, SUB);
Term Rslt_53 = term_new(VAR, 0, port(2, term_loc(Rslt_53Args)));
#ifdef STRICT
store_redex(Rslt_53Args, glblFn294);
#else
swapStore(term_loc(Rslt_53), pair_make(LAZ, 0, Rslt_53Args, glblFn294));
#endif

dupLabels[60] = "count";
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
Term glblcount349 = new_ref(glblcount348);
Term glbl_EQ_347;
void glbl_EQ_346(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_57 = pair_make(LAM, 59, SUB, NUL);
Location r_seq_57 = port(2, term_loc(seq_57));
Term y_56 = term_new(VAR, 0, port(1, term_loc(seq_57)));
seq_57 = pair_make(LAM, 59, SUB, seq_57);
Term x_55 = term_new(VAR, 0, port(1, term_loc(seq_57)));
// allocate args at core: 491

Term x_55_1;
x_55 = dupeArg(x_55, &x_55_1, 59);
// call str-count at core: 492
Term Rslt_59Args = pair_make(APP, 0, x_55_1, SUB);
Term Rslt_59 = term_new(VAR, 0, port(2, term_loc(Rslt_59Args)));
#ifdef STRICT
store_redex(Rslt_59Args, glblFn294);
#else
swapStore(term_loc(Rslt_59), pair_make(LAZ, 0, Rslt_59Args, glblFn294));
#endif

// call str= at core: 492
Term Rslt_60Args = pair_make(APP, 0, y_56, SUB);
Term Rslt_60 = term_new(VAR, 0, port(2, term_loc(Rslt_60Args)));
Rslt_60Args = pair_make(APP, 0, Rslt_59, Rslt_60Args);
Rslt_60Args = pair_make(APP, 0, new_i60(0), Rslt_60Args);
Rslt_60Args = pair_make(APP, 0, x_55, Rslt_60Args);
#ifdef STRICT
store_redex(Rslt_60Args, glblFn311);
#else
swapStore(term_loc(Rslt_60), pair_make(LAZ, 0, Rslt_60Args, glblFn311));
#endif

dupLabels[59] = "=";
// link args to body
swapStore(r_seq_57, Rslt_60);

Term Rslt_60V = get(term_loc(Rslt_60));
apps = take(port(1, term_loc(Rslt_60V)));
lams = take(port(2, term_loc(Rslt_60V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_60), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_57);
return;
}
Term glbl_EQ_347 = new_ref(glbl_EQ_346);
void glblProto31_356(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(496), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto31);
}
}
void glblProto61_358(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(495), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
Term glblsubs345;
void glblsubs344(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_65 = pair_make(LAM, 58, SUB, NUL);
Location r_seq_65 = port(2, term_loc(seq_65));
Term len_64 = term_new(VAR, 0, port(1, term_loc(seq_65)));
seq_65 = pair_make(LAM, 58, SUB, seq_65);
Term start_63 = term_new(VAR, 0, port(1, term_loc(seq_65)));
seq_65 = pair_make(LAM, 58, SUB, seq_65);
Term s_62 = term_new(VAR, 0, port(1, term_loc(seq_65)));
// allocate args at core: 494

Term s_62_1;
s_62 = dupeArg(s_62, &s_62_1, 58);

Term start_63_1;
start_63 = dupeArg(start_63, &start_63_1, 58);
Term seq_69 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_69 = port(2, term_loc(seq_69));
Term final_len_68 = term_new(VAR, 0, port(1, term_loc(seq_69)));
// allocate args at core: 495

Term final_len_68_1;
final_len_68 = dupeArg(final_len_68, &final_len_68_1, 0);
Term glblVal334_2;
glblVal334 = dupeArg(glblVal334, &glblVal334_2, 0);

// call <= at core: 496
Term Rslt_71Args = pair_make(APP, 0, new_i60(0), SUB);
Term Rslt_71 = term_new(VAR, 0, port(2, term_loc(Rslt_71Args)));
Rslt_71Args = pair_make(APP, 0, final_len_68, Rslt_71Args);
#ifdef STRICT
store_redex(Rslt_71Args, glblFn213);
#else
swapStore(term_loc(Rslt_71), pair_make(LAZ, 0, Rslt_71Args, glblFn213));
#endif

// call SubString at core: 498
Term Rslt_72Args = pair_make(APP, 0, final_len_68_1, SUB);
Term Rslt_72 = term_new(VAR, 0, port(2, term_loc(Rslt_72Args)));
Rslt_72Args = pair_make(APP, 0, start_63, Rslt_72Args);
Rslt_72Args = pair_make(APP, 0, s_62, Rslt_72Args);
#ifdef STRICT
store_redex(Rslt_72Args, glblVal334_2);
#else
swapStore(term_loc(Rslt_72), pair_make(LAZ, 0, Rslt_72Args, glblVal334_2));
#endif

// call cond at core: 496
Term Rslt_73Args = pair_make(APP, 0, Rslt_72, SUB);
Term Rslt_73 = term_new(VAR, 0, port(2, term_loc(Rslt_73Args)));
Rslt_73Args = pair_make(APP, 0, glblStr335, Rslt_73Args);
Rslt_73Args = pair_make(APP, 0, Rslt_71, Rslt_73Args);
#ifdef STRICT
store_redex(Rslt_73Args, new_ref(glblProto31_356));
#else
swapStore(term_loc(Rslt_73), pair_make(LAZ, 0, Rslt_73Args, new_ref(glblProto31_356)));
#endif
dupLabels[0] = "";
// link args to body
swapStore(r_seq_69, Rslt_73);

Term Rslt_73V = get(term_loc(Rslt_73));
apps = take(port(1, term_loc(Rslt_73V)));
lams = take(port(2, term_loc(Rslt_73V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_73), pair_make(LAZ, 0, apps, lams));
// call count at core: 495
Term Rslt_74Args = pair_make(APP, 0, s_62_1, SUB);
Term Rslt_74 = term_new(VAR, 0, port(2, term_loc(Rslt_74Args)));
#ifdef STRICT
store_redex(Rslt_74Args, new_ref(glblProto61_358));
#else
swapStore(term_loc(Rslt_74), pair_make(LAZ, 0, Rslt_74Args, new_ref(glblProto61_358)));
#endif
Term Rslt_75 = make_op(OP_SUB, Rslt_74, start_63_1);
// call min at core: 495
Term Rslt_76Args = pair_make(APP, 0, len_64, SUB);
Term Rslt_76 = term_new(VAR, 0, port(2, term_loc(Rslt_76Args)));
Rslt_76Args = pair_make(APP, 0, Rslt_75, Rslt_76Args);
#ifdef STRICT
store_redex(Rslt_76Args, glblFn269);
#else
swapStore(term_loc(Rslt_76), pair_make(LAZ, 0, Rslt_76Args, glblFn269));
#endif

// call  at core: 495
Term Rslt_77Args = pair_make(APP, 0, Rslt_76, SUB);
Term Rslt_77 = term_new(VAR, 0, port(2, term_loc(Rslt_77Args)));
#ifdef STRICT
store_redex(Rslt_77Args, seq_69);
#else
swapStore(term_loc(Rslt_77), pair_make(LAZ, 0, Rslt_77Args, seq_69));
#endif

dupLabels[58] = "subs";
// link args to body
swapStore(r_seq_65, Rslt_77);

Term Rslt_77V = get(term_loc(Rslt_77));
apps = take(port(1, term_loc(Rslt_77V)));
lams = take(port(2, term_loc(Rslt_77V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_77), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_65);
return;
}
Term glblsubs345 = new_ref(glblsubs344);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[7];
} str7 = {StringBufferType, REFS_STATIC, 0, 0, 6, "String"};
Term glblStr373 = term_new_(VAL, (Term)&str7);
Term glbltype_name370;
void glbltype_name369(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_80 = pair_make(LAM, 66, SUB, NUL);
Location r_seq_80 = port(2, term_loc(seq_80));
Term arg_79 = term_new(VAR, 0, port(1, term_loc(seq_80)));
// allocate args at core: 509
swapStore(term_loc(arg_79), ERA);
dupLabels[66] = "type-name";
// link args to body
swapStore(r_seq_80, glblStr373);

V = get(r_seq_80);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 66, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_80, term_new(VAR, 0, vLoc));
store_redex(args, seq_80);
return;
}
Term glbltype_name370 = new_ref(glbltype_name369);
Term glbltype_name372;
void glbltype_name371(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_84 = pair_make(LAM, 67, SUB, NUL);
Location r_seq_84 = port(2, term_loc(seq_84));
Term arg_83 = term_new(VAR, 0, port(1, term_loc(seq_84)));
// allocate args at core: 509
swapStore(term_loc(arg_83), ERA);
dupLabels[67] = "type-name";
// link args to body
swapStore(r_seq_84, glblStr373);

V = get(r_seq_84);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 67, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_84, term_new(VAR, 0, vLoc));
store_redex(args, seq_84);
return;
}
Term glbltype_name372 = new_ref(glbltype_name371);
Term glblstr_vect366;
void glblstr_vect365(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_88 = pair_make(LAM, 64, SUB, NUL);
Location r_seq_88 = port(2, term_loc(seq_88));
Term s_87 = term_new(VAR, 0, port(1, term_loc(seq_88)));
// allocate args at core: 512
Term glblVal131_7;
glblVal131 = dupeArg(glblVal131, &glblVal131_7, 0);

// call vect-conj at core: 513
Term Rslt_90Args = pair_make(APP, 0, s_87, SUB);
Term Rslt_90 = term_new(VAR, 0, port(2, term_loc(Rslt_90Args)));
Rslt_90Args = pair_make(APP, 0, glblVal131_7, Rslt_90Args);
#ifdef STRICT
store_redex(Rslt_90Args, glblFn134);
#else
swapStore(term_loc(Rslt_90), pair_make(LAZ, 0, Rslt_90Args, glblFn134));
#endif

dupLabels[64] = "str-vect";
// link args to body
swapStore(r_seq_88, Rslt_90);

Term Rslt_90V = get(term_loc(Rslt_90));
apps = take(port(1, term_loc(Rslt_90V)));
lams = take(port(2, term_loc(Rslt_90V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_90), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_88);
return;
}
Term glblstr_vect366 = new_ref(glblstr_vect365);
Term glblstr_vect368;
void glblstr_vect367(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_93 = pair_make(LAM, 65, SUB, NUL);
Location r_seq_93 = port(2, term_loc(seq_93));
Term s_92 = term_new(VAR, 0, port(1, term_loc(seq_93)));
// allocate args at core: 512
Term glblVal131_8;
glblVal131 = dupeArg(glblVal131, &glblVal131_8, 0);

// call vect-conj at core: 513
Term Rslt_95Args = pair_make(APP, 0, s_92, SUB);
Term Rslt_95 = term_new(VAR, 0, port(2, term_loc(Rslt_95Args)));
Rslt_95Args = pair_make(APP, 0, glblVal131_8, Rslt_95Args);
#ifdef STRICT
store_redex(Rslt_95Args, glblFn134);
#else
swapStore(term_loc(Rslt_95), pair_make(LAZ, 0, Rslt_95Args, glblFn134));
#endif

dupLabels[65] = "str-vect";
// link args to body
swapStore(r_seq_93, Rslt_95);

Term Rslt_95V = get(term_loc(Rslt_95));
apps = take(port(1, term_loc(Rslt_95V)));
lams = take(port(2, term_loc(Rslt_95V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_95), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_93);
return;
}
Term glblstr_vect368 = new_ref(glblstr_vect367);
Term glblrecurse362;
void glblrecurse361(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_99 = pair_make(LAM, 62, SUB, NUL);
Location r_seq_99 = port(2, term_loc(seq_99));
Term f_98 = term_new(VAR, 0, port(1, term_loc(seq_99)));
seq_99 = pair_make(LAM, 62, SUB, seq_99);
Term s_97 = term_new(VAR, 0, port(1, term_loc(seq_99)));
// allocate args at core: 515
swapStore(term_loc(f_98), ERA);
dupLabels[62] = "recurse";
// link args to body
swapStore(r_seq_99, s_97);

V = get(r_seq_99);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 62, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_99, term_new(VAR, 0, vLoc));
store_redex(args, seq_99);
return;
}
Term glblrecurse362 = new_ref(glblrecurse361);
Term glblrecurse364;
void glblrecurse363(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_104 = pair_make(LAM, 63, SUB, NUL);
Location r_seq_104 = port(2, term_loc(seq_104));
Term f_103 = term_new(VAR, 0, port(1, term_loc(seq_104)));
seq_104 = pair_make(LAM, 63, SUB, seq_104);
Term s_102 = term_new(VAR, 0, port(1, term_loc(seq_104)));
// allocate args at core: 515
swapStore(term_loc(f_103), ERA);
dupLabels[63] = "recurse";
// link args to body
swapStore(r_seq_104, s_102);

V = get(r_seq_104);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 63, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_104, term_new(VAR, 0, vLoc));
store_redex(args, seq_104);
return;
}
Term glblrecurse364 = new_ref(glblrecurse363);
Term glblFn376;
void glblCFn375(Term ref, Term args) {
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
Term glblFn376 = new_ref(glblCFn375);
void glblProto15_386(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(619), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto65_388(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(621), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto31_390(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(619), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto31);
}
}
Term glblFn385;
void glblCFn384(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_4 = pair_make(LAM, 69, SUB, NUL);
Location r_seq_4 = port(2, term_loc(seq_4));
Term x_3 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 69, SUB, seq_4);
Term n_2 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 69, SUB, seq_4);
Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_4)));
// allocate args at core: 614

Term v_1_1;
v_1 = dupeArg(v_1, &v_1_1, 69);

Term n_2_1;
n_2 = dupeArg(n_2, &n_2_1, 69);

Term x_3_2;
x_3 = dupeArg(x_3, &x_3_2, 69);

Term x_3_1;
x_3 = dupeArg(x_3, &x_3_1, 69);
// call = at core: 619
Term Rslt_6Args = pair_make(APP, 0, x_3, SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
Rslt_6Args = pair_make(APP, 0, n_2, Rslt_6Args);
#ifdef STRICT
store_redex(Rslt_6Args, new_ref(glblProto15_386));
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, new_ref(glblProto15_386)));
#endif
// call conj at core: 621
Term Rslt_7Args = pair_make(APP, 0, x_3_1, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
Rslt_7Args = pair_make(APP, 0, v_1_1, Rslt_7Args);
#ifdef STRICT
store_redex(Rslt_7Args, new_ref(glblProto65_388));
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, new_ref(glblProto65_388)));
#endif
// call inc at core: 621
Term Rslt_8Args = pair_make(APP, 0, x_3_2, SUB);
Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
store_redex(Rslt_8Args, glblFn121);
#else
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn121));
#endif

// call range* at core: 614
Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
Rslt_9Args = pair_make(APP, 0, n_2_1, Rslt_9Args);
Rslt_9Args = pair_make(APP, 0, Rslt_7, Rslt_9Args);
#ifdef STRICT
store_redex(Rslt_9Args, glblFn385);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn385));
#endif

// call cond at core: 619
Term Rslt_10Args = pair_make(APP, 0, Rslt_9, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
Rslt_10Args = pair_make(APP, 0, v_1, Rslt_10Args);
Rslt_10Args = pair_make(APP, 0, Rslt_6, Rslt_10Args);
#ifdef STRICT
store_redex(Rslt_10Args, new_ref(glblProto31_390));
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, new_ref(glblProto31_390)));
#endif
dupLabels[69] = "";
// link args to body
swapStore(r_seq_4, Rslt_10);

Term Rslt_10V = get(term_loc(Rslt_10));
apps = take(port(1, term_loc(Rslt_10V)));
lams = take(port(2, term_loc(Rslt_10V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_4);
return;
}
Term glblFn385 = new_ref(glblCFn384);
Term glblFn394;
void glblCFn393(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 70, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term n_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 623
Term glblVal131_9;
glblVal131 = dupeArg(glblVal131, &glblVal131_9, 0);

// call range* at core: 626
Term Rslt_4Args = pair_make(APP, 0, new_i60(0), SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
Rslt_4Args = pair_make(APP, 0, n_1, Rslt_4Args);
Rslt_4Args = pair_make(APP, 0, glblVal131_9, Rslt_4Args);
#ifdef STRICT
store_redex(Rslt_4Args, glblFn385);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn385));
#endif

dupLabels[70] = "";
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
Term glblFn394 = new_ref(glblCFn393);
Term glblProto396;
Term glblvect_reduce399;
void glblvect_reduce398(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_13 = pair_make(LAM, 71, SUB, NUL);
Location r_seq_13 = port(2, term_loc(seq_13));
Term f_12 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 71, SUB, seq_13);
Term result_11 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 71, SUB, seq_13);
Term n_10 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 71, SUB, seq_13);
Term vect_9 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 71, SUB, seq_13);
Term el_8 = term_new(VAR, 0, port(1, term_loc(seq_13)));
// allocate args at core: 637
swapStore(term_loc(el_8), ERA);
swapStore(term_loc(vect_9), ERA);
swapStore(term_loc(n_10), ERA);
swapStore(term_loc(f_12), ERA);
dupLabels[71] = "vect-reduce";
// link args to body
swapStore(r_seq_13, result_11);

V = get(r_seq_13);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 71, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_13, term_new(VAR, 0, vLoc));
store_redex(args, seq_13);
return;
}
Term glblvect_reduce399 = new_ref(glblvect_reduce398);
void glblProto101_405(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(641), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto101);
}
}
void glblProto17_407(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(641), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto396_409(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(641), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
Term glblvect_reduce402;
void glblvect_reduce401(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_21 = pair_make(LAM, 72, SUB, NUL);
Location r_seq_21 = port(2, term_loc(seq_21));
Term f_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 72, SUB, seq_21);
Term result_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 72, SUB, seq_21);
Term n_18 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 72, SUB, seq_21);
Term vect_17 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 72, SUB, seq_21);
Term el_16 = term_new(VAR, 0, port(1, term_loc(seq_21)));
// allocate args at core: 640

Term vect_17_1;
vect_17 = dupeArg(vect_17, &vect_17_1, 72);

Term n_18_1;
n_18 = dupeArg(n_18, &n_18_1, 72);

Term f_20_1;
f_20 = dupeArg(f_20, &f_20_1, 72);
// call get at core: 641
Term Rslt_23Args = pair_make(APP, 0, n_18, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
Rslt_23Args = pair_make(APP, 0, vect_17, Rslt_23Args);
#ifdef STRICT
store_redex(Rslt_23Args, new_ref(glblProto101_405));
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, new_ref(glblProto101_405)));
#endif
// call inc at core: 641
Term Rslt_24Args = pair_make(APP, 0, n_18_1, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
store_redex(Rslt_24Args, glblFn121);
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, glblFn121));
#endif

// call .x at core: 641
Term Rslt_25Args = pair_make(APP, 0, el_16, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
#ifdef STRICT
store_redex(Rslt_25Args, new_ref(glblProto17_407));
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, new_ref(glblProto17_407)));
#endif
// call f at core: 640
Term Rslt_26Args = pair_make(APP, 0, Rslt_25, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
Rslt_26Args = pair_make(APP, 0, result_19, Rslt_26Args);
#ifdef STRICT
store_redex(Rslt_26Args, f_20);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, f_20));
#endif

// call vect-reduce at core: 641
Term Rslt_27Args = pair_make(APP, 0, f_20_1, SUB);
Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
Rslt_27Args = pair_make(APP, 0, Rslt_26, Rslt_27Args);
Rslt_27Args = pair_make(APP, 0, Rslt_24, Rslt_27Args);
Rslt_27Args = pair_make(APP, 0, vect_17_1, Rslt_27Args);
Rslt_27Args = pair_make(APP, 0, Rslt_23, Rslt_27Args);
#ifdef STRICT
store_redex(Rslt_27Args, new_ref(glblProto396_409));
#else
swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, new_ref(glblProto396_409)));
#endif
dupLabels[72] = "vect-reduce";
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
Term glblvect_reduce402 = new_ref(glblvect_reduce401);
Term glblProto403;
Term glblvect_reverse413;
void glblvect_reverse412(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_11 = pair_make(LAM, 73, SUB, NUL);
Location r_seq_11 = port(2, term_loc(seq_11));
Term result_10 = term_new(VAR, 0, port(1, term_loc(seq_11)));
seq_11 = pair_make(LAM, 73, SUB, seq_11);
Term n_9 = term_new(VAR, 0, port(1, term_loc(seq_11)));
seq_11 = pair_make(LAM, 73, SUB, seq_11);
Term vect_8 = term_new(VAR, 0, port(1, term_loc(seq_11)));
seq_11 = pair_make(LAM, 73, SUB, seq_11);
Term el_7 = term_new(VAR, 0, port(1, term_loc(seq_11)));
// allocate args at core: 651
swapStore(term_loc(el_7), ERA);
swapStore(term_loc(vect_8), ERA);
swapStore(term_loc(n_9), ERA);
dupLabels[73] = "vect-reverse";
// link args to body
swapStore(r_seq_11, result_10);

V = get(r_seq_11);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 73, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_11, term_new(VAR, 0, vLoc));
store_redex(args, seq_11);
return;
}
Term glblvect_reverse413 = new_ref(glblvect_reverse412);
void glblProto101_419(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(655), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto101);
}
}
void glblProto25_421(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(655), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
void glblProto17_423(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(656), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto65_425(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(656), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto403_427(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(655), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto403);
}
}
Term glblvect_reverse416;
void glblvect_reverse415(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_18 = pair_make(LAM, 74, SUB, NUL);
Location r_seq_18 = port(2, term_loc(seq_18));
Term result_17 = term_new(VAR, 0, port(1, term_loc(seq_18)));
seq_18 = pair_make(LAM, 74, SUB, seq_18);
Term n_16 = term_new(VAR, 0, port(1, term_loc(seq_18)));
seq_18 = pair_make(LAM, 74, SUB, seq_18);
Term vect_15 = term_new(VAR, 0, port(1, term_loc(seq_18)));
seq_18 = pair_make(LAM, 74, SUB, seq_18);
Term el_14 = term_new(VAR, 0, port(1, term_loc(seq_18)));
// allocate args at core: 654

Term vect_15_1;
vect_15 = dupeArg(vect_15, &vect_15_1, 74);

Term n_16_2;
n_16 = dupeArg(n_16, &n_16_2, 74);

Term n_16_1;
n_16 = dupeArg(n_16, &n_16_1, 74);
// call <= at core: 655
Term Rslt_20Args = pair_make(APP, 0, n_16, SUB);
Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
Rslt_20Args = pair_make(APP, 0, new_i60(0), Rslt_20Args);
#ifdef STRICT
store_redex(Rslt_20Args, glblFn213);
#else
swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, glblFn213));
#endif

// call get at core: 655
Term Rslt_21Args = pair_make(APP, 0, n_16_1, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
Rslt_21Args = pair_make(APP, 0, vect_15, Rslt_21Args);
#ifdef STRICT
store_redex(Rslt_21Args, new_ref(glblProto101_419));
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, new_ref(glblProto101_419)));
#endif
// call and at core: 655
Term Rslt_22Args = pair_make(APP, 0, Rslt_21, SUB);
Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
Rslt_22Args = pair_make(APP, 0, Rslt_20, Rslt_22Args);
#ifdef STRICT
store_redex(Rslt_22Args, new_ref(glblProto25_421));
#else
swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, new_ref(glblProto25_421)));
#endif
// call dec at core: 656
Term Rslt_23Args = pair_make(APP, 0, n_16_2, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
#ifdef STRICT
store_redex(Rslt_23Args, glblFn127);
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblFn127));
#endif

// call .x at core: 656
Term Rslt_24Args = pair_make(APP, 0, el_14, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
store_redex(Rslt_24Args, new_ref(glblProto17_423));
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, new_ref(glblProto17_423)));
#endif
// call conj at core: 656
Term Rslt_25Args = pair_make(APP, 0, Rslt_24, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
Rslt_25Args = pair_make(APP, 0, result_17, Rslt_25Args);
#ifdef STRICT
store_redex(Rslt_25Args, new_ref(glblProto65_425));
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, new_ref(glblProto65_425)));
#endif
// call vect-reverse at core: 655
Term Rslt_26Args = pair_make(APP, 0, Rslt_25, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
Rslt_26Args = pair_make(APP, 0, Rslt_23, Rslt_26Args);
Rslt_26Args = pair_make(APP, 0, vect_15_1, Rslt_26Args);
Rslt_26Args = pair_make(APP, 0, Rslt_22, Rslt_26Args);
#ifdef STRICT
store_redex(Rslt_26Args, new_ref(glblProto403_427));
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, new_ref(glblProto403_427)));
#endif
dupLabels[74] = "vect-reverse";
// link args to body
swapStore(r_seq_18, Rslt_26);

Term Rslt_26V = get(term_loc(Rslt_26));
apps = take(port(1, term_loc(Rslt_26V)));
lams = take(port(2, term_loc(Rslt_26V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_18);
return;
}
Term glblvect_reverse416 = new_ref(glblvect_reverse415);
Term glblProto417;
Term glblvect_sub431;
void glblvect_sub430(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_13 = pair_make(LAM, 75, SUB, NUL);
Location r_seq_13 = port(2, term_loc(seq_13));
Term result_12 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 75, SUB, seq_13);
Term max_n_11 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 75, SUB, seq_13);
Term n_10 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 75, SUB, seq_13);
Term vect_9 = term_new(VAR, 0, port(1, term_loc(seq_13)));
seq_13 = pair_make(LAM, 75, SUB, seq_13);
Term el_8 = term_new(VAR, 0, port(1, term_loc(seq_13)));
// allocate args at core: 665
swapStore(term_loc(el_8), ERA);
swapStore(term_loc(vect_9), ERA);
swapStore(term_loc(n_10), ERA);
swapStore(term_loc(max_n_11), ERA);
dupLabels[75] = "vect-sub";
// link args to body
swapStore(r_seq_13, result_12);

V = get(r_seq_13);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 75, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_13, term_new(VAR, 0, vLoc));
store_redex(args, seq_13);
return;
}
Term glblvect_sub431 = new_ref(glblvect_sub430);
void glblProto23_435(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(669), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto23);
}
}
void glblProto101_437(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(669), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto101);
}
}
void glblProto25_439(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(669), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
void glblProto17_441(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(670), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto65_443(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(670), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto417_445(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(669), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto417);
}
}
Term glblvect_sub434;
void glblvect_sub433(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_21 = pair_make(LAM, 76, SUB, NUL);
Location r_seq_21 = port(2, term_loc(seq_21));
Term result_20 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 76, SUB, seq_21);
Term max_n_19 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 76, SUB, seq_21);
Term n_18 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 76, SUB, seq_21);
Term vect_17 = term_new(VAR, 0, port(1, term_loc(seq_21)));
seq_21 = pair_make(LAM, 76, SUB, seq_21);
Term el_16 = term_new(VAR, 0, port(1, term_loc(seq_21)));
// allocate args at core: 668

Term vect_17_1;
vect_17 = dupeArg(vect_17, &vect_17_1, 76);

Term n_18_2;
n_18 = dupeArg(n_18, &n_18_2, 76);

Term n_18_1;
n_18 = dupeArg(n_18, &n_18_1, 76);

Term max_n_19_1;
max_n_19 = dupeArg(max_n_19, &max_n_19_1, 76);
// call < at core: 669
Term Rslt_23Args = pair_make(APP, 0, max_n_19, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
Rslt_23Args = pair_make(APP, 0, n_18, Rslt_23Args);
#ifdef STRICT
store_redex(Rslt_23Args, new_ref(glblProto23_435));
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, new_ref(glblProto23_435)));
#endif
// call get at core: 669
Term Rslt_24Args = pair_make(APP, 0, n_18_1, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
Rslt_24Args = pair_make(APP, 0, vect_17, Rslt_24Args);
#ifdef STRICT
store_redex(Rslt_24Args, new_ref(glblProto101_437));
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, new_ref(glblProto101_437)));
#endif
// call and at core: 669
Term Rslt_25Args = pair_make(APP, 0, Rslt_24, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
Rslt_25Args = pair_make(APP, 0, Rslt_23, Rslt_25Args);
#ifdef STRICT
store_redex(Rslt_25Args, new_ref(glblProto25_439));
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, new_ref(glblProto25_439)));
#endif
// call inc at core: 670
Term Rslt_26Args = pair_make(APP, 0, n_18_2, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
#ifdef STRICT
store_redex(Rslt_26Args, glblFn121);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, glblFn121));
#endif

// call .x at core: 670
Term Rslt_27Args = pair_make(APP, 0, el_16, SUB);
Term Rslt_27 = term_new(VAR, 0, port(2, term_loc(Rslt_27Args)));
#ifdef STRICT
store_redex(Rslt_27Args, new_ref(glblProto17_441));
#else
swapStore(term_loc(Rslt_27), pair_make(LAZ, 0, Rslt_27Args, new_ref(glblProto17_441)));
#endif
// call conj at core: 670
Term Rslt_28Args = pair_make(APP, 0, Rslt_27, SUB);
Term Rslt_28 = term_new(VAR, 0, port(2, term_loc(Rslt_28Args)));
Rslt_28Args = pair_make(APP, 0, result_20, Rslt_28Args);
#ifdef STRICT
store_redex(Rslt_28Args, new_ref(glblProto65_443));
#else
swapStore(term_loc(Rslt_28), pair_make(LAZ, 0, Rslt_28Args, new_ref(glblProto65_443)));
#endif
// call vect-sub at core: 669
Term Rslt_29Args = pair_make(APP, 0, Rslt_28, SUB);
Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
Rslt_29Args = pair_make(APP, 0, max_n_19_1, Rslt_29Args);
Rslt_29Args = pair_make(APP, 0, Rslt_26, Rslt_29Args);
Rslt_29Args = pair_make(APP, 0, vect_17_1, Rslt_29Args);
Rslt_29Args = pair_make(APP, 0, Rslt_25, Rslt_29Args);
#ifdef STRICT
store_redex(Rslt_29Args, new_ref(glblProto417_445));
#else
swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, new_ref(glblProto417_445)));
#endif
dupLabels[76] = "vect-sub";
// link args to body
swapStore(r_seq_21, Rslt_29);

Term Rslt_29V = get(term_loc(Rslt_29));
apps = take(port(1, term_loc(Rslt_29V)));
lams = take(port(2, term_loc(Rslt_29V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_21);
return;
}
Term glblvect_sub434 = new_ref(glblvect_sub433);
void glblProto417_451(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(674), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto417);
}
}
Term glblFn449;
void glblCFn448(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_4 = pair_make(LAM, 77, SUB, NUL);
Location r_seq_4 = port(2, term_loc(seq_4));
Term len_3 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 77, SUB, seq_4);
Term start_2 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 77, SUB, seq_4);
Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_4)));
// allocate args at core: 672

Term v_1_1;
v_1 = dupeArg(v_1, &v_1_1, 77);

Term start_2_2;
start_2 = dupeArg(start_2, &start_2_2, 77);

Term start_2_1;
start_2 = dupeArg(start_2, &start_2_1, 77);
Term glblVal131_10;
glblVal131 = dupeArg(glblVal131, &glblVal131_10, 0);

// call vect-get at core: 674
Term Rslt_6Args = pair_make(APP, 0, start_2, SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
Rslt_6Args = pair_make(APP, 0, v_1, Rslt_6Args);
#ifdef STRICT
store_redex(Rslt_6Args, glblFn376);
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn376));
#endif

// call inc at core: 674
Term Rslt_7Args = pair_make(APP, 0, start_2_1, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
store_redex(Rslt_7Args, glblFn121);
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblFn121));
#endif

Term Rslt_8 = make_op(OP_ADD, start_2_2, len_3);
// call vect-sub at core: 674
Term Rslt_9Args = pair_make(APP, 0, glblVal131_10, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
Rslt_9Args = pair_make(APP, 0, Rslt_8, Rslt_9Args);
Rslt_9Args = pair_make(APP, 0, Rslt_7, Rslt_9Args);
Rslt_9Args = pair_make(APP, 0, v_1_1, Rslt_9Args);
Rslt_9Args = pair_make(APP, 0, Rslt_6, Rslt_9Args);
#ifdef STRICT
store_redex(Rslt_9Args, new_ref(glblProto417_451));
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, new_ref(glblProto417_451)));
#endif
dupLabels[77] = "";
// link args to body
swapStore(r_seq_4, Rslt_9);

Term Rslt_9V = get(term_loc(Rslt_9));
apps = take(port(1, term_loc(Rslt_9V)));
lams = take(port(2, term_loc(Rslt_9V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_4);
return;
}
Term glblFn449 = new_ref(glblCFn448);
Term glblVal450 = new_ref(vectMap);
Term glblVal450;
void glblProto73_458(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(683), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto73);
}
}
void glblProto396_460(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(683), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
void glblProto73_462(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(680), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto73);
}
}
void glblProto61_464(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(682), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
Term glblc457;
void glblc456(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_19 = pair_make(LAM, 79, SUB, NUL);
Location r_seq_19 = port(2, term_loc(seq_19));
Term s_18 = term_new(VAR, 0, port(1, term_loc(seq_19)));
seq_19 = pair_make(LAM, 79, SUB, seq_19);
Term len_17 = term_new(VAR, 0, port(1, term_loc(seq_19)));
// allocate args at core: 681
// call count at core: 682
Term Rslt_21Args = pair_make(APP, 0, s_18, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
#ifdef STRICT
store_redex(Rslt_21Args, new_ref(glblProto61_464));
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, new_ref(glblProto61_464)));
#endif
Term Rslt_22 = make_op(OP_ADD, len_17, Rslt_21);
dupLabels[79] = "";
// link args to body
swapStore(r_seq_19, Rslt_22);

Term Rslt_22V = get(term_loc(Rslt_22));
apps = take(port(1, term_loc(Rslt_22V)));
lams = take(port(2, term_loc(Rslt_22V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_19);
return;
}
Term glblc457 = new_ref(glblc456);
void glblProto396_467(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(680), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
void glblProto43_469(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(679), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto43);
}
}
void glblProto107_471(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(679), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto107);
}
}
Term glblFn455;
void glblCFn454(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 78, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term vect_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 678
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term ss_vect_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at core: 679

Term ss_vect_5_3;
ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_3, 0);

Term ss_vect_5_2;
ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_2, 0);

Term ss_vect_5_1;
ss_vect_5 = dupeArg(ss_vect_5, &ss_vect_5_1, 0);
Term seq_10 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_10 = port(2, term_loc(seq_10));
Term new_len_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
// allocate args at core: 680
// call first at core: 683
Term Rslt_12Args = pair_make(APP, 0, ss_vect_5, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
store_redex(Rslt_12Args, new_ref(glblProto73_458));
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, new_ref(glblProto73_458)));
#endif
// call str-malloc at core: 683
Term Rslt_13Args = pair_make(APP, 0, new_len_9, SUB);
Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
#ifdef STRICT
store_redex(Rslt_13Args, glblFn303);
#else
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn303));
#endif

// call vect-reduce at core: 683
Term Rslt_14Args = pair_make(APP, 0, glblFn306, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
Rslt_14Args = pair_make(APP, 0, Rslt_13, Rslt_14Args);
Rslt_14Args = pair_make(APP, 0, new_i60(1), Rslt_14Args);
Rslt_14Args = pair_make(APP, 0, ss_vect_5_1, Rslt_14Args);
Rslt_14Args = pair_make(APP, 0, Rslt_12, Rslt_14Args);
#ifdef STRICT
store_redex(Rslt_14Args, new_ref(glblProto396_460));
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, new_ref(glblProto396_460)));
#endif
dupLabels[0] = "";
// link args to body
swapStore(r_seq_10, Rslt_14);

Term Rslt_14V = get(term_loc(Rslt_14));
apps = take(port(1, term_loc(Rslt_14V)));
lams = take(port(2, term_loc(Rslt_14V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, apps, lams));
// call first at core: 680
Term Rslt_15Args = pair_make(APP, 0, ss_vect_5_2, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
store_redex(Rslt_15Args, new_ref(glblProto73_462));
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, new_ref(glblProto73_462)));
#endif
// call vect-reduce at core: 680
Term Rslt_23Args = pair_make(APP, 0, glblc457, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
Rslt_23Args = pair_make(APP, 0, new_i60(0), Rslt_23Args);
Rslt_23Args = pair_make(APP, 0, new_i60(1), Rslt_23Args);
Rslt_23Args = pair_make(APP, 0, ss_vect_5_3, Rslt_23Args);
Rslt_23Args = pair_make(APP, 0, Rslt_15, Rslt_23Args);
#ifdef STRICT
store_redex(Rslt_23Args, new_ref(glblProto396_467));
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, new_ref(glblProto396_467)));
#endif
// call  at core: 680
Term Rslt_24Args = pair_make(APP, 0, Rslt_23, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
store_redex(Rslt_24Args, seq_10);
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, seq_10));
#endif

dupLabels[0] = "";
// link args to body
swapStore(r_seq_6, Rslt_24);

Term Rslt_24V = get(term_loc(Rslt_24));
apps = take(port(1, term_loc(Rslt_24V)));
lams = take(port(2, term_loc(Rslt_24V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, apps, lams));
// call flat-map at core: 679
Term Rslt_25Args = pair_make(APP, 0, new_ref(glblProto107_471), SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
Rslt_25Args = pair_make(APP, 0, vect_1, Rslt_25Args);
#ifdef STRICT
store_redex(Rslt_25Args, new_ref(glblProto43_469));
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, new_ref(glblProto43_469)));
#endif
// call  at core: 679
Term Rslt_26Args = pair_make(APP, 0, Rslt_25, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
#ifdef STRICT
store_redex(Rslt_26Args, seq_6);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, seq_6));
#endif

dupLabels[78] = "";
// link args to body
swapStore(r_seq_2, Rslt_26);

Term Rslt_26V = get(term_loc(Rslt_26));
apps = take(port(1, term_loc(Rslt_26V)));
lams = take(port(2, term_loc(Rslt_26V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblFn455 = new_ref(glblCFn454);
void glblProto23_478(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(698), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto23);
}
}
void glblProto95_480(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto95);
}
}
void glblProto17_482(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto95_484(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto95);
}
}
void glblProto17_486(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto17);
}
}
void glblProto15_488(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto25_490(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(700), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
void glblProto31_492(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(698), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto31);
}
}
Term glblFn475;
void glblCFn474(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_4 = pair_make(LAM, 80, SUB, NUL);
Location r_seq_4 = port(2, term_loc(seq_4));
Term start_3 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 80, SUB, seq_4);
Term y_2 = term_new(VAR, 0, port(1, term_loc(seq_4)));
seq_4 = pair_make(LAM, 80, SUB, seq_4);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_4)));
// allocate args at core: 685

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

// call < at core: 698
Term Rslt_6Args = pair_make(APP, 0, new_i60(0), SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
Rslt_6Args = pair_make(APP, 0, start_3, Rslt_6Args);
#ifdef STRICT
store_redex(Rslt_6Args, new_ref(glblProto23_478));
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, new_ref(glblProto23_478)));
#endif
// call Some at core: 699
Term Rslt_7Args = pair_make(APP, 0, x_1, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
store_redex(Rslt_7Args, glblVal22_3);
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal22_3));
#endif

// call nth at core: 700
Term Rslt_8Args = pair_make(APP, 0, start_3_1, SUB);
Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
Rslt_8Args = pair_make(APP, 0, x_1_1, Rslt_8Args);
#ifdef STRICT
store_redex(Rslt_8Args, new_ref(glblProto95_480));
#else
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, new_ref(glblProto95_480)));
#endif
// call .x at core: 700
Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
#ifdef STRICT
store_redex(Rslt_9Args, new_ref(glblProto17_482));
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, new_ref(glblProto17_482)));
#endif
// call nth at core: 700
Term Rslt_10Args = pair_make(APP, 0, start_3_2, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
Rslt_10Args = pair_make(APP, 0, y_2, Rslt_10Args);
#ifdef STRICT
store_redex(Rslt_10Args, new_ref(glblProto95_484));
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, new_ref(glblProto95_484)));
#endif
// call .x at core: 700
Term Rslt_11Args = pair_make(APP, 0, Rslt_10, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
store_redex(Rslt_11Args, new_ref(glblProto17_486));
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, new_ref(glblProto17_486)));
#endif
// call = at core: 700
Term Rslt_12Args = pair_make(APP, 0, Rslt_11, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
Rslt_12Args = pair_make(APP, 0, Rslt_9, Rslt_12Args);
#ifdef STRICT
store_redex(Rslt_12Args, new_ref(glblProto15_488));
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, new_ref(glblProto15_488)));
#endif
// call dec at core: 701
Term Rslt_13Args = pair_make(APP, 0, start_3_3, SUB);
Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
#ifdef STRICT
store_redex(Rslt_13Args, glblFn127);
#else
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn127));
#endif

// call vect-= at core: 685
Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
Rslt_14Args = pair_make(APP, 0, y_2_1, Rslt_14Args);
Rslt_14Args = pair_make(APP, 0, x_1_2, Rslt_14Args);
#ifdef STRICT
store_redex(Rslt_14Args, glblFn475);
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn475));
#endif

// call and at core: 700
Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
Rslt_15Args = pair_make(APP, 0, Rslt_12, Rslt_15Args);
#ifdef STRICT
store_redex(Rslt_15Args, new_ref(glblProto25_490));
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, new_ref(glblProto25_490)));
#endif
// call cond at core: 698
Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
Rslt_16Args = pair_make(APP, 0, Rslt_7, Rslt_16Args);
Rslt_16Args = pair_make(APP, 0, Rslt_6, Rslt_16Args);
#ifdef STRICT
store_redex(Rslt_16Args, new_ref(glblProto31_492));
#else
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, new_ref(glblProto31_492)));
#endif
dupLabels[80] = "";
// link args to body
swapStore(r_seq_4, Rslt_16);

Term Rslt_16V = get(term_loc(Rslt_16));
apps = take(port(1, term_loc(Rslt_16V)));
lams = take(port(2, term_loc(Rslt_16V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_4);
return;
}
Term glblFn475 = new_ref(glblCFn474);
Term glblFn477;
void glblCFn476(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 3) {
  Term v_1 = arityArgs.args[0];
  Term n_2 = arityArgs.args[1];
  Term x_3 = arityArgs.args[2];

  result = vectStore((Vector *)v_1, get_i60(n_2), (Term)x_3);
  dec_and_free(n_2, 1);
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn477 = new_ref(glblCFn476);
void glblProto15_548(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(715), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto15_554(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(716), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto15);
}
}
void glblProto61_557(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(717), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
void glblProto25_561(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(716), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
void glblProto25_563(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(715), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
Term glbl_EQ_539;
void glbl_EQ_538(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_9 = pair_make(LAM, 103, SUB, NUL);
Location r_seq_9 = port(2, term_loc(seq_9));
Term y_8 = term_new(VAR, 0, port(1, term_loc(seq_9)));
seq_9 = pair_make(LAM, 103, SUB, seq_9);
Term x_7 = term_new(VAR, 0, port(1, term_loc(seq_9)));
// allocate args at core: 713

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
// call type-num at core: 715
Term Rslt_11Args = pair_make(APP, 0, x_7, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
store_redex(Rslt_11Args, glblFn8);
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, glblFn8));
#endif

// call type-num at core: 715
Term Rslt_12Args = pair_make(APP, 0, y_8, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
store_redex(Rslt_12Args, glblFn8);
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn8));
#endif

// call = at core: 715
Term Rslt_13Args = pair_make(APP, 0, Rslt_12, SUB);
Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
Rslt_13Args = pair_make(APP, 0, Rslt_11, Rslt_13Args);
#ifdef STRICT
store_redex(Rslt_13Args, new_ref(glblProto15_548));
#else
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, new_ref(glblProto15_548)));
#endif
// call vect-count at core: 716
Term Rslt_14Args = pair_make(APP, 0, x_7_1, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
#ifdef STRICT
store_redex(Rslt_14Args, glblFn272);
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn272));
#endif

// call vect-count at core: 716
Term Rslt_15Args = pair_make(APP, 0, y_8_1, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
store_redex(Rslt_15Args, glblFn272);
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblFn272));
#endif

// call = at core: 716
Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
Rslt_16Args = pair_make(APP, 0, Rslt_14, Rslt_16Args);
#ifdef STRICT
store_redex(Rslt_16Args, new_ref(glblProto15_554));
#else
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, new_ref(glblProto15_554)));
#endif
// call count at core: 717
Term Rslt_17Args = pair_make(APP, 0, x_7_3, SUB);
Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
#ifdef STRICT
store_redex(Rslt_17Args, new_ref(glblProto61_557));
#else
swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, new_ref(glblProto61_557)));
#endif
// call dec at core: 717
Term Rslt_18Args = pair_make(APP, 0, Rslt_17, SUB);
Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
#ifdef STRICT
store_redex(Rslt_18Args, glblFn127);
#else
swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblFn127));
#endif

// call vect-= at core: 717
Term Rslt_19Args = pair_make(APP, 0, Rslt_18, SUB);
Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
Rslt_19Args = pair_make(APP, 0, y_8_2, Rslt_19Args);
Rslt_19Args = pair_make(APP, 0, x_7_2, Rslt_19Args);
#ifdef STRICT
store_redex(Rslt_19Args, glblFn475);
#else
swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblFn475));
#endif

// call and at core: 716
Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
Rslt_20Args = pair_make(APP, 0, Rslt_16, Rslt_20Args);
#ifdef STRICT
store_redex(Rslt_20Args, new_ref(glblProto25_561));
#else
swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, new_ref(glblProto25_561)));
#endif
// call and at core: 715
Term Rslt_21Args = pair_make(APP, 0, Rslt_20, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
Rslt_21Args = pair_make(APP, 0, Rslt_13, Rslt_21Args);
#ifdef STRICT
store_redex(Rslt_21Args, new_ref(glblProto25_563));
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, new_ref(glblProto25_563)));
#endif
dupLabels[103] = "=";
// link args to body
swapStore(r_seq_9, Rslt_21);

Term Rslt_21V = get(term_loc(Rslt_21));
apps = take(port(1, term_loc(Rslt_21V)));
lams = take(port(2, term_loc(Rslt_21V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_9);
return;
}
Term glbl_EQ_539 = new_ref(glbl_EQ_538);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[7];
} str8 = {StringBufferType, REFS_STATIC, 0, 0, 6, "Vector"};
Term glblStr540 = term_new_(VAL, (Term)&str8);
Term glbltype_name537;
void glbltype_name536(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_24 = pair_make(LAM, 102, SUB, NUL);
Location r_seq_24 = port(2, term_loc(seq_24));
Term arg_23 = term_new(VAR, 0, port(1, term_loc(seq_24)));
// allocate args at core: 719
swapStore(term_loc(arg_23), ERA);
dupLabels[102] = "type-name";
// link args to body
swapStore(r_seq_24, glblStr540);

V = get(r_seq_24);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 102, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_24, term_new(VAR, 0, vLoc));
store_redex(args, seq_24);
return;
}
Term glbltype_name537 = new_ref(glbltype_name536);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str9 = {StringBufferType, REFS_STATIC, 0, 0, 1, "["};
Term glblStr542 = term_new_(VAL, (Term)&str9);
void glblProto57_567(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(723), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto57);
}
}
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str10 = {StringBufferType, REFS_STATIC, 0, 0, 1, "]"};
Term glblStr541 = term_new_(VAL, (Term)&str10);
Term glblstr_vect535;
void glblstr_vect534(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_28 = pair_make(LAM, 101, SUB, NUL);
Location r_seq_28 = port(2, term_loc(seq_28));
Term v_27 = term_new(VAR, 0, port(1, term_loc(seq_28)));
// allocate args at core: 722
Term glblVal131_11;
glblVal131 = dupeArg(glblVal131, &glblVal131_11, 0);

// call vect-conj at core: 723
Term Rslt_30Args = pair_make(APP, 0, glblStr542, SUB);
Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
Rslt_30Args = pair_make(APP, 0, glblVal131_11, Rslt_30Args);
#ifdef STRICT
store_redex(Rslt_30Args, glblFn134);
#else
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblFn134));
#endif

// call interpose at core: 723
Term Rslt_31Args = pair_make(APP, 0, glblStr162, SUB);
Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
Rslt_31Args = pair_make(APP, 0, v_27, Rslt_31Args);
#ifdef STRICT
store_redex(Rslt_31Args, new_ref(glblProto57_567));
#else
swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, new_ref(glblProto57_567)));
#endif
// call to-str at core: 723
Term Rslt_32Args = pair_make(APP, 0, Rslt_31, SUB);
Term Rslt_32 = term_new(VAR, 0, port(2, term_loc(Rslt_32Args)));
#ifdef STRICT
store_redex(Rslt_32Args, glblFn455);
#else
swapStore(term_loc(Rslt_32), pair_make(LAZ, 0, Rslt_32Args, glblFn455));
#endif

// call vect-conj at core: 723
Term Rslt_33Args = pair_make(APP, 0, Rslt_32, SUB);
Term Rslt_33 = term_new(VAR, 0, port(2, term_loc(Rslt_33Args)));
Rslt_33Args = pair_make(APP, 0, Rslt_30, Rslt_33Args);
#ifdef STRICT
store_redex(Rslt_33Args, glblFn134);
#else
swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, Rslt_33Args, glblFn134));
#endif

// call vect-conj at core: 723
Term Rslt_34Args = pair_make(APP, 0, glblStr541, SUB);
Term Rslt_34 = term_new(VAR, 0, port(2, term_loc(Rslt_34Args)));
Rslt_34Args = pair_make(APP, 0, Rslt_33, Rslt_34Args);
#ifdef STRICT
store_redex(Rslt_34Args, glblFn134);
#else
swapStore(term_loc(Rslt_34), pair_make(LAZ, 0, Rslt_34Args, glblFn134));
#endif

dupLabels[101] = "str-vect";
// link args to body
swapStore(r_seq_28, Rslt_34);

Term Rslt_34V = get(term_loc(Rslt_34));
apps = take(port(1, term_loc(Rslt_34V)));
lams = take(port(2, term_loc(Rslt_34V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_34), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_28);
return;
}
Term glblstr_vect535 = new_ref(glblstr_vect534);
Term glblcount533;
void glblcount532(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_37 = pair_make(LAM, 100, SUB, NUL);
Location r_seq_37 = port(2, term_loc(seq_37));
Term l_36 = term_new(VAR, 0, port(1, term_loc(seq_37)));
// allocate args at core: 725
// call vect-count at core: 726
Term Rslt_39Args = pair_make(APP, 0, l_36, SUB);
Term Rslt_39 = term_new(VAR, 0, port(2, term_loc(Rslt_39Args)));
#ifdef STRICT
store_redex(Rslt_39Args, glblFn272);
#else
swapStore(term_loc(Rslt_39), pair_make(LAZ, 0, Rslt_39Args, glblFn272));
#endif

dupLabels[100] = "count";
// link args to body
swapStore(r_seq_37, Rslt_39);

Term Rslt_39V = get(term_loc(Rslt_39));
apps = take(port(1, term_loc(Rslt_39V)));
lams = take(port(2, term_loc(Rslt_39V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_39), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_37);
return;
}
Term glblcount533 = new_ref(glblcount532);
Term glblget531;
void glblget530(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_43 = pair_make(LAM, 99, SUB, NUL);
Location r_seq_43 = port(2, term_loc(seq_43));
Term n_42 = term_new(VAR, 0, port(1, term_loc(seq_43)));
seq_43 = pair_make(LAM, 99, SUB, seq_43);
Term v_41 = term_new(VAR, 0, port(1, term_loc(seq_43)));
// allocate args at core: 728
// call vect-get at core: 729
Term Rslt_45Args = pair_make(APP, 0, n_42, SUB);
Term Rslt_45 = term_new(VAR, 0, port(2, term_loc(Rslt_45Args)));
Rslt_45Args = pair_make(APP, 0, v_41, Rslt_45Args);
#ifdef STRICT
store_redex(Rslt_45Args, glblFn376);
#else
swapStore(term_loc(Rslt_45), pair_make(LAZ, 0, Rslt_45Args, glblFn376));
#endif

dupLabels[99] = "get";
// link args to body
swapStore(r_seq_43, Rslt_45);

Term Rslt_45V = get(term_loc(Rslt_45));
apps = take(port(1, term_loc(Rslt_45V)));
lams = take(port(2, term_loc(Rslt_45V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_45), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_43);
return;
}
Term glblget531 = new_ref(glblget530);
Term glblnth529;
void glblnth528(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_49 = pair_make(LAM, 98, SUB, NUL);
Location r_seq_49 = port(2, term_loc(seq_49));
Term n_48 = term_new(VAR, 0, port(1, term_loc(seq_49)));
seq_49 = pair_make(LAM, 98, SUB, seq_49);
Term v_47 = term_new(VAR, 0, port(1, term_loc(seq_49)));
// allocate args at core: 731
// call vect-get at core: 732
Term Rslt_51Args = pair_make(APP, 0, n_48, SUB);
Term Rslt_51 = term_new(VAR, 0, port(2, term_loc(Rslt_51Args)));
Rslt_51Args = pair_make(APP, 0, v_47, Rslt_51Args);
#ifdef STRICT
store_redex(Rslt_51Args, glblFn376);
#else
swapStore(term_loc(Rslt_51), pair_make(LAZ, 0, Rslt_51Args, glblFn376));
#endif

dupLabels[98] = "nth";
// link args to body
swapStore(r_seq_49, Rslt_51);

Term Rslt_51V = get(term_loc(Rslt_51));
apps = take(port(1, term_loc(Rslt_51V)));
lams = take(port(2, term_loc(Rslt_51V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_51), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_49);
return;
}
Term glblnth529 = new_ref(glblnth528);
void glblProto61_573(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(735), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
void glblProto25_575(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(735), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto25);
}
}
Term glblempty_QM_527;
void glblempty_QM_526(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_54 = pair_make(LAM, 97, SUB, NUL);
Location r_seq_54 = port(2, term_loc(seq_54));
Term v_53 = term_new(VAR, 0, port(1, term_loc(seq_54)));
// allocate args at core: 734
Term glblVal131_12;
glblVal131 = dupeArg(glblVal131, &glblVal131_12, 0);

Term glblVal22_4;
glblVal22 = dupeArg(glblVal22, &glblVal22_4, 0);

// call count at core: 735
Term Rslt_56Args = pair_make(APP, 0, v_53, SUB);
Term Rslt_56 = term_new(VAR, 0, port(2, term_loc(Rslt_56Args)));
#ifdef STRICT
store_redex(Rslt_56Args, new_ref(glblProto61_573));
#else
swapStore(term_loc(Rslt_56), pair_make(LAZ, 0, Rslt_56Args, new_ref(glblProto61_573)));
#endif
// call int-= at core: 735
Term Rslt_57Args = pair_make(APP, 0, Rslt_56, SUB);
Term Rslt_57 = term_new(VAR, 0, port(2, term_loc(Rslt_57Args)));
Rslt_57Args = pair_make(APP, 0, new_i60(0), Rslt_57Args);
#ifdef STRICT
store_redex(Rslt_57Args, glblFn205);
#else
swapStore(term_loc(Rslt_57), pair_make(LAZ, 0, Rslt_57Args, glblFn205));
#endif

// call Some at core: 736
Term Rslt_58Args = pair_make(APP, 0, glblVal131_12, SUB);
Term Rslt_58 = term_new(VAR, 0, port(2, term_loc(Rslt_58Args)));
#ifdef STRICT
store_redex(Rslt_58Args, glblVal22_4);
#else
swapStore(term_loc(Rslt_58), pair_make(LAZ, 0, Rslt_58Args, glblVal22_4));
#endif

// call and at core: 735
Term Rslt_59Args = pair_make(APP, 0, Rslt_58, SUB);
Term Rslt_59 = term_new(VAR, 0, port(2, term_loc(Rslt_59Args)));
Rslt_59Args = pair_make(APP, 0, Rslt_57, Rslt_59Args);
#ifdef STRICT
store_redex(Rslt_59Args, new_ref(glblProto25_575));
#else
swapStore(term_loc(Rslt_59), pair_make(LAZ, 0, Rslt_59Args, new_ref(glblProto25_575)));
#endif
dupLabels[97] = "empty?";
// link args to body
swapStore(r_seq_54, Rslt_59);

Term Rslt_59V = get(term_loc(Rslt_59));
apps = take(port(1, term_loc(Rslt_59V)));
lams = take(port(2, term_loc(Rslt_59V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_59), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_54);
return;
}
Term glblempty_QM_527 = new_ref(glblempty_QM_526);
Term glblempty525;
void glblempty524(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_62 = pair_make(LAM, 96, SUB, NUL);
Location r_seq_62 = port(2, term_loc(seq_62));
Term coll_61 = term_new(VAR, 0, port(1, term_loc(seq_62)));
// allocate args at core: 738
swapStore(term_loc(coll_61), ERA);
Term glblVal131_13;
glblVal131 = dupeArg(glblVal131, &glblVal131_13, 0);

dupLabels[96] = "empty";
// link args to body
swapStore(r_seq_62, glblVal131_13);

V = get(r_seq_62);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 96, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_62, term_new(VAR, 0, vLoc));
store_redex(args, seq_62);
return;
}
Term glblempty525 = new_ref(glblempty524);
void glblProto69_581(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(742), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto69);
}
}
Term glblcomp523;
void glblcomp522(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_67 = pair_make(LAM, 95, SUB, NUL);
Location r_seq_67 = port(2, term_loc(seq_67));
Term v2_66 = term_new(VAR, 0, port(1, term_loc(seq_67)));
seq_67 = pair_make(LAM, 95, SUB, seq_67);
Term v1_65 = term_new(VAR, 0, port(1, term_loc(seq_67)));
// allocate args at core: 741
// call reduce at core: 742
Term Rslt_69Args = pair_make(APP, 0, glblFn134, SUB);
Term Rslt_69 = term_new(VAR, 0, port(2, term_loc(Rslt_69Args)));
Rslt_69Args = pair_make(APP, 0, v1_65, Rslt_69Args);
Rslt_69Args = pair_make(APP, 0, v2_66, Rslt_69Args);
#ifdef STRICT
store_redex(Rslt_69Args, new_ref(glblProto69_581));
#else
swapStore(term_loc(Rslt_69), pair_make(LAZ, 0, Rslt_69Args, new_ref(glblProto69_581)));
#endif
dupLabels[95] = "comp";
// link args to body
swapStore(r_seq_67, Rslt_69);

Term Rslt_69V = get(term_loc(Rslt_69));
apps = take(port(1, term_loc(Rslt_69V)));
lams = take(port(2, term_loc(Rslt_69V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_69), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_67);
return;
}
Term glblcomp523 = new_ref(glblcomp522);
void glblProto101_586(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(745), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto101);
}
}
Term glblfirst521;
void glblfirst520(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_72 = pair_make(LAM, 94, SUB, NUL);
Location r_seq_72 = port(2, term_loc(seq_72));
Term v_71 = term_new(VAR, 0, port(1, term_loc(seq_72)));
// allocate args at core: 744
// call get at core: 745
Term Rslt_74Args = pair_make(APP, 0, new_i60(0), SUB);
Term Rslt_74 = term_new(VAR, 0, port(2, term_loc(Rslt_74Args)));
Rslt_74Args = pair_make(APP, 0, v_71, Rslt_74Args);
#ifdef STRICT
store_redex(Rslt_74Args, new_ref(glblProto101_586));
#else
swapStore(term_loc(Rslt_74), pair_make(LAZ, 0, Rslt_74Args, new_ref(glblProto101_586)));
#endif
dupLabels[94] = "first";
// link args to body
swapStore(r_seq_72, Rslt_74);

Term Rslt_74V = get(term_loc(Rslt_74));
apps = take(port(1, term_loc(Rslt_74V)));
lams = take(port(2, term_loc(Rslt_74V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_74), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_72);
return;
}
Term glblfirst521 = new_ref(glblfirst520);
void glblProto61_593(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(748), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
void glblProto101_597(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(748), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto101);
}
}
Term glbllast519;
void glbllast518(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_77 = pair_make(LAM, 93, SUB, NUL);
Location r_seq_77 = port(2, term_loc(seq_77));
Term v_76 = term_new(VAR, 0, port(1, term_loc(seq_77)));
// allocate args at core: 747

Term v_76_1;
v_76 = dupeArg(v_76, &v_76_1, 93);
// call count at core: 748
Term Rslt_79Args = pair_make(APP, 0, v_76_1, SUB);
Term Rslt_79 = term_new(VAR, 0, port(2, term_loc(Rslt_79Args)));
#ifdef STRICT
store_redex(Rslt_79Args, new_ref(glblProto61_593));
#else
swapStore(term_loc(Rslt_79), pair_make(LAZ, 0, Rslt_79Args, new_ref(glblProto61_593)));
#endif
// call dec at core: 748
Term Rslt_80Args = pair_make(APP, 0, Rslt_79, SUB);
Term Rslt_80 = term_new(VAR, 0, port(2, term_loc(Rslt_80Args)));
#ifdef STRICT
store_redex(Rslt_80Args, glblFn127);
#else
swapStore(term_loc(Rslt_80), pair_make(LAZ, 0, Rslt_80Args, glblFn127));
#endif

// call get at core: 748
Term Rslt_81Args = pair_make(APP, 0, Rslt_80, SUB);
Term Rslt_81 = term_new(VAR, 0, port(2, term_loc(Rslt_81Args)));
Rslt_81Args = pair_make(APP, 0, v_76, Rslt_81Args);
#ifdef STRICT
store_redex(Rslt_81Args, new_ref(glblProto101_597));
#else
swapStore(term_loc(Rslt_81), pair_make(LAZ, 0, Rslt_81Args, new_ref(glblProto101_597)));
#endif
dupLabels[93] = "last";
// link args to body
swapStore(r_seq_77, Rslt_81);

Term Rslt_81V = get(term_loc(Rslt_81));
apps = take(port(1, term_loc(Rslt_81V)));
lams = take(port(2, term_loc(Rslt_81V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_81), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_77);
return;
}
Term glbllast519 = new_ref(glbllast518);
void glblProto61_605(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(751), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
Term glblrest517;
void glblrest516(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_84 = pair_make(LAM, 92, SUB, NUL);
Location r_seq_84 = port(2, term_loc(seq_84));
Term v_83 = term_new(VAR, 0, port(1, term_loc(seq_84)));
// allocate args at core: 750

Term v_83_1;
v_83 = dupeArg(v_83, &v_83_1, 92);
// call count at core: 751
Term Rslt_86Args = pair_make(APP, 0, v_83_1, SUB);
Term Rslt_86 = term_new(VAR, 0, port(2, term_loc(Rslt_86Args)));
#ifdef STRICT
store_redex(Rslt_86Args, new_ref(glblProto61_605));
#else
swapStore(term_loc(Rslt_86), pair_make(LAZ, 0, Rslt_86Args, new_ref(glblProto61_605)));
#endif
// call dec at core: 751
Term Rslt_87Args = pair_make(APP, 0, Rslt_86, SUB);
Term Rslt_87 = term_new(VAR, 0, port(2, term_loc(Rslt_87Args)));
#ifdef STRICT
store_redex(Rslt_87Args, glblFn127);
#else
swapStore(term_loc(Rslt_87), pair_make(LAZ, 0, Rslt_87Args, glblFn127));
#endif

// call subvec at core: 751
Term Rslt_88Args = pair_make(APP, 0, Rslt_87, SUB);
Term Rslt_88 = term_new(VAR, 0, port(2, term_loc(Rslt_88Args)));
Rslt_88Args = pair_make(APP, 0, new_i60(1), Rslt_88Args);
Rslt_88Args = pair_make(APP, 0, v_83, Rslt_88Args);
#ifdef STRICT
store_redex(Rslt_88Args, glblFn449);
#else
swapStore(term_loc(Rslt_88), pair_make(LAZ, 0, Rslt_88Args, glblFn449));
#endif

dupLabels[92] = "rest";
// link args to body
swapStore(r_seq_84, Rslt_88);

Term Rslt_88V = get(term_loc(Rslt_88));
apps = take(port(1, term_loc(Rslt_88V)));
lams = take(port(2, term_loc(Rslt_88V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_88), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_84);
return;
}
Term glblrest517 = new_ref(glblrest516);
Term glblconj515;
void glblconj514(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_92 = pair_make(LAM, 91, SUB, NUL);
Location r_seq_92 = port(2, term_loc(seq_92));
Term v_91 = term_new(VAR, 0, port(1, term_loc(seq_92)));
seq_92 = pair_make(LAM, 91, SUB, seq_92);
Term vect_90 = term_new(VAR, 0, port(1, term_loc(seq_92)));
// allocate args at core: 753
// call vect-conj at core: 754
Term Rslt_94Args = pair_make(APP, 0, v_91, SUB);
Term Rslt_94 = term_new(VAR, 0, port(2, term_loc(Rslt_94Args)));
Rslt_94Args = pair_make(APP, 0, vect_90, Rslt_94Args);
#ifdef STRICT
store_redex(Rslt_94Args, glblFn134);
#else
swapStore(term_loc(Rslt_94), pair_make(LAZ, 0, Rslt_94Args, glblFn134));
#endif

dupLabels[91] = "conj";
// link args to body
swapStore(r_seq_92, Rslt_94);

Term Rslt_94V = get(term_loc(Rslt_94));
apps = take(port(1, term_loc(Rslt_94V)));
lams = take(port(2, term_loc(Rslt_94V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_94), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_92);
return;
}
Term glblconj515 = new_ref(glblconj514);
Term glblstore513;
void glblstore512(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_99 = pair_make(LAM, 90, SUB, NUL);
Location r_seq_99 = port(2, term_loc(seq_99));
Term v_98 = term_new(VAR, 0, port(1, term_loc(seq_99)));
seq_99 = pair_make(LAM, 90, SUB, seq_99);
Term n_97 = term_new(VAR, 0, port(1, term_loc(seq_99)));
seq_99 = pair_make(LAM, 90, SUB, seq_99);
Term vect_96 = term_new(VAR, 0, port(1, term_loc(seq_99)));
// allocate args at core: 756
// call vect-store at core: 757
Term Rslt_101Args = pair_make(APP, 0, v_98, SUB);
Term Rslt_101 = term_new(VAR, 0, port(2, term_loc(Rslt_101Args)));
Rslt_101Args = pair_make(APP, 0, n_97, Rslt_101Args);
Rslt_101Args = pair_make(APP, 0, vect_96, Rslt_101Args);
#ifdef STRICT
store_redex(Rslt_101Args, glblFn477);
#else
swapStore(term_loc(Rslt_101), pair_make(LAZ, 0, Rslt_101Args, glblFn477));
#endif

dupLabels[90] = "store";
// link args to body
swapStore(r_seq_99, Rslt_101);

Term Rslt_101V = get(term_loc(Rslt_101));
apps = take(port(1, term_loc(Rslt_101V)));
lams = take(port(2, term_loc(Rslt_101V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_101), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_99);
return;
}
Term glblstore513 = new_ref(glblstore512);
void glblProto73_611(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(760), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto73);
}
}
void glblProto396_613(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(760), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
Term glblreduce511;
void glblreduce510(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_106 = pair_make(LAM, 89, SUB, NUL);
Location r_seq_106 = port(2, term_loc(seq_106));
Term f_105 = term_new(VAR, 0, port(1, term_loc(seq_106)));
seq_106 = pair_make(LAM, 89, SUB, seq_106);
Term result_104 = term_new(VAR, 0, port(1, term_loc(seq_106)));
seq_106 = pair_make(LAM, 89, SUB, seq_106);
Term v_103 = term_new(VAR, 0, port(1, term_loc(seq_106)));
// allocate args at core: 759

Term v_103_1;
v_103 = dupeArg(v_103, &v_103_1, 89);
// call first at core: 760
Term Rslt_108Args = pair_make(APP, 0, v_103, SUB);
Term Rslt_108 = term_new(VAR, 0, port(2, term_loc(Rslt_108Args)));
#ifdef STRICT
store_redex(Rslt_108Args, new_ref(glblProto73_611));
#else
swapStore(term_loc(Rslt_108), pair_make(LAZ, 0, Rslt_108Args, new_ref(glblProto73_611)));
#endif
// call vect-reduce at core: 760
Term Rslt_109Args = pair_make(APP, 0, f_105, SUB);
Term Rslt_109 = term_new(VAR, 0, port(2, term_loc(Rslt_109Args)));
Rslt_109Args = pair_make(APP, 0, result_104, Rslt_109Args);
Rslt_109Args = pair_make(APP, 0, new_i60(1), Rslt_109Args);
Rslt_109Args = pair_make(APP, 0, v_103_1, Rslt_109Args);
Rslt_109Args = pair_make(APP, 0, Rslt_108, Rslt_109Args);
#ifdef STRICT
store_redex(Rslt_109Args, new_ref(glblProto396_613));
#else
swapStore(term_loc(Rslt_109), pair_make(LAZ, 0, Rslt_109Args, new_ref(glblProto396_613)));
#endif
dupLabels[89] = "reduce";
// link args to body
swapStore(r_seq_106, Rslt_109);

Term Rslt_109V = get(term_loc(Rslt_109));
apps = take(port(1, term_loc(Rslt_109V)));
lams = take(port(2, term_loc(Rslt_109V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_109), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_106);
return;
}
Term glblreduce511 = new_ref(glblreduce510);
void glblProto77_616(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(763), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto77);
}
}
void glblProto61_618(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(763), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
void glblProto403_620(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(763), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto403);
}
}
Term glblreverse509;
void glblreverse508(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_112 = pair_make(LAM, 88, SUB, NUL);
Location r_seq_112 = port(2, term_loc(seq_112));
Term v_111 = term_new(VAR, 0, port(1, term_loc(seq_112)));
// allocate args at core: 762

Term v_111_2;
v_111 = dupeArg(v_111, &v_111_2, 88);

Term v_111_1;
v_111 = dupeArg(v_111, &v_111_1, 88);
Term glblVal131_14;
glblVal131 = dupeArg(glblVal131, &glblVal131_14, 0);

// call last at core: 763
Term Rslt_114Args = pair_make(APP, 0, v_111, SUB);
Term Rslt_114 = term_new(VAR, 0, port(2, term_loc(Rslt_114Args)));
#ifdef STRICT
store_redex(Rslt_114Args, new_ref(glblProto77_616));
#else
swapStore(term_loc(Rslt_114), pair_make(LAZ, 0, Rslt_114Args, new_ref(glblProto77_616)));
#endif
// call count at core: 763
Term Rslt_115Args = pair_make(APP, 0, v_111_2, SUB);
Term Rslt_115 = term_new(VAR, 0, port(2, term_loc(Rslt_115Args)));
#ifdef STRICT
store_redex(Rslt_115Args, new_ref(glblProto61_618));
#else
swapStore(term_loc(Rslt_115), pair_make(LAZ, 0, Rslt_115Args, new_ref(glblProto61_618)));
#endif
Term Rslt_116 = make_op(OP_SUB, Rslt_115, new_i60(2));
// call vect-reverse at core: 763
Term Rslt_117Args = pair_make(APP, 0, glblVal131_14, SUB);
Term Rslt_117 = term_new(VAR, 0, port(2, term_loc(Rslt_117Args)));
Rslt_117Args = pair_make(APP, 0, Rslt_116, Rslt_117Args);
Rslt_117Args = pair_make(APP, 0, v_111_1, Rslt_117Args);
Rslt_117Args = pair_make(APP, 0, Rslt_114, Rslt_117Args);
#ifdef STRICT
store_redex(Rslt_117Args, new_ref(glblProto403_620));
#else
swapStore(term_loc(Rslt_117), pair_make(LAZ, 0, Rslt_117Args, new_ref(glblProto403_620)));
#endif
dupLabels[88] = "reverse";
// link args to body
swapStore(r_seq_112, Rslt_117);

Term Rslt_117V = get(term_loc(Rslt_117));
apps = take(port(1, term_loc(Rslt_117V)));
lams = take(port(2, term_loc(Rslt_117V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_117), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_112);
return;
}
Term glblreverse509 = new_ref(glblreverse508);
Term glblmap507;
void glblmap506(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_121 = pair_make(LAM, 87, SUB, NUL);
Location r_seq_121 = port(2, term_loc(seq_121));
Term f_120 = term_new(VAR, 0, port(1, term_loc(seq_121)));
seq_121 = pair_make(LAM, 87, SUB, seq_121);
Term v_119 = term_new(VAR, 0, port(1, term_loc(seq_121)));
// allocate args at core: 765
Term glblVal450_1;
glblVal450 = dupeArg(glblVal450, &glblVal450_1, 0);

// call vect-map at core: 767
Term Rslt_123Args = pair_make(APP, 0, f_120, SUB);
Term Rslt_123 = term_new(VAR, 0, port(2, term_loc(Rslt_123Args)));
Rslt_123Args = pair_make(APP, 0, v_119, Rslt_123Args);
#ifdef STRICT
store_redex(Rslt_123Args, glblVal450_1);
#else
swapStore(term_loc(Rslt_123), pair_make(LAZ, 0, Rslt_123Args, glblVal450_1));
#endif

dupLabels[87] = "map";
// link args to body
swapStore(r_seq_121, Rslt_123);

Term Rslt_123V = get(term_loc(Rslt_123));
apps = take(port(1, term_loc(Rslt_123V)));
lams = take(port(2, term_loc(Rslt_123V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_123), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_121);
return;
}
Term glblmap507 = new_ref(glblmap506);
Term glblwrap505;
void glblwrap504(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_127 = pair_make(LAM, 86, SUB, NUL);
Location r_seq_127 = port(2, term_loc(seq_127));
Term v_126 = term_new(VAR, 0, port(1, term_loc(seq_127)));
seq_127 = pair_make(LAM, 86, SUB, seq_127);
Term arg_125 = term_new(VAR, 0, port(1, term_loc(seq_127)));
// allocate args at core: 769
swapStore(term_loc(arg_125), ERA);
Term glblVal131_15;
glblVal131 = dupeArg(glblVal131, &glblVal131_15, 0);

// call vect-conj at core: 770
Term Rslt_129Args = pair_make(APP, 0, v_126, SUB);
Term Rslt_129 = term_new(VAR, 0, port(2, term_loc(Rslt_129Args)));
Rslt_129Args = pair_make(APP, 0, glblVal131_15, Rslt_129Args);
#ifdef STRICT
store_redex(Rslt_129Args, glblFn134);
#else
swapStore(term_loc(Rslt_129), pair_make(LAZ, 0, Rslt_129Args, glblFn134));
#endif

dupLabels[86] = "wrap";
// link args to body
swapStore(r_seq_127, Rslt_129);

Term Rslt_129V = get(term_loc(Rslt_129));
apps = take(port(1, term_loc(Rslt_129V)));
lams = take(port(2, term_loc(Rslt_129V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_129), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_127);
return;
}
Term glblwrap505 = new_ref(glblwrap504);
void glblProto45_625(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(773), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto45);
}
}
Term glblflat_map503;
void glblflat_map502(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_133 = pair_make(LAM, 85, SUB, NUL);
Location r_seq_133 = port(2, term_loc(seq_133));
Term f_132 = term_new(VAR, 0, port(1, term_loc(seq_133)));
seq_133 = pair_make(LAM, 85, SUB, seq_133);
Term v_131 = term_new(VAR, 0, port(1, term_loc(seq_133)));
// allocate args at core: 772
Term glblVal450_2;
glblVal450 = dupeArg(glblVal450, &glblVal450_2, 0);

// call vect-map at core: 773
Term Rslt_135Args = pair_make(APP, 0, f_132, SUB);
Term Rslt_135 = term_new(VAR, 0, port(2, term_loc(Rslt_135Args)));
Rslt_135Args = pair_make(APP, 0, v_131, Rslt_135Args);
#ifdef STRICT
store_redex(Rslt_135Args, glblVal450_2);
#else
swapStore(term_loc(Rslt_135), pair_make(LAZ, 0, Rslt_135Args, glblVal450_2));
#endif

// call flatten at core: 773
Term Rslt_136Args = pair_make(APP, 0, Rslt_135, SUB);
Term Rslt_136 = term_new(VAR, 0, port(2, term_loc(Rslt_136Args)));
#ifdef STRICT
store_redex(Rslt_136Args, new_ref(glblProto45_625));
#else
swapStore(term_loc(Rslt_136), pair_make(LAZ, 0, Rslt_136Args, new_ref(glblProto45_625)));
#endif
dupLabels[85] = "flat-map";
// link args to body
swapStore(r_seq_133, Rslt_136);

Term Rslt_136V = get(term_loc(Rslt_136));
apps = take(port(1, term_loc(Rslt_136V)));
lams = take(port(2, term_loc(Rslt_136V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_136), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_133);
return;
}
Term glblflat_map503 = new_ref(glblflat_map502);
void glblProto73_628(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(776), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto73);
}
}
void glblProto396_630(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(776), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
void glblProto35_632(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(776), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto35);
}
}
Term glblflatten501;
void glblflatten500(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_139 = pair_make(LAM, 84, SUB, NUL);
Location r_seq_139 = port(2, term_loc(seq_139));
Term v_138 = term_new(VAR, 0, port(1, term_loc(seq_139)));
// allocate args at core: 775

Term v_138_1;
v_138 = dupeArg(v_138, &v_138_1, 84);
Term glblVal131_16;
glblVal131 = dupeArg(glblVal131, &glblVal131_16, 0);

// call first at core: 776
Term Rslt_141Args = pair_make(APP, 0, v_138, SUB);
Term Rslt_141 = term_new(VAR, 0, port(2, term_loc(Rslt_141Args)));
#ifdef STRICT
store_redex(Rslt_141Args, new_ref(glblProto73_628));
#else
swapStore(term_loc(Rslt_141), pair_make(LAZ, 0, Rslt_141Args, new_ref(glblProto73_628)));
#endif
// call vect-reduce at core: 776
Term Rslt_142Args = pair_make(APP, 0, new_ref(glblProto35_632), SUB);
Term Rslt_142 = term_new(VAR, 0, port(2, term_loc(Rslt_142Args)));
Rslt_142Args = pair_make(APP, 0, glblVal131_16, Rslt_142Args);
Rslt_142Args = pair_make(APP, 0, new_i60(1), Rslt_142Args);
Rslt_142Args = pair_make(APP, 0, v_138_1, Rslt_142Args);
Rslt_142Args = pair_make(APP, 0, Rslt_141, Rslt_142Args);
#ifdef STRICT
store_redex(Rslt_142Args, new_ref(glblProto396_630));
#else
swapStore(term_loc(Rslt_142), pair_make(LAZ, 0, Rslt_142Args, new_ref(glblProto396_630)));
#endif
dupLabels[84] = "flatten";
// link args to body
swapStore(r_seq_139, Rslt_142);

Term Rslt_142V = get(term_loc(Rslt_142));
apps = take(port(1, term_loc(Rslt_142V)));
lams = take(port(2, term_loc(Rslt_142V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_142), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_139);
return;
}
Term glblflatten501 = new_ref(glblflatten500);
Term glblProto317;
Term glblProto583;
Term glblProto578;
void glblCons595(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(3), args);
callArgs = pair_make(APP, 0, new_i60(45), callArgs);
interact(callArgs, construct);
return;
}
void glblEndOfList556(Term ref, Term args) {
Term callArgs = pair_make(APP, 0, new_i60(0), args);
callArgs = pair_make(APP, 0, new_i60(46), callArgs);
interact(callArgs, construct);
return;
}
void glblProto73_635(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(779), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto73);
}
}
void glblProto65_637(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(781), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto65);
}
}
void glblProto31_639(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(780), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto31);
}
}
void glblProto396_641(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(779), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto396);
}
}
Term glblfilter499;
void glblfilter498(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_146 = pair_make(LAM, 83, SUB, NUL);
Location r_seq_146 = port(2, term_loc(seq_146));
Term pred_145 = term_new(VAR, 0, port(1, term_loc(seq_146)));
seq_146 = pair_make(LAM, 83, SUB, seq_146);
Term v_144 = term_new(VAR, 0, port(1, term_loc(seq_146)));
// allocate args at core: 778

Term v_144_1;
v_144 = dupeArg(v_144, &v_144_1, 83);
Term seq_152 = pair_make(LAM, 106, SUB, NUL);
Location r_seq_152 = port(2, term_loc(seq_152));
Term x_151 = term_new(VAR, 0, port(1, term_loc(seq_152)));
seq_152 = pair_make(LAM, 106, SUB, seq_152);
Term result_150 = term_new(VAR, 0, port(1, term_loc(seq_152)));
// allocate args at core: 779

Term result_150_1;
result_150 = dupeArg(result_150, &result_150_1, 106);

Term x_151_1;
x_151 = dupeArg(x_151, &x_151_1, 106);
Term glblVal131_17;
glblVal131 = dupeArg(glblVal131, &glblVal131_17, 0);

// call first at core: 779
Term Rslt_148Args = pair_make(APP, 0, v_144, SUB);
Term Rslt_148 = term_new(VAR, 0, port(2, term_loc(Rslt_148Args)));
#ifdef STRICT
store_redex(Rslt_148Args, new_ref(glblProto73_635));
#else
swapStore(term_loc(Rslt_148), pair_make(LAZ, 0, Rslt_148Args, new_ref(glblProto73_635)));
#endif
// call pred at core: 778
Term Rslt_154Args = pair_make(APP, 0, x_151, SUB);
Term Rslt_154 = term_new(VAR, 0, port(2, term_loc(Rslt_154Args)));
#ifdef STRICT
store_redex(Rslt_154Args, pred_145);
#else
swapStore(term_loc(Rslt_154), pair_make(LAZ, 0, Rslt_154Args, pred_145));
#endif

// call conj at core: 781
Term Rslt_155Args = pair_make(APP, 0, x_151_1, SUB);
Term Rslt_155 = term_new(VAR, 0, port(2, term_loc(Rslt_155Args)));
Rslt_155Args = pair_make(APP, 0, result_150, Rslt_155Args);
#ifdef STRICT
store_redex(Rslt_155Args, new_ref(glblProto65_637));
#else
swapStore(term_loc(Rslt_155), pair_make(LAZ, 0, Rslt_155Args, new_ref(glblProto65_637)));
#endif
// call cond at core: 780
Term Rslt_156Args = pair_make(APP, 0, result_150_1, SUB);
Term Rslt_156 = term_new(VAR, 0, port(2, term_loc(Rslt_156Args)));
Rslt_156Args = pair_make(APP, 0, Rslt_155, Rslt_156Args);
Rslt_156Args = pair_make(APP, 0, Rslt_154, Rslt_156Args);
#ifdef STRICT
store_redex(Rslt_156Args, new_ref(glblProto31_639));
#else
swapStore(term_loc(Rslt_156), pair_make(LAZ, 0, Rslt_156Args, new_ref(glblProto31_639)));
#endif
dupLabels[106] = "";
// link args to body
swapStore(r_seq_152, Rslt_156);

Term Rslt_156V = get(term_loc(Rslt_156));
apps = take(port(1, term_loc(Rslt_156V)));
lams = take(port(2, term_loc(Rslt_156V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_156), pair_make(LAZ, 0, apps, lams));
// call vect-reduce at core: 779
Term Rslt_157Args = pair_make(APP, 0, seq_152, SUB);
Term Rslt_157 = term_new(VAR, 0, port(2, term_loc(Rslt_157Args)));
Rslt_157Args = pair_make(APP, 0, glblVal131_17, Rslt_157Args);
Rslt_157Args = pair_make(APP, 0, new_i60(1), Rslt_157Args);
Rslt_157Args = pair_make(APP, 0, v_144_1, Rslt_157Args);
Rslt_157Args = pair_make(APP, 0, Rslt_148, Rslt_157Args);
#ifdef STRICT
store_redex(Rslt_157Args, new_ref(glblProto396_641));
#else
swapStore(term_loc(Rslt_157), pair_make(LAZ, 0, Rslt_157Args, new_ref(glblProto396_641)));
#endif
dupLabels[83] = "filter";
// link args to body
swapStore(r_seq_146, Rslt_157);

Term Rslt_157V = get(term_loc(Rslt_157));
apps = take(port(1, term_loc(Rslt_157V)));
lams = take(port(2, term_loc(Rslt_157V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_157), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_146);
return;
}
Term glblfilter499 = new_ref(glblfilter498);
void glblProto43_644(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(787), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto43);
}
}
void glblProto75_646(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(788), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto75);
}
}
Term glblinterpose497;
void glblinterpose496(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_162 = pair_make(LAM, 82, SUB, NUL);
Location r_seq_162 = port(2, term_loc(seq_162));
Term sep_161 = term_new(VAR, 0, port(1, term_loc(seq_162)));
seq_162 = pair_make(LAM, 82, SUB, seq_162);
Term coll_160 = term_new(VAR, 0, port(1, term_loc(seq_162)));
// allocate args at core: 784
Term seq_166 = pair_make(LAM, 107, SUB, NUL);
Location r_seq_166 = port(2, term_loc(seq_166));
Term x_165 = term_new(VAR, 0, port(1, term_loc(seq_166)));
// allocate args at core: 787
Term glblVal131_18;
glblVal131 = dupeArg(glblVal131, &glblVal131_18, 0);

// call vect-conj at core: 787
Term Rslt_168Args = pair_make(APP, 0, sep_161, SUB);
Term Rslt_168 = term_new(VAR, 0, port(2, term_loc(Rslt_168Args)));
Rslt_168Args = pair_make(APP, 0, glblVal131_18, Rslt_168Args);
#ifdef STRICT
store_redex(Rslt_168Args, glblFn134);
#else
swapStore(term_loc(Rslt_168), pair_make(LAZ, 0, Rslt_168Args, glblFn134));
#endif

// call vect-conj at core: 787
Term Rslt_169Args = pair_make(APP, 0, x_165, SUB);
Term Rslt_169 = term_new(VAR, 0, port(2, term_loc(Rslt_169Args)));
Rslt_169Args = pair_make(APP, 0, Rslt_168, Rslt_169Args);
#ifdef STRICT
store_redex(Rslt_169Args, glblFn134);
#else
swapStore(term_loc(Rslt_169), pair_make(LAZ, 0, Rslt_169Args, glblFn134));
#endif

dupLabels[107] = "";
// link args to body
swapStore(r_seq_166, Rslt_169);

Term Rslt_169V = get(term_loc(Rslt_169));
apps = take(port(1, term_loc(Rslt_169V)));
lams = take(port(2, term_loc(Rslt_169V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_169), pair_make(LAZ, 0, apps, lams));
// call flat-map at core: 787
Term Rslt_170Args = pair_make(APP, 0, seq_166, SUB);
Term Rslt_170 = term_new(VAR, 0, port(2, term_loc(Rslt_170Args)));
Rslt_170Args = pair_make(APP, 0, coll_160, Rslt_170Args);
#ifdef STRICT
store_redex(Rslt_170Args, new_ref(glblProto43_644));
#else
swapStore(term_loc(Rslt_170), pair_make(LAZ, 0, Rslt_170Args, new_ref(glblProto43_644)));
#endif
// call rest at core: 788
Term Rslt_172Args = pair_make(APP, 0, Rslt_170, SUB);
Term Rslt_172 = term_new(VAR, 0, port(2, term_loc(Rslt_172Args)));
#ifdef STRICT
store_redex(Rslt_172Args, new_ref(glblProto75_646));
#else
swapStore(term_loc(Rslt_172), pair_make(LAZ, 0, Rslt_172Args, new_ref(glblProto75_646)));
#endif
dupLabels[82] = "interpose";
// link args to body
swapStore(r_seq_162, Rslt_172);

Term Rslt_172V = get(term_loc(Rslt_172));
apps = take(port(1, term_loc(Rslt_172V)));
lams = take(port(2, term_loc(Rslt_172V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_172), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_162);
return;
}
Term glblinterpose497 = new_ref(glblinterpose496);
Term glblVal560 = new_ref(glblEndOfList556);
Term glblVal560;
Term glblRslt558;
Term glblrecurse551;
void glblrecurse550(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 108, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 108, SUB, seq_3);
Term l_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 798
swapStore(term_loc(f_2), ERA);
dupLabels[108] = "recurse";
// link args to body
swapStore(r_seq_3, l_1);

V = get(r_seq_3);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 108, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_3, term_new(VAR, 0, vLoc));
store_redex(args, seq_3);
return;
}
Term glblrecurse551 = new_ref(glblrecurse550);
Term glblcount553;
void glblcount552(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_7 = pair_make(LAM, 109, SUB, NUL);
Location r_seq_7 = port(2, term_loc(seq_7));
Term l_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
// allocate args at core: 804
swapStore(term_loc(l_6), ERA);
dupLabels[109] = "count";
// link args to body
swapStore(r_seq_7, new_i60(0));

V = get(r_seq_7);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 109, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_7, term_new(VAR, 0, vLoc));
store_redex(args, seq_7);
return;
}
Term glblcount553 = new_ref(glblcount552);
Term glblVal607 = new_ref(glblCons595);
Term glblVal607;
Term glblrecurse589;
void glblrecurse588(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 110, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term f_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 110, SUB, seq_3);
Term arg_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 811

Term arg_1_2;
arg_1 = dupeArg(arg_1, &arg_1_2, 110);

Term arg_1_1;
arg_1 = dupeArg(arg_1, &arg_1_1, 110);
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
Term glblVal607_1;
glblVal607 = dupeArg(glblVal607, &glblVal607_1, 0);

// call f at core: 811
Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
#ifdef STRICT
store_redex(Rslt_7Args, f_2);
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, f_2));
#endif

// call Cons at core: 812
Term Rslt_9Args = pair_make(APP, 0, Rslt_8, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
Rslt_9Args = pair_make(APP, 0, Rslt_7, Rslt_9Args);
Rslt_9Args = pair_make(APP, 0, Rslt_5, Rslt_9Args);
#ifdef STRICT
store_redex(Rslt_9Args, glblVal607_1);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblVal607_1));
#endif

dupLabels[110] = "recurse";
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
Term glblrecurse589 = new_ref(glblrecurse588);
void glblProto317_652(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(818), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto317);
}
}
Term glblcount592;
void glblcount591(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_12 = pair_make(LAM, 111, SUB, NUL);
Location r_seq_12 = port(2, term_loc(seq_12));
Term l_11 = term_new(VAR, 0, port(1, term_loc(seq_12)));
// allocate args at core: 817
// call .len at core: 818
Term Rslt_14Args = pair_make(APP, 0, l_11, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
#ifdef STRICT
store_redex(Rslt_14Args, new_ref(glblProto317_652));
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, new_ref(glblProto317_652)));
#endif
dupLabels[111] = "count";
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
Term glblcount592 = new_ref(glblcount591);
Term glblFld598;
void glblFldFn596(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(0), args);
store_redex(args, accessField);
return;
}
Term glblFld598 = new_ref(glblFldFn596);
Term glblFld601;
void glblFldFn600(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(1), args);
store_redex(args, accessField);
return;
}
Term glblFld601 = new_ref(glblFldFn600);
Term glblFld604;
void glblFldFn603(Term ref, Term args) {
args = pair_make(APP, 0, new_i60(2), args);
store_redex(args, accessField);
return;
}
Term glblFld604 = new_ref(glblFldFn603);
void glblProto61_655(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(822), args);
  args = pair_make(APP, 0, glblStr151, args);
  interact(args, glblProto61);
}
}
Term glblFn545;
void glblCFn544(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 104, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term l_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 104, SUB, seq_3);
Term v_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at core: 820

Term l_2_1;
l_2 = dupeArg(l_2, &l_2_1, 104);
Term glblVal607_2;
glblVal607 = dupeArg(glblVal607, &glblVal607_2, 0);

// call count at core: 822
Term Rslt_5Args = pair_make(APP, 0, l_2_1, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
store_redex(Rslt_5Args, new_ref(glblProto61_655));
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, new_ref(glblProto61_655)));
#endif
// call inc at core: 822
Term Rslt_6Args = pair_make(APP, 0, Rslt_5, SUB);
Term Rslt_6 = term_new(VAR, 0, port(2, term_loc(Rslt_6Args)));
#ifdef STRICT
store_redex(Rslt_6Args, glblFn121);
#else
swapStore(term_loc(Rslt_6), pair_make(LAZ, 0, Rslt_6Args, glblFn121));
#endif

// call Cons at core: 822
Term Rslt_7Args = pair_make(APP, 0, Rslt_6, SUB);
Term Rslt_7 = term_new(VAR, 0, port(2, term_loc(Rslt_7Args)));
Rslt_7Args = pair_make(APP, 0, l_2, Rslt_7Args);
Rslt_7Args = pair_make(APP, 0, v_1, Rslt_7Args);
#ifdef STRICT
store_redex(Rslt_7Args, glblVal607_2);
#else
swapStore(term_loc(Rslt_7), pair_make(LAZ, 0, Rslt_7Args, glblVal607_2));
#endif

dupLabels[104] = "";
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
Term glblFn545 = new_ref(glblCFn544);
Term glblFn547;
void glblCFn546(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 105, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term vs_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at core: 824
// call to-str at core: 828
Term Rslt_4Args = pair_make(APP, 0, vs_1, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
store_redex(Rslt_4Args, glblFn455);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn455));
#endif

dupLabels[105] = "";
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
Term glblFn547 = new_ref(glblCFn546);
Value *(*type_name)(FnArity *, Value *) = noImpl1;
Value *(*zero)(FnArity *, Value *) = noImpl1;
Value *(*invoke0Args)(FnArity *, Value *) = noImpl1;
Value *(*invoke1Arg)(FnArity *, Value *,Value *) = noImpl2;
Value *(*invoke2Args)(FnArity *, Value *,Value *,Value *) = noImpl3;
Value *(*equalSTAR)(FnArity *, Value *,Value *) = noImpl2;
Value *(*hashSeq)(FnArity *, Value *,Value *) = noImpl2;
Value *(*count)(FnArity *, Value *) = noImpl1;
Value *(*vals)(FnArity *, Value *) = noImpl1;
Value *(*first)(FnArity *, Value *) = noImpl1;
Value *(*rest)(FnArity *, Value *) = noImpl1;
Value *(*seq)(FnArity *, Value *) = noImpl1;
Value *(*sha1)(FnArity *, Value *) = noImpl1;
Value *(*hasField)(FnArity *, Value *,Value *) = noImpl2;
Value *(*showFn)(FnArity *, Value *) = noImpl1;
Value *(*newHashSet)(FnArity *, Value *) = noImpl1;
Value *(*fn_apply)(FnArity *, Value *,Value *) = noImpl2;
Value *(*prValue)(FnArity *, Value *) = noImpl1;
Term glblFn660;
void glblCFn659(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 1) {
  Term str_1 = arityArgs.args[0];

    if (term_tag(str_1) != VAL) {
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
      long start = get_i60(ss->impls[1]);
      len = (int)get_i60(ss->impls[2]);
      src = &parent->buffer[start];
    } else {
      fprintf(stdout, "\ninvalid type for 'pr*': %ld\n", s->type);
      abort();
    }
    strncpy(dest, src, len);
    dest[len] = 0;
    sscanf(dest, "%d", &num);
    dec_and_free(str_1, 1);
    result = new_i60(num);
moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn660 = new_ref(glblCFn659);
Term glblFn663;
void glblCFn662(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 2) {
  Term x_1 = arityArgs.args[0];
  Term y_2 = arityArgs.args[1];

    unsigned x = (unsigned)get_i60(x_1);
    unsigned y = (unsigned)get_i60(y_2);
    result = new_i60(x & y);

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn663 = new_ref(glblCFn662);
Term glblFn665;
void glblCFn664(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 1) {
  Term x_1 = arityArgs.args[0];

    unsigned x = (unsigned)get_i60(x_1);
    result = new_i60(x >> 1);

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn665 = new_ref(glblCFn664);
Term glblFn669;
void glblCFn668(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 2, &arityArgs);
  Term result = NUL;
    if (arityArgs.count == 2) {
  Term x_1 = arityArgs.args[0];
  Term n_2 = arityArgs.args[1];

    unsigned x = (unsigned)get_i60(x_1);
    unsigned n = (unsigned)get_i60(n_2);
    result = new_i60(x >> n);

moveStore(port(2, term_loc(args)), result);
}
return;
}
Term glblFn669 = new_ref(glblCFn668);
Term glblVal670;
Term glblVal672;
Term glblFn674;
void glblCFn673(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 116, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term p_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 56
swapStore(term_loc(p_1), ERA);
// call pr* at sidequest.toc: 57
Term Rslt_4Args = pair_make(APP, 0, glblStr161, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
store_redex(Rslt_4Args, glblFn115);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn115));
#endif

dupLabels[116] = "";
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
Term glblFn674 = new_ref(glblCFn673);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str11 = {StringBufferType, REFS_STATIC, 0, 0, 1, "0"};
Term glblStr679 = term_new_(VAL, (Term)&str11);
Term glblVal670;
void glblCFn677(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 117, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term p_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 59
Term glblVal672_1;
glblVal672 = dupeArg(glblVal672, &glblVal672_1, 0);

// call pr* at sidequest.toc: 60
Term Rslt_4Args = pair_make(APP, 0, glblStr679, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
store_redex(Rslt_4Args, glblFn115);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn115));
#endif

// call p at sidequest.toc: 59
Term Rslt_5Args = pair_make(APP, 0, glblFn674, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, glblVal672_1, Rslt_5Args);
Rslt_5Args = pair_make(APP, 0, glblVal670, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, p_1);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, p_1));
#endif

dupLabels[117] = "";
// link args to body
swapStore(r_seq_2, Rslt_5);

Term Rslt_5V = get(term_loc(Rslt_5));
apps = take(port(1, term_loc(Rslt_5V)));
lams = take(port(2, term_loc(Rslt_5V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_4, apps);
lams = pair_make(LAM, 117, sideEffects, lams);

swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblVal670 = new_ref(glblCFn677);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[2];
} str12 = {StringBufferType, REFS_STATIC, 0, 0, 1, "1"};
Term glblStr683 = term_new_(VAL, (Term)&str12);
Term glblVal672;
void glblCFn681(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 118, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term p_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 63
// call pr* at sidequest.toc: 64
Term Rslt_4Args = pair_make(APP, 0, glblStr683, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
#ifdef STRICT
store_redex(Rslt_4Args, glblFn115);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, glblFn115));
#endif

// call p at sidequest.toc: 63
Term Rslt_5Args = pair_make(APP, 0, glblFn674, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, glblVal672, Rslt_5Args);
Rslt_5Args = pair_make(APP, 0, glblVal670, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, p_1);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, p_1));
#endif

dupLabels[118] = "";
// link args to body
swapStore(r_seq_2, Rslt_5);

Term Rslt_5V = get(term_loc(Rslt_5));
apps = take(port(1, term_loc(Rslt_5V)));
lams = take(port(2, term_loc(Rslt_5V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_4, apps);
lams = pair_make(LAM, 118, sideEffects, lams);

swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblVal672 = new_ref(glblCFn681);
Term glblFn686;
void glblCFn685(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 119, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term bits_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 67
// call bits at sidequest.toc: 67
Term Rslt_4Args = pair_make(APP, 0, glblFn674, SUB);
Term Rslt_4 = term_new(VAR, 0, port(2, term_loc(Rslt_4Args)));
Rslt_4Args = pair_make(APP, 0, glblVal672, Rslt_4Args);
Rslt_4Args = pair_make(APP, 0, glblVal670, Rslt_4Args);
#ifdef STRICT
store_redex(Rslt_4Args, bits_1);
#else
swapStore(term_loc(Rslt_4), pair_make(LAZ, 0, Rslt_4Args, bits_1));
#endif

// call  at sidequest.toc: 67
Term Rslt_5Args = pair_make(APP, 0, NUL, SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
#ifdef STRICT
store_redex(Rslt_5Args, Rslt_4);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, Rslt_4));
#endif

dupLabels[119] = "";
// link args to body
swapStore(r_seq_2, Rslt_5);

Term Rslt_5V = get(term_loc(Rslt_5));
apps = take(port(1, term_loc(Rslt_5V)));
lams = take(port(2, term_loc(Rslt_5V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblFn686 = new_ref(glblCFn685);
Term glblhbin_done691;
void glblhbin_done690(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_46 = pair_make(LAM, 123, SUB, NUL);
Location r_seq_46 = port(2, term_loc(seq_46));
Term n_45 = term_new(VAR, 0, port(1, term_loc(seq_46)));
// allocate args at sidequest.toc: 83
swapStore(term_loc(n_45), ERA);
Term seq_52 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_52 = port(2, term_loc(seq_52));
Term e_51 = term_new(VAR, 0, port(1, term_loc(seq_52)));
seq_52 = pair_make(LAM, 0, SUB, seq_52);
Term i_50 = term_new(VAR, 0, port(1, term_loc(seq_52)));
seq_52 = pair_make(LAM, 0, SUB, seq_52);
Term o_49 = term_new(VAR, 0, port(1, term_loc(seq_52)));
// allocate args at sidequest.toc: 84
swapStore(term_loc(o_49), ERA);
swapStore(term_loc(i_50), ERA);
dupLabels[0] = "hbin-e";
// link args to body
swapStore(r_seq_52, e_51);

V = get(r_seq_52);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 0, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_52, term_new(VAR, 0, vLoc));
dupLabels[123] = "hbin-done";
// link args to body
swapStore(r_seq_46, seq_52);

V = get(r_seq_46);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 123, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_46, term_new(VAR, 0, vLoc));
store_redex(args, seq_46);
return;
}
Term glblhbin_done691 = new_ref(glblhbin_done690);
Term glblFn689;
void glblCFn688(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 120, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term N_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 70

Term N_1_1;
N_1 = dupeArg(N_1, &N_1_1, 120);
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term l_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at sidequest.toc: 72

Term l_5_1;
l_5 = dupeArg(l_5, &l_5_1, 0);
Term seq_10 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_10 = port(2, term_loc(seq_10));
Term n_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
// allocate args at sidequest.toc: 73

Term n_9_2;
n_9 = dupeArg(n_9, &n_9_2, 0);

Term n_9_1;
n_9 = dupeArg(n_9, &n_9_1, 0);
Term seq_14 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_14 = port(2, term_loc(seq_14));
Term x_13 = term_new(VAR, 0, port(1, term_loc(seq_14)));
// allocate args at sidequest.toc: 74
Term seq_31 = pair_make(LAM, 122, SUB, NUL);
Location r_seq_31 = port(2, term_loc(seq_31));
Term e_30 = term_new(VAR, 0, port(1, term_loc(seq_31)));
seq_31 = pair_make(LAM, 122, SUB, seq_31);
Term i_29 = term_new(VAR, 0, port(1, term_loc(seq_31)));
seq_31 = pair_make(LAM, 122, SUB, seq_31);
Term o_28 = term_new(VAR, 0, port(1, term_loc(seq_31)));
// allocate args at sidequest.toc: 78
swapStore(term_loc(i_29), ERA);
swapStore(term_loc(e_30), ERA);
Term seq_20 = pair_make(LAM, 121, SUB, NUL);
Location r_seq_20 = port(2, term_loc(seq_20));
Term e_19 = term_new(VAR, 0, port(1, term_loc(seq_20)));
seq_20 = pair_make(LAM, 121, SUB, seq_20);
Term i_18 = term_new(VAR, 0, port(1, term_loc(seq_20)));
seq_20 = pair_make(LAM, 121, SUB, seq_20);
Term o_17 = term_new(VAR, 0, port(1, term_loc(seq_20)));
// allocate args at sidequest.toc: 76
swapStore(term_loc(o_17), ERA);
swapStore(term_loc(e_19), ERA);
Term glblVal214_3;
glblVal214 = dupeArg(glblVal214, &glblVal214_3, 0);

Term glblVal214_2;
glblVal214 = dupeArg(glblVal214, &glblVal214_2, 0);

Term Rslt_22 = make_op(OP_SUB, l_5, new_i60(1));
// call hbin at sidequest.toc: 70
Term Rslt_23Args = pair_make(APP, 0, Rslt_22, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
#ifdef STRICT
store_redex(Rslt_23Args, glblFn689);
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblFn689));
#endif

// call /2 at sidequest.toc: 77
Term Rslt_24Args = pair_make(APP, 0, n_9, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
#ifdef STRICT
store_redex(Rslt_24Args, glblFn665);
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, glblFn665));
#endif

// call  at sidequest.toc: 77
Term Rslt_25Args = pair_make(APP, 0, Rslt_24, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
#ifdef STRICT
store_redex(Rslt_25Args, Rslt_23);
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, Rslt_23));
#endif

// call i at sidequest.toc: 76
Term Rslt_26Args = pair_make(APP, 0, Rslt_25, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
#ifdef STRICT
store_redex(Rslt_26Args, i_18);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, i_18));
#endif

dupLabels[121] = "hbin-1";
// link args to body
swapStore(r_seq_20, Rslt_26);

Term Rslt_26V = get(term_loc(Rslt_26));
apps = take(port(1, term_loc(Rslt_26V)));
lams = take(port(2, term_loc(Rslt_26V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, apps, lams));
Term Rslt_33 = make_op(OP_SUB, l_5_1, new_i60(1));
// call hbin at sidequest.toc: 70
Term Rslt_34Args = pair_make(APP, 0, Rslt_33, SUB);
Term Rslt_34 = term_new(VAR, 0, port(2, term_loc(Rslt_34Args)));
#ifdef STRICT
store_redex(Rslt_34Args, glblFn689);
#else
swapStore(term_loc(Rslt_34), pair_make(LAZ, 0, Rslt_34Args, glblFn689));
#endif

// call /2 at sidequest.toc: 79
Term Rslt_35Args = pair_make(APP, 0, n_9_1, SUB);
Term Rslt_35 = term_new(VAR, 0, port(2, term_loc(Rslt_35Args)));
#ifdef STRICT
store_redex(Rslt_35Args, glblFn665);
#else
swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, Rslt_35Args, glblFn665));
#endif

// call  at sidequest.toc: 79
Term Rslt_36Args = pair_make(APP, 0, Rslt_35, SUB);
Term Rslt_36 = term_new(VAR, 0, port(2, term_loc(Rslt_36Args)));
#ifdef STRICT
store_redex(Rslt_36Args, Rslt_34);
#else
swapStore(term_loc(Rslt_36), pair_make(LAZ, 0, Rslt_36Args, Rslt_34));
#endif

// call o at sidequest.toc: 78
Term Rslt_37Args = pair_make(APP, 0, Rslt_36, SUB);
Term Rslt_37 = term_new(VAR, 0, port(2, term_loc(Rslt_37Args)));
#ifdef STRICT
store_redex(Rslt_37Args, o_28);
#else
swapStore(term_loc(Rslt_37), pair_make(LAZ, 0, Rslt_37Args, o_28));
#endif

dupLabels[122] = "hbin-0";
// link args to body
swapStore(r_seq_31, Rslt_37);

Term Rslt_37V = get(term_loc(Rslt_37));
apps = take(port(1, term_loc(Rslt_37V)));
lams = take(port(2, term_loc(Rslt_37V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_37), pair_make(LAZ, 0, apps, lams));
// call int-cond at sidequest.toc: 75
Term Rslt_38Args = pair_make(APP, 0, seq_31, SUB);
Term Rslt_38 = term_new(VAR, 0, port(2, term_loc(Rslt_38Args)));
Rslt_38Args = pair_make(APP, 0, seq_20, Rslt_38Args);
Rslt_38Args = pair_make(APP, 0, x_13, Rslt_38Args);
#ifdef STRICT
store_redex(Rslt_38Args, glblVal214_3);
#else
swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, Rslt_38Args, glblVal214_3));
#endif

dupLabels[0] = "hbin-cond";
// link args to body
swapStore(r_seq_14, Rslt_38);

Term Rslt_38V = get(term_loc(Rslt_38));
apps = take(port(1, term_loc(Rslt_38V)));
lams = take(port(2, term_loc(Rslt_38V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, apps, lams));
// call bit-and at sidequest.toc: 80
Term Rslt_41Args = pair_make(APP, 0, new_i60(1), SUB);
Term Rslt_41 = term_new(VAR, 0, port(2, term_loc(Rslt_41Args)));
Rslt_41Args = pair_make(APP, 0, n_9_2, Rslt_41Args);
#ifdef STRICT
store_redex(Rslt_41Args, glblFn663);
#else
swapStore(term_loc(Rslt_41), pair_make(LAZ, 0, Rslt_41Args, glblFn663));
#endif

// call hbin-cond at sidequest.toc: 80
Term Rslt_42Args = pair_make(APP, 0, Rslt_41, SUB);
Term Rslt_42 = term_new(VAR, 0, port(2, term_loc(Rslt_42Args)));
#ifdef STRICT
store_redex(Rslt_42Args, seq_14);
#else
swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, Rslt_42Args, seq_14));
#endif

dupLabels[0] = "hbin-n";
// link args to body
swapStore(r_seq_10, Rslt_42);

Term Rslt_42V = get(term_loc(Rslt_42));
apps = take(port(1, term_loc(Rslt_42V)));
lams = take(port(2, term_loc(Rslt_42V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_42), pair_make(LAZ, 0, apps, lams));
dupLabels[0] = "hbin-step";
// link args to body
swapStore(r_seq_6, seq_10);

V = get(r_seq_6);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 0, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_6, term_new(VAR, 0, vLoc));
// call hbin-step at sidequest.toc: 70
Term Rslt_43Args = pair_make(APP, 0, N_1_1, SUB);
Term Rslt_43 = term_new(VAR, 0, port(2, term_loc(Rslt_43Args)));
#ifdef STRICT
store_redex(Rslt_43Args, seq_6);
#else
swapStore(term_loc(Rslt_43), pair_make(LAZ, 0, Rslt_43Args, seq_6));
#endif

// call int-cond at sidequest.toc: 71
Term Rslt_54Args = pair_make(APP, 0, glblhbin_done691, SUB);
Term Rslt_54 = term_new(VAR, 0, port(2, term_loc(Rslt_54Args)));
Rslt_54Args = pair_make(APP, 0, Rslt_43, Rslt_54Args);
Rslt_54Args = pair_make(APP, 0, N_1, Rslt_54Args);
#ifdef STRICT
store_redex(Rslt_54Args, glblVal214_2);
#else
swapStore(term_loc(Rslt_54), pair_make(LAZ, 0, Rslt_54Args, glblVal214_2));
#endif

dupLabels[120] = "";
// link args to body
swapStore(r_seq_2, Rslt_54);

Term Rslt_54V = get(term_loc(Rslt_54));
apps = take(port(1, term_loc(Rslt_54V)));
lams = take(port(2, term_loc(Rslt_54V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_54), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblFn689 = new_ref(glblCFn688);
Term glblbin_0698;
void glblbin_0697(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_33 = pair_make(LAM, 127, SUB, NUL);
Location r_seq_33 = port(2, term_loc(seq_33));
Term e_32 = term_new(VAR, 0, port(1, term_loc(seq_33)));
seq_33 = pair_make(LAM, 127, SUB, seq_33);
Term i_31 = term_new(VAR, 0, port(1, term_loc(seq_33)));
seq_33 = pair_make(LAM, 127, SUB, seq_33);
Term o_30 = term_new(VAR, 0, port(1, term_loc(seq_33)));
// allocate args at sidequest.toc: 100
swapStore(term_loc(o_30), ERA);
swapStore(term_loc(i_31), ERA);
dupLabels[127] = "bin-0";
// link args to body
swapStore(r_seq_33, e_32);

V = get(r_seq_33);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 127, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_33, term_new(VAR, 0, vLoc));
store_redex(args, seq_33);
return;
}
Term glblbin_0698 = new_ref(glblbin_0697);
Term glblFn695;
void glblCFn694(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 124, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term v_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 124, SUB, seq_3);
Term bits_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at sidequest.toc: 87

Term bits_1_2;
bits_1 = dupeArg(bits_1, &bits_1_2, 124);

Term bits_1_1;
bits_1 = dupeArg(bits_1, &bits_1_1, 124);

Term v_2_2;
v_2 = dupeArg(v_2, &v_2_2, 124);

Term v_2_1;
v_2 = dupeArg(v_2, &v_2_1, 124);
Term seq_20 = pair_make(LAM, 126, SUB, NUL);
Location r_seq_20 = port(2, term_loc(seq_20));
Term e_19 = term_new(VAR, 0, port(1, term_loc(seq_20)));
seq_20 = pair_make(LAM, 126, SUB, seq_20);
Term i_18 = term_new(VAR, 0, port(1, term_loc(seq_20)));
seq_20 = pair_make(LAM, 126, SUB, seq_20);
Term o_17 = term_new(VAR, 0, port(1, term_loc(seq_20)));
// allocate args at sidequest.toc: 96
swapStore(term_loc(i_18), ERA);
swapStore(term_loc(e_19), ERA);
Term seq_10 = pair_make(LAM, 125, SUB, NUL);
Location r_seq_10 = port(2, term_loc(seq_10));
Term e_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
seq_10 = pair_make(LAM, 125, SUB, seq_10);
Term i_8 = term_new(VAR, 0, port(1, term_loc(seq_10)));
seq_10 = pair_make(LAM, 125, SUB, seq_10);
Term o_7 = term_new(VAR, 0, port(1, term_loc(seq_10)));
// allocate args at sidequest.toc: 91
swapStore(term_loc(o_7), ERA);
swapStore(term_loc(e_9), ERA);
Term glblVal214_5;
glblVal214 = dupeArg(glblVal214, &glblVal214_5, 0);

Term glblVal214_4;
glblVal214 = dupeArg(glblVal214, &glblVal214_4, 0);

// call bit-and at sidequest.toc: 89
Term Rslt_5Args = pair_make(APP, 0, new_i60(1), SUB);
Term Rslt_5 = term_new(VAR, 0, port(2, term_loc(Rslt_5Args)));
Rslt_5Args = pair_make(APP, 0, v_2, Rslt_5Args);
#ifdef STRICT
store_redex(Rslt_5Args, glblFn663);
#else
swapStore(term_loc(Rslt_5), pair_make(LAZ, 0, Rslt_5Args, glblFn663));
#endif

// call dec at sidequest.toc: 93
Term Rslt_12Args = pair_make(APP, 0, bits_1_1, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
store_redex(Rslt_12Args, glblFn127);
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, glblFn127));
#endif

// call /2 at sidequest.toc: 93
Term Rslt_13Args = pair_make(APP, 0, v_2_1, SUB);
Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
#ifdef STRICT
store_redex(Rslt_13Args, glblFn665);
#else
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, glblFn665));
#endif

// call bin at sidequest.toc: 87
Term Rslt_14Args = pair_make(APP, 0, Rslt_13, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
Rslt_14Args = pair_make(APP, 0, Rslt_12, Rslt_14Args);
#ifdef STRICT
store_redex(Rslt_14Args, glblFn695);
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn695));
#endif

// call i at sidequest.toc: 91
Term Rslt_15Args = pair_make(APP, 0, Rslt_14, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
store_redex(Rslt_15Args, i_8);
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, i_8));
#endif

dupLabels[125] = "bin-1-1";
// link args to body
swapStore(r_seq_10, Rslt_15);

Term Rslt_15V = get(term_loc(Rslt_15));
apps = take(port(1, term_loc(Rslt_15V)));
lams = take(port(2, term_loc(Rslt_15V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, apps, lams));
// call dec at sidequest.toc: 98
Term Rslt_22Args = pair_make(APP, 0, bits_1_2, SUB);
Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
#ifdef STRICT
store_redex(Rslt_22Args, glblFn127);
#else
swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, glblFn127));
#endif

// call /2 at sidequest.toc: 98
Term Rslt_23Args = pair_make(APP, 0, v_2_2, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
#ifdef STRICT
store_redex(Rslt_23Args, glblFn665);
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, glblFn665));
#endif

// call bin at sidequest.toc: 87
Term Rslt_24Args = pair_make(APP, 0, Rslt_23, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
Rslt_24Args = pair_make(APP, 0, Rslt_22, Rslt_24Args);
#ifdef STRICT
store_redex(Rslt_24Args, glblFn695);
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, glblFn695));
#endif

// call o at sidequest.toc: 96
Term Rslt_25Args = pair_make(APP, 0, Rslt_24, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
#ifdef STRICT
store_redex(Rslt_25Args, o_17);
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, o_17));
#endif

dupLabels[126] = "bin-1-1";
// link args to body
swapStore(r_seq_20, Rslt_25);

Term Rslt_25V = get(term_loc(Rslt_25));
apps = take(port(1, term_loc(Rslt_25V)));
lams = take(port(2, term_loc(Rslt_25V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, apps, lams));
// call int-cond at sidequest.toc: 89
Term Rslt_26Args = pair_make(APP, 0, seq_20, SUB);
Term Rslt_26 = term_new(VAR, 0, port(2, term_loc(Rslt_26Args)));
Rslt_26Args = pair_make(APP, 0, seq_10, Rslt_26Args);
Rslt_26Args = pair_make(APP, 0, Rslt_5, Rslt_26Args);
#ifdef STRICT
store_redex(Rslt_26Args, glblVal214_5);
#else
swapStore(term_loc(Rslt_26), pair_make(LAZ, 0, Rslt_26Args, glblVal214_5));
#endif

// call int-cond at sidequest.toc: 88
Term Rslt_35Args = pair_make(APP, 0, glblbin_0698, SUB);
Term Rslt_35 = term_new(VAR, 0, port(2, term_loc(Rslt_35Args)));
Rslt_35Args = pair_make(APP, 0, Rslt_26, Rslt_35Args);
Rslt_35Args = pair_make(APP, 0, bits_1, Rslt_35Args);
#ifdef STRICT
store_redex(Rslt_35Args, glblVal214_4);
#else
swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, Rslt_35Args, glblVal214_4));
#endif

dupLabels[124] = "";
// link args to body
swapStore(r_seq_3, Rslt_35);

Term Rslt_35V = get(term_loc(Rslt_35));
apps = take(port(1, term_loc(Rslt_35V)));
lams = take(port(2, term_loc(Rslt_35V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_35), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_3);
return;
}
Term glblFn695 = new_ref(glblCFn694);
Term glblVal696;
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[12];
} str13 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr1 0 bit\n"};
Term glblStr704 = term_new_(VAL, (Term)&str13);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[12];
} str14 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr1 1 bit\n"};
Term glblStr705 = term_new_(VAL, (Term)&str14);
Term glblVal696;
void glblCFn701(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 128, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 105
Term seq_8 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term e_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 0, SUB, seq_8);
Term i_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 0, SUB, seq_8);
Term o_5 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at sidequest.toc: 106

Term o_5_1;
o_5 = dupeArg(o_5, &o_5_1, 0);
swapStore(term_loc(i_6), ERA);
Term seq_19 = pair_make(LAM, 130, SUB, NUL);
Location r_seq_19 = port(2, term_loc(seq_19));
Term p_18 = term_new(VAR, 0, port(1, term_loc(seq_19)));
// allocate args at sidequest.toc: 110
Term seq_12 = pair_make(LAM, 129, SUB, NUL);
Location r_seq_12 = port(2, term_loc(seq_12));
Term p_11 = term_new(VAR, 0, port(1, term_loc(seq_12)));
// allocate args at sidequest.toc: 107
// call pr* at sidequest.toc: 108
Term Rslt_14Args = pair_make(APP, 0, glblStr704, SUB);
Term Rslt_14 = term_new(VAR, 0, port(2, term_loc(Rslt_14Args)));
#ifdef STRICT
store_redex(Rslt_14Args, glblFn115);
#else
swapStore(term_loc(Rslt_14), pair_make(LAZ, 0, Rslt_14Args, glblFn115));
#endif

// call clr1 at sidequest.toc: 105
Term Rslt_15Args = pair_make(APP, 0, p_11, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
store_redex(Rslt_15Args, glblVal696);
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, glblVal696));
#endif

// call o at sidequest.toc: 106
Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
#ifdef STRICT
store_redex(Rslt_16Args, o_5);
#else
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, o_5));
#endif

dupLabels[129] = "clr1-0";
// link args to body
swapStore(r_seq_12, Rslt_16);

Term Rslt_16V = get(term_loc(Rslt_16));
apps = take(port(1, term_loc(Rslt_16V)));
lams = take(port(2, term_loc(Rslt_16V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_14, apps);
lams = pair_make(LAM, 129, sideEffects, lams);

swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, apps, lams));
// call pr* at sidequest.toc: 111
Term Rslt_21Args = pair_make(APP, 0, glblStr705, SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
#ifdef STRICT
store_redex(Rslt_21Args, glblFn115);
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, glblFn115));
#endif

// call clr1 at sidequest.toc: 105
Term Rslt_22Args = pair_make(APP, 0, p_18, SUB);
Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
#ifdef STRICT
store_redex(Rslt_22Args, glblVal696);
#else
swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, glblVal696));
#endif

// call o at sidequest.toc: 106
Term Rslt_23Args = pair_make(APP, 0, Rslt_22, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
#ifdef STRICT
store_redex(Rslt_23Args, o_5_1);
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, o_5_1));
#endif

dupLabels[130] = "clr1-1";
// link args to body
swapStore(r_seq_19, Rslt_23);

Term Rslt_23V = get(term_loc(Rslt_23));
apps = take(port(1, term_loc(Rslt_23V)));
lams = take(port(2, term_loc(Rslt_23V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_21, apps);
lams = pair_make(LAM, 130, sideEffects, lams);

swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, apps, lams));
// call x at sidequest.toc: 105
Term Rslt_24Args = pair_make(APP, 0, e_7, SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
Rslt_24Args = pair_make(APP, 0, seq_19, Rslt_24Args);
Rslt_24Args = pair_make(APP, 0, seq_12, Rslt_24Args);
#ifdef STRICT
store_redex(Rslt_24Args, x_1);
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, x_1));
#endif

dupLabels[0] = "clr1-f";
// link args to body
swapStore(r_seq_8, Rslt_24);

Term Rslt_24V = get(term_loc(Rslt_24));
apps = take(port(1, term_loc(Rslt_24V)));
lams = take(port(2, term_loc(Rslt_24V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, apps, lams));
dupLabels[128] = "";
// link args to body
swapStore(r_seq_2, seq_8);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 128, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblVal696 = new_ref(glblCFn701);
Term glblVal703;
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[12];
} str15 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr2 0 bit\n"};
Term glblStr710 = term_new_(VAL, (Term)&str15);
Term glblclr2_0713;
void glblclr2_0712(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_12 = pair_make(LAM, 132, SUB, NUL);
Location r_seq_12 = port(2, term_loc(seq_12));
Term p_11 = term_new(VAR, 0, port(1, term_loc(seq_12)));
// allocate args at sidequest.toc: 119
Term seq_16 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_16 = port(2, term_loc(seq_16));
Term o_15 = term_new(VAR, 0, port(1, term_loc(seq_16)));
// allocate args at sidequest.toc: 120
// call pr* at sidequest.toc: 121
Term Rslt_18Args = pair_make(APP, 0, glblStr710, SUB);
Term Rslt_18 = term_new(VAR, 0, port(2, term_loc(Rslt_18Args)));
#ifdef STRICT
store_redex(Rslt_18Args, glblFn115);
#else
swapStore(term_loc(Rslt_18), pair_make(LAZ, 0, Rslt_18Args, glblFn115));
#endif

// call clr2 at sidequest.toc: 117
Term Rslt_19Args = pair_make(APP, 0, p_11, SUB);
Term Rslt_19 = term_new(VAR, 0, port(2, term_loc(Rslt_19Args)));
#ifdef STRICT
store_redex(Rslt_19Args, glblVal703);
#else
swapStore(term_loc(Rslt_19), pair_make(LAZ, 0, Rslt_19Args, glblVal703));
#endif

// call o at sidequest.toc: 120
Term Rslt_20Args = pair_make(APP, 0, Rslt_19, SUB);
Term Rslt_20 = term_new(VAR, 0, port(2, term_loc(Rslt_20Args)));
#ifdef STRICT
store_redex(Rslt_20Args, o_15);
#else
swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, Rslt_20Args, o_15));
#endif

dupLabels[0] = "clr2-0-c";
// link args to body
swapStore(r_seq_16, Rslt_20);

Term Rslt_20V = get(term_loc(Rslt_20));
apps = take(port(1, term_loc(Rslt_20V)));
lams = take(port(2, term_loc(Rslt_20V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_18, apps);
lams = pair_make(LAM, 0, sideEffects, lams);

swapStore(term_loc(Rslt_20), pair_make(LAZ, 0, apps, lams));
dupLabels[132] = "clr2-0";
// link args to body
swapStore(r_seq_12, seq_16);

V = get(r_seq_12);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 132, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_12, term_new(VAR, 0, vLoc));
store_redex(args, seq_12);
return;
}
Term glblclr2_0713 = new_ref(glblclr2_0712);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[12];
} str16 = {StringBufferType, REFS_STATIC, 0, 0, 11, "clr2 1 bit\n"};
Term glblStr711 = term_new_(VAL, (Term)&str16);
Term glblclr2_1715;
void glblclr2_1714(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_23 = pair_make(LAM, 133, SUB, NUL);
Location r_seq_23 = port(2, term_loc(seq_23));
Term p_22 = term_new(VAR, 0, port(1, term_loc(seq_23)));
// allocate args at sidequest.toc: 123
Term seq_27 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_27 = port(2, term_loc(seq_27));
Term o_26 = term_new(VAR, 0, port(1, term_loc(seq_27)));
// allocate args at sidequest.toc: 124
// call pr* at sidequest.toc: 125
Term Rslt_29Args = pair_make(APP, 0, glblStr711, SUB);
Term Rslt_29 = term_new(VAR, 0, port(2, term_loc(Rslt_29Args)));
#ifdef STRICT
store_redex(Rslt_29Args, glblFn115);
#else
swapStore(term_loc(Rslt_29), pair_make(LAZ, 0, Rslt_29Args, glblFn115));
#endif

// call clr2 at sidequest.toc: 117
Term Rslt_30Args = pair_make(APP, 0, p_22, SUB);
Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
#ifdef STRICT
store_redex(Rslt_30Args, glblVal703);
#else
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, glblVal703));
#endif

// call o at sidequest.toc: 124
Term Rslt_31Args = pair_make(APP, 0, Rslt_30, SUB);
Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
#ifdef STRICT
store_redex(Rslt_31Args, o_26);
#else
swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, o_26));
#endif

dupLabels[0] = "clr2-1-c";
// link args to body
swapStore(r_seq_27, Rslt_31);

Term Rslt_31V = get(term_loc(Rslt_31));
apps = take(port(1, term_loc(Rslt_31V)));
lams = take(port(2, term_loc(Rslt_31V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_29, apps);
lams = pair_make(LAM, 0, sideEffects, lams);

swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, apps, lams));
dupLabels[133] = "clr2-1";
// link args to body
swapStore(r_seq_23, seq_27);

V = get(r_seq_23);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 133, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_23, term_new(VAR, 0, vLoc));
store_redex(args, seq_23);
return;
}
Term glblclr2_1715 = new_ref(glblclr2_1714);
Term glblVal703;
void glblCFn707(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 131, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 117
Term seq_8 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term e_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 0, SUB, seq_8);
Term i_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 0, SUB, seq_8);
Term o_5 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at sidequest.toc: 118
swapStore(term_loc(i_6), ERA);
Term seq_34 = pair_make(LAM, 134, SUB, NUL);
Location r_seq_34 = port(2, term_loc(seq_34));
Term o_33 = term_new(VAR, 0, port(1, term_loc(seq_34)));
// allocate args at sidequest.toc: 127
swapStore(term_loc(o_33), ERA);
dupLabels[134] = "clr-2-done";
// link args to body
swapStore(r_seq_34, e_7);

V = get(r_seq_34);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 134, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_34, term_new(VAR, 0, vLoc));
// call x at sidequest.toc: 117
Term Rslt_36Args = pair_make(APP, 0, seq_34, SUB);
Term Rslt_36 = term_new(VAR, 0, port(2, term_loc(Rslt_36Args)));
Rslt_36Args = pair_make(APP, 0, glblclr2_1715, Rslt_36Args);
Rslt_36Args = pair_make(APP, 0, glblclr2_0713, Rslt_36Args);
#ifdef STRICT
store_redex(Rslt_36Args, x_1);
#else
swapStore(term_loc(Rslt_36), pair_make(LAZ, 0, Rslt_36Args, x_1));
#endif

// call  at sidequest.toc: 118
Term Rslt_38Args = pair_make(APP, 0, o_5, SUB);
Term Rslt_38 = term_new(VAR, 0, port(2, term_loc(Rslt_38Args)));
#ifdef STRICT
store_redex(Rslt_38Args, Rslt_36);
#else
swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, Rslt_38Args, Rslt_36));
#endif

dupLabels[0] = "clr2-f";
// link args to body
swapStore(r_seq_8, Rslt_38);

Term Rslt_38V = get(term_loc(Rslt_38));
apps = take(port(1, term_loc(Rslt_38V)));
lams = take(port(2, term_loc(Rslt_38V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_38), pair_make(LAZ, 0, apps, lams));
dupLabels[131] = "";
// link args to body
swapStore(r_seq_2, seq_8);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 131, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblVal703 = new_ref(glblCFn707);
Term glblVal709;
Term glblrep_O724;
void glblrep_O723(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_8 = pair_make(LAM, 139, SUB, NUL);
Location r_seq_8 = port(2, term_loc(seq_8));
Term x_7 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 139, SUB, seq_8);
Term f_6 = term_new(VAR, 0, port(1, term_loc(seq_8)));
seq_8 = pair_make(LAM, 139, SUB, seq_8);
Term p_5 = term_new(VAR, 0, port(1, term_loc(seq_8)));
// allocate args at sidequest.toc: 155

Term f_6_1;
f_6 = dupeArg(f_6, &f_6_1, 139);
Term seq_13 = pair_make(LAM, 136, SUB, NUL);
Location r_seq_13 = port(2, term_loc(seq_13));
Term k_12 = term_new(VAR, 0, port(1, term_loc(seq_13)));
// allocate args at sidequest.toc: 158
// call rep at sidequest.toc: 154
Term Rslt_10Args = pair_make(APP, 0, p_5, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
store_redex(Rslt_10Args, glblVal709);
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblVal709));
#endif

// call f at sidequest.toc: 155
Term Rslt_15Args = pair_make(APP, 0, k_12, SUB);
Term Rslt_15 = term_new(VAR, 0, port(2, term_loc(Rslt_15Args)));
#ifdef STRICT
store_redex(Rslt_15Args, f_6_1);
#else
swapStore(term_loc(Rslt_15), pair_make(LAZ, 0, Rslt_15Args, f_6_1));
#endif

// call f at sidequest.toc: 155
Term Rslt_16Args = pair_make(APP, 0, Rslt_15, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
#ifdef STRICT
store_redex(Rslt_16Args, f_6);
#else
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, f_6));
#endif

dupLabels[136] = "rep-0-c";
// link args to body
swapStore(r_seq_13, Rslt_16);

Term Rslt_16V = get(term_loc(Rslt_16));
apps = take(port(1, term_loc(Rslt_16V)));
lams = take(port(2, term_loc(Rslt_16V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, apps, lams));
// call  at sidequest.toc: 158
Term Rslt_17Args = pair_make(APP, 0, x_7, SUB);
Term Rslt_17 = term_new(VAR, 0, port(2, term_loc(Rslt_17Args)));
Rslt_17Args = pair_make(APP, 0, seq_13, Rslt_17Args);
#ifdef STRICT
store_redex(Rslt_17Args, Rslt_10);
#else
swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, Rslt_17Args, Rslt_10));
#endif

dupLabels[139] = "rep-O";
// link args to body
swapStore(r_seq_8, Rslt_17);

Term Rslt_17V = get(term_loc(Rslt_17));
apps = take(port(1, term_loc(Rslt_17V)));
lams = take(port(2, term_loc(Rslt_17V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_17), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_8);
return;
}
Term glblrep_O724 = new_ref(glblrep_O723);
Term glblrep_I726;
void glblrep_I725(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_23 = pair_make(LAM, 140, SUB, NUL);
Location r_seq_23 = port(2, term_loc(seq_23));
Term x_22 = term_new(VAR, 0, port(1, term_loc(seq_23)));
seq_23 = pair_make(LAM, 140, SUB, seq_23);
Term f_21 = term_new(VAR, 0, port(1, term_loc(seq_23)));
seq_23 = pair_make(LAM, 140, SUB, seq_23);
Term p_20 = term_new(VAR, 0, port(1, term_loc(seq_23)));
// allocate args at sidequest.toc: 162

Term f_21_2;
f_21 = dupeArg(f_21, &f_21_2, 140);

Term f_21_1;
f_21 = dupeArg(f_21, &f_21_1, 140);
Term seq_28 = pair_make(LAM, 138, SUB, NUL);
Location r_seq_28 = port(2, term_loc(seq_28));
Term k_27 = term_new(VAR, 0, port(1, term_loc(seq_28)));
// allocate args at sidequest.toc: 165
// call rep at sidequest.toc: 154
Term Rslt_25Args = pair_make(APP, 0, p_20, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
#ifdef STRICT
store_redex(Rslt_25Args, glblVal709);
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, glblVal709));
#endif

// call f at sidequest.toc: 162
Term Rslt_30Args = pair_make(APP, 0, k_27, SUB);
Term Rslt_30 = term_new(VAR, 0, port(2, term_loc(Rslt_30Args)));
#ifdef STRICT
store_redex(Rslt_30Args, f_21_1);
#else
swapStore(term_loc(Rslt_30), pair_make(LAZ, 0, Rslt_30Args, f_21_1));
#endif

// call f at sidequest.toc: 162
Term Rslt_31Args = pair_make(APP, 0, Rslt_30, SUB);
Term Rslt_31 = term_new(VAR, 0, port(2, term_loc(Rslt_31Args)));
#ifdef STRICT
store_redex(Rslt_31Args, f_21);
#else
swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, Rslt_31Args, f_21));
#endif

dupLabels[138] = "rep-1-c";
// link args to body
swapStore(r_seq_28, Rslt_31);

Term Rslt_31V = get(term_loc(Rslt_31));
apps = take(port(1, term_loc(Rslt_31V)));
lams = take(port(2, term_loc(Rslt_31V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_31), pair_make(LAZ, 0, apps, lams));
// call f at sidequest.toc: 162
Term Rslt_32Args = pair_make(APP, 0, x_22, SUB);
Term Rslt_32 = term_new(VAR, 0, port(2, term_loc(Rslt_32Args)));
#ifdef STRICT
store_redex(Rslt_32Args, f_21_2);
#else
swapStore(term_loc(Rslt_32), pair_make(LAZ, 0, Rslt_32Args, f_21_2));
#endif

// call  at sidequest.toc: 165
Term Rslt_33Args = pair_make(APP, 0, Rslt_32, SUB);
Term Rslt_33 = term_new(VAR, 0, port(2, term_loc(Rslt_33Args)));
Rslt_33Args = pair_make(APP, 0, seq_28, Rslt_33Args);
#ifdef STRICT
store_redex(Rslt_33Args, Rslt_25);
#else
swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, Rslt_33Args, Rslt_25));
#endif

dupLabels[140] = "rep-I";
// link args to body
swapStore(r_seq_23, Rslt_33);

Term Rslt_33V = get(term_loc(Rslt_33));
apps = take(port(1, term_loc(Rslt_33V)));
lams = take(port(2, term_loc(Rslt_33V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_33), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_23);
return;
}
Term glblrep_I726 = new_ref(glblrep_I725);
Term glblrep_E728;
void glblrep_E727(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_38 = pair_make(LAM, 141, SUB, NUL);
Location r_seq_38 = port(2, term_loc(seq_38));
Term x_37 = term_new(VAR, 0, port(1, term_loc(seq_38)));
seq_38 = pair_make(LAM, 141, SUB, seq_38);
Term f_36 = term_new(VAR, 0, port(1, term_loc(seq_38)));
// allocate args at sidequest.toc: 169
swapStore(term_loc(f_36), ERA);
dupLabels[141] = "rep-E";
// link args to body
swapStore(r_seq_38, x_37);

V = get(r_seq_38);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 141, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_38, term_new(VAR, 0, vLoc));
store_redex(args, seq_38);
return;
}
Term glblrep_E728 = new_ref(glblrep_E727);
Term glblVal709;
void glblCFn719(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 135, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term xs_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 154
// call xs at sidequest.toc: 154
Term Rslt_40Args = pair_make(APP, 0, glblrep_E728, SUB);
Term Rslt_40 = term_new(VAR, 0, port(2, term_loc(Rslt_40Args)));
Rslt_40Args = pair_make(APP, 0, glblrep_I726, Rslt_40Args);
Rslt_40Args = pair_make(APP, 0, glblrep_O724, Rslt_40Args);
#ifdef STRICT
store_redex(Rslt_40Args, xs_1);
#else
swapStore(term_loc(Rslt_40), pair_make(LAZ, 0, Rslt_40Args, xs_1));
#endif

dupLabels[135] = "";
// link args to body
swapStore(r_seq_2, Rslt_40);

Term Rslt_40V = get(term_loc(Rslt_40));
apps = take(port(1, term_loc(Rslt_40V)));
lams = take(port(2, term_loc(Rslt_40V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_40), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblVal709 = new_ref(glblCFn719);
Term glblFn722;
void glblCFn721(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 137, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term b_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 172
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term t_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at sidequest.toc: 173
swapStore(term_loc(t_5), ERA);
Term seq_10 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_10 = port(2, term_loc(seq_10));
Term f_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
// allocate args at sidequest.toc: 174

Term f_9_1;
f_9 = dupeArg(f_9, &f_9_1, 0);
// call b at sidequest.toc: 172
Term Rslt_12Args = pair_make(APP, 0, f_9_1, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
Rslt_12Args = pair_make(APP, 0, f_9, Rslt_12Args);
#ifdef STRICT
store_redex(Rslt_12Args, b_1);
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, b_1));
#endif

dupLabels[0] = "";
// link args to body
swapStore(r_seq_10, Rslt_12);

Term Rslt_12V = get(term_loc(Rslt_12));
apps = take(port(1, term_loc(Rslt_12V)));
lams = take(port(2, term_loc(Rslt_12V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, apps, lams));
dupLabels[0] = "";
// link args to body
swapStore(r_seq_6, seq_10);

V = get(r_seq_6);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 0, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_6, term_new(VAR, 0, vLoc));
dupLabels[137] = "";
// link args to body
swapStore(r_seq_2, seq_6);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 137, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblFn722 = new_ref(glblCFn721);
Term glblFn734;
void glblCFn733(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 142, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term b_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 177
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term t_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at sidequest.toc: 178
swapStore(term_loc(t_5), ERA);
Term seq_10 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_10 = port(2, term_loc(seq_10));
Term f_9 = term_new(VAR, 0, port(1, term_loc(seq_10)));
// allocate args at sidequest.toc: 179

Term f_9_2;
f_9 = dupeArg(f_9, &f_9_2, 0);

Term f_9_1;
f_9 = dupeArg(f_9, &f_9_1, 0);
// call f at sidequest.toc: 179
Term Rslt_12Args = pair_make(APP, 0, f_9_1, SUB);
Term Rslt_12 = term_new(VAR, 0, port(2, term_loc(Rslt_12Args)));
#ifdef STRICT
store_redex(Rslt_12Args, f_9);
#else
swapStore(term_loc(Rslt_12), pair_make(LAZ, 0, Rslt_12Args, f_9));
#endif

// call b at sidequest.toc: 177
Term Rslt_13Args = pair_make(APP, 0, f_9_2, SUB);
Term Rslt_13 = term_new(VAR, 0, port(2, term_loc(Rslt_13Args)));
Rslt_13Args = pair_make(APP, 0, Rslt_12, Rslt_13Args);
#ifdef STRICT
store_redex(Rslt_13Args, b_1);
#else
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, Rslt_13Args, b_1));
#endif

dupLabels[0] = "";
// link args to body
swapStore(r_seq_10, Rslt_13);

Term Rslt_13V = get(term_loc(Rslt_13));
apps = take(port(1, term_loc(Rslt_13V)));
lams = take(port(2, term_loc(Rslt_13V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_13), pair_make(LAZ, 0, apps, lams));
dupLabels[0] = "";
// link args to body
swapStore(r_seq_6, seq_10);

V = get(r_seq_6);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 0, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_6, term_new(VAR, 0, vLoc));
dupLabels[142] = "";
// link args to body
swapStore(r_seq_2, seq_6);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 142, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblFn734 = new_ref(glblCFn733);
Term glblVal736;
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[7];
} str17 = {StringBufferType, REFS_STATIC, 0, 0, 6, "bad-f\n"};
Term glblStr740 = term_new_(VAL, (Term)&str17);
Term glblVal736;
void glblCFn738(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_3 = pair_make(LAM, 143, SUB, NUL);
Location r_seq_3 = port(2, term_loc(seq_3));
Term o_2 = term_new(VAR, 0, port(1, term_loc(seq_3)));
seq_3 = pair_make(LAM, 143, SUB, seq_3);
Term x_1 = term_new(VAR, 0, port(1, term_loc(seq_3)));
// allocate args at sidequest.toc: 183
Term seq_7 = pair_make(LAM, 144, SUB, NUL);
Location r_seq_7 = port(2, term_loc(seq_7));
Term p_6 = term_new(VAR, 0, port(1, term_loc(seq_7)));
// allocate args at sidequest.toc: 184
swapStore(term_loc(p_6), ERA);
// call pr* at sidequest.toc: 185
Term Rslt_9Args = pair_make(APP, 0, glblStr740, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
#ifdef STRICT
store_redex(Rslt_9Args, glblFn115);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, glblFn115));
#endif

// call o at sidequest.toc: 183
Term Rslt_10Args = pair_make(APP, 0, NUL, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
store_redex(Rslt_10Args, o_2);
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, o_2));
#endif

dupLabels[144] = "bad-f";
// link args to body
swapStore(r_seq_7, Rslt_10);

Term Rslt_10V = get(term_loc(Rslt_10));
apps = take(port(1, term_loc(Rslt_10V)));
lams = take(port(2, term_loc(Rslt_10V)));

vLoc = port(2, term_loc(apps));
apps = pair_make(APP, 0, Rslt_9, apps);
lams = pair_make(LAM, 144, sideEffects, lams);

swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, apps, lams));
// call x at sidequest.toc: 183
Term Rslt_11Args = pair_make(APP, 0, seq_7, SUB);
Term Rslt_11 = term_new(VAR, 0, port(2, term_loc(Rslt_11Args)));
#ifdef STRICT
store_redex(Rslt_11Args, x_1);
#else
swapStore(term_loc(Rslt_11), pair_make(LAZ, 0, Rslt_11Args, x_1));
#endif

dupLabels[143] = "";
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
Term glblVal736 = new_ref(glblCFn738);
Term glblFn743;
void glblCFn742(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 145, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term b_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 188
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term f_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at sidequest.toc: 189
// call b at sidequest.toc: 188
Term Rslt_8Args = pair_make(APP, 0, f_5, SUB);
Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
store_redex(Rslt_8Args, b_1);
#else
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, b_1));
#endif

dupLabels[0] = "";
// link args to body
swapStore(r_seq_6, Rslt_8);

Term Rslt_8V = get(term_loc(Rslt_8));
apps = take(port(1, term_loc(Rslt_8V)));
lams = take(port(2, term_loc(Rslt_8V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, apps, lams));
dupLabels[145] = "";
// link args to body
swapStore(r_seq_2, seq_6);

V = get(r_seq_2);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 145, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_2, term_new(VAR, 0, vLoc));
store_redex(args, seq_2);
return;
}
Term glblFn743 = new_ref(glblCFn742);
Term glblinput748;
void glblinput747(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_14 = pair_make(LAM, 147, SUB, NUL);
Location r_seq_14 = port(2, term_loc(seq_14));
Term f_13 = term_new(VAR, 0, port(1, term_loc(seq_14)));
seq_14 = pair_make(LAM, 147, SUB, seq_14);
Term t_12 = term_new(VAR, 0, port(1, term_loc(seq_14)));
// allocate args at sidequest.toc: 197
swapStore(term_loc(f_13), ERA);
dupLabels[147] = "input";
// link args to body
swapStore(r_seq_14, t_12);

V = get(r_seq_14);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 147, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_14, term_new(VAR, 0, vLoc));
store_redex(args, seq_14);
return;
}
Term glblinput748 = new_ref(glblinput747);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[8];
} str18 = {StringBufferType, REFS_STATIC, 0, 0, 7, "boogity"};
Term glblStr749 = term_new_(VAL, (Term)&str18);
Term glblfalse751;
void glblfalse750(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_19 = pair_make(LAM, 148, SUB, NUL);
Location r_seq_19 = port(2, term_loc(seq_19));
Term f_18 = term_new(VAR, 0, port(1, term_loc(seq_19)));
// allocate args at sidequest.toc: 200
dupLabels[148] = "false";
// link args to body
swapStore(r_seq_19, f_18);

V = get(r_seq_19);
apps = pair_make(APP, 0, V, SUB);
lams = pair_make(LAM, 148, SUB, NUL);
swapStore(port(2, term_loc(lams)),term_new(VAR, 0, port(1, term_loc(lams))));
vLoc = port(2, term_loc(apps));
swapStore(vLoc, pair_make(LAZ, 0, apps, lams));
swapStore(r_seq_19, term_new(VAR, 0, vLoc));
store_redex(args, seq_19);
return;
}
Term glblfalse751 = new_ref(glblfalse750);
struct {TYPE_SIZE type;
        REFS_SIZE refs;
        int64_t *hash;
        Value *parent;
        int64_t len;
        char buffer[14];
} str19 = {StringBufferType, REFS_STATIC, 0, 0, 13, "sidequest.toc"};
Term glblStr755 = term_new_(VAL, (Term)&str19);
void glblProto95_754(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(193), args);
  args = pair_make(APP, 0, glblStr755, args);
  interact(args, glblProto95);
}
}
void glblProto39_756(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(193), args);
  args = pair_make(APP, 0, glblStr755, args);
  interact(args, glblProto39);
}
}
void glblProto29_758(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 1, &arityArgs);
  if (arityArgs.count == 1) {
  swapStore(port(1, term_loc(args)), arityArgs.args[0]);
  args = pair_make(APP, 0, new_i60(193), args);
  args = pair_make(APP, 0, glblStr755, args);
  interact(args, glblProto29);
}
}
Term glblFn746;
void glblCFn745(Term ref, Term args) {
Term apps, lams, V;
Location vLoc;
Term seq_2 = pair_make(LAM, 146, SUB, NUL);
Location r_seq_2 = port(2, term_loc(seq_2));
Term args_1 = term_new(VAR, 0, port(1, term_loc(seq_2)));
// allocate args at sidequest.toc: 192
Term seq_6 = pair_make(LAM, 0, SUB, NUL);
Location r_seq_6 = port(2, term_loc(seq_6));
Term reps_5 = term_new(VAR, 0, port(1, term_loc(seq_6)));
// allocate args at sidequest.toc: 193
// call hbin at sidequest.toc: 195
Term Rslt_8Args = pair_make(APP, 0, new_i60(8), SUB);
Term Rslt_8 = term_new(VAR, 0, port(2, term_loc(Rslt_8Args)));
#ifdef STRICT
store_redex(Rslt_8Args, glblFn689);
#else
swapStore(term_loc(Rslt_8), pair_make(LAZ, 0, Rslt_8Args, glblFn689));
#endif

// call  at sidequest.toc: 193
Term Rslt_9Args = pair_make(APP, 0, reps_5, SUB);
Term Rslt_9 = term_new(VAR, 0, port(2, term_loc(Rslt_9Args)));
#ifdef STRICT
store_redex(Rslt_9Args, Rslt_8);
#else
swapStore(term_loc(Rslt_9), pair_make(LAZ, 0, Rslt_9Args, Rslt_8));
#endif

// call rep at sidequest.toc: 195
Term Rslt_10Args = pair_make(APP, 0, Rslt_9, SUB);
Term Rslt_10 = term_new(VAR, 0, port(2, term_loc(Rslt_10Args)));
#ifdef STRICT
store_redex(Rslt_10Args, glblVal709);
#else
swapStore(term_loc(Rslt_10), pair_make(LAZ, 0, Rslt_10Args, glblVal709));
#endif

// call  at sidequest.toc: 196
Term Rslt_16Args = pair_make(APP, 0, glblinput748, SUB);
Term Rslt_16 = term_new(VAR, 0, port(2, term_loc(Rslt_16Args)));
Rslt_16Args = pair_make(APP, 0, glblFn722, Rslt_16Args);
#ifdef STRICT
store_redex(Rslt_16Args, Rslt_10);
#else
swapStore(term_loc(Rslt_16), pair_make(LAZ, 0, Rslt_16Args, Rslt_10));
#endif

// call  at sidequest.toc: 199
Term Rslt_21Args = pair_make(APP, 0, new_i60(95), SUB);
Term Rslt_21 = term_new(VAR, 0, port(2, term_loc(Rslt_21Args)));
Rslt_21Args = pair_make(APP, 0, glblfalse751, Rslt_21Args);
Rslt_21Args = pair_make(APP, 0, glblStr749, Rslt_21Args);
#ifdef STRICT
store_redex(Rslt_21Args, Rslt_16);
#else
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, Rslt_21Args, Rslt_16));
#endif

dupLabels[0] = "";
// link args to body
swapStore(r_seq_6, Rslt_21);

Term Rslt_21V = get(term_loc(Rslt_21));
apps = take(port(1, term_loc(Rslt_21V)));
lams = take(port(2, term_loc(Rslt_21V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_21), pair_make(LAZ, 0, apps, lams));
// call nth at sidequest.toc: 193
Term Rslt_22Args = pair_make(APP, 0, new_i60(1), SUB);
Term Rslt_22 = term_new(VAR, 0, port(2, term_loc(Rslt_22Args)));
Rslt_22Args = pair_make(APP, 0, args_1, Rslt_22Args);
#ifdef STRICT
store_redex(Rslt_22Args, new_ref(glblProto95_754));
#else
swapStore(term_loc(Rslt_22), pair_make(LAZ, 0, Rslt_22Args, new_ref(glblProto95_754)));
#endif
// call map at sidequest.toc: 193
Term Rslt_23Args = pair_make(APP, 0, glblFn660, SUB);
Term Rslt_23 = term_new(VAR, 0, port(2, term_loc(Rslt_23Args)));
Rslt_23Args = pair_make(APP, 0, Rslt_22, Rslt_23Args);
#ifdef STRICT
store_redex(Rslt_23Args, new_ref(glblProto39_756));
#else
swapStore(term_loc(Rslt_23), pair_make(LAZ, 0, Rslt_23Args, new_ref(glblProto39_756)));
#endif
// call either at sidequest.toc: 193
Term Rslt_24Args = pair_make(APP, 0, new_i60(4), SUB);
Term Rslt_24 = term_new(VAR, 0, port(2, term_loc(Rslt_24Args)));
Rslt_24Args = pair_make(APP, 0, Rslt_23, Rslt_24Args);
#ifdef STRICT
store_redex(Rslt_24Args, new_ref(glblProto29_758));
#else
swapStore(term_loc(Rslt_24), pair_make(LAZ, 0, Rslt_24Args, new_ref(glblProto29_758)));
#endif
// call  at sidequest.toc: 193
Term Rslt_25Args = pair_make(APP, 0, Rslt_24, SUB);
Term Rslt_25 = term_new(VAR, 0, port(2, term_loc(Rslt_25Args)));
#ifdef STRICT
store_redex(Rslt_25Args, seq_6);
#else
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, Rslt_25Args, seq_6));
#endif

dupLabels[146] = "";
// link args to body
swapStore(r_seq_2, Rslt_25);

Term Rslt_25V = get(term_loc(Rslt_25));
apps = take(port(1, term_loc(Rslt_25V)));
lams = take(port(2, term_loc(Rslt_25V)));

vLoc = port(2, term_loc(apps));
swapStore(term_loc(Rslt_25), pair_make(LAZ, 0, apps, lams));
store_redex(args, seq_2);
return;
}
Term glblFn746 = new_ref(glblCFn745);
Term glblProto51;
void glblCFn52(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'wrap' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblwrap505);
break;
default: {
fprintf(stderr, "No implementation of 'wrap' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto51 = new_ref(glblCFn52);
Term glblProto69;
void glblCFn70(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'reduce' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblreduce511);
break;
default: {
fprintf(stderr, "No implementation of 'reduce' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto69 = new_ref(glblCFn70);
Term glblProto105;
void glblCFn106(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'vals' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'vals' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto105 = new_ref(glblCFn106);
Term glblProto63;
void glblCFn64(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'empty' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblempty525);
break;
default: {
fprintf(stderr, "No implementation of 'empty' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto63 = new_ref(glblCFn64);
Term glblProto81;
void glblCFn82(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'split' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'split' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto81 = new_ref(glblCFn82);
Term glblProto89;
void glblCFn90(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'drop-while' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'drop-while' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto89 = new_ref(glblCFn90);
Term glblProto37;
void glblCFn38(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'recurse' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glblrecurse364);
break;
case 5:
store_redex(args, glblrecurse362);
break;
case 46:
store_redex(args, glblrecurse551);
break;
case 43:
store_redex(args, glblrecurse197);
break;
case 45:
store_redex(args, glblrecurse589);
break;
case 42:
store_redex(args, glblrecurse147);
break;
default: {
fprintf(stderr, "No implementation of 'recurse' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto37 = new_ref(glblCFn38);
Term glblProto313;
void glblFldFn314(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.s' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 5:
store_redex(args, glblFld329);
break;
default: {
fprintf(stderr, "No implementation of '.s' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto313 = new_ref(glblFldFn314);
Term glblProto578;
void glblFldFn580(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.head' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 45:
store_redex(args, glblFld598);
break;
default: {
fprintf(stderr, "No implementation of '.head' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto578 = new_ref(glblFldFn580);
Term glblProto29;
void glblCFn30(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'either' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glbleither195);
break;
case 42:
store_redex(args, glbleither145);
break;
default: {
fprintf(stderr, "No implementation of 'either' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto29 = new_ref(glblCFn30);
Term glblProto93;
void glblCFn94(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'reverse' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblreverse509);
break;
default: {
fprintf(stderr, "No implementation of 'reverse' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto93 = new_ref(glblCFn94);
Term glblProto15;
void glblCFn16(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
store_redex(args, glbl_EQ_262);
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glbl_EQ_347);
break;
case 5:
store_redex(args, glbl_EQ_324);
break;
case 43:
store_redex(args, glbl_EQ_201);
break;
case 8:
store_redex(args, glbl_EQ_539);
break;
case 42:
store_redex(args, glbl_EQ_149);
break;
default: {
fprintf(stderr, "No implementation of '=' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto15 = new_ref(glblCFn16);
Term glblProto95;
void glblCFn96(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'nth' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblnth529);
break;
default: {
fprintf(stderr, "No implementation of 'nth' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto95 = new_ref(glblCFn96);
Term glblProto3;
void glblCFn4(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
store_redex(args, glbltype_name266);
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glbltype_name372);
break;
case 5:
store_redex(args, glbltype_name370);
break;
case 8:
store_redex(args, glbltype_name537);
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
Term glblProto25;
void glblCFn26(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'and' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glbland191);
break;
case 42:
store_redex(args, glbland141);
break;
default: {
fprintf(stderr, "No implementation of 'and' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto25 = new_ref(glblCFn26);
Term glblProto307;
void glblCFn308(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'subs' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glblsubs345);
break;
case 5:
store_redex(args, glblsubs326);
break;
default: {
fprintf(stderr, "No implementation of 'subs' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto307 = new_ref(glblCFn308);
Term glblProto583;
void glblFldFn584(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.tail' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 45:
store_redex(args, glblFld601);
break;
default: {
fprintf(stderr, "No implementation of '.tail' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto583 = new_ref(glblFldFn584);
Term glblProto91;
void glblCFn92(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'take-while' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'take-while' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto91 = new_ref(glblCFn92);
Term glblProto403;
void glblCFn404(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'vect-reverse' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblvect_reverse416);
break;
case 42:
store_redex(args, glblvect_reverse413);
break;
default: {
fprintf(stderr, "No implementation of 'vect-reverse' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto403 = new_ref(glblCFn404);
Term glblProto45;
void glblCFn46(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'flatten' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblflatten501);
break;
default: {
fprintf(stderr, "No implementation of 'flatten' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto45 = new_ref(glblCFn46);
Term glblProto23;
void glblCFn24(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
store_redex(args, glbl_LT_260);
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of '<' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto23 = new_ref(glblCFn24);
Term glblProto87;
void glblCFn88(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'drop' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'drop' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto87 = new_ref(glblCFn88);
Term glblProto39;
void glblCFn40(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'map' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblmap187);
break;
case 8:
store_redex(args, glblmap507);
break;
case 42:
store_redex(args, glblmap137);
break;
default: {
fprintf(stderr, "No implementation of 'map' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto39 = new_ref(glblCFn40);
Term glblProto41;
void glblCFn42(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'map!' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'map!' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto41 = new_ref(glblCFn42);
Term glblProto65;
void glblCFn66(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'conj' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblconj515);
break;
default: {
fprintf(stderr, "No implementation of 'conj' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto65 = new_ref(glblCFn66);
Term glblProto73;
void glblCFn74(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'first' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblfirst521);
break;
default: {
fprintf(stderr, "No implementation of 'first' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto73 = new_ref(glblCFn74);
Term glblProto79;
void glblCFn80(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'butlast' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'butlast' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto79 = new_ref(glblCFn80);
Term glblProto396;
void glblCFn397(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'vect-reduce' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblvect_reduce402);
break;
case 42:
store_redex(args, glblvect_reduce399);
break;
default: {
fprintf(stderr, "No implementation of 'vect-reduce' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto396 = new_ref(glblCFn397);
Term glblProto33;
void glblCFn34(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'zero' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'zero' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto33 = new_ref(glblCFn34);
Term glblProto43;
void glblCFn44(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'flat-map' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblflat_map185);
break;
case 8:
store_redex(args, glblflat_map503);
break;
default: {
fprintf(stderr, "No implementation of 'flat-map' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto43 = new_ref(glblCFn44);
Term glblProto71;
void glblCFn72(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'vec' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'vec' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto71 = new_ref(glblCFn72);
Term glblProto315;
void glblFldFn316(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.start' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 5:
store_redex(args, glblFld331);
break;
default: {
fprintf(stderr, "No implementation of '.start' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto315 = new_ref(glblFldFn316);
Term glblProto417;
void glblCFn418(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'vect-sub' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblvect_sub434);
break;
case 42:
store_redex(args, glblvect_sub431);
break;
default: {
fprintf(stderr, "No implementation of 'vect-sub' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto417 = new_ref(glblCFn418);
Term glblProto75;
void glblCFn76(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'rest' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblrest517);
break;
default: {
fprintf(stderr, "No implementation of 'rest' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto75 = new_ref(glblCFn76);
Term glblProto47;
void glblCFn48(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'extend' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'extend' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto47 = new_ref(glblCFn48);
Term glblProto83;
void glblCFn84(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'split-with' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'split-with' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto83 = new_ref(glblCFn84);
Term glblProto59;
void glblCFn60(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'empty?' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblempty_QM_527);
break;
default: {
fprintf(stderr, "No implementation of 'empty?' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto59 = new_ref(glblCFn60);
Term glblProto67;
void glblCFn68(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'filter' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblfilter499);
break;
default: {
fprintf(stderr, "No implementation of 'filter' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto67 = new_ref(glblCFn68);
Term glblProto97;
void glblCFn98(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'store' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblstore513);
break;
default: {
fprintf(stderr, "No implementation of 'store' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto97 = new_ref(glblCFn98);
Term glblProto85;
void glblCFn86(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'take' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'take' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto85 = new_ref(glblCFn86);
Term glblProto61;
void glblCFn62(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'count' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glblcount349);
break;
case 5:
store_redex(args, glblcount322);
break;
case 46:
store_redex(args, glblcount553);
break;
case 8:
store_redex(args, glblcount533);
break;
case 45:
store_redex(args, glblcount592);
break;
default: {
fprintf(stderr, "No implementation of 'count' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto61 = new_ref(glblCFn62);
Term glblProto17;
void glblFldFn18(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.x' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblFld21);
break;
default: {
fprintf(stderr, "No implementation of '.x' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto17 = new_ref(glblFldFn18);
Term glblProto107;
void glblCFn108(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
store_redex(args, glblstr_vect264);
} else {
switch(dispVal->type) {
case 2:
store_redex(args, glblstr_vect368);
break;
case 5:
store_redex(args, glblstr_vect366);
break;
case 43:
store_redex(args, glblstr_vect203);
break;
case 8:
store_redex(args, glblstr_vect535);
break;
default: {
fprintf(stderr, "No implementation of 'str-vect' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto107 = new_ref(glblCFn108);
Term glblProto57;
void glblCFn58(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'interpose' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblinterpose497);
break;
default: {
fprintf(stderr, "No implementation of 'interpose' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto57 = new_ref(glblCFn58);
Term glblProto35;
void glblCFn36(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'comp' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblcomp523);
break;
default: {
fprintf(stderr, "No implementation of 'comp' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto35 = new_ref(glblCFn36);
Term glblProto103;
void glblCFn104(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'keys' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'keys' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto103 = new_ref(glblCFn104);
Term glblProto55;
void glblCFn56(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'apply' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'apply' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto55 = new_ref(glblCFn56);
Term glblProto27;
void glblCFn28(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'or' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblor189);
break;
case 42:
store_redex(args, glblor139);
break;
default: {
fprintf(stderr, "No implementation of 'or' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto27 = new_ref(glblCFn28);
Term glblProto31;
void glblCFn32(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
store_redex(args, glblcond258);
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblcond193);
break;
case 42:
store_redex(args, glblcond143);
break;
default: {
fprintf(stderr, "No implementation of 'cond' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto31 = new_ref(glblCFn32);
Term glblProto77;
void glblCFn78(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'last' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glbllast519);
break;
default: {
fprintf(stderr, "No implementation of 'last' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto77 = new_ref(glblCFn78);
Term glblProto317;
void glblFldFn318(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of '.len' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 5:
store_redex(args, glblFld333);
break;
case 45:
store_redex(args, glblFld604);
break;
default: {
fprintf(stderr, "No implementation of '.len' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto317 = new_ref(glblFldFn318);
Term glblProto99;
void glblCFn100(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'assoc' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'assoc' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto99 = new_ref(glblCFn100);
Term glblProto53;
void glblCFn54(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'extract' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 43:
store_redex(args, glblextract199);
break;
default: {
fprintf(stderr, "No implementation of 'extract' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto53 = new_ref(glblCFn54);
Term glblProto49;
void glblCFn50(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'duplicate' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'duplicate' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto49 = new_ref(glblCFn50);
Term glblProto109;
void glblCFn110(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'free-resource' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
default: {
fprintf(stderr, "No implementation of 'free-resource' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto109 = new_ref(glblCFn110);
Term glblProto101;
void glblCFn102(Term ref, Term args) {
  NativeArgs arityArgs = {0, {}};
  args = strictArgs(ref, args, 3, &arityArgs);
  if (arityArgs.count == 3) {
    String *fileStr = (String *)arityArgs.args[0];
    Value *dispVal = (Value *)arityArgs.args[2];
    swapStore(term_loc(args), (Term)dispVal);
    if (term_tag((Term)dispVal) == I60) {
fprintf(stderr, "No implementation of 'get' found for integers called from %.*s: %ld\n", (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
} else {
switch(dispVal->type) {
case 8:
store_redex(args, glblget531);
break;
default: {
fprintf(stderr, "No implementation of 'get' found for type %s (%ld) called from %.*s: %ld\n", typeName(dispVal->type), dispVal->type, (int)fileStr->len, fileStr->buffer, get_i60(arityArgs.args[1]));
BOOM("Dispatch failed");
}
}
}
}
return;
}
Term glblProto101 = new_ref(glblCFn102);
char *dupLabels[154];
unsigned refsCount = 205;
unsigned refNameCount = 205;
refMap refNames[205] = {{intCond, "intCond"},
{vectMap, "vectMap"},
{vectConjFn, "vectConj"},
{graphFn, "graph"},
{accessFieldFn, "accessField"},
{hvmVectFn, "hvmVect"},
{exitProg, "exitProg"},
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
{glblrecurse550, "recurse 46"},
{glblrecurse196, "recurse 43"},
{glblrecurse588, "recurse 45"},
{glblrecurse146, "recurse 42"},
{glblFldFn314, ".s"},
{glblFldFn328, ".s 5"},
{glblCFn129, "*"},
{glblFldFn580, ".head"},
{glblFldFn596, ".head 45"},
{glblCFn30, "either"},
{glbleither194, "either 43"},
{glbleither144, "either 42"},
{glblCFn212, "<="},
{glblCFn94, "reverse"},
{glblreverse508, "reverse 8"},
{glblCFn16, "="},
{glbl_EQ_346, "= 2"},
{glbl_EQ_323, "= 5"},
{glbl_EQ_200, "= 43"},
{glbl_EQ_538, "= 8"},
{glbl_EQ_148, "= 42"},
{glbl_EQ_261, "= 1"},
{glblCFn271, "vect-count"},
{glblCons595, "Cons"},
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
{glblc456, "core: 680"},
{glblCFn544, "cons"},
{glblCFn308, "subs"},
{glblsubs344, "subs 2"},
{glblsubs325, "subs 5"},
{glblFldFn584, ".tail"},
{glblFldFn600, ".tail 45"},
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
{glblCFn302, "str-malloc"},
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
{glblc163, "core: 314"},
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
{glblcount552, "count 46"},
{glblcount532, "count 8"},
{glblcount591, "count 45"},
{glblFldFn18, ".x"},
{glblFldFn20, ".x 43"},
{glblSome19, "Some"},
{glblCFn108, "str-vect"},
{glblstr_vect367, "str-vect 2"},
{glblstr_vect365, "str-vect 5"},
{glblstr_vect202, "str-vect 43"},
{glblstr_vect534, "str-vect 8"},
{glblstr_vect263, "str-vect 1"},
{glblCFn120, "inc"},
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
{glblCFn32, "cond"},
{glblcond192, "cond 43"},
{glblcond142, "cond 42"},
{glblcond257, "cond 1"},
{glblCFn78, "last"},
{glbllast518, "last 8"},
{glblc165, "core: 312"},
{glblFldFn318, ".len"},
{glblFldFn332, ".len 5"},
{glblFldFn603, ".len 45"},
{glblCFn100, "assoc"},
{glblCFn293, "str-count"},
{glblCFn54, "extract"},
{glblextract198, "extract 43"},
{glblCFn50, "duplicate"},
{glblCFn277, "identity"},
{glblSubString327, "SubString"},
{glblCFn110, "free-resource"},
{glblCFn102, "get"},
{glblget530, "get 8"},
{glblCFn208, "int-<"},
{glblCFn677, "pr0"},
{glblCFn685, "output"},
{glblrep_E727, "rep-E"},
{glblCFn668, ">>"},
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
typeNameMap typeNames[21] = {{1, "Integer"},
{2, "String"},
{8, "Vector"},
{9, "VectorNode"},
{11, "HashMap"},
{12, "HashMap"},
{13, "HashMap"},
{14, "HashMap"}
,
{11, "BitmapIndexedNode"},
{45, "Cons"},
{18, "Opaque"},
{3, "FnArity"},
{12, "ArrayNode"},
{43, "Some"},
{13, "HashCollisionNode"},
{42, "None"},
{46, "EndOfList"},
{44, "Leaf"},
{5, "SubString"}};
void normGlobals(){
Term apps, lams, V;
Location vLoc;
{
Term glblVal14_1;
glblVal14 = dupeArg(glblVal14, &glblVal14_1, 0);

// call None at core: 65
Term glblRslt13Args = pair_make(APP, 0, NUL, SUB);
glblRslt13 = term_new(VAR, 0, port(2, term_loc(glblRslt13Args)));
store_redex(glblRslt13Args, glblVal14_1);
}
{
Term glblVal297_1;
glblVal297 = dupeArg(glblVal297, &glblVal297_1, 0);

// call Leaf at core: 430
Term glblRslt296Args = pair_make(APP, 0, NUL, SUB);
glblRslt296 = term_new(VAR, 0, port(2, term_loc(glblRslt296Args)));
store_redex(glblRslt296Args, glblVal297_1);
}
{
Term glblVal560_1;
glblVal560 = dupeArg(glblVal560, &glblVal560_1, 0);

// call EndOfList at core: 797
Term glblRslt558Args = pair_make(APP, 0, NUL, SUB);
glblRslt558 = term_new(VAR, 0, port(2, term_loc(glblRslt558Args)));
store_redex(glblRslt558Args, glblVal560_1);
}
  normalize(NULL);
  if (term_tag(glblVal607) == VAR)
    glblVal607 = take(term_loc(glblVal607));
  if (term_tag(glblRslt558) == VAR)
    glblRslt558 = take(term_loc(glblRslt558));
  if (term_tag(glblVal560) == VAR)
    glblVal560 = take(term_loc(glblVal560));
  if (term_tag(glblVal450) == VAR)
    glblVal450 = take(term_loc(glblVal450));
  if (term_tag(glblVal334) == VAR)
    glblVal334 = take(term_loc(glblVal334));
  if (term_tag(glblRslt296) == VAR)
    glblRslt296 = take(term_loc(glblRslt296));
  if (term_tag(glblVal297) == VAR)
    glblVal297 = take(term_loc(glblVal297));
  if (term_tag(glblVal214) == VAR)
    glblVal214 = take(term_loc(glblVal214));
  if (term_tag(glblVal22) == VAR)
    glblVal22 = take(term_loc(glblVal22));
  if (term_tag(glblRslt13) == VAR)
    glblRslt13 = take(term_loc(glblRslt13));
  if (term_tag(glblVal14) == VAR)
    glblVal14 = take(term_loc(glblVal14));
  if (term_tag(glblVal9) == VAR)
    glblVal9 = take(term_loc(glblVal9));
}
void freeGlobals(){
  freeGlobal(glblVal607);
  freeGlobal(glblRslt558);
  freeGlobal(glblVal560);
  freeGlobal(glblVal450);
  freeGlobal(glblVal334);
  freeGlobal(glblRslt296);
  freeGlobal(glblVal297);
  freeGlobal(glblVal214);
  freeGlobal(glblVal22);
  freeGlobal(glblRslt13);
  freeGlobal(glblVal14);
  freeGlobal(glblVal9);
  normalize(NULL);
}
Term mainFn = new_ref(glblCFn745);
int typeCount = 47;
