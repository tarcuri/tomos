TOOLS=./tools/cross/bin

CC = ${TOOLS}/i586-elf-gcc
AS = ${TOOLS}/i586-elf-as
LD = ${TOOLS}/i586-elf-ld

CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin-memcpy -fno-builtin-memset -fno-builtin-memmove -fno-builtin-strcpy
AFLAGS =

LIB_SRC = support.c kernel/interrupt.c kernel/mm.c kernel/pg.c kernel/heap.c kernel/process.c tomsh.c
LIB_OBJ = support.o interrupt.o mm.o pg.o heap.o process.o tomsh.o

DRV_SRC = ./dev/keyboard.c ./dev/console.c ./dev/clock.c ./dev/pci.c ./dev/ata.c
DRV_OBJ = keyboard.o console.o clock.o pci.o ata.o

# TODO: generate depfiles

BASEDIR=${PWD}

all: tomos

# general libs
libs: ${LIB_SRC} 
	${CC} ${CFLAGS} -o support.o -c support.c
	${CC} ${CFLAGS} -I${BASEDIR} -o interrupt.o -c kernel/interrupt.c
	${CC} ${CFLAGS} -I${BASEDIR} -o heap.o -c kernel/heap.c
	${CC} ${CFLAGS} -I${BASEDIR} -o mm.o -c kernel/mm.c
	${CC} ${CFLAGS} -I${BASEDIR} -o pg.o -c kernel/pg.c
	${CC} ${CFLAGS} -I${BASEDIR} -o process.o -c kernel/process.c
	${CC} ${CFLAGS} -o tomsh.o -c tomsh.c

# devices
drivers: ${DRV_SRC} 
	${CC} ${CFLAGS} -I. -o clock.o -c ./dev/clock.c
	${CC} ${CFLAGS} -I. -o console.o -c ./dev/console.c
	${CC} ${CFLAGS}	-I. -o keyboard.o -c ./dev/keyboard.c
	${CC} ${CFLAGS} -I. -o pci.o -c ./dev/pci.c
	${CC} ${CFLAGS} -I. -o ata.o -c ./dev/ata.c

# tomos
tomos: libs drivers
	${CC} ${CFLAGS} -o loader.o -c loader.S
	${CC} ${CFLAGS} -o kernel.o -c kernel.c
	${CC} ${CFLAGS} -o isr_stubs.o -c isr_stubs.S
	${LD} -T linker.ld -o tomos.bin loader.o kernel.o isr_stubs.o \
			     ${LIB_OBJ} ${DRV_OBJ} 
	cat stage1 stage2 pad tomos.bin > tomos.img

bochs: tomos
	dd if=tomos.img of=disk.img conv=notrunc

vbox: tomos
	dd if=tomos.img of=disk-flat.vmdk conv=notrunc

qemu: tomos
	dd if=tomos.img of=qemu-disk.raw conv=notrunc

clean:
	rm *.o *.bin
