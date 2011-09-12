#ifndef DEVICE_H
#define DEVICE_H

typedef struct device
{
  unsigned char type;

  int (*_open)(int);
  int (*_read)(int);
  int (*_write)(int);
  int (*_ctrl)(unsigned int, void *);
} device_t;

#endif
