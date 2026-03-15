#include "../test.h"

#include "../../src/rtc.h"

static unsigned char cmos_index;
static unsigned char cmos_data[256];

u8 test_inb(u16 port) {
  if (port == 0x71) {
    return cmos_data[cmos_index];
  }
  return 0;
}

void test_outb(u16 port, u8 value) {
  if (port == 0x70) {
    cmos_index = value;
  }
}

void test_outw(u16 port, u16 value) {
  (void)port;
  (void)value;
}

static void load_bcd_time(void) {
  memset(cmos_data, 0, sizeof(cmos_data));
  cmos_data[0x0A] = 0x00;
  cmos_data[0x0B] = 0x00;
  cmos_data[0x00] = 0x56;
  cmos_data[0x02] = 0x34;
  cmos_data[0x04] = 0x12;
  cmos_data[0x07] = 0x15;
  cmos_data[0x08] = 0x03;
  cmos_data[0x09] = 0x26;
}

TEST(rtc_read_time_decodes_bcd_values) {
  rtc_time_t now;
  load_bcd_time();

  ASSERT_TRUE(rtc_read_time(&now));
  ASSERT_EQ(now.second, 56);
  ASSERT_EQ(now.minute, 34);
  ASSERT_EQ(now.hour, 12);
  ASSERT_EQ(now.day, 15);
  ASSERT_EQ(now.month, 3);
  ASSERT_EQ(now.year, 2026);
  TEST_PASS_MSG();
}

TEST(rtc_read_time_handles_null_pointer) {
  load_bcd_time();
  ASSERT_FALSE(rtc_read_time(0));
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nRTC Tests:\n");
  RUN_TEST(rtc_read_time_decodes_bcd_values);
  RUN_TEST(rtc_read_time_handles_null_pointer);
  TEST_SUMMARY();
}
