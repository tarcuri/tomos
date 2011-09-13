#include "dev/ata.h"
#include "dev/console.h"

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

  // clear nIEN
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x0);

  c_printf("ata_alt_status: 0x%x\n", ata_alt_status());

  ata_state = ATA_INIT;

  //ata_identify_device();
  //ata_do_pio_data();
  unsigned char buffer[512];
  ata_read_sector(0, buffer);
}


void ata_isr(int vector, int code)
{
  asm("cli");

  c_printf("ATA INTERRUPT\n");
  if (ata_state == ATA_INTRQ_WAIT) {
    // expected this interrupt as per PIO protocol
    c_printf("expected interrupt received\n");

    // now in PIO Check_Status state
    ata_state = ATA_CHECK_STATUS;

    // wait 400ns before reading STATUS register (if interrupts were off)

    // i don't think we need to here since nIEN is clear...
    unsigned char status = ata_alt_status();

    while (status & ATA_STATUS_BUSY)
      status = ata_alt_status();

    if (status & ATA_STATUS_DRQ) {
      // ready to transfer data
      unsigned char sector[512];

      c_printf("reading sector");
      int i;
      for (i = 0; status & ATA_STATUS_DRQ; i += 2) {
        sector[i] = __inw(ata_cmd_reg | ATA_CMD_R_DATA);
        c_printf("%x", sector[i]);

        status = ata_alt_status();
        if (i >= 512)
          break;
      }
      c_printf("DONE\n");
    } else {
      // BSY and DRQ are clear, command completed with error
      c_printf("ATA command completed with error!\n");
    }
  }

  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
  __outb(PIC_SLAVE_CMD_PORT, PIC_EOI);

  asm("sti");
}

unsigned char ata_alt_status()
{
  unsigned char status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);

  int i;
  for (i = 0; i < 5; ++i) {
    status = __inb(ata_ctrl_reg | ATA_CTRL_R_ALT_STATUS);
  }
  return status;
}

/**
 * ATA: READ SECTOR (S)
 *
 */
void ata_read_sector(unsigned int lba, void *buf)
{
  // PIO protocol
  unsigned char status = ata_alt_status();
  while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_READY))
    status = ata_alt_status();

  // inputs
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, ATA_DEV_LBA_BIT | ATA_PRI_CHANNEL | ((lba >> 24) & 0xFF));

  __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, 1);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_LOW, lba & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_MID, (lba >> 8) & 0xFF);
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_HIGH, (lba >> 16) & 0xFF);

  // issue command, enter INTRQ_wait state
  ata_state = ATA_INTRQ_WAIT;
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_READ_SECTORS);
}

void ata_do_pio_data()
{
        /*
  LBA partitioning:
  1st 8 bits: ATA_CMD_BR_CYL_LOW
  2nd 8 bits: ATA_CMD_BR_CYL_MID
  3rd 8 bits: ATA_CMD_BR_CYL_HIGH
	  last 4 bits: ATA_CMD_BR_DRIVE_SELECT
*/
  c_printf("ATA: PIO data command\n");
  
  unsigned char status = ata_alt_status();
  while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_READY)) {
    status = ata_alt_status();
  }

  unsigned int lba = 0;

  c_printf("ATA: request: initiating transfer command [ 0x%x ]\n", ata_alt_status());
  unsigned char lba_low  = (unsigned char) lba;
  unsigned char lba_mid  = (unsigned char) lba >> 8;
  unsigned char lba_high = (unsigned char) lba >> 16;

  // set the device register, handle highest 4 bits of LBA
  __outb(ata_cmd_reg | ATA_CMD_R_DEVICE, 0x40 | ((lba >> 24) & 0x0f) | ATA_PRI_CHANNEL);

  __outb(ata_cmd_reg | ATA_CMD_R_LBA_LOW, lba_low );
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_MID, lba_mid );
  __outb(ata_cmd_reg | ATA_CMD_R_LBA_HIGH, lba_high );
  __outb(ata_cmd_reg | ATA_CMD_R_SECTOR_COUNT, 0x01 );

  // now write READ SECTOR(S) to the command register
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_READ_SECTORS);

  // NOTE: interrupts are ON!!!

  // now we need to wait for an IRQ to proceed....
  // OR we can poll ;)

  unsigned int res = ata_alt_status();

  if (res & ATA_STATUS_BUSY)
    c_printf("BUSY AFTER READ_SECTORS\n");

  if (res & ATA_STATUS_DEVICE_FAULT)
    panic("ATA DEVICE FAULT\n");
  if (res & ATA_STATUS_DRQ)
    c_printf("DRQ is set!\n");
  if (res & ATA_STATUS_ERROR)
    panic("ATA READ_SECTORS error!!\n");
}


