TOOLS=./tools/cross/bin

CC = ${TOOLS}/i586-elf-gcc
AS = ${TOOLS}/i586-elf-as
LD = ${TOOLS}/i586-elf-ld

CFLAGS = -nostdlib -nostartfiles -nodefaultlibs
AFLAGS =

all: tomos

libs:	
	${CC} ${CFLAGS} -o support.o -c support.c
	${CC} ${CFLAGS} -o intr.o -c intr.c
	${CC} ${CFLAGS} -o mm.o -c mm.c
	${CC} ${CFLAGS} -o cio.o -c cio.c 
	${CC} ${CFLAGS} -o pg.o -c pg.c
	${CC} ${CFLAGS}	-I. -o keyboard.o -c ./dev/keyboard.c

tomos: loader.S kernel.c libs
	${CC} ${CFLAGS} -o loader.o -c loader.S
	${CC} ${CFLAGS} -o kernel.o -c kernel.c
	${CC} ${CFLAGS} -o isr_stubs.o -c isr_stubs.S
	${LD} -T linker.ld -o tomos.bin loader.o kernel.o support.o intr.o cio.o mm.o isr_stubs.o pg.o \
			      keyboard.o

floppy:	tomos
	cat stage1 stage2 pad tomos.bin > floppy.img
	dd if=floppy.img of=fd.img conv=notrunc
	rm floppy.img

clean:
	rm *.o *.bin
