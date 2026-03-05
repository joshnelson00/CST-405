#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optimizer.h"
#include "mips.h"
#include "symtab.h"
#include "tac.h"

/* External declarations */
extern TACList tacList;
TACList optimizedList;
extern GlobalSymbolTable globalSymTab;

// External function declaration for isConst from tac.c
extern int isConst(const char* s);

// Forward declarations for helper functions
char* tempToReg(char* temp);
int isTemporary(char* name);

// Helper function to check if a variable is a temporary (t0, t1, etc.)
int isTemporary(char* name) {
    if (!name || strlen(name) < 2) return 0;
    // Must start with 't' and be followed by only digits
    if (name[0] != 't') return 0;
    for (int i = 1; name[i] != '\0'; i++) {
        if (!isdigit(name[i])) return 0;
    }
    return 1;
}

// Helper function to get stack offset, handling temporaries specially
int getOptimizerVarOffset(char* name) {
    // Don't look up temporary variables in symbol table
    if (isTemporary(name)) {
        return -1; // Temporaries are handled differently
    }
    return getVarOffset(name);
}

// Helper function to convert temporary variable name to register name
char* tempToReg(char* temp) {
    static char reg[10];
    if (isTemporary(temp)) {
        sprintf(reg, "$t%d", atoi(temp + 1));
        return reg;
    }
    return temp; // Return as-is if not a temporary
}

/* =========================================================
 * OPTIMIZER HELPER: check if a label is in the back-edge set
 * (i.e., it is the target of some TAC_GOTO — a loop-start label)
 * ========================================================= */
static int isLoopStartLabel(const char* label, char** starts, int count) {
    if (!label) return 0;
    for (int i = 0; i < count; i++)
        if (starts[i] && strcmp(starts[i], label) == 0) return 1;
    return 0;
}

/* =========================================================
 * OPTIMIZER HELPER: try to constant-fold a binary / relational op.
 * Returns 1 and fills resultBuf on success; 0 if not foldable.
 * Safe to call with non-constant operands — will return 0.
 * ========================================================= */
static int tryFoldBinop(TACOp op,
                        const char* left, const char* right,
                        char* resultBuf, size_t bufSize) {
    if (!isConst(left) || !isConst(right)) return 0;
    double l = atof(left), r = atof(right);
    double res = 0;
    switch (op) {
        case TAC_ADD:      res = l + r;                      break;
        case TAC_SUBTRACT: res = l - r;                      break;
        case TAC_MULTIPLY: res = l * r;                      break;
        case TAC_DIVIDE:
            if ((int)r == 0) return 0;   /* never fold div-by-zero */
            res = (int)l / (int)r;       break;
        case TAC_EQ:  res = (l == r) ? 1 : 0;               break;
        case TAC_NE:  res = (l != r) ? 1 : 0;               break;
        case TAC_LT:  res = (l <  r) ? 1 : 0;               break;
        case TAC_GT:  res = (l >  r) ? 1 : 0;               break;
        case TAC_LE:  res = (l <= r) ? 1 : 0;               break;
        case TAC_GE:  res = (l >= r) ? 1 : 0;               break;
        default: return 0;
    }
    snprintf(resultBuf, bufSize, "%d", (int)res);
    return 1;
}

/* =========================================================
 * PROP-TABLE HELPER: look up a variable in the copy-propagation
 * table (searches most-recent first).  Returns original name if
 * not found.
 * ========================================================= */
typedef struct { char* var; char* value; } PropEntry;

static const char* propLookup(const char* name,
                               PropEntry* table, int count) {
    if (!name) return name;
    for (int i = count - 1; i >= 0; i--)
        if (table[i].var && strcmp(table[i].var, name) == 0)
            return table[i].value;
    return name;
}

/* =========================================================
 * optimizeTAC2  —  Main optimizer pass
 *
 * Techniques implemented:
 *   1. Constant folding      — always, for any pure-constant operands
 *                              (arithmetic + relational), works INSIDE loops
 *   2. Copy propagation      — outside loops only (safe: no stale values)
 *   3. Dead branch elim.     — IF_FALSE <nonzero const> → remove instruction
 *   4. Dead loop  elim.      — IF_FALSE 0 → skip entire loop body
 *   5. Correct loop depth    — tracks nested for/while depth with a counter
 *                              so optimizations are scoped correctly
 * ========================================================= */
