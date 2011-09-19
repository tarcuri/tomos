#include "pci.h"
#include "console.h"
#include "support.h"

#include "heap.h"

void
pci_init()
{
  // probe for intel devices
  c_printf("[pci]     enumerating PCI bus...\n");

  // store a list of all the pci devices
  pci_detect_devices();
}

void pci_probe_device_config(pci_dev_t *dev, int echo)
{
  dev->vendor = pci_read_config_word(dev->bus, dev->slot, dev->func, 0);
  dev->device = pci_read_config_word(dev->bus, dev->slot, dev->func, 2);

  dev->command = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x04);
  dev->status  = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x06);

  dev->class_code    = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x0A) & 0xFF;
  dev->subclass_code = (pci_read_config_word(dev->bus, dev->slot, dev->func, 0x0A) >> 8) & 0x7F;

  dev->header_type = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x0E) & 0xAF;


  dev->bar0 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x10);
  dev->bar1 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x14);
  dev->bar2 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x18);
  dev->bar3 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x1C);
  dev->bar4 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x20);
  dev->bar5 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x24);

  dev->irq      = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x3C) & 0xFF;
  dev->intr_pin = (pci_read_config_word(dev->bus, dev->slot, dev->func, 0x3C) >> 8) & 0xFF;

  if (echo) {
    c_printf("PCI[%x:%x:%x]  Vendor: %x, Device: %x\n", dev->bus, dev->slot, dev->func, dev->vendor, dev->device);
    c_printf("Class Code          : 0x%x\n", dev->class_code);
    c_printf("Sub-Class Code      : 0x%x\n", dev->subclass_code);
    c_printf("Header Type         : 0x%x\n", dev->header_type);
    c_printf("Command Register    : 0x%x\n", dev->command);
    if (dev->command & 0x7 == 0x7)
      c_printf("    BUS MASTER, MEMORY SPACE, IO SPACE\n");
    c_printf("Status Register     : 0x%x\n", dev->status);
    c_printf("Base Address Reg. 0 : 0x%x\n", dev->bar0);
    c_printf("Base Address Reg. 1 : 0x%x\n", dev->bar1);
    c_printf("Base Address Reg. 2 : 0x%x\n", dev->bar2);
    c_printf("Base Address Reg. 3 : 0x%x\n", dev->bar3);
    c_printf("Bus Master Address  : 0x%x\n", dev->bar4);
    c_printf("Base Address Reg. 5 : 0x%x\n", dev->bar5);
    c_printf("Interrupt Line (IRQ): 0x%x\n", dev->irq);
    c_printf("Interrupt Pin       : 0x%x\n", dev->intr_pin);
  }
}



void
pci_detect_devices()
{
  unsigned short bus, slot, func;

  //pci_list_head = (pci_dev_t *) kmalloc(sizeof(pci_dev_t), 0);
  pci_list_head   = 0;
  pci_dev_t *dev  = 0;
  pci_dev_t *prev = 0;

  dev->prev = 0;
  dev->next = 0;

  for (bus = 0; bus < 3; ++bus) {
    for (slot = 0; slot < 32; ++slot) {
      for (func = 0; func < 8; ++func) {
        unsigned short vendor = pci_read_config_word(bus, slot, func, 0);
        if (vendor != 0xFFFF) {
          dev = (pci_dev_t *) kmalloc(sizeof(pci_dev_t), 0);

          // add it to the list
          dev->bus  = bus;
          dev->slot = slot;
          dev->func = func;

          if (!pci_list_head) {
            dev->next = 0;
            dev->prev = 0;
            pci_list_head = dev;	// pci device list pointer
            prev = dev;
          } else {
            dev->prev  = prev;
            dev->next  = 0;
            prev->next = dev;
            prev = dev;
          }
        } // func
      } // slot
    } // bus 
  }
}

unsigned int
pci_read_config_dword(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset)
{
  unsigned int address;
  unsigned int lbus = (unsigned int) bus;
  unsigned int lslot = (unsigned int) slot;
  unsigned int lfunc = (unsigned int) func;


  // create configuration addresss
  address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | 
                           (offset & PCI_CONFIG_ADDR_REGISTER) |
                           (PCI_CONFIG_ADDR_ENABLE));

  // write it to the CONFIG_ADDRESS port
  __outl(PCI_CONFIG_ADDRESS, address);

  return __inl(PCI_CONFIG_DATA);
}

unsigned short 
pci_read_config_word(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset)
{
  unsigned int address;
  unsigned int lbus = (unsigned int) bus;
  unsigned int lslot = (unsigned int) slot;
  unsigned int lfunc = (unsigned int) func;

  unsigned short tmp = 0;

  // create configuration addresss
  address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | 
                           (offset & PCI_CONFIG_ADDR_REGISTER) |
                           (PCI_CONFIG_ADDR_ENABLE));

  // write it to the CONFIG_ADDRESS port
  __outl(PCI_CONFIG_ADDRESS, address);

  // choose the first word of the 32-bit register
  tmp = (unsigned short)((__inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
  return tmp;
}
