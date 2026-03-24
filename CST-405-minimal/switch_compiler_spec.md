# Switch Statement — Compiler Implementation Specification

**Course:** CST-405  
**Scope:** Lexical analysis through intermediate code generation  
**Style:** Grammar-agnostic; implementation details are illustrative, not prescriptive

---

## 1. Overview and Purpose

A switch statement is a multi-way branch construct that dispatches control to one of several labeled code points based on the value of a single controlling expression. Its defining characteristic is that the controlling expression is evaluated **exactly once**, and its result is compared against a finite set of **compile-time integer constants** (the case labels). This is in contrast to an if-else chain, where each condition is an independent expression evaluated in sequence.

The compiler's job is to translate this high-level dispatch intent into a lower-level representation that either tests values sequentially, indexes into a jump table, or navigates a binary search tree of comparisons — depending on the number and density of case values.

---

## 2. Tokens

The following terminals must be recognized by the lexical analyzer before the parser ever sees input. The exact token names used internally are implementation-defined, but each must be distinguishable:

| Surface text | Role                                            |
|--------------|-------------------------------------------------|
| `switch`     | Begins the switch statement                     |
| `case`       | Introduces a labeled branch                     |
| `default`    | Introduces the catch-all branch                 |
| `break`      | Exits the innermost enclosing switch or loop    |
| `:`          | Separates a label from the statements that follow it |

**Ordering constraint.** These keywords must be recognized before the general identifier rule. In any scanner that processes alternatives in order (e.g., longest-match with ordered tie-breaking), a keyword rule that follows a general identifier pattern will be silently swallowed as an identifier. Keywords must take priority.

---

## 3. Abstract Syntax

### 3.1 Node Types Required

Three new node types are needed. Every existing node type in the AST remains unchanged:

**SwitchNode**  
Represents the entire switch construct. It holds:
- A reference to the controlling expression subtree (evaluated once at runtime).
- A reference to an ordered sequence of case clause nodes.

**CaseNode**  
Represents a single case or default clause. It holds:
- An integer constant value (the case label). This field is meaningful only when the node is not a default clause.
- A boolean flag distinguishing a normal case from the `default` clause.
- A reference to the body — a (possibly empty) sequence of statements that executes when this clause is selected.
- A reference to the next clause in the sequence (i.e., the case list is a linked structure, not a flat array). This `next` pointer is `null` for the final clause.

**BreakNode**  
Represents a `break` statement. It carries no payload beyond its node type. Its presence in a statement list is the only information needed; any context required for code generation (i.e., which label to jump to) is resolved during code generation using a stack, not stored in the node itself.

### 3.2 Case List Ordering

The case list must preserve **source order**. The parser should append each new clause to the **end** of the linked list, not prepend it to the front. Reversing the list at any point is functionally valid but requires care; if the list is built by prepending and then reversed, the reversal must happen before semantic analysis so that duplicate detection and fall-through traversal operate on the correct order.

### 3.3 Fall-Through Representation

A case clause with an empty body is legal and represents an intentional fall-through. The body field for such a clause is `null`. This is not an error and should not be treated as one. The clause still exists in the list as a valid target for the dispatch mechanism; its empty body simply means execution falls immediately into the next clause's body at runtime.

---

## 4. Syntactic Structure

The switch statement is syntactically composed of three nested levels:

1. **The switch wrapper** — contains the controlling expression and a brace-delimited block.
2. **The case list** — a sequence of zero or more case clauses inside the block. An empty case list (a switch with no cases) is syntactically valid, though semantically a no-op.
3. **Each case clause** — a label (`case <constant>:` or `default:`), followed by zero or more statements. Statements inside a clause are terminated naturally when the parser encounters another `case` keyword, the `default` keyword, or the closing brace of the switch block. An explicit `break` is a statement like any other; it is not required by the grammar.

**Critical parser note.** The statement rule that collects statements inside a clause body must recognize `case`, `default`, and `}` as terminators — not as valid statement starters. In most bottom-up parsers, this falls out naturally because none of those tokens can begin a statement production. In top-down parsers (recursive descent), the statement-list parsing function must explicitly check for these tokens before attempting to parse another statement.

**`break` as an ordinary statement.** `break` is a statement alternative in the statement production, at the same level as assignment, function call, return, and so on. It is not special syntactically — only semantically. Treating it as a special case at the parser level is an error that can produce grammar conflicts.

**`default` placement.** The grammar must allow `default` to appear anywhere in the case list — beginning, middle, or end. Restricting `default` to appear only last is a semantic policy, not a syntactic one; enforcing it in the grammar overconstrains and may reject valid programs.

---

## 5. Semantic Constraints

The following rules must be enforced after the AST is fully built (or incrementally during a post-parse AST walk). Violations are compile-time errors.

### 5.1 Controlling Expression Type

The controlling expression must resolve to an integral type. Accepted types are integer types and character types (which are integer types at the machine level). Types that are not acceptable include floating-point types and pointer-to-character (string) types. If the controlling expression has a non-integral type, report a type error at the location of the switch keyword.

