# Calf → DPF (DISTRHO Plugin Framework) Port Roadmap

Status: **implementation in progress** — Phases 0–5 + Phase 4d landed on the
`calf-dpf` branch; **51 plugins** build LV2 + VST3 + CLAP + JACK standalone.
Phases 6 (standalone story) and 7 (validation/release) are the remaining
work. Target: build the Calf plugin suite as DPF plugins so that LV2, VST3,
CLAP, and JACK-standalone binaries fall out of a single source tree. (VST2
dropped — Steinberg sunset the SDK and DPF only emits VST2 under explicit
opt-in.)

## Current state (one-line per phase)

| Phase | Status | Notes |
|-------|--------|-------|
| 0 — scaffolding              | ✅ done  | `calf-dpf/` skeleton, Hello plugin, DPF submodule |
| 1 — DSP decoupling           | ✅ done  | `libcalfdsp.a` builds with no GTK / LV2 SDK |
| 2 — parameter/MIDI bridge    | ✅ done  | `CalfDpfBridge.hpp` template, MIDI dispatch under `MAX_SAMPLE_RUN` slicing |
| 3 — state / configure-vars   | ✅ done  | `configure()` ↔ `setState`/`getState` |
| 4 — shared UI lib            | ✅ done  | knob / vu / led / toggle / combo / label / value / line-graph widgets, packing solver, XML→C++ codegen, real `CalfLineGraph` via UI-side DSP shadow |
| 5 — per-plugin migration     | ✅ done  | 51 plugins; every audio_module in `modulelist.h` covered |
| 6 — standalone story         | ✅ done  | `calfjackhost` dropped; `calf-dpf/MIGRATION.md` points users at Carla |
| 7 — validation / release     | 🔶 partial | smoke tests landed (`make smoke`, `make dsp-smoke`, `make lv2_render`); per-plugin Lilv render in place; bit-exact null-test vs upstream Calf still TODO |

See `calf-dpf/README.md` for the per-target build commands.

**Branding:** plugins ship under `DISTRHO_PLUGIN_BRAND = "CalfDPFClaude"`
so DAW vendor menus distinguish this fork from upstream Calf at a
glance. URIs remain `calf-studio-gear.org/plugins/...` so existing
sessions still resolve.

**Build isolation:** each plugin uses its own `DPF_BUILD_DIR =
../../build/<Name>` rather than a shared tree. DPF compiles
`DistrhoPluginMain_*.cpp` against the current plugin's
`DistrhoPluginInfo.h` and caches the resulting object; a shared build
dir would bake the first plugin's URI / unique-id into every other
plugin's `.so`. This was discovered during Phase 7b setup — see commit
log.

---

## 1. Project at a glance

- ~36 kLOC of `.cpp` under `src/`, ~19 kLOC of headers under `src/calf/`.
- **~55 audio modules** declared in `src/calf/modulelist.h` (44 stable + a few
  experimental: monosynth, organ, fluidsynth, wavetable, all the dynamics /
  EQ / delay / mod / filter / distortion / tools / analyzer family).
- Single static lib `libcalf` aggregates DSP + LV2 wrap; a separate
  `libcalflv2gui` carries the GTK2 UI; `calfjackhost` is the stand-alone host.
- `calfmakerdf` is an in-tree codegen that walks the metadata registry and
  emits the LV2 `.ttl` manifests at install time.

### 1.1 Dependencies today

| Dependency      | Where used                                  | Action for DPF |
|-----------------|---------------------------------------------|----------------|
| LV2 SDK         | `src/lv2wrap.{h,cpp}`, `src/lv2gui.cpp`     | Replaced by DPF (DPF generates LV2 itself). |
| GTK+ 2.12       | All of `src/ctl_*.cpp`, `src/gui*.cpp`      | Removed; UI re-implemented on DPF's UI layer (OpenGL via DGL). |
| Cairo           | Custom controls, graph rendering            | Re-implement on top of DGL / NanoVG (DPF ships NanoVG). |
| FluidSynth      | `src/fluidsynth.cpp` (SF2 player)           | Keep — link the SF2 plugin only. |
| Expat           | GUI XML + preset XML parsing                | Keep for preset load/save; GUI XML format becomes obsolete. |
| JACK            | `calfjackhost`, `jack_client.cpp`           | Replace with DPF JACK standalone target. |
| LASH            | `host_session.cpp`                          | Drop (out of scope, Linux-only legacy). |
| pffft           | `src/pffft.c` (analyzer)                    | Keep, vendor as-is. |

---

## 2. Architecture today

