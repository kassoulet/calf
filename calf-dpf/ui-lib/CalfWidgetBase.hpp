/*
 * CalfWidgetBase — common base for all Calf NanoVG widgets.
 *
 * Holds the parameter_properties reference + index, exposes a Callback
 * for value changes, normalizes setValue()/getValue(), and pre-loads
 * DPF's shared font. Per-widget subclasses implement onNanoDisplay()
 * and input handlers.
 *
 * Output-only widgets (VU meter, LED) ignore mouse input and just
 * reflect parameter changes pushed via setValue with notify=false.
 */
#ifndef CALF_WIDGET_BASE_HPP
#define CALF_WIDGET_BASE_HPP

#include "NanoVG.hpp"
#include "SubWidget.hpp"
#include <calf/giface.h>
#include <cmath>

START_NAMESPACE_DGL

class CalfWidgetBase : public NanoSubWidget
{
public:
    struct Callback {
        virtual ~Callback() = default;
        virtual void widgetValueChanged(CalfWidgetBase* w, float v) = 0;
    };

    CalfWidgetBase(NanoTopLevelWidget* parent,
                   const calf_plugins::parameter_properties& props,
                   uint32_t paramIndex)
        : NanoSubWidget(parent),
          fProps(props),
          fIndex(paramIndex),
          fValue(props.def_value),
          fCallback(nullptr)
    {
        loadSharedResources();
    }

    void setCallback(Callback* cb) noexcept { fCallback = cb; }

    void setValue(float v, bool notify)
    {
        if (v < fProps.min) v = fProps.min;
        if (v > fProps.max) v = fProps.max;
        if (std::fabs(v - fValue) < 1e-9f) return;
        fValue = v;
        if (notify && fCallback) fCallback->widgetValueChanged(this, v);
        repaint();
    }

    float    getValue()      const noexcept { return fValue; }
    uint32_t getParamIndex() const noexcept { return fIndex; }
    const calf_plugins::parameter_properties& getProps() const noexcept { return fProps; }

protected:
    const calf_plugins::parameter_properties& fProps;
    const uint32_t fIndex;
    float          fValue;
    Callback*      fCallback;
};

END_NAMESPACE_DGL

#endif
