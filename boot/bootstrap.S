/*
** bootstrap.S
**
** g++ -c -o bootstrap.s bootstrap.S
** ld -Ttext 0x7C00 -Tdata 0x7C00 -s --oformat binary bootstrap.s -o bootstrap.bin
** dd if=bootstrap.bin of=disk.img conv=notrunc
*/

	.code16
	.text

msg_hello:
	.ascii "Hello, TROMOS\n\r"

	.globl _start
_start:
	movw	$0x07C0, %ax	/* must first set data segment */
	movw	%ax, %ds

	movb	$0x03, %ah		/* get cursor position/size (ret: CX, DX) */
	int	$0x10

	addb	$0x01, %dh

	movb	$0x0E, %bl		/* color arg */

	movw	$msg_hello, %bp
	movw	$0x0F, %cx
	movw	$0x1301, %ax
	int	$0x10			/* write string */

	jmp	.

	.org	510
	.word	0xAA55
