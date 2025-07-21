.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 20
    sd t0, -24(s0)
    ld t1, -24(s0)
    addi t2, t1, -1
    sd t2, -24(s0)
    ld t3, -24(s0)
    addi t4, t3, -1
    sd t4, -24(s0)
    ld t5, -24(s0)
    addi t6, t5, -1
    sd t6, -24(s0)
    ld t0, -24(s0)
    addi t1, t0, -1
    sd t1, -24(s0)
    ld t2, -24(s0)
    addi t3, t2, -1
    sd t3, -24(s0)
    ld t4, -24(s0)
    mv a0, t4
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
