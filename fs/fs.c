#include "fs.h"
#include "ext2.h"
#include "kernel/heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dir_entry_t *opendir(uint32_t inode_id)
{
  inode_t *inode = find_inode(inode_id);

  void *buf = kmalloc(1024, 0);
  read_block(inode->dblock_ptr_0, buf, 1);

  uint32_t name_len = ((dir_entry_t *) buf)->name_len + 1;
  dir = kmalloc(sizeof(dir_entry_t) + name_len, 0);
  memcpy((void *) dir, buf, name_len); 

  kfree(buf);
  return dir;
}
