#ifndef SUPPORT_H
#define SUPPORT_H

void sp_inb(int port);
void sp_inb(int port);
void sp_inb(int port);

void sp_outb(int port, int value);
void sp_outw(int port, int value);
void sp_outl(int port, int value);

/* get EFLAGS register */
int sp_get_flags(void);

void sp_pause(void);

#endif
