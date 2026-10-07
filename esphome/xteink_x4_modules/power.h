#pragma once

#include <ctime>
#include "driver/gpio.h"

// The X4 cuts its own power when the ESP32 goes into deep sleep unless the
// power latch on GPIO13 is held. Hold it, don't drive it.
inline void xteink_hold_power_latch() {
  gpio_hold_en(GPIO_NUM_13);
  gpio_deep_sleep_hold_en();
}

// Milliseconds until the next full hour (+5 s), so the page is redrawn on
// the hour. Falls back to one hour without a valid clock.
inline uint32_t xteink_ms_to_next_hour(const esphome::ESPTime &now) {
  if (!now.is_valid())
    return 3600UL * 1000UL;
  uint32_t secs = 3600 - (now.minute * 60 + now.second) + 5;
  return secs * 1000UL;
}
