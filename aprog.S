.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 20
    sd t0, -24(s0)
    ld t1, -24(s0)
    li t2, 0
    mv a0, t1
    mv a1, t2
    call test
    mv t3, a0
    sd t3, -24(s0)
    li t4, 69
    mv a0, t4
    call putchar
    mv t5, a0
    li t6, 68
    mv a0, t6
    call putchar
    mv t0, a0
    li t1, 87
    mv a0, t1
    call putchar
    mv t2, a0
    li t3, 49
    mv a0, t3
    call putchar
    mv t4, a0
    li t5, 78
    mv a0, t5
    call putchar
    mv t6, a0
    li t0, 3
    mv a0, t0
    call putchar
    mv t1, a0
    li t2, 10
    mv a0, t2
    call putchar
    mv t3, a0
    ld t4, -24(s0)
    mv a0, t4
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
test:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    sd a0, -24(s0)
    sd a1, -32(s0)
    li t0, 0
    sd t0, -40(s0)
L19:
    ld t1, -40(s0)
    li t2, 5
    slt t3, t1, t2
    beqz t3, L21
    ld t4, -24(s0)
    addi t5, t4, 1
    sd t5, -24(s0)
L20:
    ld t6, -40(s0)
    addi t0, t6, 1
    sd t0, -40(s0)
    j L19
L21:
    ld t1, -24(s0)
    ld t2, -32(s0)
    add t3, t1, t2
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
