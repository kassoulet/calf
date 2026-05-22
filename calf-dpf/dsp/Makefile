#!/usr/bin/make -f
# libcalfdsp.a — Calf DSP core, host-agnostic.

NAME     = calfdsp
SRCDIR   = src
INCDIR   = include
BUILDDIR = ../build/dsp

CXX      ?= g++
CC       ?= gcc

PKG_CFLAGS := $(shell pkg-config --cflags fluidsynth expat)

# ENABLE_EXPERIMENTAL gates FluidSynth, Wavetable, Pitch, and PsyClipper
# in upstream Calf. They're all functional today, just historically not
# considered "stable" enough for the default LV2 set.
CXXFLAGS += -std=c++11 -O2 -fPIC -ffast-math -Wall \
            -I$(INCDIR) -I. $(PKG_CFLAGS) -DENABLE_EXPERIMENTAL=1
CFLAGS   += -O2 -fPIC -ffast-math -I. $(PKG_CFLAGS) -DENABLE_EXPERIMENTAL=1

SRCS_CXX := $(wildcard $(SRCDIR)/*.cpp)
SRCS_C   := $(wildcard $(SRCDIR)/*.c)
OBJS     := $(SRCS_CXX:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o) \
            $(SRCS_C:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

libcalfdsp.a: $(OBJS)
	ar rcs $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR) libcalfdsp.a

.PHONY: clean
