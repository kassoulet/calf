/*
 * CalfLayout — tiny GTK-style packing solver for codegen-emitted UIs.
 *
 * Mirrors the subset of GTK2 box/table semantics that Calf's gui XML
 * actually uses: HBox/VBox with expand+fill+spacing+padding, Table with
 * attach-x/y/w/h plus per-axis homogeneous flag, Align with align+scale,
 * and a Frame that draws a labelled border in its own paint pass.
 *
 * Tree is built once at UI construction (codegen emits add() calls), then
 * place() runs top-down to assign absolute positions+sizes to leaf widgets
 * via DGL's setAbsolutePos/setSize. Containers are non-rendering except
 * Frame, whose draw() the UI invokes from onNanoDisplay().
 */
#ifndef CALF_LAYOUT_HPP
#define CALF_LAYOUT_HPP

#include "CalfWidgetBase.hpp"
#include "NanoVG.hpp"
#include <algorithm>
#include <memory>
#include <vector>

START_NAMESPACE_DGL

struct CalfPacking {
    bool  expandX = true;
    bool  expandY = true;
    bool  fillX   = true;
    bool  fillY   = true;
    int   padX    = 0;
    int   padY    = 0;
    int   attachX = 0;
    int   attachY = 0;
    int   attachW = 1;
    int   attachH = 1;
};

class CalfLayoutItem
{
public:
    virtual ~CalfLayoutItem() = default;
    virtual int  minW() const = 0;
    virtual int  minH() const = 0;
    virtual void place(int x, int y, int w, int h) = 0;
    virtual void draw(NanoVG& /*vg*/) {}
};

class CalfWidgetItem : public CalfLayoutItem
{
public:
    CalfWidgetItem(CalfWidgetBase* widget, int natW = -1, int natH = -1)
        : fWidget(widget),
          fMinW(natW < 0 ? static_cast<int>(widget->getWidth())  : natW),
          fMinH(natH < 0 ? static_cast<int>(widget->getHeight()) : natH) {}

    int  minW() const override { return fMinW; }
    int  minH() const override { return fMinH; }

    void place(int x, int y, int w, int h) override
    {
        fWidget->setAbsolutePos(x, y);
        fWidget->setSize(static_cast<uint>(std::max(1, w)),
                         static_cast<uint>(std::max(1, h)));
    }

private:
    CalfWidgetBase* fWidget;
    int fMinW, fMinH;
};

class CalfBoxItem : public CalfLayoutItem
{
public:
    enum Orientation { HORIZONTAL, VERTICAL };

    CalfBoxItem(Orientation o, int spacing = 0, bool homogeneous = false)
        : fOrient(o), fSpacing(spacing), fHomogeneous(homogeneous) {}

    void add(std::unique_ptr<CalfLayoutItem> item, CalfPacking pack)
    {
        fChildren.push_back({std::move(item), pack});
    }

    int minW() const override
    {
        if (fChildren.empty()) return 0;
        if (fOrient == HORIZONTAL) {
            int sum = fSpacing * (static_cast<int>(fChildren.size()) - 1);
            for (const auto& c : fChildren) sum += c.item->minW() + 2*c.pack.padX;
            return sum;
        } else {
            int mx = 0;
            for (const auto& c : fChildren) mx = std::max(mx, c.item->minW() + 2*c.pack.padX);
            return mx;
        }
    }

    int minH() const override
    {
        if (fChildren.empty()) return 0;
        if (fOrient == VERTICAL) {
            int sum = fSpacing * (static_cast<int>(fChildren.size()) - 1);
            for (const auto& c : fChildren) sum += c.item->minH() + 2*c.pack.padY;
            return sum;
        } else {
            int mx = 0;
            for (const auto& c : fChildren) mx = std::max(mx, c.item->minH() + 2*c.pack.padY);
            return mx;
        }
    }

