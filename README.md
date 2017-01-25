# x86id
A small utility to read the information provided by the CPUID instruction seperate from the included cpuid support in gcc (cpuid.h)

At this time I have only tested the program on intel processors and I am aware that AMD processors will not work correctly with this program.

# Building
There is no makefile currently, to build x86id just build with `gcc -g -Wall -Wextra cpuid.c -o x86id`
