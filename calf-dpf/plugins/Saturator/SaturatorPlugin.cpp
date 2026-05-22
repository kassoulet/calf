/*
 * Calf Saturator — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfSaturatorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::saturator_audio_module>
{
public:
    CalfSaturatorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Saturator — tube saturation with adjustable drive, blend and tone.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'S', 'a', 't');
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

Plugin* createPlugin() { return new CalfSaturatorPlugin(); }

END_NAMESPACE_DISTRHO
