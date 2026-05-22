/*
 * Calf BassEnhancer — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfBassEnhancerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::bassenhancer_audio_module>
{
public:
    CalfBassEnhancerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Bass Enhancer — psychoacoustic low-end enhancement via harmonic generation.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'B', 'E', 'n');
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

Plugin* createPlugin() { return new CalfBassEnhancerPlugin(); }

END_NAMESPACE_DISTRHO
