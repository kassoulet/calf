/*
 * Calf Gate — DPF adapter. All DSP and parameter metadata comes from
 * libcalfdsp.a; this file contributes only the unique ID and the run()
 * forwarder.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfGatePlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::gate_audio_module>
{
public:
    CalfGatePlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Gate — stereo expander/gate with adjustable knee, "
               "ratio, and external/internal detection.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'G', 't', 'e'); }

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

Plugin* createPlugin() { return new CalfGatePlugin(); }

END_NAMESPACE_DISTRHO