```
        +-------------------------------------------------+
        |    metadata.h / metadata.cpp                    |  (port / param tables, GTK-clean)
        |    plugin_metadata<T> CRTP base                 |
        +-------------------------------------------------+
                          |
                          v
        +-------------------------------------------------+
        |    audio_module<Metadata>  (giface.h)           |
        |    DSP modules: modules_comp/_delay/_dist/...   |  (GTK-clean, RT-safe)
        |    monosynth / organ / wavetable / fluidsynth   |
        +-------------------------------------------------+
              |                              |
              v                              v
        +-----------+                +--------------------+
        | lv2wrap   |                |  jackhost / GTK    |
        | (LV2 host |                |  GUI (ctl_*, gui*) |
        |  glue)    |                |                    |
        +-----------+                +--------------------+
```

### 2.1 Good news (porting-friendly)

- **DSP / metadata layer is already host-agnostic.** `giface.h` and
  `metadata.h` import zero GTK; `audio_module_iface` is a clean virtual
  interface (`process`, `note_on`, `set_sample_rate`, `params_changed`, …).
  These map 1:1 onto DPF's `Plugin` virtuals.
- **Parameter metadata is centralized.** `parameter_properties` captures
  range, scale (linear / log / dB / quad), unit (Hz, dB, ms, BPM, …),
  enum choices, and widget hints. All of this maps onto DPF's
  `Parameter` + `ParameterEnumerationValues` + `ParameterHints`.
- **GUI XML files (52 of them under `gui/gui/*.xml`)** are pure declarative
  layout. They can be parsed once at build/port time and re-emitted as
  DPF UI code (or kept as data and interpreted by a shared DPF UI runtime).

### 2.2 Bad news

- **GTK2 is deeply baked into the UI.** Every custom control
  (`ctl_knob`, `ctl_vumeter`, `ctl_linegraph`, `ctl_phasegraph`,
  `ctl_curve`, `ctl_pattern`, …) is a `GtkDrawingArea` subclass with
  Cairo paint code. 13 custom widgets, ~9 kLOC of GTK glue.
- **The host application `calfjackhost`** (rack with patching, session
  management, preset browser, modulation matrix UI) is GTK2 + JACK. DPF
  doesn't ship a rack host — only individual standalones.
- **DSSI-style `configure(key, value)` state** is used for state that
  doesn't fit in float params (organ presets, wavetable data, modmatrix
  rows). DPF supports state via `setState`/`stateChanged`, so this is
  doable but every plugin needs an audit.
- **Cross-plugin shared static lib.** Today `libcalf.so` is loaded once
  and serves all 55 modules. DPF produces one binary per plugin (or one
  per format), so a thin shared static / object-archive structure has
  to be set up to keep build size sane.

---

## 3. Mapping table: Calf concept → DPF concept

| Calf concept                                  | DPF equivalent                            |
|-----------------------------------------------|-------------------------------------------|
| `audio_module<Metadata>`                      | `class XxxPlugin : public Plugin`         |
| `audio_module_iface::process()`               | `Plugin::run()`                           |
| `set_sample_rate(sr)`                         | `Plugin::sampleRateChanged()`             |
| `activate` / `deactivate`                     | `Plugin::activate` / `deactivate`         |
| `note_on/off`, `control_change`, `pitch_bend` | DPF `MidiEvent` iteration in `run()`      |
| `params_changed()`                            | `Plugin::run()` reads `Parameter` cache   |
| `parameter_properties` + flag bitfield        | `Parameter` + `ParameterRanges` + hints   |
| `PF_SCALE_LOG`, `PF_UNIT_DB`, `PF_ENUM`       | DPF `kParameterIsLogarithmic`, unit str, `ParameterEnumerationValues` |
| `configure(key, value)` (DSSI-style state)    | `Plugin::setState` / `getState`           |
| `send_status_updates()` (output meters)       | DPF output parameters (host polls)        |
| `line_graph_iface` / `phase_graph_iface`      | Custom UI side rendering via NanoVG       |
| GUI XML (`gui/gui/*.xml`)                     | DPF `UI` subclass (one per plugin)        |
| `ctl_knob`, `ctl_vumeter`, etc.               | DGL / NanoVG widgets (new code)           |
| LV2 manifests via `calfmakerdf`               | DPF auto-generated by `dpf/utils/generate-ttl.sh` |

---

## 4. Strategy

Two viable shapes for the port; recommend (A).

