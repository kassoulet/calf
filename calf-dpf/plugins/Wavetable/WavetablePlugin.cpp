/*
 * Calf Wavetable — DPF adapter.
 *
 * Wavetable data + modmatrix rows carried in configure-vars, threaded
 * through the bridge's state pipeline. Upstream Calf gates this module
 * behind ENABLE_EXPERIMENTAL; calf-dpf enables it in the DSP build.
 */
#define ENABLE_EXPERIMENTAL 1
#include "CalfDpfBridge.hpp"
#include <calf/wavetable.h>

START_NAMESPACE_DISTRHO

class CalfWavetablePlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::wavetable_audio_module>
{
public:
    CalfWavetablePlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Wavetable — wavetable synthesiser with configurable "
               "modulation matrix and per-voice envelopes.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'W', 'v', 't'); }

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

Plugin* createPlugin() { return new CalfWavetablePlugin(); }

END_NAMESPACE_DISTRHO
