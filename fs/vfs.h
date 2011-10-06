#ifndef VFS_H
#define VFS_H

#include "ext2.h"
#include "dev/device.h"

typedef void * (*read_inode_func)(device_t *, uint32_t);

// common file model: superblock, inode, file, dentry
typedef struct inode
{
  device_t *		dev;
  uint32_t		ino;
  uint16_t		mode;
  uint16_t		uid;
  uint16_t		gid;
  uint32_t		a_time;
  uint32_t		c_time;
  uint32_t		m_time;
  uint32_t		d_time;
  uint32_t		block_size;
  // inode operations
} inode_t;

struct superblock
{
  device_t *	dev;
  inode_t *	mounted_inode;		// first inode in this file system
  inode_t *	covered_inode;
  uint32_t	block_size;
  // superblock operations (r/w inodes, superblocks
  read_inode_func	read_inode;
};

// file systems will implement their own directory stream, just pass along the handle
typedef void dir_t;

typedef struct dirent 
{
  uint32_t inode;
  uint16_t rec_len;
  uint8_t  name_len;
  uint8_t  file_type;
  char     name[256];
} dirent_t;

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

// data
struct superblock *vfs_sb;

// functions
void vfs_init(void);

int vfs_open(char *name, uint32_t flags);

#endif