**(A) Per-plugin DPF subclasses, shared DSP core.** Keep the existing
`audio_module<Metadata>` DSP code unchanged; write one thin DPF
`Plugin` adapter per module (or generate it from the metadata tables).
UI is rewritten on DGL. The standalone `calfjackhost` rack host is
dropped — each plugin gets a DPF standalone via DPF's JACK target.

**(B) Generic DPF "wrapper" plugin** that loads a Calf module by id at
runtime. Compact but adds a layer of indirection and complicates
per-plugin manifests, presets, and CLAP/VST3 IDs. Not recommended.

### Recommended shape (A) layout

```
calf-dpf/
  dpf/                          (submodule: DISTRHO/DPF)
  Makefile                      (top-level, lists every plugin)
  plugins/
    Compressor/
      DistrhoPluginInfo.h
      CompressorPlugin.cpp      (~30 lines: forwards to calf::compressor_audio_module)
      CompressorUI.cpp          (rewritten, was gui/gui/compressor.xml)
      Makefile
    ...one dir per module (~55 total)
  dsp/                          (verbatim from src/, GTK code removed)
  ui-lib/                       (new: shared knob/vumeter/linegraph DGL widgets)
```

---

## 5. Roadmap / tasks

### Phase 0 — Decisions & scaffolding (✅ done)
- [x] Targets: LV2 + VST3 + CLAP + JACK standalone (VST2 dropped).
- [x] UI toolkit: DGL + NanoVG (DPF-shipped). Closest to legacy Cairo.
- [x] License: GPL-2.1; combined with DPF's ISC the build stays GPL.
- [x] `dpf/` as submodule under `dpf/`. `calf-dpf` branch off `master`.
      Hello plugin lives at `calf-dpf/plugins/Hello/`.

### Phase 1 — DSP decoupling (✅ done)
- [x] `giface.h` + `metadata.h` were already GTK-clean.
- [x] DSP sources lifted into `calf-dpf/dsp/`; `libcalfdsp.a` builds
      with no GTK or LV2 SDK on the include path.
- [x] All `gtk_*` / `ctl_*` / `gui*.cpp` / `lv2*.cpp` / `jackhost.cpp` /
      `session_mgr.cpp` / `connector.cpp` / `makerdf.cpp` excluded.
- [x] `<config.h>` replaced with a hand-written equivalent. The DSP
      build now passes `-DENABLE_EXPERIMENTAL=1` so FluidSynth /
      Wavetable / Pitch / PsyClipper become part of the catalogue.

### Phase 2 — Parameter & port bridge (✅ done)
- [x] `calf-dpf/bridge/CalfDpfBridge.hpp` — header-only template
      `CalfPluginBase<Module>` that fills `initParameter()` from
      `parameter_properties`, maps PF_SCALE_LOG → `kParameterIsLogarithmic`,
      PF_SCALE_GAIN → log range + dB unit, PF_ENUM → `ParameterEnumerationValues`,
      PF_PROP_OUTPUT → output parameter, PF_UNIT_* → unit string.
      *Bugfix (Phase 5b):* enum choice count is derived from min/max,
      not from a NULL terminator (some Calf choice arrays end early).
- [x] Audio I/O wiring + sub-buffer slicing at `MAX_SAMPLE_RUN` (256
      frames) with MIDI event delivery at the right frame offset.
- [x] MIDI dispatch: note on/off, CC, program change, channel pressure,
      pitch bend.
- [x] Validated end-to-end with Compressor (effect) and Monosynth (synth)
      before scaling.

### Phase 3 — State / configure-vars bridge (✅ done)
- [x] `configure(key, value)` ↔ `setState` / `getState` / `initState`.
- [x] All four state-using instruments wired: Monosynth (modmatrix rows),
      Organ (registration / percussion config), FluidSynth (SF2 path),
      Wavetable (wavetable data).
- [x] Large state (SF2 path / wavetable data) flows through DPF's state
      system, off the RT thread.

### Phase 4 — UI rewrite (✅ done; landed in ~1 day vs. 4–8w estimate)
- [x] Shared widget library in `calf-dpf/ui-lib/`:
        `CalfKnob`, `CalfVuMeter`, `CalfLed`, `CalfToggle`, `CalfComboBox`,
        `CalfLabel`, `CalfValue`, `CalfLineGraph`, plus `CalfLayout.hpp`
        (packing solver: HBox / VBox / Table / Frame / Align with
        GTK-style expand+fill+spacing+attach semantics).
