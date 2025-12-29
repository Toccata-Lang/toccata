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
*(To be defined)*

### 3.2 Execution Model
*(To be defined)*

## 4. Standard Library
*(To be defined)*

## 5. Examples
*(To be added)*

## 6. Tooling Guidelines
*(To be defined)*