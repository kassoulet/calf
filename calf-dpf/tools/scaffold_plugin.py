#!/usr/bin/env python3
"""
scaffold_plugin.py — create a new plugins/<Name>/ directory wired to the
existing Calf DSP module + bridge + codegen.

Writes DistrhoPluginInfo.h, <Name>Plugin.cpp (CalfPluginBase adapter),
Makefile, and invokes xml2ui.py to produce <Name>UI.cpp. The four files
together are everything a stable, no-graph, no-state effect needs.

Usage:
    scaffold_plugin.py --name Saturator --module saturator \\
        --header calf/modules_dist.h --xml saturator \\
        --unique-id cSat --description "..." --inout stereo

Run from repo root.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


INFO_TMPL = """\
#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND   "Calf"
#define DISTRHO_PLUGIN_NAME    "{name}"
#define DISTRHO_PLUGIN_URI     "https://calf-studio-gear.org/plugins/{slug}"
#define DISTRHO_PLUGIN_CLAP_ID "org.calf-studio-gear.{slug}"

#define DISTRHO_PLUGIN_BRAND_ID  Calf
#define DISTRHO_PLUGIN_UNIQUE_ID {uid}

#define DISTRHO_PLUGIN_HAS_UI       1
#define DISTRHO_UI_USE_NANOVG       1
#define DISTRHO_PLUGIN_IS_RT_SAFE   1
#define DISTRHO_PLUGIN_NUM_INPUTS   {ins}
#define DISTRHO_PLUGIN_NUM_OUTPUTS  {outs}
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT {want_midi}
#define DISTRHO_PLUGIN_WANT_STATE   0

