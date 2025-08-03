	.file	"program.c"
	.option nopic
	.attribute arch, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-48
	sd	s0,40(sp)
	addi	s0,sp,48
	lui	a5,%hi(.LC0)
	fld	fa5,%lo(.LC0)(a5)
	fsd	fa5,-24(s0)
	lui	a5,%hi(.LC1)
	fld	fa5,%lo(.LC1)(a5)
	fsd	fa5,-32(s0)
	fld	fa4,-24(s0)
	fld	fa5,-32(s0)
	fadd.d	fa5,fa4,fa5
	fsd	fa5,-40(s0)
	fld	fa5,-40(s0)
	fcvt.w.d a5,fa5,rtz
	sext.w	a5,a5
	mv	a0,a5
	ld	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	main, .-main
	.section	.rodata
	.align	3
.LC0:
	.word	0
	.word	1073217536
	.align	3
.LC1:
	.word	0
	.word	1074003968
	.ident	"GCC: () 10.2.0"
