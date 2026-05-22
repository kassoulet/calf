/*
 * Calf Reverb — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_delay.h>

START_NAMESPACE_DISTRHO

class CalfReverbPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::reverb_audio_module>
{
public:
    CalfReverbPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Reverb — algorithmic stereo reverb with adjustable size and damping.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'R', 'v', 'b');
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

Plugin* createPlugin() { return new CalfReverbPlugin(); }

END_NAMESPACE_DISTRHO
