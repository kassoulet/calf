/*
 * Calf DPF port — Hello sanity-check plugin.
 *
 * Verifies that the DPF submodule, build glue, and target format
 * pipeline (LV2 / VST3 / CLAP / JACK) work before any real Calf
 * DSP is touched.
 */

#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND   "CalfDPFClaude"
#define DISTRHO_PLUGIN_NAME    "Hello"
#define DISTRHO_PLUGIN_URI     "https://calf-studio-gear.org/plugins/hello"
#define DISTRHO_PLUGIN_CLAP_ID "org.calf-studio-gear.hello"

#define DISTRHO_PLUGIN_BRAND_ID  CalfDPFClaude
#define DISTRHO_PLUGIN_UNIQUE_ID cHel

#define DISTRHO_PLUGIN_HAS_UI       0
#define DISTRHO_PLUGIN_IS_RT_SAFE   1
#define DISTRHO_PLUGIN_NUM_INPUTS   2
#define DISTRHO_PLUGIN_NUM_OUTPUTS  2
#define DISTRHO_PLUGIN_WANT_STATE   0

#endif
