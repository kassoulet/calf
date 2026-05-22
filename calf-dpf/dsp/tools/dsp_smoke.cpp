/*
 * Phase 1 smoke test for libcalfdsp.a.
 *
 * Instantiates a compressor, sets sample rate, activates it, runs one
 * MAX_SAMPLE_RUN block of silence through it. Exits 0 on success.
 * Verifies that the DSP core links and runs without GTK / LV2 / JACK.
 */
#include <calf/modules_comp.h>
#include <cstdio>
#include <cstring>

using namespace calf_plugins;

int main()
{
    compressor_audio_module mod;

    // Set up port-array pointers (DSP core needs valid float* even for
    // unused params; back them with stack buffers).
    float dummy_param[compressor_metadata::param_count];
    std::memset(dummy_param, 0, sizeof(dummy_param));
    float inL[MAX_SAMPLE_RUN] = {0};
    float inR[MAX_SAMPLE_RUN] = {0};
    float outL[MAX_SAMPLE_RUN] = {0};
    float outR[MAX_SAMPLE_RUN] = {0};
    float* ins[2]  = {inL, inR};
    float* outs[2] = {outL, outR};
    for (int i = 0; i < compressor_metadata::in_count; ++i)
        mod.ins[i] = ins[i];
    for (int i = 0; i < compressor_metadata::out_count; ++i)
        mod.outs[i] = outs[i];
    for (int i = 0; i < compressor_metadata::param_count; ++i)
        mod.params[i] = &dummy_param[i];

    mod.post_instantiate(48000);
    mod.set_sample_rate(48000);
    mod.activate();
    mod.params_changed();
    uint32_t produced = mod.process(0, MAX_SAMPLE_RUN, ~0u, ~0u);
    mod.deactivate();

    std::printf("compressor process returned mask 0x%x\n", produced);
    return 0;
}