    void place(int x, int y, int w, int h) override
    {
        const int N = static_cast<int>(fChildren.size());
        if (N == 0) return;

        const bool horiz = (fOrient == HORIZONTAL);
        const int  total = horiz ? w : h;
        const int  fixed = fSpacing * (N - 1);

        // Sum minimum sizes on the main axis (incl. padding).
        int minSum = 0;
        int expandCount = 0;
        for (const auto& c : fChildren) {
            const int m = horiz ? (c.item->minW() + 2*c.pack.padX)
                                : (c.item->minH() + 2*c.pack.padY);
            minSum += m;
            const bool exp = horiz ? c.pack.expandX : c.pack.expandY;
            if (exp) ++expandCount;
        }

        int extra = std::max(0, total - fixed - minSum);

        // Homogeneous = equal cells; ignore individual mins for size, keep alignment.
        std::vector<int> sizes(N);
        if (fHomogeneous) {
            const int cell = std::max(0, (total - fixed) / N);
            for (int i = 0; i < N; ++i) sizes[i] = cell;
        } else {
            // Distribute extra among expanders.
            const int per = expandCount > 0 ? (extra / expandCount) : 0;
            int rem      = expandCount > 0 ? (extra - per*expandCount) : 0;
            for (int i = 0; i < N; ++i) {
                const auto& c = fChildren[i];
                int m = horiz ? (c.item->minW() + 2*c.pack.padX)
                              : (c.item->minH() + 2*c.pack.padY);
                bool exp = horiz ? c.pack.expandX : c.pack.expandY;
                sizes[i] = m + (exp ? per + (rem > 0 ? (rem--, 1) : 0) : 0);
            }
        }

        int cursor = horiz ? x : y;
        for (int i = 0; i < N; ++i) {
            auto& c = fChildren[i];
            const int slot = sizes[i];
            int cx, cy, cw, ch;
            if (horiz) {
                cx = cursor + c.pack.padX;
                cy = y      + c.pack.padY;
                cw = slot   - 2*c.pack.padX;
                ch = h      - 2*c.pack.padY;
            } else {
                cx = x      + c.pack.padX;
                cy = cursor + c.pack.padY;
                cw = w      - 2*c.pack.padX;
                ch = slot   - 2*c.pack.padY;
            }

            // fill=0 collapses the child to its min size and centers within the slot.
            if (horiz && !c.pack.fillX) {
                int mw = c.item->minW();
                cx += (cw - mw) / 2; cw = mw;
            }
            if (!horiz && !c.pack.fillY) {
                int mh = c.item->minH();
                cy += (ch - mh) / 2; ch = mh;
            }
            // Cross-axis fill: if !fill, collapse to min.
            if (horiz && !c.pack.fillY) {
                int mh = c.item->minH();
                cy += (ch - mh) / 2; ch = mh;
            }
            if (!horiz && !c.pack.fillX) {
                int mw = c.item->minW();
                cx += (cw - mw) / 2; cw = mw;
            }

            c.item->place(cx, cy, std::max(0, cw), std::max(0, ch));
            cursor += slot + fSpacing;
        }
    }

    void draw(NanoVG& vg) override
    {
        for (auto& c : fChildren) c.item->draw(vg);
    }

private:
    struct Entry { std::unique_ptr<CalfLayoutItem> item; CalfPacking pack; };
    Orientation        fOrient;
    int                fSpacing;
    bool               fHomogeneous;
    std::vector<Entry> fChildren;
};

class CalfTableItem : public CalfLayoutItem
{
public:
    CalfTableItem(int rows, int cols, bool homogeneous = false,
                  int spacingX = 0, int spacingY = 0)
        : fRows(rows), fCols(cols), fHomogeneous(homogeneous),
          fSpacingX(spacingX), fSpacingY(spacingY) {}

    void add(std::unique_ptr<CalfLayoutItem> item, CalfPacking pack)
    {
        fChildren.push_back({std::move(item), pack});
    }

    /* GTK's gtk_table auto-grows when an attach exceeds the declared
     * rows/cols (some Calf XMLs declare rows="1" but attach-y up to 3).
     * Mirror that here so minH / minW / place() agree on the row+col
     * count actually in use. */
    int effRows() const
    {
        int r = fRows;
        for (const auto& c : fChildren)
            r = std::max(r, c.pack.attachY + c.pack.attachH);
        return std::max(1, r);
    }
    int effCols() const
    {
        int c = fCols;
        for (const auto& ch : fChildren)
            c = std::max(c, ch.pack.attachX + ch.pack.attachW);
        return std::max(1, c);
    }

