.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 5
    sd t0, -24(s0)
    ld t1, -24(s0)
    mv a0, t1
    call fib
    mv t2, a0
    sd t2, -32(s0)
    ld t3, -32(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
fib:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    sd a0, -24(s0)
    ld t0, -24(s0)
    li t1, 1
    slt t2, t1, t0
    xori t2, t2, 1
    beqz t2, L9
L7:
    ld t3, -24(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
    j L8
L9:
    ld t4, -24(s0)
    li t5, 1
    sub t6, t4, t5
    mv a0, t6
    call fib
    mv t0, a0
    ld t1, -24(s0)
    li t2, 2
    sub t3, t1, t2
    mv a0, t3
    call fib
    mv t4, a0
    add t5, t0, t4
    mv a0, t5
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
L8:
