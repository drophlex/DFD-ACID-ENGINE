# Status · real state, no promises

Updated 2026-11-18 (1.1).

| Part | State | Tested how |
|---|---|---|
| Engine (`engine/dfd_acid.h`) | ✅ working | 30 native checks (`tests/engine_test.cpp`): pitch, timing, ratchets, probability, ties, slides, conditions, swing, micro-timing, morph, host sync, fuzz at 44.1 / 48 / 96 kHz, CPU |
| Web app (`web/`) | ✅ working | 12 Node tests against the real WebAssembly engine + browser E2E (Chromium): sound, playhead, MIDI and WAV export, no console errors |
| Offline file | ✅ working | opened from disk in Chromium, plays |
| TB-3 link | ⚠ code ready, **not tested on a real TB-3** | DEMO simulator test only. Messages follow the official TB-3 MIDI implementation. |
| VST3 / AU / app | 🧪 beta | built by CI for Mac (universal) and Windows x64; pluginval strictness 8 passes on Mac, Windows and Linux; auval passes; the plug-in page tested with a stand-in JUCE backend (messages, restore, save, drag). **Not yet tested by hand in a DAW.** |
| FM-1 firmware | ✅ working | host tests: the 1152 web patterns note for note, every control, power-on recovery, 40 screen states, the 19 sounds (no clipping, accent, zap, pump), Felucca's full suite; built by CI with the JieLi toolchain. **Tested by the owner on a real FM-1 (2026-11-18): installs, runs, works.** |

Pending: tests by the owner (plug-in in Ableton, a real TB-3); The Firmware Hub submission of the FM-1 firmware (identity to check); TB-3 named FX parameters per type.
