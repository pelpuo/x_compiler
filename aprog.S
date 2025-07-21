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
    li t0, 0
    sd t0, -24(s0)
    li t1, 0
    sd t1, -32(s0)
L4:
    ld t2, -32(s0)
    li t3, 5
    slt t4, t3, t2
    xori t4, t4, 1
    beqz t4, L6
    ld t5, -24(s0)
    mv a0, t5
    call test
    mv t6, a0
    sd t6, -24(s0)
L5:
    ld t0, -32(s0)
    li t1, 1
    add t2, t0, t1
    sd t2, -32(s0)
    j L4
L6:
    ld t3, -24(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
