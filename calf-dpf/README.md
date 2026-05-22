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
  - one UI `.cpp` (DGL / NanoVG; codegen-emitted from `gui/gui/*.xml`)
  - a `Makefile`
- `dsp/` — Calf DSP modules, decoupled from GTK + LV2 SDK.
- `ui-lib/` — shared DGL widgets + `CalfLayout.hpp` packing solver.
- `tools/` — `xml2ui.py`, the Calf-XML → DPF-UI codegen.
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
- [x] Phase 1: DSP decoupling
- [x] Phase 2: parameter / MIDI bridge
- [x] Phase 3: state bridge
- [x] Phase 4: shared UI lib *(simple widgets + layout engine; CalfLineGraph still a placeholder rect — real frequency-response / FFT rendering is the v1.1 visual upgrade)*
- [x] Phase 5: per-plugin migration *(51 plugins; see `make list`)*
- [ ] Phase 6: standalone story
- [ ] Phase 7: validation + release *(smoke tests landed — `make smoke` and `make dsp-smoke`; bit-exact null-test vs upstream Calf LV2 still TODO)*

## Build everything

```sh
git submodule update --init --recursive
make -C calf-dpf            # builds all 51 plugin subdirs
```

Per-plugin builds also work: `make -C calf-dpf/plugins/Compressor`.

## Smoke tests

```sh
make -C calf-dpf smoke      # launch each JACK standalone briefly,
                            # report load-time crashes
make -C calf-dpf dsp-smoke  # push audio through every Calf module
                            # directly, report PASS / SILENT / FAIL
```

Today: 51/51 standalones launch, 48 emit audio with default params,
3 are SILENT until configure-vars are populated (Monosynth modmatrix,
FluidSynth SF2 path, Wavetable data).

## XML→C++ codegen

`tools/xml2ui.py` reads a Calf GTK layout XML (`../gui/gui/<name>.xml`)
and emits one DPF UI subclass. Re-run when an XML file changes; the
generated `.cpp` is committed so the build stays hermetic.

```sh
python3 calf-dpf/tools/xml2ui.py gui/gui/compressor.xml \
    --class-name CompressorUI \
    --metadata-class compressor_metadata \
    -o calf-dpf/plugins/Compressor/CompressorUI.cpp
```

Coverage today: `vbox` / `hbox` / `table` / `frame` / `align` / `scrolled`
/ `notebook` (rendered as a plain vbox until a real notebook lands) /
`<if>`. Widgets: knob, vumeter, led, toggle, combo, label, value.
Line-graph / phase-graph / curve / pattern / tuner / keyboard / scale
render as a CalfLineGraph stub (empty rect) so layouts compile; real
widgets land later in Phase 4.
