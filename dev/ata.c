#include "dev/ata.h"
#include "dev/console.h"
#include "dev/disk.h"

#include "x86.h"
#include "intr.h"


unsigned int ata_cmd_reg  = ATA_PRI_COMMAND_REG;
unsigned int ata_ctrl_reg = ATA_PRI_CONTROL_REG;

#define ATA_INIT		0
#define ATA_INTRQ_WAIT		1
#define	ATA_CHECK_STATUS	2
#define ATA_TRANSFER_DATA	3

unsigned int ata_state;


// TODO: clean up the mess and figure out whats going on here
void ata_init()
{
  _install_isr(INT_VEC_PRI_IDE, ata_isr);

  // select device 0, LBA
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_DEV_LBA_BIT | ATA_PRI_CHANNEL);

  // set the sector multiple
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);
  __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, 1);
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_SET_MULTIPLE);
  while (ata_alt_status(5) & ATA_STATUS_BUSY)
    ;

  if (ata_alt_status(0) & ATA_STATUS_ERROR)
    c_printf("ERROR: ATA SET MULTIPLE\n");

  // clear nIEN
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x0);

  ata_state = ATA_INIT;
}

static inline void insw(unsigned short port, void *addr, unsigned int cnt)
{
   asm volatile("rep; insw"
       : "+D" (addr), "+c" (cnt)
       : "d" (port)
       : "memory");
}

void ata_isr(int vector, int code)
{
  asm("cli");

  //__outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);
  c_printf("ATA_ISR\n");

  if (ata_state != ATA_INTRQ_WAIT) {
    c_printf("UNEXPECTED ATA INTERRRUPT\n");
  }

  // now in PIO Check_Status state
  ata_state = ATA_CHECK_STATUS;

  unsigned int status;

  if (!current_disk_request) {
    // NON-DATA command interrupt
    c_printf("NON-DATA command interrupt\n");
    while (ata_alt_status(0) & ATA_STATUS_BUSY)
      ;
  } else {
    switch (current_disk_request->cmd) {
    case DISK_CMD_READ:
      status = ata_alt_status(0);
      while (status & ATA_STATUS_BUSY)
        status = ata_alt_status(5);
  
      if (status & ATA_STATUS_DRQ) {
        unsigned short *buf = (unsigned short *) ((unsigned int)current_disk_request->buffer
                               + (current_disk_request->blocks_complete * DISK_BLOCK_SIZE));
  
        int i;
        for (i = 0; i <  DISK_BLOCK_SIZE/2; ++i) {
          *buf++ = __inw(ata_cmd_reg | ATA_CMD_R_DATA);
          //c_printf("%x", *(buf - 1));
        }
        //insw(ata_cmd_reg | ATA_CMD_R_DATA, buf, DISK_BLOCK_SIZE * 2);
        c_printf("read a sector:\n");

        current_disk_request->blocks_complete += 1;
        ata_state = ATA_INTRQ_WAIT;
  
        if (current_disk_request->blocks_complete == current_disk_request->num_blocks)
          current_disk_request->status = DISK_STATUS_IO_SUCCESS;
      } else {
        current_disk_request->status = DISK_STATUS_IO_ERROR;
        panic("ATA IO error!\n");
      }
  
      break;
    case DISK_CMD_WRITE:
    case DISK_CMD_REQUEST:
    default:
      c_printf("NOT YET IMPLEMENTED\n");
      break;
    }
  }
  
  c_printf("LEAVING\n");
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x00 );

  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
  __outb(PIC_SLAVE_CMD_PORT, PIC_EOI);

  asm("sti");
}

unsigned char ata_alt_status(unsigned int poll)
{
  unsigned char status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);

  int i;
  for (i = 0; i < poll; ++i) {
    status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);
  }
  return status;
}

/**
 * ATA: READ SECTOR (S)
 * PIT data-in protocol
 */
