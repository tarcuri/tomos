#ifndef FS_EXT2_H
#define FS_EXT2_H

#include "dev/device.h"
#include "dev/disk.h"

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

typedef struct superblock
{
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t blocks_reserved;
  uint32_t blocks_unallocated;
  uint32_t inodes_unallocated;
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
} __attribute__((__packed__)) superblock_t;



typedef struct inode
{
  uint16_t type_permissions;
  uint16_t user_id;

  uint32_t size_low;
  uint32_t access_time;
  uint32_t creation_time;
  uint32_t modification_time;
  uint32_t deteltion_time;

  uint16_t group_id;
  uint16_t hard_links;		// hard links to this inode

  uint32_t disk_sectors;	// sectors in use by this inode
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
  uint32_t generation_num;
  uint32_t extended_attr_block;
  uint32_t size_high;
  uint32_t block_addr_of_frag;

  uint32_t os_specific_2;
  uint32_t os_specific_3;
  uint32_t os_specific_4;
} __attribute__((__packed__)) inode_t;


void ext2_init(void);
superblock_t *read_superblock(device_t *dev);

#endif
