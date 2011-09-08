ARG1    = 8                     // Offset to 1st argument
ARG2    = 12                    // Offset to 2nd argument

/*
** Name:        sp_inb, sp_inw, sp_inl
**
** Description: read a single byte, word, or longword from the specified
**              input port
** Arguments:   the port number
** Returns:     the byte, word, or longword value from the port (unused
**              high-order bytes are zeroed)
*/
        .globl  sp_inb, sp_inw, sp_inl


sp_inb:
        enter   $0,$0
        xorl    %eax,%eax       // Clear the high order bytes of %eax
        movl    ARG1(%ebp),%edx // Move port number to %edx
        inb     (%dx)           // Get a byte from the port into %al (low
        leave                   //   byte of %eax)
        ret
sp_inw:
        enter   $0,$0
        xorl    %eax,%eax       // Clear the high order bytes of %eax
        movl    ARG1(%ebp),%edx // Move port number to %edx
        inw     (%dx)           // Get a word from the port into %ax (low
        leave                   //   word of %eax)
        ret
sp_inl:
        enter   $0,$0
        xorl    %eax,%eax       // Clear the high order bytes of %eax
        movl    ARG1(%ebp),%edx // Move port number to %edx
        inl     (%dx)           // Get a longword from the port into %eax
        leave
        ret

/*
** Name:        sp_outb, sp_outw, sp_outl
**
** Description: write a single byte, word, or longword to the specified
**              output port
** Arguments:   the port number, the value to write to it (unused high-order
**              bytes are ignored)
*/
        .globl  sp_outb, sp_outw, sp_outl
sp_outb:
        enter   $0,$0
        movl    ARG1(%ebp),%edx // Get the port number into %edx,
        movl    ARG2(%ebp),%eax //   and the value into %eax
        outb    (%dx)           // Output that byte to the port
        leave                   //   (only %al is sent)
        ret
sp_outw:
        enter   $0,$0
        movl    ARG1(%ebp),%edx // Get the port number into %edx,
        movl    ARG2(%ebp),%eax //   and the value into %eax
        outw    (%dx)           // Output that word to the port.
        leave                   //   (only %ax is sent)
        ret
sp_outl:
        enter   $0,$0
        movl    ARG1(%ebp),%edx // Get the port number into %edx,
        movl    ARG2(%ebp),%eax //   and the value into %eax
        outl    (%dx)           // Output that longword to the port.
        leave
        ret

/*
** sp_get_flags: return the current processor flags
**      int sp_get_flags( void );
*/
        .globl  sp_get_flags

sp_get_flags:
        pushf                   // Push flags on the stack,
        pop     %eax            //   and pop them into eax.
        ret

/*
** sp_pause: halt until something happens
**      void sp_pause( void );
*/
        .globl  sp_pause

sp_pause:
        enter   $0,$0
        sti
        hlt
        leave
        ret

