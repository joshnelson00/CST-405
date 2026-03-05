.data

.text
.globl main

fn_sign_test:
    subu $sp, $sp, 24
    sw $ra, 20($sp)
    sw $fp, 16($sp)
    move $fp, $sp
    sw $a0, 0($fp)
    li $t0, -1
    sw $t0, 8($fp)
    li $t0, -1
    sw $t0, 4($fp)
    lw $t0, 0($fp)
    li $t1, 0
    sgt $t2, $t0, $t1
    sw $t2, 12($fp)
    lw $t0, 12($fp)
    beqz $t0, L0    # branch if false
    li $t0, 7
    sw $t0, 4($fp)
L0:    # merge point — register state invalidated
    lw $v0, 4($fp)
    lw $ra, 20($sp)
    lw $fp, 16($sp)
    addu $sp, $sp, 24
    jr $ra
main:
    subu $sp, $sp, 96
    sw $ra, 92($sp)
    sw $fp, 88($sp)
    move $fp, $sp
    li $t0, 10
    sw $t0, 0($fp)
    li $t0, 5
    sw $t0, 4($fp)
    li $t0, 0
    sw $t0, 8($fp)
    li $t0, 1
    sw $t0, 20($fp)
    li $t0, 1
    sw $t0, 8($fp)
L1:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    lw $t1, 4($fp)
    sgt $t2, $t0, $t1
    sw $t2, 24($fp)
    lw $t0, 24($fp)
    beqz $t0, L2    # branch if false
    li $t0, 2
    sw $t0, 8($fp)
    j L3    # unconditional jump (all vars in memory)
L2:    # merge point — register state invalidated
    li $t0, 99
    sw $t0, 8($fp)
L3:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    lw $t1, 0($fp)
    sgt $t2, $t0, $t1
    sw $t2, 28($fp)
    lw $t0, 28($fp)
    beqz $t0, L4    # branch if false
    li $t0, 99
    sw $t0, 8($fp)
    j L5    # unconditional jump (all vars in memory)
L4:    # merge point — register state invalidated
    li $t0, 3
    sw $t0, 8($fp)
L5:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    lw $t1, 4($fp)
    sgt $t2, $t0, $t1
    sw $t2, 32($fp)
    lw $t0, 32($fp)
    beqz $t0, L6    # branch if false
    lw $t0, 0($fp)
    li $t1, 10
    seq $t2, $t0, $t1
    sw $t2, 36($fp)
    lw $t0, 36($fp)
    beqz $t0, L8    # branch if false
    li $t0, 4
    sw $t0, 8($fp)
    j L9    # unconditional jump (all vars in memory)
L8:    # merge point — register state invalidated
    li $t0, 99
    sw $t0, 8($fp)
L9:    # merge point — register state invalidated
    j L7    # unconditional jump (all vars in memory)
L6:    # merge point — register state invalidated
    li $t0, 99
    sw $t0, 8($fp)
L7:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    li $t1, 5
    slt $t2, $t0, $t1
    sw $t2, 40($fp)
    lw $t0, 40($fp)
    beqz $t0, L10    # branch if false
    li $t0, 99
    sw $t0, 8($fp)
    j L11    # unconditional jump (all vars in memory)
L10:    # merge point — register state invalidated
    lw $t0, 0($fp)
    li $t1, 8
    slt $t2, $t0, $t1
    sw $t2, 44($fp)
    lw $t0, 44($fp)
    beqz $t0, L12    # branch if false
    li $t0, 99
    sw $t0, 8($fp)
    j L13    # unconditional jump (all vars in memory)
L12:    # merge point — register state invalidated
    lw $t0, 0($fp)
    li $t1, 15
    slt $t2, $t0, $t1
    sw $t2, 48($fp)
    lw $t0, 48($fp)
    beqz $t0, L14    # branch if false
    li $t0, 5
    sw $t0, 8($fp)
    j L15    # unconditional jump (all vars in memory)
L14:    # merge point — register state invalidated
    li $t0, 99
    sw $t0, 8($fp)
L15:    # merge point — register state invalidated
L13:    # merge point — register state invalidated
L11:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    lw $t1, 0($fp)
    slt $t2, $t0, $t1
    sw $t2, 52($fp)
    lw $t0, 52($fp)
    beqz $t0, L16    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L16:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    li $t1, 10
    sle $t2, $t0, $t1
    sw $t2, 56($fp)
    lw $t0, 56($fp)
    beqz $t0, L17    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L17:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    lw $t1, 4($fp)
    sgt $t2, $t0, $t1
    sw $t2, 60($fp)
    lw $t0, 60($fp)
    beqz $t0, L18    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L18:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    li $t1, 10
    sge $t2, $t0, $t1
    sw $t2, 64($fp)
    lw $t0, 64($fp)
    beqz $t0, L19    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L19:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    li $t1, 10
    seq $t2, $t0, $t1
    sw $t2, 68($fp)
    lw $t0, 68($fp)
    beqz $t0, L20    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L20:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    lw $t1, 4($fp)
    sne $t2, $t0, $t1
    sw $t2, 72($fp)
    lw $t0, 72($fp)
    beqz $t0, L21    # branch if false
    li $t0, 6
    sw $t0, 8($fp)
L21:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    lw $a0, 0($fp)
    jal fn_sign_test
    sw $v0, 76($fp)
    lw $t0, 76($fp)
    sw $t0, 8($fp)
    lw $a0, 76($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    lw $t0, 0($fp)
    lw $t1, 4($fp)
    sgt $t2, $t0, $t1
    sw $t2, 80($fp)
    lw $t0, 80($fp)
    beqz $t0, L22    # branch if false
    lw $t0, 4($fp)
    lw $t1, 0($fp)
    sgt $t2, $t0, $t1
    sw $t2, 84($fp)
    lw $t0, 84($fp)
    beqz $t0, L23    # branch if false
    li $t0, 99
    sw $t0, 8($fp)
    j L24    # unconditional jump (all vars in memory)
L23:    # merge point — register state invalidated
    li $t0, 8
    sw $t0, 8($fp)
L24:    # merge point — register state invalidated
L22:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $v0, 0
    li $v0, 10
    syscall
    li $v0, 10
    syscall
