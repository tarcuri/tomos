#ifndef PCI_H
#define PCI_H

#define PCI_VENDOR_ORACLE		0x80EE
#define PCI_ORACLE_VIRT_BOX_GRAPHICS	0xBEEF
#define PCI_ORACLE_VIRT_BOX_UNKNOWN	0xCAFE

#define PCI_VENDOR_AMD			0x1022
#define PCI_AMD_ETHERNET_CONTROLER	0x2000		// AMD PCnet LANCE PCI Ethernet Controller

#define PCI_VENDOR_APPLE		0x106B

// _need_ to get the Nvidia docs...
#define PCI_VENDOR_NVIDIA		0x10DE		// well this makes sense, all nvidia chipsets in my rig
#define PCI_NVIDIA_IDE_CONTROLLER	0x0054		// NVidia nForce 4 SLI

#define PCI_VENDOR_INTEL		0x8086
#define PCI_INTEL_MEMORY_NATOMA		0x1237
#define PCI_INTEL_AUREAL_SOUND		0x2415		// Aureal (AD1881 SOUNDMAX) Placa Mãe Asaki P3-141
#define PCI_INTEL_PIIX3_PCI_ISA_BRIDGE	0x7000
#define PCI_INTEL_IDE_CTRL		0x7010
#define PCI_INTEL_PIIX4_IDE_CONTROLLER	0x7111		// PIIX4/4E/4M IDE Controller
#define PCI_INTEL_PIIX4_PWR_CONTROLLER	0x7113		// PIIX4/4E/4M Power Management Controller
#define PCI_INTEL_PATA_IDE_CONTROLLER	0x269E		// PATA100 IDE Controller 631x/632x ESB2
#define PCI_INTEL_IDE_BUS_MASTER	0xD001		// VirtualBox's PCI bus master location

#define PCI_CONFIG_ADDRESS		0xCF8
#define PCI_CONFIG_DATA			0xCFC

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
  unsigned int func;

  unsigned short vendor;
  unsigned short device;

  unsigned short command;
  unsigned short status;

  unsigned char class_code;
  unsigned char subclass_code;
  unsigned char header_type;


  unsigned int bar0;
  unsigned int bar1;
  unsigned int bar2;
  unsigned int bar3;
  unsigned int bar4;
  unsigned int bar5;

  unsigned char intr_pin;
  unsigned char irq;

  struct pci_device *next;
  struct pci_device *prev;
} pci_dev_t;

// data
pci_dev_t *pci_list_head;

// functions
void pci_init(void);
void pci_detect_devices(void);
void pci_probe_device_config(pci_dev_t *, int);

unsigned int pci_read_config_dword(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned char offset);
unsigned short pci_read_config_word(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned char offset);

#endif
