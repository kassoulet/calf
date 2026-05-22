/*
 * Calf Compressor UI — per-parameter widget dispatch.
 *
 * Reads each parameter's PF_CTL_* hint and instantiates the matching
 * widget: knob, vu meter, LED, toggle, or combo. Layout is a 4×4 grid
 * in source-table order (matching how the legacy GTK UI presented
 * them). Future passes will move to the bespoke GTK XML layout via
 * codegen — for now the grid is sufficient to validate sync end-to-end.
 */
#include "DistrhoUI.hpp"
#include "CalfKnob.hpp"
#include "CalfVuMeter.hpp"
#include "CalfLed.hpp"
#include "CalfToggle.hpp"
#include "CalfComboBox.hpp"

#include <calf/metadata.h>

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::CalfWidgetBase;
using DGL_NAMESPACE::CalfKnob;
using DGL_NAMESPACE::CalfVuMeter;
using DGL_NAMESPACE::CalfLed;
using DGL_NAMESPACE::CalfToggle;
using DGL_NAMESPACE::CalfComboBox;

class CompressorUI : public UI, public CalfWidgetBase::Callback
{
public:
    static constexpr uint kCols  = 4;
    static constexpr uint kRows  = 4;
    static constexpr uint kCellW = 72;
    static constexpr uint kCellH = 96;
    static constexpr uint kPad   = 6;

    CompressorUI()
        : UI(kCols * kCellW + 2 * kPad,
             kRows * kCellH + 2 * kPad)
    {
        loadSharedResources();
        using namespace calf_plugins;
        compressor_metadata meta;
        for (uint32_t i = 0; i < compressor_metadata::param_count; ++i) {
            const parameter_properties* pp = meta.get_param_props(static_cast<int>(i));
            CalfWidgetBase* w = makeWidget(*pp, i);
            const uint row = i / kCols;
            const uint col = i % kCols;
            w->setAbsolutePos(kPad + col * kCellW, kPad + row * kCellH);
            w->setCallback(this);
            fWidgets[i] = w;
        }
    }

    ~CompressorUI() override
    {
        for (CalfWidgetBase* w : fWidgets) delete w;
    }

protected:
    void onNanoDisplay() override
    {
        beginPath();
        rect(0, 0, getWidth(), getHeight());
        fillColor(Color(0.12f, 0.12f, 0.14f));
        fill();
    }

    void parameterChanged(uint32_t index, float value) override
    {
        if (index < calf_plugins::compressor_metadata::param_count && fWidgets[index])
            fWidgets[index]->setValue(value, /*notify=*/false);
    }

    /* CalfWidgetBase::Callback */
    void widgetValueChanged(CalfWidgetBase* w, float v) override
    {
        editParameter(w->getParamIndex(), true);
        setParameterValue(w->getParamIndex(), v);
        editParameter(w->getParamIndex(), false);
    }

private:
    CalfWidgetBase* makeWidget(const calf_plugins::parameter_properties& pp,
                               uint32_t idx)
    {
        using namespace calf_plugins;
        switch (pp.flags & PF_CTLMASK) {
            case PF_CTL_METER:  return new CalfVuMeter(this, pp, idx);
            case PF_CTL_LED:    return new CalfLed    (this, pp, idx);
            case PF_CTL_TOGGLE: return new CalfToggle (this, pp, idx);
            case PF_CTL_COMBO:  return new CalfComboBox(this, pp, idx);
            case PF_CTL_KNOB:   /* fallthrough */
            default:            return new CalfKnob   (this, pp, idx);
        }
    }

    CalfWidgetBase* fWidgets[calf_plugins::compressor_metadata::param_count] = {nullptr};
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorUI)
};

UI* createUI() { return new CompressorUI(); }

END_NAMESPACE_DISTRHO
