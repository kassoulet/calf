/*
 * Calf Deesser — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfDeesserPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::deesser_audio_module>
{
public:
    CalfDeesserPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Deesser — split-band sibilance removal.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'D', 's', 's');
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

Plugin* createPlugin() { return new CalfDeesserPlugin(); }

END_NAMESPACE_DISTRHO
