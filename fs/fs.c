#include "fs.h"
#include "ext2.h"
#include "kernel/heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dir_t *opendir(uint32_t inode_num)
{
  return find_inode(inode_num);
}

// read a single directory entry
dirent_t *readdir(dir_t *dir)
{
  uint32_t nsectors = ((inode_t *)dir)->blocks;
  uint32_t nblocks = nsectors * (fs_block_size/512);

  // always read in block chunks, ingnoring the extra sector
  if (nsectors % 2)
    nsectors++;

  void *buf = kmalloc(nsectors * 512, 0);
  void *bufp = buf;

  uint32_t bread = 0;
  uint32_t *dblock = &(((inode_t *)dir)->dblock_ptr_0);
  while (bread < nblocks) {
    read_block(*dblock++, bufp, 1);	// 1 at a time for now
    bread++;
    bufp = (void *) (((uint32_t)bufp) + 1024);
  }
}


