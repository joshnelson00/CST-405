.data

.text
.globl main

fn_square:
    subu $sp, $sp, 16
    sw $ra, 12($sp)
    sw $fp, 8($sp)
    move $fp, $sp
    sw $a0, 0($fp)
    lw $t0, 0($fp)
    lw $t1, 0($fp)
    mult $t0, $t1
    mflo $t2
    sw $t2, 4($fp)
    lw $v0, 4($fp)
    lw $ra, 12($sp)
    lw $fp, 8($sp)
    addu $sp, $sp, 16
    jr $ra
main:
    subu $sp, $sp, 72
    sw $ra, 68($sp)
    sw $fp, 64($sp)
    move $fp, $sp
    li $t0, 42
    sw $t0, 20($fp)
    li $t0, 42
    sw $t0, 0($fp)
    li $a0, 42
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 1
    sw $t0, 24($fp)
    li $a0, 1
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    j L1    # unconditional jump (all vars in memory)
L0:    # merge point — register state invalidated
    li $a0, 0
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
L1:    # merge point — register state invalidated
    lw $t0, 0($fp)
    li $t1, 50
    sgt $t2, $t0, $t1
    sw $t2, 28($fp)
    lw $t0, 28($fp)
    beqz $t0, L2    # branch if false
    li $a0, 1
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    j L3    # unconditional jump (all vars in memory)
L2:    # merge point — register state invalidated
    li $a0, 0
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
L3:    # merge point — register state invalidated
    li $t0, 0
    sw $t0, 8($fp)
    li $t0, 1
    sw $t0, 4($fp)
L4:    # merge point — register state invalidated
    lw $t0, 4($fp)
    li $t1, 4
    sle $t2, $t0, $t1
    sw $t2, 32($fp)
    lw $t0, 32($fp)
    beqz $t0, L5    # branch if false
    lw $t0, 8($fp)
    lw $t1, 4($fp)
    add $t2, $t0, $t1
    sw $t2, 36($fp)
    lw $t0, 36($fp)
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    li $t1, 1
    add $t2, $t0, $t1
    sw $t2, 40($fp)
    lw $t0, 40($fp)
    sw $t0, 4($fp)
    j L4    # unconditional jump (all vars in memory)
L5:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $t0, 0
    sw $t0, 8($fp)
    li $t0, 1
    sw $t0, 4($fp)
L6:    # merge point — register state invalidated
    lw $t0, 4($fp)
    li $t1, 5
    sle $t2, $t0, $t1
    sw $t2, 44($fp)
    lw $t0, 44($fp)
    beqz $t0, L7    # branch if false
    lw $t0, 8($fp)
    lw $t1, 4($fp)
    add $t2, $t0, $t1
    sw $t2, 48($fp)
    lw $t0, 48($fp)
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    li $t1, 1
    add $t2, $t0, $t1
    sw $t2, 52($fp)
    lw $t0, 52($fp)
    sw $t0, 4($fp)
    j L6    # unconditional jump (all vars in memory)
L7:    # merge point — register state invalidated
    lw $a0, 8($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $a0, 5
    jal fn_square
    sw $v0, 56($fp)
    lw $t0, 56($fp)
    sw $t0, 12($fp)
    lw $a0, 56($fp)
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
