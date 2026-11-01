# Mini Compiler Back-End: CFG, Liveness, Constant Propagation & Dead-Code Elimination

A compiler optimization pipeline built from scratch in C++, for a small C-like
language. The front end (lexer + parser) is hand-written with Flex/Bison and
emits three-address code (TAC); the back end then runs control-flow analysis,
constant propagation, live-variable analysis, and dead-code elimination on
that TAC.



## Pipeline

```
source.c
   │  (Flex lexer + Bison parser)
   ▼
Three-Address Code (TAC)
   │  (CFG.build — leader algorithm)
   ▼
Unreachable-code elimination   (CFG reachability from entry block)
   │
Constant propagation            (fold constant expressions, substitute
   │                              known values; resets at every label,
   │                              since a label is a control-flow merge
   │                              point where a single assumed value is
   │                              no longer safe)
   ▼
CFG is rebuilt                  (constant folding can change which jump
   │                              targets are live, so the graph from
   │                              step 1 can go stale)
   ▼
Liveness analysis               (iterative backward dataflow:
   │                              IN[B] = USE[B] ∪ (OUT[B] − DEF[B]))
   ▼
Dead-code elimination           (per-instruction backward sweep using
   │                              the liveness result — removes
   │                              dead-store assignments)
   ▼
Optimized TAC
```

Each phase prints its intermediate state, so running the compiler on any
source file shows the full pipeline, not just the final answer.

## Language supported

- Declarations: `int x;`, `float x;`
- Assignment: `x = expr;`
- Arithmetic: `+ - * /`, unary minus, parentheses
- Comparisons: `== != < > <= >=` (each produces `1`/`0`)
- Control flow: `if (expr) { ... }`, `if (expr) { ... } else { ... }`,
  `while (expr) { ... }`, arbitrarily nested
- `return expr;`

## Build

Requires `g++` (C++17), `bison`, and `flex`.

```bash
make        # generates parser.tab.c/.h and lex.yy.c, then builds ./dce_compiler
```

```bash
make clean  # removes the generated/compiled files only — never your sources
```

## Run

```bash
./dce_compiler tests/test_control_flow.c
```

This prints, in order: the raw TAC, the CFG, the unreachable-code pass,
the state after constant propagation, the liveness sets for every block,
and the final optimized TAC with dead instructions removed.

## Tests

`tests/` holds small, self-contained `.c`-style sample programs used to
exercise the pipeline manually (there's no formal test harness/assertions —
each file is meant to be run and its output read):

| File | Exercises |
|---|---|
| `test_vars.c`, `test_assign_only.c`, `test_reassign.c`, `test_return_flow.c`, `test_input1.c` | straight-line code: declarations, arithmetic, reassignment |
| `test_undeclared_use.c` | a variable used before being written (`c = a + b` where `b` was never assigned) |
| `test_control_flow.c` | `if/else`, `while`, a relational condition, and a nested `if` inside a `while` |



