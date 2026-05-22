/*
 * Calf FilterClavier — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfFilterClavierPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::filterclavier_audio_module>
{
public:
    CalfFilterClavierPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Filter Clavier — MIDI-controlled biquad filter.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'F', 'C', 'l');
    }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {
        runBlock(inputs, outputs, frames, midiEvents, midiEventCount);
    }
};

Plugin* createPlugin() { return new CalfFilterClavierPlugin(); }

END_NAMESPACE_DISTRHO
