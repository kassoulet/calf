/* Calf DSP plugin pack — DPF port.
 *
 * This file instantiates per-module static template members that are
 * declared in metadata.cpp / individual module headers. The original
 * upstream file also defined LADSPA/LV2/DSSI entry points; those are
 * dropped here because the DPF adapter layer under calf-dpf/plugins/
 * instantiates the modules directly by class name.
 */
#include <config.h>
#include <calf/modules_tools.h>
#include <calf/modules_delay.h>
#include <calf/modules_comp.h>
#include <calf/modules_limit.h>
#include <calf/modules_dist.h>
#include <calf/modules_filter.h>
#include <calf/modules_mod.h>
#include <calf/modules_pitch.h>
#include <calf/modules_synths.h>
#include <calf/organ.h>

using namespace calf_plugins;

#define PER_MODULE_ITEM(name, isSynth, jackname) \
    template<> const char *plugin_metadata<name##_metadata>::port_names[]; \
    template<> parameter_properties plugin_metadata<name##_metadata>::param_props[]; \
    template<> ladspa_plugin_info plugin_metadata<name##_metadata>::plugin_info;

#include <calf/modulelist.h>
