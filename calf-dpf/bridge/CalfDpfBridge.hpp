/*
 * Calf → DPF bridge helpers (header-only).
 *
 * Maps Calf's parameter_properties / port-name tables and module I/O
 * onto DPF Plugin virtuals. Used by every per-plugin adapter under
 * calf-dpf/plugins/<Name>/.
 *
 * The helpers here are not a generic abstraction layer — they encode
 * exactly the conventions baked into Calf's metadata system, so each
 * adapter stays in the 30-50 line range.
 */
#ifndef CALF_DPF_BRIDGE_HPP
#define CALF_DPF_BRIDGE_HPP

#include "DistrhoPlugin.hpp"
#include <calf/giface.h>
#include <calf/metadata.h>
#include <cstring>
#include <vector>

START_NAMESPACE_DISTRHO

namespace calf_dpf {

/* ----- unit mapping -------------------------------------------------- */

inline const char* unit_from_flags(uint32_t flags)
{
    switch (flags & calf_plugins::PF_UNITMASK) {
        case calf_plugins::PF_UNIT_DB:        return "dB";
        case calf_plugins::PF_UNIT_DBFS:      return "dBFS";
        case calf_plugins::PF_UNIT_COEF:      return "x";
        case calf_plugins::PF_UNIT_HZ:        return "Hz";
        case calf_plugins::PF_UNIT_SEC:       return "s";
        case calf_plugins::PF_UNIT_MSEC:      return "ms";
        case calf_plugins::PF_UNIT_CENTS:     return "cents";
        case calf_plugins::PF_UNIT_SEMITONES: return "semitones";
        case calf_plugins::PF_UNIT_BPM:       return "BPM";
        case calf_plugins::PF_UNIT_DEG:       return "deg";
        case calf_plugins::PF_UNIT_NOTE:      return "note";
        case calf_plugins::PF_UNIT_RPM:       return "RPM";
        case calf_plugins::PF_UNIT_SAMPLES:   return "samples";
        default:                              return "";
    }
}

/* ----- one-shot parameter initialisation ----------------------------- */

/// Fill a DPF Parameter from a Calf parameter_properties entry.
/// `pp` is one row of the param_props table. `symbol_fallback` is used
/// when pp.short_name is null.
inline void init_parameter(Parameter& parameter,
                           const calf_plugins::parameter_properties& pp)
{
    using namespace calf_plugins;

    uint32_t hints = 0;
    const uint32_t type = pp.flags & PF_TYPEMASK;
    const uint32_t scale = pp.flags & PF_SCALEMASK;

    if (pp.flags & PF_PROP_OUTPUT) {
        hints |= kParameterIsOutput;
    } else {
        hints |= kParameterIsAutomatable;
    }
    if (type == PF_BOOL)
        hints |= kParameterIsBoolean | kParameterIsInteger;
    if (type == PF_INT || type == PF_ENUM)
        hints |= kParameterIsInteger;
    if (scale == PF_SCALE_LOG || scale == PF_SCALE_GAIN || scale == PF_SCALE_LOG_INF)
        hints |= kParameterIsLogarithmic;

    parameter.hints      = hints;
    parameter.name       = pp.name      ? pp.name      : pp.short_name;
    parameter.symbol     = pp.short_name ? pp.short_name : pp.name;
    parameter.unit       = unit_from_flags(pp.flags);
    parameter.ranges.def = pp.def_value;
    parameter.ranges.min = pp.min;
    parameter.ranges.max = pp.max;

    if (type == PF_ENUM && pp.choices) {
        uint8_t count = 0;
        for (const char** p = pp.choices; *p; ++p) ++count;
        ParameterEnumerationValue* vals = new ParameterEnumerationValue[count];
        for (uint8_t i = 0; i < count; ++i) {
            vals[i].value = static_cast<float>(i) + pp.min;
            vals[i].label = pp.choices[i];
        }
        parameter.enumValues.count          = count;
        parameter.enumValues.values         = vals;
        parameter.enumValues.restrictedMode = true;
        parameter.enumValues.deleteLater    = true;
    }
}

/* ----- per-plugin glue ---------------------------------------------- *
 *
 * Holds the parameter shadow cache (one float per Calf parameter), plus
 * the input/output buffer pointer pre-image arrays that Calf expects to
 * read on every process() call.
 */
template <class CalfModule>
class CalfPluginBase : public Plugin
{
public:
    using Module   = CalfModule;
    using Metadata = typename CalfModule::metadata_type;

