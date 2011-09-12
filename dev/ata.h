#ifndef ATA_H
#define ATA_H

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

/**
 * Error register (RO)
 */
#define ATA_ERROR_CMD_ABORTED		(1 << 2)

/**
 * Data register (R/W) 16-bit word
 *
 * "This register shall be accessed for host PIO data transfer only when DRQ is set to one
 *  and DMACK- is not asserted."
 */


/**
 * Device register (R/W) (also parameters when Command register is written)
 *
 * "This register shall be written only when both BSY and DRQ are cleared to zero
 *  and DMACK- is not asserted."
 */
#define ATA_DEVICE_DEV_BIT		(1 << 4)


#define    ATA_IDENT_DEVICETYPE   0
#define    ATA_IDENT_CYLINDERS   2
#define    ATA_IDENT_HEADS      6
#define    ATA_IDENT_SECTORS      12
#define    ATA_IDENT_SERIAL   20
#define    ATA_IDENT_MODEL      54
#define    ATA_IDENT_CAPABILITIES   98
#define    ATA_IDENT_FIELDVALID   106
#define    ATA_IDENT_MAX_LBA   120
#define   ATA_IDENT_COMMANDSETS   164
#define    ATA_IDENT_MAX_LBA_EXT   200


// ATA commands
void ata_read_sectors();
void ata_write_sectors();

#endif
