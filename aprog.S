.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 10
    mv t1, t0
    sw t1, -20(s0)
    li t2, 20
    sw t2, -24(s0)
    lw t3, -20(s0)
    lw t4, -24(s0)
    mv t5, t4
    add t6, t3, t5
    sw t6, -28(s0)
    lw t0, -28(s0)
    mv a0, t0
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
