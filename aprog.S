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
    li t0, 5
    sd t0, -24(s0)
    la t6, counter
    ld t1, 0(t6)
    mv a0, t1
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
