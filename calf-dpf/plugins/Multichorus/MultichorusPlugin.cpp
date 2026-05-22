/*
 * Calf Multichorus — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_mod.h>

START_NAMESPACE_DISTRHO

class CalfMultichorusPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::multichorus_audio_module>
{
public:
    CalfMultichorusPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Multichorus — multi-voice stereo chorus.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'C', 'h');
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

Plugin* createPlugin() { return new CalfMultichorusPlugin(); }

END_NAMESPACE_DISTRHO
