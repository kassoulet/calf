/*
 * Calf Compressor — DPF adapter.
 *
 * Thin shim around calf_plugins::compressor_audio_module. All parameter
 * metadata, scaling, and DSP comes verbatim from libcalfdsp.a; the only
 * code unique to this file is the unique ID and the run() forwarder.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_comp.h>

START_NAMESPACE_DISTRHO

class CalfCompressorPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::compressor_audio_module>
{
public:
    CalfCompressorPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Compressor — stereo feed-forward compressor with "
               "RMS/peak detection and soft knee.";
    }
    int64_t getUniqueId() const override { return d_cconst('c', 'C', 'm', 'p'); }

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

Plugin* createPlugin() { return new CalfCompressorPlugin(); }

END_NAMESPACE_DISTRHO
