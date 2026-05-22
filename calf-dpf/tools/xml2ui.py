#!/usr/bin/env python3
"""
xml2ui.py — translate a Calf gui/gui/<name>.xml into a DPF UI subclass.

Reads the GTK-era layout XML, walks the container/widget tree, and emits
a single .cpp file containing one ``<Name>UI : public UI`` class. The
emitted code uses the CalfLayoutItem hierarchy from calf-dpf/ui-lib/
(BoxItem / TableItem / FrameItem / AlignItem / WidgetItem) and the
existing widget set (Knob, VuMeter, Led, Toggle, ComboBox, Label, Value,
LineGraph).

Param binding uses parameter_properties::short_name → index, resolved at
UI construction time against the supplied metadata class.

Unsupported widgets (scale, button, tube, keyboard, …) and containers
(notebook, scrolled) are emitted as best-effort stubs so the file still
compiles and the layout still works. They'll be filled in as the widget
library grows.
"""

from __future__ import annotations

import argparse
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


# Widget tag → (C++ class, include header). Tags absent from this map fall
# back to CalfLineGraph as an inert visual placeholder.
WIDGET_MAP: dict[str, tuple[str, str]] = {
    "knob":       ("CalfKnob",      "CalfKnob.hpp"),
    "vumeter":    ("CalfVuMeter",   "CalfVuMeter.hpp"),
    "led":        ("CalfLed",       "CalfLed.hpp"),
    "toggle":     ("CalfToggle",    "CalfToggle.hpp"),
    "check":      ("CalfToggle",    "CalfToggle.hpp"),
    "radio":      ("CalfToggle",    "CalfToggle.hpp"),
    "combo":      ("CalfComboBox",  "CalfComboBox.hpp"),
    "label":      ("CalfLabel",     "CalfLabel.hpp"),
    "value":      ("CalfValue",     "CalfValue.hpp"),
    "line-graph": ("CalfLineGraph", "CalfLineGraph.hpp"),
    "phase-graph":("CalfLineGraph", "CalfLineGraph.hpp"),
    "curve":      ("CalfLineGraph", "CalfLineGraph.hpp"),
    "pattern":    ("CalfLineGraph", "CalfLineGraph.hpp"),
    "tuner":      ("CalfLineGraph", "CalfLineGraph.hpp"),
    "keyboard":   ("CalfLineGraph", "CalfLineGraph.hpp"),
    "meterscale": ("CalfLabel",     "CalfLabel.hpp"),  # decorative ruler placeholder
    "hscale":     ("CalfKnob",      "CalfKnob.hpp"),
    "vscale":     ("CalfKnob",      "CalfKnob.hpp"),
    "button":     ("CalfToggle",    "CalfToggle.hpp"),
    "entry":      ("CalfValue",     "CalfValue.hpp"),
    "tube":       ("CalfLed",       "CalfLed.hpp"),
}

CONTAINER_TAGS = {"hbox", "vbox", "table", "frame", "align", "scrolled", "notebook", "if"}


@dataclass
class Emitter:
    lines: list[str] = field(default_factory=list)
    indent: int = 0
    includes: set[str] = field(default_factory=set)
    fresh_counter: int = 0

    def emit(self, line: str = "") -> None:
        if line:
            self.lines.append("    " * self.indent + line)
        else:
            self.lines.append("")

    def fresh(self, prefix: str) -> str:
        self.fresh_counter += 1
        return f"{prefix}{self.fresh_counter}"

    def include(self, hdr: str) -> None:
        self.includes.add(hdr)


# --- helpers ---------------------------------------------------------------

def b(s: Optional[str], default: bool) -> bool:
    if s is None:
        return default
    return s.strip().lower() in ("1", "true", "yes", "on")


def i(s: Optional[str], default: int) -> int:
    if s is None:
        return default
    try:
        return int(s)
    except ValueError:
        return default


def f(s: Optional[str], default: float) -> float:
    if s is None:
        return default
    try:
        return float(s)
    except ValueError:
        return default


def cstr(s: Optional[str]) -> str:
    if s is None:
        return "nullptr"
    # Escape backslashes and quotes for C string literal.
    esc = s.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{esc}"'


# --- packing -------------------------------------------------------------

