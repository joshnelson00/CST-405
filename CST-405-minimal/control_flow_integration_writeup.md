# Control-Flow and Boolean Integration Writeup

## 1. Scope and Goal

This document describes the team's integration approach, design decisions, and implementation challenges for:

- if (then)
- if-then-else
- nested if statements
- switch/case/default/break
- boolean decision behavior (comparison-based truth values)

The analysis covers the full compiler pipeline: scanner, parser/semantic checks, AST, TAC, optimization, and backend code generation.

## 2. End-to-End Integration Approach

The implementation follows a staged compiler strategy where each phase adds only the responsibility it should own:

1. Scanner recognizes control-flow and comparison tokens.
2. Parser builds structured AST nodes and resolves grammar ambiguity (dangling else).
3. Semantic checks validate switch constraints and break scoping.
4. AST provides stable node types and linked case representation.
5. TAC lowers control flow into labels, conditional branches, and gotos.
6. Optimizer improves branch behavior while preserving semantics.
7. Backend emits MIPS branch and compare instructions from optimized TAC.

This phase-by-phase layering reduced coupling and made control-flow behavior testable at each intermediate form (AST, TAC, optimized TAC, MIPS).

## 3. Scanner Integration

### Design Decisions

- Added explicit keyword rules for if/else/switch/case/default/break before identifier rules.
- Added comparison operator tokenization (==, !=, <, >, <=, >=).
- Added logical operator tokenization (&&, ||, !).
- Kept boolean behavior expression-based (comparison results), not a separate bool token/type.

### Challenges

- Keyword priority had to be correct so control-flow words were not lexed as identifiers.
- Operator ordering had to avoid ambiguity between ! and !=, and ensure two-character operators (&&, ||, ==, !=, <=, >=) are matched correctly.

### Code Changes (2 chunks)

```lex
"if"            { yycolumn += yyleng; return IF; }
"else"          { yycolumn += yyleng; return ELSE; }
"switch"        { yycolumn += yyleng; return SWITCH; }
"case"          { yycolumn += yyleng; return CASE; }
"default"       { yycolumn += yyleng; return DEFAULT; }
"break"         { yycolumn += yyleng; return BREAK; }
```

```lex
"=="            { yycolumn += yyleng; return EQ; }
"!="            { yycolumn += yyleng; return NE; }
"<="            { yycolumn += yyleng; return LE; }
">="            { yycolumn += yyleng; return GE; }
"&&"            { yycolumn += yyleng; return AND; }
"||"            { yycolumn += yyleng; return OR; }
"!"             { yycolumn++; return NOT; }
"<"             { yycolumn++; return LT; }
">"             { yycolumn++; return GT; }
```

## 4. Parser and Semantic Integration

### Design Decisions

- Used precedence to resolve dangling else in favor of nearest if.
- Added dedicated grammar productions for if/no-else, if-else, switch, case/default, and break.
- Implemented semantic checks for:
  - constant-condition warnings in if
  - switch controlling expression must be integral
  - duplicate case detection
  - max one default
  - break validity via context depth

### Challenges

- Handling dangling else without introducing grammar conflicts.
- Preserving correct break scope for nested loop/switch combinations.
- Enforcing switch semantic rules without over-constraining grammar.

### Code Changes (2 chunks)

```bison
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

if_stmt:
    IF '(' expr ')' stmt %prec LOWER_THAN_ELSE {
        checkIfCondition($3);
        $$ = createIf($3, $5, NULL);
    }
    | IF '(' expr ')' stmt ELSE stmt {
        checkIfCondition($3);
        $$ = createIf($3, $5, $7);
    }
    ;
```

