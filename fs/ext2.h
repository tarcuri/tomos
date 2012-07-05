#ifndef FS_EXT2_H
#define FS_EXT2_H

#include "dev/device.h"
#include "dev/disk.h"

#include "vfs.h"

#include <stdint.h>

// TODO: expanded superblock fields present if MAJOR_VERSION >= 1

// misc
#define FS_STATE_CLEAN		0x01
#define FS_STATE_ERRORS		0x02

#define ERROR_IGNORE		0x01
#define ERROR_REMOUNT		0x02
#define ERROR_KERNEL_PANIC	0x03

// creator OS ids
#define OS_CREATOR_LINUX	0x00
#define OS_CREATOR_HURD		0x01
#define OS_CREATOR_FREEBSD	0x03
#define OS_CREATOR_BSD_LITE	0x04

// EXT2 mode field (taken from EXT2 docs by Dave Poirier)
// http://www.nongnu.org/ext2-doc/ext2.html
// -- file format --
#define EXT2_S_IFSOCK		0xC000		// socket
#define EXT2_S_IFLNK		0xA000		// symbolic link
#define EXT2_S_IFREG		0x8000		// regular file
#define EXT2_S_IFBLK		0x6000		// block device
#define EXT2_S_IFDIR		0x4000		// directory
#define EXT2_S_IFCHR		0x2000		// character device
#define EXT2_S_IFIFO		0x1000		// fifo
// -- process execution user/group override --
#define EXT2_S_ISUID		0x0800		// Set process User ID
#define EXT2_S_ISGID		0x0400		// Set process Group ID
#define EXT2_S_ISVTX		0x0200		// sticky bit
// -- access rights --
#define EXT2_S_IRUSR		0x0100		// user read
#define	EXT2_S_IWUSR		0x0080		// user write
#define EXT2_S_IXUSR		0x0040		// user execute
#define EXT2_S_IRGRP		0x0020		// group read
#define EXT2_S_IWGRP		0x0010		// group write
#define EXT2_S_IXGRP		0x0008		// group execute
#define EXT2_S_IROTH		0x0004		// others read
#define EXT2_S_IWOTH		0x0002		// others write
#define EXT2_S_IXOTH		0x0001		// others execute

// EXT2 file types
#define EXT2_FT_UNKNOWN		0	// Unknown File Type
#define EXT2_FT_REG_FILE	1	// Regular File
#define EXT2_FT_DIR		2	// Directory File
#define EXT2_FT_CHRDEV		3	// Character Device
#define EXT2_FT_BLKDEV		4	// Block Device
#define EXT2_FT_FIFO		5	// Buffer File
#define EXT2_FT_SOCK		6	// Socket File
#define EXT2_FT_SYMLINK		7	// Symbolic Link

// ext2 superblock
typedef struct ext2_superblock
{
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t blocks_reserved;
  uint32_t free_blocks;
  uint32_t free_inodes;
  uint32_t first_data_block;
  uint32_t block_size_shift;
  uint32_t frag_size_shift;
  uint32_t blocks_per_group;
  uint32_t frags_per_group;
  uint32_t inodes_per_group;
  uint32_t last_mount_time;
  uint32_t last_write_time;
  // 2 bytes
  uint16_t mounts_since_ccheck;
  uint16_t mounts_allowed_ccheck;
  uint16_t ext2_signature;
  uint16_t file_system_state;
  uint16_t ext2_error_action;
  uint16_t minor_version;
  // 4 bytes
  uint32_t p_time_last_ccheck;
  uint32_t forced_ccheck_interval;
  uint32_t creation_os_id;
  uint32_t major_version;
  // 2 bytes
  uint16_t reserved_user_id;
  uint16_t reserved_group_id;

  uint32_t first_inode;
  uint16_t inode_size;
  uint8_t  reserved[1024-90];
} __attribute__((__packed__)) ext2_superblock_t;

// ext2 block group descriptor
typedef struct ext2_block_group_descriptor
{
  uint32_t block_usage_block;
  uint32_t inode_usage_block;
  uint32_t inode_table_block;
  uint16_t free_blocks;
  uint16_t free_inodes;
  uint16_t num_dirs;
  uint16_t pad;
  uint8_t  reserved[12];
} ext2_bg_desc_t;

// ext2 inode
typedef struct ext2_inode
{
  uint16_t mode;
  uint16_t uid;

  uint32_t size_low;
  uint32_t a_time;
  uint32_t c_time;
  uint32_t m_time;
  uint32_t d_time;

  uint16_t gid;
  uint16_t links_count;		// hard links to this inode

  uint32_t blocks;		// 512 byte sectors in use by this inode
  uint32_t flags;
  uint32_t os_specific_1;
  uint32_t dblock_ptr_0;
  uint32_t dblock_ptr_1;
  uint32_t dblock_ptr_2;
  uint32_t dblock_ptr_3;
  uint32_t dblock_ptr_4;
  uint32_t dblock_ptr_5;
  uint32_t dblock_ptr_6;
  uint32_t dblock_ptr_7;
  uint32_t dblock_ptr_8;
  uint32_t dblock_ptr_9;
  uint32_t dblock_ptr_10;
  uint32_t dblock_ptr_11;
  uint32_t singly_iblock_ptr;	// points to a block that is a list of block pointers to data
  uint32_t doubly_iblock_ptr;	// points to a block that is a list of block pointers to SIBs
  uint32_t triply_iblock_ptr;	// points to a block that is a list of block pointers to DIBs
  uint32_t generation;
  uint32_t extended_attr_block;
  uint32_t size_high;
  uint32_t block_addr_of_frag;

  uint32_t os_specific_2;
  uint32_t os_specific_3;
  uint32_t os_specific_4;
} __attribute__((__packed__)) ext2_inode_t;

typedef struct ext2_dirent
{
  uint32_t inode;
  uint16_t rec_len;
  uint8_t  name_len;
  uint8_t  file_type;
  char     name[1];	// size = sizeof(ext2_dirent_t) + strlen(d.name)
} ext2_dirent_t;

typedef struct ext2_dstream
{
  ext2_inode_t *	inode;
  ext2_dirent_t *	current;	// current entry pointer
  void *		buffer;		// directory data block buffer
  uint32_t		bufsize;
} ext2_dir_t;

// data
device_t *		fs_dev;
ext2_superblock_t *	fs_sb;
ext2_bg_desc_t *	fs_bgd_table;
ext2_inode_t *		fs_inode_table;
uint32_t		fs_block_size;
uint32_t		fs_block_groups;

// functions
void ext2_init(void);
void read_block(uint32_t fs_block, void *buf, uint32_t len);

#define get_block_group(inum)	((inum - 1) / fs_sb->inodes_per_group)
#define get_block_index(inum)	((inum - 1) % fs_sb->inodes_per_group)
//#define get_block(i)		((i * fs_sb->inode_size) / fs_block_size)

// read an entire inode table into a buffer, and return the pointer
ext2_inode_t *		read_inode_table(device_t *dev, uint32_t table_block);

ext2_bg_desc_t *	read_bgd_table(device_t *dev);

ext2_superblock_t *	read_superblock(device_t *dev);

// vfs read_inode_func
ext2_inode_t *		ext2_read_inode(device_t *, uint32_t);

ext2_dir_t *ext2_opendir(device_t *dev, uint32_t ino);

#endif
