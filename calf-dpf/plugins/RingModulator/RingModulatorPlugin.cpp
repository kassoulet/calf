/*
 * Calf RingModulator — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_mod.h>

START_NAMESPACE_DISTRHO

class CalfRingModulatorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::ringmodulator_audio_module>
{
public:
    CalfRingModulatorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Ring Modulator — sine + LFO ring modulator.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'R', 'M', 'd');
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

Plugin* createPlugin() { return new CalfRingModulatorPlugin(); }

END_NAMESPACE_DISTRHO