```bison
switch_stmt:
    SWITCH '(' expr ')' { enterBreakContext(); } '{' case_clause_list_opt '}' {
        if (!isIntegralExpr($3)) {
            fprintf(stderr, "\nSemantic Error at line %d:\n", yyline);
            fprintf(stderr, "   switch controlling expression must be integral (int)\n");
            fprintf(stderr, "Suggestions:\n");
            fprintf(stderr, "   - Use an int expression in switch(...)\n");
            fprintf(stderr, "   - Convert float expressions before switching\n\n");
            semantic_error_count++;
        }
        validateSwitchCases($7);
        exitBreakContext();
        $$ = createSwitch($3, $7);
    }
    ;

break_stmt:
    BREAK ';' {
        if (break_context_depth == 0) {
            fprintf(stderr, "\nSemantic Error at line %d:\n", yyline);
            fprintf(stderr, "   'break' is only valid inside a loop or switch\n");
            fprintf(stderr, "Suggestion: Place break inside while/for/switch blocks\n\n");
            semantic_error_count++;
        }
        $$ = createBreak();
    }
    ;
```

## 5. AST Integration

### Design Decisions

- Added explicit node kinds for NODE_IF, NODE_SWITCH, NODE_CASE, NODE_BREAK.
- Represented case clauses as a linked list to preserve source order.
- Stored else body as nullable pointer, enabling both if-only and if-else with one node shape.
- Stored comparison operations as OP_EQ/OP_NE/OP_LT/OP_GT/OP_LE/OP_GE.

### Challenges

- Ensuring case list append order remained stable through parsing and traversal.
- Representing fall-through naturally (empty case body) without extra node types.

### Code Changes (2 chunks)

```c
typedef enum {
    NODE_WHILE,
    NODE_FOR,
    NODE_IF,
    NODE_SWITCH,
    NODE_CASE,
    NODE_BREAK,
    NODE_STRUCT_DEF,
    NODE_FIELD_DECL,
    NODE_MEMBER_ACCESS,
    NODE_MEMBER_ASSIGN,
    NODE_ADDR_OF
} NodeType;

#define OP_EQ 1000
#define OP_NE 1001
#define OP_LT 1002
#define OP_GT 1003
#define OP_LE 1004
#define OP_GE 1005
```

```c
/* Append case clause to preserve source order */
ASTNode* appendCase(ASTNode* list, ASTNode* clause) {
    if (!clause) return list;
    if (!list) return clause;

    ASTNode* curr = list;
    while (curr->data.case_stmt.next) {
        curr = curr->data.case_stmt.next;
    }
    curr->data.case_stmt.next = clause;
    return list;
}

/* Create an if-statement node.
 * else_stmt is NULL when there is no else clause. */
ASTNode* createIf(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;
    return node;
}
```

## 6. TAC Integration

### Design Decisions

- Lowered if/if-else to TAC_IF_FALSE + TAC_LABEL + TAC_GOTO patterns.
- Lowered switch using linear dispatch over case labels.
- Evaluated switch expression once and stored it in a temporary variable.
- Implemented break with a stack of break labels for nested constructs.
- Comparison operations generate TAC values (0 or 1), then branches consume those values.

### Challenges

- Correct label generation for nested control flow.
- Ensuring break targets innermost active loop/switch.
- Handling switch dispatch/default consistently while preserving fall-through behavior.

### Code Changes (2 chunks)

```c
case NODE_BREAK: {
    char* break_target = currentBreakLabel();
    if (break_target) {
        appendTAC(createTAC(TAC_GOTO, break_target, NULL, NULL));
    } else {
        fprintf(stderr, "TAC Error: break without enclosing context\n");
    }
    break;
}

case NODE_IF: {
    char* cond = generateTACExpr(node->data.if_stmt.condition);
    if (node->data.if_stmt.else_stmt) {
        char* else_lbl = newLabel();
        char* end_lbl  = newLabel();
        appendTAC(createTAC(TAC_IF_FALSE, cond, NULL, else_lbl));
        generateTAC(node->data.if_stmt.then_stmt);
        appendTAC(createTAC(TAC_GOTO, end_lbl, NULL, NULL));
        appendTAC(createTAC(TAC_LABEL, else_lbl, NULL, NULL));
        generateTAC(node->data.if_stmt.else_stmt);
        appendTAC(createTAC(TAC_LABEL, end_lbl, NULL, NULL));
    } else {
        char* end_lbl = newLabel();
        appendTAC(createTAC(TAC_IF_FALSE, cond, NULL, end_lbl));
        generateTAC(node->data.if_stmt.then_stmt);
        appendTAC(createTAC(TAC_LABEL, end_lbl, NULL, NULL));
    }
    break;
}
```

