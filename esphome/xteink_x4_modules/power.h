#pragma once

#include <ctime>
#include <Arduino.h>
#include <SPI.h>
#include "driver/gpio.h"
#include "esp_sleep.h"

// E-paper controller (SSD1677) pins, as wired on the X4
static const int XTEINK_EPD_CS = 21;
static const int XTEINK_EPD_DC = 4;
static const int XTEINK_EPD_RST = 5;
static const int XTEINK_EPD_BUSY = 6;

inline void xteink_epd_send(bool command, uint8_t value) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(XTEINK_EPD_DC, command ? LOW : HIGH);
  digitalWrite(XTEINK_EPD_CS, LOW);
  SPI.transfer(value);
  digitalWrite(XTEINK_EPD_CS, HIGH);
  SPI.endTransaction();
}

inline void xteink_epd_wait_idle() {
  uint32_t start = millis();
  while (digitalRead(XTEINK_EPD_BUSY) == HIGH && millis() - start < 3000)
    delay(1);
}

// The display driver leaves the panel's charge pumps and oscillator running
// after a refresh. Switch them off and put the SSD1677 into deep sleep; the
// picture stays. The driver resets the panel at the next boot, which wakes it.
inline void xteink_display_power_off() {
  xteink_epd_wait_idle();
  xteink_epd_send(true, 0x21);   // Display update control 1
  xteink_epd_send(false, 0x40);  //   normal mode
  xteink_epd_send(true, 0x22);   // Display update control 2
  xteink_epd_send(false, 0x03);  //   analog off, clock off
  xteink_epd_send(true, 0x20);   // Master activation
  xteink_epd_wait_idle();
  xteink_epd_send(true, 0x10);   // Deep sleep
  xteink_epd_send(false, 0x01);  //   mode 1, RAM kept
  // Keep chip select and reset high while the ESP32 sleeps
  gpio_hold_en((gpio_num_t) XTEINK_EPD_CS);
  gpio_hold_en((gpio_num_t) XTEINK_EPD_RST);
}

// Release the pins held through deep sleep, before the display driver
// starts and pulses reset.
inline void xteink_release_display_pins() {
  gpio_hold_dis((gpio_num_t) XTEINK_EPD_CS);
  gpio_hold_dis((gpio_num_t) XTEINK_EPD_RST);
}

// True after flashing, reset or power-on; false after a wake from deep sleep
// (timer or power button).
inline bool xteink_cold_boot() {
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED;
}

// The X4 cuts its own power when the ESP32 goes into deep sleep unless the
// power latch on GPIO13 is held. Hold it, don't drive it.
inline void xteink_hold_power_latch() {
  gpio_hold_en(GPIO_NUM_13);
  gpio_deep_sleep_hold_en();
}

// Is this hour inside the quiet window [quiet_start, quiet_end)?
// The window may wrap past midnight (e.g. 23 → 6). Equal hours = no window.
inline bool xteink_quiet_hour(int hour, int quiet_start, int quiet_end) {
  if (quiet_start == quiet_end)
    return false;
  if (quiet_start < quiet_end)
    return hour >= quiet_start && hour < quiet_end;
  return hour >= quiet_start || hour < quiet_end;
}

// Milliseconds until the next full hour (+5 s) outside the quiet window, so
// the page is redrawn on the hour and not at night. Falls back to one hour
// without a valid clock.
inline uint32_t xteink_ms_to_next_wake(const esphome::ESPTime &now, int quiet_start, int quiet_end) {
  if (!now.is_valid())
    return 3600UL * 1000UL;
  uint32_t secs = 3600 - (now.minute * 60 + now.second) + 5;
  int hour = (now.hour + 1) % 24;
  // Woke a little early (the sleep timer runs ~1 % fast, so a night's sleep
  // ends a few minutes early): that wake counts for the coming hour
  if (secs < 600) {
    secs += 3600;
    hour = (hour + 1) % 24;
  }
  for (int i = 0; i < 24 && xteink_quiet_hour(hour, quiet_start, quiet_end); i++) {
    secs += 3600;
    hour = (hour + 1) % 24;
  }
  return secs * 1000UL;
}
