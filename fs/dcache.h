#ifndef DENTRY_CACHE_H
#define DENTRY_CACHE_H

#include <stdint.h>

#define DCACHE_SIZE	32

static struct dentry_cache 
{
  uint32_t *	table;
  uint32_t	size;
} dcache;

// dentry cache
uint32_t djb2_hash(uint8_t *s)
{
  uint32_t hash = 5381;

  while (*s)
    hash = ((hash << 5) + hash) + *s++;

  return hash;
}

void dcache_insert(const char *s, uint32_t ino)
{
  uint32_t hashval = djb2_hash(s) % DCACHE_SIZE;

  dcache.table[hashval] = ino;
  if (dcache.table[hashval] == 0)
    dcache.size++;
}

uint32_t dcache_lookup(const char *s)
{
  return dcache.table[hash(s) % DCACHE_SIZE]; 
}

#endif
