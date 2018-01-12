
linear-whitespace = ',' | ' ' | '\t';

newline = '\n';

whitespace = linear-whitespace | '\r' | newline;

open-paren = { whitespace }, '(', { whitespace };

lower-alpha = 'a' - 'z';

upper-alpha = 'A' - 'Z';

alpha = lower-alpha | upper-alpha;

digit = '0' - '9';

namespace-punct = '.' | '_' | '<' | '>' | '=' | '*' | '+' | '!' | '-' | '?';

rest-of-namespace = { alpha | digit | namespace-punct };

namespace = alpha, rest-of-namespace, '/';

symbol-start = alpha | '.' | '_' | '<' | '>' | '=' | '+' | '-' | '*' | '/';

symbol-punct = '.' | '_' | '<' | '>' | '=' | '+' | '-' | '*' | '/' | '!' | '?';

symbol-char = alpha | digit | symbol-punct;

rest-of-symbol = { symbol-char };

tagged-symbol = [namespace], symbol-start, rest-of-symbol;

arg = { whitespace }, tagged-symbol, { whitespace };

var-arg = { whitespace }, '&', arg;

list-destructure = { whitespace }, '[', { destructure }, var-arg | , ']';

close-paren = { whitespace }, ')';

type-assertion = open-paren, { whitespace }, 'instance?', whitespace, { whitespace }, tagged-symbol, whitespace, { whitespace }, tagged-symbol, close-paren;

number = digit, { digit } | '-', digit, { digit };

assert = open-paren, 'assert', whitespace, { whitespace }, type-assertion | type-assertion, close-paren;

assert-result = open-paren, 'assert-result', whitespace, { whitespace }, tagged-symbol, whitespace, { whitespace }, type-assertion, close-paren;

not-eol = not '\n';

block-comment = { whitespace }, ';', { ';' }, { not-eol }, newline, { { whitespace }, ';', { ';' }, { not-eol }, newline };

