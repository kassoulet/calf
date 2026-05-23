#!/usr/bin/env python3
"""
repaint_uis.py — retrofit the themed background hook into every
already-generated <Name>UI.cpp so we don't have to remember each
plugin's xml2ui.py argument set.

This file applies three textual edits to every plugin UI file:

  1. add `#include "CalfTheme.hpp"` right after `#include "CalfLayout.hpp"`
  2. replace the flat dark fill block in onNanoDisplay() with one
     that paints background_plugin.png stretched across the UI
  3. add a `CalfTheme fTheme{*this};` member next to fRoot

Idempotent: re-running on an already-patched file is a no-op.

Once we're ready to make this canonical, xml2ui.py already emits the
same three patterns, so a future codegen sweep can drop this script.
"""
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent.parent
PLUGINS = REPO / "calf-dpf" / "plugins"
XML_DIR = REPO / "gui" / "gui"

OLD_INCLUDE = '#include "CalfLayout.hpp"\n'
NEW_INCLUDE = '#include "CalfLayout.hpp"\n#include "CalfTheme.hpp"\n'

OLD_BG = """    void onNanoDisplay() override
    {
        beginPath();
        rect(0, 0, getWidth(), getHeight());
        fillColor(Color(0.12f, 0.12f, 0.14f));
        fill();
        if (fRoot) fRoot->draw(*this);
    }
"""

NEW_BG = """    void onNanoDisplay() override
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());

        // Themed background: stretch the GTK Calf_Default plugin
        // background across the whole UI. Falls back to the dark flat
        // fill if the asset failed to decode.
        beginPath();
        rect(0, 0, w, h);
        NanoImage* _bg = fTheme.image("background_plugin.png");
        if (_bg && _bg->isValid()) {
            fillPaint(imagePattern(0, 0, w, h, 0.0f, *_bg, 1.0f));
        } else {
            fillColor(Color(0.12f, 0.12f, 0.14f));
        }
        fill();

        if (fRoot) fRoot->draw(*this);
    }
"""

OLD_MEMBER = "    std::unique_ptr<CalfLayoutItem>                   fRoot;\n"
NEW_MEMBER = (
    "    std::unique_ptr<CalfLayoutItem>                   fRoot;\n"
    "    CalfTheme                                         fTheme{*this};\n"
)

XML_HEADER = re.compile(r"from gui/gui/(\S+\.xml)\.")
KNOB_NEW   = re.compile(
    r'^(?P<indent>\s*)auto\* (?P<var>w_\d+) = new CalfKnob\(this, \*meta\.get_param_props\(_resolve\("(?P<param>[^"]+)"\)\), _resolve\("[^"]+"\)\);',
    re.MULTILINE,
)
TOGGLE_NEW = re.compile(
    r'^(?P<indent>\s*)auto\* (?P<var>w_\d+) = new CalfToggle\(this, \*meta\.get_param_props\(_resolve\("(?P<param>[^"]+)"\)\), _resolve\("[^"]+"\)\);',
    re.MULTILINE,
)


def collect_knob_sizes(xml_path: Path) -> dict:
    """Return {param_name: size_int} for every <knob param="X" size="N">."""
    if not xml_path.exists():
        return {}
    try:
        root = ET.parse(xml_path).getroot()
    except ET.ParseError:
        return {}
    sizes = {}
    for k in root.iter("knob"):
        p = k.attrib.get("param")
        s = k.attrib.get("size")
        if p and s is not None:
            try:
                sizes[p] = int(s)
            except ValueError:
                pass
    return sizes


def collect_toggle_icons(xml_path: Path) -> dict:
    """Return {param_name: icon_str} for every <toggle param="X" icon="…">."""
    if not xml_path.exists():
        return {}
    try:
        root = ET.parse(xml_path).getroot()
    except ET.ParseError:
        return {}
    icons = {}
    for t in root.iter("toggle"):
        p = t.attrib.get("param")
        i = t.attrib.get("icon")
        if p and i:
            icons[p] = i
    return icons


def _inject_after_new(text: str, pattern, lookup: dict, call_fmt: str, marker: str) -> str:
    """Generic: after every `new X(…_resolve("P")…)` line whose P is in
    `lookup`, splice a `<var>-><call_fmt>;` line in. Skips if a call
    with `marker` already follows the new-line."""
    if not lookup:
        return text
    out = []
    last = 0
    for m in pattern.finditer(text):
        param = m.group("param")
        if param not in lookup:
            continue
        var = m.group("var")
        indent = m.group("indent")
        tail_start = m.end()
        nxt = text[tail_start:tail_start + 200]
        if f"{var}->{marker}(" in nxt[:120]:
            continue
        out.append(text[last:m.end()])
        out.append(f"\n{indent}{var}->{call_fmt.format(value=lookup[param])};")
        last = m.end()
    out.append(text[last:])
    return "".join(out)


def inject_knob_sizes(text: str, sizes: dict) -> str:
    return _inject_after_new(text, KNOB_NEW, sizes, "setKnobSize({value})", "setKnobSize")


def inject_toggle_icons(text: str, icons: dict) -> str:
    return _inject_after_new(text, TOGGLE_NEW, icons, 'setIcon("{value}")', "setIcon")


def patch(path: Path) -> bool:
    text = path.read_text()
    orig = text
    if 'CalfTheme.hpp' not in text:
        text = text.replace(OLD_INCLUDE, NEW_INCLUDE, 1)
    if OLD_BG in text:
        text = text.replace(OLD_BG, NEW_BG, 1)
    if 'CalfTheme                                         fTheme' not in text:
        text = text.replace(OLD_MEMBER, NEW_MEMBER, 1)
    m = XML_HEADER.search(text)
    if m:
        xml = XML_DIR / m.group(1)
        text = inject_knob_sizes(text, collect_knob_sizes(xml))
        text = inject_toggle_icons(text, collect_toggle_icons(xml))
    if text != orig:
        path.write_text(text)
        return True
    return False

def main() -> int:
    changed = 0
    skipped = 0
    for ui in sorted(PLUGINS.glob("*/[A-Z]*UI.cpp")):
        if patch(ui):
            changed += 1
        else:
            skipped += 1
    print(f"patched {changed}, unchanged {skipped}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
