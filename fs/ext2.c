#include "ext2.h"
#include "kernel/heap.h"
#include "dev/ata.h"

#include <stdio.h>

superblock_t *fs_sb;
uint32_t fs_block_size;
uint32_t fs_block_groups;
block_group_desc_t *fs_bgd_table;


void ext2_init()
{
  device_t *hdd = ata_open();

  fs_sb = read_superblock(hdd);

  fs_block_size = 1024 << fs_sb->block_size_shift;

  // determine number of block groups
  uint32_t block_groups_b = (fs_sb->total_blocks / fs_sb->blocks_per_group) + 1;
  uint32_t block_groups_i = (fs_sb->total_inodes / fs_sb->inodes_per_group) + 1;

  fs_block_groups = (block_groups_b > block_groups_i) ? block_groups_b : block_groups_i;
  c_printf("[ext2]    %d block groups\n", fs_block_groups);

  // block group descriptor table
  fs_bgd_table = read_bgd_table(hdd);
  
  // which block group for root dir? (inode 2)
  uint32_t root_bg  = (2 - 1) / fs_sb->inodes_per_group;
  uint32_t index    = (2 - 1) % fs_sb->inodes_per_group;
  uint32_t root_blk = (index * fs_sb->inode_size) / fs_block_size;

  // root BG descriptor looks OK...
  c_printf("BGD:\n");
  c_printf("  %d\n", fs_bgd_table[root_bg].block_usage_block);
  c_printf("  %d\n", fs_bgd_table[root_bg].inode_usage_block);
  c_printf("  %d\n", fs_bgd_table[root_bg].inode_table_block);
  c_printf("  %d\n", fs_bgd_table[root_bg].free_blocks);
  c_printf("  %d\n", fs_bgd_table[root_bg].free_inodes);
  c_printf("  num_dirs: %d\n", fs_bgd_table[root_bg].num_dirs);

  c_printf("root inode index: %d\n", index);
  c_printf("[ext2]    inode table found at block %d\n", fs_bgd_table[root_bg].inode_table_block);
  inode_t *root_inode = read_inode(hdd, fs_bgd_table[root_bg].inode_table_block, index);

  print_inode(root_inode);
}

void print_inode(inode_t *inode)
{
  c_printf("inode:\n");
  c_printf("  mode: %x\n", inode->mode);
  c_printf("   uid: %x\n", inode->uid);
  c_printf("c-time: %x\n", inode->c_time);
}

inode_t *read_inode(device_t *dev, uint32_t table_block_addr, uint32_t index)
{
  disk_request_t dr;
  inode_t *inode_table = (inode_t *) kmalloc(512, 0);

  dr.cmd = DISK_CMD_READ;
  dr.lba = 2 + (table_block_addr * 2);
  dr.num_blocks = 1;
  dr.blocks_complete = 0;
  dr.buffer = (void *) inode_table;

  c_printf("reading inode from disk sector %d\n", dr.lba);
  dev->_ctrl(DISK_CMD_READ, (void *) &dr);

  inode_t *inode = (inode_t *) kmalloc(sizeof(inode_t), 0);

  c_printf("READ INODE\n");

  memcpy((void *) inode, (void *) &inode_table[index], sizeof(inode_t));

  kfree(inode_table);

  return inode;
}

block_group_desc_t *read_bgd_table(device_t *dev)
{
  disk_request_t dr;

  uint32_t bs_bytes = sizeof(block_group_desc_t) * fs_block_groups;

  dr.cmd = DISK_CMD_READ;
  dr.lba = 2 + (fs_block_size/512);
  dr.num_blocks = (bs_bytes / 512) + ((bs_bytes % 512) ? 1 : 0);
  dr.blocks_complete = 0;

  block_group_desc_t *bgd_table = (block_group_desc_t *) kmalloc(dr.num_blocks * 512, 0);

  dr.buffer = (void *) bgd_table;

  dev->_ctrl(DISK_CMD_READ, (void *) &dr);

  return bgd_table;
}

superblock_t *read_superblock(device_t *dev)
{
  // superblock is 1024 bytes from the beginning of the volume
  superblock_t *sb = (superblock_t *) kmalloc(sizeof(superblock_t), 0);

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
  c_printf("  Blocks Unallocated  : %d\n", sb->free_blocks);
  c_printf("  Inodes Unallocated  : %d\n", sb->free_inodes);
  c_printf("  First Data Block    : %d\n", sb->first_data_block);
  c_printf("  Block Size Shift    : %d\n", sb->block_size_shift);
  c_printf("  Fragment Size Shift : %d\n", sb->frag_size_shift);
  c_printf("  Blocks / group      : %d\n", sb->blocks_per_group);
  c_printf("  Fragments / group   : %d\n", sb->frags_per_group);
  c_printf("  Inodes / group      : %d\n", sb->inodes_per_group);
#if 0 
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
#endif

  return sb;
}
