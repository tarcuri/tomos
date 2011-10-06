#ifndef DENTRY_CACHE_H
#define DENTRY_CACHE_H

#include <stdint.h>

struct dhash
{
  void **	table;
  uint32_t	size;
};

// dentry cache
uint32_t djb2_hash(uint8_t *s)
{
  uint32_t hash = 5381;

  while (*s)
    hash = ((hash << 5) + hash) + *s++;

  return hash;
}

#endif
