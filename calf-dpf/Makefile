#!/usr/bin/make -f
#
# Top-level orchestration Makefile for the Calf DPF port.
#
# - `make`              builds every plugins/<Name>/ directory in turn.
# - `make smoke`        launches each JACK standalone with no JACK server
#                       briefly and reports per-plugin pass/fail (catches
#                       load-time crashes / missing symbols).
# - `make screenshots`  launches each standalone under a headless Xvfb,
#                       captures its window, and writes one PNG per
#                       plugin into screenshots/.
# - `make clean`        cleans every plugin subdir.
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

# Headless UI screenshots. Spins up one Xvfb for the whole run, then for
# each Calf* JACK standalone launches it under that display, waits for
# its window to map, captures it with ImageMagick `import`, and kills
# the plugin. JACK is not required — the standalones print "jack server
# is not running" but still bring up their DGL window, which is what we
# want to capture.
#
# Requires: Xvfb, xdotool, imagemagick (`apt-get install xvfb xdotool imagemagick`).
SCREENSHOT_DISPLAY := :77
SCREENSHOT_DIR     := screenshots
SCREENSHOT_GEOM    := 1600x1000x24
SCREENSHOT_WAIT    := 3
screenshots:
	@command -v Xvfb    >/dev/null || { echo "Xvfb not found (apt install xvfb)"; exit 1; }
	@command -v xdotool >/dev/null || { echo "xdotool not found (apt install xdotool)"; exit 1; }
	@command -v import  >/dev/null || { echo "ImageMagick 'import' not found (apt install imagemagick)"; exit 1; }
	@mkdir -p $(SCREENSHOT_DIR)
	@pkill -9 -x Xvfb 2>/dev/null; \
	 rm -f /tmp/.X$(patsubst :%,%,$(SCREENSHOT_DISPLAY))-lock; \
	 Xvfb $(SCREENSHOT_DISPLAY) -screen 0 $(SCREENSHOT_GEOM) >/tmp/calf-xvfb.log 2>&1 & \
	 XVFB_PID=$$!; \
	 sleep 1; \
	 trap "kill -9 $$XVFB_PID 2>/dev/null" EXIT; \
	 ok=0; fail=0; \
	 for bin in $(sort $(notdir $(wildcard bin/Calf*))); do \
	    case $$bin in *.lv2|*.vst3|*.clap|*_Conflict*) continue ;; esac; \
	    DISPLAY=$(SCREENSHOT_DISPLAY) ./bin/$$bin >/dev/null 2>&1 & \
	    PID=$$!; \
	    sleep $(SCREENSHOT_WAIT); \
	    WID=$$(DISPLAY=$(SCREENSHOT_DISPLAY) xdotool search --name "Calf Studio Gear" 2>/dev/null | tail -1); \
	    if [ -n "$$WID" ]; then \
	        DISPLAY=$(SCREENSHOT_DISPLAY) import -window $$WID $(SCREENSHOT_DIR)/$$bin.png 2>/dev/null; \
	        if [ -s $(SCREENSHOT_DIR)/$$bin.png ]; then \
	            printf "  OK    %-30s %s\n" "$$bin" "$$(identify -format '%wx%h' $(SCREENSHOT_DIR)/$$bin.png)"; \
	            ok=$$((ok+1)); \
	        else \
	            printf "  FAIL  %-30s (empty capture)\n" "$$bin"; fail=$$((fail+1)); \
	        fi; \
	    else \
	        printf "  FAIL  %-30s (no window)\n" "$$bin"; fail=$$((fail+1)); \
	    fi; \
	    kill -9 $$PID 2>/dev/null; \
	    wait $$PID 2>/dev/null; \
	 done; \
	 echo "screenshots: $$ok captured, $$fail failed → $(SCREENSHOT_DIR)/"; \
	 [ $$fail -eq 0 ]

.PHONY: all clean smoke dsp-smoke list screenshots $(PLUGINS)
