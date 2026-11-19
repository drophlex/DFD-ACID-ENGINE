# Changelog

## 1.1.0 · 2026-11-18 · DIRTY

- **Two new voices** in the engine: **DIRTY** (a wall of seven detuned saws) and **FIDGET** (two hard-detuned oscillators plus one an octave up),
  with **pitch drop** per note (the electro "zap"), **FILTH** wavefolding and a tempo-locked **sidechain PUMP**.
- **DIRTY LAB**, a special section for filthy electro house: one-button **☠ DIRTY GENERATE**, 6 riff families (octave pump, zap bounce,
  grind 16ths, call & response, stab gate, drop ride), 9 original dirty sounds and 6 big macros (FILTH · PUMP · ZAP · WIDTH · SCREAM · BITE).
- Two new styles: **ELECTRO HOUSE / DIRTY** and **BLOGHOUSE / FIDGET**. Every style now brings its own sound when you pick it.
- **Plug-in** (beta): VST3 · AU · standalone app for Mac (universal) and Windows x64, the page in the system web view over the native engine; host sync, MIDI in / out, every parameter automatable, state with the set, save windows, drag MIDI. pluginval 8 + auval in CI.
- **FM-1 firmware** (beta, untested on hardware): the DFD ACID voice, its own screens, the web generator's patterns note for note, a DIRTY page; installer with backup and way back.
- **Manuals** for the web, the plug-in and the FM-1, English / Spanish / both.
- Generator keeps notes in key at the edges of the range (folds octaves instead of clamping). 2 new factory patterns.

## 1.0.0 · 2026-11-18 · web

- **New engine** `engine/dfd_acid.h`: one header-only C++17 file shared by the web (WebAssembly), the plugin and the FM-1 work.
  Band-limited saw ⇄ square, sub, noise, 4-pole TPT ladder with resonance compensation, accent build-up, log-domain glide,
  pre and post drive with tone, 1× / 2× / 4× oversampling with a 4th-order decimator, soft limiter.
- **8 FX**, tempo-synced: crush, ring mod, stutter, dynamic filter, chorus / flanger / phaser, compressor, ping-pong delay, FDN reverb.
- **Sequencer** at sample resolution: up to 4096 steps (256 in the page), accent, slide, tie, gate, velocity, ratchets 1-8,
  probability, conditions (FILL, !FILL, 1ST, !1ST, A:B), micro-timing, swing, humanize, A / B morph, MIDI events out.
- **Generator** rewritten: 8 genre styles built from rhythm grids and note palettes (no copied riffs), motif + variation + turnaround,
  16 candidates scored for usability, deterministic seeds, DNA lock (notes / rhythm / accents / slides), polyrhythmic accents,
  probability, conditions, gate variation. Mutations stay in the style.
- **Page**: AudioWorklet engine, EASY / EXPERT layers, English by default with Spanish, step inspector, FX rack, PERFORM (XY pad,
  hold pads, transpose, playable keys), undo / redo, factory patterns, local library, project .json v2 (reads v1).
- **Exports**: MIDI rendered from the engine's own events (what you hear is what you get), drag MIDI, WAV 48 kHz / 24-bit, stems.
- **TB-3**: TWIN LINK modes OFF / VIRTUAL / HARDWARE / TWIN, sequencer events sent with audio-aligned timestamps, DEMO simulator,
  restore reads and downloads a safety backup of the current active patch before writing, panic.
- **Manual** (PDF, 17 pages) in English, Spanish and both: `manual/`. Built by `tools/manual/`.
- Offline single-file version that runs from disk. Donate button. Exact credit line. GPL-3.0.

## 0.x · prototype as delivered

The original prototype (commit `bea4cc0`).
