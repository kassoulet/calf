# Calf Studio gear

[![Build status](https://github.com/calf-studio-gear/calf/actions/workflows/build.yml/badge.svg)](https://github.com/calf-studio-gear/calf/actions/workflows/build.yml)

Calf Studio Gear is an audio plug-in pack for LV2 and JACK environments
under LINUX operating systems. The suite contains lots of effects
(delay, modulation, signal processing, filters, equalizers, dynamics,
distortion and mastering effects), instruments (SF2 player, organ
simulator and a monophonic synthesizer) and tools (analyzer, mono/stereo
tools, crossovers). Calf Studio Gear aims for a professional audience.

Please visit the website below for further information,
screenshots and installation instructions.

http://calf-studio-gear.org

## Dependencies and licenses

This project is licensed under GPL-2.1-only.
It uses GTK and FluidSynth (LGPL-2.1-or-later).
It also uses Expat (MIT) and LV2 (ISC).
For details, see `licenses/README_LICENSES.txt` in CALF's source tree.

## Features

- Instruments and tone generators (Organ, Monosynth, Wavetable, Fluidsynth)
- Modulation effects (Multi Chorus, Phaser, Flanger, Rotary, Pulsator, Ring Modulator)
- Delay effects (Reverb, Vintage Delay,Compensation Delay Line, Reverse Delay)
- Dynamic processors (Compressor, Sidechain Compressor, Multiband Compressor, Mono Compressor, Deesser, Gate, Sidechain Gate, Multiband Gate, Limiter, Multiband Limiter, Sidechain Limiter, Transient Designer)
- Filters and equalizers (Filter, Filterclavier, Envelope Filter, Equalizer 5 Band, Equalizer 8 Band, Equalizer 12 Band, Equalizer 30 Band, Vocoder, Emphasis)
- Distortion and enhancement (Saturator, Exciter, Bass Enhancer, Tape Simulator, Vinyl, Crusher)
- Tools (Mono Input, Stereo Tools, Haas Stereo Enhancer, Multi Spread, Analyzer, X-Over 2 Band, X-Over 3 Band, X-Over 4 Band)

## Usage

Once installed the plug-ins can be loaded by any LV2 / VST3 / CLAP host.
For a standalone rack, use [Carla](https://kx.studio/Applications:Carla) —
the legacy `calfjackhost` has been retired (see
`calf-dpf/MIGRATION.md`).

The HTML plug-in manual ships under `/usr/[local]/share/doc/calf`
(depending on your install prefix) and is also reachable from the
plug-in UI menus.

## Build

Calf is built as DPF plug-ins (LV2 + VST3 + CLAP + JACK standalone).
There is no CMake / autotools any more — the build is a plain Makefile.

```sh
git submodule update --init --recursive
make -C calf-dpf -j$(nproc)
```

Artifacts land under `calf-dpf/bin/`. See
[`calf-dpf/README.md`](calf-dpf/README.md) for per-plugin builds, smoke
tests, and the XML→UI codegen. CI runs the same commands; see
[.github/workflows/build.yml](.github/workflows/build.yml).

