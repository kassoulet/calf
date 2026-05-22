/*
 * Calf TapeSimulator — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfTapeSimulatorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::tapesimulator_audio_module>
{
public:
    CalfTapeSimulatorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Tape Simulator — analog tape saturation + speed wobble.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'T', 'p', 'e');
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

Plugin* createPlugin() { return new CalfTapeSimulatorPlugin(); }

END_NAMESPACE_DISTRHO
