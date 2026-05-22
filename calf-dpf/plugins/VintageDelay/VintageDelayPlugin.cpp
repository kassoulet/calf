/*
 * Calf VintageDelay — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_delay.h>

START_NAMESPACE_DISTRHO

class CalfVintageDelayPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::vintage_delay_audio_module>
{
public:
    CalfVintageDelayPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Vintage Delay — tempo-synced delay with stereo flavors.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'V', 'D', 'l');
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

Plugin* createPlugin() { return new CalfVintageDelayPlugin(); }

END_NAMESPACE_DISTRHO
