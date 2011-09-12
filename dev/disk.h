#ifndef DISK_H
#define DISK_H

// define a disk request
typedef struct disk_request
{
  unsigned char		cmd;		// request command (read, write, etc)
  unsigned char		status;		// current request status

  unsigned int		lba;		// logical block address to transfer to/from
  unsigned int 		nblocks;	// # of blocks to transfer

  void			*buffer;
} disk_request_t;

#endif
