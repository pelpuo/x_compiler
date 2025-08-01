.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1.500000
    sd t0, -24(s0)
    li t1, 2.500000
    sd t1, -32(s0)
    ld t2, -24(s0)
    ld t3, -32(s0)
    add t4, t2, t3
    sd t4, -40(s0)
    ld t5, -40(s0)
    mv a0, t5
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
