#pragma once

#include <ctime>
#include "driver/gpio.h"
#include "esp_sleep.h"

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
  // Woke a little early (the sleep timer drifts): skip to the hour after
  if (secs < 120) {
    secs += 3600;
    hour = (hour + 1) % 24;
  }
  for (int i = 0; i < 24 && xteink_quiet_hour(hour, quiet_start, quiet_end); i++) {
    secs += 3600;
    hour = (hour + 1) % 24;
  }
  return secs * 1000UL;
}
