/*
 * Calf Pulsator — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_mod.h>

START_NAMESPACE_DISTRHO

class CalfPulsatorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::pulsator_audio_module>
{
public:
    CalfPulsatorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Pulsator — tempo-synced tremolo + auto-pan.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'P', 'l', 's');
    }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        runBlock(inputs, outputs, frames, nullptr, 0);
    }
};

Plugin* createPlugin() { return new CalfPulsatorPlugin(); }

END_NAMESPACE_DISTRHO
