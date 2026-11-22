#!/usr/bin/env bash
# Render the dashboard without the device: builds esphome/screenshot-demo.yaml
# for the PC (ESPHome host platform + SDL) with the device's own page code,
# runs it in a virtual X display and saves docs/preview-landscape.png and
# docs/preview.png (portrait), inverted so they look like the e-paper.
#
# Linux only. Needs ESPHome, libsdl2-dev, xvfb, imagemagick.
#   bash scripts/screenshots.sh
set -euo pipefail
cd "$(dirname "$0")/.."

# The page lambda from the device config, on an SDL display instead of the
# panel (minus the panel's refresh-mode call, which SDL doesn't have)
mkdir -p esphome/.esphome
{
  printf 'display:\n  - platform: sdl\n    id: epaper_display\n    update_interval: never\n'
  printf '    auto_clear_enabled: true\n    dimensions:\n      width: 800\n      height: 480\n'
  sed -n '/^    lambda: |-$/,$p' esphome/xteink_x4_modules/display.yaml | grep -v 'set_refresh_mode'
} > esphome/.esphome/demo-display.yaml

esphome compile esphome/screenshot-demo.yaml
bin=$(find esphome/.esphome/build/xteink-x4-demo -type f -name program -perm -u+x | head -1)

Xvfb :96 -screen 0 800x480x24 >/dev/null 2>&1 &
xvfb=$!
trap 'kill $app $xvfb 2>/dev/null || true' EXIT
sleep 1
TZ=Europe/Berlin DISPLAY=:96 "$bin" >/dev/null 2>&1 &
app=$!

# Landscape at boot, portrait from 3 s on; shoot mid-way
sleep 1.5
DISPLAY=:96 import -window root -negate -colorspace Gray -depth 8 docs/preview-landscape.png
sleep 3
DISPLAY=:96 import -window root -negate -colorspace Gray -depth 8 -rotate 90 docs/preview.png
echo "saved docs/preview-landscape.png, docs/preview.png"
