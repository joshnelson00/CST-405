.data

.text
.globl main

fn_sum:
    subu $sp, $sp, 24
    sw $ra, 20($sp)
    sw $fp, 16($sp)
    move $fp, $sp
    sw $a0, 0($fp)
    lw $t1, 0($fp)
    lw $t0, 0($t1)
    sw $t0, 4($fp)
    lw $t1, 0($fp)
    lw $t0, 4($t1)
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    lw $t1, 8($fp)
    add $t2, $t0, $t1
    sw $t2, 12($fp)
    lw $v0, 12($fp)
    lw $ra, 20($sp)
    lw $fp, 16($sp)
    addu $sp, $sp, 24
    jr $ra
main:
    subu $sp, $sp, 48
    sw $ra, 44($sp)
    sw $fp, 40($sp)
    move $fp, $sp
    li $t0, 4
    sw $t0, 0($fp)
    li $t0, 7
    sw $t0, 4($fp)
    addiu $t0, $fp, 0
    sw $t0, 28($fp)
    lw $t0, 28($fp)
    sw $t0, 8($fp)
    lw $a0, 8($fp)
    jal fn_sum
    sw $v0, 32($fp)
    lw $t0, 32($fp)
    sw $t0, 12($fp)
    lw $a0, 32($fp)
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    lw $t1, 8($fp)
    lw $t0, 0($t1)
    sw $t0, 36($fp)
    lw $a0, 36($fp)
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
