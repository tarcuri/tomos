#include "pci.h"
#include "console.h"
#include "support.h"

pci_dev_t ide_controller;

void
pci_init()
{
  // probe for intel devices
  c_printf("[pci]     enumerating PCI bus...\n");

  // TODO: detect/verify class codes
  pci_detect_devices();

  c_printf("[pci]     probing IDE controller [%x:%x:%x]\n",
           ide_controller.bus, ide_controller.slot, ide_controller.function);
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

unsigned int pci_read_config_dword_dev(pci_dev_t d, int offset)
{
  unsigned int address;
  unsigned int lbus = (unsigned int) d.bus;
  unsigned int lslot = (unsigned int) d.slot;
  unsigned int lfunc = (unsigned int) d.function;


  // create configuration addresss
  address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | 
                           (offset & PCI_CONFIG_ADDR_REGISTER) |
                           (PCI_CONFIG_ADDR_ENABLE));

  // write it to the CONFIG_ADDRESS port
  __outl(PCI_CONFIG_ADDRESS, address);

  return __inl(PCI_CONFIG_DATA);
}

unsigned short 
pci_read_config_word(unsigned short bus, unsigned short slot, unsigned short func, unsigned short offset)
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

void
pci_detect_devices()
{
  unsigned short bus, slot, func;

  for (bus = 0; bus < 0xF; ++bus) {
    for (slot = 0; slot < 0xFF; ++slot) {
      for (func = 0; func < 0xF; ++func) {
        unsigned short vend_id = pci_read_config_word(bus, slot, func, 0);
        unsigned short dev_id  = pci_read_config_word(bus, slot, func, 2);

        if (vend_id != 0xFFFF) {
          if ((vend_id == PCI_VENDOR_INTEL) && (dev_id == PCI_INTEL_IDE_CTRL)) {
            c_printf("[pci]     IDE controller found [%x:%x:%x]\n", bus, slot, func);
            ide_controller.bus = bus;
            ide_controller.slot = slot;
            ide_controller.function = func;
          } else {
            c_printf("[pci]     vendor: %x, device %x [%x:%x:%x]\n", vend_id, dev_id, bus, slot, func);
          }
        } else {
          break;
        }
      }
    }
  }
}

pci_dev_t 
pci_find_device( unsigned short vendor, unsigned short device)
{
  unsigned short bus;
  unsigned short slot;
  unsigned char func;

  pci_dev_t dev = { 0xffffffff, 0xffffffff, 0xffffffff };
	
  for ( bus = 0; bus < 0xffff; ++bus ) {
    for ( slot = 0; slot < 0xffff; ++slot ) {
      for ( func = 0; func < 0xff; ++func ) {
        unsigned short vend_id = pci_read_config_word(bus, slot, func, 2);
        unsigned short dev_id  = pci_read_config_word(bus, slot, func, 0);

        if ((vendor == vend_id) && (device == dev_id)) {
	  c_printf("PCI: found PCI device with device_id [ 0x%04x ] and vendor_id [ 0x%04x ]\n",
                   device, vendor );

          dev.bus      = bus;
	  dev.slot     = slot;
	  dev.function = func;

          return dev;
	}
      }
    }
  }

  return dev;
}
