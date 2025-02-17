.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    sd t0, -8(s0)
    ld t1, -8(s0)
    li t2, 3
    bge t1, t2, L4
    ld t3, -8(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
    j L5
L4:
    li t4, 8
    sd t4, -16(s0)
L5:
    ld t5, -8(s0)
    add t6, t5, t0
    mv a0, t6
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
    li t1, 0
    mv a0, t1
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
