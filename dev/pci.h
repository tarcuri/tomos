#ifndef PCI_H
#define PCI_H

#define PCI_VENDOR_INTEL		0x8086
#define PCI_INTEL_IDE_CTRL		0x7010

#define PCI_CONFIG_ADDRESS		0xCF8
#define PCI_CONFIG_DATA			0xCFC

#define IDE_BAR_0			0x1F0		// primary control channel i/o space
#define IDE_BAR_1			0x3F4		// device control register
#define IDE_BAR_2			0x170		// sec. control channel i/o space
#define IDE_BAR_3			0x374		// alt. status register
#define IDE_BAR_4					// bus master IDE

/**
 * CONFIG_ADDRESS register
 */
#define PCI_CONFIG_ADDR_REGISTER	0x000000FC
#define PCI_CONFIG_ADDR_FUNCTION	0x00000700		// function number
#define PCI_CONFIG_ADDR_DEVICE		0x0000F800		// device number
#define PCI_CONFIG_ADDR_BUS		0x00FF0000		// bus number
#define PCI_CONFIG_ADDR_ENABLE		0x80000000		// enable bit 31

typedef struct pci_device
{
  unsigned int bus;
  unsigned int slot;
  unsigned int function;
} pci_dev_t;

void pci_init(void);
void pci_detect_devices(void);

pci_dev_t pci_find_device(unsigned short vendor, unsigned short device);

unsigned int pci_read_config_dword_dev(pci_dev_t d, int);
unsigned int pci_read_config_dword(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned char offset);
unsigned short pci_read_config_word(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned short offset);

#endif
