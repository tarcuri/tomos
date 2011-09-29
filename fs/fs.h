#ifndef FS_H
#define FS_H

typedef struct dirent
{
  uint32_t inode;
} dirent_t;

dir_entry_t *opendir(uint32_t inode);
dirent_t *readdir(dir_entry_t *dir);

#endif
