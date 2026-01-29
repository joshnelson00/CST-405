.data

.text
.globl main


main:
    li $a0, 5
    li $a1, 3
    jal adding
    move $a0, $v0
    li $v0, 1
    syscall
    li $v0, 11
    li $a0, 10
    syscall
    li $v0, 10
    syscall

adding:
    add $v0, $a0, $a1
    jr $ra

multiply:
    mult $a0, $a1
    mflo $v0
    jr $ra