### 5.2 Case Value Must Be a Compile-Time Constant

Every `case` label value must be a compile-time integer constant expression — a numeric literal, a character literal, or an enum member. A case value that references a variable, even a `const`-qualified one in C, is not legal in C99/C11. The semantic analyzer must verify that the value field in each CaseNode was produced from a constant expression during parsing, or re-evaluate it during semantic analysis.

### 5.3 No Duplicate Case Values

Within a single switch, no two case labels may carry the same integer value. This includes values that are textually different but numerically equal (e.g., `case 10:` and `case '\n':` are the same value, 10, and cannot coexist).

Implementation strategy: collect all case values into a set as the case list is traversed. On each new case label, check membership before inserting. Report the first duplicate found as an error, including both the current location and the location of the original. At most one `default` clause is permitted per switch.

### 5.4 `break` Scope Validation

A `break` statement is only valid when it appears lexically inside a switch statement or a loop construct. It is not valid at top-level or inside a plain block that is not part of a control structure. The semantic analyzer must track a "break-valid" context (a counter or flag pushed on entry to a switch or loop and popped on exit). If a `break` node is encountered while the counter is zero, report an error.

**Important nested-switch nuance.** A `break` inside a switch that is itself inside a loop exits the **switch**, not the loop. This is purely a code generation concern, not a semantic error. The semantic analyzer only needs to confirm that `break` appears inside *some* enclosing switch or loop; the code generator resolves which specific label to jump to via the break-label stack described in Section 7.

### 5.5 Default Clause

The `default` clause is optional. Its absence is not an error. If no case matches at runtime and no default clause exists, the entire switch body is skipped. The semantic analyzer should neither require nor forbid the default clause; whether to emit a warning for its absence is a quality-of-implementation decision, not a correctness requirement.

---

## 6. Intermediate Representation (Three-Address Code)

The standard translation for a switch statement into TAC (Three-Address Code) uses a **linear scan**: one conditional branch instruction per case, followed by an unconditional jump to the default label (or to the end label if no default exists). This is O(n) in the number of cases but is correct for all inputs and straightforward to generate.

### 6.1 Linear Scan Pattern

Given a switch on expression `e` with cases `c₁, c₂, ..., cₙ`, a default, and a break-exit label `L_end`:

```
t₀ = <evaluate controlling expression e>

IF t₀ != c₁ GOTO L_test_2
L_body_1:
  <body of case 1>
  GOTO L_end              ; generated for break; omitted if fall-through is intended

L_test_2:
IF t₀ != c₂ GOTO L_test_3
L_body_2:
  <body of case 2>
  GOTO L_end

  ...

L_test_n:
IF t₀ != cₙ GOTO L_default
L_body_n:
  <body of case n>
  GOTO L_end

L_default:
  <body of default>
  GOTO L_end

L_end:
```

**Notes on this pattern:**
- The controlling expression is assigned to a temporary **before** any branch. This ensures it is evaluated exactly once even if the expression has side effects.
- Each `GOTO L_end` is emitted in place of a `break` node. If a case clause has no `break` (fall-through), no unconditional jump is emitted at the end of the body, and execution falls naturally into the next body.
- The `default` clause, if present, is placed at the point where all case tests have failed. If `default` appears in the source at a position other than last, the generated code still places the default body after all dispatch tests, not inline at its source position. (Alternatively, the dispatch code can jump to wherever `default` appears; either is correct.)
- If no `default` is present, the final `GOTO L_default` in the dispatch chain becomes `GOTO L_end`.

### 6.2 Break Label Stack

The code generator must maintain a stack of "active break labels." On entry to each switch statement, push `L_end` (the label immediately after the switch body) onto the stack. On exit from the switch, pop it. When a `break` node is encountered, emit `GOTO <top of stack>`.

This mechanism generalizes correctly to nested switches and to switches nested inside loops: each construct pushes its own exit label, and `break` always resolves to the innermost one.

### 6.3 Alternative Dispatch Strategies (Not Required, but Described for Completeness)

A compiler may choose among three strategies based on the number and density of case values:

**Linear scan** (described above): Always correct; preferred for small case counts (fewer than ~5) or when simplicity is required.

**Jump table**: When case values form a dense integer range (density ≥ ~50% of the range `[min_case, max_case]`), an array of target addresses can be allocated. At runtime, the controlling expression is used directly as an array index (with bounds checking) to load the target address and perform an indirect jump. This achieves O(1) dispatch. The table must contain an entry for every integer in the range, with gaps pointing to the default label. Bounds checking — verifying the value is within `[min_case, max_case]` before indexing — is mandatory.

**Binary search tree**: When there are many sparse cases, a decision tree of comparisons can reduce average dispatch to O(log n) without requiring a contiguous address table. This is generated as a balanced tree of TAC comparisons, with each internal node testing the midpoint of the remaining case range.

---

## 7. Code Generation (Target Assembly)

The TAC patterns above map onto target assembly (e.g., MIPS, x86, ARM) as follows. The patterns are described abstractly:

