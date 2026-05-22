/*
 * Calf ReverseDelay — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_delay.h>

START_NAMESPACE_DISTRHO

class CalfReverseDelayPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::reverse_delay_audio_module>
{
public:
    CalfReverseDelayPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Reverse Delay — tempo-synced reverse-tape echo.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'R', 'v', 'D');
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

Plugin* createPlugin() { return new CalfReverseDelayPlugin(); }

END_NAMESPACE_DISTRHO
