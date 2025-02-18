.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    sd t0, -8(s0)
    li t1, 0
    sd t1, -16(s0)
    ld t2, -8(s0)
    li t3, 3
    slt t4, t3, t2
    xori t4, t4, 1
    beqz t4, L7
L5:
    li t5, 5
    sd t5, -8(s0)
    ld t6, -8(s0)
    mv a0, t6
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
    j L6
L7:
    li t0, 8
    sd t0, -16(s0)
L6:
    ld t1, -8(s0)
    ld t2, -16(s0)
    add t3, t1, t2
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
