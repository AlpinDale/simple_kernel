#include "../test.h"

#include "../../src/pci.h"

static unsigned int last_config_address;

static u32 fake_pci_config_read(unsigned int address) {
  unsigned int bus = (address >> 16) & 0xFFu;
  unsigned int slot = (address >> 11) & 0x1Fu;
  unsigned int function = (address >> 8) & 0x7u;
  unsigned int offset = address & 0xFCu;

  if (bus == 0 && slot == 0 && function == 0) {
    if (offset == 0x00) {
      return 0x11111234u;
    }
    if (offset == 0x08) {
      return 0x06000000u;
    }
    if (offset == 0x0C) {
      return 0x00800000u;
    }
  }

  if (bus == 0 && slot == 0 && function == 1) {
    if (offset == 0x00) {
      return 0x813910ECu;
    }
    if (offset == 0x08) {
      return 0x02000000u;
    }
    if (offset == 0x0C) {
      return 0x00000000u;
    }
  }

  if (bus == 0 && slot == 1 && function == 0) {
    if (offset == 0x00) {
      return 0x29228086u;
    }
    if (offset == 0x08) {
      return 0x01018000u;
    }
    if (offset == 0x0C) {
      return 0x00000000u;
    }
  }

  return 0xFFFFFFFFu;
}

u8 test_inb(u16 port) {
  (void)port;
  return 0;
}

void test_outb(u16 port, u8 value) {
  (void)port;
  (void)value;
}

void test_outw(u16 port, u16 value) {
  (void)port;
  (void)value;
}

u32 test_inl(u16 port) {
  if (port == 0xCFC) {
    return fake_pci_config_read(last_config_address);
  }
  return 0xFFFFFFFFu;
}

void test_outl(u16 port, u32 value) {
  if (port == 0xCF8) {
    last_config_address = value;
  }
}

TEST(pci_config_address_encodes_fields) {
  u32 address = pci_config_address(2, 3, 4, 0x14);

  ASSERT_EQ(address, 0x80021C14u);
  TEST_PASS_MSG();
}

TEST(pci_enumerate_reads_present_devices) {
  pci_device_t devices[PCI_MAX_FUNCTIONS];
  u8 count = pci_enumerate(devices, PCI_MAX_FUNCTIONS);

  ASSERT_EQ(count, 3);

  ASSERT_EQ(devices[0].bus, 0);
  ASSERT_EQ(devices[0].slot, 0);
  ASSERT_EQ(devices[0].function, 0);
  ASSERT_EQ(devices[0].vendor_id, 0x1234);
  ASSERT_EQ(devices[0].device_id, 0x1111);
  ASSERT_EQ(devices[0].class_code, 0x06);
  ASSERT_EQ(devices[0].subclass, 0x00);

  ASSERT_EQ(devices[1].bus, 0);
  ASSERT_EQ(devices[1].slot, 0);
  ASSERT_EQ(devices[1].function, 1);
  ASSERT_EQ(devices[1].vendor_id, 0x10EC);
  ASSERT_EQ(devices[1].device_id, 0x8139);
  ASSERT_EQ(devices[1].class_code, 0x02);

  ASSERT_EQ(devices[2].slot, 1);
  ASSERT_EQ(devices[2].vendor_id, 0x8086);
  ASSERT_EQ(devices[2].device_id, 0x2922);
  ASSERT_EQ(devices[2].subclass, 0x01);
  TEST_PASS_MSG();
}

TEST(pci_class_name_maps_common_classes) {
  ASSERT_EQ(strcmp(pci_class_name(0x06, 0x00), "Host bridge"), 0);
  ASSERT_EQ(strcmp(pci_class_name(0x02, 0x00), "Network controller"), 0);
  ASSERT_EQ(strcmp(pci_class_name(0x01, 0x01), "IDE storage"), 0);
  ASSERT_EQ(strcmp(pci_class_name(0xFF, 0x00), "Unknown device"), 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nPCI Tests:\n");
  RUN_TEST(pci_config_address_encodes_fields);
  RUN_TEST(pci_enumerate_reads_present_devices);
  RUN_TEST(pci_class_name_maps_common_classes);
  TEST_SUMMARY();
}
