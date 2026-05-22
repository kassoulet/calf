/*
 * Calf MultibandGate — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfMultibandGatePlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::multibandgate_audio_module>
{
public:
    CalfMultibandGatePlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 4-band multiband gate.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'b', 'G');
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

Plugin* createPlugin() { return new CalfMultibandGatePlugin(); }

END_NAMESPACE_DISTRHO
