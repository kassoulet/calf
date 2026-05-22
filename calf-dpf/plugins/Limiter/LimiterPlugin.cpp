/*
 * Calf Limiter — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_limit.h>

START_NAMESPACE_DISTRHO

class CalfLimiterPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::limiter_audio_module>
{
public:
    CalfLimiterPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Limiter — stereo brick-wall limiter with look-ahead "
               "and configurable release.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'L', 'm', 't'); }

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

Plugin* createPlugin() { return new CalfLimiterPlugin(); }

END_NAMESPACE_DISTRHO