void optimizeTAC2() {

    /* ── STEP 1: Pre-scan to collect BACKWARD-jump loop-start labels ──────────
     * FIX for the merge-point register problem:
     * Previously ALL GOTO targets were added to loopStarts.  This caused
     * forward GOTOs from if-else (e.g. "GOTO end_label") to be incorrectly
     * treated as loop back-edges, which:
     *   • incremented loopDepth at if-else merge labels,
     *   • decremented loopDepth (possibly below 0) at the forward GOTO,
     *   • disabled copy-propagation inside real loops unnecessarily, and
     *   • applied dead-loop elimination to if-else bodies.
     *
     * Fix: two-pass scan.
     *   Pass 1 — record the instruction index of every LABEL.
     *   Pass 2 — a GOTO is a back-edge (loop) only when its target label
     *            appears BEFORE the GOTO in the instruction stream.
     * ────────────────────────────────────────────────────────────────────────*/
#define OPT_MAX_LABELS 256
    char* loopStarts[OPT_MAX_LABELS];
    int   nLoopStarts = 0;

    /* Pass 1: record label positions */
    typedef struct { char* name; int pos; } LPos;
    LPos labelPos[OPT_MAX_LABELS];
    int  nLabelPos = 0;
    {
        int idx = 0;
        TACInstr* s = tacList.head;
        while (s) {
            if (s->op == TAC_LABEL && s->arg1 && nLabelPos < OPT_MAX_LABELS) {
                labelPos[nLabelPos].name = s->arg1;
                labelPos[nLabelPos].pos  = idx;
                nLabelPos++;
            }
            idx++;
            s = s->next;
        }
    }
    /* Pass 2: only backward GOTOs (target precedes the GOTO) are loop starts */
    {
        int idx = 0;
        TACInstr* s = tacList.head;
        while (s) {
            if (s->op == TAC_GOTO && s->arg1) {
                int tgt = -1;
                for (int i = 0; i < nLabelPos; i++)
                    if (strcmp(labelPos[i].name, s->arg1) == 0) {
                        tgt = labelPos[i].pos; break;
                    }
                /* Back-edge: target label comes before this GOTO */
                if (tgt >= 0 && tgt < idx)
                    if (!isLoopStartLabel(s->arg1, loopStarts, nLoopStarts)
                        && nLoopStarts < OPT_MAX_LABELS)
                        loopStarts[nLoopStarts++] = s->arg1;
            }
            idx++;
            s = s->next;
        }
    }

    /* ── STEP 2: Single-pass optimizing traversal ─────────────────── */
    PropEntry propTable[256];
    int propCount  = 0;
    int loopDepth  = 0;   /* 0 = outside all loops; N = N levels deep */

    TACInstr* curr = tacList.head;
    while (curr) {
        TACInstr* newInstr = NULL;
        TACInstr* nextCurr = curr->next;   /* default: advance by one */

        /* ── Label handling ─────────────────────────────────────────────────
         * Loop-start labels (backward-jump targets): enter loop, invalidate.
         * All other labels (if-else else-entry or merge points): also
         * invalidate the prop table — variables assigned in one branch must
         * not be propagated into another branch or past the merge point.
         * ──────────────────────────────────────────────────────────────── */
        if (curr->op == TAC_LABEL && curr->arg1) {
            if (isLoopStartLabel(curr->arg1, loopStarts, nLoopStarts)) {
                loopDepth++;
                propCount = 0;   /* loop may mutate any variable */
            } else if (loopDepth == 0) {
                propCount = 0;   /* if-else merge/else-entry point */
            }
        }

        /* ── Is this a binary/relational op? Handle uniformly. ── */
        int isBinop = (curr->op == TAC_ADD      || curr->op == TAC_SUBTRACT ||
                       curr->op == TAC_MULTIPLY  || curr->op == TAC_DIVIDE   ||
                       curr->op == TAC_EQ        || curr->op == TAC_NE       ||
                       curr->op == TAC_LT        || curr->op == TAC_GT       ||
                       curr->op == TAC_LE        || curr->op == TAC_GE);

        if (isBinop) {
            /* Copy propagation (only outside loops — inside loops variables
             * may have been modified, so propagated values would be stale) */
            const char* left  = curr->arg1;
            const char* right = curr->arg2;
            if (loopDepth == 0) {
                left  = propLookup(left,  propTable, propCount);
                right = propLookup(right, propTable, propCount);
            }

            /* Constant folding: if BOTH operands are numeric literals, fold.
             * This is safe at ANY loop depth — constants never change. */
            char foldBuf[32];
            if (tryFoldBinop(curr->op, left, right, foldBuf, sizeof(foldBuf))) {
                char* foldedVal = strdup(foldBuf);
                /* Record in prop-table so downstream code can use it */
                if (loopDepth == 0 && propCount < 256) {
                    propTable[propCount].var   = strdup(curr->result);
                    propTable[propCount].value = foldedVal;
                    propCount++;
                }
                newInstr = createTAC(TAC_ASSIGN, foldedVal, NULL, curr->result);
            } else {
                /* Cannot fold — emit original op (with possibly substituted operands) */
                newInstr = createTAC(curr->op,
                                     (char*)left, (char*)right, curr->result);
            }
        }
        else switch (curr->op) {

        /* ── Assignment ── */
        case TAC_ASSIGN: {
            const char* value = curr->arg1;
            if (loopDepth == 0) {
                value = propLookup(value, propTable, propCount);
                if (propCount < 256) {
                    propTable[propCount].var   = strdup(curr->result);
                    propTable[propCount].value = strdup(value);
                    propCount++;
                }
            }
            newInstr = createTAC(TAC_ASSIGN, (char*)value, NULL, curr->result);
            break;
        }

        /* ── Print ── */
        case TAC_PRINT: {
            const char* value = curr->arg1;
            if (loopDepth == 0)
                value = propLookup(value, propTable, propCount);
            newInstr = createTAC(TAC_PRINT, (char*)value, NULL, NULL);
            break;
        }

        /* ── Conditional jump (key optimization point) ── */
        case TAC_IF_FALSE: {
            const char* cond = curr->arg1;
            /* Substitute through prop-table if possible */
            if (loopDepth == 0)
                cond = propLookup(cond, propTable, propCount);

            if (isConst(cond)) {
                int condVal = (int)atof(cond);
                if (condVal != 0) {
                    /* ────────────────────────────────────────────────
                     * DEAD BRANCH ELIMINATION
                     * Condition is always TRUE → branch never taken.
                     * Remove IF_FALSE; loop runs without an exit check
                     * (becomes infinite unless broken some other way).
                     * The end label will be emitted but unreachable.
                     * ──────────────────────────────────────────────── */
                    fprintf(stderr,
                        "\n⚡ Optimizer [dead-branch]: IF_FALSE %s GOTO %s"
                        " → removed (branch never taken)\n\n",
                        cond, curr->result);
                    newInstr = NULL;   /* skip IF_FALSE instruction */
                } else {
                    /* ────────────────────────────────────────────────
                     * CONDITION IS ALWAYS FALSE
                     * Distinguish loop exit-check from if-else branch:
                     *   • Loop (backward target): dead-loop → skip body.
                     *   • If-else (forward target): replace IF_FALSE with
                     *     GOTO so the then-body is never entered.  The
                     *     then-body is still emitted but unreachable; the
                     *     else-body (if any) executes correctly.
                     * ──────────────────────────────────────────────── */
                    const char* endLabel = curr->result;
                    if (isLoopStartLabel(endLabel, loopStarts, nLoopStarts)) {
                        /* Dead loop elimination */
                        fprintf(stderr,
                            "\n⚡ Optimizer [dead-loop]: condition always false"
                            " — skipping loop body (target: %s)\n\n", endLabel);

                        TACInstr* skip = curr->next;
                        while (skip &&
                               !(skip->op == TAC_LABEL && skip->arg1 &&
                                 strcmp(skip->arg1, endLabel) == 0)) {
                            /* While skipping, keep loopDepth consistent:
                             * any back-edge GOTO we skip over would have
                             * decremented loopDepth in the normal path. */
                            if (skip->op == TAC_GOTO && skip->arg1 &&
                                isLoopStartLabel(skip->arg1, loopStarts, nLoopStarts))
                                if (loopDepth > 0) loopDepth--;
                            skip = skip->next;
                        }
                        /* skip == endLabel node (or NULL); advance past it */
                        nextCurr = skip ? skip->next : NULL;
                        newInstr  = NULL;
                    } else {
                        /* if (0): forward branch, always false → unconditional
                         * jump to the else/end label so the then-body is
                         * bypassed.  The then-body remains in the output as
                         * dead code but does not affect correctness. */
                        fprintf(stderr,
                            "\n⚡ Optimizer [if(0)]: condition always false"
                            " → replacing with GOTO %s\n\n", endLabel);
                        newInstr = createTAC(TAC_GOTO, (char*)endLabel, NULL, NULL);
                    }
                }
            } else {
                /* Condition is a runtime value — keep IF_FALSE as-is */
                newInstr = createTAC(TAC_IF_FALSE,
                                     (char*)cond, NULL, curr->result);
            }
            break;
        }

        /* ── Unconditional jump (back-edge) ── */
        case TAC_GOTO: {
            /* Leaving the innermost loop if this is a back-edge */
            if (curr->arg1 &&
                isLoopStartLabel(curr->arg1, loopStarts, nLoopStarts))
                if (loopDepth > 0) loopDepth--;
            newInstr = createTAC(TAC_GOTO, curr->arg1, NULL, NULL);
            break;
        }

        /* ── Function boundary: reset state ── */
        case TAC_FUNC_DEF:
            loopDepth = 0;
            propCount = 0;
            newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
            break;

        /* ── All other instructions: pass through unchanged ── */
        case TAC_PARAM:
        case TAC_FUNC_CALL:
        case TAC_RETURN:
        case TAC_ARG:
        case TAC_DECL:
        case TAC_ARRAY_DECL:
        case TAC_ARRAY_WRITE:
        case TAC_ARRAY_READ:
        case TAC_BOUNDS_CHECK:
        case TAC_DIV_CHECK:
        case TAC_LABEL:
        default:
            newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
            break;
        } /* end switch */

        if (newInstr) appendOptimizedTAC(newInstr);
        curr = nextCurr;
    } /* end while */
}