```c
char* switch_value = generateTACExpr(node->data.switch_stmt.expr);
if (switch_value) {
    appendTAC(createTAC(TAC_ASSIGN, switch_value, NULL, switch_var));
}

char* dispatch_done = newLabel();
char* end_label = newLabel();
pushBreakLabel(end_label);

appendTAC(createTAC(TAC_EQ, switch_var, value_str, cmp_temp));
appendTAC(createTAC(TAC_IF_FALSE, cmp_temp, NULL, fail_label));
appendTAC(createTAC(TAC_GOTO, labels[i], NULL, NULL));

appendTAC(createTAC(TAC_LABEL, end_label, NULL, NULL));
popBreakLabel();
free(cases);
free(labels);
    break;
}
```

## 7. Optimization Integration

### Design Decisions

- Folded constant comparisons (EQ/NE/LT/GT/LE/GE) into integer constants.
- Optimized TAC_IF_FALSE when condition is compile-time constant.
- Distinguished loop back-edges from forward gotos to avoid corrupting branch semantics.
- Invalidated propagation at merge labels to avoid crossing branch boundaries unsafely.

### Challenges

- Incorrectly classifying forward gotos as loop edges causes wrong loopDepth behavior.
- Aggressive branch optimization risks deleting necessary control transfer in if-else and loops.

### Code Changes (2 chunks)

```c
static int tryFoldBinop(TACOp op, const char* left, const char* right,
                        char* resultBuf, size_t bufSize) {
    if (!isConst(left) || !isConst(right)) return 0;
    double l = atof(left), r = atof(right), res = 0;
    switch (op) {
        case TAC_EQ: res = (l == r) ? 1 : 0; break;
        case TAC_NE: res = (l != r) ? 1 : 0; break;
        case TAC_LT: res = (l <  r) ? 1 : 0; break;
        case TAC_GT: res = (l >  r) ? 1 : 0; break;
        case TAC_LE: res = (l <= r) ? 1 : 0; break;
        case TAC_GE: res = (l >= r) ? 1 : 0; break;
        default: return 0;
    }
    snprintf(resultBuf, bufSize, "%d", (int)res);
    return 1;
}
```

```c
case TAC_IF_FALSE: {
    const char* cond = curr->arg1;
    if (loopDepth == 0) cond = propLookup(cond, propTable, propCount);
    if (isConst(cond)) {
        int condVal = (int)atof(cond);
        if (condVal != 0) {
            newInstr = NULL;
        } else {
            const char* endLabel = curr->result;
            if (isLoopStartLabel(endLabel, loopStarts, nLoopStarts)) {
                TACInstr* skip = curr->next;
                while (skip &&
                       !(skip->op == TAC_LABEL && skip->arg1 &&
                         strcmp(skip->arg1, endLabel) == 0)) {
                    if (skip->op == TAC_GOTO && skip->arg1 &&
                        isLoopStartLabel(skip->arg1, loopStarts, nLoopStarts))
                        if (loopDepth > 0) loopDepth--;
                    skip = skip->next;
                }
                nextCurr = skip ? skip->next : NULL;
                newInstr = NULL;
            } else {
                newInstr = createTAC(TAC_GOTO, (char*)endLabel, NULL, NULL);
            }
        }
    } else {
        newInstr = createTAC(TAC_IF_FALSE, (char*)cond, NULL, curr->result);
    }
    break;
}
```

