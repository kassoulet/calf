/*
 * Calf Vocoder — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_filter.h>

START_NAMESPACE_DISTRHO

class CalfVocoderPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::vocoder_audio_module>
{
public:
    CalfVocoderPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Vocoder — channel vocoder, modulator + carrier inputs.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'V', 'o', 'c');
    }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        if (input && index >= 2) {
            port.groupId = kPortGroupStereo;
            port.hints |= kAudioPortIsSidechain;
        } else {
            port.groupId = kPortGroupStereo;
        }
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        runBlock(inputs, outputs, frames, nullptr, 0);
    }
};

Plugin* createPlugin() { return new CalfVocoderPlugin(); }

END_NAMESPACE_DISTRHO
