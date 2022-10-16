phony: all
all: lib.inc
	nasm -felf64 $<
	ld -o run lib.o 
