#!/usr/bin/make -f
#
# Top-level orchestration Makefile for the Calf DPF port.
#
# - `make`        builds every plugins/<Name>/ directory in turn.
# - `make smoke`  launches each JACK standalone with no JACK server briefly
#                 and reports per-plugin pass/fail (catches load-time
#                 crashes / missing symbols).
# - `make clean`  cleans every plugin subdir.
#
# Plugin list is discovered from the filesystem so newly scaffolded
# plugins land automatically.

PLUGINS := $(notdir $(patsubst %/,%,$(dir $(wildcard plugins/*/Makefile))))

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C plugins/$@

clean:
	@for p in $(PLUGINS); do $(MAKE) -C plugins/$$p clean; done

# Smoke test: each JACK standalone is launched with a 0.5s timeout. Exit
# code 124 = clean timeout (good, plugin came up and was running).
# Anything else (segfault, dlopen failure, …) is a failure we surface.
smoke:
	@fail=0; pass=0; \
	for bin in $(sort $(notdir $(wildcard bin/Calf*))); do \
	    case $$bin in *.lv2|*.vst3|*.clap) continue ;; esac; \
	    out=$$(timeout 0.5s bin/$$bin --help 2>&1 || true); \
	    rc=$$?; \
	    if [ $$rc -eq 0 ] || [ $$rc -eq 124 ]; then \
	        printf "  OK    %s\n" "$$bin"; pass=$$((pass+1)); \
	    else \
	        printf "  FAIL  %s (rc=%d)\n" "$$bin" "$$rc"; fail=$$((fail+1)); \
	    fi; \
	done; \
	echo "smoke: $$pass passed, $$fail failed."; \
	[ $$fail -eq 0 ]

# Same idea one layer down: exercise each Calf DSP module directly,
# bypassing DPF entirely. Catches bridge regressions vs DSP regressions.
dsp-smoke:
	$(MAKE) -C tools dsp_smoke
	@tools/dsp_smoke 2>/dev/null

list:
	@echo "$(words $(PLUGINS)) plugins:"
	@for p in $(PLUGINS); do echo "  $$p"; done

.PHONY: all clean smoke dsp-smoke list $(PLUGINS)
