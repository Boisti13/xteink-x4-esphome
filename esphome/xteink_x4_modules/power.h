#pragma once

#include <ctime>
#include <Arduino.h>
#include <SPI.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "nvs.h"

// "Power off" mode cuts the battery, so the next start is a power-on like
// after flashing. A flag in flash tells the two apart; set right before
// switching off, cleared at the next start.
inline void xteink_set_switched_off(bool on) {
  nvs_handle_t h;
  if (nvs_open("xteink", NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_set_u8(h, "off", on ? 1 : 0);
  nvs_commit(h);
  nvs_close(h);
}

// Read once per start: was the X4 switched off by "Power off" mode?
// Clears the flag, and releases GPIO13 in case it is still held low (the
// battery wasn't cut because USB powered the X4).
inline bool xteink_was_switched_off() {
  static int cached = -1;
  if (cached < 0) {
    uint8_t off = 0;
    nvs_handle_t h;
    if (nvs_open("xteink", NVS_READONLY, &h) == ESP_OK) {
      nvs_get_u8(h, "off", &off);
      nvs_close(h);
    }
    cached = off ? 1 : 0;
    if (off) {
      xteink_set_switched_off(false);
      gpio_hold_dis(GPIO_NUM_13);
      gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
    }
  }
  return cached == 1;
}

// True after flashing, reset or plugging in; false after a wake from deep
// sleep (timer or power button) and after the power button switched the X4
// back on in "Power off" mode.
inline bool xteink_cold_boot() {
  if (xteink_was_switched_off())
    return false;
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED;
}

// The X4 cuts its own power when the ESP32 goes into deep sleep unless the
// power latch on GPIO13 is held. Hold it, don't drive it.
inline void xteink_hold_power_latch() {
  gpio_hold_en(GPIO_NUM_13);
  gpio_deep_sleep_hold_en();
}

// "Power off" mode: drive GPIO13 low, which disconnects the battery (as
// CrossPoint does), so the X4 draws nothing until the power button switches
// it back on. On USB power the board stays up; the deep sleep that follows
// then waits for the power button instead.
inline void xteink_power_off() {
  xteink_set_switched_off(true);
  gpio_hold_dis(GPIO_NUM_13);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_13, 0);
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

// microSD card on the display's SPI bus (SCLK 8, MOSI 10, MISO 7), chip
// select GPIO12. The card is powered whenever the X4 is, deep sleep
// included, but nothing here ever talks to it, and a card that was never
// initialised idles at a higher current than one that was. Initialise it in
// SPI mode (CMD0, CMD8, ACMD41) so it drops into its own low-power idle,
// then deselect it.
static const int XTEINK_SD_CS = 12;
static const int XTEINK_EPD_CS = 21;

// Send one SD command and return its R1 answer (0xFF = no answer)
inline uint8_t xteink_sd_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
  SPI.transfer(0xFF);
  SPI.transfer(0x40 | cmd);
  SPI.transfer(arg >> 24);
  SPI.transfer(arg >> 16);
  SPI.transfer(arg >> 8);
  SPI.transfer(arg);
  SPI.transfer(crc);
  uint8_t r1 = 0xFF;
  for (int i = 0; i < 10 && (r1 & 0x80); i++)
    r1 = SPI.transfer(0xFF);
  return r1;
}

// Returns what happened, for the "SD card" sensor
inline const char *xteink_sd_card_idle() {
  digitalWrite(XTEINK_EPD_CS, HIGH);  // keep the panel off the bus
  pinMode(XTEINK_SD_CS, OUTPUT);
  digitalWrite(XTEINK_SD_CS, HIGH);
  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  // At least 74 clocks with the card deselected
  for (int i = 0; i < 10; i++)
    SPI.transfer(0xFF);
  digitalWrite(XTEINK_SD_CS, LOW);
  const char *result;
  if (xteink_sd_command(0, 0, 0x95) != 0x01) {
    result = "no card";
  } else {
    // CMD8: SD v2 cards answer 0x01 plus four bytes; old ones 0x05
    uint32_t hcs = 0;
    if (xteink_sd_command(8, 0x1AA, 0x87) == 0x01) {
      for (int i = 0; i < 4; i++)
        SPI.transfer(0xFF);
      hcs = 1UL << 30;
    }
    uint8_t r1;
    uint32_t start = millis();
    do {
      xteink_sd_command(55, 0, 0x01);
      r1 = xteink_sd_command(41, hcs, 0x01);
    } while (r1 != 0x00 && millis() - start < 1000);
    result = r1 == 0x00 ? "idle" : "no answer";
  }
  digitalWrite(XTEINK_SD_CS, HIGH);
  SPI.transfer(0xFF);  // the card lets go of MISO after one more byte
  SPI.endTransaction();
  return result;
}
