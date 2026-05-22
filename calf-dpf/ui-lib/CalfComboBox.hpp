/*
 * CalfComboBox — enum selector for PF_ENUM | PF_CTL_COMBO params.
 *
 * Click cycles to the next enum value (wraps); scroll changes by ±1.
 * Renders the current choice's text label from
 * parameter_properties::choices.
 */
#ifndef CALF_COMBO_BOX_HPP
#define CALF_COMBO_BOX_HPP

#include "CalfWidgetBase.hpp"
#include <cmath>

START_NAMESPACE_DGL

class CalfComboBox : public CalfWidgetBase
{
public:
    CalfComboBox(NanoTopLevelWidget* parent,
                 const calf_plugins::parameter_properties& props,
                 uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(72, 96);
        fCount = 0;
        if (props.choices)
            for (const char** p = props.choices; *p; ++p) ++fCount;
    }

    void setShowLabels(bool show) noexcept { fShowLabels = show; }

protected:
    void onNanoDisplay() override
    {
        const float W = static_cast<float>(getWidth());
        const float H = static_cast<float>(getHeight());
        const float frameY = fShowLabels ? 28.0f : (H * 0.5f - 13.0f);

        beginPath();
        roundedRect(4, frameY, W - 8, 26, 3.0f);
        fillColor(Color(0.16f, 0.16f, 0.18f));
        fill();
        strokeColor(Color(0.40f, 0.40f, 0.45f));
        strokeWidth(1.0f);
        stroke();

        beginPath();
        moveTo(W - 14, frameY + 10);
        lineTo(W - 8,  frameY + 10);
        lineTo(W - 11, frameY + 16);
        closePath();
        fillColor(Color(0.85f, 0.85f, 0.85f));
        fill();

        const int idx = static_cast<int>(std::round(fValue - fProps.min));
        const char* label = (fProps.choices && idx >= 0 && idx < fCount)
                              ? fProps.choices[idx]
                              : "";
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(10.0f);
        fillColor(Color(0.95f, 0.95f, 1.0f));
        textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
        text(W * 0.5f - 4, frameY + 13, label, nullptr);

        if (fShowLabels) {
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            text(W * 0.5f, 8, fProps.short_name ? fProps.short_name : fProps.name, nullptr);
        }
    }

    bool onMouse(const MouseEvent& ev) override
    {
        if (ev.button != 1 || !ev.press) return false;
        if (!contains(ev.pos)) return false;
        cycle(+1);
        return true;
    }

    bool onScroll(const ScrollEvent& ev) override
    {
        if (!contains(ev.pos)) return false;
        cycle(ev.delta.getY() > 0 ? +1 : -1);
        return true;
    }

private:
    void cycle(int delta)
    {
        if (fCount <= 0) return;
        int idx = static_cast<int>(std::round(fValue - fProps.min)) + delta;
        if (idx < 0)        idx = fCount - 1;
        if (idx >= fCount)  idx = 0;
        setValue(fProps.min + static_cast<float>(idx), /*notify=*/true);
    }

    int  fCount;
    bool fShowLabels = true;
};

END_NAMESPACE_DGL

#endif
