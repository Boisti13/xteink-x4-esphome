# Changelog

All notable changes, newest first. Versions follow [semantic versioning](https://semver.org/); each release is tagged `vX.Y.Z` on `main`.

## Unreleased

## v0.1.0 — 2026-10-07

### Added
- **ESPHome config for the Xteink X4** (`esphome/xteink-x4.yaml`): ESP32-C3 with the 16 MB flash layout, Wi-Fi, encrypted API, OTA, Home Assistant time.
- **One portrait page**: weekday and date, clock, a message line, battery and Wi-Fi signal. It redraws when a Home Assistant value changes, on a short press of the power button, and every 5 minutes.
- **All seven buttons** (four front, up/down, power) and the **battery level** as Home Assistant entities.
- **Home Assistant package** `packages/xteink_x4.yaml`: `input_text.xteink_x4_message` and `sensor.xteink_x4_header`.
- README, `secrets.example.yaml`, MIT license.
