/*
 * CalfLabel — static or param-bound text label.
 *
 * When created with a parameter, displays props.name (or short_name as
 * fallback). When created with explicit text, displays that. Output-only —
 * ignores mouse input. The Calf XML pattern attaches a label next to each
 * knob/value/combo trio, so this widget needs to be small and unobtrusive.
 */
#ifndef CALF_LABEL_HPP
#define CALF_LABEL_HPP

#include "CalfWidgetBase.hpp"
#include <string>

START_NAMESPACE_DGL

class CalfLabel : public CalfWidgetBase
{
public:
    CalfLabel(NanoTopLevelWidget* parent,
              const calf_plugins::parameter_properties& props,
              uint32_t paramIndex,
              const char* explicitText = nullptr)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        if (explicitText && *explicitText) {
            fText = explicitText;
        } else {
            fText = props.name ? props.name
                               : (props.short_name ? props.short_name : "");
        }
        setSize(60, 14);
    }

protected:
    void onNanoDisplay() override
    {
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        fontSize(11.0f);
        fillColor(Color(0.85f, 0.85f, 0.85f));
        textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
        text(getWidth() * 0.5f, getHeight() * 0.5f, fText.c_str(), nullptr);
    }

private:
    std::string fText;
};

END_NAMESPACE_DGL

#endif