#endif
"""

PLUGIN_TMPL = """\
/*
 * Calf {name} — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <{header}>

START_NAMESPACE_DISTRHO

class Calf{name}Plugin
    : public calf_dpf::CalfPluginBase<calf_plugins::{module}_audio_module>
{{
public:
    Calf{name}Plugin() = default;

protected:
    const char* getDescription() const override
    {{
        return "{description}";
    }}
    int64_t getUniqueId() const override
    {{
        return d_cconst('{u0}', '{u1}', '{u2}', '{u3}');
    }}

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {{
        {init_port_body}
        Plugin::initAudioPort(input, index, port);
    }}

    {run_signature}
    {{
        runBlock(inputs, outputs, frames, {midi_args});
    }}
}};

Plugin* createPlugin() {{ return new Calf{name}Plugin(); }}

END_NAMESPACE_DISTRHO
"""

MAKEFILE_TMPL = """\
#!/usr/bin/make -f

NAME = Calf{name}

FILES_DSP = {name}Plugin.cpp
FILES_UI  = {name}UI.cpp

CALF_DSP_DIR  = ../../dsp
CALF_BRIDGE   = ../../bridge
CALF_UILIB    = ../../ui-lib
BUILD_CXX_FLAGS_EXTRA  = -I$(CALF_DSP_DIR)/include -I$(CALF_DSP_DIR) -I$(CALF_BRIDGE) -I$(CALF_UILIB)
BUILD_CXX_FLAGS_EXTRA += $(shell pkg-config --cflags fluidsynth expat)
EXTRA_DSP_LIBS = $(CALF_DSP_DIR)/libcalfdsp.a $(shell pkg-config --libs fluidsynth expat) -lpthread
EXTRA_UI_LIBS  = $(CALF_DSP_DIR)/libcalfdsp.a $(shell pkg-config --libs fluidsynth expat) -lpthread

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR  = ../../build

include ../../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += $(BUILD_CXX_FLAGS_EXTRA) -std=gnu++17

TARGETS  = jack
TARGETS += lv2_sep
TARGETS += vst3
TARGETS += clap

DPF_DIR = ../../../dpf
LV2_GEN = $(DPF_DIR)/utils/lv2_ttl_generator

all: dspdep $(TARGETS) lv2_ttl

dspdep:
\t$(MAKE) -C $(CALF_DSP_DIR)

$(LV2_GEN):
\t$(MAKE) -C $(DPF_DIR)/utils/lv2-ttl-generator

lv2_ttl: $(LV2_GEN) lv2_dsp
\tcd ../.. && ../dpf/utils/generate-ttl.sh bin

.PHONY: lv2_ttl dspdep
"""


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--name", required=True,
                   help="Plugin display name, CamelCase (e.g. Saturator)")
    p.add_argument("--module", required=True,
                   help="C++ module class root, lower (e.g. saturator)")
    p.add_argument("--header", default="calf/audio_fx.h",
                   help="Header that declares <module>_audio_module")
    p.add_argument("--xml", required=True,
                   help="gui/gui/<xml>.xml basename (without extension)")
    p.add_argument("--unique-id", required=True,
                   help="4-character unique ID (e.g. cSat)")
    p.add_argument("--description", required=True)
    p.add_argument("--inout",
                   choices=("stereo", "mono", "mono-to-stereo",
                            "sidechain-stereo"),
                   default="stereo",
                   help="port shape: stereo=2/2, mono=1/1, mono-to-stereo=1/2, "
                        "sidechain-stereo=4/2 (2 main + 2 sidechain). "
                        "Synths (0/2 + IS_SYNTH + state) are hand-written.")
    p.add_argument("--want-midi", action="store_true",
                   help="Enable MIDI input; run() takes midiEvents/Count and "
                        "forwards them to the bridge's MIDI dispatcher.")
    p.add_argument("--outs", type=int, default=0,
                   help="Override output channel count (for crossover plugins "
                        "with 4/6/8 outputs paired into stereo bands). "
                        "Outputs are grouped as consecutive stereo pairs.")
    p.add_argument("--repo-root", type=Path, default=Path.cwd(),
                   help="repo root (default: cwd)")
    args = p.parse_args()

    if len(args.unique_id) != 4:
        print("error: --unique-id must be 4 characters", file=sys.stderr)
        return 2

    if args.inout == "stereo":
        ins, outs, group = 2, 2, "kPortGroupStereo"
    elif args.inout == "mono":
        ins, outs, group = 1, 1, "kPortGroupMono"
    elif args.inout == "mono-to-stereo":
        # Output is stereo, input is mono. initAudioPort selects per-port.
        ins, outs, group = 1, 2, "MONO_TO_STEREO"
    else:  # sidechain-stereo
        # 2 main + 2 sidechain = 4 inputs; sidechain ports use a separate group.
        ins, outs, group = 4, 2, "SIDECHAIN_STEREO"
    if args.outs > 0:
        outs = args.outs
        if args.outs != 2:
            group = "MULTI_STEREO_OUT"  # paired stereo bands on the output side

    slug = args.module
    plugins_dir = args.repo_root / "calf-dpf" / "plugins" / args.name
    plugins_dir.mkdir(parents=True, exist_ok=True)

    u = args.unique_id

    if group == "MONO_TO_STEREO":
        init_port_body = (
            "port.groupId = input ? kPortGroupMono : kPortGroupStereo;"
        )
    elif group == "SIDECHAIN_STEREO":
        # First two inputs are the main signal, last two are the sidechain.
        init_port_body = (
            "if (input && index >= 2) {\n"
            "            port.groupId = kPortGroupStereo;\n"
            "            port.hints |= kAudioPortIsSidechain;\n"
            "        } else {\n"
            "            port.groupId = kPortGroupStereo;\n"
            "        }"
        )
    elif group == "MULTI_STEREO_OUT":
        # Multi-band crossover: each pair of outputs is one band's stereo
        # pair. We keep all of them in kPortGroupStereo; hosts can name
        # them by port symbol. A proper per-band PortGroup is a follow-up.
        init_port_body = "port.groupId = kPortGroupStereo;"
    else:
        init_port_body = f"port.groupId = {group};"

    if args.want_midi:
        run_signature = (
            "void run(const float** inputs, float** outputs, uint32_t frames,\n"
            "             const MidiEvent* midiEvents, uint32_t midiEventCount) override"
        )
        midi_args = "midiEvents, midiEventCount"
    else:
        run_signature = (
            "void run(const float** inputs, float** outputs, uint32_t frames) override"
        )
        midi_args = "nullptr, 0"

    (plugins_dir / "DistrhoPluginInfo.h").write_text(INFO_TMPL.format(
        name=args.name, slug=slug, uid=u, ins=ins, outs=outs,
        want_midi=1 if args.want_midi else 0))
    (plugins_dir / f"{args.name}Plugin.cpp").write_text(PLUGIN_TMPL.format(
        name=args.name, module=args.module, header=args.header,
        description=args.description.replace('"', '\\"'),
        u0=u[0], u1=u[1], u2=u[2], u3=u[3],
        init_port_body=init_port_body,
        run_signature=run_signature, midi_args=midi_args))
    (plugins_dir / "Makefile").write_text(MAKEFILE_TMPL.format(name=args.name))

    # Codegen the UI.
    xml_path = args.repo_root / "gui" / "gui" / f"{args.xml}.xml"
    ui_out   = plugins_dir / f"{args.name}UI.cpp"
    subprocess.check_call([
        sys.executable,
        str(args.repo_root / "calf-dpf" / "tools" / "xml2ui.py"),
        str(xml_path),
        "--class-name",      f"{args.name}UI",
        "--metadata-class",  f"{args.module}_metadata",
        "-o", str(ui_out),
    ])
    print(f"scaffolded {args.name} in {plugins_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
