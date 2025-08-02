.section .rodata
.LD0:
    .double 1.500000
.LD1:
    .double 2.500000
.text
.globl main
.type main, @function
main:
    addi sp, sp, -128
    sd ra, 120(sp)
    sd s0, 112(sp)
    addi s0, sp, 128
    la t6, .LD0
    fld ft0, 0(t6)
    fsd ft0, -32(s0)
    la t6, .LD1
    fld ft1, 0(t6)
    fsd ft1, -40(s0)
    fld ft2, -32(s0)
    fld ft3, -40(s0)
    fadd.d ft4, ft2, ft3
    fsd ft4, -48(s0)
    fld ft5, -48(s0)
    fmv.d fa0, ft5
    ld ra, 120(sp)
    ld s0, 112(sp)
    addi sp, sp, 128
    ret
