/*
 * Phase 3 smoke test — Calf configure() roundtrip.
 *
 * Verifies that a Monosynth modmatrix row written via configure() comes
 * back through send_configures() with the same key/value. This is the
 * pure-DSP version of the path that DPF setState → Calf, getState ←
 * Calf will follow at host level.
 */
#include <calf/modules_synths.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

using namespace calf_plugins;

struct Captor : send_configure_iface {
    std::vector<std::pair<std::string, std::string>> seen;
    void send_configure(const char* k, const char* v) override
    {
        seen.emplace_back(k, v ? v : "");
    }
};

int main()
{
    monosynth_audio_module mod;
    float dummy[monosynth_metadata::param_count] = {0};
    for (int i = 0; i < monosynth_metadata::param_count; ++i)
        mod.params[i] = &dummy[i];

    mod.post_instantiate(48000);
    mod.set_sample_rate(48000);

    // Write column 3 (the "amount" float) on row 3.
    char* err = mod.configure("mod_matrix:3,3", "0.42");
    std::printf("configure returned: %s\n", err ? err : "(ok)");

    // Read it back through send_configures.
    Captor cap;
    mod.send_configures(&cap);

    bool found = false;
    for (const auto& kv : cap.seen) {
        if (kv.first == "mod_matrix:3,3") {
            std::printf("roundtrip: %s = '%s'\n", kv.first.c_str(), kv.second.c_str());
            // Calf reformats floats (e.g. "0.420000"). Substring match.
            if (kv.second.find("0.42") != std::string::npos) found = true;
            break;
        }
    }
    std::printf("total state keys: %zu\n", cap.seen.size());
    return found ? 0 : 1;
}
