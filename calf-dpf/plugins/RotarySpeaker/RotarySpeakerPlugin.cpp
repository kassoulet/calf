/*
 * Calf RotarySpeaker — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_mod.h>

START_NAMESPACE_DISTRHO

class CalfRotarySpeakerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::rotary_speaker_audio_module>
{
public:
    CalfRotarySpeakerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Rotary Speaker — Leslie cabinet emulation with MIDI rotor control.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'R', 'S', 'p');
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

Plugin* createPlugin() { return new CalfRotarySpeakerPlugin(); }

END_NAMESPACE_DISTRHO
