#include "pci.h"

#include "io.h"

#define PCI_CONFIG_ADDRESS_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT 0xCFC

u32 pci_config_address(u8 bus, u8 slot, u8 function, u8 offset) {
  return (1u << 31) | ((u32)bus << 16) | ((u32)slot << 11) |
         ((u32)function << 8) | (offset & 0xFCu);
}

u32 pci_config_read32(u8 bus, u8 slot, u8 function, u8 offset) {
  outl(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, slot, function, offset));
  return inl(PCI_CONFIG_DATA_PORT);
}

u16 pci_config_read16(u8 bus, u8 slot, u8 function, u8 offset) {
  u32 value = pci_config_read32(bus, slot, function, offset);
  return (u16)((value >> ((offset & 2u) * 8u)) & 0xFFFFu);
}

static void pci_fill_device(pci_device_t *device, u8 bus, u8 slot, u8 function) {
  u32 id = pci_config_read32(bus, slot, function, 0x00);
  u32 class_reg = pci_config_read32(bus, slot, function, 0x08);
  u32 header_reg = pci_config_read32(bus, slot, function, 0x0C);

  device->bus = bus;
  device->slot = slot;
  device->function = function;
  device->vendor_id = (u16)(id & 0xFFFFu);
  device->device_id = (u16)(id >> 16);
  device->prog_if = (u8)((class_reg >> 8) & 0xFFu);
  device->subclass = (u8)((class_reg >> 16) & 0xFFu);
  device->class_code = (u8)((class_reg >> 24) & 0xFFu);
  device->header_type = (u8)((header_reg >> 16) & 0xFFu);
}

u8 pci_enumerate(pci_device_t *devices, u8 max_devices) {
  if (devices == 0 || max_devices == 0) {
    return 0;
  }

  u8 count = 0;

  for (u16 bus = 0; bus < 256 && count < max_devices; bus++) {
    for (u8 slot = 0; slot < 32 && count < max_devices; slot++) {
      u16 vendor = pci_config_read16((u8)bus, slot, 0, 0x00);
      if (vendor == 0xFFFFu) {
        continue;
      }

      pci_fill_device(&devices[count++], (u8)bus, slot, 0);

      u32 header_reg = pci_config_read32((u8)bus, slot, 0, 0x0C);
      u8 header_type = (u8)((header_reg >> 16) & 0xFFu);

      if ((header_type & 0x80u) == 0) {
        continue;
      }

      for (u8 function = 1; function < 8 && count < max_devices; function++) {
        vendor = pci_config_read16((u8)bus, slot, function, 0x00);
        if (vendor == 0xFFFFu) {
          continue;
        }
        pci_fill_device(&devices[count++], (u8)bus, slot, function);
      }
    }
  }

  return count;
}

const char *pci_class_name(u8 class_code, u8 subclass) {
  switch (class_code) {
  case 0x01:
    switch (subclass) {
    case 0x01:
      return "IDE storage";
    case 0x06:
      return "SATA storage";
    default:
      return "Mass storage";
    }
  case 0x02:
    return "Network controller";
  case 0x03:
    return "Display controller";
  case 0x04:
    return "Multimedia controller";
  case 0x06:
    switch (subclass) {
    case 0x00:
      return "Host bridge";
    case 0x01:
      return "ISA bridge";
    case 0x04:
      return "PCI bridge";
    default:
      return "Bridge device";
    }
  case 0x0C:
    switch (subclass) {
    case 0x03:
      return "USB controller";
    default:
      return "Serial bus controller";
    }
  default:
    return "Unknown device";
  }
}