void ata_read_sectors(disk_request_t *dr)
{
  ata_state = ATA_INIT;

  // PIO protocol
  unsigned char status = ata_alt_status(0);
  while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_READY))
    status = ata_alt_status(0);

  c_printf("issuing read (%d:%d)\n", dr->num_blocks, __inb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT) & 0xff);

  // inputs
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_DEV_LBA_BIT | ATA_PRI_CHANNEL
                     | ((dr->lba >> 24) & 0xFF));

  __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, dr->num_blocks);
  c_printf("issuing read (%d:%d)\n", dr->num_blocks, __inb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT) & 0xff);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_LOW, dr->lba & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_MID, (dr->lba >> 8) & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_HIGH, (dr->lba >> 16) & 0xFF);

  current_disk_request = dr;

  // issue command, enter INTRQ_wait state

  // while we have data blocks to transfer
  while (dr->blocks_complete < dr->num_blocks) {
    if (ata_state == ATA_INIT) {
      ata_state = ATA_INTRQ_WAIT;
      //__outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_READ_SECTORS);
      __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_READ_MULTIPLE);
    }

    if (ata_alt_status(0) & ATA_STATUS_ERROR)
      c_printf(">");
  }
}

void ata_identify_device()
{
  // we are going to poll for this command...
  asm( "cli" );

  // set nIEN
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);

  // we need to wait for DRDY
  unsigned char status = ata_alt_status(0);

  //c_printf("ATA: identify device: waiting for DRDY...\n");
  while ( !(status & ATA_STATUS_READY) ) {
    status = ata_alt_status(0);
    //c_printf("s: %x\n");
  }

  // write the IDENTIFY DEVICE command
  //c_printf("ATA: identify device: sending command byte\n");
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_IDENTIFY_DEVICE);
  
  // now transfer 256 words of data
  int word_i;
  unsigned short ata_id_device_data[256];

  /*
  ** i)  wait for DRQ=1 and BSY=0 (cycle status register)
  ** ii) transfer a word, if more remain, repeat
  */	
  // now transfer a word
  //c_printf("ATA: identify device: reading coniguration data");
  for (word_i = 0; word_i < 256; ++word_i) {
    // wait for DRQ
    while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_DRQ))
      status = ata_alt_status(0);

    // read from Data register
    ata_id_device_data[ word_i ] = __inw(ata_cmd_reg | ATA_CMD_R_DATA);

  }
  // read status
  status = ata_alt_status(0);

  ata_print_device_info(ata_id_device_data);

  // clear nIEN so we get interrupts again
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x00 );

  // Subaru Technica International (STi)
  asm( "sti" );
}

void ata_print_device_info( unsigned short *dev_data )
{
  // check for NULL pointers...

  // see Table 16 of ATA/ATAPI-7 Volume 1
  c_printf("SECTORS PER INTERRUPT: %x\n", dev_data[59] );

  // words 27-46: model number (40 ASCII characters)
  char model_num[ 41 ];
  memcpy( (void *) &model_num[0], (void *) &dev_data[ 27 ], 40 );

  model_num[ 40 ] = '\0';

  // un-scramble?
  int len = 40;
  char c, *p = model_num, *end = model_num + (len & ~1);  
  
  /* Swap characters. */  
  while (p != end) {  
    c = *p;  
    *p = *(p + 1);  
    *(p + 1) = c;  
    p += 2;  
  }

    /* Make sure we have a NULL byte at the end. Wipe out trailing garbage. */  
  p = end - 1;  
  *p-- = '\0';  
  while (p-- != model_num) {  
    c = *p;  
    if (c > 32 && c < 127)  
      break;  
    *p = '\0';  
  } 

  c_printf("MODEL: (%c%c) %s\n", model_num[0], model_num[1],(char *) &model_num[0]);

  // words 60-61 are the total number of user addressable sectors
  unsigned int user_addressable_sectors = *( (unsigned int *) &dev_data[ 60 ]);

  c_printf("Total number of user addressable sectors (maximum LBA + 1): 0x%x\n", user_addressable_sectors);

  // check for multi-word DMA support (word 63)
  unsigned short dma_modes = dev_data[ 63 ];
  
  c_printf("DMA modes 0x%x\n", dma_modes );

  if ( dma_modes & 0x0004 ) {
  	c_printf("Multiword DMA mode 2 and below are supported\n");
  } else if ( dma_modes & 0x0002 ) {
  	c_printf("Multiword DMA mode 1 and below are supported\n");
  } else if ( dma_modes & 0x0001 ) {
  	c_printf("Multiword DMA mode 0 is supported\n");
  }

  // word 64: supported PIO modes (1st byte only)
}
