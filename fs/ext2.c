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

  printf("b: %d, i: %d\n", block_groups_b, block_groups_i);

/*
  int i;
  for (i = 0; i < (sb->total_inodes - sb->inodes_unallocated); ++i) {
    printf("reading inode #%d...\n", i);

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
  printf("\nSuperblock Info:\n");
  printf("  Total inodes        : %3d\n", sb->total_inodes);
  printf("  Total blocks        : %3d\n", sb->total_blocks);
  printf("  Blocks Reserved     : %3d\n", sb->blocks_reserved);
  printf("  Blocks Unallocated  : %3d\n", sb->blocks_unallocated);
  printf("  Inodes Unallocated  : %3d\n", sb->inodes_unallocated);
  printf("  First Data Block    : %3d\n", sb->first_data_block);
  printf("  Block Size Shift    : %3d\n", sb->block_size_shift);
  printf("  Fragment Size Shift : %3d\n", sb->frag_size_shift);
  printf("  Blocks / group      : %3d\n", sb->blocks_per_group);
  printf("  Fragments / group   : %3d\n", sb->frags_per_group);
  printf("  Last mount time     : %3d\n", sb->last_mount_time);
  printf("  Last write time     : %3d\n", sb->last_write_time);
  printf("  Mounts since check  : %3d\n", sb->mounts_since_ccheck);
  printf("  Mounts allowed      : %3d\n", sb->mounts_allowed_ccheck);
  printf("  EXT2 Signature      : %3d\n", sb->ext2_signature);
  printf("  File System State   : %3d\n", sb->file_system_state);
  printf("  EXT2 Error Action   : %3d\n", sb->ext2_error_action);
  printf("  Minor Version       : %3d\n", sb->minor_version);
  printf("  POSTIX time check   : %3d\n", sb->p_time_last_ccheck);
  printf("  Forced check int.   : %3d\n", sb->forced_ccheck_interval);
  printf("  Creation OS ID      : %xh\n", sb->creation_os_id);
  printf("  Major Version       : %3d\n", sb->major_version);
  printf("  Reserved User ID    : %3d\n", sb->reserved_user_id);
  printf("  Reserved Group ID   : %3d\n", sb->reserved_group_id);
}
