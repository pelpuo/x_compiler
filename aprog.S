.text
.globl main
.type main, @function
main:
    addi sp, sp, -128
    sd ra, 120(sp)
    sd s0, 112(sp)
    addi s0, sp, 128
    li t0, 5
    mv a0, t0
    ld ra, 120(sp)
    ld s0, 112(sp)
    addi sp, sp, 128
    ret