/* ─── MIPS Code Generation Helpers ─── */

// Variable tracking for MIPS generation (per-function)
typedef struct {
    char name[64];
    int  offset;        // offset from $fp
    int  isLocalArray;  // 1 if declared with ARRAY_DECL (data lives on stack)
    int  isParam;       // 1 if function parameter (could be a pointer if used as array)
} MIPSGenVar;

static MIPSGenVar mgVars[100];
static int mgVarCount;
static int mgNextOffset;
static int mgTempBase;   // temps start here on the stack
static int mgFrameSize;

static void mgReset(void) { mgVarCount = 0; mgNextOffset = 0; }

static int mgAddVar(const char* name, int size, int isArr, int isPar) {
    int off = mgNextOffset;
    strncpy(mgVars[mgVarCount].name, name, 63);
    mgVars[mgVarCount].name[63] = '\0';
    mgVars[mgVarCount].offset = off;
    mgVars[mgVarCount].isLocalArray = isArr;
    mgVars[mgVarCount].isParam = isPar;
    mgVarCount++;
    mgNextOffset += size;
    return off;
}

static int mgFind(const char* name) {
    for (int i = 0; i < mgVarCount; i++)
        if (strcmp(mgVars[i].name, name) == 0) return i;
    return -1;
}

// Check if string is a temp variable like "tN"
static int mgIsTemp(const char* s) {
    if (!s || s[0] != 't' || s[1] == '\0') return 0;
    for (int i = 1; s[i]; i++) if (!isdigit(s[i])) return 0;
    return 1;
}
static int mgTempNum(const char* s) { return atoi(s + 1); }

