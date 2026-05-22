/*
 * Calf Emphasis — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfEmphasisPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::emphasis_audio_module>
{
public:
    CalfEmphasisPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Emphasis — RIAA + CD pre/de-emphasis filters.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'E', 'm', 'p');
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

Plugin* createPlugin() { return new CalfEmphasisPlugin(); }

END_NAMESPACE_DISTRHO
