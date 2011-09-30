#include "ext2.h"
#include "kernel/heap.h"
#include "dev/ata.h"

#include <stdio.h>
#include <time.h>




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
  fs_dev = hdd;

  printf("[ext2]    initialization complete\n");
}

void read_block(uint32_t fs_block, void *buf, uint32_t len)
{
  disk_request_t dr;

  dr.cmd = DISK_CMD_READ;
  dr.lba = fs_block * (fs_block_size/512);
  dr.num_blocks = len * (fs_block_size/512);
  dr.blocks_complete = 0;
  dr.buffer = buf;	// buf should be allocated

  fs_dev->_ctrl(DISK_CMD_READ, (void *) &dr);
}

uint32_t open_inode_table(uint32_t inum)
{
  uint32_t group = get_block_group(inum);

  return fs_bgd_table[group].inode_table_block;
}

inode_t *get_inode(uint32_t inum, inode_t *inode_table)
{
  uint32_t index = get_block_index(inum);
  return &inode_table[index];
}

// read an entire inode table into a buffer, and return the pointer
inode_t *read_inode_table(device_t *dev, uint32_t table_block)
{
  uint32_t table_size = fs_sb->inode_size * fs_sb->inodes_per_group;
  inode_t *inode_table = (inode_t *) kmalloc(table_size, 0);

  // transfer 256 sectors at a time
  uint32_t nsectors = table_size / 512;
  uint32_t nsect_to_read = (nsectors > 256) ? 256 : nsectors;
  uint32_t sect_transferred = 0;

  void *bufp = (void *) inode_table;

  disk_request_t dr;
  dr.cmd = DISK_CMD_READ;
  dr.lba = (table_block * 2);
  while (sect_transferred < nsectors) {
    dr.num_blocks = (nsect_to_read == 256) ? 0 : nsect_to_read;
    dr.blocks_complete = 0;
    dr.buffer = bufp;

    // read this chunk
    dev->_ctrl(DISK_CMD_READ, (void *) &dr);
    printf("read chunk\n");
    
    // increment, everything
    dr.lba += nsect_to_read;
    bufp = (void*) (((uint32_t)bufp) + (nsect_to_read * 512));
    sect_transferred += nsect_to_read;
    nsect_to_read = ((nsectors - sect_transferred) > 256) ? 256 : (nsectors - sect_transferred);
  }

  return inode_table;
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