def packing_literal(elem: ET.Element) -> str:
    """Emit a CalfPacking{} initializer for child packing attrs."""
    a = elem.attrib

    # GTK shorthand: `expand`/`fill` set both axes unless overridden.
    expand = b(a.get("expand"), True)
    fill   = b(a.get("fill"),   True)
    ex = b(a.get("expand-x"), expand)
    ey = b(a.get("expand-y"), expand)
    fx = b(a.get("fill-x"),   fill)
    fy = b(a.get("fill-y"),   fill)
    px = i(a.get("pad-x"), 0)
    py = i(a.get("pad-y"), 0)
    ax = i(a.get("attach-x"), 0)
    ay = i(a.get("attach-y"), 0)
    aw = i(a.get("attach-w"), 1)
    ah = i(a.get("attach-h"), 1)

    parts = [
        f".expandX={'true' if ex else 'false'}",
        f".expandY={'true' if ey else 'false'}",
        f".fillX={'true' if fx else 'false'}",
        f".fillY={'true' if fy else 'false'}",
    ]
    if px: parts.append(f".padX={px}")
    if py: parts.append(f".padY={py}")
    if ax: parts.append(f".attachX={ax}")
    if ay: parts.append(f".attachY={ay}")
    if aw != 1: parts.append(f".attachW={aw}")
    if ah != 1: parts.append(f".attachH={ah}")
    return "CalfPacking{" + ", ".join(parts) + "}"


# --- widget creation -----------------------------------------------------

def emit_widget(em: Emitter, elem: ET.Element) -> Optional[str]:
    """Emit code that instantiates the widget and returns its raw pointer name
    (a CalfWidgetBase*). Returns None if the element is to be skipped."""
    tag = elem.tag
    cls, hdr = WIDGET_MAP.get(tag, ("CalfLineGraph", "CalfLineGraph.hpp"))
    em.include(hdr)

    var = em.fresh("w_")
    param = elem.attrib.get("param")

    # Param-based widgets: resolve short_name → index at runtime. If param is
    # absent (decorative widgets, label with text=, etc.), bind to param 0 as
    # a harmless placeholder — the widget either ignores its param (label
    # with explicit text) or simply mirrors a value it never edits.
    if param:
        idx_expr = f'_resolve("{param}")'
    else:
        idx_expr = "0u"

    if tag == "label":
        text_attr = elem.attrib.get("text")
        text_arg = cstr(text_attr) if text_attr else "nullptr"
        em.emit(
            f"auto* {var} = new CalfLabel(this, *meta.get_param_props({idx_expr}), "
            f"{idx_expr}, {text_arg});"
        )
    elif tag == "line-graph":
        w = i(elem.attrib.get("width"),  240)
        h = i(elem.attrib.get("height"), 160)
        em.emit(
            f"auto* {var} = new CalfLineGraph(this, *meta.get_param_props({idx_expr}), "
            f"{idx_expr}, {w}, {h});"
        )
    else:
        em.emit(
            f"auto* {var} = new {cls}(this, *meta.get_param_props({idx_expr}), {idx_expr});"
        )

    # Codegen-driven layouts always render labels separately.
    if cls in ("CalfKnob", "CalfVuMeter", "CalfLed", "CalfToggle", "CalfComboBox"):
        em.emit(f"{var}->setShowLabels(false);")

    # Explicit width/height attrs (line-graph etc.)
    w_attr = elem.attrib.get("width")
    h_attr = elem.attrib.get("height")
    if w_attr or h_attr:
        cur_w = i(w_attr, 0) or "{var}->getWidth()".format(var=var)
        cur_h = i(h_attr, 0) or "{var}->getHeight()".format(var=var)
        em.emit(f"{var}->setSize(static_cast<uint>({cur_w}), static_cast<uint>({cur_h}));")

    em.emit(f"fWidgets.emplace_back({var});")
    if param:
        em.emit(f"fByIndex[{idx_expr}].push_back({var});")
    return var


# --- container / widget dispatch ------------------------------------------

