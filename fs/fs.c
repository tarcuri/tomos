#include "fs.h"
#include "kernel/heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_fs()
{
  printf("Listing root directory\n");
  dir_t *dir = opendir(2);	// open the root directory

  // list its contents
  dirent_t *ent = readdir(dir);
  while (ent != NULL) {
    printf("  %d: %s\n", ent->inode, ent->name);
    ent = readdir(dir);
  }
}

// TODO: need file type checks!
dir_t *opendir(uint32_t inode_num)
{
  uint32_t table_block = open_inode_table(inode_num);

  // read in the inode table for this dir
  dir_t *dir = (dir_t *) kmalloc(sizeof(dir_t), 0);

  dir->inode_table = read_inode_table(fs_dev, table_block);

  printf("<GOT INODE TABLE>\n");
  dir->curr_idx    = get_block_index(inode_num);
  dir->curr_inode  = &(dir->inode_table[dir->curr_idx]);

  // read in the directory data blocks
  dir->dblock_bufsize = dir->curr_inode->blocks * 512;
  dir->dblock_buffer  = kmalloc(dir->dblock_bufsize, 0);
  void *bufp = dir->dblock_buffer;

  uint32_t bread = 0;
  uint32_t nblocks = dir->curr_inode->blocks * (fs_block_size/512);
  uint32_t *dblock = &(dir->curr_inode->dblock_ptr_0);
  while (bread < nblocks) {
    read_block(*dblock++, bufp, 1);
    bread++;
    bufp = (void *) (((uint32_t)bufp) + 1024);
  }

  dir->curr_ent = (dirent_t *) dir->dblock_buffer;
  return dir;
}

// read a single directory entry
dirent_t *readdir(dir_t *dir)
{
  if (dir->curr_ent > (dirent_t *) (((uint32_t)dir->dblock_buffer) + dir->dblock_bufsize))
    return NULL;

  dirent_t *to_read = dir->curr_ent;
  dir->curr_ent = (dirent_t *) (((uint32_t)dir->curr_ent) + dir->curr_ent->rec_len);

  return to_read;
}


