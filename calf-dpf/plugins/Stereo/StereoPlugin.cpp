/*
 * Calf Stereo — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_tools.h>

START_NAMESPACE_DISTRHO

class CalfStereoPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::stereo_audio_module>
{
public:
    CalfStereoPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Stereo Tools — width, balance, M/S, L/R routing.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'S', 't', 'e');
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

Plugin* createPlugin() { return new CalfStereoPlugin(); }

END_NAMESPACE_DISTRHO
