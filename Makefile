TOOLS=./tools/cross/bin

CC = ${TOOLS}/i586-elf-gcc
AS = ${TOOLS}/i586-elf-as
LD = ${TOOLS}/i586-elf-ld

CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin-memcpy -fno-builtin-memset -fno-builtin-memmove -fno-builtin-strcpy
AFLAGS =

LIB_SRC = support.c intr.c mm.c pg.c heap.c tomsh.c
LIB_OBJ = support.o intr.o mm.o pg.o heap.o tomsh.o

DRV_SRC = ./dev/keyboard.c ./dev/console.c ./dev/clock.c ./dev/pci.c ./dev/ata.c
DRV_OBJ = keyboard.o console.o clock.o pci.o ata.o

# TODO: generate depfiles

all: tomos

# general libs
libs: ${LIB_SRC} 
	${CC} ${CFLAGS} -o support.o -c support.c
	${CC} ${CFLAGS} -o intr.o -c intr.c
	${CC} ${CFLAGS} -o mm.o -c mm.c
	${CC} ${CFLAGS} -o pg.o -c pg.c
	${CC} ${CFLAGS} -o heap.o -c heap.c
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
	dd if=tomos.img of=disk.img conv=notrunc
	dd if=tomos.img of=disk-flat.vmdk conv=notrunc

clean:
	rm *.o *.bin *.img
