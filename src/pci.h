#ifndef PCI_H
#define PCI_H

#include "types.h"

#define PCI_MAX_FUNCTIONS 64

typedef struct {
  u8 bus;
  u8 slot;
  u8 function;
  u16 vendor_id;
  u16 device_id;
  u8 class_code;
  u8 subclass;
  u8 prog_if;
  u8 header_type;
} pci_device_t;

u32 pci_config_address(u8 bus, u8 slot, u8 function, u8 offset);
u32 pci_config_read32(u8 bus, u8 slot, u8 function, u8 offset);
u16 pci_config_read16(u8 bus, u8 slot, u8 function, u8 offset);
u8 pci_enumerate(pci_device_t *devices, u8 max_devices);
const char *pci_class_name(u8 class_code, u8 subclass);

#endif
