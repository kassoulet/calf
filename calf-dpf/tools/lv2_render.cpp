/*
 * lv2_render.cpp — minimal Lilv host that drives one of our LV2 plugins.
 *
 * Loads the plugin at the supplied URI, connects all control ports to
 * their default values, runs N frames of audio (silence by default; a
 * 1 kHz sine wave with --tone), and reports per-channel RMS + peak.
 *
 * Used by `make null-test` to confirm every Calf-DPF LV2 actually
 * loads, runs, and produces sensible output. The diff vs. upstream
 * Calf LV2 lives in tools/null_test.py — this binary is the renderer.
 *
 * Build: `make -C calf-dpf/tools lv2_render` (driven by the existing
 * tools/Makefile).
 */

#include <lilv/lilv.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr uint32_t kSampleRate = 48000;
constexpr uint32_t kFrames     = 4096;  // ~85 ms

struct PortInfo {
    enum Kind { kAudioIn, kAudioOut, kControlIn, kControlOut, kAtomIn, kAtomOut, kOther };
    uint32_t index;
    Kind     kind;
    float    defaultVal = 0.0f;
};

const char* kindName(PortInfo::Kind k)
{
    switch (k) {
        case PortInfo::kAudioIn:   return "audio_in";
        case PortInfo::kAudioOut:  return "audio_out";
        case PortInfo::kControlIn: return "control_in";
        case PortInfo::kControlOut:return "control_out";
        case PortInfo::kAtomIn:    return "atom_in";
        case PortInfo::kAtomOut:   return "atom_out";
        default:                   return "other";
    }
}

float port_default(LilvWorld* w, const LilvPlugin* plugin, uint32_t i)
{
    LilvNode* defNode = nullptr;
    lilv_plugin_get_port_ranges_float(plugin, nullptr, nullptr, nullptr);
    const LilvPort* port = lilv_plugin_get_port_by_index(plugin, i);
    LilvNode* def = nullptr;
    LilvNode* min = nullptr;
    LilvNode* max = nullptr;
    lilv_port_get_range(plugin, port, &def, &min, &max);
    float v = def ? lilv_node_as_float(def) : 0.0f;
    if (def) lilv_node_free(def);
    if (min) lilv_node_free(min);
    if (max) lilv_node_free(max);
    (void)w; (void)defNode;
    return v;
}

