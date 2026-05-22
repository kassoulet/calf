# Calf → DPF (DISTRHO Plugin Framework) Port Roadmap

Status: planning / analysis. Target: build the Calf plugin suite as DPF plugins
so that LV2, VST2, VST3, CLAP, and JACK-standalone binaries fall out of a
single source tree.

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

### Phase 0 — Decisions & scaffolding (1 week)
- [ ] Confirm target formats: at minimum LV2 + VST3 + CLAP + JACK standalone.
- [ ] Decide on UI toolkit inside DPF: pure DGL widgets, NanoVG, or a
      third option (e.g., ImGui-via-DPF). NanoVG is recommended — closest
      to current Cairo-based rendering.
- [ ] Pick license posture. Calf is GPL-2.1-only; DPF is ISC. The combined
      product is GPL. Confirm with maintainers (`AUTHORS`, upstream).
- [ ] Add `dpf/` as a git submodule. Stand up a `calf-dpf` branch off
      `master`. Wire one trivial "hello plugin" target to verify builds.

### Phase 1 — DSP decoupling (1–2 weeks)
- [ ] Audit `src/calf/giface.h` and `metadata.h`: confirm no GTK leakage
      (already mostly clean — the single `gtk`-substring match in
      `giface.h` is a comment).
- [ ] Move `src/calf/*.h` DSP headers + the `modules_*.cpp` / `audio_fx.cpp` /
      `monosynth.cpp` / `organ.cpp` / `wavetable.cpp` / `fluidsynth.cpp` /
      `analyzer.cpp` / `modmatrix.cpp` into `calf-dpf/dsp/`.
- [ ] Strip the `lv2wrap.{h,cpp}` + `lv2gui.cpp` + `makerdf.cpp` + all
      `gtk_*`, `ctl_*`, `gui*.cpp`, `connector.cpp`, `host_session.cpp`,
      `jack_client.cpp`, `jackhost.cpp`, `session_mgr.cpp` files from the
      DSP tree.
- [ ] Replace `<config.h>` includes with a hand-written equivalent
      (`VERSION`, `ENABLE_EXPERIMENTAL`, no `USE_LV2` / `USE_GUI` defines).
- [ ] Build the DSP core as a static `libcalfdsp.a`; verify it compiles
      stand-alone with no GTK / LV2 SDK on the include path.

### Phase 2 — Parameter & port bridge (1 week)
- [ ] Write a header-only adapter `dpf_bridge.h` that, given a
      `plugin_metadata_iface*`, generates the DPF `initParameter()` body:
        - linear → `ParameterRanges`
        - `PF_SCALE_LOG` → `kParameterIsLogarithmic`
        - `PF_SCALE_GAIN` → log range, unit dB
        - `PF_ENUM` → `ParameterEnumerationValues`
        - `PF_PROP_OUTPUT` → output parameter
        - `PF_UNIT_*` → unit string ("Hz", "dB", "ms", "BPM", …)
- [ ] Audio I/O port mapping: `audio_module::ins/outs` arrays connect to
      DPF's `inputs`/`outputs` in `run()`; `audio_module::params` array
      points at DPF parameter cache.
- [ ] MIDI bridge: iterate `MidiEvent`s in `run()`, dispatch into
      `note_on / note_off / control_change / pitch_bend / channel_pressure`
      between sub-buffer slices of `MAX_SAMPLE_RUN` (=256) samples.
- [ ] Validate with **one** simple effect (Compressor) and **one** synth
      (Monosynth) end-to-end before scaling.

### Phase 3 — State / configure-vars bridge (3–5 days)
- [ ] Map `audio_module::configure(key, value)` + `send_configures()` onto
      DPF `Plugin::setState` / `getState` / `initState`.
- [ ] Audit which plugins use it: organ (preset/voice config), wavetable
      (wavetable data), monosynth (modmatrix rows), fluidsynth (SF2 path).
- [ ] Handle large state (wavetable, SF2 path) without blocking RT.

### Phase 4 — UI rewrite (4–8 weeks; the long pole)
- [ ] Build the shared DGL widget library in `ui-lib/`:
        - `CalfKnob` (replaces `ctl_knob`)
        - `CalfVUMeter` + scale (`ctl_vumeter`, `ctl_meterscale`)
        - `CalfLineGraph` (replaces `ctl_linegraph` — biggest single
          widget at 1546 LOC; hosts FFT, EQ curves, compressor curves)
        - `CalfPhaseGraph`, `CalfCurve`, `CalfPattern`, `CalfKeyboard`,
          `CalfTuner`, `CalfTube`, `CalfLED`, `CalfButton`, `CalfFader`,
          `CalfComboBox`, `CalfNotebook`, `CalfFrame`
        - Plug the existing `line_graph_iface` / `phase_graph_iface`
          pulls into the UI side — DPF UIs can ask the DSP via parameter
          values + state, but the *graph* path needs an out-of-band
          channel (e.g., `requestParameterValueChange` for triggers,
          state for snapshots).
