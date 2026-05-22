/*
 * Calf XOver4 — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfXOver4Plugin
    : public calf_dpf::CalfPluginBase<calf_plugins::xover4_audio_module>
{
public:
    CalfXOver4Plugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 4-band crossover — split stereo input into four stereo bands.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'X', 'o', '4');
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

Plugin* createPlugin() { return new CalfXOver4Plugin(); }

END_NAMESPACE_DISTRHO
