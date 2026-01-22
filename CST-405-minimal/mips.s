.data

.text
.globl main
main:
    # Allocate stack space
    addi $sp, $sp, -400

    # Declared x (int) at offset 0
    # Declared y (int) at offset 4
    # Declared z (int) at offset 8
    li $t0, 3
    sw $t0, 0($sp)
    lw $t0, 0($sp)
    li $t1, 5
    add $t2, $t0, $t1
    sw $t2, 4($sp)
    lw $t0, 4($sp)
    move $a0, $t0
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall

    # Exit program
    addi $sp, $sp, 400
    li $v0, 10
    syscall
