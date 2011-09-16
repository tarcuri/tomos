#ifndef TOM_SHELL_H
#define TOM_SHELL_H

char prompt[64];
char command_line[512];

void command_loop(void);

#endif
