.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    li t1, 2
    li t2, 3
    mul t3, t1, t2
    li t4, 4
    beq t4, t5, t6
    li t7, 5
    mv t8, t7
    j t9
L5:
    li t8, 0
    j t9
L6:
    li t8, 1
L7:
    div t10, t3, t8
    sub t11, t0, t10
    mv a0, t11
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
