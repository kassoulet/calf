/*
 * Calf Crusher — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfCrusherPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::crusher_audio_module>
{
public:
    CalfCrusherPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Crusher — bit-depth + sample-rate reduction lo-fi tool.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'C', 'r', 's');
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

Plugin* createPlugin() { return new CalfCrusherPlugin(); }

END_NAMESPACE_DISTRHO
