.text
.globl main
main:
    addi sp, sp, -64
    sd ra, 56(sp)
    sd s0, 48(sp)
    addi s0, sp, 64
    li t0, 2
    neg t1, t0
    not t2, t1
    mv a0, t2
    ld ra, 56(sp)
    ld s0, 48(sp)
    addi sp, sp, 64
    ret