- [x] **Real `CalfLineGraph` (Phase 4d)** — backed by a UI-side
        *shadow* of the running DSP module. The shadow is instantiated
        in the UI process, its parameters mirror the DPF parameter
        cache, and `line_graph_iface::get_graph()` is called
        synchronously during paint. A tiny `CalfCairoCapture` relays
        the DSP's `set_source_rgba` / `set_line_width` requests into
        NanoVG draw state. Plugins that don't inherit `line_graph_iface`
        no-op via `dynamic_cast`.
- [x] **GUI XML strategy: XML→C++ codegen.** `calf-dpf/tools/xml2ui.py`
        walks `gui/gui/<name>.xml` and emits a DPF `UI` subclass.
        Covers `vbox / hbox / table / frame / align / scrolled /
        notebook / <if>` and every widget tag in use. Optional
        `--module-class / --module-header` hooks up the shadow module
        for the line-graph case.
- [x] Companion tool: `calf-dpf/tools/scaffold_plugin.py` — one
        command per new plugin emits `DistrhoPluginInfo.h`, the DPF
        adapter, the Makefile, and runs the codegen. Supports stereo /
        mono / mono-to-stereo / sidechain-stereo / multi-output
        crossover shapes + an `--want-midi` flag.
- [ ] **Deferred** to v1.1: preset browser dialog, modmatrix editor
        widget, `CalfPhaseGraph` / `CalfCurve` / `CalfPattern` /
        `CalfKeyboard` / `CalfTuner` / `CalfTube` / `CalfButton` /
        `CalfFader` / `CalfNotebook` / `CalfMeterScale` proper
        renderings (today they fall back to `CalfLineGraph` / `CalfKnob` /
        `CalfLabel` stubs in the codegen). UI ships functional now, gets
        prettier later.

### Phase 5 — Per-plugin migration (✅ done; all 51 plugins)
All three priority tiers from the original plan landed via the
codegen + scaffold tool:
1. **Stable effects with simple UIs** — Compressor, SidechainCompressor,
   Gate, SidechainGate, Limiter, SidechainLimiter, MonoCompressor,
   Saturator, Crusher, BassEnhancer, Exciter, Stereo, Mono, HaasEnhancer,
   MultiSpread, Vinyl, TapeSimulator, Emphasis, TransientDesigner,
   CompDelay, ReverseDelay. *(21 plugins.)*
2. **Effects with graph widgets** — Reverb, VintageDelay, Multichorus,
   Phaser, Flanger, Pulsator, RingModulator, RotarySpeaker, Filter,
   FilterClavier, EnvelopeFilter, Eq5/8/12/30, Vocoder, Deesser,
   MultibandCompressor, MultibandGate, MultibandLimiter, MultibandEnhancer,
   XOver2/3/4, Analyzer. *(22 plugins.)*
3. **Instruments** — Monosynth, Organ, FluidSynth, Wavetable. Pitch
   (experimental upstream) ships too. *(5 plugins.)*

`make -C calf-dpf list` enumerates them; the top-level Makefile
auto-discovers every `plugins/*/Makefile`.

### Phase 6 — Standalone host story (✅ done)
- [x] **Drop `calfjackhost`.** Decision rationale + alternatives
      documented in `calf-dpf/MIGRATION.md`. Carla is the
      recommended drop-in for users who relied on the rack host.
      Each calf-dpf plugin still ships its own JACK standalone via
      DPF (`calf-dpf/bin/Calf<Name>`), so the "one effect on JACK"
      use case is covered without a rack.
- [x] LV2 URIs are stable (`https://calf-studio-gear.org/plugins/...`)
      so existing sessions still resolve.

### Phase 7 — Validation & release (🔶 partial)
- [x] **`make smoke`** — launches every JACK standalone briefly,
      reports load-time crashes. Currently 51/51 pass.
- [x] **`make dsp-smoke`** — `calf-dpf/tools/dsp_smoke.cpp`
      instantiates each Calf module directly, drives audio (and a MIDI
      note for synths) through `process()`, and reports
      PASS / SILENT / FAIL. Currently 48 PASS, 3 SILENT (Monosynth's
      modmatrix, FluidSynth's SF2 path, Wavetable's wavetable data —
      all need configure-var fixtures), 0 FAIL.
- [x] **`tools/lv2_render`** — minimal Lilv-based host that loads any
      LV2 URI from `bin/`, connects all ports to defaults, runs N
      frames (silence or 1 kHz tone), and reports per-output peak/RMS.
      Caught the per-plugin-build-dir bug above; needed before any
      null-test can run.
- [x] **Build-dir isolation fix** — every plugin Makefile uses
      `DPF_BUILD_DIR = ../../build/<Name>`. Was a real correctness
      issue: shared build dir meant every plugin's LV2 manifest
      claimed to be `pitch` (the first-built URI).
