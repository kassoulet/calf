# Calf — DPF port (work in progress)

This directory contains the in-progress port of Calf Studio Gear to the
DISTRHO Plugin Framework (DPF). It currently lives alongside the legacy
autotools/CMake tree on the `calf-dpf` branch; it does not yet replace
anything.

See `../roadmap.md` for the full plan.

## Layout

- `plugins/` — one directory per plugin. Each contains:
  - `DistrhoPluginInfo.h`
  - one DSP `.cpp` (thin adapter forwarding to `dsp/`)
  - one UI `.cpp` (DGL / NanoVG)
  - a `Makefile`
- `dsp/` — Calf DSP modules, decoupled from GTK + LV2 SDK.
- `ui-lib/` — shared DGL widgets (knobs, VU meters, line graphs).
- `../dpf/` — DISTRHO/DPF git submodule.

## Build (Hello / sanity check)

```sh
git submodule update --init --recursive
make -C calf-dpf/plugins/Hello
```

Artifacts land under `calf-dpf/bin/` in the formats listed by `TARGETS`
(LV2, VST3, CLAP, JACK standalone).

## Status

- [x] Phase 0: scaffolding + hello plugin
- [ ] Phase 1: DSP decoupling
- [ ] Phase 2: parameter / MIDI bridge
- [ ] Phase 3: state bridge
- [ ] Phase 4: shared UI lib
- [ ] Phase 5: per-plugin migration
- [ ] Phase 6: standalone story
- [ ] Phase 7: validation + release
