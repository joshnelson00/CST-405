#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"

MIPSInstr* createMIPS(MIPSOp op, char* result, char* arg1, char* arg2, char* comment) {
    MIPSInstr* instr = malloc(sizeof(MIPSInstr));
    instr->op = op;
    instr->result = result ? strdup(result) : NULL;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->comment = comment ? strdup(comment) : NULL;
    instr->next = NULL;
    return instr;
}

void appendMIPS(MIPSList* list, MIPSInstr* instr) {
    if (!list->head) {
        list->head = list->tail = instr;
    } else {
        list->tail->next = instr;
        list->tail = instr;
    }
}

int countMIPSInstructions(MIPSList* list) {
    int count = 0;
    MIPSInstr* curr = list->head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    return count;
}

void printMIPS(MIPSList* list, FILE* output) {
    MIPSInstr* curr = list->head;
    while (curr) {
        switch(curr->op) {
            case MIPS_LI:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    li %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    li %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_LW:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    lw %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    lw %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_SW:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    sw %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    sw %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_L_S:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    l.s %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    l.s %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_S_S:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    s.s %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    s.s %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_ADD:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    add %s, %s, %s    # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    add %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_SUB:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    sub %s, %s, %s    # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    sub %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_MUL:
                if (curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    mult %s, %s       # %s\n", curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    mult %s, %s\n", curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_DIV:
                if (curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    div %s, %s        # %s\n", curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    div %s, %s\n", curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_MFLO:
                if (curr->result) {
                    if (curr->comment) {
                        fprintf(output, "    mflo %s           # %s\n", curr->result, curr->comment);
                    } else {
                        fprintf(output, "    mflo %s\n", curr->result);
                    }
                }
                break;
            case MIPS_ADD_S:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    add.s %s, %s, %s  # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    add.s %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_SUB_S:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    sub.s %s, %s, %s  # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    sub.s %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_MUL_S:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    mul.s %s, %s, %s  # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    mul.s %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_DIV_S:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    div.s %s, %s, %s  # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    div.s %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_MTC1:
                if (curr->arg1 && curr->result) {
                    if (curr->comment) {
                        fprintf(output, "    mtc1 %s, %s        # %s\n", curr->arg1, curr->result, curr->comment);
                    } else {
                        fprintf(output, "    mtc1 %s, %s\n", curr->arg1, curr->result);
                    }
                }
                break;
            case MIPS_MFC1:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    mfc1 %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    mfc1 %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_CVT_S_W:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    cvt.s.w %s, %s     # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    cvt.s.w %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_TRUNC_W_S:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    trunc.w.s %s, %s  # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    trunc.w.s %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_MOVE:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    move %s, %s        # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    move %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_MOV_S:
                if (curr->result && curr->arg1) {
                    if (curr->comment) {
                        fprintf(output, "    mov.s %s, %s       # %s\n", curr->result, curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    mov.s %s, %s\n", curr->result, curr->arg1);
                    }
                }
                break;
            case MIPS_ADDI:
                if (curr->result && curr->arg1 && curr->arg2) {
                    if (curr->comment) {
                        fprintf(output, "    addi %s, %s, %s    # %s\n", curr->result, curr->arg1, curr->arg2, curr->comment);
                    } else {
                        fprintf(output, "    addi %s, %s, %s\n", curr->result, curr->arg1, curr->arg2);
                    }
                }
                break;
            case MIPS_SYSCALL:
                if (curr->comment) {
                    fprintf(output, "    syscall            # %s\n", curr->comment);
                } else {
                    fprintf(output, "    syscall\n");
                }
                break;
            case MIPS_COMMENT:
                if (curr->comment) {
                    fprintf(output, "    # %s\n", curr->comment);
                } else {
                    // Skip NULL comments entirely
                }
                break;
            case MIPS_JAL:
                if (curr->arg1 && curr->arg1[0] != '\0') {
                    if (curr->comment) {
                        fprintf(output, "    jal %s  # %s\n", curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    jal %s\n", curr->arg1);
                    }
                } else {
                    // Skip NULL function calls entirely
                }
                break;
            case MIPS_JR:
                if (curr->arg1 && curr->arg1[0] != '\0') {
                    if (curr->comment) {
                        fprintf(output, "    jr %s  # %s\n", curr->arg1, curr->comment);
                    } else {
                        fprintf(output, "    jr %s\n", curr->arg1);
                    }
                } else {
                    // Skip NULL jump instructions entirely
                }
                break;
            case MIPS_LABEL:
                if (curr->arg1) {
                    fprintf(output, "\n%s:\n", curr->arg1);
                    if (curr->comment) {
                        fprintf(output, "    # %s\n", curr->comment);
                    }
                }
                break;
        }
        curr = curr->next;
    }
}

void freeMIPSList(MIPSList* list) {
    MIPSInstr* curr = list->head;
    while (curr) {
        MIPSInstr* next = curr->next;
        if (curr->result) free(curr->result);
        if (curr->arg1) free(curr->arg1);
        if (curr->arg2) free(curr->arg2);
        if (curr->comment) free(curr->comment);
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
}
