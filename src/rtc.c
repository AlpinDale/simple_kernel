#include "io.h"
#include "rtc.h"

#define CMOS_INDEX_PORT 0x70
#define CMOS_DATA_PORT 0x71

static u8 cmos_read(u8 reg) {
  outb(CMOS_INDEX_PORT, reg);
  return inb(CMOS_DATA_PORT);
}

static u8 rtc_updating(void) { return (cmos_read(0x0A) & 0x80) != 0; }

static u8 bcd_to_bin(u8 value) { return (u8)((value & 0x0F) + ((value / 16) * 10)); }

static void rtc_sample(rtc_time_t *time, u8 *status_b) {
  time->second = cmos_read(0x00);
  time->minute = cmos_read(0x02);
  time->hour = cmos_read(0x04);
  time->day = cmos_read(0x07);
  time->month = cmos_read(0x08);
  time->year = cmos_read(0x09);
  *status_b = cmos_read(0x0B);
}

int rtc_read_time(rtc_time_t *time) {
  if (time == 0) {
    return 0;
  }

  rtc_time_t first;
  rtc_time_t second;
  u8 status_b = 0;

  while (rtc_updating()) {
  }

  rtc_sample(&first, &status_b);

  do {
    while (rtc_updating()) {
    }
    rtc_sample(&second, &status_b);
  } while (first.second != second.second || first.minute != second.minute ||
           first.hour != second.hour || first.day != second.day ||
           first.month != second.month || first.year != second.year);

  *time = second;

  if ((status_b & 0x04) == 0) {
    time->second = bcd_to_bin(time->second);
    time->minute = bcd_to_bin(time->minute);
    time->hour = (u8)((time->hour & 0x80) | bcd_to_bin(time->hour & 0x7F));
    time->day = bcd_to_bin(time->day);
    time->month = bcd_to_bin(time->month);
    time->year = bcd_to_bin((u8)time->year);
  }

  if ((status_b & 0x02) == 0 && (time->hour & 0x80) != 0) {
    time->hour = (u8)(((time->hour & 0x7F) + 12) % 24);
  } else {
    time->hour &= 0x7F;
  }

  time->year = (u16)(2000 + time->year);
  return 1;
}