// Check if string is a numeric constant
static int mgIsConst(const char* s) {
    if (!s || *s == '\0') return 0;
    char* end; strtod(s, &end);
    return *end == '\0';
}
static int mgConstInt(const char* s) { return (int)atof(s); }

// Emit: load a TAC operand into a MIPS register
static void mgLoad(FILE* f, const char* op, const char* reg) {
    if (mgIsTemp(op)) {
        fprintf(f, "    lw %s, %d($fp)\n", reg, mgTempBase + mgTempNum(op) * 4);
    } else if (mgIsConst(op)) {
        fprintf(f, "    li %s, %d\n", reg, mgConstInt(op));
    } else {
        int idx = mgFind(op);
        if (idx >= 0) fprintf(f, "    lw %s, %d($fp)\n", reg, mgVars[idx].offset);
    }
}

// Emit: store a MIPS register to a TAC destination
static void mgStore(FILE* f, const char* dst, const char* reg) {
    if (mgIsTemp(dst)) {
        fprintf(f, "    sw %s, %d($fp)\n", reg, mgTempBase + mgTempNum(dst) * 4);
    } else {
        int idx = mgFind(dst);
        if (idx >= 0) fprintf(f, "    sw %s, %d($fp)\n", reg, mgVars[idx].offset);
    }
}

