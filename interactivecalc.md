# Interaction Calculus

## Overview

The Interaction Calculus is a formal system describing computation through interactions between polarized nodes. Nodes are classified by type, each with a polarity (+ or −). Interactions occur between nodes of opposite polarity, while auxiliary ports connect only to nodes of matching polarity.

## Polarization Rules

1. **Interaction rule**: Two nodes can interact only if they have opposite polarities.
2. **Auxiliary port rule**: An auxiliary port can only connect to a node of the same polarity as the port.

## Triangle Nodes

Triangle nodes have three ports: one **principal port** (at a vertex) and two **auxiliary ports** (on the opposite edge). The principal port is designated by its position relative to the triangle's orientation; the two remaining ports are Port 1 (left) and Port 2 (right).

| Node | Node polarity | Port 1 | Port 2 | Principal |
|---|---|---|---|---|
| **APP** | − | + | − | − |
| **LAM** | + | − | + | + |
| **SUP** | + | + | + | − |
| **DUP** | − | − | − | + |
| **LAZ** | + | − | + | + |
| **\*** | − | + | − | − |

### APP (Application)

Application node. Consumes two arguments. Port 1 (+) receives the function, Port 2 (−) receives the operand. Principal port (−) produces the result.

### LAM (Lambda)

Lambda abstraction. Produces a function. Port 1 (−) binds a variable, Port 2 (+) carries a free variable. Principal port (+) emits the abstraction.

### SUP (Superposition)

Superposition node. Fans out from one input to two outputs. Both auxiliary ports are positive. Principal port is negative.

### DUP (Duplication)

Duplication node. Copies a value. Both auxiliary ports are negative. Principal port is positive.

### LAZ (Lazy)

Lazy thunk node. Delays evaluation. Same port polarities as LAM: Port 1 (−), Port 2 (+). Principal port is positive.

### \* (Negation placeholder)

Placeholder node that can be instantiated as APP, OPX, or OPY. All three share the same polarities: node polarity −, Port 1 +, Port 2 −.

## Non-Triangle Nodes

These nodes have no auxiliary port distinction — only a node-level polarity.

| Node | Node polarity |
|---|---|
| **ERA** | − |
| **NUL** | + |
| **#** | + |
| **SUB** | − |
| **OPX** | − |
| **OPY** | − |

### ERA (Erasure)

Erasure point. Consumes and discards its input. Negative polarity.

### NUL (Null)

Null/absorber node. Terminal value. Positive polarity.

### # (Number)

Number literal. Positive polarity.

### SUB (Substitution)

Substitution node. Negative polarity.

### OPX / OPY (Operations)

Computational operations. Both have node polarity −, Port 1 +, Port 2 −. OPX is the X-flavored operation, OPY the Y-flavored variant.

## Variables

Variables (a, b, x, y) are neutral ports that carry terms. They are not connectives and do not have polarity themselves. Their effective polarity in a given context is determined by the node they connect to.

## Interaction Patterns

The system defines interaction patterns in `interactions.dot`, organized into subgraphs. Each subgraph represents a pair of interaction types and contains two rows of patterns.

### APP/LAM

Application interacts with lambda abstraction.

- **Row 1**: APP receives inputs on both ports and produces to LAM's principal port; LAM fans out through its auxiliary ports to outputs.
- **Row 2**: Direct connection from inputs to outputs (reduced form).

### ERA/LAM

Erasure interacts with lambda abstraction.

- **Row 1**: ERA connects to LAM's principal port; LAM's auxiliary ports produce x and y.
- **Row 2**: ERA connects to x; NUL connects to y. Variables are preserved across the interaction.

### APP/NUL

Application produces a null result.

- **Row 1**: APP consumes inputs and produces NUL.
- **Row 2**: One input passes through to NUL, the other is erased.

### SUB/NUL

Substitution produces a null result.

- **Row 1**: SUB consumes inputs and produces NUL.
- **Row 2**: One input passes through to NUL, the other is erased.

### OP/NUL

Operation produces a null result.

- **Row 1**: OP consumes inputs and produces NUL.
- **Row 2**: One input passes through to NUL, the other is erased.

### OPXNUM / OPYNUM

Operations with number literals.

- **OPXNUM**: OPX consumes two inputs and produces a number. OPY consumes a number and an input, producing an output.
- **OPYNUM**: OPY consumes a number and an input, producing a new number. A separate pattern combines two numbers with an operation.

### NEGSUP / NEGSUP1 / NEGSUP2

Negation (via \*) interacting with superposition.

- **NEGSUP**: \* connects to SUP's principal port; SUP fans out. A complex branching pattern involves LAZ, DUP, and multiple \* nodes.
- **NEGSUP1**: \* → SUP with one NUL output and a simplified branching form.
- **NEGSUP2**: \* → SUP with the other output as NUL and a simplified form.

### ERASUP (ERA/SUP)

Erasure interacting with superposition.

- **Row 1**: ERA connects to SUP's principal port; SUP fans out to `x` and `y`.
- **Row 2**: ERA fans out to both `x` and `y` (both outputs go through erasure).

### DUPNUL (DUP/NUL)

Duplication interacting with null.

- **Row 1**: DUP receives inputs and produces NUL.
- **Row 2**: Both inputs go directly to separate NUL nodes (each input is individually absorbed).

### DUPNUM (DUP/NUM)

Duplication interacting with number literals.

- **Row 1**: DUP receives inputs and produces a number `#`.
- **Row 2**: Both inputs go to separate number outputs `#` (duplication produces two copies).

### DUPLAM (DUP/LAM)

Duplication interacting with lambda abstraction.

- **Row 1**: DUP receives inputs and produces to LAM's principal port; LAM fans out to `x` and `y`.
- **Row 2**: Both inputs connect to separate LAM abstractions; each LAM fans through LAZ nodes to DUP and SUP, producing `x` and `y` through a complex branching pattern.

### DUPSUP (DUP/SUP)

Duplication interacting with superposition (two variants).

- **Row 1** (both): DUP receives inputs and produces to SUP's principal port; SUP fans out to `x` and `y`.
  - First variant labels DUP with `n`, second with `m`.
- **Row 2** (both): Inputs connect to SUP nodes which fan through LAZ→DUP chains to produce `x` and `y` through a symmetrical branching pattern.
  - First variant labels DUP with `n`, second uses `m`/`n` labeling.

## Port Connection Summary

| Source Port | Polarity | Can Connect To |
|---|---|---|
| APP Port 1 | + | + nodes |
| APP Port 2 | − | − nodes |
| LAM Port 1 | − | − nodes |
| LAM Port 2 | + | + nodes |
| SUP Port 1 | + | + nodes |
| SUP Port 2 | + | + nodes |
| DUP Port 1 | − | − nodes |
| DUP Port 2 | − | − nodes |
| LAZ Port 1 | − | − nodes |
| LAZ Port 2 | + | + nodes |
| \* Port 1 | + | + nodes |
| \* Port 2 | − | − nodes |
| Principal (−) | − | + nodes (interaction) |
| Principal (+) | + | − nodes (interaction) |
