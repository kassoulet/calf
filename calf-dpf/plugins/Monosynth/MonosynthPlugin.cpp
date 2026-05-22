/*
 * Calf Monosynth — DPF adapter.
 *
 * Wraps calf_plugins::monosynth_audio_module. MIDI input is delivered
 * by DPF and dispatched in calf_dpf::CalfPluginBase::runBlock().
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_synths.h>

START_NAMESPACE_DISTRHO

class CalfMonosynthPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::monosynth_audio_module>
{
public:
    CalfMonosynthPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Monosynth — virtual-analogue monophonic synth with "
               "dual oscillators, modulation matrix, and dual envelopes.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'M', 'S', 'y'); }

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

Plugin* createPlugin() { return new CalfMonosynthPlugin(); }

END_NAMESPACE_DISTRHO