- [ ] Decide GUI XML strategy:
        - **(preferred)** Write a one-off XML→C++ codegen that reads
          `gui/gui/*.xml` and emits one `UI` subclass per plugin. The
          XML grammar is small and consistent.
        - Or: ship a runtime XML interpreter inside the UI lib. More
          code, but lets the XML files stay as-is.
- [ ] Re-implement preset browser as a DPF UI dialog (existing
      `preset.xml` parser code can be reused).
- [ ] Re-implement modulation matrix UI (monosynth, organ) — currently
      `modmatrix.cpp` + GTK; the DSP side stays, only the editor moves.

### Phase 5 — Per-plugin migration (rolling; 1–3 days per plugin)
Prioritized order — easiest / highest-impact first, gnarliest last:
1. **Stable effects with simple UIs** (no big graphs): Compressor,
   SidechainCompressor, Gate, SidechainGate, Limiter, MonoCompressor,
   Saturator, Crusher, BassEnhancer, Exciter, Stereo, Mono, Haas,
   MultiSpread, Vinyl, TapeSimulator, Emphasis, TransientDesigner,
   CompDelay, ReverseDelay. (~20 plugins; the regression-test bulk.)
2. **Effects with graph widgets:** Reverb, VintageDelay, Multichorus,
   Phaser, Flanger, Pulsator, RingModulator, RotarySpeaker, Filter,
   FilterClavier, EnvelopeFilter, EQ5/8/12/30, Vocoder, Deesser,
   MultibandCompressor, MultibandGate, MultibandLimiter,
   SidechainLimiter, MultibandEnhancer, XOver2/3/4, Analyzer.
3. **Instruments:** Monosynth, Organ, FluidSynth, Wavetable. These have
   the largest UIs and the heaviest state.

### Phase 6 — Standalone host story (1 week)
- [ ] Decide: drop `calfjackhost` rack entirely (recommended; DPF
      standalones cover the single-plugin case) **or** keep it as a
      separate non-DPF mini-rack that loads DPF plugins via Carla.
- [ ] If dropped: document migration path for existing rack users
      (Carla, Ardour, Reaper).

### Phase 7 — Validation & release (2 weeks)
- [ ] Run `lv2lint` / `sord_validate` (former `sordi`) on all generated
      manifests. Was already part of the build (`WANT_SORDI`).
- [ ] Bit-exact null test for each effect: run the original LV2 and the
      DPF LV2 on the same input, diff. DSP code is the same library,
      so the diff should be zero up to denormals.
- [ ] Test on Linux + Windows + macOS. (Calf is Linux-primary today;
      DPF gives Win/macOS support effectively for free, but FluidSynth
      and Cairo dependencies on those platforms need verification.)
- [ ] Update `README.md`, `INSTALL`, `ChangeLog`, bump version
      (suggest `0.91.0-dpf` or `1.0.0`).

---

## 6. Risk register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| UI rewrite balloons (8 weeks → 20 weeks) | High | High | Codegen XML→DGL; ship effects without graphs first, accept "v1 has plainer UI". |
| `line_graph_iface` realtime polling has no clean DPF channel | Medium | Medium | Use DPF output params for scalar snapshots; use `requestStateChange` only for slow updates. |
| FluidSynth / pffft / Cairo on Windows/macOS | Medium | Medium | FluidSynth has Win/macOS builds; vendor pffft; Cairo not needed once UI moves to NanoVG. |
| Modmatrix UI complexity (monosynth, organ) | Medium | Medium | Re-use existing modmatrix DSP; UI is a flat table — straightforward DGL. |
| GPL-2.1 vs CLAP/VST3 SDK linking terms | Low | High | DPF abstracts SDK linkage; verify with DPF maintainer + Calf authors. |
| Loss of `calfjackhost` rack will upset users | Medium | Low | Document Carla as drop-in. |

---

## 7. Effort estimate (rough)

| Phase | Calendar weeks (one engineer) |
|-------|-------------------------------|
| 0 — scaffolding         | 1 |
| 1 — DSP decoupling      | 1–2 |
| 2 — param/MIDI bridge   | 1 |
| 3 — state bridge        | 1 |
| 4 — shared UI lib       | 4–8 |
| 5 — per-plugin (55×)    | 6–10 (parallelizable) |
| 6 — standalone story    | 1 |
| 7 — validation/release  | 2 |
| **Total** | **~17–26 weeks** |

The UI is the dominant cost. If acceptable, a "v1" that ships effects
with auto-generated parameter-only UIs (no custom graphs / VU meters)
could be done in ~6 weeks; the rich UIs land in v1.1.

---

## 8. Open questions

- Are upstream Calf maintainers on board? (Need their sign-off on a
  branch / fork.)
- Drop `calfjackhost` entirely, or keep as a separate codebase?
- LASH: confirm we drop it. (Already deprecated upstream.)
- Wavetable / FluidSynth: are these in scope for v1, or defer?
- Preset format: keep XML (Expat), or switch to DPF's state KV format?
