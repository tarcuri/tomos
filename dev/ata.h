#ifndef ATA_H
#define ATA_H

// command block registers
#define ATA_PRI_CMD_REGISTER		0x01F0
#define ATA_SEC_CMD_REGISTER		0x0170

#define	ATA_CTRL_ALT_STATUS			0x06
#define ATA_CTRL_DEV_CONTROL			0x06

#define IDE_CTRL_BLK_REG_DEVICE_CONTROL		0x0006
#define IDE_CTRL_BLK_REG_ALT_STATUS		0x0006

// control block register addresses
#define ATA_PRI_CTRL_REGISTER		0x03f4
#define ATA_SEC_CTRL_REGISTER 		0x0374
// device status
#define ATA_DEVICE_READY                0x01
#define ATA_DEVICE_BUSY                 0x02
#define ATA_DEVICE_UNAVAILABLE          0x03

// command block register offsets
#define ATA_CMD_BR_DATA			0x00	// data (r/w)
#define ATA_CMD_BR_ERROR		0x01	// error status (ro)
#define ATA_CMD_BR_FEATURES		0x01	// features (wo)
#define ATA_CMD_BR_SEC_COUNT		0x02	// sector count (r/w)
#define ATA_CMD_BR_CYL_LOW		0x03	// cylinder low (r/w)
#define ATA_CMD_BR_CYL_MID		0x04	// cylinder mid (r/w)
#define ATA_CMD_BR_CYL_HIGH		0x05	// cylinder high (r/w)
#define ATA_CMD_BR_DRIVE_SELECT		0x06	// drive select (r/w)
#define ATA_CMD_BR_STATUS		0x07	// status (ro)
#define ATA_CMD_BR_COMMAND		0x07	// command (wo)

#define ATA_CMD_DATA			0x00	// data (r/w)
#define ATA_CMD_ERROR			0x01	// error status (ro)
#define ATA_CMD_FEATURES		0x01	// features (wo)
#define ATA_CMD_SEC_COUNT		0x02	// sector count (r/w)
#define ATA_CMD_CYL_LOW			0x03	// cylinder low (r/w)
#define ATA_CMD_CYL_MID			0x04	// cylinder mid (r/w)
#define ATA_CMD_CYL_HIGH		0x05	// cylinder high (r/w)
#define ATA_CMD_DRIVE_SELECT		0x06	// drive select (r/w)
#define ATA_CMD_STATUS			0x07	// status (ro)
#define ATA_CMD_COMMAND			0x07	// command (wo)

// IDE status register bits
#define ATA_CMD_BR_R_STS_BUSY  		0x80 	// bit 7 - busy
#define ATA_CMD_BR_R_STS_DRDY		0x40	// bit 6 - device ready
#define ATA_CMD_BR_R_STS_DWF		0x20	// bit 5 -
#define ATA_CMD_BR_R_STS_DSC		0x10	// bit 4 -
#define ATA_CMD_BR_R_STS_DRQ		0x08	// bit 3 - device request
#define ATA_CMD_BR_R_STS_CORR		0x04	// bit 2 - 
#define ATA_CMD_BR_R_STS_IDX		0x02	// bit 1 - 
#define ATA_CMD_BR_R_STS_ERR		0x01	// bit 0 - error

// the 5th bit of the device register selects the IDE drive (set to select drive 1)
#define ATA_DEV_REG_SELECT_LBA		0x40
#define ATA_DEV_REG_SELECT_1		0x10

// ATA commands
#define ATA_DEVICE_RESET		0x08
#define ATA_READ_SECTORS		0x20
#define ATA_WRITE_SECTORS		0x30
#define ATA_DEVICE_CONFIG_IDENTIFY     	0xb1
#define ATA_DEVICE_CONFIG_ID_FEATURES  	0xc2
#define ATA_IDENTIFY_DEVICE		0xec
#define ATA_FLUSH_CACHE                	0xe7

// ATA IO ports
#define ATA_DATA_IO_PORT		0x1F0		// r/w PIO data bytes here
#define ATA_FEAT_ERR_PORT		0x1F1		// used for ATAPI
#define ATA_SECT_COUNT_PORT		0x1F2		// number of sectors to r/w
#define ATA_LBA_LOW_PORT		0x1F3		// LBA low address
#define ATA_LBA_MID_PORT		0x1F4		// LBA mid address
#define ATA_LBA_HIGH_PORT		0x1F5		// LBA high address
#define ATA_DRIVE_IO_PORT		0x1F6		// r/w PIO data bytes here
#define ATA_CMD_STATUS_PORT		0x1F7		// send commands or read status

// the status byte
#define ATA_STATUS_ERROR		(1 << 0)
#define ATA_STATUS_DRQ			(1 << 3)	// drive has PIO data, or ready to accept data
#define ATA_STATUS_SERVICE		(1 << 4)	// overlapped mode service request
#define ATA_STATUS_DRIVE_FAULT		(1 << 5)	// drive fault error (does not set ERR)
#define ATA_STATUS_READY		(1 << 6)	// clear when spun down/erro, set otherwise
#define ATA_STATUS_BUSY			(1 << 7)	// preparing to send/recv. data (wait to clear)

/**
 * Device Control register (WO)
 *
 * "Allows host to software reset attached devices and to enable or disable the assertion of the
 *  INTRQ signal by a selected device."
 */
#define ATA_DEV_CONTROL_nIEN		(1 << 1)	// enables interrupts
#define ATA_DEV_CONTROL_SRESET		(1 << 2)
#define ATA_DEV_CONTROL_HOB		(1 << 7)	// set to read back the high-order byte of LBA48

/**
 * Alternate Status register (RO)
 */
#define ATA_ALT_STATUS_ERR		(1 << 0)
#define ATA_ALT_STATUS_DATA_REQ		(1 << 3)
#define ATA_ALT_STATUS_CMD_DEP		(1 << 4)
#define ATA_ALT_STATUS_DEV_FAULT	(1 << 5)
#define ATA_ALT_STATUS_DEV_READY	(1 << 6)
#define ATA_ALT_STATUS_BUSY		(1 << 7)



// 1) need to issue DEVICE_CONFIGURATION_SET command
#define ATA_PACKET			0xA0
#define ATA_IDENTIFY_PACKET_DEVICE	0xA1

// ATA commands
void ata_init(void);

#endif
