CFLAGS = -m32 -nostdlib -nostartfiles -nodefaultlibs
AFLAGS = --32

all: tromos

libs:	cio.c
	gcc ${CFLAGS} -o support.o -c support.S
	gcc ${CFLAGS} -o cio.o -c cio.c 

tromos: loader.S kmain.c libs
	gcc ${CFLAGS} -o loader.o -c loader.S
	gcc ${CFLAGS} -o kmain.o -c kmain.c
	ld -m elf_i386 -T linker.ld -o kmain.bin loader.o kmain.o support.o cio.o

floppy:	tromos
	cat stage1 stage2 pad kmain.bin > floppy.img
	dd if=floppy.img of=fd.img conv=notrunc
