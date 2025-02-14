.option nopic
.attribute arch, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
.attribute unaligned_access, 0
.attribute stack_align, 16
.text
.align 1

.section .text
.globl _start
_start:
la sp, stack_top
call main
li a7, 93   # syscall_exit
ecall
.globl main
.type main, @function
main:
addi sp, sp, -16
sd ra, 8(sp)
sd s0, 0(sp)
addi s0, sp, 16