int run_plugin(const char* uri, bool tone, bool verbose)
{
    LilvWorld* world = lilv_world_new();
    lilv_world_load_all(world);

    LilvNode* uriNode = lilv_new_uri(world, uri);
    const LilvPlugins* plugins = lilv_world_get_all_plugins(world);
    const LilvPlugin* plugin = lilv_plugins_get_by_uri(plugins, uriNode);
    if (!plugin) {
        std::fprintf(stderr, "lv2_render: plugin not found: %s\n", uri);
        lilv_node_free(uriNode);
        lilv_world_free(world);
        return 2;
    }

    // Classify each port.
    LilvNode* audioPortClass   = lilv_new_uri(world, LV2_CORE__AudioPort);
    LilvNode* controlPortClass = lilv_new_uri(world, LV2_CORE__ControlPort);
    LilvNode* inputPortClass   = lilv_new_uri(world, LV2_CORE__InputPort);
    LilvNode* outputPortClass  = lilv_new_uri(world, LV2_CORE__OutputPort);
    LilvNode* atomPortClass    = lilv_new_uri(world, "http://lv2plug.in/ns/ext/atom#AtomPort");

    const uint32_t nPorts = lilv_plugin_get_num_ports(plugin);
    std::vector<PortInfo> ports(nPorts);

    // Per-port buffers.
    std::vector<std::vector<float>> audioBufs(nPorts);
    std::vector<float>              controlVals(nPorts, 0.0f);
    // Atom ports get a tiny "empty sequence" body so the plugin can
    // safely look at it (DPF's MIDI input port lives here).
    std::vector<std::vector<uint8_t>> atomBufs(nPorts);

    uint32_t nAudioIn = 0, nAudioOut = 0;
    for (uint32_t i = 0; i < nPorts; ++i) {
        const LilvPort* p = lilv_plugin_get_port_by_index(plugin, i);
        const bool isAudio   = lilv_port_is_a(plugin, p, audioPortClass);
        const bool isControl = lilv_port_is_a(plugin, p, controlPortClass);
        const bool isAtom    = lilv_port_is_a(plugin, p, atomPortClass);
        const bool isInput   = lilv_port_is_a(plugin, p, inputPortClass);
        const bool isOutput  = lilv_port_is_a(plugin, p, outputPortClass);

        ports[i].index = i;
        if (isAudio && isInput)        { ports[i].kind = PortInfo::kAudioIn;   ++nAudioIn;  audioBufs[i].assign(kFrames, 0.0f); }
        else if (isAudio && isOutput)  { ports[i].kind = PortInfo::kAudioOut;  ++nAudioOut; audioBufs[i].assign(kFrames, 0.0f); }
        else if (isControl && isInput) { ports[i].kind = PortInfo::kControlIn; controlVals[i] = port_default(world, plugin, i); ports[i].defaultVal = controlVals[i]; }
        else if (isControl && isOutput){ ports[i].kind = PortInfo::kControlOut;controlVals[i] = 0.0f; }
        else if (isAtom && isInput)    { ports[i].kind = PortInfo::kAtomIn;    atomBufs[i].assign(1024, 0); }
        else if (isAtom && isOutput)   { ports[i].kind = PortInfo::kAtomOut;   atomBufs[i].assign(1024, 0); }
        else                           { ports[i].kind = PortInfo::kOther; }
    }

    // Fill audio inputs.
    for (uint32_t i = 0; i < nPorts; ++i) {
        if (ports[i].kind != PortInfo::kAudioIn) continue;
        if (tone) {
            // 1 kHz sine, -12 dBFS.
            constexpr float freq = 1000.0f;
            constexpr float amp  = 0.25f;
            for (uint32_t f = 0; f < kFrames; ++f)
                audioBufs[i][f] = std::sin(2.0f * 3.14159265358979f * freq * f / kSampleRate) * amp;
        }
        // else: silence — leaves DSP idle, exercises bypass paths.
    }

    LilvInstance* instance = lilv_plugin_instantiate(plugin, kSampleRate, nullptr);
    if (!instance) {
        std::fprintf(stderr, "lv2_render: instantiate failed: %s\n", uri);
        lilv_node_free(uriNode);
        lilv_node_free(audioPortClass);
        lilv_node_free(controlPortClass);
        lilv_node_free(inputPortClass);
        lilv_node_free(outputPortClass);
        lilv_node_free(atomPortClass);
        lilv_world_free(world);
        return 3;
    }

    // Wire ports.
    for (uint32_t i = 0; i < nPorts; ++i) {
        switch (ports[i].kind) {
            case PortInfo::kAudioIn:
            case PortInfo::kAudioOut:
                lilv_instance_connect_port(instance, i, audioBufs[i].data());
                break;
            case PortInfo::kControlIn:
            case PortInfo::kControlOut:
                lilv_instance_connect_port(instance, i, &controlVals[i]);
                break;
            case PortInfo::kAtomIn:
            case PortInfo::kAtomOut:
                // Initialise with an empty Atom Sequence header
                // (size=0, type=0). DPF's MIDI loop iterates over
                // the body bytes, so an empty body is fine.
                lilv_instance_connect_port(instance, i, atomBufs[i].data());
                break;
            default:
                lilv_instance_connect_port(instance, i, nullptr);
                break;
        }
    }

    lilv_instance_activate(instance);
    lilv_instance_run(instance, kFrames);
    lilv_instance_deactivate(instance);

    // Inspect outputs.
    float peak = 0.0f;
    double sumsq = 0.0;
    uint32_t totalSamples = 0;
    bool nan_or_inf = false;
    for (uint32_t i = 0; i < nPorts; ++i) {
        if (ports[i].kind != PortInfo::kAudioOut) continue;
        for (uint32_t f = 0; f < kFrames; ++f) {
            const float v = audioBufs[i][f];
            if (!std::isfinite(v)) { nan_or_inf = true; continue; }
            const float a = std::fabs(v);
            if (a > peak) peak = a;
            sumsq += double(v) * double(v);
            ++totalSamples;
        }
    }

    const double rms = totalSamples ? std::sqrt(sumsq / totalSamples) : 0.0;

    const char* tag;
    int         rc;
    if (nan_or_inf)               { tag = "NAN  "; rc = 1; }
    else if (nAudioOut == 0)      { tag = "NO_OUT"; rc = 0; }
    else if (peak < 1e-7f)        { tag = "SILENT"; rc = 0; }
    else if (peak > 4.0f)         { tag = "CLIP "; rc = 1; }
    else                          { tag = "OK   "; rc = 0; }

    std::printf("%s  %s  in=%u out=%u peak=%.4f rms=%.4f\n",
                tag, uri, nAudioIn, nAudioOut, peak, rms);
    (void)verbose;

    lilv_instance_free(instance);
    lilv_node_free(uriNode);
    lilv_node_free(audioPortClass);
    lilv_node_free(controlPortClass);
    lilv_node_free(inputPortClass);
    lilv_node_free(outputPortClass);
    lilv_node_free(atomPortClass);
    lilv_world_free(world);
    return rc;
}

} // namespace

int main(int argc, char** argv)
{
    bool tone = false;
    bool verbose = false;
    const char* uri = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--tone") == 0) tone = true;
        else if (std::strcmp(argv[i], "-v") == 0) verbose = true;
        else uri = argv[i];
    }
    if (!uri) {
        std::fprintf(stderr,
            "usage: lv2_render [--tone] [-v] <lv2-plugin-uri>\n"
            "Loads the plugin via Lilv, drives %u frames at %u Hz through it,\n"
            "and reports per-output peak/RMS. --tone uses a 1 kHz sine input.\n",
            kFrames, kSampleRate);
        return 64;
    }
    return run_plugin(uri, tone, verbose);
}
