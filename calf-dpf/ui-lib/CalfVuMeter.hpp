/*
 * CalfVuMeter — horizontal bar meter for PF_CTL_METER params.
 *
 * The bar fills proportionally to the parameter's normalized position
 * via parameter_properties::to_01 so log/gain-scaled meters (most of
 * them) display correctly. Green/yellow/red zones at 60% / 80% / 100%.
 * Output-only — ignores mouse input. PF_CTLO_REVERSE flag inverts fill
 * direction (used by compression-reduction meters in Calf).
 */
#ifndef CALF_VU_METER_HPP
#define CALF_VU_METER_HPP

#include "CalfWidgetBase.hpp"
#include <string>

START_NAMESPACE_DGL

class CalfVuMeter : public CalfWidgetBase
{
public:
    CalfVuMeter(NanoTopLevelWidget* parent,
                const calf_plugins::parameter_properties& props,
                uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(72, 96);
        fReverse = (props.flags & calf_plugins::PF_CTLO_REVERSE) != 0;
    }

protected:
    void onNanoDisplay() override
    {
        const float W = static_cast<float>(getWidth());
        const float H = static_cast<float>(getHeight());

        // Frame
        beginPath();
        rect(4, 30, W - 8, 24);
        fillColor(Color(0.08f, 0.08f, 0.09f));
        fill();

        // Fill bar
        double pos = fProps.to_01(fValue);
        if (pos < 0.0) pos = 0.0;
        if (pos > 1.0) pos = 1.0;
        if (fReverse) pos = 1.0 - pos;
        const float fillW = static_cast<float>(pos) * (W - 12);

        // Three colored segments under the fill mask.
        if (fillW > 0.5f) {
            const float green  = std::min(fillW, (W - 12) * 0.6f);
            const float yellow = std::min(fillW, (W - 12) * 0.8f) - green;
            const float red    = fillW - green - yellow;

            beginPath();
            rect(6, 32, green, 20);
            fillColor(Color(0.40f, 0.85f, 0.30f));
            fill();

            if (yellow > 0) {
                beginPath();
                rect(6 + green, 32, yellow, 20);
                fillColor(Color(0.95f, 0.80f, 0.20f));
                fill();
            }
            if (red > 0) {
                beginPath();
                rect(6 + green + yellow, 32, red, 20);
                fillColor(Color(0.95f, 0.30f, 0.20f));
                fill();
            }
        }

        // Label
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(11.0f);
        fillColor(Color(0.85f, 0.85f, 0.85f));
        textAlign(ALIGN_CENTER | ALIGN_TOP);
        text(W * 0.5f, 8, fProps.short_name ? fProps.short_name : fProps.name, nullptr);

        // Value
        std::string s = fProps.to_string(fValue);
        fontSize(10.0f);
        fillColor(Color(0.7f, 0.85f, 1.0f));
        text(W * 0.5f, 60, s.c_str(), nullptr);
    }

private:
    bool fReverse = false;
};

END_NAMESPACE_DGL

#endif
