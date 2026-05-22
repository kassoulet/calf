/*
 * Calf XOver2 — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfXOver2Plugin
    : public calf_dpf::CalfPluginBase<calf_plugins::xover2_audio_module>
{
public:
    CalfXOver2Plugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 2-band crossover — split stereo input into two stereo bands.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'X', 'o', '2');
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

Plugin* createPlugin() { return new CalfXOver2Plugin(); }

END_NAMESPACE_DISTRHO
