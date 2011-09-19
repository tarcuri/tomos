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
#define PCI_INTEL_PRO1000_ETHERNET	0x100E
#define PCI_INTEL_MEMORY_NATOMA		0x1237
#define PCI_INTEL_AUREAL_SOUND		0x2415		// Aureal (AD1881 SOUNDMAX) Placa Mãe Asaki P3-141
#define PCI_INTEL_PIIX3_PCI_ISA_BRIDGE	0x7000
#define PCI_INTEL_PIIX3_IDE_INTERFACE	0x7010		// PIIX3 IDE Interface (Triton II) [function 1]
#define PCI_INTEL_PIIX4_IDE_CONTROLLER	0x7111		// PIIX4/4E/4M IDE Controller
#define PCI_INTEL_PIIX4_PWR_CONTROLLER	0x7113		// PIIX4/4E/4M Power Management Controller
#define PCI_INTEL_PATA_IDE_CONTROLLER	0x269E		// PATA100 IDE Controller 631x/632x ESB2

#define PCI_CONFIG_ADDRESS		0xCF8
#define PCI_CONFIG_DATA			0xCFC

// finish off the PCI device status register bits
#define PCI_STATUS_INTERRUPT_STATE	0x08

/**
 * CONFIG_ADDRESS register
 */
#define PCI_CONFIG_ADDR_REGISTER	0x000000FC
#define PCI_CONFIG_ADDR_FUNCTION	0x00000700		// function number
#define PCI_CONFIG_ADDR_DEVICE		0x0000F800		// device number
#define PCI_CONFIG_ADDR_BUS		0x00FF0000		// bus number
#define PCI_CONFIG_ADDR_ENABLE		0x80000000		// enable bit 31


typedef struct pci_vendor
{
  unsigned short id;
  char *short_name;
  char *long_name;
} pci_vendor_t;

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

typedef struct pci_class_code
{
  unsigned char class_code;
  unsigned char sub_class;
  unsigned char prog_if;
  char *base_desc;
  char *sub_desc;
  char *prog_desc;
} pci_class_t;

// data
static pci_vendor_t pci_vendors[] =
{
  { 0x1022, "AMD", "Advanced Micro Devices" },
  { 0x106B, "Apple", "Apple Computer" },
  { 0x10DE, "Nvidia", "NVIDIA Corportation" },
  { 0x8086, "Intel", "Intel Corporation" },
  { 0x80EE, "Oracle", "Oracle VirtualBox" }
};

