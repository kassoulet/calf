/*
 * Calf Eq30 — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfEq30Plugin
    : public calf_dpf::CalfPluginBase<calf_plugins::equalizer30band_audio_module>
{
public:
    CalfEq30Plugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 30-band graphic equalizer.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'E', '3', '0');
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

Plugin* createPlugin() { return new CalfEq30Plugin(); }

END_NAMESPACE_DISTRHO
