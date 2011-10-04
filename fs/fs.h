#ifndef FS_H
#define FS_H

#include "ext2.h"

typedef struct dir_stream
{
  inode_t *inode_table;		// allocated by read_inode_table
  void *dblock_buffer;		// temp. hold this inodes data blocks
  uint32_t dblock_bufsize;

  uint32_t curr_idx;		// current inode index being read
  inode_t  *curr_inode;
  dirent_t *curr_ent;
} dir_t;

dir_t  *opendir(uint32_t inode_num);
int	closedir(dir_t *dir);
dirent_t *readdir(dir_t *dir);

#endif
