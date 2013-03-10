#include "dev/ata.h"
#include "dev/console.h"

#include "x86.h"
#include "kernel/interrupt.h"


uint32_t ata_cmd_reg  = ATA_PRI_COMMAND_REG;
uint32_t ata_ctrl_reg = ATA_PRI_CONTROL_REG;

#define ATA_INIT		0
#define ATA_INTRQ_WAIT		1
#define	ATA_CHECK_STATUS	2
#define ATA_TRANSFER_DATA	3

/*
 * driver interface
 */
device_t *ata_open()
{
  device_t *d = (device_t *) kmalloc(sizeof(device_t));

  d->type   = DEVICE_BLOCK;
  d->_read  = 0;
  d->_write = 0;
  d->_ctrl  = ata_ctrl;

  return d;
}

int32_t ata_ctrl(uint32_t cmd, void *buf)
{
  //c_printf("ATA_CTRL\n");
  int32_t res = 0;

  disk_request_t *d = (disk_request_t *) buf;

  switch(cmd) {
  case DISK_CMD_READ:
    //c_printf("DISK READ OPERATION\n");
    ata_read_multiple(d);
    break;
  default:
    res = 1;
  };

  return res;
}

/*
 * module code
 */
void ata_init()
{
  _install_isr(INT_VEC_PRI_IDE, ata_isr);

  // disable interrupts on the slave
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_SEC_CHANNEL);
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);

  // select device 0, LBA
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_DEV_LBA_BIT | ATA_PRI_CHANNEL);

  // set the sector multiple
  // write SECTOR COUNT only when BUSY and DRQ are clear
  if ((ata_alt_status(5) & (ATA_STATUS_BUSY | ATA_STATUS_DRQ)) == 0)
    __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, 1);
  else
    c_printf("Unable to write SECTOR COUNT register (0x%x)\n", ata_alt_status(0));

  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_SET_MULTIPLE);

  if (ata_alt_status(0) & ATA_STATUS_ERROR)
    c_printf("ERROR: ATA SET MULTIPLE\n");

  // clear pending interrupt
  __inb(ata_cmd_reg | ATA_CMD_R_STATUS);

  // clear nIEN
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x0);

  c_printf("[ata]     ATA driver initialized\n");
}


void ata_isr(int32_t vector, int32_t code)
{
  //asm volatile ("cli" : : :"memory");

  // when in the Check_status state, the host shall read the STATUS register
  uint32_t status = __inb(ata_cmd_reg | ATA_CMD_R_STATUS);

  if (!current_disk_request) {
    // NON-DATA command interrupt
    c_printf("NON-DATA command interrupt, %x\n", ata_alt_status(0));
  } else {
    switch (current_disk_request->cmd) {
    case DISK_CMD_READ:
      status = ata_alt_status(0);
      while (status & ATA_STATUS_BUSY)
        status = ata_alt_status(5);

      if (status & ATA_STATUS_DRQ) {
        uint16_t *buf = (uint16_t *) ((uint32_t)current_disk_request->buffer
                               + (current_disk_request->blocks_complete * DISK_BLOCK_SIZE));
  
        uint32_t nblocks = (current_disk_request->num_blocks)
                               ? (current_disk_request->num_blocks) : 256;
        int32_t i;
        for (i = 0; i <  DISK_BLOCK_SIZE/2; ++i)
          *buf++ = __inw(ata_cmd_reg | ATA_CMD_R_DATA);

        current_disk_request->blocks_complete += ((i*2)/DISK_BLOCK_SIZE);
                                            
        if (current_disk_request->blocks_complete == nblocks) {
          //c_printf("ATA REQUEST COMPLETE\n");
          current_disk_request->status = DISK_STATUS_IO_SUCCESS;
          current_disk_request = 0;
        }
      } else {
        if (status & 0x1) {
          ata_error_info();
        } else {
          c_printf("no DRQ, but not ERR bit set\n");
        }
      }
  
      break;
    case DISK_CMD_WRITE:
    case DISK_CMD_REQUEST:
    default:
      c_printf("NOT YET IMPLEMENTED\n");
      break;
    }
  }

  if (ata_alt_status(0) & ATA_STATUS_ERROR)
    ata_error_info();
  
  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
  __outb(PIC_SLAVE_CMD_PORT, PIC_EOI);

  //asm volatile ("sti" : : :"memory");
}

