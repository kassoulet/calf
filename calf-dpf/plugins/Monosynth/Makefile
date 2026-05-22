#!/usr/bin/make -f

NAME = CalfMonosynth

FILES_DSP = MonosynthPlugin.cpp

CALF_DSP_DIR  = ../../dsp
CALF_BRIDGE   = ../../bridge
BUILD_CXX_FLAGS_EXTRA  = -I$(CALF_DSP_DIR)/include -I$(CALF_DSP_DIR) -I$(CALF_BRIDGE)
BUILD_CXX_FLAGS_EXTRA += $(shell pkg-config --cflags fluidsynth expat)
EXTRA_DSP_LIBS = $(CALF_DSP_DIR)/libcalfdsp.a $(shell pkg-config --libs fluidsynth expat) -lpthread

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR  = ../../build

include ../../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += $(BUILD_CXX_FLAGS_EXTRA)

TARGETS  = jack
TARGETS += lv2_dsp
TARGETS += vst3
TARGETS += clap

DPF_DIR = ../../../dpf
LV2_GEN = $(DPF_DIR)/utils/lv2_ttl_generator

all: dspdep $(TARGETS) lv2_ttl

dspdep:
	$(MAKE) -C $(CALF_DSP_DIR)

$(LV2_GEN):
	$(MAKE) -C $(DPF_DIR)/utils/lv2-ttl-generator

lv2_ttl: $(LV2_GEN) lv2_dsp
	cd ../.. && ../dpf/utils/generate-ttl.sh bin

.PHONY: lv2_ttl dspdep
