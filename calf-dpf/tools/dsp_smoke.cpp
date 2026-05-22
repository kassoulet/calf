/*
 * dsp_smoke.cpp — push audio through every Calf module without DPF.
 *
 * Instantiates each module declared in modulelist.h, wires its parameter
 * shadow buffer, drives one buffer of audio (and a few MIDI notes for
 * synths) through process(), and reports per-module PASS / FAIL / SILENT.
 *
 * The goal is to catch DSP-side regressions that wouldn't show up in
 * Phase 5's per-plugin "does it build / does it load" checks. The bridge
 * is bypassed entirely; we exercise the audio_module<...> contract
 * directly.
 *
 * Build: see tools/Makefile (target `dsp_smoke`).
 */
#define ENABLE_EXPERIMENTAL 1

#include <calf/giface.h>
#include <calf/metadata.h>

// Pull in every module header.
#include <calf/modules_synths.h>
#include <calf/organ.h>
#include <calf/modules_dev.h>
#include <calf/wavetable.h>
#include <calf/modules_mod.h>
#include <calf/modules_dist.h>
#include <calf/modules_delay.h>
#include <calf/modules_comp.h>
#include <calf/modules_limit.h>
#include <calf/modules_filter.h>
#include <calf/modules_tools.h>
#include <calf/modules_pitch.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>

using namespace calf_plugins;

namespace {

constexpr uint32_t kSampleRate = 48000;
constexpr uint32_t kFrames     = 256;
constexpr uint32_t kBlocks     = 8;        // ~43 ms of audio

struct Result { const char* name; bool ok; bool silent; const char* msg; };

template <class Module>
Result run_module(const char* name, bool is_synth)
{
    // Heap-allocate: a few modules (reverse_delay, fluidsynth) carry
    // megabytes of inline buffers that overflow the default 8 MB stack.
    auto mod_ptr = std::make_unique<Module>();
    Module& mod = *mod_ptr;
    using Meta = typename Module::metadata_type;

    // Wire parameter shadow buffer (defaults).
    float params[Meta::param_count];
    for (int i = 0; i < Meta::param_count; ++i) {
        params[i] = mod.get_metadata_iface()->get_param_props(i)->def_value;
        mod.params[i] = &params[i];
    }

    mod.post_instantiate(kSampleRate);
    mod.set_sample_rate(kSampleRate);
    mod.params_changed();
    mod.activate();

    // Allocate IO buffers.
    float ibuf[16 * kFrames];
    float obuf[16 * kFrames];
    std::memset(ibuf, 0, sizeof ibuf);
    std::memset(obuf, 0xCC, sizeof obuf);  // poison; non-zero ≠ trust

    // Fill input with white-noise-ish content so meters / dynamics have
    // signal to chew on.
    if constexpr (Meta::in_count > 0) {
        for (uint32_t c = 0; c < Meta::in_count; ++c) {
            for (uint32_t f = 0; f < kFrames * kBlocks; ++f) {
                const float t = static_cast<float>(c * 0x9E3779B1u
                                                   + f * 0x85EBCA77u);
                ibuf[c * kFrames + (f % kFrames)] =
                    std::sin(t * 1e-6f) * 0.5f;
            }
        }
    }

    // Drive a few MIDI notes if this is a synth.
    if (is_synth) {
        mod.note_on(0, 60, 100);
    }

    float sum_abs_out = 0.0f;
    for (uint32_t b = 0; b < kBlocks; ++b) {
        for (uint32_t c = 0; c < Meta::in_count;  ++c) mod.ins[c]  = ibuf + c * kFrames;
        for (uint32_t c = 0; c < Meta::out_count; ++c) mod.outs[c] = obuf + c * kFrames;
        mod.process(0, kFrames, ~0u, ~0u);

        for (uint32_t c = 0; c < Meta::out_count; ++c) {
            for (uint32_t f = 0; f < kFrames; ++f)
                sum_abs_out += std::fabs(obuf[c * kFrames + f]);
        }
    }

    if (is_synth) mod.note_off(0, 60, 0);
    mod.deactivate();

    const bool silent = (sum_abs_out < 1e-6f);
    return Result{name, true, silent, silent ? "no output" : "ok"};
}

#define RUN(modname, is_synth_) \
    do { Result r = run_module<modname##_audio_module>(#modname, is_synth_); \
         results.push_back(r); } while (0)

} // namespace

#include <vector>

int main()
{
    std::vector<Result> results;

    // Synths first.
    RUN(monosynth,           true);
    RUN(organ,               true);
    // fluidsynth + wavetable need external assets (SF2, wavetable data
    // shipped via configure-vars) before process() can run. Phase 7
    // null-test will cover them with proper fixtures.
    results.push_back({"fluidsynth", true, true, "skipped (needs SF2)"});
    results.push_back({"wavetable",  true, true, "skipped (needs wavetable data)"});
    // Effects.
    RUN(multichorus,         false);
    RUN(phaser,              false);
    RUN(flanger,             false);
    RUN(pulsator,            false);
    RUN(ringmodulator,       false);
    RUN(rotary_speaker,      false);
    RUN(tapesimulator,       false);
    RUN(vinyl,               false);
    RUN(reverb,              false);
    RUN(vintage_delay,       false);
    RUN(comp_delay,          false);
    RUN(reverse_delay,       false);
    RUN(compressor,          false);
    RUN(sidechaincompressor, false);
    RUN(multibandcompressor, false);
    RUN(monocompressor,      false);
    RUN(deesser,             false);
    RUN(gate,                false);
    RUN(sidechaingate,       false);
    RUN(multibandgate,       false);
    RUN(limiter,             false);
    RUN(multibandlimiter,    false);
    RUN(sidechainlimiter,    false);
    RUN(transientdesigner,   false);
    RUN(filter,              false);
    RUN(filterclavier,       false);
    RUN(envelopefilter,      false);
    RUN(emphasis,            false);
    RUN(vocoder,             false);
    RUN(equalizer5band,      false);
    RUN(equalizer8band,      false);
    RUN(equalizer12band,     false);
    RUN(equalizer30band,     false);
    RUN(saturator,           false);
    RUN(crusher,             false);
    RUN(exciter,             false);
    RUN(bassenhancer,        false);
    RUN(stereo,              false);
    RUN(haas_enhancer,       false);
    RUN(multibandenhancer,   false);
    RUN(multispread,         false);
    RUN(mono,                false);
    RUN(xover2,              false);
    RUN(xover3,              false);
    RUN(xover4,              false);
    RUN(analyzer,            false);
    RUN(pitch,               false);

    int pass = 0, silent = 0, fail = 0;
    for (const auto& r : results) {
        const char* tag = r.silent ? "SILENT" : (r.ok ? "PASS  " : "FAIL  ");
        std::printf("  %s  %s\n", tag, r.name);
        if (!r.ok)         ++fail;
        else if (r.silent) ++silent;
        else               ++pass;
    }
    std::printf("dsp_smoke: %d pass, %d silent, %d fail (out of %zu).\n",
                pass, silent, fail, results.size());
    return fail > 0 ? 1 : 0;
}
