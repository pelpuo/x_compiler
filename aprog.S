.data
counter:
    .word 7
.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 121
    sd t0, -24(s0)
    li t1, -2009260032
    sd t1, -32(s0)
    li t2, 30
    sd t2, -40(s0)
    ld t3, -24(s0)
    ld t4, -32(s0)
    add t5, t3, t4
    sd t5, -48(s0)
    ld t6, -24(s0)
    mv a0, t6
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
