# Migrating from Calf to calf-dpf

This document covers the user-visible differences between the legacy
GTK-era Calf release and the DPF-based port (`calf-dpf` branch).

Aimed at people who used Calf as an LV2 plugin in a DAW, or who ran the
`calfjackhost` rack as a standalone effects chain. **TL;DR:** plugins
work the same in any LV2 / VST3 / CLAP host; the rack host goes away
and is replaced by an existing mature alternative.

---

## What changed

### Same DSP, four plugin formats

Every Calf audio module — all 51 of them, from Compressor through
Wavetable — is now available as **LV2 + VST3 + CLAP + JACK standalone**
from a single source build. The DSP code is unchanged from upstream;
DPF generates each host wrapper at build time.

If you were loading Calf as LV2 in Ardour / Reaper / Bitwig / etc., you
can keep doing that. The URIs are stable
(`https://calf-studio-gear.org/plugins/<name>`) so existing sessions
will still find the plugins after the upgrade.

### `calfjackhost` is dropped

Upstream Calf shipped its own rack host (`calfjackhost`), a JACK-based
patcher with session management and a built-in modulation matrix. The
DPF port doesn't build it. Two reasons:

1. **DPF doesn't ship a rack host.** It produces one binary per plugin.
2. **Modern alternatives cover the same ground better.** Carla, Ardour,
   Reaper, Bitwig all host LV2 / VST3 / CLAP plugins, including the
   Calf set, in environments that get more development attention than
   a Calf-specific rack could.

### Recommended replacements

| Use case | Replacement |
|----------|-------------|
| JACK rack with multiple plugins chained on the fly | **[Carla](https://kx.studio/Applications:Carla)** — exact drop-in. Patchbay, session save, JACK MIDI, LV2/VST3/CLAP. |
| Per-plugin JACK standalone (one effect on JACK) | Each calf-dpf plugin already ships a JACK standalone — `calf-dpf/bin/CalfCompressor`, etc. |
| Calf-style session with `.calfpre` presets | Re-load the same plugins in Carla and re-save as Carla project. Preset XML support inside the plugins is unchanged. |
| Embedded inside a DAW | Use LV2 / VST3 / CLAP directly as before. |

### LASH session management dropped

LASH has been deprecated upstream for years; calf-dpf doesn't link it.
Use your DAW's session save instead, or Carla's project files.

### Cairo dependency gone

The UI is now NanoVG (via DPF's DGL layer), not Cairo. Cairo is no
longer in the dependency list. The visual style is close to the
original GTK look but not pixel-identical.

---

## Build and install

```sh
git submodule update --init --recursive
make -C calf-dpf            # builds every plugin
```

Artifacts land in `calf-dpf/bin/`:

- `CalfFoo.lv2/` — LV2 bundle
- `CalfFoo.vst3/` — VST3 bundle
- `CalfFoo.clap` — CLAP plugin
- `CalfFoo` — JACK standalone executable

Copy the bundles into the usual locations (`~/.lv2/`,
`~/.vst3/`, `~/.clap/`).

See `calf-dpf/README.md` for more detail.

---

## Reporting regressions

If a session that worked with upstream Calf misbehaves under
calf-dpf, please attach:

1. The DAW / host you were using.
2. The specific plugin (URI) and parameter values.
3. A short JACK / WAV capture showing the difference, if relevant.

The DSP code is identical, so audible differences are almost certainly
bugs in the DPF bridge or UI layer rather than in the DSP itself.