    int minW() const override
    {
        const int cols = effCols();
        std::vector<int> colMin(cols, 0);
        for (const auto& c : fChildren) {
            if (c.pack.attachW != 1) continue;
            const int col = c.pack.attachX;
            if (col < 0 || col >= cols) continue;
            colMin[col] = std::max(colMin[col], c.item->minW() + 2*c.pack.padX);
        }
        int sum = fSpacingX * std::max(0, cols - 1);
        for (int v : colMin) sum += v;
        return sum;
    }

    int minH() const override
    {
        const int rows = effRows();
        std::vector<int> rowMin(rows, 0);
        for (const auto& c : fChildren) {
            if (c.pack.attachH != 1) continue;
            const int row = c.pack.attachY;
            if (row < 0 || row >= rows) continue;
            rowMin[row] = std::max(rowMin[row], c.item->minH() + 2*c.pack.padY);
        }
        int sum = fSpacingY * std::max(0, rows - 1);
        for (int v : rowMin) sum += v;
        return sum;
    }

    void place(int x, int y, int w, int h) override
    {
        const int cols = effCols();
        const int rows = effRows();
        std::vector<int> colW(cols, 0), rowH(rows, 0);

        if (fHomogeneous) {
            const int cw = (w - fSpacingX * std::max(0, cols - 1)) / std::max(1, cols);
            const int ch = (h - fSpacingY * std::max(0, rows - 1)) / std::max(1, rows);
            std::fill(colW.begin(), colW.end(), cw);
            std::fill(rowH.begin(), rowH.end(), ch);
        } else {
            // First pass: minimums from single-cell children.
            for (const auto& c : fChildren) {
                if (c.pack.attachW == 1) {
                    int col = c.pack.attachX;
                    if (col >= 0 && col < cols)
                        colW[col] = std::max(colW[col], c.item->minW() + 2*c.pack.padX);
                }
                if (c.pack.attachH == 1) {
                    int row = c.pack.attachY;
                    if (row >= 0 && row < rows)
                        rowH[row] = std::max(rowH[row], c.item->minH() + 2*c.pack.padY);
                }
            }
            // Distribute extra w/h evenly across columns/rows.
            int usedW = fSpacingX * std::max(0, cols - 1);
            int usedH = fSpacingY * std::max(0, rows - 1);
            for (int v : colW) usedW += v;
            for (int v : rowH) usedH += v;
            int extraW = w - usedW;
            int extraH = h - usedH;
            if (extraW > 0 && cols > 0) {
                int per = extraW / cols, rem = extraW % cols;
                for (int& v : colW) { v += per + (rem > 0 ? (rem--, 1) : 0); }
            }
            if (extraH > 0 && rows > 0) {
                int per = extraH / rows, rem = extraH % rows;
                for (int& v : rowH) { v += per + (rem > 0 ? (rem--, 1) : 0); }
            }
        }

        std::vector<int> colX(cols + 1, 0), rowY(rows + 1, 0);
        colX[0] = x;
        for (int i = 0; i < cols; ++i) colX[i+1] = colX[i] + colW[i] + fSpacingX;
        rowY[0] = y;
        for (int i = 0; i < rows; ++i) rowY[i+1] = rowY[i] + rowH[i] + fSpacingY;

        for (auto& c : fChildren) {
            int ax = std::clamp(c.pack.attachX, 0, cols - 1);
            int ay = std::clamp(c.pack.attachY, 0, rows - 1);
            int aw = std::clamp(c.pack.attachW, 1, cols - ax);
            int ah = std::clamp(c.pack.attachH, 1, rows - ay);
            int cx = colX[ax] + c.pack.padX;
            int cy = rowY[ay] + c.pack.padY;
            int cw = (colX[ax + aw] - fSpacingX) - colX[ax] - 2*c.pack.padX;
            int ch = (rowY[ay + ah] - fSpacingY) - rowY[ay] - 2*c.pack.padY;

            if (!c.pack.fillX) {
                int mw = c.item->minW();
                cx += (cw - mw) / 2; cw = mw;
            }
            if (!c.pack.fillY) {
                int mh = c.item->minH();
                cy += (ch - mh) / 2; ch = mh;
            }
            c.item->place(cx, cy, std::max(0, cw), std::max(0, ch));
        }
    }

