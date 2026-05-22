/*
 * Calf Vinyl — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_dist.h>

START_NAMESPACE_DISTRHO

class CalfVinylPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::vinyl_audio_module>
{
public:
    CalfVinylPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Vinyl — vinyl playback emulation with aging artifacts.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'V', 'n', 'l');
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

Plugin* createPlugin() { return new CalfVinylPlugin(); }

END_NAMESPACE_DISTRHO
