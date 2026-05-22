/*
 * CalfLineGraph — placeholder for the legacy ctl_linegraph widget.
 *
 * The real widget (1546 LOC in src/ctl_linegraph.cpp) hosts EQ curves,
 * compressor transfer curves, FFT analyzer, and interactive frequency
 * handles. This stub just draws an empty grid-on-black rectangle so
 * codegen-emitted UIs that reference <line-graph> compile and lay out.
 * Real rendering will land later in Phase 4 once the line_graph_iface
 * → DPF channel is in place.
 */
#ifndef CALF_LINE_GRAPH_HPP
#define CALF_LINE_GRAPH_HPP

#include "CalfWidgetBase.hpp"

START_NAMESPACE_DGL

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

protected:
    void onNanoDisplay() override
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());

        beginPath();
        rect(0.5f, 0.5f, w - 1, h - 1);
        fillColor(Color(0.06f, 0.06f, 0.08f));
        fill();
        strokeColor(Color(0.30f, 0.30f, 0.35f));
        strokeWidth(1.0f);
        stroke();

        strokeColor(Color(0.15f, 0.18f, 0.20f));
        for (int i = 1; i < 4; ++i) {
            beginPath();
            const float gy = h * i / 4.0f;
            moveTo(1, gy); lineTo(w - 1, gy); stroke();
            const float gx = w * i / 4.0f;
            beginPath();
            moveTo(gx, 1); lineTo(gx, h - 1); stroke();
        }
    }
};

END_NAMESPACE_DGL

#endif
