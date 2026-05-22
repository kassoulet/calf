#!/usr/bin/make -f
# Build host-side helpers (smoke test, etc.).

CALF_DSP_DIR := ../dsp

CXX      ?= g++
PKG_CFLAGS := $(shell pkg-config --cflags fluidsynth expat)
PKG_LIBS   := $(shell pkg-config --libs   fluidsynth expat)

CXXFLAGS += -std=c++17 -O2 -Wall \
            -I$(CALF_DSP_DIR)/include -I$(CALF_DSP_DIR) \
            $(PKG_CFLAGS) -DENABLE_EXPERIMENTAL=1

LDFLAGS  += $(CALF_DSP_DIR)/libcalfdsp.a $(PKG_LIBS) -lpthread

LILV_CFLAGS := $(shell pkg-config --cflags lilv-0 2>/dev/null)
LILV_LIBS   := $(shell pkg-config --libs   lilv-0 2>/dev/null)

all: dsp_smoke lv2_render

dsp_smoke: dsp_smoke.cpp $(CALF_DSP_DIR)/libcalfdsp.a
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

# lv2_render does not need libcalfdsp — it loads the plugins as LV2
# bundles, the same way a real host would. Just needs Lilv.
lv2_render: lv2_render.cpp
	$(CXX) -std=c++17 -O2 -Wall $(LILV_CFLAGS) $< $(LILV_LIBS) -o $@

$(CALF_DSP_DIR)/libcalfdsp.a:
	$(MAKE) -C $(CALF_DSP_DIR)

clean:
	rm -f dsp_smoke lv2_render

.PHONY: all clean
