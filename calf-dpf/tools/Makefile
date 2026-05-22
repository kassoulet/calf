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

all: dsp_smoke

dsp_smoke: dsp_smoke.cpp $(CALF_DSP_DIR)/libcalfdsp.a
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

$(CALF_DSP_DIR)/libcalfdsp.a:
	$(MAKE) -C $(CALF_DSP_DIR)

clean:
	rm -f dsp_smoke

.PHONY: all clean
