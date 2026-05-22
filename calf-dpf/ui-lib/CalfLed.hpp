/*
 * CalfLed — small indicator LED for PF_CTL_LED output params.
 *
 * Bool-ish: lit when value >= 0.5, dim otherwise. Output-only; the DSP
 * lights it (e.g. clip indicators).
 */
#ifndef CALF_LED_HPP
#define CALF_LED_HPP

#include "CalfWidgetBase.hpp"

START_NAMESPACE_DGL

class CalfLed : public CalfWidgetBase
{
public:
    CalfLed(NanoTopLevelWidget* parent,
            const calf_plugins::parameter_properties& props,
            uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(72, 96);
    }

protected:
    void onNanoDisplay() override
    {
        const float W  = static_cast<float>(getWidth());
        const float cx = W * 0.5f;
        const float cy = 36.0f;
        const bool  on = fValue >= 0.5f;

        // Outer bezel
        beginPath();
        circle(cx, cy, 12.0f);
        fillColor(Color(0.10f, 0.10f, 0.12f));
        fill();

        // Lamp
        beginPath();
        circle(cx, cy, 9.0f);
        fillColor(on ? Color(0.95f, 0.30f, 0.25f) : Color(0.25f, 0.10f, 0.10f));
        fill();

        if (on) {
            // Hot spot.
            beginPath();
            circle(cx - 2.5f, cy - 2.5f, 3.0f);
            fillColor(Color(1.0f, 0.85f, 0.85f, 0.7f));
            fill();
        }

        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(11.0f);
        fillColor(Color(0.85f, 0.85f, 0.85f));
        textAlign(ALIGN_CENTER | ALIGN_TOP);
        text(cx, 66.0f, fProps.short_name ? fProps.short_name : fProps.name, nullptr);
    }
};

END_NAMESPACE_DGL

#endif
