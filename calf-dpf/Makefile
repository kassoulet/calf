#!/usr/bin/make -f
# Top-level Makefile for the Calf DPF port.
# Iterates each subdir under plugins/.

PLUGINS = Hello

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C plugins/$@

clean:
	@for p in $(PLUGINS); do $(MAKE) -C plugins/$$p clean; done

.PHONY: all clean $(PLUGINS)