## 8. Backend (Codegen/MIPS) Integration

### Design Decisions

- Final control-flow emission is performed from optimized TAC.
- TAC_IF_FALSE lowers to beqz branch.
- TAC comparison ops lower to seq/sne/slt/sgt/sle/sge and store integer truth values.
- Label/goto emission explicitly documents merge points and memory-safety assumptions.

### Challenges

- Guaranteeing branch correctness at control-flow merge points.
- Preserving correctness when optimizer rewrites conditional branches.
- Maintaining consistency between AST-oriented codegen module and TAC-based backend path.

### Code Changes (2 chunks)

```c
case TAC_IF_FALSE:
    mgLoad(out, curr->arg1, "$t0");
    fprintf(out, "    beqz $t0, %s    # branch if false\n", curr->result);
    break;

case TAC_GOTO:
    fprintf(out, "    j %s    # unconditional jump (all vars in memory)\n",
            curr->arg1);
    break;
```

```c
case TAC_EQ:
    mgLoad(out, curr->arg1, "$t0");
    mgLoad(out, curr->arg2, "$t1");
    fprintf(out, "    seq $t2, $t0, $t1\n");
    mgStore(out, curr->result, "$t2");
    break;

case TAC_LT:
    mgLoad(out, curr->arg1, "$t0");
    mgLoad(out, curr->arg2, "$t1");
    fprintf(out, "    slt $t2, $t0, $t1\n");
    mgStore(out, curr->result, "$t2");
    break;
```

## 9. Boolean Logic Integration (Required for Correct Decisions)

### What is implemented

- Comparisons are fully integrated through parser -> AST -> TAC -> optimizer -> MIPS:
  - ==, !=, <, >, <=, >=
- Logical operators are fully integrated through scanner -> parser -> AST -> TAC -> optimizer -> MIPS:
    - &&, ||, !
- Comparison evaluation yields integer truth values (0 = false, nonzero = true).
- if/while/for/switch control flow consumes these truth values with TAC_IF_FALSE.
- TAC generation for && and || uses short-circuit control flow so right-hand expressions are evaluated only when required.

### Integration notes

- Unary ! is lowered to a boolean inversion pattern in TAC.
- && and || are lowered with labels and conditional branches to preserve language-level short-circuit semantics.

### Design implication

The current model supports both relational and compound boolean conditions directly in expressions used by if, loops, and switch-controlling logic.

### Challenge summary

- Implementing && and || required explicit short-circuit lowering in TAC generation.
- This required precedence updates and careful parser integration to avoid regressions.
- Optimizer still needs to preserve short-circuit behavior and avoid unsafe folding across side-effect boundaries.

## 10. Validation Evidence

The test suite contains direct coverage for control flow behavior:

- test_files/test2.cm: simple if, if-else true/false, nested if, chained else-if, dangling-else, all comparison operators.
- test_files/test_switch_basic.cm: switch/case/default/break dispatch behavior.
- test_files/test.cm and test_files/test4.cm: branch and constant-condition behavior relevant to optimizer rewrites.
- test_files/test_boolean_basic.cm: direct &&, ||, ! expression behavior.
- test_files/test_boolean_short_circuit.cm: runtime short-circuit validation using function side effects.
- test_files/test_boolean_nested_switch.cm: mixed boolean + nested control-flow coverage.

## 11. Key Integration Outcomes

1. if/if-else and nested if behavior is correctly represented from grammar to assembly.
2. switch/case/default/break is integrated with semantic validation and TAC lowering.
3. break scoping is safely handled with parser context checks and TAC break-label stack.
4. Comparison and logical boolean semantics are implemented consistently and optimized.
5. Short-circuit logical operators (&&, ||, !) are integrated end-to-end and validated with SPIM execution tests.
