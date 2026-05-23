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

    /* Codegen-emitted UIs render the param label as a separate widget. */
    void setShowLabels(bool show) noexcept { fShowLabels = show; }

protected:
    void onNanoDisplay() override
    {
        const float W = static_cast<float>(getWidth());
        const float H = static_cast<float>(getHeight());

        // Bar takes the centre band; leave room above/below for labels when shown.
        const float topReserve = fShowLabels ? 22.0f : 4.0f;
        const float botReserve = fShowLabels ? 30.0f : 4.0f;
        const float barX = 4.0f;
        const float barY = topReserve;
        const float barW = std::max(0.0f, W - 8.0f);
        const float barH = std::max(0.0f, H - topReserve - botReserve);

        // Recessed bezel: dark gradient + subtle outer rim. Mirrors the
        // legacy GTK look where the meter sits in a sunken panel.
        beginPath();
        roundedRect(barX, barY, barW, barH, 2.0f);
        fillPaint(linearGradient(barX, barY, barX, barY + barH,
                                 Color(0.04f, 0.04f, 0.05f),
                                 Color(0.10f, 0.10f, 0.12f)));
        fill();
        beginPath();
        roundedRect(barX + 0.5f, barY + 0.5f, barW - 1.0f, barH - 1.0f, 2.0f);
        strokeColor(Color(0.0f, 0.0f, 0.0f, 0.6f));
        strokeWidth(1.0f);
        stroke();

        double pos = fProps.to_01(fValue);
        if (pos < 0.0) pos = 0.0;
        if (pos > 1.0) pos = 1.0;
        if (fReverse) pos = 1.0 - pos;
        const float innerW = std::max(0.0f, barW - 4.0f);
        const float innerH = std::max(0.0f, barH - 4.0f);
        const float fillW  = static_cast<float>(pos) * innerW;

        if (fillW > 0.5f) {
            // Continuous LED-style gradient: green → yellow → red over
            // the whole 0..1 span, then scissored to the actual fillW.
            // imagePattern-less linear gradient gives that backlit look
            // without needing a dedicated PNG strip.
            const float gx = barX + 2;
            const float gy = barY + 2;
            beginPath();
            rect(gx, gy, fillW, innerH);
            // Two-stop NanoVG gradients only, so paint three overlapping
            // segments to fake the three-stop ramp.
            const float green  = std::min(fillW, innerW * 0.6f);
            const float yellow = std::min(fillW, innerW * 0.8f) - std::min(fillW, innerW * 0.6f);
            const float red    = fillW - std::min(fillW, innerW * 0.8f);
            fillPaint(linearGradient(gx, gy, gx + green, gy,
                                     Color(0.18f, 0.55f, 0.20f),
                                     Color(0.55f, 0.95f, 0.40f)));
            fill();
            if (yellow > 0) {
                beginPath();
                rect(gx + green, gy, yellow, innerH);
                fillPaint(linearGradient(gx + green, gy, gx + green + yellow, gy,
                                         Color(0.85f, 0.75f, 0.20f),
                                         Color(0.98f, 0.90f, 0.35f)));
                fill();
            }
            if (red > 0) {
                beginPath();
                rect(gx + green + yellow, gy, red, innerH);
                fillPaint(linearGradient(gx + green + yellow, gy, gx + fillW, gy,
                                         Color(0.85f, 0.20f, 0.12f),
                                         Color(1.0f, 0.45f, 0.30f)));
                fill();
            }

            // Glass highlight across the top third of the filled band.
            beginPath();
            rect(gx, gy, fillW, innerH * 0.45f);
            fillPaint(linearGradient(gx, gy, gx, gy + innerH * 0.45f,
                                     Color(1.0f, 1.0f, 1.0f, 0.28f),
                                     Color(1.0f, 1.0f, 1.0f, 0.0f)));
            fill();
        }

        if (fShowLabels) {
            fontFace(NANOVG_DEJAVU_SANS_TTF);
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            text(W * 0.5f, 4, fProps.short_name ? fProps.short_name : fProps.name, nullptr);

            std::string s = fProps.to_string(fValue);
            fontSize(10.0f);
            fillColor(Color(0.7f, 0.85f, 1.0f));
            text(W * 0.5f, H - 16, s.c_str(), nullptr);
        }
    }

private:
    bool fReverse    = false;
    bool fShowLabels = true;
};

END_NAMESPACE_DGL

#endif
