#include "fs.h"
#include "kernel/heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: need file type checks!
dir_t *opendir(uint32_t inode_num)
{
  uint32_t table_block = open_inode_table(inode_num);

  // read in the inode table for this dir
  dir_t *dir = (dir_t *) kmalloc(sizeof(dir_t), 0);

  dir->inode_table = read_inode_table(fs_dev, table_block);

  dir->curr_idx    = get_block_index(inode_num);
  dir->curr_inode  = &(dir->inode_table[dir->curr_idx]);

  printf("opening %d (%d)\n", inode_num, dir->curr_idx);

  // read in the directory data blocks
  dir->dblock_bufsize = dir->curr_inode->blocks * 512;
  dir->dblock_buffer  = kmalloc(dir->dblock_bufsize, 0);
  void *bufp = dir->dblock_buffer;

  printf("directory has %d blocks [%d bytes]\n", dir->curr_inode->blocks, dir->dblock_bufsize);

  uint32_t bread = 0;
  uint32_t nblocks = dir->dblock_bufsize / fs_block_size;
  uint32_t *dblock = &(dir->curr_inode->dblock_ptr_0);
  while (bread < nblocks) {
    printf("reading dblock %d [0x%x]\n", *dblock, dblock);
    read_block(*dblock++, bufp, 1);
    bread++;
    bufp = (void *) (((uint32_t)bufp) + 1024);
  }

  dir->curr_ent = (dirent_t *) dir->dblock_buffer;
  return dir;
}

int closedir(dir_t *dir)
{
  kfree(dir->inode_table);
  kfree(dir->dblock_buffer);
  kfree(dir);

  return 0;
}

// read a single directory entry
dirent_t *readdir(dir_t *dir)
{
  if (dir->curr_ent >= (dirent_t *) (((uint32_t)dir->dblock_buffer) + dir->dblock_bufsize))
    return NULL;

  dirent_t *to_read = dir->curr_ent;
  dir->curr_ent = (dirent_t *) (((uint32_t)dir->curr_ent) + dir->curr_ent->rec_len);

  return to_read;
}


