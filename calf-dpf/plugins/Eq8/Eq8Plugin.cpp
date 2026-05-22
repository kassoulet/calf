/*
 * Calf Eq8 — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfEq8Plugin
    : public calf_dpf::CalfPluginBase<calf_plugins::equalizer8band_audio_module>
{
public:
    CalfEq8Plugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 8-band parametric equalizer + LP/HP.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'E', 'q', '8');
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

Plugin* createPlugin() { return new CalfEq8Plugin(); }

END_NAMESPACE_DISTRHO
