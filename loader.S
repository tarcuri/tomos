.global loader                 # making entry point visible to linker
# as --32 -o loader.o loader.S

# setting up the Multiboot header - see GRUB docs for details
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum required

.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

	.data
/*
** GDT
*/
	.globl	gdt_start
gdt_start:
	.word	0,0,0,0		# null descriptor

gdt_code_seg:       /* limit FFFFF, base 0, R/E code seg, 32-bit 4K */
        .word   0xFFFF
        .word   0x0000
        .byte   0x00
        .byte   0x9A    /* 1 00 1 1010: present, prio 0, C/D, R/E code */
        .byte   0xCF    /* 1 1 00 1111: 4K, 32-bit, 0, 0, limit[19:16] */
        .byte   0x00

gdt_data_seg:       /* limit FFFFF, base 0, R/W data seg, 32-bit 4K */
        .word   0xFFFF
        .word   0x0000
        .byte   0x00
        .byte   0x92    /* 1 00 1 0010: present, prio 0, C/D, R/W data */
        .byte   0xCF
        .byte   0x00

gdt_stack_seg:      /* limit FFFFF, base 0, R/W data seg, 32-bit 4K */
        .word   0xFFFF  /* limit[15:0] */
        .word   0x0000  /* base[15:0] */
        .byte   0x00    /* base[23:16] */
        .byte   0x92    /* access byte */
        .byte   0xCF    /* granularity */
        .byte   0x00    /* base[31:24] */

gdt_end:
gdt_len = gdt_end - gdt_start

/*
** Next, the GDTR and IDTR contents.
*/
	.global	gdt_48
gdt_48:
	.word   0
	.word	0,0

# reserve initial kernel stack space
	.set	STACKSIZE, 0x4000          # that is, 16k.
	.globl	stack_start
stack_start:
	.comm	stack, STACKSIZE, 32      # reserve 16k stack on a quadword boundary

	.text

loader:
	mov	$(stack + STACKSIZE), %esp 	# set up the stack
	mov	$stack, %ebp
	mov	%ebp, %ecx			# pass on the base pointer
	push	%ecx
	push	%eax                       	# Multiboot magic number
	push	%ebx                   		# Multiboot data structure

	# What do we know?
	# + cr0: VM, IF are cleared
	# + eflags: PG cleared, PE set (protected mode)

	# now we need to setup the GDTR
	movl	$gdt_start, %eax
	movl	$gdt_48, %ebx
 	movl	%eax, 2(%ebx)
	movw	$(gdt_end - gdt_start), %ax
	movw	%ax, (%ebx)
        lgdt	gdt_48

# Do we need to reset CS?
#        .byte   0x66, 0xEA      /* 32-bit mode, jmp */
#        .long   reload_cs	/* target address */
#        .word   0x0008          /* descriptor #1, GDT, RPL=0 */
#reload_cs:
	mov	$0x10, %eax
	mov	%eax, %ds
	mov	%eax, %es
	mov	%eax, %fs
	mov	%eax, %gs
	mov	$0x18, %eax
	mov	%eax, %ss

	call	kernel				# call kernel proper

	cli

	hlt                    # halt machine should kernel return
	jmp	.

