#include "vfs.h"
#include "dcache.h"
#include "kernel/heap.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct superblock *sb_list;
struct superblock *sb_list_head;
uint32_t sb_list_size;

void vfs_init()
{
  // for now we only have ext2, so read the superblock and bgd table
  ext2_init();

  // dentry cache
  dcache.table = (uint32_t *) kmalloc(sizeof(uint32_t) * DCACHE_SIZE, 0);

  // 1 superblock for now, use the disk ext2 sb
  vfs_sb = (struct superblock *) kmalloc(sizeof(struct superblock), 0);

  vfs_sb->dev = fs_dev;
  vfs_sb->read_inode = (read_inode_func) ext2_read_inode;
}

dir_t *vfs_opendir(char *name)
{
  // search the dentry cache
  uint32_t ino = dcache_lookup(name);

  if (ino) {
    // found it
  } else {
    // TODO: need to find inode matching this directory name
    ino = find_inode(name, 0);
    dcache_insert(name, ino);
  }

  // everything is "mounted" on the save ext2 fs
  //return vfs_sb->opendir(vfs_sb->dev, ino);
  return (dir_t *) 0;
}

uint32_t find_inode(char *name, uint32_t inode)
{
  if (strncmp(name, "/", strlen(name)) == 0)
    return 2;
}
