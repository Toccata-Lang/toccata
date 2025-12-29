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
    - `def`: Associates a symbol with the value of an expression. Only valid at the top level.
      Example:
      ```
      (def some-result (f 6 'some-string))
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