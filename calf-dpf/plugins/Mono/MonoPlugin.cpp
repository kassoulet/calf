/*
 * Calf Mono — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_tools.h>

START_NAMESPACE_DISTRHO

class CalfMonoPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::mono_audio_module>
{
public:
    CalfMonoPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Mono Input — mono-to-stereo distributor with gain + stereo enhancement.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'n', 'o');
    }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = input ? kPortGroupMono : kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        runBlock(inputs, outputs, frames, nullptr, 0);
    }
};

Plugin* createPlugin() { return new CalfMonoPlugin(); }

END_NAMESPACE_DISTRHO
