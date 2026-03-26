.data

.text
.globl main

fn_computeArea:
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
    mult $t0, $t1
    mflo $t2
    sw $t2, 12($fp)
    lw $t0, 12($fp)
    lw $t1, 0($fp)
    sw $t0, 8($t1)
    lw $ra, 20($sp)
    lw $fp, 16($sp)
    addu $sp, $sp, 24
    jr $ra
main:
    subu $sp, $sp, 40
    sw $ra, 36($sp)
    sw $fp, 32($sp)
    move $fp, $sp
    li $t0, 6
    sw $t0, 0($fp)
    li $t0, 7
    sw $t0, 4($fp)
    addiu $t0, $fp, 0
    sw $t0, 24($fp)
    lw $a0, 24($fp)
    jal fn_computeArea
    lw $t0, 8($fp)
    sw $t0, 28($fp)
    lw $a0, 28($fp)
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
