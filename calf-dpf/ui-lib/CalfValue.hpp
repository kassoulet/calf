/*
 * CalfValue — read-only numeric value display for a parameter.
 *
 * Renders props.to_string(value) so units (Hz, dB, ms, %, x) print the
 * same way the legacy GTK UI did. Output-only — ignores mouse input.
 * Updated by parameterChanged() pushing setValue(..., notify=false).
 */
#ifndef CALF_VALUE_HPP
#define CALF_VALUE_HPP

#include "CalfWidgetBase.hpp"
#include <string>

START_NAMESPACE_DGL

class CalfValue : public CalfWidgetBase
{
public:
    CalfValue(NanoTopLevelWidget* parent,
              const calf_plugins::parameter_properties& props,
              uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(60, 14);
    }

protected:
    void onNanoDisplay() override
    {
        std::string s = fProps.to_string(fValue);
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(10.0f);
        fillColor(Color(0.7f, 0.85f, 1.0f));
        textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
        text(getWidth() * 0.5f, getHeight() * 0.5f, s.c_str(), nullptr);
    }
};

END_NAMESPACE_DGL

#endif
