/*
 * CalfKnob — a single-parameter rotary knob, NanoVG-rendered.
 *
 * Tracks DPF parameter min/max/log/enum hints from a calf
 * parameter_properties row so the visual sweep matches what the
 * original GTK ctl_knob did. The widget stores the *natural* value
 * (same domain as Calf's float, e.g. milliseconds, dB-as-gain, Hz)
 * — conversion to/from 0..1 sweep position uses
 * parameter_properties::to_01 / from_01 so log-scaled params feel
 * right under drag.
 *
 * Used by per-plugin UI subclasses in plugins/<Name>/.
 */
#ifndef CALF_KNOB_HPP
#define CALF_KNOB_HPP

#include "NanoVG.hpp"
#include "SubWidget.hpp"
#include <calf/giface.h>
#include <cmath>
#include <cstdio>
#include <string>

START_NAMESPACE_DGL

class CalfKnob : public NanoSubWidget
{
public:
    struct Callback {
        virtual ~Callback() = default;
        virtual void knobValueChanged(CalfKnob* knob, float value) = 0;
    };

    CalfKnob(NanoTopLevelWidget* parent,
             const calf_plugins::parameter_properties& props,
             uint32_t paramIndex)
        : NanoSubWidget(parent),
          fProps(props),
          fIndex(paramIndex),
          fValue(props.def_value),
          fCallback(nullptr)
    {
        setSize(72, 96);  // 72 wide × 96 tall (knob + label band)
        loadSharedResources();
    }

    void setCallback(Callback* cb) noexcept { fCallback = cb; }

    void setValue(float v, bool notify)
    {
        if (v < fProps.min) v = fProps.min;
        if (v > fProps.max) v = fProps.max;
        if (std::fabs(v - fValue) < 1e-9f) return;
        fValue = v;
        if (notify && fCallback) fCallback->knobValueChanged(this, v);
        repaint();
    }

    float    getValue()      const noexcept { return fValue; }
    uint32_t getParamIndex() const noexcept { return fIndex; }

protected:
    void onNanoDisplay() override
    {
        const float w   = static_cast<float>(getWidth());
        const float cx  = w * 0.5f;
        const float cy  = 36.0f;
        const float r   = 26.0f;

        const float pos01 = static_cast<float>(fProps.to_01(fValue));
        // Sweep from 7 o'clock to 5 o'clock (135° → 405°).
        const float a0 = 0.75f * M_PI;
        const float a1 = 2.25f * M_PI;
        const float a  = a0 + (a1 - a0) * pos01;

        // Track arc (dim).
        beginPath();
        arc(cx, cy, r, a0, a1, NanoVG::Winding::CW);
        strokeColor(Color(0.25f, 0.25f, 0.28f));
        strokeWidth(4.0f);
        stroke();

        // Active arc (filled portion).
        beginPath();
        arc(cx, cy, r, a0, a, NanoVG::Winding::CW);
        strokeColor(Color(0.96f, 0.62f, 0.16f));
        strokeWidth(4.0f);
        stroke();

        // Indicator dot at the end of the active arc.
        const float ix = cx + std::cos(a) * r;
        const float iy = cy + std::sin(a) * r;
        beginPath();
        circle(ix, iy, 4.0f);
        fillColor(Color(1.0f, 0.85f, 0.5f));
        fill();

        // Label (param name).
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(11.0f);
        fillColor(Color(0.85f, 0.85f, 0.85f));
        textAlign(ALIGN_CENTER | ALIGN_TOP);
        text(cx, 66.0f, fProps.short_name ? fProps.short_name : fProps.name, nullptr);

        // Value text.
        std::string s = fProps.to_string(fValue);
        fontSize(10.0f);
        fillColor(Color(0.7f, 0.85f, 1.0f));
        text(cx, 80.0f, s.c_str(), nullptr);
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
        // 200 px of drag = full sweep.
        double pos = fDragStart01 + (dy / 200.0);
        if (pos < 0.0) pos = 0.0;
        if (pos > 1.0) pos = 1.0;
        float v = fProps.from_01(pos);
        setValue(v, /*notify=*/true);
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
    const calf_plugins::parameter_properties& fProps;
    const uint32_t fIndex;
    float          fValue;
    Callback*      fCallback;
    bool           fDragging   = false;
    int            fDragStartY = 0;
    double         fDragStart01 = 0.0;
};

END_NAMESPACE_DGL

#endif