    static uint32_t calfStateCount()
    {
        Metadata m;
        std::vector<std::string> v;
        m.get_configure_vars(v);
        return static_cast<uint32_t>(v.size());
    }

    CalfPluginBase()
        : Plugin(Metadata::param_count, /*programs=*/0, calfStateCount()),
          fModule()
    {
        // Wire the parameter shadow buffer into Calf's module before
        // anything reads from it.
        for (int i = 0; i < Metadata::param_count; ++i) {
            fParamCache[i] = fModule.get_metadata_iface()
                                ->get_param_props(i)->def_value;
            fModule.params[i] = &fParamCache[i];
        }
        // Cache the static configure-var key list once. Each entry will
        // become one DPF state slot.
        Metadata().get_configure_vars(fStateKeys);
        // post_instantiate has to be called once, after which set_sample_rate
        // / activate are valid. Host-supplied SR is plugged in via DPF.
        fModule.post_instantiate(static_cast<uint32_t>(getSampleRate()));
        fModule.set_sample_rate(static_cast<uint32_t>(getSampleRate()));
    }

protected:
    /* ---- DPF virtuals provided by the metadata ---------------------- */
    const char* getLabel()    const override { return Metadata::impl_get_label();    }
    const char* getMaker()    const override { return "Calf Studio Gear"; }
    const char* getHomePage() const override { return "https://calf-studio-gear.org/"; }
    const char* getLicense()  const override { return "GPL-2.0-only"; }
    uint32_t    getVersion()  const override { return d_version(0, 91, 0); }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        if (index >= Metadata::param_count) return;
        const calf_plugins::parameter_properties* pp =
            fModule.get_metadata_iface()->get_param_props(static_cast<int>(index));
        calf_dpf::init_parameter(parameter, *pp);
    }

