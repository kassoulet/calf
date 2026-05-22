/*
 * CalfToggle — boolean toggle button for PF_BOOL params.
 *
 * Click-to-flip. min/max in parameter_properties are usually 0/1; we
 * treat anything >= 0.5 as ON.
 */
#ifndef CALF_TOGGLE_HPP
#define CALF_TOGGLE_HPP

#include "CalfWidgetBase.hpp"

START_NAMESPACE_DGL

class CalfToggle : public CalfWidgetBase
{
public:
    CalfToggle(NanoTopLevelWidget* parent,
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
        const bool  on = fValue >= 0.5f;

        const float trackY = fShowLabels ? 26.0f : (H * 0.5f - 11.0f);

        beginPath();
        roundedRect(10, trackY, W - 20, 22, 11.0f);
        fillColor(on ? Color(0.20f, 0.50f, 0.30f) : Color(0.18f, 0.18f, 0.20f));
        fill();
        strokeColor(Color(0.08f, 0.08f, 0.09f));
        strokeWidth(1.0f);
        stroke();

        const float thumbX = on ? W - 30 : 12;
        beginPath();
        circle(thumbX + 9, trackY + 11, 8.0f);
        fillColor(on ? Color(0.85f, 0.95f, 0.80f) : Color(0.55f, 0.55f, 0.58f));
        fill();

        if (fShowLabels) {
            fontFace(NANOVG_DEJAVU_SANS_TTF);
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            text(W * 0.5f, 8, fProps.short_name ? fProps.short_name : fProps.name, nullptr);

            fontSize(9.0f);
            fillColor(Color(0.7f, 0.85f, 1.0f));
            text(W * 0.5f, 60, on ? "ON" : "OFF", nullptr);
        }
    }

    bool onMouse(const MouseEvent& ev) override
    {
        if (ev.button != 1 || !ev.press) return false;
        if (!contains(ev.pos)) return false;
        const float newV = fValue >= 0.5f ? fProps.min : fProps.max;
        setValue(newV, /*notify=*/true);
        return true;
    }

private:
    bool fShowLabels = true;
};

END_NAMESPACE_DGL

#endif
