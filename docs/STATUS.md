# Status · real state, no promises

Updated 2026-11-18.

| Part | State | Tested how |
|---|---|---|
| Engine (`engine/dfd_acid.h`) | ✅ working | 30 native checks (`tests/engine_test.cpp`): pitch, timing, ratchets, probability, ties, slides, conditions, swing, micro-timing, morph, host sync, fuzz at 44.1 / 48 / 96 kHz, CPU |
| Web app (`web/`) | ✅ working | 12 Node tests against the real WebAssembly engine + browser E2E (Chromium): sound, playhead, MIDI and WAV export, no console errors |
| Offline file | ✅ working | opened from disk in Chromium, plays |
| TB-3 link | ⚠ code ready, **not tested on a real TB-3** | DEMO simulator test only. Messages follow the official TB-3 MIDI implementation. |
| VST3 / AU | 🛠 in development | — (not built yet on the new engine) |
| FM-1 firmware | 🛠 in development | — (will not be published before a test on the owner's FM-1, with a backup and a way back) |

Pending: TB-3 named FX parameters per type, plugin build + pluginval / auval + CI, FM-1 port, plugin and FM-1 manuals (the web manual is done).
