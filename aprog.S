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
    lw t1, -20(s0)
    mv a0, t1
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
