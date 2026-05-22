/*
 * Calf Pitch — DPF adapter.
 *
 * Upstream Calf gates this module behind ENABLE_EXPERIMENTAL; calf-dpf
 * enables it in the DSP build.
 */
#define ENABLE_EXPERIMENTAL 1
#include "CalfDpfBridge.hpp"
#include <calf/modules_pitch.h>

START_NAMESPACE_DISTRHO

class CalfPitchPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::pitch_audio_module>
{
public:
    CalfPitchPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Pitch — pitch detection / shifting.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'P', 't', 'h');
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

Plugin* createPlugin() { return new CalfPitchPlugin(); }

END_NAMESPACE_DISTRHO