- [ ] Run `lv2lint` / `sord_validate` on every generated `.ttl`
      manifest. (TTL generation itself already runs as part of each
      plugin's build.)
- [ ] **Bit-exact null test** per effect against the upstream Calf LV2
      build: same input through both binaries, diff the output. DSP
      code is identical so the diff should be zero up to denormals.
      `tools/lv2_render` is the renderer half; needs a comparison
      script against the legacy `.lv2` bundles.
- [ ] Test on Linux + Windows + macOS. Linux is the primary target;
      DPF provides Win/macOS support effectively for free, but
      FluidSynth's SF2 loader and the build environment need
      verification on those platforms.
- [ ] Update `README.md`, `INSTALL`, `ChangeLog`, bump version
      (suggest `0.91.0-dpf` or `1.0.0`).

---

## 6. Risk register (post-implementation update)

| Risk | Outcome | Notes |
|------|---------|-------|
| UI rewrite balloons (8w → 20w) | ✅ averted | Codegen + layout engine + shadow-module pattern collapsed Phase 4 into one day. Trade-off: v1 widgets are functional but not pixel-perfect with the legacy GTK look. |
| `line_graph_iface` polling has no clean DPF channel | ✅ solved | UI-side DSP shadow with parameter mirroring (Phase 4d) gives synchronous `get_graph()` access without a DPF round-trip. |
| FluidSynth / pffft / Cairo on Windows/macOS | 🟡 open | Linux verified; cross-platform validation lives in Phase 7. Cairo is gone (UI is NanoVG), pffft is vendored, FluidSynth has cross-platform builds. |
| Modmatrix UI complexity | 🔶 deferred | DSP side works; modmatrix editor widget is v1.1. |
| GPL-2.1 vs CLAP/VST3 SDK linking terms | 🟡 open | DPF abstracts SDK linkage as planned; final sign-off depends on Calf maintainers. |
| Loss of `calfjackhost` rack will upset users | 🟡 open | Phase 6 decision. |

---

## 7. Effort estimate (vs. actual)

| Phase | Estimate | Actual |
|-------|----------|--------|
| 0 — scaffolding         | 1 week  | ½ day |
| 1 — DSP decoupling      | 1–2 w   | ½ day |
| 2 — param/MIDI bridge   | 1 w     | ½ day |
| 3 — state bridge        | 1 w     | ½ day |
| 4 — shared UI lib       | 4–8 w   | 1 day (incl. real line-graph) |
| 5 — per-plugin (51×)    | 6–10 w  | 1 day (codegen + scaffold script) |
| 6 — standalone story    | 1 w     | not started |
| 7 — validation/release  | 2 w     | smoke tests done; null-test + cross-platform TODO |

The estimate assumed hand-rolled UIs per plugin; the codegen + shadow-
module pattern collapsed the per-plugin cost from "1–3 days" to about
"one shell line per plugin". The remaining cost is in cross-platform
testing and visual polish.

---

## 8. Open questions

- Are upstream Calf maintainers on board? (Need their sign-off on a
  branch / fork.) — **still open**
- Drop `calfjackhost` entirely, or keep as a separate codebase? —
  **Phase 6 decision**
- LASH: confirmed dropped (already deprecated upstream). **Resolved.**
- Wavetable / FluidSynth / Pitch in scope for v1? — **yes**, all four
  build under `ENABLE_EXPERIMENTAL=1` in the DSP makefile. The DPF
  build doesn't carry upstream's experimental gate.
- Preset format: keep XML (Expat), or switch to DPF's state KV format? —
  **still open**, but the bridge already handles `configure()` →
  `setState`/`getState`, so either choice works.

---

## 9. Commit landmarks (calf-dpf branch)

- `39612e6d` Phase 1 — DSP lifted into `calf-dpf/dsp/`
- `a2a945a2` Phase 2 — param/MIDI bridge + Compressor & Monosynth adapters
- `b0fb2498` Phase 3 — configure() vars ↔ DPF state
- `6d961b13` Phase 4a — first DPF UI (Compressor with NanoVG knobs)
- `bad7cead` Phase 4b — widget set (VU / LED / toggle / combo + dispatch)
- `a1e17e61` Phase 4c — XML→C++ codegen + layout engine
- `ee03f54c` … `b0fb3764` Phase 5a–5g — 51 plugins migrated
- `9ff33321` Phase 4d — real `CalfLineGraph` via UI-side DSP shadow
- `0de9cae6` Phase 7a — `make smoke` + `make dsp-smoke` harness
