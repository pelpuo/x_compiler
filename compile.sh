#!/bin/bash
clear && make && ./comp program.c
riscv64-unknown-linux-gnu-as -o aprog.o aprog.S
riscv64-unknown-linux-gnu-gcc -o aprog aprog.o -march=rv64imafd -mabi=lp64d -static
qemu-riscv64 aprog

echo
echo "Executing Program..."

echo "Return : " $?