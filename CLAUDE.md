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
- Validate locally (`esphome config esphome/xteink-x4.yaml`); the owner
  compiles and flashes through the ESPHome app in Home Assistant.
- Public repo: no secrets, IPs or personal entity names in committed files.
