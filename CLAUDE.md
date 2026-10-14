# Working on xteink-x4-esphome

- `git pull` first — the owner works from more than one PC.
- Work on **`dev`**, push to `dev`. `main` is only for releases, and only when
  the owner asks ("merge to main and make a new release"): merge `dev` into
  `main`, tag `vX.Y.Z`, GitHub release with the CHANGELOG section as notes.
- Every user-visible change: an entry under *Unreleased* in `CHANGELOG.md`
  (Added / Changed / Fixed / Under the hood), README updated to match, and
  `esphome.project.version` in `esphome/xteink-x4.yaml` bumped on release.
- **Everything Home Assistant needs goes into `packages/xteink_x4.yaml`** —
  one file, no UI helpers, nothing in `configuration.yaml`. Entity IDs start
  with `xteink_x4_`.
- **ESPHome checks run on the build server** (CT 117 `esphome-builder` on
  pve_rptu_dev), never locally as long as pve_rptu_dev is reachable (owner,
  2026-10-14): `bash ../claude-setup/scripts/esphome-check.sh esphome/xteink-x4.yaml`
  (add `config` to skip the compile). It sends the working tree, uncommitted
  changes included, and uses `esphome/secrets.example.yaml` (dummy values);
  same ESPHome as Home Assistant's app. Only if pve_rptu_dev is unreachable,
  validate locally with a venv outside the Drive folder. The owner compiles
  and flashes through the ESPHome app in Home Assistant.
- Public repo: no secrets, IPs or personal entity names in committed files.
