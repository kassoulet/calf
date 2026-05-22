/*
 * Calf DPF port — Hello plugin.
 *
 * Stereo unity-gain passthrough with a single "gain" parameter.
 * Exists only to validate the DPF build pipeline; will be deleted
 * once at least one real Calf module has landed.
 */

#include "DistrhoPlugin.hpp"

#include <cmath>

START_NAMESPACE_DISTRHO

class CalfHelloPlugin : public Plugin
{
public:
    CalfHelloPlugin()
        : Plugin(1, 0, 0),
          fGainDb(0.0f)
    {}

protected:
    const char* getLabel()       const override { return "CalfHello"; }
    const char* getDescription() const override { return "Calf DPF port sanity-check plugin."; }
    const char* getMaker()       const override { return "Calf Studio Gear"; }
    const char* getHomePage()    const override { return "https://calf-studio-gear.org/"; }
    const char* getLicense()     const override { return "GPL-2.0-only"; }
    uint32_t    getVersion()     const override { return d_version(0, 91, 0); }
    int64_t     getUniqueId()    const override { return d_cconst('c', 'H', 'e', 'l'); }

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        if (index != 0)
            return;
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Gain";
        parameter.symbol     = "gain";
        parameter.unit       = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -24.0f;
        parameter.ranges.max = 24.0f;
    }

    float getParameterValue(uint32_t index) const override
    {
        return index == 0 ? fGainDb : 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override
    {
        if (index == 0)
            fGainDb = value;
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float g = std::pow(10.0f, fGainDb * 0.05f);
        for (uint32_t ch = 0; ch < 2; ++ch)
        {
            const float* in  = inputs[ch];
            float*       out = outputs[ch];
            for (uint32_t i = 0; i < frames; ++i)
                out[i] = in[i] * g;
        }
    }

private:
    float fGainDb;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CalfHelloPlugin)
};

Plugin* createPlugin() { return new CalfHelloPlugin(); }

END_NAMESPACE_DISTRHO
