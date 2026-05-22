/*
 * Calf Exciter — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfExciterPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::exciter_audio_module>
{
public:
    CalfExciterPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Exciter — psychoacoustic high-frequency enhancement.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'E', 'x', 'c');
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

Plugin* createPlugin() { return new CalfExciterPlugin(); }

END_NAMESPACE_DISTRHO
