#ifndef ATA_H
#define ATA_H

#include <stdint.h>

#include "disk.h"
#include "device.h"

#define ATA_PRI_CHANNEL		0x00
#define ATA_SEC_CHANNEL		0x10

// ATA command block registers
#define ATA_PRI_COMMAND_REG		0x01F0
#define ATA_CMD_R_DATA			0x0000
#define ATA_CMD_R_FEATURES		0x0001
#define ATA_CMD_R_ERROR			0x0001
#define ATA_CMD_R_SECTOR_COUNT		0x0002
#define ATA_CMD_R_LBA_LOW		0x0003
#define ATA_CMD_R_LBA_MID		0x0004
#define ATA_CMD_R_LBA_HIGH		0x0005
#define ATA_CMD_R_DEVICE		0x0006
#define ATA_CMD_R_COMMAND		0x0007
#define ATA_CMD_R_STATUS		0x0007

#define ATA_DEV_LBA_BIT			0x40

// Intel ICH6 (for VirtualBox)

// ATA control block registers
#define ATA_PRI_CONTROL_REG		0x03F6
#define ATA_CTRL_R_DEVICE		0x0000
#define ATA_CTRL_R_ALT_STATUS		0x0000

// Device Control register (WO)
#define ATA_DEV_CONTROL_nIEN		(1 << 1)	// enables interrupts
#define ATA_DEV_CONTROL_SRESET		(1 << 2)
#define ATA_DEV_CONTROL_HOB		(1 << 7)	// set to read back the high-order byte of LBA48

// the status byte
#define ATA_STATUS_ERROR		(1 << 0)
#define ATA_STATUS_DRQ			(1 << 3)	// drive has PIO data, or ready to accept data
#define ATA_STATUS_SERVICE		(1 << 4)	// overlapped mode service request
#define ATA_STATUS_DEVICE_FAULT		(1 << 5)	// drive fault error (does not set ERR)
#define ATA_STATUS_READY		(1 << 6)	// clear when spun down/erro, set otherwise
#define ATA_STATUS_BUSY			(1 << 7)	// preparing to send/recv. data (wait to clear)

// ATA/ATAPI-7 commands
#define ATA_DEVICE_RESET		0x08
#define ATA_READ_SECTORS		0x20
#define ATA_WRITE_SECTORS		0x30
#define ATA_DEVICE_CONFIG_IDENTIFY	0xB1
#define ATA_DEVICE_CONFIG_ID_FEATURES	0xC2
#define ATA_READ_MULTIPLE		0xC4
#define ATA_SET_MULTIPLE		0xC6
#define ATA_IDENTIFY_DEVICE		0xEC
#define ATA_FLUSH_CACHE			0xE7

void ata_init(void);
void ata_isr(int32_t, int32_t);

// driver interface
device_t *ata_open(void);
int32_t ata_read(int32_t);
int32_t ata_write(int32_t);
int32_t ata_ctrl(uint32_t, void *);

// ATA commands
void ata_read_multiple(disk_request_t *);
void ata_write_multiple(disk_request_t *);

uint8_t ata_alt_status(uint32_t);
void ata_identify_device(void);
void ata_print_device_info(uint16_t *);

#endif
