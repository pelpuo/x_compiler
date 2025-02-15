.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    seqz t1, t0
    li t2, 2
    li t3, 3
    mul t4, t2, t3
    li t5, 4
    beq t5, t6, t7
    li t8, 5
    mv t9, t8
    j t10
L6:
    li t9, 0
    j t10
L7:
    li t9, 1
L8:
    div t11, t4, t9
    sub t12, t1, t11
    mv a0, t12
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
