# MiniC Compiler Implementation Guide

## Overview

This document provides a comprehensive guide to understanding and implementing the MiniC compiler, which compiles a subset of C-like language to MIPS assembly. The compiler follows a traditional multi-phase architecture and generates MIPS assembly code that adheres to the standard MIPS ABI.

## Table of Contents

1. [Compiler Architecture](#compiler-architecture)
2. [Phase-by-Phase Implementation](#phase-by-phase-implementation)
3. [MIPS ABI and Register Usage](#mips-abi-and-register-usage)
4. [Data Structures and Key Components](#data-structures-and-key-components)
5. [Implementation Guidelines](#implementation-guidelines)
6. [Common Issues and Solutions](#common-issues-and-solutions)
7. [Testing and Validation](#testing-and-validation)

## Compiler Architecture

The MiniC compiler follows a classic multi-phase compilation pipeline:

```
Source Code (.cm) → Lexer → Parser → AST → TAC → Optimizer → MIPS Code → .s file
```

### Phase 1: Lexical Analysis (Scanner)
- **File**: `scanner.l` (Flex specification)
- **Purpose**: Tokenizes the input source code
- **Key Tokens**: `int`, `float`, `void`, identifiers, numbers, operators, punctuation
- **Implementation**: Regular expressions define token patterns

### Phase 2: Syntax Analysis (Parser)
- **File**: `parser.y` (Bison specification)
- **Purpose**: Parses tokens into Abstract Syntax Tree (AST)
- **Grammar**: Defines language syntax for declarations, expressions, statements, functions
- **Output**: AST structure representing program hierarchy

### Phase 3: Semantic Analysis
- **Files**: `symtab.c`, `ast.c`
- **Purpose**: Type checking, symbol table management
- **Key Functions**:
  - `addVar()`: Adds variables to symbol table
  - `getVarOffset()`: Gets stack offset for variables
  - Type checking for expressions and assignments

### Phase 4: Intermediate Code Generation
- **File**: `tac.c`
- **Purpose**: Converts AST to Three-Address Code (TAC)
- **TAC Instructions**: Simple operations with at most 3 operands
- **Key Functions**:
  - `generateTAC()`: Main TAC generation function
  - `generateTACExpr()`: Handles expression generation
  - `newTemp()`: Creates temporary variables

### Phase 5: Code Optimization
- **File**: `optimizer.c`
- **Purpose**: Optimizes TAC and generates MIPS code
- **Optimizations**: Constant folding, copy propagation, dead code elimination
- **Key Functions**:
  - `optimizeTAC()`: Applies optimizations to TAC
  - `generateMIPSFromOptimizedTAC()`: Converts optimized TAC to MIPS

### Phase 6: Code Generation
- **Files**: `mips.c`, `optimizer.c`
- **Purpose**: Generates final MIPS assembly code
- **Output**: `.s` file containing MIPS assembly

## Phase-by-Phase Implementation

### Phase 1: Lexical Analysis Implementation

The lexer uses Flex to tokenize the input. Key implementation details:

```flex
%{
#include "parser.tab.h"
%}

/* Regular expressions */
DIGIT    [0-9]
ID       [a-zA-Z][a-zA-Z0-9]*
NUMBER   {DIGIT}+
FLOAT    {DIGIT}+"."{DIGIT}+

/* Token rules */
{NUMBER}    { yylval.num = atoi(yytext); return NUM; }
{FLOAT}     { yylval.flt = atof(yytext); return FLT; }
{ID}        { yylval.var.name = strdup(yytext); return VAR; }
"+"         { return '+'; }
";"         { return ';'; }
```

**Implementation Guidelines:**
1. Each token returns a terminal symbol defined in the parser
2. Semantic values (`yylval`) carry token information to the parser
3. Handle whitespace and comments appropriately

### Phase 2: Syntax Analysis Implementation

The parser uses Bison to build AST nodes:

```bison
%union {
    int num;
    float flt;
    char* var;
    ASTNode* node;
}

%token <num> NUM
%token <flt> FLT
%token <var> VAR

%%

program: stmt_list { $$ = $1; }

stmt_list: stmt_list stmt { 
    $$ = createASTNode(NODE_STMT_LIST, $1, $2, NULL); 
}
          | stmt { $$ = $1; }

stmt: decl_stmt | assign_stmt | print_stmt | func_def

decl_stmt: type VAR ';' {
    $$ = createASTNode(NODE_DECL, NULL, NULL, NULL);
    $$->data.var.name = $2;
    $$->data.var.type = $1;
}
```

**Key AST Node Types:**
- `NODE_DECL`: Variable declarations
- `NODE_ASSIGN`: Assignment statements
- `NODE_BINOP`: Binary operations (+, -, *, /)
- `NODE_PRINT`: Print statements
- `NODE_FUNC`: Function definitions
- `NODE_FUNC_CALL`: Function calls

### Phase 3: Semantic Analysis Implementation

The symbol table manages variable information:

```c
typedef struct {
    char* name;
    VarType type;
    int offset;
    int is_param;
} SymbolEntry;

typedef struct {
    SymbolEntry entries[100];
    int count;
    int next_offset;
} SymbolTable;

// Key functions
int addVar(char* name, VarType type);
int getVarOffset(char* name);
VarType getVarType(char* name);
```

**Stack Frame Layout:**
```
High addresses
+------------------+
|   Previous FP    | 0($fp)
+------------------+
|   Return Addr    | -4($fp)
+------------------+
|   Saved Regs     | -8($fp), -12($fp), ...
+------------------+
|   Local 1        | -16($fp)
+------------------+
|   Local 2        | -20($fp)
+------------------+
|   ...            |
+------------------+
Low addresses
```

### Phase 4: Intermediate Code Generation

TAC represents operations in a simple, uniform format:

```c
typedef enum {
    TAC_ADD, TAC_SUBTRACT, TAC_MULTIPLY, TAC_DIVIDE,
    TAC_ASSIGN, TAC_PRINT, TAC_FUNC_DEF, TAC_FUNC_CALL,
    TAC_PARAM, TAC_RETURN, TAC_DECL, TAC_ARG
} TACOp;

typedef struct TACInstr {
    TACOp op;
    char* arg1;
    char* arg2; 
    char* result;
    struct TACInstr* next;
} TACInstr;
```

**TAC Instruction Examples:**
```
t0 = x + y        // TAC_ADD
result = t0       // TAC_ASSIGN
PRINT result       // TAC_PRINT
CALL add           // TAC_FUNC_CALL
RETURN result      // TAC_RETURN
```

### Phase 5: Code Optimization

The optimizer applies several transformations:

**Constant Folding:**
```
t0 = 5 + 3    →    t0 = 8
```

**Copy Propagation:**
```
t0 = x
y = t0        →    y = x
```

**Dead Code Elimination:**
```
x = 5          // Never used → eliminated
```

### Phase 6: MIPS Code Generation

The final phase generates MIPS assembly following the standard ABI.

## MIPS ABI and Register Usage

### Register Classification

**Argument Registers ($a0-$a3):**
- Used for passing function arguments
- First argument in $a0, second in $a1, etc.
- Caller-saved (can be overwritten by called function)

**Return Value Register ($v0-$v1):**
- Used for function return values
- Primary return value in $v0
- Caller-saved

**Temporary Registers ($t0-$t9):**
- For temporary calculations and expressions
- Caller-saved (caller must preserve if needed)
- Can be used freely within functions

**Saved Registers ($s0-$s7):**
- For values that must survive across function calls
- Callee-saved (called function must preserve)
- Use for local variables that persist

**Special Registers:**
- `$sp`: Stack pointer
- `$fp`: Frame pointer
- `$ra`: Return address
- `$zero`: Constant zero

### MIPS ABI Function Call Convention

**Function Prologue:**
```mips
func_name:
    # No stack allocation needed for simple functions
    # If locals exist: addi $sp, $sp, -N
    # If using $s registers: save them
    # If nested calls: save $ra
```

**Function Epilogue:**
```mips
    # Restore saved registers if used
    # Restore $ra if saved
    # Restore stack if allocated: addi $sp, $sp, N
    jr $ra
```

**Function Call Sequence:**
```mips
    # Set up arguments
    li $a0, 5      # First argument
    li $a1, 3      # Second argument
    
    # Make the call
    jal function_name
    
    # Return value is in $v0
    move $t0, $v0  # Save return value if needed
```

### Stack Frame Management

**When Stack is Needed:**
- Function has local variables
- Function calls other functions (needs to save $ra)
- Function uses $s registers
- Function needs more than 4 arguments

**Stack Frame Size Calculation:**
```
frame_size = (num_locals * 4) + (num_saved_regs * 4) + (ra_saved ? 4 : 0)
```

**Proper Stack Usage:**
```mips
# Allocate stack space
addi $sp, $sp, -frame_size

# Save registers if needed
sw $ra, 4($sp)
sw $s0, 8($sp)

# Function body...

# Restore registers
lw $s0, 8($sp)
lw $ra, 4($sp)

# Deallocate stack
addi $sp, $sp, frame_size
jr $ra
```

## Data Structures and Key Components

### AST Node Structure

```c
typedef enum {
    NODE_NUM, NODE_FLT, NODE_VAR, NODE_BINOP,
    NODE_ASSIGN, NODE_PRINT, NODE_DECL,
    NODE_FUNC, NODE_FUNC_CALL, NODE_PARAM,
    NODE_RETURN, NODE_STMT_LIST, NODE_ARG_LIST
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        int num;
        float flt;
        struct { char* name; VarType type; } var;
        struct { ASTNode* left; ASTNode* right; char op; } binop;
        struct { char* var; ASTNode* value; } assign;
        struct { ASTNode* expr; } print;
        struct { char* name; ASTNode* params; ASTNode* body; struct ASTNode* next; } func;
        struct { char* name; ASTNode* args; } func_call;
        struct { char* name; struct ASTNode* next; } param;
        struct { ASTNode* expr; } return_stmt;
        struct { ASTNode* stmt; struct ASTNode* next; } stmtlist;
        struct { ASTNode* arg; struct ASTNode* next; } arg_list;
    } data;
    struct ASTNode* next;
} ASTNode;
```

### TAC Instruction Structure

```c
typedef struct TACInstr {
    TACOp op;
    char* arg1;
    char* arg2;
    char* result;
    struct TACInstr* next;
} TACInstr;

typedef struct {
    TACInstr* head;
    TACInstr* tail;
    int tempCount;
} TACList;
```

### MIPS Instruction Structure

```c
typedef enum {
    MIPS_LI, MIPS_LW, MIPS_SW, MIPS_ADD, MIPS_SUB,
    MIPS_MUL, MIPS_DIV, MIPS_MFLO, MIPS_ADDI,
    MIPS_MOVE, MIPS_JAL, MIPS_JR, MIPS_SYSCALL,
    MIPS_LABEL, MIPS_COMMENT
} MIPSOp;

typedef struct MIPSInstr {
    MIPSOp op;
    char* result;
    char* arg1;
    char* arg2;
    char* comment;
    struct MIPSInstr* next;
} MIPSInstr;
```

## Implementation Guidelines

### 1. Function Implementation Strategy

**Simple Functions (no locals, no nested calls):**
```mips
add:
    add $v0, $a0, $a1    # Compute result
    jr $ra               # Return
```

**Functions with Locals:**
```mips
function:
    addi $sp, $sp, -8    # Allocate space for 2 locals
    # ... function body using stack offsets ...
    addi $sp, $sp, 8     # Restore stack
    jr $ra
```

**Functions with Nested Calls:**
```mips
caller:
    addi $sp, $sp, -4    # Space to save $ra
    sw $ra, 0($sp)      # Save return address
    jal callee          # Make nested call
    lw $ra, 0($sp)      # Restore return address
    addi $sp, $sp, 4     # Restore stack
    jr $ra
```

### 2. Expression Evaluation

**Binary Operations:**
```c
// Generate: result = left + right
case TAC_ADD:
    // If both operands are constants
    if (isConst(left) && isConst(right)) {
        int result = atoi(left) + atoi(right);
        // Generate: li $v0, result
    } else {
        // Load operands into registers
        // Generate: add $v0, $reg1, $reg2
    }
```

### 3. Memory Management

**Variable Storage:**
- **Arguments**: Passed in $a0-$a3, no stack storage needed
- **Locals**: Stored on stack at negative offsets from $fp
- **Temporaries**: Kept in $t registers, no memory storage needed
- **Return values**: Always in $v0

### 4. Type Handling

**Integer Operations:**
```mips
add $v0, $a0, $a1    # Integer addition
sub $v0, $a0, $a1    # Integer subtraction
mult $a0, $a1         # Integer multiplication
mflo $v0             # Get result from LO register
div $a0, $a1         # Integer division
mflo $v0             # Get quotient
```

**Float Operations:**
```mips
add.s $f0, $f12, $f14 # Float addition
sub.s $f0, $f12, $f14 # Float subtraction
mul.s $f0, $f12, $f14 # Float multiplication
div.s $f0, $f12, $f14 # Float division
```

## Common Issues and Solutions

### 1. Stack Corruption

**Problem:** Functions don't restore stack properly
```mips
# WRONG
function:
    addi $sp, $sp, -8
    # ... function body ...
    jr $ra  # Missing stack restoration!
```

**Solution:** Always balance stack operations
```mips
# CORRECT
function:
    addi $sp, $sp, -8
    # ... function body ...
    addi $sp, $sp, 8   # Restore stack
    jr $ra
```

### 2. Register Clobbering

**Problem:** Using callee-saved registers without preserving
```mips
# WRONG
function:
    move $s0, $a0     # Using $s0 without saving
    # ... call another function ...
    jr $ra             # $s0 value lost!
```

**Solution:** Save/restore callee-saved registers
```mips
# CORRECT
function:
    addi $sp, $sp, -4
    sw $s0, 0($sp)     # Save $s0
    move $s0, $a0
    # ... call another function ...
    lw $s0, 0($sp)     # Restore $s0
    addi $sp, $sp, 4
    jr $ra
```

### 3. Return Address Issues

**Problem:** Nested calls without saving $ra
```mips
# WRONG
caller:
    jal callee         # $ra gets overwritten
    # ... more code ...
    jr $ra             # Wrong return address!
```

**Solution:** Save $ra before nested calls
```mips
# CORRECT
caller:
    addi $sp, $sp, -4
    sw $ra, 0($sp)     # Save $ra
    jal callee
    lw $ra, 0($sp)     # Restore $ra
    addi $sp, $sp, 4
    jr $ra
```

### 4. Incorrect Register Usage

**Problem:** Using wrong registers for specific purposes
```mips
# WRONG
add $t0, $a0, $a1    # Result in $t0 instead of $v0
move $v0, $t0        # Unnecessary move
jr $ra
```

**Solution:** Use correct registers directly
```mips
# CORRECT
add $v0, $a0, $a1    # Result directly in $v0
jr $ra
```

## Testing and Validation

### 1. Unit Testing Strategy

**Test Individual Components:**
- Lexer: Test token recognition
- Parser: Test AST construction
- TAC Generator: Test intermediate code generation
- Optimizer: Test optimization passes
- MIPS Generator: Test assembly output

### 2. Integration Testing

**Test Complete Programs:**
```c
// Test basic arithmetic
int main() {
    int x = 5 + 3;
    print(x);
}

// Test function calls
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5, 3);
    print(result);
}
```

### 3. MIPS Simulation Testing

**Using SPIM:**
```bash
spim -file program.s
```

**Expected Output:**
```
8
```

### 4. Validation Checklist

- [ ] All functions have proper `jr $ra`
- [ ] Stack is balanced in every function
- [ ] Arguments passed in $a0-$a3
- [ ] Return values in $v0
- [ ] No memory leaks in data structures
- [ ] Register usage follows ABI conventions
- [ ] No fall-through between functions
- [ ] Proper handling of edge cases

## Advanced Topics

### 1. Register Allocation

**Graph Coloring Algorithm:**
1. Build interference graph
2. Color graph with available registers
3. Spill to memory if necessary

### 2. Advanced Optimizations

**Loop Optimizations:**
- Loop unrolling
- Induction variable elimination
- Strength reduction

**Memory Optimizations:**
- Common subexpression elimination
- Dead store elimination
- Register renaming

### 3. Error Handling

**Compile-time Errors:**
- Syntax errors with line numbers
- Type mismatches
- Undefined variables

**Runtime Errors:**
- Stack overflow detection
- Division by zero handling
- Null pointer checks

## Conclusion

This guide provides a comprehensive foundation for implementing and extending the MiniC compiler. The key principles to remember are:

1. **Follow the MIPS ABI strictly** - Proper register usage and calling conventions
2. **Maintain stack discipline** - Always balance push/pop operations
3. **Use appropriate data structures** - AST, TAC, and symbol tables
4. **Test thoroughly** - Unit tests, integration tests, and MIPS simulation
5. **Handle edge cases** - Empty programs, error conditions, optimization boundaries

The compiler architecture is modular and extensible, allowing for additional language features, optimizations, and target architectures. Each phase has well-defined responsibilities and interfaces, making the system maintainable and understandable.

For further development, consider adding support for:
- Control flow (if/else, while, for)
- Arrays and pointers
- More advanced optimizations
- Additional target architectures
- Better error reporting and debugging support
