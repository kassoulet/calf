/*
 * CalfToggle — boolean toggle button for PF_BOOL params.
 *
 * Click-to-flip. min/max in parameter_properties are usually 0/1; we
 * treat anything >= 0.5 as ON. The legacy GTK widget rendered the
 * toggle as a 100×160 bitmap with the off state in the top half
 * (0..80) and the on state in the bottom half (80..160); some XMLs
 * select a different icon via icon="bypass"/"mute"/… and we honour
 * that with the matching `toggle_2_<icon>.png` asset.
 */
#ifndef CALF_TOGGLE_HPP
#define CALF_TOGGLE_HPP

#include "CalfWidgetBase.hpp"
#include "CalfTheme.hpp"
#include <string>

START_NAMESPACE_DGL

class CalfToggle : public CalfWidgetBase
{
public:
    CalfToggle(NanoTopLevelWidget* parent,
               const calf_plugins::parameter_properties& props,
               uint32_t paramIndex)
        : CalfWidgetBase(parent, props, paramIndex),
          fTheme(*static_cast<NanoVG*>(this))
    {
        setSize(50, 40);
    }

    void setShowLabels(bool show) noexcept { fShowLabels = show; }

    /* GTK toggle icon hint from the XML, e.g. "bypass", "mute".
     * Empty / unknown falls back to the generic toggle_2.png art. */
    void setIcon(const char* icon) noexcept
    {
        if (icon && *icon) fIcon = icon;
        else               fIcon.clear();
    }

protected:
    void onNanoDisplay() override
    {
        const float W  = static_cast<float>(getWidth());
        const float H  = static_cast<float>(getHeight());
        const bool  on = fValue >= 0.5f;

        NanoImage* img = nullptr;
        if (!fIcon.empty()) {
            std::string name = "toggle_2_" + fIcon + ".png";
            img = fTheme.image(name.c_str());
        }
        if (img == nullptr || !img->isValid())
            img = fTheme.image("toggle_2.png");

        if (img && img->isValid()) {
            // Source bitmap: 100 wide × 160 tall, off=top 80, on=bottom 80.
            // imagePattern's (ox,oy,ex,ey) define the unscaled image
            // origin and natural extent; the fill rect picks the
            // displayed region. To show the off (or on) half scaled to
            // the widget, we set extent = (W, 2H) — twice the widget
            // height — and translate origin by -H to grab the lower
            // half when on.
            const float oy = on ? -H : 0.0f;
            beginPath();
            rect(0, 0, W, H);
            fillPaint(imagePattern(0.0f, oy, W, H * 2.0f, 0.0f, *img, 1.0f));
            fill();
        } else {
            // Fallback to a flat pill if the asset is missing.
            beginPath();
            roundedRect(2, 2, W - 4, H - 4, std::min(H, W) * 0.4f);
            fillColor(on ? Color(0.20f, 0.50f, 0.30f) : Color(0.18f, 0.18f, 0.20f));
            fill();
        }

        if (fShowLabels) {
            fontFace(NANOVG_DEJAVU_SANS_TTF);
            fontSize(11.0f);
            fillColor(Color(0.85f, 0.85f, 0.85f));
            textAlign(ALIGN_CENTER | ALIGN_BOTTOM);
            text(W * 0.5f, H - 2, on ? "ON" : "OFF", nullptr);
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
    CalfTheme   fTheme;
    std::string fIcon;
    bool        fShowLabels = true;
};

END_NAMESPACE_DGL

#endif
