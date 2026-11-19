# DFD ACID ENGINE / M-VAVE FM-1 TARGET

This directory contains the portable **C synthesis and 64-step sequencer core**, not flashable firmware. Source files compile without a DAW and can be integrated with an FM-1 platform audio callback and MIDI interface.

## Current status

- ✅ `acid_core.c` exposes `dfd_process_sample()`, `dfd_note()` and MIDI note/CC output hooks.
- ✅ 16-step default pattern, 64-step capacity (hardware-tailored rather than an arbitrary VST limit), saw/sub, resonant filter, distortion, glide/accent.
- ☐ Map FM-1 keyscan, display, flash preset memory, USB MIDI and audio to the selected firmware SDK.
- ☐ Compile against the chosen M-VAVE/fm1_open or FM-1 Core platform toolchain.
- ☐ Test in a firmware simulator and on the user's exact FM-1 before flashing.
- ☐ Provide verified recovery instructions and a signed/reproducible release image.

### Why there is no .fwsc yet

The M-VAVE FM-1 uses a device-specific compiler and firmware packaging system. A portable sound engine is not interchangeable with firmware, and guessing firmware headers is a reliable way to turn a useful synthesizer into an expensive paperweight. Flash only firmware verified for your exact model.

## Bare C build check

`cc -std=c11 -O2 -Wall -Wextra -c acid_core.c -o /tmp/acid_core.o`

## Key platform references

- https://github.com/m-vave/fm1_open
- https://github.com/konsumer/foma
- https://altstrument.org/devices/m-vave-fm-1

One promising option is a modular FM-1 firmware that can host synth modules; such integration needs to follow that platform's module ABI and license rather than copy unrelated code.