def emit_node(em: Emitter, elem: ET.Element, parent_var: str, parent_pack: str) -> None:
    """Emit code that creates `elem` and adds it to `parent_var` with packing
    `parent_pack`. `parent_var` may be empty for root."""
    tag = elem.tag

    def add_to_parent(item_expr: str) -> None:
        if parent_var:
            em.emit(f"{parent_var}->add({item_expr}, {parent_pack});")

    if tag == "if":
        # `<if cond="directlink">` — we always support DSP-side hooks via DPF,
        # so emit children. Other conds (configure, …) are build-time gates;
        # we conservatively emit them too. Negated forms (`!cond`) we skip.
        cond = elem.attrib.get("cond", "")
        if cond.startswith("!"):
            return
        for child in elem:
            emit_node(em, child, parent_var, parent_pack)
        return

    if tag == "scrolled":
        # No CalfScrolled yet; pass through the single child (or skip if none).
        for child in elem:
            emit_node(em, child, parent_var, parent_pack)
        return

    if tag in ("vbox", "hbox"):
        var = em.fresh("c_")
        orient = "VERTICAL" if tag == "vbox" else "HORIZONTAL"
        spacing = i(elem.attrib.get("spacing"), 0)
        homog = b(elem.attrib.get("homogeneous"), False)
        em.emit("{")
        em.indent += 1
        em.emit(
            f"auto {var} = std::make_unique<CalfBoxItem>("
            f"CalfBoxItem::{orient}, {spacing}, {'true' if homog else 'false'});"
        )
        for child in elem:
            emit_node(em, child, var, packing_literal(child))
        if parent_var:
            em.emit(f"{parent_var}->add(std::move({var}), {parent_pack});")
        else:
            em.emit(f"_root = std::move({var});")
        em.indent -= 1
        em.emit("}")
        return

    if tag == "table":
        var = em.fresh("c_")
        rows = i(elem.attrib.get("rows"), 1)
        cols = i(elem.attrib.get("cols"), 1)
        homog = b(elem.attrib.get("homogeneous"), False)
        sx = i(elem.attrib.get("spacing-x"), i(elem.attrib.get("spacing"), 0))
        sy = i(elem.attrib.get("spacing-y"), i(elem.attrib.get("spacing"), 0))
        em.emit("{")
        em.indent += 1
        em.emit(
            f"auto {var} = std::make_unique<CalfTableItem>("
            f"{rows}, {cols}, {'true' if homog else 'false'}, {sx}, {sy});"
        )
        for child in elem:
            emit_node(em, child, var, packing_literal(child))
        if parent_var:
            em.emit(f"{parent_var}->add(std::move({var}), {parent_pack});")
        else:
            em.emit(f"_root = std::move({var});")
        em.indent -= 1
        em.emit("}")
        return

    if tag == "frame":
        var = em.fresh("c_")
        label = cstr(elem.attrib.get("label"))
        em.emit("{")
        em.indent += 1
        # Build a sub-container holding the frame's children, then wrap it.
        # Most frames in Calf XML have exactly one child container, but
        # be lenient — wrap multiple children in an implicit vbox.
        inner = em.fresh("c_")
        em.emit(f"auto {inner} = std::make_unique<CalfBoxItem>(CalfBoxItem::VERTICAL, 0, false);")
        for child in elem:
            emit_node(em, child, inner, "CalfPacking{}")
        em.emit(f"auto {var} = std::make_unique<CalfFrameItem>({label}, std::move({inner}));")
        if parent_var:
            em.emit(f"{parent_var}->add(std::move({var}), {parent_pack});")
        else:
            em.emit(f"_root = std::move({var});")
        em.indent -= 1
        em.emit("}")
        return

    if tag == "align":
        var = em.fresh("c_")
        ax = f(elem.attrib.get("align-x"), 0.5)
        ay = f(elem.attrib.get("align-y"), 0.5)
        sx = f(elem.attrib.get("scale-x"), 0.0)
        sy = f(elem.attrib.get("scale-y"), 0.0)
        em.emit("{")
        em.indent += 1
        inner = em.fresh("c_")
        em.emit(f"auto {inner} = std::make_unique<CalfBoxItem>(CalfBoxItem::VERTICAL, 0, false);")
        for child in elem:
            emit_node(em, child, inner, "CalfPacking{}")
        em.emit(
            f"auto {var} = std::make_unique<CalfAlignItem>("
            f"std::move({inner}), {ax}f, {ay}f, {sx}f, {sy}f);"
        )
        if parent_var:
            em.emit(f"{parent_var}->add(std::move({var}), {parent_pack});")
        else:
            em.emit(f"_root = std::move({var});")
        em.indent -= 1
        em.emit("}")
        return

    if tag == "notebook":
        # No notebook widget yet — fall back to vertical stack so the page
        # contents render (without tabs). Page labels are dropped.
        var = em.fresh("c_")
        em.emit("{")
        em.indent += 1
        em.emit(
            f"auto {var} = std::make_unique<CalfBoxItem>("
            f"CalfBoxItem::VERTICAL, 4, false);"
        )
        for child in elem:
            emit_node(em, child, var, packing_literal(child))
        if parent_var:
            em.emit(f"{parent_var}->add(std::move({var}), {parent_pack});")
        else:
            em.emit(f"_root = std::move({var});")
        em.indent -= 1
        em.emit("}")
        return

    # Widget leaf.
    widget_var = emit_widget(em, elem)
    if widget_var:
        add_to_parent(f"std::make_unique<CalfWidgetItem>({widget_var})")


