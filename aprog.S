.data
.globl counter
counter:
    .word 7
.globl counter2
counter2:
    .quad 7
.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    la t6, counter
    lw t0, 0(t6)
    la t6, counter2
    ld t1, 0(t6)
    add t2, t0, t1
    sw t2, -20(s0)
    li t3, 12
    sw t3, -24(s0)
    li t4, 32
    sw t4, -28(s0)
    lw t5, -28(s0)
    lw t6, -24(s0)
    add t0, t5, t6
    sd t0, -36(s0)
    lw t1, -20(s0)
    mv a0, t1
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
