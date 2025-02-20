.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 10
    sd t0, -8(s0)
    ld t1, -8(s0)
    li t2, 1
    beq t1, t2, L2
    li t3, 2
    beq t1, t3, L3
    j L4
L2:
    ld t4, -8(s0)
    li t5, 1
    sd t5, -16(s0)
    j L5
L3:
    ld t6, -8(s0)
    li t0, 2
    sd t0, -24(s0)
    j L5
L4:
    ld t1, -8(s0)
    li t2, 3
    sd t2, -32(s0)
L5:
    ld t3, -8(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
