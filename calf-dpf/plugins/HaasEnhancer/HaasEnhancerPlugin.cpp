/*
 * Calf HaasEnhancer — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_delay.h>

START_NAMESPACE_DISTRHO

class CalfHaasEnhancerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::haas_enhancer_audio_module>
{
public:
    CalfHaasEnhancerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Haas Stereo Enhancer — short-delay stereo widening.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'H', 'a', 'a');
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

Plugin* createPlugin() { return new CalfHaasEnhancerPlugin(); }

END_NAMESPACE_DISTRHO
