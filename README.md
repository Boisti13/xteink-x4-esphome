# Xteink X4 ESPHome

[![Latest release](https://img.shields.io/github/v/release/Boisti13/xteink-x4-esphome?label=release)](https://github.com/Boisti13/xteink-x4-esphome/releases/latest)
[![ESPHome](https://img.shields.io/badge/ESPHome-ESP32--C3-000000?logo=esphome)](https://esphome.io/)
[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-package-41BDF5?logo=homeassistant&logoColor=white)](packages/xteink_x4.yaml)
[![License: MIT](https://img.shields.io/github/license/Boisti13/xteink-x4-esphome)](LICENSE)

Turn the **Xteink X4**, a pocket e-reader (ESP32-C3, 4.26" 800×480
e-paper), into a small display for [Home Assistant](https://www.home-assistant.io/).
It runs [ESPHome](https://esphome.io/) and shows what Home Assistant sends it:
the date, the time and a message line. Its buttons and battery level show up
as entities in Home Assistant. Everything Home Assistant needs for it is in
**one package file**.

> **Status: early.** v0.1 is the starting point: one page, always on. Deep
> sleep, more pages and button navigation are next — see [Roadmap](#roadmap).

## Highlights

- **One page, portrait**: weekday and date, clock, a message line, battery and Wi-Fi at the bottom
- **Driven by Home Assistant**: set `input_text.xteink_x4_message` from the UI, a script or an automation, and the screen redraws at once
- **All seven buttons as entities**: four front buttons, up/down on the side, power — use them in automations (a short press on power redraws the page)
- **Battery level** in percent, plus Wi-Fi signal
- **One Home Assistant package** (`packages/xteink_x4.yaml`): every helper and template sensor the display uses, nothing to click together in the UI
- **OTA updates** after the first USB flash; 16 MB flash with two app slots

## How it works

```
Home Assistant                               Xteink X4 (ESPHome)
packages/xteink_x4.yaml                      esphome/xteink-x4.yaml
  input_text.xteink_x4_message  ──API──▶      text_sensor ─┐
  sensor.xteink_x4_header       ──API──▶      text_sensor ─┼─▶ display redraw
  time                          ──API──▶      time        ─┘
  binary_sensor.xteink_x4_*     ◀──API──      buttons
  sensor.xteink_x4_battery      ◀──API──      battery ADC
```

The page redraws when a Home Assistant value changes, when you press the
power button, and every 5 minutes (`refresh_interval` in
`esphome/xteink-x4.yaml`). It uses the panel's half refresh (~1.7 s), which
leaves no ghosting.

## Install

You need Home Assistant with the **ESPHome Device Builder** app (add-on), an
Xteink X4 and a USB-C cable.

**1. Home Assistant package.** Turn on packages once in `configuration.yaml`,
if you haven't already:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

Copy [`packages/xteink_x4.yaml`](packages/xteink_x4.yaml) to
`/config/packages/`, then run *Developer tools → YAML → Check configuration*
and reload *All YAML configuration*.

**2. ESPHome config.** Copy everything in [`esphome/`](esphome) into
`/config/esphome/`:

| File | What it is |
|---|---|
| `xteink-x4.yaml` | the device config |
| `xteink_x4_partitions.csv` | flash layout for the X4's 16 MB |
| `xteink_x4_modules/` | display, fonts, sensors, buttons |
| `secrets.example.yaml` | copy its keys into your ESPHome `secrets.yaml` and fill them in |

**3. First flash over USB.** Back up the stock firmware first if you want to go
back later (see [Going back](#going-back-to-the-reader-firmware)). In the
ESPHome Device Builder: *xteink-x4 → Install → Plug into this computer*, or
build *Manual download* and flash it with [ESPHome Web](https://web.esphome.io/).
After that, updates go over Wi-Fi.

**4. Adopt in Home Assistant.** The device appears under *Settings → Devices
& services* as *Xteink X4*. Allow it to **perform Home Assistant actions**
in its ESPHome options.

## Home Assistant entities

**From the package** (the display reads these):

| Entity | What it does |
|---|---|
| `input_text.xteink_x4_message` | the message line on the screen, up to 255 characters |
| `sensor.xteink_x4_header` | the header, e.g. *Wed, 7 Oct* |

**From the device**:

| Entity | State |
|---|---|
| `sensor.xteink_x4_battery` | battery in % |
| `sensor.xteink_x4_wi_fi_signal` | Wi-Fi signal in dBm |
| `binary_sensor.xteink_x4_button_1` … `_button_4` | front buttons, on while pressed |
| `binary_sensor.xteink_x4_button_up` / `_button_down` | side buttons |
| `binary_sensor.xteink_x4_power_button` | power button |

Example — put the next calendar appointment on the screen:

```yaml
automation:
  - alias: "Xteink X4: next appointment"
    triggers:
      - trigger: state
        entity_id: calendar.family
    actions:
      - action: input_text.set_value
        target:
          entity_id: input_text.xteink_x4_message
        data:
          value: >-
            {{ state_attr('calendar.family', 'message') or 'Nothing planned' }}
```

## Hardware

| | |
|---|---|
| SoC | ESP32-C3, 400 KB SRAM, no PSRAM, 16 MB flash |
| Display | 4.26" 800×480 GDEQ0426T82, SSD1677 — SPI: SCLK 8, MOSI 10, CS 21, DC 4, RST 5, BUSY 6 |
| Buttons | power GPIO3 (active low); front buttons on a resistor ladder at GPIO1, side buttons at GPIO2 |
| Battery | 650 mAh LiPo, voltage on GPIO0 through a 1:2 divider; charging detect GPIO20 |
| Other | microSD on the same SPI bus (CS 12, MISO 7); GPIO13 holds the power on |

The display, button and battery drivers come from
[ngxson/esphome-component-xteink](https://github.com/ngxson/esphome-component-xteink),
loaded by ESPHome at build time and pinned to a fixed commit. Pin details
from Adafruit's [CircuitPython on the Xteink X4](https://learn.adafruit.com/circuitpython-on-the-xteink-x4-ereader/pinouts).

## Going back to the reader firmware

ESPHome replaces the stock (or [CrossPoint](https://github.com/crosspoint-reader))
firmware. To keep a way back, read the whole flash before the first ESPHome
flash:

```bash
esptool.py --chip esp32c3 --baud 921600 read_flash 0 0x1000000 xteink-x4-stock.bin
```

and write it back later with `esptool.py --chip esp32c3 write_flash 0 xteink-x4-stock.bin`.

## Roadmap

- **Deep sleep** between updates, woken by a timer or the power button — weeks on one charge instead of a day
- **More pages** (weather, calendar, rooms), switched with the front buttons
- Charging state (GPIO20) and battery voltage as their own sensors
- Partial refresh for the clock

## Repository layout

```
esphome/
  xteink-x4.yaml              # device config
  xteink_x4_partitions.csv    # 16 MB flash layout
  secrets.example.yaml
  xteink_x4_modules/
    display.yaml              # page layout (lambda)
    fonts.yaml
    sensors.yaml              # battery, Wi-Fi
    text_sensors.yaml         # values from Home Assistant
    binary_sensors.yaml       # seven buttons
packages/
  xteink_x4.yaml              # everything on the Home Assistant side
```

## Development

`main` holds released versions, `dev` is where work happens. Changes go to
`dev`, get tested on a real X4, and are merged to `main` with a release
(`vX.Y.Z` tag, notes from [CHANGELOG.md](CHANGELOG.md)). Validate a config
without a device:

```bash
esphome config esphome/xteink-x4.yaml
```

## License

[MIT](LICENSE). The ESPHome components it loads are a separate project with
their own terms.
