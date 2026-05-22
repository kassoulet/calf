/*
 * Calf DPF port — standalone config.h.
 *
 * Replaces the autotools/cmake-generated header. Intentionally does NOT
 * define USE_LV2 / USE_JACK / USE_LASH so that all host-glue
 * #if blocks compile out. The DSP layer is host-agnostic.
 */
#ifndef CALF_DPF_CONFIG_H
#define CALF_DPF_CONFIG_H

#define PACKAGE_NAME    "calf"
#define PACKAGE_VERSION "0.91.0-dpf"
#define PACKAGE_STRING  "calf 0.91.0-dpf"

#define PKGDOCDIR       "/usr/local/share/doc/calf/"
#define PKGLIBDIR       "/usr/local/share/calf/"

/* USE_LV2, USE_JACK, USE_LASH, ENABLE_EXPERIMENTAL: intentionally undefined */

#endif