**Controlling expression evaluation**: Load the result of the controlling expression into a register. This register must remain live (not reused or clobbered) for the entire duration of the dispatch chain.

**Case test**: Compare the expression register against an immediate constant (the case value). If they are not equal, branch forward to the next test label. If they are equal, fall through into the case body.

**Break (unconditional exit)**: Emit an unconditional jump to the switch's exit label. Since this is a forward reference at the time the case body is generated, the exit label must be allocated before the case bodies are emitted and backpatched or resolved at the end.

**Label at merge points**: At every case label and at the exit label, the register allocator must treat the location as a potential merge point — execution may arrive here from multiple predecessors. Any temporaries live across a case boundary must be spilled to memory and reloaded, not assumed to be in registers. This is the same treatment as the merge point at the end of an if-statement.

**Nested switch**: Each nested switch is an independent instance of all the above. The inner switch's `L_end` is a different label from the outer switch's `L_end`. The break-label stack ensures the correct one is used.

---

## 8. Full Compiler Phase Checklist

The following summarizes what must change in each phase of a multi-phase compiler. No phase is exempt:

**Lexical analysis**: Recognize `switch`, `case`, `default`, `break`, and `:` as distinct tokens. Keywords must have higher priority than the general identifier rule.

**Syntax analysis**: Add grammar rules to recognize the switch statement form, the case list, individual case and default clauses, an optional statement list per clause (which may be empty), and `break` as a statement alternative. The grammar must be conflict-free; verify this with your parser generator. The statement rule that handles clause bodies must be bounded by the presence of `case`, `default`, or `}`.

**AST construction**: Instantiate `SwitchNode`, `CaseNode`, and `BreakNode` in the parser actions. Build the case list by appending to the tail to preserve source order. Initialize `next` pointers to `null` in each `CaseNode` constructor; the parser sets them during list construction.

**Semantic analysis**: Validate the controlling expression type (integral only); validate each case value is a compile-time constant; check for duplicate case values across the entire switch using a set; validate `break` only appears inside an active switch or loop context; optionally warn on missing `default`.

**TAC / IR generation**: Evaluate the controlling expression into a temporary. Emit a linear scan of `IF_FALSE` instructions, one per case. Emit unconditional jumps for `break` using the break-label stack. Emit the default or end label at the conclusion of the dispatch chain. Emit case body labels at the corresponding points in the body sequence.

**Target code generation**: Lower each TAC switch instruction to compare-and-branch sequences. Spill live registers at every label (merge point). Resolve all forward references to the switch exit label.

---

## 9. Edge Cases and Pitfalls

**Fall-through without a body.** A case clause whose body is null (empty) is a valid fall-through group leader. The dispatch code must still emit a test for this case's value and jump to its label; it just so happens that no statements follow before the next case begins. This is how `case 1: case 2: <body>` works — both values dispatch to the same code.

**Duplicate case values.** Two `case 5:` labels in the same switch is a semantic error, not a parse error. The grammar allows any sequence of case clauses; uniqueness is a semantic constraint. Detect it during semantic analysis.

**Variable declarations in case bodies without a nested block.** In C, declaring a variable inside a case body (without wrapping it in braces) creates a scope issue: if a different case is selected at runtime, the variable's declaration is skipped but its scope still begins. This is a subtlety for the semantic analyzer. The safe behavior is to require that declarations inside a switch appear inside their own brace-delimited nested block.

**`break` exits the switch, not any enclosing loop.** A `break` inside a switch that is nested inside a loop only exits the switch. This is correctly handled by the break-label stack — each construct pushes only its own exit label; `break` resolves to the top of the stack, which is the innermost construct.

**Non-integral controlling expression.** Attempting to switch on a `float`, `double`, or pointer-to-char is a type error. The semantic analyzer must reject this before TAC generation is attempted.

**`default` anywhere in the case list.** The C standard permits `default` to appear anywhere — first, last, or in the middle of the case list. The TAC dispatch logic must handle this: the `default` body can be placed at any position in the emitted code, but the dispatch chain must still ensure all case tests happen before falling through to the default. A simple implementation always emits the default body at the end of the dispatch chain regardless of its source position.

**No cases at all.** A switch with an empty case list is syntactically legal. The controlling expression is still evaluated (for side effects), but no branches are emitted and execution falls directly to `L_end`. This is a no-op and may merit a warning.

---

## 10. Summary of Invariants

The following invariants must hold at all times through the compilation pipeline:

1. The controlling expression is evaluated into a temporary exactly once, before any dispatch.
2. Every case value is an integer constant known at compile time; no case value is computed at runtime.
3. No two case clauses in the same switch carry the same integer value.
4. At most one `default` clause exists per switch.
5. Every `break` node appears within the lexical scope of at least one switch or loop.
6. A `break` at runtime exits exactly the innermost enclosing switch or loop — no more.
7. Fall-through is the default behavior in the absence of `break`; it is not an error.
8. The case list in the AST preserves source order.
9. At every label that can be reached from multiple predecessors (every case label and the exit label), register-allocated temporaries are treated as potentially invalid and are reloaded from memory.
