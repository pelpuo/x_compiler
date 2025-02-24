.text
.globl main
.type main, @function
test:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    sd a0, -24(s0)
    ld t0, -24(s0)
    li t1, 5
    add t2, t0, t1
    mv a0, t2
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 10
    sd t0, -24(s0)
    ld t1, -24(s0)
    ld t2, -24(s0)
    li t3, 5
    add t4, t2, t3
    sd t4, -32(s0)
    ld t5, -24(s0)
    mv a0, t5
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
