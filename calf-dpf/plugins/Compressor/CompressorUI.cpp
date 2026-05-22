/*
 * Calf Compressor UI — minimal NanoVG layout.
 *
 * Lays out all 16 Calf compressor parameters as CalfKnob widgets in a
 * 4 × 4 grid. Output params (meters, clip LEDs, reduction) are visually
 * present but read-only — their value text updates from parameterChanged.
 *
 * This is intentionally a starter UI: no skinning, no fancy meters, no
 * graph. The next sub-phase swaps the meter params for a CalfVuMeter
 * widget and adds the compression-curve graph.
 */
#include "DistrhoUI.hpp"
#include "CalfKnob.hpp"

#include <calf/metadata.h>

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::CalfKnob;

class CompressorUI : public UI, public CalfKnob::Callback
{
public:
    static constexpr uint kCols = 4;
    static constexpr uint kRows = 4;
    static constexpr uint kKnobW = 72;
    static constexpr uint kKnobH = 96;
    static constexpr uint kPad  = 6;

    CompressorUI()
        : UI(kCols * kKnobW + 2 * kPad,
             kRows * kKnobH + 2 * kPad)
    {
        loadSharedResources();
        using namespace calf_plugins;
        compressor_metadata meta;
        for (uint32_t i = 0; i < compressor_metadata::param_count; ++i) {
            const parameter_properties* pp = meta.get_param_props(static_cast<int>(i));
            CalfKnob* k = new CalfKnob(this, *pp, i);
            const uint row = i / kCols;
            const uint col = i % kCols;
            k->setAbsolutePos(kPad + col * kKnobW, kPad + row * kKnobH);
            k->setCallback(this);
            fKnobs[i] = k;
        }
    }

    ~CompressorUI() override
    {
        for (CalfKnob* k : fKnobs) delete k;
    }

protected:
    void onNanoDisplay() override
    {
        // Background panel.
        beginPath();
        rect(0, 0, getWidth(), getHeight());
        fillColor(Color(0.12f, 0.12f, 0.14f));
        fill();
    }

    void parameterChanged(uint32_t index, float value) override
    {
        if (index < calf_plugins::compressor_metadata::param_count && fKnobs[index])
            fKnobs[index]->setValue(value, /*notify=*/false);
    }

    /* CalfKnob::Callback */
    void knobValueChanged(CalfKnob* knob, float value) override
    {
        editParameter(knob->getParamIndex(), true);
        setParameterValue(knob->getParamIndex(), value);
        editParameter(knob->getParamIndex(), false);
    }

private:
    CalfKnob* fKnobs[calf_plugins::compressor_metadata::param_count] = {nullptr};
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorUI)
};

UI* createUI() { return new CompressorUI(); }

END_NAMESPACE_DISTRHO