static pci_class_t pci_class_table [] =
{
  { 0x00, 0x00, 0x00, "Pre-2.0 PCI Specification Device", "Non-VGA","" },
  { 0x00, 0x01, 0x00, "Pre-2.0 PCI Specification Device", "VGA Compatible", "" },

  { 0x01, 0x00, 0x00, "Mass Storage Controller", "SCSI", "" },
  { 0x01, 0x01, 0x00, "Mass Storage Controller", "IDE", "" },
  { 0x01, 0x02, 0x00, "Mass Storage Controller", "Floppy", "" },
  { 0x01, 0x03, 0x00, "Mass Storage Controller", "IPI", "" },
  { 0x01, 0x04, 0x00, "Mass Storage Controller", "RAID", "" },
  { 0x01, 0x06, 0x00, "Mass Storage Controller", "SATA", "" },
  { 0x01, 0x80, 0x00, "Mass Storage Controller", "Other", "" },

  { 0x02, 0x00, 0x00, "Network Controller", "Ethernet", "" },
  { 0x02, 0x01, 0x00, "Network Controller", "Token Ring", "" },
  { 0x02, 0x02, 0x00, "Network Controller", "FDDI", "" },
  { 0x02, 0x03, 0x00, "Network Controller", "ATM", "" },
  { 0x02, 0x80, 0x00, "Network Controller", "Other", "" },

  { 0x01, 0x05, 0x20, "Mass Storage Controller", "ATA controller with single DMA", "" }, 
  { 0x01, 0x05, 0x30, "Mass Storage Controller", "ATA controller with chained DMA", "" },

  { 0x03, 0x00, 0x00, "Display Controller", "PC Compatible", "VGA" },
  { 0x03, 0x00, 0x01, "Display Controller", "PC Compatible", "8514" },
  { 0x03, 0x01, 0x00, "Display Controller", "XGA", "" },
  { 0x03, 0x80, 0x00, "Display Controller", "Other", "" },

  { 0x04, 0x00, 0x00, "Multimedia Device", "Video", "" },
  { 0x04, 0x01, 0x00, "Multimedia Device", "Audio", "" },
  { 0x04, 0x80, 0x00, "Multimedia Device", "Other", "" },

  { 0x05, 0x00, 0x00, "Memory Controller", "RAM", "" },
  { 0x05, 0x01, 0x00, "Memory Controller", "Flash", "" },
  { 0x05, 0x80, 0x00, "Memory Controller", "Other", "" },

  { 0x06, 0x00, 0x00, "Bridge Device", "Host/PCI", "" },
  { 0x06, 0x01, 0x00, "Bridge Device", "PCI/ISA", "" },
  { 0x06, 0x02, 0x00, "Bridge Device", "PCI/EISA", "" },
  { 0x06, 0x03, 0x00, "Bridge Device", "PCI/Micro Channel", "" },
  { 0x06, 0x04, 0x00, "Bridge Device", "PCI/PCI", "" },
  { 0x06, 0x05, 0x00, "Bridge Device", "PCI/PCMCIA", "" },
  { 0x06, 0x06, 0x00, "Bridge Device", "PCI/NuBus", "" },
  { 0x06, 0x07, 0x00, "Bridge Device", "PCI/CardBus", "" },
  { 0x06, 0x80, 0x00, "Bridge Device", "Other", "" },

  { 0x07, 0x00, 0x00, "Simple Communications Controller", "Serial", "Generic XT Compatible" },
  { 0x07, 0x00, 0x01, "Simple Communications Controller", "Serial", "16450 Compatible" },
  { 0x07, 0x00, 0x02, "Simple Communications Controller", "Serial", "16550 Compatible" },
  { 0x07, 0x01, 0x00, "Simple Communications Controller", "Parallel", "Standard" },
  { 0x07, 0x01, 0x00, "Simple Communications Controller", "Parallel", "Bidirectional" },
  { 0x07, 0x01, 0x01, "Simple Communications Controller", "Parallel", "ECP 1.X Compliant" },
  { 0x07, 0x80, 0x02, "Simple Communications Controller", "Other", "" },

  { 0x08, 0x00, 0x00, "Base Systems Peripheral", "PIC (Programmable Interrupt Controller)", "Generic 8259" },
  { 0x08, 0x00, 0x01, "Base Systems Peripheral", "PIC (Programmable Interrupt Controller)", "ISA" },
  { 0x08, 0x00, 0x02, "Base Systems Peripheral", "PIC (Programmable Interrupt Controller)", "PCI" },
  { 0x08, 0x01, 0x00, "Base Systems Peripheral", "DMA (Direct Memory Access)", "Generic 8259" },
  { 0x08, 0x01, 0x01, "Base Systems Peripheral", "DMA (Direct Memory Access)", "ISA" },
  { 0x08, 0x01, 0x02, "Base Systems Peripheral", "DMA (Direct Memory Access)", "EISA" },
  { 0x08, 0x02, 0x00, "Base Systems Peripheral", "System Timer", "Generic 8259" },
  { 0x08, 0x02, 0x01, "Base Systems Peripheral", "System Timer", "ISA" },
  { 0x08, 0x02, 0x02, "Base Systems Peripheral", "System Timer", "EISA" },
  { 0x08, 0x03, 0x00, "Base Systems Peripheral", "RTC (Real Time Clock)", "Generic" },
  { 0x08, 0x03, 0x01, "Base Systems Peripheral", "RTC (Real Time Clock)", "ISA" },
  { 0x08, 0x80, 0x00, "Base Systems Peripheral", "Other", "" },

  { 0x09, 0x00, 0x00, "Input Device", "Keyboard", "" },
  { 0x09, 0x01, 0x00, "Input Device", "Digitizer (Pen)", "" },
  { 0x09, 0x02, 0x00, "Input Device", "Mouse", "" },
  { 0x09, 0x80, 0x00, "Input Device", "Other", "" },

  { 0x0A, 0x00, 0x00, "Docking Station", "Generic", "" },
  { 0x0A, 0x80, 0x00, "Docking Station", "Other", "" },

  { 0x0B, 0x00, 0x00, "Processor", "i386", "" },
  { 0x0B, 0x01, 0x00, "Processor", "i486", "" },
  { 0x0B, 0x02, 0x00, "Processor", "Pentium", "" },
  { 0x0B, 0x10, 0x00, "Processor", "Alpha", "" },
  { 0x0B, 0x20, 0x00, "Processor", "Power PC", "" },
  { 0x0B, 0x80, 0x00, "Processor", "Co-processor", "" },

  { 0x0C, 0x00, 0x00, "Serial Bus Controller", "Firewire (IEEE 1394)", "" },
  { 0x0C, 0x01, 0x00, "Serial Bus Controller", "ACCESS.bus", "" },
  { 0x0C, 0x02, 0x00, "Serial Bus Controller", "SSA (Serial Storage Archetecture)", "" },
  { 0x0C, 0x03, 0x00, "Serial Bus Controller", "USB (Universal Serial Bus)", "" },
  { 0x0C, 0x04, 0x00, "Serial Bus Controller", "Fibre Channel", "" },

  { 0xFF, 0x00, 0x00, "Unknown", "Device Does Not Fit In Class Codes", "UDF" },
};

static unsigned int pci_class_list_size  = sizeof(pci_class_table) / sizeof(pci_class_t);
static unsigned int pci_vendor_list_size = sizeof(pci_vendors) / sizeof(pci_vendor_t);

pci_dev_t *pci_list_head;

// functions
void pci_init(void);
void pci_detect_devices(void);
void pci_probe_device_config(pci_dev_t *, int);

unsigned int pci_read_config_dword(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned char offset);
unsigned short pci_read_config_word(unsigned short bus, unsigned short slot,
                                    unsigned short func, unsigned char offset);

static int find_class_index(unsigned char class_code, unsigned char subclass);
static int find_vendor_index(unsigned short id);
#endif
