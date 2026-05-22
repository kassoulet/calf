/*
 * Calf MonoCompressor — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfMonoCompressorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::monocompressor_audio_module>
{
public:
    CalfMonoCompressorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf MonoCompressor — single-channel feed-forward compressor "
               "with soft knee, RMS/peak detection, and lookahead.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'M', 'C', 'm'); }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupMono;
        Plugin::initAudioPort(input, index, port);
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        runBlock(inputs, outputs, frames, nullptr, 0);
    }
};

Plugin* createPlugin() { return new CalfMonoCompressorPlugin(); }

END_NAMESPACE_DISTRHO
