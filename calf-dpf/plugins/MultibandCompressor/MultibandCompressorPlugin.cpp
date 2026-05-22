/*
 * Calf MultibandCompressor — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfMultibandCompressorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::multibandcompressor_audio_module>
{
public:
    CalfMultibandCompressorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf 4-band multiband compressor.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'M', 'b', 'C');
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

Plugin* createPlugin() { return new CalfMultibandCompressorPlugin(); }

END_NAMESPACE_DISTRHO
