/*
 * Calf Analyzer — DPF adapter. DSP and metadata from libcalfdsp.a.
 */
#include "CalfDpfBridge.hpp"
#include <calf/modules_tools.h>

START_NAMESPACE_DISTRHO

class CalfAnalyzerPlugin
    : public calf_dpf::CalfPluginBase<calf_plugins::analyzer_audio_module>
{
public:
    CalfAnalyzerPlugin() = default;

protected:
    const char* getDescription() const override
    {
        return "Calf Analyzer — FFT spectrum + waveform display.";
    }
    int64_t getUniqueId() const override
    {
        return d_cconst('c', 'A', 'n', 'l');
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

Plugin* createPlugin() { return new CalfAnalyzerPlugin(); }

END_NAMESPACE_DISTRHO