uint8_t ata_alt_status(uint32_t poll)
{
  uint8_t status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);

  int32_t i;
  for (i = 0; i < poll; ++i) {
    status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);
  }
  return status;
}

void ata_error_info()
{
  current_disk_request->status = DISK_STATUS_IO_ERROR;
  uint8_t err_reg = __inb(ata_cmd_reg | ATA_CMD_R_ERROR);
  c_printf("ATA ERROR: 0x%x\n", err_reg);
  if (err_reg & 0x02) c_printf(" NM");
  if (err_reg & 0x04) c_printf(" ABRT");
  if (err_reg & 0x08) c_printf(" MCR");
  if (err_reg & 0x10) c_printf(" IDNF");
  if (err_reg & 0x20) c_printf(" MC");
  if (err_reg & 0x40) c_printf(" UNC");
  c_printf("\n");
}

/*
 * ATA: READ MULTIPLE
 * PIT data-in protocol
 */
void ata_read_multiple(disk_request_t *dr)
{
  current_disk_request = dr;

  // PIO protocol
  uint8_t status = ata_alt_status(0);
  while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_READY))
    status = ata_alt_status(0);

  // inputs
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_DEV_LBA_BIT | ATA_PRI_CHANNEL
                     | ((dr->lba >> 24) & 0xF));

  __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, dr->num_blocks);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_LOW, dr->lba & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_MID, (dr->lba >> 8) & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_HIGH, (dr->lba >> 16) & 0xFF);

  //c_printf("ATA READ_MULTIPLE(%d, %d)   [0x%x]\n", dr->lba, dr->num_blocks, ata_alt_status(0));
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_READ_MULTIPLE);

  while (ata_alt_status(0) & ATA_STATUS_BUSY)
    ;

  //c_printf("ALT_STATUS: 0x%x\n", ata_alt_status(0));

  uint32_t nblocks = (dr->num_blocks) ? dr->num_blocks : 256;
  while (dr->blocks_complete < nblocks) {
    //c_printf("ALT_STATUS: 0x%x\n", ata_alt_status(0));
    
    if (ata_alt_status(0) & ATA_STATUS_ERROR) {
      ata_error_info();
      panic("ATA error\n");
    }
  }
}

void ata_identify_device()
{
  // we are going to poll for this command...
  asm( "cli" );

  // set nIEN
  //__outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);

  // we need to wait for DRDY
  uint8_t status = ata_alt_status(0);

  //c_printf("ATA: identify device: waiting for DRDY...\n");
  while ( !(status & ATA_STATUS_READY) ) {
    status = ata_alt_status(0);
    //c_printf("s: %x\n");
  }

  // write the IDENTIFY DEVICE command
  //c_printf("ATA: identify device: sending command byte\n");
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_IDENTIFY_DEVICE);
  
  // now transfer 256 words of data
  int32_t word_i;
  uint16_t ata_id_device_data[256];

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
  //__outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x00 );

  // clear pending interrupt
  __inb(ata_cmd_reg | ATA_CMD_R_STATUS);
 
  // Subaru Technica International (STi)
  asm( "sti" );
}

void ata_print_device_info( uint16_t *dev_data )
{
  // check for NULL pointers...

  // see Table 16 of ATA/ATAPI-7 Volume 1
  if (dev_data[59] & 0x0100)
    c_printf("SECTORS PER BLOCK: %x\n", dev_data[59] & 0xFF );

  // words 27-46: model number (40 ASCII characters)
  char model_num[ 41 ];
  memcpy( (void *) &model_num[0], (void *) &dev_data[ 27 ], 40 );

  model_num[ 40 ] = '\0';

  // un-scramble?
  int32_t len = 40;
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
  uint32_t user_addressable_sectors = *( (uint32_t *) &dev_data[ 60 ]);

  c_printf("Disk Size: %d MB (%d sectors)\n", (user_addressable_sectors * 512)/1000000, user_addressable_sectors);

  // check for multi-word DMA support (word 63)
  uint16_t dma_modes = dev_data[ 63 ];
  
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
