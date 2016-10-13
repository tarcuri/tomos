TOOLS=./tools/cross/bin

CPP = ${TOOLS}/i586-pc-tomos-cpp
CC = ${TOOLS}/i586-pc-tomos-gcc
AS = ${TOOLS}/i586-pc-tomos-as
LD = ${TOOLS}/i586-pc-tomos-ld

#CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin-memcpy -fno-builtin-memset -fno-builtin-memmove -fno-builtin-strcpy
CFLAGS = -g -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-hosted
AFLAGS =

LIB_SRC = support.c syscalls.c kernel/interrupt.c kernel/mm.c kernel/paging.c \
          kernel/heap.c kernel/process.c kernel/timer.c kernel/user.c tomsh.c
LIB_OBJ = support.o syscalls.o interrupt.o mm.o paging.o heap.o process.o \
	  timer.o user.o tomsh.o

DRV_SRC = ./dev/keyboard.c ./dev/console.c ./dev/clock.c ./dev/pci.c ./dev/ata.c
DRV_OBJ = keyboard.o console.o clock.o pci.o ata.o

APP_SRC = ./apps/hello_tomos.c
APP_OBJ = hello_tomos.o

# TODO: generate depfiles

BASEDIR=${PWD}

all: tomos

# general libs
libs: ${LIB_SRC} 
	${CC} ${CFLAGS} -o support.o -c support.c
	${CC} ${CFLAGS} -I${BASEDIR} -o cpu.o -c kernel/cpu.c
	${CC} ${CFLAGS} -I${BASEDIR} -o interrupt.o -c kernel/interrupt.c
	${CC} ${CFLAGS} -I${BASEDIR} -o heap.o -c kernel/heap.c
	${CC} ${CFLAGS} -I${BASEDIR} -o mm.o -c kernel/mm.c
	${CC} ${CFLAGS} -I${BASEDIR} -o paging.o -c kernel/paging.c
	${CC} ${CFLAGS} -I${BASEDIR} -o stack.o -c kernel/stack.c
	${CC} ${CFLAGS} -I${BASEDIR} -o process.o -c kernel/process.c
	${CC} ${CFLAGS} -I${BASEDIR} -o user.o -c kernel/user.c
	${CC} ${CFLAGS} -I${BASEDIR} -o timer.o -c kernel/timer.c
	${CC} ${CFLAGS}	-I${BASEDIR} -o syscalls.o -c syscalls.c
	${CC} ${CFLAGS} -I. -o tomsh.o -c tomsh.c

# devices
drivers: ${DRV_SRC} 
	${CC} ${CFLAGS} -I. -o clock.o -c ./dev/clock.c
	${CC} ${CFLAGS} -I. -o console.o -c ./dev/console.c
	${CC} ${CFLAGS}	-I. -o keyboard.o -c ./dev/keyboard.c
	${CC} ${CFLAGS} -I. -o pci.o -c ./dev/pci.c
	${CC} ${CFLAGS} -I. -o ata.o -c ./dev/ata.c
	${CC} ${CFLAGS} -I. -o ext2.o -c ./fs/ext2.c
	${CC} ${CFLAGS} -I. -o vfs.o -c ./fs/vfs.c

hello_tomos.o: apps/hello_tomos.c
	${CC} ${CFLAGS} -I. -o hello_tomos.o -c ./apps/hello_tomos.c

hello_tomos: hello_tomos.o
	${LD} -o hello_tomos hello_tomos.o ./lib/libc.a ./lib/libm.a ./lib/libnosys.a ./lib/libg.a

applications: hello_tomos

# tomos
tomos: libs drivers
	${CC} ${CFLAGS} -o loader.o -c loader.S
	${CC} ${CFLAGS} -I. -o kernel.o -c kernel.c
	${CC} ${CFLAGS} -o isr_stubs.o -c isr_stubs.S
	${CPP} ${CFLAGS} -dM syscalls.h > syscalls_asm.h
	${CC} ${CFLAGS} -o syscall_stubs.o -c syscall_stubs.S
	rm syscalls_asm.h
	${LD} -T linker.ld -o tomos.bin loader.o kernel.o isr_stubs.o syscall_stubs.o \
			     ${LIB_OBJ} ${DRV_OBJ} stack.o ext2.o vfs.o \
                             ./lib/libc.a ./lib/libm.a ./lib/libnosys.a ./lib/libg.a
	cat stage1 stage2 pad tomos.bin > tomos.img
	ls -l tomos.bin | awk '{print int($$5/512)}' | xargs printf '\n>>> tomos is at least %d blocks in size\n\n'

bochs: tomos
	dd if=tomos.img of=fd.img conv=notrunc

vbox: tomos
	dd if=tomos.img of=floppy.img conv=notrunc

qemu: tomos
	dd if=tomos.img of=fd.img conv=notrunc

clean:
	rm *.o *.bin
