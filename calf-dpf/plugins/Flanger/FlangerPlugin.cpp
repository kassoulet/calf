/*
 * Calf Flanger — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_mod.h>

START_NAMESPACE_DISTRHO

class CalfFlangerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::flanger_audio_module>
{
public:
    CalfFlangerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Flanger — stereo flanger with feedback control.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'F', 'l', 'g');
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

Plugin* createPlugin() { return new CalfFlangerPlugin(); }

END_NAMESPACE_DISTRHO
