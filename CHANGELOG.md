# Changelog

All notable changes, newest first. Versions follow [semantic versioning](https://semver.org/); each release is tagged `vX.Y.Z` on `main`.

## Unreleased

### Under the hood
- Config checks run on the ESPHome build server (`claude-setup/scripts/esphome-check.sh`, see `CLAUDE.md`); `secrets.example.yaml` has a valid dummy API key so `esphome config` works with it.

## v0.3.0 — 2026-10-13

### Added
- **Button only** wake mode: with `input_boolean.xteink_x4_button_only` (also a *Button only* switch on the X4's device page) the X4 skips the hourly wakes and sleeps until the power button is pressed — like switched off, but the button still brings a fresh page. The footer then reads *20:46 · press power*. Switching back to hourly takes effect after the next button press. Without an answer from Home Assistant the X4 keeps waking hourly.
- **Outside temperature** in the header, at the right with a thermometer icon (*16.5°*), in both layouts — to know how warmly to dress the baby. Comes from `sensor.xteink_x4_outdoor` in the package, which reads `sensor.virtual_outdoor_temperature`.
- **Landscape layout**: the page can also be shown in landscape, with sleep and diapers on the left and medicine and supplements on the right (entries at 23 px under 28 px headings, so names fit the narrower column; up to 3 entries each). A **Landscape** switch on the X4's device page, mirrored by `input_boolean.xteink_x4_landscape`, switches between landscape and portrait without reflashing — at once while the X4 is awake, otherwise at the next wake. Set the exact rotations with `landscape_rotation` / `portrait_rotation` in `esphome/xteink-x4.yaml`.
- **Battery voltage** as its own sensor (`sensor.xteink_x4_battery_voltage`, V) — steadier than the percentage for following the battery over days.

### Fixed
- **Button 4 and Button down stuck "on"** (since the battery voltage sensor): ESPHome's ADC sensor claimed the ADC, so the button driver read 0 on GPIO1/GPIO2, which it takes for those two buttons. The battery is now read with Arduino's `analogReadMilliVolts()`, like the buttons.

### Under the hood
- The deep sleep component no longer has a default `sleep_duration` (the 60 min fallback was never used); `go_to_sleep` passes one, or none in button-only mode so only the power button wakes it.
- The battery is read directly from GPIO0 (×2 divider, 16 samples) instead of the `xteink_battery` driver; the percentage comes from the voltage along a LiPo curve that matches the old one, so the history carries on.
- Tried and undone: switching the e-paper panel off (and holding its chip select and reset high) before deep sleep. Instead of saving power, the battery drained faster — about 18 % a day instead of 12.5 %, and about three times as much overnight.

## v0.2.0 — 2026-10-07

### Added
- **Sprout Track dashboard**: the page now shows how long the baby has been awake or asleep (and since when), the last diaper (how long ago, time, wet/dirty), wet and dirty diapers today, and every medicine and supplement dose today with time, name and amount.
- **Deep sleep**: the X4 wakes on every full hour, draws the page once and sleeps again — about a week on one charge. The power button wakes it at once.
- **Quiet hours**: no wakes from 23:00 to 6:00 — the last page of the day is drawn at 22:00, the next at 6:00. Set with `quiet_start` / `quiet_end` in `esphome/xteink-x4.yaml`.
- **Keep awake** switch (`input_boolean.xteink_x4_keep_awake`) for OTA updates; switching it off sends the device straight back to sleep. The same switch also sits on the X4's device page (*Keep awake*, under Configuration) while it's awake.
- **Battery and charging in the footer**: *Battery 87 %*, *charging* while USB is plugged in (GPIO20, also as `binary_sensor.xteink_x4_usb_power`), *plugged in, full* at 100 %.
- **Low battery warning**: at 15 % or less, without USB, the footer turns into a black *Battery 14 % – please charge* bar. Set the level with `battery_warning` in `esphome/xteink-x4.yaml`.
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
- A wake up to 10 minutes before the hour counts for that hour: the sleep timer runs about 1 % fast, so the night's sleep ended around 5:56 and was followed by an extra wake at 6:00.
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
