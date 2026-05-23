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

    void setShowLabels(bool show) noexcept { fShowLabels = show; }

protected:
    void onNanoDisplay() override
    {
        const float W  = static_cast<float>(getWidth());
        const float H  = static_cast<float>(getHeight());
        const float cx = W * 0.5f;
        const float cy = fShowLabels ? 36.0f : H * 0.5f;
        const bool  on = fValue >= 0.5f;

        // Outer bezel: subtle radial gradient for a metallic ring.
        beginPath();
        circle(cx, cy, 12.0f);
        fillPaint(radialGradient(cx - 2.0f, cy - 2.0f, 4.0f, 14.0f,
                                 Color(0.35f, 0.35f, 0.38f),
                                 Color(0.08f, 0.08f, 0.10f)));
        fill();
        beginPath();
        circle(cx, cy, 12.0f);
        strokeColor(Color(0.04f, 0.04f, 0.05f));
        strokeWidth(0.8f);
        stroke();

        // Lamp + halo. When lit, paint a soft outer glow over the
        // bezel for a believable backlit feel.
        if (on) {
            beginPath();
            circle(cx, cy, 16.0f);
            fillPaint(radialGradient(cx, cy, 6.0f, 18.0f,
                                     Color(0.95f, 0.30f, 0.25f, 0.55f),
                                     Color(0.95f, 0.30f, 0.25f, 0.0f)));
            fill();
        }

        beginPath();
        circle(cx, cy, 9.0f);
        if (on) {
            fillPaint(radialGradient(cx - 1.5f, cy - 1.5f, 1.0f, 10.0f,
                                     Color(1.0f, 0.95f, 0.85f),
                                     Color(0.85f, 0.18f, 0.12f)));
        } else {
            fillPaint(radialGradient(cx - 1.5f, cy - 1.5f, 1.0f, 10.0f,
                                     Color(0.32f, 0.14f, 0.13f),
                                     Color(0.14f, 0.06f, 0.06f)));
        }
        fill();

        if (on) {
            // Specular hot spot.
            beginPath();
            circle(cx - 2.5f, cy - 2.5f, 2.5f);
            fillColor(Color(1.0f, 0.95f, 0.90f, 0.75f));
            fill();
        }

        if (fShowLabels) {
            fontFace(NANOVG_DEJAVU_SANS_TTF);
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            text(cx, 66.0f, fProps.short_name ? fProps.short_name : fProps.name, nullptr);
        }
    }

private:
    bool fShowLabels = true;
};

END_NAMESPACE_DGL

#endif