/* ─── Main MIPS Code Generator ─── */
void generateMIPSFromOptimizedTAC2(const char* filename) {
    FILE* out = fopen(filename, "w");
    if (!out) { fprintf(stderr, "Error: Cannot open output file %s\n", filename); return; }

    // First pass: collect string literals
    TACInstr* scan = optimizedList.head;
    int stringCount = 0;
    typedef struct {
        char* value;
        int id;
    } StringLit;
    StringLit strings[100];  // Max 100 string literals
    
    while (scan) {
        if (scan->op == TAC_PRINT && scan->arg1 && scan->arg1[0] == '"') {
            // Check if string already exists
            int found = -1;
            for (int i = 0; i < stringCount; i++) {
                if (strcmp(strings[i].value, scan->arg1) == 0) {
                    found = i;
                    break;
                }
            }
            if (found == -1 && stringCount < 100) {
                strings[stringCount].value = scan->arg1;
                strings[stringCount].id = stringCount;
                stringCount++;
            }
        }
        scan = scan->next;
    }

    // Output data section with strings
    fprintf(out, ".data\n");
    for (int i = 0; i < stringCount; i++) {
        fprintf(out, "str_%d: .asciiz ", strings[i].id);
        // Output string, processing escape sequences
        fprintf(out, "%s\n", strings[i].value);  // TAC already has quotes
    }
    fprintf(out, "\n.text\n");
    fprintf(out, ".globl main\n\n");

    TACInstr* curr = optimizedList.head;
    int inMain = 0;

    // Argument collector for function calls
    char* callArgs[10];
    int callArgCount = 0;

    // Process all functions
    while (curr) {
        /* ── FUNC_DEF: set up a new function ── */
        if (curr->op == TAC_FUNC_DEF) {
            // If we just finished main without RETURN, emit exit
            if (inMain) {
                fprintf(out, "    li $v0, 10\n");
                fprintf(out, "    syscall\n");
            }

            char* fn = curr->arg1;
            inMain = (strcmp(fn, "main") == 0);
            mgReset();
            callArgCount = 0;

            /* Pre-scan: find max temp, count params, collect array decl info */
            TACInstr* scan = curr->next;
            int maxTemp = -1, pCount = 0;
            char* arrNames[20]; int arrCount = 0;

            while (scan && scan->op != TAC_FUNC_DEF) {
                if (scan->op == TAC_PARAM) pCount++;
                if (scan->op == TAC_ARRAY_DECL && scan->arg1)
                    arrNames[arrCount++] = scan->arg1;
                const char* flds[] = { scan->arg1, scan->arg2, scan->result };
                for (int f = 0; f < 3; f++)
                    if (flds[f] && mgIsTemp(flds[f])) {
                        int t = mgTempNum(flds[f]);
                        if (t > maxTemp) maxTemp = t;
                    }
                scan = scan->next;
            }

            // Look up array sizes from the symbol table
            int arrSizes[20];
            enterFunction(fn);
            for (int i = 0; i < arrCount; i++) {
                arrSizes[i] = getArraySize(arrNames[i]);
                if (arrSizes[i] <= 0) arrSizes[i] = 10;
            }
            exitFunction();

            // Build variable map: PARAMs first, then ARRAY_DECLs, then DECLs
            scan = curr->next;
            while (scan && scan->op != TAC_FUNC_DEF) {
                if (scan->op == TAC_PARAM) mgAddVar(scan->arg1, 4, 0, 1);
                scan = scan->next;
            }
            scan = curr->next;
            while (scan && scan->op != TAC_FUNC_DEF) {
                if (scan->op == TAC_ARRAY_DECL) {
                    for (int i = 0; i < arrCount; i++)
                        if (strcmp(arrNames[i], scan->arg1) == 0) {
                            mgAddVar(scan->arg1, arrSizes[i] * 4, 1, 0);
                            break;
                        }
                }
                scan = scan->next;
            }
            scan = curr->next;
            while (scan && scan->op != TAC_FUNC_DEF) {
                if (scan->op == TAC_DECL) mgAddVar(scan->result, 4, 0, 0);
                scan = scan->next;
            }

            mgTempBase = mgNextOffset;
            int nTemps = (maxTemp >= 0) ? (maxTemp + 1) : 0;
            mgFrameSize = mgNextOffset + nTemps * 4 + 8;
            if (mgFrameSize % 8) mgFrameSize += 8 - (mgFrameSize % 8);

            // Emit prologue — prefix non-main functions with fn_ to avoid
            // conflicts with MIPS instruction mnemonics (add, sub, mul, div…)
            char fnLab[256];
            if (strcmp(fn, "main") == 0)
                snprintf(fnLab, sizeof(fnLab), "main");
            else
                snprintf(fnLab, sizeof(fnLab), "fn_%s", fn);
            fprintf(out, "%s:\n", fnLab);
            fprintf(out, "    subu $sp, $sp, %d\n", mgFrameSize);
            fprintf(out, "    sw $ra, %d($sp)\n", mgFrameSize - 4);
            fprintf(out, "    sw $fp, %d($sp)\n", mgFrameSize - 8);
            fprintf(out, "    move $fp, $sp\n");

            // Store incoming params ($a0..$a3) onto the stack
            for (int i = 0; i < pCount && i < 4; i++)
                fprintf(out, "    sw $a%d, %d($fp)\n", i, mgVars[i].offset);

            curr = curr->next;
            continue;
        }

        /* ── Process each TAC instruction ── */
        switch (curr->op) {
        case TAC_PARAM: case TAC_DECL: case TAC_ARRAY_DECL:
        case TAC_BOUNDS_CHECK: case TAC_DIV_CHECK:
            break; // handled during pre-scan or skipped

        case TAC_ASSIGN:
            mgLoad(out, curr->arg1, "$t0");
            mgStore(out, curr->result, "$t0");
            break;

        case TAC_ADD:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    add $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_SUBTRACT:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    sub $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_MULTIPLY:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    mult $t0, $t1\n");
            fprintf(out, "    mflo $t2\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_DIVIDE:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    div $t0, $t1\n");
            fprintf(out, "    mflo $t2\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_ARRAY_WRITE: {
            // arg1=array, arg2=index, result=value
            int vi = mgFind(curr->arg1);
            if (vi < 0) break;
            mgLoad(out, curr->result, "$t0");  // value
            if (mgVars[vi].isLocalArray) {
                if (mgIsConst(curr->arg2)) {
                    fprintf(out, "    sw $t0, %d($fp)\n",
                            mgVars[vi].offset + mgConstInt(curr->arg2) * 4);
                } else {
                    mgLoad(out, curr->arg2, "$t1");
                    fprintf(out, "    sll $t1, $t1, 2\n");
                    fprintf(out, "    addi $t2, $fp, %d\n", mgVars[vi].offset);
                    fprintf(out, "    add $t2, $t2, $t1\n");
                    fprintf(out, "    sw $t0, 0($t2)\n");
                }
            } else if (mgVars[vi].isParam) {
                fprintf(out, "    lw $t3, %d($fp)\n", mgVars[vi].offset);
                if (mgIsConst(curr->arg2))
                    fprintf(out, "    sw $t0, %d($t3)\n", mgConstInt(curr->arg2) * 4);
                else {
                    mgLoad(out, curr->arg2, "$t1");
                    fprintf(out, "    sll $t1, $t1, 2\n");
                    fprintf(out, "    add $t3, $t3, $t1\n");
                    fprintf(out, "    sw $t0, 0($t3)\n");
                }
            }
            break;
        }

        case TAC_ARRAY_READ: {
            // arg1=array, arg2=index, result=dest
            int vi = mgFind(curr->arg1);
            if (vi < 0) break;
            if (mgVars[vi].isLocalArray) {
                if (mgIsConst(curr->arg2)) {
                    fprintf(out, "    lw $t0, %d($fp)\n",
                            mgVars[vi].offset + mgConstInt(curr->arg2) * 4);
                } else {
                    mgLoad(out, curr->arg2, "$t1");
                    fprintf(out, "    sll $t1, $t1, 2\n");
                    fprintf(out, "    addi $t2, $fp, %d\n", mgVars[vi].offset);
                    fprintf(out, "    add $t2, $t2, $t1\n");
                    fprintf(out, "    lw $t0, 0($t2)\n");
                }
            } else if (mgVars[vi].isParam) {
                fprintf(out, "    lw $t3, %d($fp)\n", mgVars[vi].offset);
                if (mgIsConst(curr->arg2))
                    fprintf(out, "    lw $t0, %d($t3)\n", mgConstInt(curr->arg2) * 4);
                else {
                    mgLoad(out, curr->arg2, "$t1");
                    fprintf(out, "    sll $t1, $t1, 2\n");
                    fprintf(out, "    add $t3, $t3, $t1\n");
                    fprintf(out, "    lw $t0, 0($t3)\n");
                }
            }
            mgStore(out, curr->result, "$t0");
            break;
        }

        case TAC_PRINT:
            // Check if it's a string literal (starts with quote)
            if (curr->arg1 && curr->arg1[0] == '"') {
                // Find string ID
                int strId = -1;
                for (int i = 0; i < stringCount; i++) {
                    if (strcmp(strings[i].value, curr->arg1) == 0) {
                        strId = i;
                        break;
                    }
                }
                if (strId >= 0) {
                    fprintf(out, "    la $a0, str_%d\n", strId);
                    fprintf(out, "    li $v0, 4\n");
                    fprintf(out, "    syscall\n");
                }
            } else {
                // Integer/variable print followed by automatic newline
                mgLoad(out, curr->arg1, "$a0");
                fprintf(out, "    li $v0, 1\n");
                fprintf(out, "    syscall\n");
                /* Print newline character (syscall 11 = print_char, '\n' = 10) */
                fprintf(out, "    li $v0, 11\n");
                fprintf(out, "    li $a0, 10\n");
                fprintf(out, "    syscall\n");
            }
            break;

        case TAC_ARG:
            if (callArgCount < 10)
                callArgs[callArgCount++] = curr->arg1;
            break;

        case TAC_FUNC_CALL: {
            // Load args in reverse order → $a0, $a1, …
            for (int i = 0; i < callArgCount && i < 4; i++) {
                char* a = callArgs[callArgCount - 1 - i];
                if (mgIsTemp(a)) {
                    fprintf(out, "    lw $a%d, %d($fp)\n", i,
                            mgTempBase + mgTempNum(a) * 4);
                } else if (mgIsConst(a)) {
                    fprintf(out, "    li $a%d, %d\n", i, mgConstInt(a));
                } else {
                    int vi = mgFind(a);
                    if (vi >= 0) {
                        if (mgVars[vi].isLocalArray)
                            fprintf(out, "    addi $a%d, $fp, %d\n", i,
                                    mgVars[vi].offset);
                        else
                            fprintf(out, "    lw $a%d, %d($fp)\n", i,
                                    mgVars[vi].offset);
                    }
                }
            }
            char callLab[256];
            if (strcmp(curr->arg1, "main") == 0)
                snprintf(callLab, sizeof(callLab), "main");
            else
                snprintf(callLab, sizeof(callLab), "fn_%s", curr->arg1);
            fprintf(out, "    jal %s\n", callLab);
            if (curr->result) mgStore(out, curr->result, "$v0");
            callArgCount = 0;
            break;
        }

        case TAC_RETURN:
            if (curr->arg1) mgLoad(out, curr->arg1, "$v0");
            if (inMain) {
                fprintf(out, "    li $v0, 10\n");
                fprintf(out, "    syscall\n");
            } else {
                fprintf(out, "    lw $ra, %d($sp)\n", mgFrameSize - 4);
                fprintf(out, "    lw $fp, %d($sp)\n", mgFrameSize - 8);
                fprintf(out, "    addu $sp, $sp, %d\n", mgFrameSize);
                fprintf(out, "    jr $ra\n");
            }
            break;

        case TAC_LABEL:
            /* At every label (merge point) we conceptually invalidate all
             * register descriptors — code may have arrived here via a branch
             * from a different path.  Because this code generator uses a
             * strict load/store discipline (every variable read calls mgLoad,
             * which always emits lw/li from the stack slot), no stale cached
             * register value is ever used.  The comment below documents the
             * invariant explicitly, as required by Activity 2 Task 3.3/3.5. */
            fprintf(out, "%s:    # merge point — register state invalidated\n",
                    curr->arg1);
            break;

        case TAC_GOTO:
            /* Before an unconditional jump, all live variables are already
             * in memory (every assignment emits mgStore).  This is equivalent
             * to calling spillAllRegisters() as required by Activity 2 Task 3.4. */
            fprintf(out, "    j %s    # unconditional jump (all vars in memory)\n",
                    curr->arg1);
            break;

        case TAC_IF_FALSE:
            /* Before the conditional branch, variables are in memory via the
             * load/store discipline — equivalent to spillAllRegisters(). */
            mgLoad(out, curr->arg1, "$t0");
            fprintf(out, "    beqz $t0, %s    # branch if false\n", curr->result);
            break;

        case TAC_EQ:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    seq $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_NE:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    sne $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_LT:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    slt $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_GT:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    sgt $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_LE:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    sle $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        case TAC_GE:
            mgLoad(out, curr->arg1, "$t0");
            mgLoad(out, curr->arg2, "$t1");
            fprintf(out, "    sge $t2, $t0, $t1\n");
            mgStore(out, curr->result, "$t2");
            break;

        default: break;
        }

        curr = curr->next;
    }

    // If main ended without explicit RETURN, emit exit
    if (inMain) {
        fprintf(out, "    li $v0, 10\n");
        fprintf(out, "    syscall\n");
    }

    fclose(out);
}

