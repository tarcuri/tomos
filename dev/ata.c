#include "dev/ata.h"
#include "dev/console.h"

void ata_init()
{
  unsigned int ata_cmd_reg  = ATA_PRI_CMD_REGISTER;
  unsigned int ata_ctrl_reg = ATA_PRI_CTRL_REGISTER;

  // select device 1
  __outb( ata_cmd_reg | ATA_CMD_BR_DRIVE_SELECT,
         ATA_DEV_REG_SELECT_LBA | ATA_DEV_REG_SELECT_1 );	

  // clear nIEN
  __outb( ata_ctrl_reg | IDE_CTRL_BLK_REG_DEVICE_CONTROL, 0x00 );

	unsigned char status = __inb( ata_ctrl_reg | ATA_CTRL_ALT_STATUS );

        int i;
	for (i = 0; i < 5; ++i) {
		status = __inb( ata_ctrl_reg | ATA_CTRL_ALT_STATUS );
	}

  c_printf("alt_status: 0x%x\n", status);
  c_printf("[ata]     interrupts enabled\n");
}
