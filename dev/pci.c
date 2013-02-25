#include "pci.h"
#include "console.h"
#include "support.h"

#include "kernel/heap.h"

void
pci_init()
{
  // probe for intel devices
  c_printf("[pci]     enumerating PCI bus...\n");

  // store a list of all the pci devices
  pci_detect_devices();
}

// TODO: read an entrire config space into a structure
void pci_probe_device_config(pci_dev_t *dev, int echo)
{
  dev->vendor = pci_read_config_word(dev->bus, dev->slot, dev->func, 0);
  dev->device = pci_read_config_word(dev->bus, dev->slot, dev->func, 2);

  dev->command = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x04);
  dev->status  = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x06);

  dev->class_code    = (pci_read_config_word(dev->bus, dev->slot, dev->func, 0x0A) >> 8) & 0xFF;
  dev->subclass_code = pci_read_config_word(dev->bus, dev->slot, dev->func, 0x0A) & 0xFF;

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
    c_printf("B%x:D%x:F%x:  %s, Device: %x [%x]\n", dev->bus, dev->slot, dev->func,
             pci_vendors[find_vendor_index(dev->vendor)].long_name, dev->device, dev->header_type);


    int class_index = find_class_index(dev->class_code, dev->subclass_code); 
    if (class_index >= 0)
      c_printf("%s: %s\n", pci_class_table[class_index].base_desc, pci_class_table[class_index].sub_desc);
    else
      c_printf("Unknown PCI device class: %xh:%xh\n", dev->class_code, dev->subclass_code);

    c_printf("Command Register    : 0x%x", dev->command);
    if (dev->command & 0x1)
      c_printf(" IO");
    if (dev->command & 0x2)
      c_printf(" MS");
    if (dev->command & 0x4)
      c_printf(" BM");
    c_printf("\n");
    c_printf("Status Register     : 0x%x\n", dev->status);
    if (dev->bar0)
      c_printf("Base Address Reg. 0 : 0x%x\n", dev->bar0);
    if (dev->bar1)
      c_printf("Base Address Reg. 1 : 0x%x\n", dev->bar1);
    if (dev->bar2)
      c_printf("Base Address Reg. 2 : 0x%x\n", dev->bar2);
    if (dev->bar3)
      c_printf("Base Address Reg. 3 : 0x%x\n", dev->bar3);
    if (dev->bar4)
      c_printf("Bus Master Address  : 0x%x\n", dev->bar4);
    if (dev->bar5)
      c_printf("Base Address Reg. 5 : 0x%x\n", dev->bar5);
    c_printf("Interrupt Line (IRQ): 0x%x\n", dev->irq);
    c_printf("Interrupt Pin       : 0x%x\n", dev->intr_pin);
  }
}



void
pci_detect_devices()
{
  unsigned short bus, slot, func;

  //pci_list_head = (pci_dev_t *) kmalloc(sizeof(pci_dev_t));
  pci_list_head   = 0;
  pci_dev_t *dev  = 0;
  pci_dev_t *prev = 0;

  dev->prev = 0;
  dev->next = 0;

  for (bus = 0; bus < 3; ++bus) {
    for (slot = 0; slot < 32; ++slot) {

      // test function 0, then 1-7 iff it is a multifunctional device
      for (func = 0; func < 8; ++func) {
        unsigned short vendor = pci_read_config_word(bus, slot, func, 0);
        uint32_t phys;
        if (vendor != 0xFFFF) {
          dev = (pci_dev_t *) kmalloc_p(sizeof(pci_dev_t), 0, &phys);

          dev->bus  = bus;
          dev->slot = slot;
          dev->func = func;

          // insert it into the device list
          if (!pci_list_head) {
            dev->next = 0;
            dev->prev = 0;
            pci_list_head = dev;
            prev = dev;
          } else {
            dev->prev  = prev;
            dev->next  = 0;
            prev->next = dev;
            prev = dev;
          }

          // only continue if this is a multifuncitonal device
          if ((pci_read_config_word(bus, slot, func, 0x0E) & 0xFF) & 0x80)
            continue;
          else
            break;
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

static int find_class_index(unsigned char class, unsigned char subclass)
{
  pci_class_t *device_class;

  int i;
  for (i = 0; i < pci_class_list_size; ++i)
    if ((pci_class_table[i].class_code == class) && (pci_class_table[i].sub_class == subclass))
      return i;

  return -1;
}

static int find_vendor_index(unsigned short id)
{
  int i;
  for (i = 0; i < pci_vendor_list_size; ++i)
    if (pci_vendors[i].id == id)
      return i;

  return -1;
}