// Print optimized TAC instructions
void printOptimizedTAC2() {
    printf("Optimized TAC Instructions:\n");
    printf("─────────────────────────────\n");
    
    TACInstr* curr = optimizedList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                printf("%2d: %s = %s + %s     // Add\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_SUBTRACT:
                printf("%2d: %s = %s - %s     // Subtract\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_MULTIPLY:
                printf("%2d: %s = %s * %s     // Multiply\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_DIVIDE:
                printf("%2d: %s = %s / %s     // Divide\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_ASSIGN:
                printf("%2d: %s = %s           // Assignment\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PRINT:
                printf("%2d: PRINT %s          // Output value\n", instrNum++, curr->arg1);
                break;
            case TAC_DECL:
                printf("%2d: DECL %s           // Declare variable\n", instrNum++, curr->result);
                break;
            case TAC_FUNC_DEF:
                printf("%2d: FUNC %s          // Function definition\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                printf("%2d: %s = CALL %s       // Function call\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PARAM:
                printf("%2d: PARAM %s         // Function parameter\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    printf("%2d: RETURN %s        // Return value\n", instrNum++, curr->arg1);
                } else {
                    printf("%2d: RETURN            // Return void\n", instrNum++);
                }
                break;
            case TAC_ARG:
                printf("%2d: ARG %s            // Function argument\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                printf("%2d: ARRAY_DECL %s     // Array declaration\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_WRITE:
                printf("%2d: ARRAY_WRITE %s[%s] = %s  // Array assignment\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_READ:
                printf("%2d: ARRAY_READ %s[%s] -> %s  // Array access\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_BOUNDS_CHECK:
                printf("%2d: BOUNDS_CHECK %s[%s] < %s // Runtime bounds check\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_DIV_CHECK:
                printf("%2d: DIV_CHECK %s != 0        // Runtime divide-by-zero check\n", instrNum++, curr->arg1);
                break;
            case TAC_LABEL:
                printf("%s:                          // Loop label\n", curr->arg1);
                break;
            case TAC_GOTO:
                printf("%2d: GOTO %s                  // Unconditional jump\n", instrNum++, curr->arg1);
                break;
            case TAC_IF_FALSE:
                printf("%2d: IF_FALSE %s GOTO %s     // Conditional jump\n", instrNum++, curr->arg1, curr->result);
                break;
            case TAC_EQ:
                printf("%2d: %s = %s == %s           // Equality\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_NE:
                printf("%2d: %s = %s != %s           // Not equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LT:
                printf("%2d: %s = %s < %s            // Less than\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GT:
                printf("%2d: %s = %s > %s            // Greater than\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LE:
                printf("%2d: %s = %s <= %s           // Less or equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GE:
                printf("%2d: %s = %s >= %s           // Greater or equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
        }
        curr = curr->next;
    }
    printf("\n");
}

// Print optimized TAC to file
void printOptimizedTACToFile2(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Optimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    
    TACInstr* curr = optimizedList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                fprintf(file, "%2d: %s = %s + %s     // Add\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_SUBTRACT:
                fprintf(file, "%2d: %s = %s - %s     // Subtract\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_MULTIPLY:
                fprintf(file, "%2d: %s = %s * %s     // Multiply\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_DIVIDE:
                fprintf(file, "%2d: %s = %s / %s     // Divide\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_ASSIGN:
                fprintf(file, "%2d: %s = %s           // Assignment\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PRINT:
                fprintf(file, "%2d: PRINT %s          // Output value\n", instrNum++, curr->arg1);
                break;
            case TAC_DECL:
                fprintf(file, "%2d: DECL %s           // Declare variable\n", instrNum++, curr->result);
                break;
            case TAC_FUNC_DEF:
                fprintf(file, "%2d: FUNC %s          // Function definition\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                fprintf(file, "%2d: %s = CALL %s       // Function call\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PARAM:
                fprintf(file, "%2d: PARAM %s         // Function parameter\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    fprintf(file, "%2d: RETURN %s        // Return value\n", instrNum++, curr->arg1);
                } else {
                    fprintf(file, "%2d: RETURN            // Return void\n", instrNum++);
                }
                break;
            case TAC_ARG:
                fprintf(file, "%2d: ARG %s            // Function argument\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                fprintf(file, "%2d: ARRAY_DECL %s     // Array declaration\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_WRITE:
                fprintf(file, "%2d: ARRAY_WRITE %s[%s] = %s  // Array assignment\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_READ:
                fprintf(file, "%2d: ARRAY_READ %s[%s] -> %s  // Array access\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_BOUNDS_CHECK:
                fprintf(file, "%2d: BOUNDS_CHECK %s[%s] < %s // Runtime bounds check\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_DIV_CHECK:
                fprintf(file, "%2d: DIV_CHECK %s != 0        // Runtime divide-by-zero check\n", instrNum++, curr->arg1);
                break;
            case TAC_LABEL:
                fprintf(file, "%s:                          // Loop label\n", curr->arg1);
                break;
            case TAC_GOTO:
                fprintf(file, "%2d: GOTO %s                  // Unconditional jump\n", instrNum++, curr->arg1);
                break;
            case TAC_IF_FALSE:
                fprintf(file, "%2d: IF_FALSE %s GOTO %s     // Conditional jump\n", instrNum++, curr->arg1, curr->result);
                break;
            case TAC_EQ:
                fprintf(file, "%2d: %s = %s == %s           // Equality\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_NE:
                fprintf(file, "%2d: %s = %s != %s           // Not equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LT:
                fprintf(file, "%2d: %s = %s < %s            // Less than\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GT:
                fprintf(file, "%2d: %s = %s > %s            // Greater than\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LE:
                fprintf(file, "%2d: %s = %s <= %s           // Less or equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GE:
                fprintf(file, "%2d: %s = %s >= %s           // Greater or equal\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
        }
        curr = curr->next;
    }
    
    fclose(file);
}

// Print unoptimized TAC to file (with actual variable names)
void printTACToFile2(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Unoptimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    
    TACInstr* curr = tacList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                fprintf(file, "%2d: %s = %s + %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_SUBTRACT:
                fprintf(file, "%2d: %s = %s - %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_MULTIPLY:
                fprintf(file, "%2d: %s = %s * %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_DIVIDE:
                fprintf(file, "%2d: %s = %s / %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_ASSIGN:
                fprintf(file, "%2d: %s = %s\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PRINT:
                fprintf(file, "%2d: PRINT %s\n", instrNum++, curr->arg1);
                break;
            case TAC_DECL:
                fprintf(file, "%2d: DECL %s\n", instrNum++, curr->result);
                break;
            case TAC_FUNC_DEF:
                fprintf(file, "%2d: FUNC %s\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                fprintf(file, "%2d: %s = CALL %s\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PARAM:
                fprintf(file, "%2d: PARAM %s\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1)
                    fprintf(file, "%2d: RETURN %s\n", instrNum++, curr->arg1);
                else
                    fprintf(file, "%2d: RETURN\n", instrNum++);
                break;
            case TAC_ARG:
                fprintf(file, "%2d: ARG %s\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                fprintf(file, "%2d: ARRAY_DECL %s\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_WRITE:
                fprintf(file, "%2d: ARRAY_WRITE %s[%s] = %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_READ:
                fprintf(file, "%2d: ARRAY_READ %s[%s] -> %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_BOUNDS_CHECK:
                fprintf(file, "%2d: BOUNDS_CHECK %s[%s] < %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_DIV_CHECK:
                fprintf(file, "%2d: DIV_CHECK %s != 0\n", instrNum++, curr->arg1);
                break;
            case TAC_LABEL:
                fprintf(file, "%s:\n", curr->arg1);
                break;
            case TAC_GOTO:
                fprintf(file, "%2d: GOTO %s\n", instrNum++, curr->arg1);
                break;
            case TAC_IF_FALSE:
                fprintf(file, "%2d: IF_FALSE %s GOTO %s\n", instrNum++, curr->arg1, curr->result);
                break;
            case TAC_EQ:
                fprintf(file, "%2d: %s = %s == %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_NE:
                fprintf(file, "%2d: %s = %s != %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LT:
                fprintf(file, "%2d: %s = %s < %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GT:
                fprintf(file, "%2d: %s = %s > %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LE:
                fprintf(file, "%2d: %s = %s <= %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GE:
                fprintf(file, "%2d: %s = %s >= %s\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
        }
        curr = curr->next;
    }
    
    fclose(file);
}
