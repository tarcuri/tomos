#include "ext2.h"
#include "dev/ata.h"

#include <stdio.h>

void ext2_init()
{
  device_t *hdd = ata_open();

  superblock_t *sb = read_superblock(hdd);

  uint32_t block_size = 1024 << sb->block_size_shift;

  // determine number of block groups
  uint32_t block_groups_b = (sb->total_blocks / sb->blocks_per_group) + 1;
  uint32_t block_groups_i = (sb->total_inodes / sb->inodes_per_group) + 1;

  c_printf("b: %d, i: %d\n", block_groups_b, block_groups_i);

/*
  int i;
  for (i = 0; i < (sb->total_inodes - sb->inodes_unallocated); ++i) {
    c_printf("reading inode #%d...\n", i);

    // determine which block the inode belongs too
  }
*/
}

superblock_t *read_superblock(device_t *dev)
{
  // superblock is 1024 bytes from the beginning of the volume
  superblock_t *sb = (superblock_t *) kmalloc(sizeof(superblock_t));

  // device should be ata block device
  disk_request_t dr;

  dr.cmd = DISK_CMD_READ;
  dr.lba = 2;
  dr.num_blocks = 2;
  dr.blocks_complete = 0;
  dr.buffer = (void *) sb;

  // call the IOCTRL
  dev->_ctrl(DISK_CMD_READ, (void *) &dr);

  // print info
  c_printf("\nSuperblock Info:\n");
  c_printf("  Total inodes        : %d\n", sb->total_inodes);
  c_printf("  Total blocks        : %d\n", sb->total_blocks);
  c_printf("  Blocks Reserved     : %d\n", sb->blocks_reserved);
  c_printf("  Blocks Unallocated  : %d\n", sb->blocks_unallocated);
  c_printf("  Inodes Unallocated  : %d\n", sb->inodes_unallocated);
  c_printf("  First Data Block    : %d\n", sb->first_data_block);
  c_printf("  Block Size Shift    : %d\n", sb->block_size_shift);
  c_printf("  Fragment Size Shift : %d\n", sb->frag_size_shift);
  c_printf("  Blocks / group      : %d\n", sb->blocks_per_group);
  c_printf("  Fragments / group   : %d\n", sb->frags_per_group);
  c_printf("  Inodes / group      : %d\n", sb->inodes_per_group);
  c_printf("  Last mount time     : %d\n", sb->last_mount_time);
  c_printf("  Last write time     : %d\n", sb->last_write_time);
  c_printf("  Mounts since check  : %d\n", sb->mounts_since_ccheck);
  c_printf("  Mounts allowed      : %d\n", sb->mounts_allowed_ccheck);
  c_printf("  EXT2 Signature      : %d\n", sb->ext2_signature);
  c_printf("  File System State   : %d\n", sb->file_system_state);
  c_printf("  EXT2 Error Action   : %d\n", sb->ext2_error_action);
  c_printf("  Minor Version       : %d\n", sb->minor_version);
  c_printf("  POSTIX time check   : %d\n", sb->p_time_last_ccheck);
  c_printf("  Forced check int.   : %d\n", sb->forced_ccheck_interval);
  c_printf("  Creation OS ID      : %xh\n", sb->creation_os_id);
  c_printf("  Major Version       : %d\n", sb->major_version);
  c_printf("  Reserved User ID    : %d\n", sb->reserved_user_id);
  c_printf("  Reserved Group ID   : %d\n", sb->reserved_group_id);
}
