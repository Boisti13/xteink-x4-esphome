# Changelog

All notable changes, newest first. Versions follow [semantic versioning](https://semver.org/); each release is tagged `vX.Y.Z` on `main`.

## Unreleased

### Added
- **Sprout Track dashboard**: the page now shows how long the baby has been awake or asleep (and since when), the last diaper (how long ago, time, wet/dirty), wet and dirty diapers today, and every medicine and supplement dose today with time, name and amount.
- **Deep sleep**: the X4 wakes on every full hour, draws the page once and sleeps again — about a week on one charge. The power button wakes it at once.
- **Quiet hours**: no wakes from 23:00 to 6:00 — the last page of the day is drawn at 22:00, the next at 6:00. Set with `quiet_start` / `quiet_end` in `esphome/xteink-x4.yaml`.
- **Keep awake** switch (`input_boolean.xteink_x4_keep_awake`) for OTA updates; switching it off sends the device straight back to sleep.
- **Wake time sensors** *Wake: connected after* and *Wake: total* (seconds), sent right before each sleep — to see where the battery goes.
- Material Design icons on the page; fonts with umlauts and typographic punctuation.
- README: install steps for the Sprout Track secrets, entity table, battery life, a preview of the page.

### Changed
- The Home Assistant package polls the Sprout Track API (`/status` and the medicine and supplement lists) and builds the display values; the API key and URLs come from `secrets.yaml`.
- The page is no longer redrawn on a timer or on every change, only once per wake and on the power button.
- If Home Assistant can't be reached on a wake, the old picture stays instead of a page of dashes.
- **After flashing or a reset** the X4 shows a *Waiting for Home Assistant* screen and stays awake for up to 10 minutes, so there is time to add it to Home Assistant. Before, it gave up after 30 s and slept for an hour without drawing anything.

### Removed
- The message line (`input_text.xteink_x4_message`).

### Under the hood
- `xteink_x4_modules/power.h` holds the power latch (GPIO13) through deep sleep and works out the time to the next wake, skipping the quiet hours.
- **Fixed IP address** (`xteink_x4_ip`, `_gateway`, `_subnet` in `secrets.yaml`): no DHCP on every wake. Reserve the address on the router as well.
- Shorter wakes: the fixed 1 s pause before drawing and 2 s pause after it are gone (the refresh itself blocks until the panel is done).
- `safe_mode: boot_is_good_after: 10s`, so short wakes don't end in safe mode.
- Images use the `platform: file` syntax; the ignored `platformio_options` are gone.

## v0.1.0 — 2026-10-07

### Added
- **ESPHome config for the Xteink X4** (`esphome/xteink-x4.yaml`): ESP32-C3 with the 16 MB flash layout, Wi-Fi, encrypted API, OTA, Home Assistant time.
- **One portrait page**: weekday and date, clock, a message line, battery and Wi-Fi signal. It redraws when a Home Assistant value changes, on a short press of the power button, and every 5 minutes.
- **All seven buttons** (four front, up/down, power) and the **battery level** as Home Assistant entities.
- **Home Assistant package** `packages/xteink_x4.yaml`: `input_text.xteink_x4_message` and `sensor.xteink_x4_header`.
- README, `secrets.example.yaml`, MIT license.