void ata_identify_device()
{
  // we are going to poll for this command...
  asm( "cli" );

  // set nIEN
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, ATA_DEV_CONTROL_nIEN);

  // we need to wait for DRDY
  unsigned char status = ata_alt_status();

  c_printf("ATA: identify device: waiting for DRDY...\n");
  while ( !(status & ATA_STATUS_READY) ) {
    status = ata_alt_status();
    //c_printf("s: %x\n");
  }

  // write the IDENTIFY DEVICE command
  c_printf("ATA: identify device: sending command byte\n");
  __outb(ata_cmd_reg | ATA_CMD_R_COMMAND, ATA_IDENTIFY_DEVICE);
  
  // now transfer 256 words of data
  int word_i;
  unsigned short ata_id_device_data[256];

  /*
  ** i)  wait for DRQ=1 and BSY=0 (cycle status register)
  ** ii) transfer a word, if more remain, repeat
  */	
  // now transfer a word
  c_printf("ATA: identify device: reading coniguration data");
  for (word_i = 0; word_i < 256; ++word_i) {
    // wait for DRQ
    while ((status & ATA_STATUS_BUSY) || !(status & ATA_STATUS_DRQ))
      status = ata_alt_status();

    // read from Data register
    ata_id_device_data[ word_i ] = __inw(ata_cmd_reg | ATA_CMD_R_DATA);

    c_printf(".");
  }
  c_printf("\n");
  
  // read status
  status = ata_alt_status();

  ata_print_device_info( (unsigned short *) &ata_id_device_data[0] );

  // clear nIEN so we get interrupts again
  __outb(ata_ctrl_reg | ATA_CTRL_R_DEVICE, 0x00 );

  // Subaru Technica International (STi)
  asm( "sti" );
}

void memcpy(void *dst, void *src, unsigned int n)
{
  unsigned char *d = (unsigned char *) dst;
  unsigned char *s = (unsigned char *) src;
  int i;
  for (i = 0; i < n; ++i)
    *d++ = *s++;
}

void ata_print_device_info( unsigned short *dev_data )
{
  // check for NULL pointers...
  c_printf("ATA IDENTIFY DEVICE:\n\n");

  // see Table 16 of ATA/ATAPI-7 Volume 1

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

  // word 88: Ultra DMA modes
  unsigned short ultra_dma_modes = dev_data[ 88 ];

  c_printf("Ultra DMA modes 0x%x\n", ultra_dma_modes );

  if ( ultra_dma_modes & 0x0030 ) {
  	c_printf("Ultra DMA mode 6 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0020 ) {
  	c_printf("Ultra DMA mode 5 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0010 ) {
  	c_printf("Ultra DMA mode 4 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0008 ) {
  	c_printf("Ultra DMA mode 3 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0004 ) {
  	c_printf("Ultra DMA mode 2 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0002 ) {
  	c_printf("Ultra DMA mode 1 and below are supported\n");
  } else if ( ultra_dma_modes & 0x0001 ) {
  	c_printf("Ultra DMA mode 0 is supported\n");
  }
  
}
