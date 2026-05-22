/*
 * Calf TransientDesigner — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfTransientDesignerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::transientdesigner_audio_module>
{
public:
    CalfTransientDesignerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Transient Designer — attack/sustain shaping.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'T', 'r', 'D');
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

Plugin* createPlugin() { return new CalfTransientDesignerPlugin(); }

END_NAMESPACE_DISTRHO
