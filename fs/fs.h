#ifndef FS_H
#define FS_H

typedef struct dirent
{
  inode_t *inode;
  uint16_t mode;
  char *name;
} dirent_t;

typedef inode_t dir_t;

dir_t *opendir(uint32_t inode_num);
dirent_t *readdir(dir_t *dir);

#endif
