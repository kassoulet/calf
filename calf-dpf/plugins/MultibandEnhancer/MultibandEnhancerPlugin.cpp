/*
 * Calf MultibandEnhancer — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_tools.h>

START_NAMESPACE_DISTRHO

class CalfMultibandEnhancerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::multibandenhancer_audio_module>
{
public:
    CalfMultibandEnhancerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf multiband stereo enhancer.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'b', 'E');
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

Plugin* createPlugin() { return new CalfMultibandEnhancerPlugin(); }

END_NAMESPACE_DISTRHO
