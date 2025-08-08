.text
.globl main
.type main, @function
main:
    addi sp, sp, -128
    sd ra, 120(sp)
    sd s0, 112(sp)
    addi s0, sp, 128
    li t0, 50
    sw t0, -28(s0)
    li t1, 0
    sw     lw t1, -32(s0)
t1, -32(s0)
    li t2, 0
    sw     lw t2, -36(s0)
t2, -36(s0)
    li t3, 0
    sd     ld t3, -44(s0)
t3, -44(s0)
    lea t4, -32(s0)
    sd t4, -44(s0)
    li t5, 100
    ld t6, -44(s0)
    sd t5, 0(t6)
    ld t0, -44(s0)
    ld t1, 0(t0)
    sw t1, -36(s0)
    lea t2, -28(s0)
    sd t2, -44(s0)
    lw t3, -36(s0)
    ld t4, -44(s0)
    ld t5, 0(t4)
    add t6, t3, t5
    sw t6, -36(s0)
    lw t0, -36(s0)
    mv a0, t0
    ld ra, 120(sp)
    ld s0, 112(sp)
    addi sp, sp, 128
    ret
