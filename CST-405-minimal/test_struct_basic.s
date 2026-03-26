.data

.text
.globl main

main:
    subu $sp, $sp, 32
    sw $ra, 28($sp)
    sw $fp, 24($sp)
    move $fp, $sp
    li $t0, 3
    sw $t0, 0($fp)
    li $t0, 7
    sw $t0, 4($fp)
    lw $t0, 0($fp)
    sw $t0, 8($fp)
    lw $t0, 4($fp)
    sw $t0, 12($fp)
    lw $t0, 8($fp)
    lw $t1, 12($fp)
    add $t2, $t0, $t1
    sw $t2, 16($fp)
    lw $a0, 16($fp)
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
