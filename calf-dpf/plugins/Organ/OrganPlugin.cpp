/*
 * Calf Organ — DPF adapter.
 *
 * Wraps calf_plugins::organ_audio_module — a Hammond-style drawbar organ
 * with 9 harmonics, percussion + key click sections, and a Leslie sim.
 * State variables (drawbar registrations, percussion patterns, etc.)
 * flow through the bridge's configure-var → DPF state pipeline.
 */
#include "CalfDpfBridge.hpp"
#include <calf/organ.h>

START_NAMESPACE_DISTRHO

class CalfOrganPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::organ_audio_module>
{
public:
    CalfOrganPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Organ — drawbar organ with percussion + key click + "
               "rotary-speaker emulation. Polyphonic, 9 harmonics per voice.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'O', 'r', 'g'); }

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

Plugin* createPlugin() { return new CalfOrganPlugin(); }

END_NAMESPACE_DISTRHO
