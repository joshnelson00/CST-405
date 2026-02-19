# Newline Support in Print Statements - Implementation Summary

## Feature Overview
Implemented string literal support in the compiler, allowing print statements to include newlines and other escape sequences.

## What Was Changed

### 1. Lexer (scanner.l)
- Added pattern to recognize string literals: `\"[^\"]*\"`
- Returns `STRING` token for quoted text
- Positioned before identifier pattern for correct precedence

### 2. Parser (parser.y)
- Added `STRING` token declaration with `<str>` type
- Updated expression rules to accept `STRING` tokens
- Calls `createStr()` to build AST nodes for strings

### 3. Abstract Syntax Tree (ast.h & ast.c)
- Added `NODE_STR` node type for string literals
- Added `str` field to AST node union
- Implemented `createStr()` function that:
  - Strips quotes from input
  - Processes escape sequences (`\n`, `\t`, `\r`, `\\`, `\"`)
  - Stores processed string in AST node

### 4. TAC Generation (tac.c)
- Added `NODE_STR` case to `generateTACExpr()`
- Returns string with quotes preserved for TAC output

### 5. MIPS Code Generation (optimizer.c)
- Modified `generateMIPSFromOptimizedTAC2()` to:
  - Scan TAC for string literals (starting with `"`)
  - Collect unique strings in data structure
  - Output `.data` section with labeled `.asciiz` directives
- Updated `TAC_PRINT` case to:
  - Detect string literals vs. numeric values
  - Use `la` (load address) + syscall 4 for strings
  - Use existing logic for numeric printing

## Example Usage

```c
int main() {
    print("Hello, World!\n");
    print("Line 1\n");
    print("Line 2\n");
    int x;
    x = 42;
    print("Value: ");
    print(x);
    print("\nDone!\n");
    return 0;
}
```

## Generated MIPS Output

```asm
.data
str_0: .asciiz "Hello, World!\n"
str_1: .asciiz "Line 1\n"
str_2: .asciiz "Line 2\n"
str_3: .asciiz "Value: "
str_4: .asciiz "\nDone!\n"

.text
.globl main
main:
    la $a0, str_0
    li $v0, 4
    syscall
    ...
```

## Supported Escape Sequences
- `\n` - Newline
- `\t` - Tab
- `\r` - Carriage return
- `\\` - Backslash
- `\"` - Quote

## Testing
Created test files:
- `test_newlines.cm` - Basic newline testing
- `test_string_features.cm` - Comprehensive test with tabs, multiple lines, mixed string/numeric output

All tests compile successfully and generate correct MIPS assembly code.