    float getParameterValue(uint32_t index) const override
    {
        return index < Metadata::param_count ? fParamCache[index] : 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override
    {
        if (index < Metadata::param_count)
            fParamCache[index] = value;
    }

    void sampleRateChanged(double newSampleRate) override
    {
        fModule.set_sample_rate(static_cast<uint32_t>(newSampleRate));
    }

    void activate()   override { fModule.params_changed(); fModule.activate(); }
    void deactivate() override { fModule.deactivate(); }

#if DISTRHO_PLUGIN_WANT_STATE
    /* ---- Calf configure() vars <-> DPF State ----------------------- *
     *
     * Each entry returned by Metadata::get_configure_vars() becomes one
     * DPF state slot. setState forwards to module->configure(); getState
     * snapshots a single key via a one-shot send_configure_iface captor.
     */
    void initState(uint32_t index, State& state) override
    {
        if (index >= fStateKeys.size()) return;
        state.key          = fStateKeys[index].c_str();
        state.label        = fStateKeys[index].c_str();
        state.hints        = kStateIsHostWritable | kStateIsOnlyForDSP;
        state.defaultValue = "";
    }

    void setState(const char* key, const char* value) override
    {
        char* err = fModule.configure(key, value);
        // configure() returns char* error string (rare, ignored here).
        (void)err;
    }

    String getState(const char* key) const override
    {
        struct OneValueCaptor : calf_plugins::send_configure_iface {
            const char* needle;
            String      found;
            bool        seen = false;
            void send_configure(const char* k, const char* v) override
            {
                if (!seen && std::strcmp(k, needle) == 0) {
                    found = v;
                    seen  = true;
                }
            }
        } cap;
        cap.needle = key;
        const_cast<Module&>(fModule).send_configures(&cap);
        return cap.seen ? cap.found : String("");
    }
#endif // DISTRHO_PLUGIN_WANT_STATE

    /* ---- sub-buffer processing helper ------------------------------- *
     *
     * Calf modules process at most MAX_SAMPLE_RUN frames per call. This
     * helper slices the host's run() block accordingly and dispatches
     * MIDI events at the right frame offset. Effects (no MIDI) call
     * runBlock(inputs, outputs, frames, nullptr, 0).
     */
    void runBlock(const float** inputs, float** outputs, uint32_t frames,
                  const MidiEvent* midiEvents, uint32_t midiEventCount)
    {
        // Re-push parameter values into Calf each block. Calf reads the
        // float* pointers (already wired in the ctor) but expects
        // params_changed() between updates.
        fModule.params_changed();

        uint32_t midiIdx = 0;
        uint32_t pos = 0;
        while (pos < frames) {
            // Deliver any MIDI events landing at exactly this frame.
            while (midiIdx < midiEventCount && midiEvents[midiIdx].frame <= pos) {
                dispatchMidi(midiEvents[midiIdx]);
                ++midiIdx;
            }
            // Run up to MAX_SAMPLE_RUN, but stop early at the next MIDI
            // event so its frame timing is respected.
            uint32_t end = pos + calf_plugins::MAX_SAMPLE_RUN;
            if (end > frames) end = frames;
            if (midiIdx < midiEventCount && midiEvents[midiIdx].frame < end)
                end = midiEvents[midiIdx].frame;

            // Point Calf's port buffers at the right sub-slice.
            for (int c = 0; c < Metadata::in_count;  ++c) fModule.ins[c]  = const_cast<float*>(inputs[c]  + pos);
            for (int c = 0; c < Metadata::out_count; ++c) fModule.outs[c] = outputs[c] + pos;

            fModule.process(0, end - pos, ~0u, ~0u);
            pos = end;
        }
        // Drain trailing MIDI events (at or past `frames`).
        while (midiIdx < midiEventCount) {
            dispatchMidi(midiEvents[midiIdx]);
            ++midiIdx;
        }
    }

private:
    void dispatchMidi(const MidiEvent& ev)
    {
        if (ev.size < 1) return;
        const uint8_t* d = (ev.size > MidiEvent::kDataSize && ev.dataExt) ? ev.dataExt : ev.data;
        const uint8_t status  = d[0] & 0xF0;
        const int     channel = d[0] & 0x0F;
        switch (status) {
            case 0x80:  // Note off
                if (ev.size >= 3) fModule.note_off(channel, d[1], d[2]);
                break;
            case 0x90:  // Note on (vel 0 == note off, per MIDI spec)
                if (ev.size >= 3) {
                    if (d[2] == 0) fModule.note_off(channel, d[1], 0);
                    else           fModule.note_on (channel, d[1], d[2]);
                }
                break;
            case 0xB0:  // CC
                if (ev.size >= 3) fModule.control_change(channel, d[1], d[2]);
                break;
            case 0xC0:  // Program change
                if (ev.size >= 2) fModule.program_change(channel, d[1]);
                break;
            case 0xD0:  // Channel pressure
                if (ev.size >= 2) fModule.channel_pressure(channel, d[1]);
                break;
            case 0xE0:  // Pitch bend
                if (ev.size >= 3) {
                    int value = (int(d[2]) << 7 | int(d[1])) - 8192;
                    fModule.pitch_bend(channel, value);
                }
                break;
        }
    }

protected:
    Module fModule;
    float  fParamCache[Metadata::param_count];
    std::vector<std::string> fStateKeys;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CalfPluginBase)
};

} // namespace calf_dpf

END_NAMESPACE_DISTRHO

#endif
