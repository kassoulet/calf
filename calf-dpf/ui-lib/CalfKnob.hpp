/*
 * CalfKnob — rotary knob NanoVG widget.
 *
 * Uses Calf parameter_properties::to_01 / from_01 so log-scaled params
 * feel right under drag (200 px vertical = full sweep). Value text is
 * formatted by Calf's own to_string so units (dB, ms, %, x) print
 * consistently with the legacy GTK UI.
 */
#ifndef CALF_KNOB_HPP
#define CALF_KNOB_HPP

#include "CalfWidgetBase.hpp"
#include <string>

START_NAMESPACE_DGL

class CalfKnob : public CalfWidgetBase
{
public:
    CalfKnob(NanoTopLevelWidget* parent,
             const calf_plugins::parameter_properties& props,
             uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(72, 96);
    }

    /* Codegen-emitted UIs render label + value as separate widgets and
     * pass false here so the knob's own text doesn't overlap. */
    void setShowLabels(bool show) noexcept { fShowLabels = show; }

protected:
    void onNanoDisplay() override
    {
        const float w   = static_cast<float>(getWidth());
        const float h   = static_cast<float>(getHeight());
        const float cx  = w * 0.5f;
        const float cy  = fShowLabels ? 36.0f : h * 0.5f;
        const float r   = std::min(cx, cy) - 4.0f;

        const float pos01 = static_cast<float>(fProps.to_01(fValue));
        const float a0 = 0.75f * M_PI;
        const float a1 = 2.25f * M_PI;
        const float a  = a0 + (a1 - a0) * pos01;

        beginPath();
        arc(cx, cy, r, a0, a1, NanoVG::Winding::CW);
        strokeColor(Color(0.25f, 0.25f, 0.28f));
        strokeWidth(4.0f);
        stroke();

        beginPath();
        arc(cx, cy, r, a0, a, NanoVG::Winding::CW);
        strokeColor(Color(0.96f, 0.62f, 0.16f));
        strokeWidth(4.0f);
        stroke();

        const float ix = cx + std::cos(a) * r;
        const float iy = cy + std::sin(a) * r;
        beginPath();
        circle(ix, iy, 4.0f);
        fillColor(Color(1.0f, 0.85f, 0.5f));
        fill();

        if (fShowLabels) {
            fontFace(NANOVG_DEJAVU_SANS_TTF);
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            text(cx, 66.0f, fProps.short_name ? fProps.short_name : fProps.name, nullptr);

            std::string s = fProps.to_string(fValue);
            fontSize(10.0f);
            fillColor(Color(0.7f, 0.85f, 1.0f));
            text(cx, 80.0f, s.c_str(), nullptr);
        }
    }

    bool onMouse(const MouseEvent& ev) override
    {
        if (ev.button != 1) return false;
        if (ev.press && contains(ev.pos)) {
            fDragging = true;
            fDragStartY = ev.pos.getY();
            fDragStart01 = fProps.to_01(fValue);
            return true;
        }
        if (!ev.press && fDragging) {
            fDragging = false;
            return true;
        }
        return false;
    }

    bool onMotion(const MotionEvent& ev) override
    {
        if (!fDragging) return false;
        const float dy = static_cast<float>(fDragStartY - ev.pos.getY());
        double pos = fDragStart01 + (dy / 200.0);
        if (pos < 0.0) pos = 0.0;
        if (pos > 1.0) pos = 1.0;
        setValue(fProps.from_01(pos), /*notify=*/true);
        return true;
    }

    bool onScroll(const ScrollEvent& ev) override
    {
        if (!contains(ev.pos)) return false;
        double pos = fProps.to_01(fValue) + ev.delta.getY() * 0.02;
        if (pos < 0.0) pos = 0.0;
        if (pos > 1.0) pos = 1.0;
        setValue(fProps.from_01(pos), /*notify=*/true);
        return true;
    }

private:
    bool   fShowLabels  = true;
    bool   fDragging    = false;
    int    fDragStartY  = 0;
    double fDragStart01 = 0.0;
};

END_NAMESPACE_DGL

#endif
