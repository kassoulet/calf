/*
 * Calf MultibandLimiter — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_limit.h>

START_NAMESPACE_DISTRHO

class CalfMultibandLimiterPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::multibandlimiter_audio_module>
{
public:
    CalfMultibandLimiterPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 4-band multiband limiter.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'b', 'L');
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

Plugin* createPlugin() { return new CalfMultibandLimiterPlugin(); }

END_NAMESPACE_DISTRHO
