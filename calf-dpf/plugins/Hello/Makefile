#!/usr/bin/make -f

NAME = CalfHello

FILES_DSP = HelloPlugin.cpp

# Land artifacts inside calf-dpf/ instead of the legacy tree root.
DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR  = ../../build/Hello

include ../../../dpf/Makefile.plugins.mk

TARGETS  = jack
TARGETS += lv2_dsp
TARGETS += vst3
TARGETS += clap

DPF_DIR = ../../../dpf
LV2_GEN = $(DPF_DIR)/utils/lv2_ttl_generator

all: $(TARGETS) lv2_ttl

$(LV2_GEN):
	$(MAKE) -C $(DPF_DIR)/utils/lv2-ttl-generator

lv2_ttl: $(LV2_GEN) lv2_dsp
	cd ../.. && ../dpf/utils/generate-ttl.sh bin

.PHONY: lv2_ttl
