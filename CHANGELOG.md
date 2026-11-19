# Changelog

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
- Offline single-file version that runs from disk. Donate button. Exact credit line. GPL-3.0.

## 0.x · prototype as delivered

The original prototype (commit `bea4cc0`).
