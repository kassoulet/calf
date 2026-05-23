/*
 * CalfTheme — runtime accessor for the embedded Calf_Default GTK PNGs.
 *
 * Each plugin UI has its own NanoVG context and its own image set;
 * the cache lives in the owning UI (passed in via a small Theme
 * struct). Use as:
 *
 *     CalfTheme theme(*this);
 *     NanoImage* bg = theme.image("background_plugin.png");
 *     if (bg && bg->isValid()) {
 *         Paint p = imagePattern(0,0, getWidth(), getHeight(), 0.f, *bg, 1.f);
 *         beginPath(); rect(0,0,getWidth(),getHeight()); fillPaint(p); fill();
 *     }
 */
#ifndef CALF_THEME_HPP
#define CALF_THEME_HPP

#include "NanoVG.hpp"
#include "theme_assets.hpp"
#include <memory>
#include <string>
#include <unordered_map>

START_NAMESPACE_DGL

class CalfTheme
{
public:
    explicit CalfTheme(NanoVG& ctx) : fCtx(ctx) {}

    NanoImage* image(const char* name)
    {
        auto it = fCache.find(name);
        if (it != fCache.end()) return it->second.get();

        const calf_theme::ThemeAsset* a = calf_theme::find(name);
        if (a == nullptr) {
            fCache[name] = nullptr;
            return nullptr;
        }
        auto img = std::unique_ptr<NanoImage>(new NanoImage(
            fCtx.createImageFromMemory(a->data, a->size, (NanoVG::ImageFlags)0)));
        if (!img->isValid()) {
            fCache[name] = nullptr;
            return nullptr;
        }
        NanoImage* raw = img.get();
        fCache[name] = std::move(img);
        return raw;
    }

private:
    NanoVG& fCtx;
    std::unordered_map<std::string, std::unique_ptr<NanoImage>> fCache;
};

END_NAMESPACE_DGL

#endif
