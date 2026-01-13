.data

.text
.globl main
main:
    # Allocate stack space
    addi $sp, $sp, -400

    # Declared x at offset 0
    # Declared y at offset 4
    # Declared z at offset 8
    # Declared w at offset 12
    li $t0, 10
    sw $t0, 0($sp)
    li $t0, 20
    sw $t0, 4($sp)
    lw $t0, 0($sp)
    lw $t1, 4($sp)
    add $t0, $t0, $t1
    sw $t0, 8($sp)
    lw $t0, 8($sp)
    # Print integer
    move $a0, $t0
    li $v0, 1
    syscall
    # Print newline
    li $v0, 11
    li $a0, 10
    syscall
    lw $t0, 8($sp)
    li $t1, 5
    sub $t0, $t0, $t1
    sw $t0, 0($sp)
    lw $t0, 0($sp)
    # Print integer
    move $a0, $t0
    li $v0, 1
    syscall
    # Print newline
    li $v0, 11
    li $a0, 10
    syscall
    lw $t0, 0($sp)
    lw $t1, 4($sp)
    add $t0, $t0, $t1
    li $t1, 2
    mult $t0, $t1
    mflo $t0
    sw $t0, 12($sp)
    lw $t0, 12($sp)
    # Print integer
    move $a0, $t0
    li $v0, 1
    syscall
    # Print newline
    li $v0, 11
    li $a0, 10
    syscall
    lw $t0, 12($sp)
    lw $t1, 4($sp)
    div $t0, $t1
    mflo $t0
    sw $t0, 8($sp)
    lw $t0, 8($sp)
    # Print integer
    move $a0, $t0
    li $v0, 1
    syscall
    # Print newline
    li $v0, 11
    li $a0, 10
    syscall
    lw $t0, 0($sp)
    lw $t1, 4($sp)
    lw $t2, 8($sp)
    mult $t1, $t2
    mflo $t1
    add $t0, $t0, $t1
    li $t1, 10
    li $t2, 2
    div $t1, $t2
    mflo $t1
    sub $t0, $t0, $t1
    sw $t0, 0($sp)
    lw $t0, 0($sp)
    # Print integer
    move $a0, $t0
    li $v0, 1
    syscall
    # Print newline
    li $v0, 11
    li $a0, 10
    syscall

    # Exit program
    addi $sp, $sp, 400
    li $v0, 10
    syscall
