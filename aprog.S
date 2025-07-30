.data
.globl counter
counter:
    .quad 7
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
    li t0, 0
    sd t0, -24(s0)
    li t1, 12
    sd t1, -24(s0)
    la t6, counter
    lw t2, 0(t6)
    la t6, counter2
    ld t3, 0(t6)
    add t4, t2, t3
    sw t4, -28(s0)
    lw t5, -28(s0)
    mv a0, t5
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
