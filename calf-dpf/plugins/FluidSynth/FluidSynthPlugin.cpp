/*
 * Calf FluidSynth — DPF adapter.
 *
 * Wraps calf_plugins::fluidsynth_audio_module — a SoundFont (SF2) player
 * built around libfluidsynth. The SF2 path travels through the bridge's
 * configure-var → DPF state pipeline (Phase 3). Upstream Calf gates this
 * module behind ENABLE_EXPERIMENTAL; calf-dpf enables it in the DSP
 * build because the actual functionality is stable on Linux.
 *
 * Not RT-safe — fluidsynth can allocate during loadFile() and during
 * voice steal; we declare DISTRHO_PLUGIN_IS_RT_SAFE 0 accordingly.
 */
#define ENABLE_EXPERIMENTAL 1
#include "CalfDpfBridge.hpp"
#include <calf/modules_dev.h>

START_NAMESPACE_DISTRHO

class CalfFluidSynthPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::fluidsynth_audio_module>
{
public:
    CalfFluidSynthPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf FluidSynth — SoundFont (SF2) player wrapped around "
               "libfluidsynth, with reverb + chorus.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'F', 'l', 'S'); }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        if (!input) port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float**, float** outputs, uint32_t frames,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {
        runBlock(nullptr, outputs, frames, midiEvents, midiEventCount);
    }
};

Plugin* createPlugin() { return new CalfFluidSynthPlugin(); }

END_NAMESPACE_DISTRHO
