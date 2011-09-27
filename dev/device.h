#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_BLOCK	0x01
#define DEVICE_CHAR	0x02

typedef struct device
{
  unsigned char type;

  int (*_read)(int);
  int (*_write)(int);
  int (*_ctrl)(unsigned int, void *);
} device_t;

#endif