read-string-fragment = (* not '"' or '\' *)

backslash = '\\', '\\';

doublequote = '\\', '\"';

tab = '\\', 't';

backspace = '\\', 'b';

return = '\\', 'r';

formfeed = '\\', 'f';

string = '\"', { read-string-fragment | backslash | doublequote | tab | backspace | return | formfeed | newline }, '\"';

read-inline = open-paren, 'inline', whitespace, { whitespace }, tagged-symbol, whitespace, { whitespace }, [tagged-symbol, whitespace, { whitespace }], string, close-paren;

read-inline-body = { assert | assert-result | block-comment }, { whitespace }, read-inline, { block-comment };

\_FILE\_ = { whitespace }, '\_FILE\_';

\_LINE\_ = { whitespace }, '\_LINE\_';

quoted-value = { whitespace }, open-paren, { quoted-value }, close-paren | '[', quoted-value, { quoted-value }, ']' | number | string | tagged-symbol, { whitespace };

quoted = ''', quoted-value;

or = open-paren, 'or', whitespace, { whitespace }, expr, { expr }, close-paren;

and = open-paren, 'and', whitespace, { whitespace }, expr, { expr }, close-paren;

either = open-paren, 'either', whitespace, { whitespace }, expr, expr, close-paren;

fn-doc = block-comment | ;

single-arity = list-destructure, fn-doc, read-inline-body | expr, { expr } | , { whitespace };

arities = single-arity | fn-doc, open-paren, single-arity, close-paren, { open-paren, single-arity, close-paren };

fn = open-paren, 'fn', whitespace, { whitespace }, [tagged-symbol], { whitespace }, arities, close-paren | ;

destructure = list-destructure | arg;

let-binding = { whitespace }, [block-comment], { whitespace }, destructure, expr;

let = open-paren, 'let', { whitespace }, '[', let-binding, { let-binding }, ']', expr, { expr }, close-paren | ;

do = open-paren, 'do', whitespace, { whitespace }, expr, { expr }, close-paren;

comp = '(', { whitespace }, 'comp', whitespace, { whitespace }, expr, { expr }, { whitespace }, ')' | ;

apply = '(', { whitespace }, 'apply', whitespace, { whitespace }, expr, { expr }, { whitespace }, ')' | ;

apply-to = '(', { whitespace }, 'apply-to', whitespace, { whitespace }, expr, expr, { expr }, { whitespace }, ')' | ;

protocol-implementation = open-paren, tagged-symbol, single-arity, close-paren;

protocol-implementations = { whitespace }, tagged-symbol, protocol-implementation, { protocol-implementation } | assert | block-comment;

reify = open-paren, 'reify', whitespace, { whitespace }, protocol-implementations, { protocol-implementations }, close-paren;

vector = '[', { expr }, ']' | ;

hash-map = '{', { expr, expr }, '}' | ;

hash-set = '#{', { expr }, '}' | ;

for-let = { whitespace } | block-comment, ':let', whitespace, { whitespace }, '[', let-binding, { let-binding }, ']', { whitespace };

for-when = { whitespace } | block-comment, ':when', whitespace, { whitespace }, expr;

for-when-not = { whitespace } | block-comment, ':when-not', whitespace, { whitespace }, expr;

for-binding = for-let | for-when | for-when-not | let-binding | block-comment, { whitespace };

for = open-paren, 'for', whitespace, { whitespace }, '[', let-binding | , { for-binding } | , ']', expr |  | , close-paren;

call = open-paren, expr, { expr }, close-paren;

->exp = tagged-symbol | call, { whitespace };

-> = open-paren, '->', expr, ->exp, { ->exp } | , close-paren;

expr = { whitespace }, number | string | _FILE_ | _LINE_ | tagged-symbol | quoted | or | and | either | fn | let | do | comp | apply | apply-to | reify | vector | hash-map | hash-set | for | -> | assert | assert-result | read-inline | call | block-comment, { whitespace };

main = open-paren, 'main', list-destructure, read-inline-body | expr, { expr } | , close-paren | ;

def = open-paren, 'def', whitespace, { whitespace }, tagged-symbol, whitespace, { whitespace }, { block-comment }, read-inline | expr, { whitespace }, { block-comment }, close-paren | tagged-symbol, close-paren | ;

prototype = assert | open-paren, tagged-symbol, list-destructure, fn-doc, read-inline-body | { expr }, close-paren | ;

defprotocol = open-paren, 'defprotocol', whitespace, { whitespace }, tagged-symbol, block-comment | prototype, { block-comment | prototype }, close-paren | ;

defn = open-paren, 'defn', whitespace, { whitespace }, tagged-symbol, whitespace, { whitespace }, arities, close-paren | ;

extend-type = open-paren, 'extend-type', whitespace, { whitespace }, tagged-symbol, protocol-implementations, { protocol-implementations }, close-paren | ;

deftype = open-paren, 'deftype', whitespace, { whitespace }, tagged-symbol, list-destructure, { protocol-implementations }, close-paren | ;

module = open-paren, 'module', whitespace, { whitespace }, string, close-paren | ;

git-kwargs = { whitespace, { whitespace }, ':', 'tag', whitespace, { whitespace }, string | whitespace, { whitespace }, ':', 'sha', whitespace, { whitespace }, string | whitespace, { whitespace }, ':', 'branch', whitespace, { whitespace }, string };

git-dep = open-paren, 'git-dependency', whitespace, { whitespace }, string, whitespace, { whitespace }, string, git-kwargs, close-paren | ;

add-ns = open-paren, 'add-ns', whitespace, { whitespace }, tagged-symbol, module | git-dep, close-paren | ;

bad-expr = { whitespace }, '(';

top-level = main | read-inline | def | defprotocol | defn | extend-type | deftype | block-comment | add-ns | bad-expr;