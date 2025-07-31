.text
.globl main
.type main, @function
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 1
    sw t0, -20(s0)
    li t1, 5
    sw t1, -24(s0)
    li t2, 0
    sw t2, -28(s0)
    lw t3, -20(s0)
    lw t4, -24(s0)
    mul t5, t3, t4
    sw t5, -28(s0)
    li t6, 1
    sw t6, -32(s0)
L5:
    lw t0, -32(s0)
    li t1, 10
    slt t2, t1, t0
    xori t2, t2, 1
    beqz t2, L7
    lw t3, -20(s0)
    lw t4, -32(s0)
    mul t5, t3, t4
    sw t5, -20(s0)
L6:
    lw t6, -32(s0)
    li t0, 1
    add t1, t6, t0
    sw t1, -32(s0)
    j L5
L7:
    lw t2, -20(s0)
    mv a0, t2
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
