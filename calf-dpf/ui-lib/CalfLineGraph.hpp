/*
 * CalfLineGraph — frequency-response / transfer-curve / spectrum widget.
 *
 * Calls into a calf_plugins::line_graph_iface (the same interface the
 * legacy GTK ctl_linegraph used) for curve samples. The DSP module is
 * typically a UI-side *shadow* of the running DSP — same class, same
 * params, but instantiated in the UI process so we can query it
 * synchronously during paint without crossing a thread/process boundary.
 *
 * Multi-curve plugins (e.g. compressor draws both the y=x reference line
 * and the actual transfer curve) loop subindex 0..N until get_graph()
 * returns false. The Calf side calls cairo_iface::set_source_rgba() and
 * set_line_width() between curves to express colour and stroke width;
 * CalfCairoCapture relays those into NanoVG draw state.
 *
 * With no source bound, the widget renders the empty-grid placeholder it
 * had before Phase 4d — codegen still works for plugins without graphs,
 * and plugins-with-graphs that haven't wired a shadow module yet still
 * draw something sensible.
 */
#ifndef CALF_LINE_GRAPH_HPP
#define CALF_LINE_GRAPH_HPP

#include "CalfWidgetBase.hpp"
#include <calf/giface.h>

START_NAMESPACE_DGL

// Minimal cairo_iface impl: just captures the color/stroke the DSP
// module wants for the *next* curve. set_dash and draw_label are
// intentionally no-ops — the dashed-reference-line look and per-tick
// labels are nice-to-haves that can land later.
struct CalfCairoCapture : public calf_plugins::cairo_iface
{
    float r = 0.85f, g = 0.85f, b = 0.85f, a = 1.0f;
    float lineWidth = 1.0f;

    void set_source_rgba(float r_, float g_, float b_, float a_ = 1.f) override
    { r = r_; g = g_; b = b_; a = a_; }
    void set_line_width(float w) override { lineWidth = w; }
    void set_dash(const double*, int) override {}
    void draw_label(const char*, float, float, int, float, float) override {}
};

class CalfLineGraph : public CalfWidgetBase
{
public:
    CalfLineGraph(NanoTopLevelWidget* parent,
                  const calf_plugins::parameter_properties& props,
                  uint32_t paramIndex,
                  int natW = 240, int natH = 160)
        : CalfWidgetBase(parent, props, paramIndex)
    {
        setSize(static_cast<uint>(natW), static_cast<uint>(natH));
    }

    /* Bind the widget to a (UI-side) Calf module that implements
     * line_graph_iface. graphIndex is the legacy "parameter port"
     * argument; for most graphs it's just 0 (or the index of the
     * parameter whose `graph="…"` hint references this widget). */
    void setGraphSource(const calf_plugins::line_graph_iface* src,
                        int graphIndex)
    {
        fSource     = src;
        fGraphIndex = graphIndex;
        repaint();
    }

    /* Call after the shadow module's params_changed() so the curve
     * redraws on the next paint cycle. */
    void refresh() { repaint(); }

protected:
    void onNanoDisplay() override
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());

        // Background + frame.
        beginPath();
        rect(0.5f, 0.5f, w - 1, h - 1);
        fillColor(Color(0.06f, 0.06f, 0.08f));
        fill();
        strokeColor(Color(0.30f, 0.30f, 0.35f));
        strokeWidth(1.0f);
        stroke();

        // Grid.
        strokeColor(Color(0.15f, 0.18f, 0.20f));
        for (int i = 1; i < 4; ++i) {
            beginPath();
            const float gy = h * i / 4.0f;
            moveTo(1, gy); lineTo(w - 1, gy); stroke();
            const float gx = w * i / 4.0f;
            beginPath();
            moveTo(gx, 1); lineTo(gx, h - 1); stroke();
        }

        if (!fSource) return;

        // Iterate curves until get_graph() reports no more.
        constexpr int kMaxPoints = 256;
        float data[kMaxPoints];
        for (int sub = 0; sub < 16; ++sub) {
            CalfCairoCapture ctx;
            int mode = 0;
            for (int i = 0; i < kMaxPoints; ++i) data[i] = 0.0f;

            // get_graph fills data with values in [-1, 1] (output range
            // is normalized). Returns false to stop iteration.
            const bool more = fSource->get_graph(
                fGraphIndex, sub, /*phase=*/0,
                data, kMaxPoints, &ctx, &mode);
            if (!more) break;

            // Plot the captured curve. Points are spread across the
            // widget; INFINITY values (per legacy code, used to denote
            // gaps / discontinuities) start a new subpath.
            beginPath();
            bool gap = true;
            for (int i = 0; i < kMaxPoints; ++i) {
                if (std::isfinite(data[i])) {
                    // data ∈ [-1, 1] → widget ∈ [h-1, 1] (flip Y).
                    const float x = 1.0f + (w - 2.0f) * i / (kMaxPoints - 1);
                    const float y = (h - 1.0f) - (data[i] + 1.0f) * 0.5f * (h - 2.0f);
                    if (gap) { moveTo(x, y); gap = false; }
                    else     { lineTo(x, y); }
                } else {
                    gap = true;
                }
            }
            strokeColor(Color(ctx.r, ctx.g, ctx.b, ctx.a));
            strokeWidth(ctx.lineWidth > 0.0f ? ctx.lineWidth : 1.5f);
            stroke();
        }
    }

private:
    const calf_plugins::line_graph_iface* fSource     = nullptr;
    int                                   fGraphIndex = 0;
};

END_NAMESPACE_DGL

#endif
