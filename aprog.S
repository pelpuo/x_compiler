.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    li t1, 2
    sub t2, t0, t1
    li t3, 3
    mul t4, t2, t3
    li t5, 4
    li t6, 5
    div t7, t4, t8
    mv a0, t7
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
