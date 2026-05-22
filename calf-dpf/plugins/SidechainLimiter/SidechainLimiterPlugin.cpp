/*
 * Calf SidechainLimiter — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_limit.h>

START_NAMESPACE_DISTRHO

class CalfSidechainLimiterPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::sidechainlimiter_audio_module>
{
public:
    CalfSidechainLimiterPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Sidechain Limiter — multiband limiter with sidechain input.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'S', 'L', 'm');
    }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        if (input && index >= 2) {
            port.groupId = kPortGroupStereo;
            port.hints |= kAudioPortIsSidechain;
        } else {
            port.groupId = kPortGroupStereo;
        }
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        runBlock(inputs, outputs, frames, nullptr, 0);
    }
};

Plugin* createPlugin() { return new CalfSidechainLimiterPlugin(); }

END_NAMESPACE_DISTRHO
