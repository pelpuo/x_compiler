.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 10
    sd t0, -8(s0)
    li t1, 0
    sd t1, -16(s0)
L1:
    ld t2, -16(s0)
    li t3, 10
    slt t4, t2, t3
    beqz t4, L3
    ld t5, -8(s0)
    ld t6, -16(s0)
    add t0, t5, t6
    sd t0, -24(s0)
L2:
    ld t1, -16(s0)
    ld t2, -16(s0)
    li t3, 1
    add t4, t2, t3
    sd t4, -32(s0)
    j L1
L3:
    ld t5, -8(s0)
    mv a0, t5
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
