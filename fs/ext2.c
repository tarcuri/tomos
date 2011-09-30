#include "ext2.h"
#include "kernel/heap.h"
#include "dev/ata.h"

#include <stdio.h>
#include <time.h>

device_t *fs_dev;
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
  
  fs_dev = hdd;

  inode_t *root_inode = find_inode(2);
  
  // now parse the root directory's entries
  void *buf = kmalloc(1024, 0);

  read_block(root_inode->dblock_ptr_0, buf, 1);

  uint32_t *dirp = (uint32_t *) buf;

  int i;
  char name[256];
  for (i = 0; i < 4; ++i) {
    dir_entry_t *dir = (dir_entry_t *)dirp;

    printf("inode: %d\n", dir->inode);
    strncpy(name, dir->name, dir->name_len);
    name[dir->name_len] = '\0';
    printf(" %s\n", name);
    dirp = (void *) ((uint32_t)dirp + dir->rec_len);

    if (dir->inode == 11) {
      // read this dir
      inode_t *kernel_dir = find_inode(dir->inode);
      printf("block0: %x\n", kernel_dir->dblock_ptr_0);

      // read the first data block
      void *kdir = kmalloc(1024, 0);
      read_block(kernel_dir->dblock_ptr_0, kdir, 1);

      uint32_t *kp = (uint32_t *) kdir;
      int j;
      for (j = 0; j < 6; ++j) {
        dir_entry_t *d = (dir_entry_t *)kp;

        printf("inode: %d\n", d->inode);
        strncpy(name, d->name, d->name_len);
        name[d->name_len] = '\0';
        printf(" %s\n", name);
        if (d->file_type & EXT2_FT_REG_FILE) {
          inode_t *fnode = find_inode(d->inode);
          void *fdata = kmalloc(1024, 0);
          read_block(fnode->dblock_ptr_0, fdata, 1);
          printf("->> %s\n", (char *) fdata);
          kfree(fdata);
        }

        kp = (void *) ((uint32_t)kp + d->rec_len);
      }
    }
  }
  printf("[ext2]    initialization complete\n");

  kfree(buf);
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

inode_t *find_inode(uint32_t inode)
{
  uint32_t bg  = (inode - 1) / fs_sb->inodes_per_group;
  uint32_t idx = (inode - 1) % fs_sb->inodes_per_group;
  uint32_t blk = (idx * fs_sb->inode_size) / fs_block_size;

  return read_inode(fs_dev, fs_bgd_table[bg].inode_table_block, idx);
}

inode_t *read_inode(device_t *dev, uint32_t table_block_addr, uint32_t index)
{
  disk_request_t dr;
  uint32_t table_size = fs_sb->inode_size * fs_sb->inodes_per_group;
  inode_t *inode_table = (inode_t *) kmalloc(table_size, 0);

  dr.cmd = DISK_CMD_READ;
  dr.lba = (table_block_addr * 2);
  dr.num_blocks = (table_size / 512);
  dr.blocks_complete = 0;
  dr.buffer = (void *) inode_table;

  dev->_ctrl(DISK_CMD_READ, (void *) &dr);

  inode_t *inode = (inode_t *) kmalloc(sizeof(inode_t), 0);
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
