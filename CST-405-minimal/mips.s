.data
str_0: .asciiz "
"

.text
.globl main

main:
    subu $sp, $sp, 24
    sw $ra, 20($sp)
    sw $fp, 16($sp)
    move $fp, $sp
    li $t0, 5
    sw $t0, 0($fp)
    li $t0, 1
    sw $t0, 8($fp)
    li $a0, 1
    li $v0, 1
    syscall
L0:
    la $a0, str_0
    li $v0, 4
    syscall
    li $v0, 10
    syscall