# --- driver ---------------------------------------------------------------

def generate(xml_path: Path, class_name: str, metadata_class: str,
             metadata_header: str) -> str:
    tree = ET.parse(xml_path)
    root_elem = tree.getroot()

    em = Emitter()
    em.indent = 2
    emit_node(em, root_elem, "", "")

    extra_includes = sorted(em.includes)
    include_lines = "\n".join(f'#include "{h}"' for h in extra_includes)

    using_lines = "\n".join(
        f"using DGL_NAMESPACE::{name};" for name in sorted({
            "CalfWidgetBase",
            "CalfLayoutItem",
            "CalfWidgetItem",
            "CalfBoxItem",
            "CalfTableItem",
            "CalfFrameItem",
            "CalfAlignItem",
            "CalfPacking",
            *[WIDGET_MAP.get(t, ("CalfLineGraph",""))[0] for t in WIDGET_MAP]
        })
    )

    body = "\n".join(em.lines)

    return f"""/*
 * Generated by calf-dpf/tools/xml2ui.py from gui/gui/{xml_path.name}.
 * Do not edit by hand — re-run the codegen.
 */
#include "DistrhoUI.hpp"
#include "CalfLayout.hpp"
{include_lines}
#include <{metadata_header}>
#include <map>
#include <memory>
#include <string>
#include <vector>

START_NAMESPACE_DISTRHO

{using_lines}

class {class_name} : public UI, public CalfWidgetBase::Callback
{{
public:
    {class_name}() : UI(640, 400)
    {{
        loadSharedResources();
        using namespace calf_plugins;
        using Meta = {metadata_class};
        Meta meta;

        std::map<std::string, uint32_t> _byName;
        for (uint32_t i = 0; i < Meta::param_count; ++i) {{
            const parameter_properties* pp = meta.get_param_props(static_cast<int>(i));
            if (pp && pp->short_name) _byName[pp->short_name] = i;
        }}
        auto _resolve = [&](const char* n) -> uint32_t {{
            auto it = _byName.find(n);
            return it == _byName.end() ? 0u : it->second;
        }};

        std::unique_ptr<CalfLayoutItem> _root;
{body}

        for (auto& w : fWidgets) w->setCallback(this);
        if (_root) {{
            const int W = std::max(320, _root->minW());
            const int H = std::max(240, _root->minH());
            setSize(static_cast<uint>(W), static_cast<uint>(H));
            _root->place(0, 0, W, H);
        }}
        fRoot = std::move(_root);
    }}

protected:
    void onNanoDisplay() override
    {{
        beginPath();
        rect(0, 0, getWidth(), getHeight());
        fillColor(Color(0.12f, 0.12f, 0.14f));
        fill();
        if (fRoot) fRoot->draw(*this);
    }}

    void parameterChanged(uint32_t index, float value) override
    {{
        auto it = fByIndex.find(index);
        if (it == fByIndex.end()) return;
        for (auto* w : it->second) w->setValue(value, /*notify=*/false);
    }}

    void widgetValueChanged(CalfWidgetBase* w, float v) override
    {{
        editParameter(w->getParamIndex(), true);
        setParameterValue(w->getParamIndex(), v);
        editParameter(w->getParamIndex(), false);
    }}

private:
    std::vector<std::unique_ptr<CalfWidgetBase>>      fWidgets;
    std::map<uint32_t, std::vector<CalfWidgetBase*>>  fByIndex;
    std::unique_ptr<CalfLayoutItem>                   fRoot;
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR({class_name})
}};

UI* createUI() {{ return new {class_name}(); }}

END_NAMESPACE_DISTRHO
"""


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("xml", type=Path, help="path to gui/gui/<name>.xml")
    p.add_argument("--class-name", required=True,
                   help="C++ UI class name, e.g. CompressorUI")
    p.add_argument("--metadata-class", required=True,
                   help="C++ metadata class, e.g. compressor_metadata")
    p.add_argument("--metadata-header", default="calf/metadata.h",
                   help="header to include for the metadata class")
    p.add_argument("-o", "--output", type=Path,
                   help="output .cpp path (default: stdout)")
    args = p.parse_args()

    src = generate(args.xml, args.class_name, args.metadata_class,
                   args.metadata_header)
    if args.output:
        args.output.write_text(src)
    else:
        sys.stdout.write(src)
    return 0


if __name__ == "__main__":
    sys.exit(main())
