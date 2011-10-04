#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#define DEVICE_BLOCK	0x01
#define DEVICE_CHAR	0x02

typedef struct device
{
  uint8_t type;

  uint32_t state;	// device specific state
  uint32_t reg;		// device specific command/data register

  int32_t (*_read)(int32_t);
  int32_t (*_write)(int32_t);
  int32_t (*_ctrl)(uint32_t, void *);
} device_t;

#endif
