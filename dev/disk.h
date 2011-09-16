#ifndef DISK_H
#define DISK_H

#define DISK_MAX_REQUESTS	128
#define DISK_BLOCK_SIZE		512

#define DISK_CMD_READ		1
#define DISK_CMD_WRITE		2
#define DISK_CMD_REQUEST	3

#define DISK_STATUS_READ_PENDING      	0x01
#define DISK_STATUS_WRITE_PENDING     	0x02
#define DISK_STATUS_IO_SUCCESS        	0x03
#define DISK_STATUS_IO_ERROR 		0x04

// define a disk request
typedef struct disk_request
{
  unsigned char		cmd;			// request command (read, write, etc)
  unsigned char		status;			// current request status

  unsigned int		lba;			// logical block address to transfer to/from
  unsigned int 		num_blocks;		// # of blocks to transfer
  unsigned int		blocks_complete; 	// blocks already transferred (offset to buffer)

  void			*buffer;

  struct disk_request	*prev;		// linked list of requests
  struct disk_request	*next;
} disk_request_t;

disk_request_t *pending_disk_requests;
disk_request_t *current_disk_request;

// functions
int disk_insert_request(disk_request_t *r);
void disk_print_requests(void);

#endif
