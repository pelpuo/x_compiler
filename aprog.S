.text
.globl main
.type main, @function
fac:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    sd a0, -24(s0)
    li t0, 1
    sw t0, -28(s0)
    li t1, 1
    sw t1, -32(s0)
L1:
    lw t2, -32(s0)
    lw t3, -24(s0)
    slt t4, t3, t2
    xori t4, t4, 1
    beqz t4, L3
    lw t5, -32(s0)
    lw t6, -28(s0)
    mul t0, t6, t5
    sw t0, -28(s0)
L2:
    lw t1, -32(s0)
    addi t2, t1, 1
    sw t2, -32(s0)
    j L1
L3:
    lw t3, -28(s0)
    mv a0, t3
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
    sw t0, -20(s0)
    li t1, 5
    mv a0, t1
    call fac
    mv t2, a0
    sw t2, -20(s0)
    lw t3, -20(s0)
    mv a0, t3
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
