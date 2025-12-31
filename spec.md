# Toccata Language Specification

This document defines the syntax and semantics of the Toccata programming language.

## 1. Introduction

### 1.1 Purpose
Toccata is designed to be a minimalist, LLM-friendly language for generating code, tests, and documentation.

## 2. Syntax

### 2.1 Lexical Structure

Toccata has the following atomic literals:

- **Numbers**: Integers or decimals (e.g., `42`, `3.14`).
- **Symbols**: A sequence of non-whitespace characters bounded by whitespace or special characters.
- **String Literals**:
  - Single-quoted: A symbol preceded by a single quote (`'`). No escapes allowed.
  - Double-quoted: Wrapped in `"` with escaped special characters using backslashes. Example: `"Hello, \"world\"!"`.
- **Expressions**: A sequence of atoms or subexpressions surrounded by parentheses.
  - Simple expressions are usually function calls, where the first sub-expression is the function to be called.
  - The first symbol may be one of a few special forms (e.g., `def`, `defn`, `deftype`, `defp`).
    - Top Level Special Forms
      - `def`: Associates a symbol with the value of an expression. Only valid at the top level.
        Example:
        ```
        (def some-result (f 6 'some-string))
        ```
      - defmacro: Defines a macro. Similar to defn, but the expressions are evaluated at compile time, and the compiler then compiles the result.
        - The macro name is followed by a parameter vector containing only one symbol.
        Example:
        ```
        (defmacro when [test] `(if ~test (do ~@(rest (expr)))))
        ```
      - `defn`: Defines a named function. The function name is followed by a parameter list in brackets and then sub-expressions. The last expression's result is returned.
        Example:
        ```
        (defn inc [x] (+ 1 x))
        ```
      - `deftype`: Defines a named type. *(Details to be specified in the Types section.)*
      - `defp`: Defines a prototype function to be implemented by types.
        - Must have at least one parameter.
        - Sub-expressions are optional and serve as a default implementation.
        - The specific implementation is determined at runtime by the type of the first parameter.
        - If no implementation exists, the default body runs (or throws an exception if none provided).
      - `extend-type`: Extends a type with additional methods or fields. *(Details to be specified in the Types section.)*
      - `add-ns`: Tells the compiler to add definitions from another file. It specifies the namespace prefix used to distinguish imported symbols.
      - `inline`: Allows a chunk of low-level C code to be used. An inline expression may appear as the only non-comment expression in the body of a "defn".
      - `main`: Specifies the entry point of the program. Has the same structure as "defn".
      - Single-line comments: Start with a ";" and go to the end of the line. These may also appear inside expressions and are treated as whitespace.
    - Sub Expression Special Forms
      - `fn`: Specifies an anonymous function value with a parameter list and a body of sub-expressions.
        Example:
        ```
        (fn [x] (+ x 1)) ; an anonymous function to increment 'x'
        ```
      - `let`: Binding pairs are enclosed in brackets followed by one or more sub-expressions.
        Example:
        ```
        (let [x 5 y 10] (+ x y)) ; Binds x to 5 and y to 10, then evaluates (+ x y)
        ```
      - `do`: The body contains one or more sub-expressions.
      - `|`: Superposition. This causes a separate code path to be executed for each of the sub-expressions in the expression.
        Example:
        ```
        (print (| "Hello" "World")) ; Prints both "Hello" and "World" independently
        ```
      - `.field-name`: Field getters, which are a symbol representing the name of the field with a preceding ".".
      - `module`: Only valid when in an "add-ns" form. Contains a string which is a file path relative to the file it appears in, of another file to get definitions from.
      - Type constraints: Start with "!", followed by a symbol and then a type expression.
      - `->`: Threading. The result of each expression gets inserted as the first argument in the next expression.
      - `match`: Takes an expression, then a sequence of type/value expression pairs, and then an optional expression. It checks the type of the first expression against the type expression of each pair and when it finds a match, returns the value expression of that pair. The final, optional expression is returned if no other match is found. If no match is found and no default value is provided, an exception is thrown.
    - The following symbols have special meaning in Toccata:
      - `_FILE_`: evaluates to the file name in which it appears
      - `_LINE_`: evaluates to the line number on which it appears
- **Vector Expressions**: Like an expression but surrounded by brackets (`[]`).
  Example:
  ```
  [a 'b 26]
  ```
- **Hash Map Expressions**: Pairs of expressions (key-value) surrounded by curly braces (`{}`).
  Example:
  ```
  {a 1 b 2}
  ```

### 2.2 Grammar

*(Full EBNF grammar to be defined later.)*

## 3. Semantics

### 3.1 Types
Toccata is a gradual, dependently typed language. Type constraints are optional. There are several built-in types.

#### Integer
Integers are always 60 bits wide.

#### Float
*(Details to be specified)*

#### String
*(Details to be specified)*

#### Vector
Plain vectors may hold values of any type. They are always appended to at the end and are indexed by an integer position.

#### HashMap
Plain hash maps may map any hashable value to any other value.

#### Named Types
Named types are defined using 'deftype' in various forms.

The simplest example is `(deftype Some [x])` where the name of the type and the name of the constructor function are the same.

If no fields are given, as in `(deftype None [])`, a single value with the same name as the type is created instead of a constructor function.

Constructors with fields are product types. Sum types may be created using `deftype` by giving a number of constructors
```
(deftype Maybe
    (None [])
    (Some [x]))
```
In this case, None and Some are also types.

Constructors may also specify implementations of protocol functions.
```
(deftype SomeType [x y]
   (proto-fn [v n]
      (+ x n)))
```

Type constraints may be defined for fields. (Type constraints are described below)
```
(deftype List
    (EndOfList [])
    (Cons [head tail]
      ! tail List
      ))
```
where `List` is a recursive type and the `tail` field of a `Cons` must either be and EndOfList or a Cons type.

Defining a vector of a values whose types are constrained in some way is done like
```
(deftyp IntVect [Integer])
```

Likewise, constraining the keys and values in a hash map are done by
```
(deftype IntStrMap {Integer String})
```
#### Type Constraints
Symbols may be constrained to only allow a set of values. These type constraints are indicated by a line starting with "!", followed by the symbol and then a type expression describing the values allowed. For a function, the special symbol "->" denotes a constraint that the value returned by the function must satisfy.

#### Type Expressions
String, Integer, Float, and Vector can be further constrained by expressions. Any string or number literal can be a type constraint. If a symbol is used as a constraint, it evaluates to the type constraint of that symbol in the code at that point. Constraints can be composed to form more precise constraints.

##### min
Specifies a number with a minimum value.
Example:
```
(min 2)
```

##### max
Specifies a number with a maximum value.
Example:
```
(max 10)
```

##### min-length
Specifies the minimum length for vectors and strings.
Example:
```
(min-length 3)
```

##### max-length
Specifies the maximum length for vectors and strings.
Example:
```
(max-length 5)
```

##### length
Specifies an exact length for vectors and strings.
Example:
```
(length 4)
```

##### any-of
At least one of the sub expressions must be satisfied
Example:
```
(any-of Integer Float)
```

##### all-of
All sub expressions must be satisfied.
Example:
```
(all-of Integer (min 0) (max 5))
```

### 3.2 Execution Model
*(To be defined)*

## 4. Standard Library
*(To be defined)*

## 5. Examples
*(To be added)*

## 6. Tooling Guidelines
*(To be defined)*