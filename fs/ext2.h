#ifndef FS_EXT2_H
#define FS_EXT2_H

// superblock offsets
#define TOTAL_INODES		0x00	// 4 bytes
#define TOTAL_BLOCKS		0x04
#define BLOCKS_RESERVED_SUPER	0x08
#define BLOCKS_UNALLOCATED	0x0C
#define INODES_UNALLOCATED	0x10
#define SUPERBLOCK_INDEX	0x14
#define BLOCK_SIZE_SHIFT	0x18
#define FRAGMENT_SIZE_SHIFT	0x1C
#define BLOCKS_PER_GROUP	0x20
#define FRAGMENTS_PER_GROUP	0x24
#define LAST_MOUNT_TIME		0x28
#define LAST_WRITE_TIME		0x2C
#define	MOUNTS_SINCE_C_CHECK	0x30	// 2 bytes
#define MOUNTS_ALLOWED_C_CHECK	0x32
#define EXT2_SIGNATURE		0x34
#define FILE_SYSTEM_STATE	0x36
#define EXT2_ERROR_ACTION	0x38
#define MINOR_VERSION		0x3A
#define P_TIME_LAST_C_CHECK	0x3C	// 4 bytes
#define FORCED_C_CHECK_INTERVAL	0x40
#define CREATION_OS_ID		0x44
#define MAJOR_VERSION		0x48
#define RESERVED_USER_ID	0x4C	// 2 bytes
#define RESERVED_GROUP_ID	0x4E

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
  unsigned int total_inodes;
  unsigned int total_blocks;
  unsigned int blocks_reserved;
  unsigned int blocks_unallocated;
  unsigned int inodes_unallocated;
  unsigned int sb_index;
  unsigned int block_size_shift;
  unsigned int frag_size_shift;
  unsigned int blocks_per_group;
  unsigned int frags_per_group;
  unsigned int last_mount_time;
  unsigned int last_write_time;
  // 2 bytes
  unsigned short mounts_since_ccheck;
  unsigned short mounts_allowed_ccheck;
  unsigned short ex2_signature;
  unsigned short file_system_state;
  unsigned short ext2_error_action;
  unsigned short minor_version;
  // 4 bytes
  unsigned int p_time_last_ccheck;
  unsigned int forced_ccheck_interval;
  unsigned int creation_os_id;
  unsigned int major_version;
  // 2 bytes
  unsigned short reserved_user_id;
  unsigned short reserved_group_id;
} __attribute__(__packed__) superblock_t;



typedef struct inode
{
  unsigned short type_permissions;
  unsigned short user_id;
  // TODO: finish inode
/*
4	 7	 4	 Lower 32 bits of size in bytes
8	 11	 4	 Last Access Time (in POSIX time)
12	 15	 4	 Creation Time (in POSIX time)
16	 19	 4	 Last Modification time (in POSIX time)
20	 23	 4	 Deletion time (in POSIX time)
24	 25	 2	 Group ID
26	 27	 2	 Count of hard links (directory entries) to this inode. When this reaches 0, the data blocks are marked as unallocated.
28	 31	 4	 Count of disk sectors (not Ext2 blocks) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode.
32	 35	 4	 Flags (see below)
36	 39	 4	Operating System Specific value #1
40	 43	 4	 Direct Block Pointer 0
44	 47	 4	 Direct Block Pointer 1
48	 51	 4	 Direct Block Pointer 2
52	 55	 4	 Direct Block Pointer 3
56	 59	 4	 Direct Block Pointer 4
60	 63	 4	 Direct Block Pointer 5
64	 67	 4	 Direct Block Pointer 6
68	 71	 4	 Direct Block Pointer 7
72	 75	 4	 Direct Block Pointer 8
76	 79	 4	 Direct Block Pointer 9
80	 83	 4	 Direct Block Pointer 10
84	 87	 4	 Direct Block Pointer 11
88	 91	 4	 Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)
92	 95	 4	 Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)
96	 99	 4	 Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)
100	 103	 4	 Generation number (Primarily used for NFS)
104	 107	 4	 In Ext2 version 0, this field is reserved. In version >= 1, Extended attribute block (File ACL).
108	 111	 4	 In Ext2 version 0, this field is reserved. In version >= 1, Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory
112	 115	 4	 Block address of fragment
116	 127	 12	Operating System Specific Value #2o
*/
} __attribute__(__packed__) inode_t;

static void read_superblock(void *buf, device_t *dev)
{
  // superblock is 1024 bytes from the beginning of the volume
  buf = kmalloc(1024);

  // device should be ata block device
  disk_request_t *dr = (disk_request_t *) kmalloc(disk_request_t);

  dr->cmd = ATA_READ_MULTIPLE;
  dr->lba = 2;
  dr->num_blocks = 2;
  dr->blocks_complete = 0;
  dr->buffer = buf;

  // call the IOCTRL
  dev->_ctrl();
}

#endif