    void draw(NanoVG& vg) override
    {
        for (auto& c : fChildren) c.item->draw(vg);
    }

private:
    struct Entry { std::unique_ptr<CalfLayoutItem> item; CalfPacking pack; };
    int                fRows, fCols;
    bool               fHomogeneous;
    int                fSpacingX, fSpacingY;
    std::vector<Entry> fChildren;
};

class CalfAlignItem : public CalfLayoutItem
{
public:
    CalfAlignItem(std::unique_ptr<CalfLayoutItem> child,
                  float alignX = 0.5f, float alignY = 0.5f,
                  float scaleX = 0.0f, float scaleY = 0.0f)
        : fChild(std::move(child)),
          fAlignX(alignX), fAlignY(alignY),
          fScaleX(scaleX), fScaleY(scaleY) {}

    int minW() const override { return fChild->minW(); }
    int minH() const override { return fChild->minH(); }

    void place(int x, int y, int w, int h) override
    {
        const int mw = fChild->minW();
        const int mh = fChild->minH();
        const int cw = fScaleX > 0.0f ? w : std::min(w, mw);
        const int ch = fScaleY > 0.0f ? h : std::min(h, mh);
        const int cx = x + static_cast<int>((w - cw) * fAlignX);
        const int cy = y + static_cast<int>((h - ch) * fAlignY);
        fChild->place(cx, cy, cw, ch);
    }

    void draw(NanoVG& vg) override { fChild->draw(vg); }

private:
    std::unique_ptr<CalfLayoutItem> fChild;
    float fAlignX, fAlignY, fScaleX, fScaleY;
};

class CalfFrameItem : public CalfLayoutItem
{
public:
    CalfFrameItem(const char* label, std::unique_ptr<CalfLayoutItem> child)
        : fLabel(label ? label : ""), fChild(std::move(child)) {}

    int minW() const override { return fChild->minW() + 2*kPadX; }
    int minH() const override { return fChild->minH() + kPadTop + kPadBot; }

    void place(int x, int y, int w, int h) override
    {
        fX = x; fY = y; fW = w; fH = h;
        fChild->place(x + kPadX,
                      y + kPadTop,
                      std::max(0, w - 2*kPadX),
                      std::max(0, h - kPadTop - kPadBot));
    }

    void draw(NanoVG& vg) override
    {
        vg.beginPath();
        vg.roundedRect(fX + 0.5f, fY + 0.5f, fW - 1.0f, fH - 1.0f, 4.0f);
        vg.strokeColor(Color(0.45f, 0.45f, 0.5f));
        vg.strokeWidth(1.0f);
        vg.stroke();

        if (!fLabel.empty()) {
            vg.fontFace(NANOVG_DEJAVU_SANS_TTF);
            vg.fontSize(11.0f);
            vg.textAlign(NanoVG::ALIGN_LEFT | NanoVG::ALIGN_MIDDLE);
            Rectangle<float> bounds;
            vg.textBounds(0, 0, fLabel.c_str(), nullptr, bounds);
            const float tw = bounds.getWidth();

            vg.beginPath();
            vg.rect(fX + 10, fY - 1, tw + 8, 4);
            vg.fillColor(Color(0.12f, 0.12f, 0.14f));
            vg.fill();

            vg.fillColor(Color(0.85f, 0.85f, 0.9f));
            vg.text(fX + 14, fY + 4, fLabel.c_str(), nullptr);
        }

        fChild->draw(vg);
    }

private:
    static constexpr int kPadX   = 6;
    static constexpr int kPadTop = 14;
    static constexpr int kPadBot = 6;

    std::string                     fLabel;
    std::unique_ptr<CalfLayoutItem> fChild;
    int fX = 0, fY = 0, fW = 0, fH = 0;
};

END_NAMESPACE_DGL

#endif
