#ifndef FS_H
#define FS_H

#include "ext2.h"
#include "dev/device.h"

typedef struct dir_stream
{
  inode_t *	inode_table;		// allocated by read_inode_table
  void *	dblock_buffer;		// temp. hold this inodes data blocks
  uint32_t	dblock_bufsize;

  uint32_t	curr_idx;		// current inode index being read
  inode_t *	curr_inode;
  dirent_t *	curr_ent;
} dir_t;

struct file_ops
{
  int		(*open)(struct vfs_node *node);
  int		(*close)(struct vfs_node *node);
  int		(*read)(struct vfs_node *node, uint32_t offset, uint32_t size, uint8_t *buf);
  int		(*write)(struct vfs_node *node, uint32_t offset, uint32_t size, uint8_t *buf);
  dir_t *	(*opendir)(struct vfs_node *node);
  int		(*closedir)(dir_t *dir);
  dirent_t *	(*readdir)(dir_t *dir);
};

struct vfs_node
{
  char			name[256];
  uint32_t		ino;		// inode number
  uint32_t		mode;		// file permissions mode
  uint32_t		uid;
  uint32_t		gid;
  uint32_t		flags;
  uint32_t		len;		// length of file in bytes
  struct file_ops *	op;		// file operation function pointers
  struct vfs_node *	ptr;		// used for mountpoints and symlinks
} vfs_node_t;

// leave these for now, but should be repalced by filesystem specific function pointers
dir_t *		opendir(uint32_t inode_num);
int		closedir(dir_t *dir);
dirent_t *	readdir(dir_t *dir);

#endif
