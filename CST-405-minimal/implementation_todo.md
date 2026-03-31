# Compiler Implementation TODO Roadmap

## Completed in this pass

1. Boolean and char lexical/parsing support (int-backed)
- Scanner: added `boolean`, `char`, `true`, `false`, and `'x'` literal handling.
- Parser: added `BOOLEAN`/`CHAR` tokens to declarations, parameters, and function return types.
- AST/Symbol/TAC/MIPS: reused existing int pipeline (no new runtime type needed).
- Validation: compiled + executed multiple tests via SPIM.

2. Forward function calls in single-pass parse
- Symbol validation: updated function-call validation to allow unresolved calls during early parse.
- Parser/AST/TAC/MIPS: no structural changes needed.
- Validation: `main` can call helper functions defined later in the file.

3. `test_final.c` grammar alignment
- Removed unsupported `string` typed variables in favor of direct string literal prints.
- Reworked increment section to avoid non-existent true global variable semantics.
- Fixed float assignment expression to match strict type-check behavior.

## Remaining items (prioritized)

1. True global variables
- Scanner: no change.
- Parser: add top-level global declaration list before function list.
- AST: add node(s) for global declarations.
- Symbol table: maintain global var scope distinct from function-local scope.
- TAC: emit global storage and global load/store operations.
- MIPS: place globals in `.data` and generate correct addressing.
- Tests:
  - global read/write from `main`
  - global mutation inside helper function
  - global arrays

2. First-class string variables (`string x; x = "..."; print(x);`)
- Scanner: add `string` keyword token.
- Parser: declaration and assignment rules for string type.
- AST: represent string-typed variable declarations and assignments.
- Symbol table: add `TYPE_STRING`.
- TAC: add string assignment/move semantics.
- MIPS: store/load string addresses, print via syscall 4, include newline behavior consistency.
- Tests:
  - string literal assignment
  - string variable print
  - string arrays (optional advanced)

3. Char output semantics (print character vs ASCII int)
- Scanner: already supports char literals.
- Parser/AST: already int-backed.
- Symbol table: optionally add `TYPE_CHAR` for print-dispatch.
- TAC/MIPS: if `TYPE_CHAR`, print via syscall 11 instead of syscall 1.
- Tests:
  - `char c; c = 'A'; print(c);` should output `A` not `65`.

4. Better function-call validation with forward declarations
- Scanner/Parser: optional support for function prototypes.
- Symbol table: store unresolved call list and verify after full parse.
- TAC/MIPS: no major change.
- Tests:
  - valid forward call
  - typo function name should fail at end-of-parse
  - wrong arg count for forward-declared function

5. Numeric coercion policy improvements
- Scanner: no change.
- Parser/semantic: support controlled implicit int->float promotion in assignments/returns.
- AST/TAC: annotate cast nodes or emit conversion TAC.
- MIPS: emit conversion instructions systematically.
- Tests:
  - `float r = a / b;`
  - mixed arithmetic expressions and return values.

## Suggested execution order
1. True global variables
2. String variable type
3. Char print semantics
4. Post-parse unresolved function validation
5. Coercion/cast cleanup
