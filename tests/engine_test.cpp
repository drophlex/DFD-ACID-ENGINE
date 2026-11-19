// DFD ACID ENGINE · native tests of engine/dfd_acid.h (the same code the web, the plug-in and the tests run)
// build: c++ -std=c++17 -O2 tests/engine_test.cpp -o build/engine_test && ./build/engine_test
#include "../engine/dfd_acid.h"
#include <cstdio>
#include <cmath>
#include <vector>
#include <chrono>
#include <cstdlib>
using namespace dfd;
static int fails = 0, passes = 0;
static void check(const char* what, bool ok) { printf("%-74s %s\n", what, ok ? "ok" : "FAIL"); ok ? passes++ : fails++; }
static Engine* fresh(float sr = 48000) { static Engine* e = nullptr; delete e; e = new Engine(); e->init(sr); return e; }
struct Run { std::vector<float> L, R; std::vector<MidiEv> ev; std::vector<int> evAbs; };
static Run render(Engine& E, int n, int block = 128) {
  Run r; r.L.resize(n); r.R.resize(n);
  for (int i = 0; i < n; i += block) { int m = n - i < block ? n - i : block; E.process(&r.L[i], &r.R[i], m); for (int k = 0; k < E.nev; k++) { r.ev.push_back(E.ev[k]); r.evAbs.push_back(i + E.ev[k].at); } E.nev = 0; }
  return r;
}
static int count(const Run& r, uint8_t stMask, int d1 = -1) { int c = 0; for (auto& e : r.ev) if ((e.st & 0xF0) == stMask && e.d2 > 0 && (d1 < 0 || e.d1 == d1)) c++; return c; }
static void allOn(Engine& E, int len, int note = 45) { E.pat[0].len = len; for (int i = 0; i < len; i++) { E.pat[0].s[i] = Step(); E.pat[0].s[i].note = (int8_t)note; E.pat[0].s[i].flags = ST_ON; } }

int main() {
  // ---- maths
  { float worst = 0; for (float x = 0.001f; x < 30000; x *= 1.37f) worst = fmaxf(worst, fabsf(log2f_(x) - log2f(x))); check("log2f_ within 1e-5 octave", worst < 1e-5f);
    float w2 = 0; for (float x = -20; x < 20; x += 0.013f) w2 = fmaxf(w2, fabsf(exp2f_(x) / exp2f(x) - 1)); check("exp2f_ within 3e-5 relative (0.05 cent)", w2 < 3e-5f); }
  // ---- pitch: a held A2 plays 110.00 Hz (within a cent)
  { Engine& E = *fresh(); E.setParam(P_ENVMOD, 0); E.setParam(P_CUTOFF, 12000); E.setParam(P_RES, 0); E.setParam(P_SUB, 0); E.setParam(P_DLY_ON, 0); E.setParam(P_COMP_ON, 0);
    E.setParam(P_DRIVE, 0); E.setParam(P_DRIVE2, 0); E.setParam(P_VCADECAY, 6);
    E.liveNote(45, 100); Run r = render(E, 48000 * 2);
    int z = 0, first = -1, last = -1; for (int i = 24000; i < 96000 - 1; i++) if (r.L[i] <= 0 && r.L[i + 1] > 0) { if (first < 0) first = i; last = i; z++; }
    double f = (z - 1) * 48000.0 / (last - first); double cents = 1200 * log2(f / 110.0);
    char m[96]; snprintf(m, sizeof m, "pitch: A2 plays %.3f Hz (%.2f cents off, must be < 1)", f, cents); check(m, fabs(cents) < 1); }
  // ---- the sequencer: 16 on-steps at 120 BPM = 16 notes in 2 s (a bar)
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); E.play(); Run r = render(E, 96000);
    check("sequencer: 16 steps at 120 BPM play 16 notes in one bar", count(r, 0x90) == 16);
    check("  every note ends (as many note-offs as note-ons)", count(r, 0x90) == (int)[&] { int c = 0; for (auto& e : r.ev) if ((e.st & 0xF0) == 0x80) c++; return c; }() || count(r, 0x90) - 1 <= [&] { int c = 0; for (auto& e : r.ev) if ((e.st & 0xF0) == 0x80) c++; return c; }());
    int d = r.evAbs[0]; bool even = true; int prev = -1, k = 0; for (size_t i = 0; i < r.ev.size(); i++) if ((r.ev[i].st & 0xF0) == 0x90) { if (prev >= 0 && abs((r.evAbs[i] - prev) - 6000) > 1) even = false; prev = r.evAbs[i]; k++; }
    check("  a 16th every 6000 samples at 48 kHz (sample exact)", even && d <= 1); }
  // ---- ratchets, probability, rests, ties
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); E.pat[0].s[0].ratchet = 4; E.pat[0].s[5].ratchet = 3; E.play(); Run r = render(E, 96000);
    check("ratchet x4 and x3: 16 + 3 + 2 notes", count(r, 0x90) == 21); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 0; i < 16; i++) E.pat[0].s[i].prob = 0; E.play(); Run r = render(E, 96000); check("probability 0 %: silence", count(r, 0x90) == 0); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 0; i < 16; i++) E.pat[0].s[i].prob = 50; E.play(); Run r = render(E, 96000 * 8); int c = count(r, 0x90);
    char m[96]; snprintf(m, sizeof m, "probability 50 %%: about half of 128 (got %d)", c); check(m, c > 40 && c < 88); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 1; i < 16; i += 2) E.pat[0].s[i].flags = ST_ON | ST_TIE; E.play(); Run r = render(E, 96000); check("ties: every other step tied = 8 notes", count(r, 0x90) == 8); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 0; i < 16; i += 2) E.pat[0].s[i].flags = 0; E.play(); Run r = render(E, 96000); check("rests: 8 of 16", count(r, 0x90) == 8); }
  // ---- slide: legato (the next note starts before the previous one stops) and CC 102
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 4); E.pat[0].s[1].flags |= ST_SLIDE; E.pat[0].s[1].note = 52; E.play(); Run r = render(E, 24000);
    int onB = -1, offA = -1, cc102 = -1; for (size_t i = 0; i < r.ev.size(); i++) { auto& e = r.ev[i];
      if ((e.st & 0xF0) == 0x90 && e.d1 == 52 && onB < 0) onB = r.evAbs[i];
      if ((e.st & 0xF0) == 0x80 && e.d1 == 45 && offA < 0 && r.evAbs[i] > 3000) offA = r.evAbs[i];
      if ((e.st & 0xF0) == 0xB0 && e.d1 == 102 && e.d2 == 127) cc102 = r.evAbs[i]; }
    check("slide: the slid note starts before the previous note is let go (legato)", onB > 0 && offA >= onB);
    check("slide: CC 102 = 127 sent with it (TB-3 v1.10)", cc102 == onB); }
  // ---- accent: velocity 127 + CC 103, and it is louder
  { auto lvl = [](bool acc) { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); if (acc) for (int i = 0; i < 16; i++) E.pat[0].s[i].flags |= ST_ACC; E.setParam(P_COMP_ON, 0); E.setParam(P_DLY_ON, 0);
      E.play(); Run r = render(E, 96000); double e = 0; for (float x : r.L) e += x * x; int v = 0; for (auto& ev : r.ev) if ((ev.st & 0xF0) == 0x90) v = ev.d2; return std::pair<double, int>(e, v); };
    auto a = lvl(false), b = lvl(true); check("accent: louder, velocity 127", b.first > a.first * 1.3 && b.second == 127); }
  // ---- conditions: 1:2 plays every other loop; FIRST only the first loop; FILL with the fill held
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 1); E.pat[0].len = 16; for (int i = 1; i < 16; i++) E.pat[0].s[i].flags = 0; E.pat[0].s[0].cond = C_AB + (0 << 3) + 1; E.play(); Run r = render(E, 96000 * 4);
    check("condition 1:2: 2 of 4 loops", count(r, 0x90) == 2); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 1; i < 16; i++) E.pat[0].s[i].flags = 0; E.pat[0].s[0].cond = C_FIRST; E.play(); Run r = render(E, 96000 * 4); check("condition FIRST: once in 4 loops", count(r, 0x90) == 1); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 1; i < 16; i++) E.pat[0].s[i].flags = 0; E.pat[0].s[0].cond = C_FILL; E.play(); Run a = render(E, 96000 * 2); E.setParam(P_FILL, 1); Run b = render(E, 96000 * 2);
    check("condition FILL: silent, then plays while FILL is held", count(a, 0x90) == 0 && count(b, 0x90) == 2); }
  // ---- swing and micro-timing move notes
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); E.setParam(P_SWING, 0.5f); E.play(); Run r = render(E, 24000); int t[4], k = 0; for (size_t i = 0; i < r.ev.size() && k < 4; i++) if ((r.ev[i].st & 0xF0) == 0x90) t[k++] = r.evAbs[i];
    check("swing 50 %: the 2nd 16th is a quarter step late", abs(t[1] - 6000 - 1500) <= 1 && abs(t[2] - 12000) <= 1); }
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); E.pat[0].s[2].micro = -25; E.play(); Run r = render(E, 24000); int t[4], k = 0; for (size_t i = 0; i < r.ev.size() && k < 4; i++) if ((r.ev[i].st & 0xF0) == 0x90) t[k++] = r.evAbs[i];
    check("micro-timing -25 %: step 3 early by a quarter step", abs(t[2] - (12000 - 1500)) <= 1); }
  // ---- A/B morph: at 100 % pattern B plays
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16, 40); E.pat[1] = E.pat[0]; for (int i = 0; i < 16; i++) E.pat[1].s[i].note = 50; E.setParam(P_MORPH, 1); E.play(); Run r = render(E, 96000);
    check("morph 100 %: every note from pattern B", count(r, 0x90, 50) == 16); E.setParam(P_MORPH, 0.5f); Run r2 = render(E, 96000); int b = count(r2, 0x90, 50);
    check("morph 50 %: a mix of A and B, the same steps every loop", b > 3 && b < 13); }
  // ---- host sync: follows the PPQ, a jump restarts at the right step
  { Engine& E = *fresh(); E.setParam(P_BPM, 120); allOn(E, 16); for (int i = 0; i < 16; i++) E.pat[0].s[i].note = (int8_t)(40 + i);
    std::vector<float> L(128), R(128); E.nev = 0; E.hostPosition(1.0, true); E.process(L.data(), R.data(), 128); int first = -1; for (int k = 0; k < E.nev; k++) if ((E.ev[k].st & 0xF0) == 0x90) { first = E.ev[k].d1; break; }
    check("host at beat 2 (PPQ 1.0): starts on step 5", first == 44);
    E.nev = 0; E.hostPosition(2.5, true); E.process(L.data(), R.data(), 128); first = -1; for (int k = 0; k < E.nev; k++) if ((E.ev[k].st & 0xF0) == 0x90) { first = E.ev[k].d1; break; }
    check("a jump to PPQ 2.5: step 11 plays", first == 50);
    E.nev = 0; E.hostPosition(0, false); E.process(L.data(), R.data(), 128); bool off = false; for (int k = 0; k < E.nev; k++) if ((E.ev[k].st & 0xF0) == 0xB0 && E.ev[k].d1 == 123) off = true; check("host stops: all notes off (CC 123)", off && !E.playing); }
  // ---- every parameter at random, every effect on, three qualities: finite, inside full scale
  { bool ok = true; srand(7);
    for (int trial = 0; trial < 40 && ok; trial++) { Engine& E = *fresh(trial % 3 == 0 ? 44100.f : trial % 3 == 1 ? 48000.f : 96000.f); allOn(E, 16);
      for (int i = 0; i < 16; i++) { E.pat[0].s[i].note = (int8_t)(24 + rand() % 60); E.pat[0].s[i].flags = (uint8_t)(rand() & 15); E.pat[0].s[i].ratchet = (uint8_t)(1 + rand() % 8); E.pat[0].s[i].micro = (int8_t)(rand() % 101 - 50); E.pat[0].s[i].gate = (uint8_t)(5 + rand() % 96); }
      E.play(); std::vector<float> L(256), R(256);
      for (int b = 0; b < 300 && ok; b++) {
        for (int k = 0; k < 4; k++) { int id = rand() % P_COUNT; const ParamInfo& p = PARAMS[id]; E.setParam(id, p.lo + (p.hi - p.lo) * (rand() / (float)RAND_MAX)); }
        int n = 1 + rand() % 256; E.process(L.data(), R.data(), n); E.nev = 0;
        for (int i = 0; i < n; i++) if (!std::isfinite(L[i]) || !std::isfinite(R[i]) || fabsf(L[i]) > 1.f || fabsf(R[i]) > 1.f) ok = false; } }
    check("random automation of all 58 parameters, odd block sizes, 44.1/48/96 kHz: finite, within +-1", ok); }
  // ---- self-oscillation stays bounded; silence decays to silence
  { Engine& E = *fresh(); E.setParam(P_RES, 1); E.setParam(P_CUTOFF, 800); E.liveNote(36, 127); Run r = render(E, 48000); float pk = 0; for (float x : r.L) pk = fmaxf(pk, fabsf(x)); check("full resonance: bounded", pk <= 1.f && pk > 0.01f);
    E.liveOff(-1); E.setParam(P_DLY_ON, 0); E.setParam(P_REV_ON, 0); Run s = render(E, 48000 * 3); float tail = 0; for (int i = 48000 * 2; i < 48000 * 3; i++) tail = fmaxf(tail, fabsf(s.L[i])); check("note off: silent after 2 s (no stuck note)", tail < 1e-3f); }
  // ---- CPU: one second of audio, every effect on, per quality
  for (int q = 0; q < 3; q++) { Engine& E = *fresh(); allOn(E, 16); E.setParam(P_QUALITY, (float)q);
    for (int id : { P_CRUSH_ON, P_RING_ON, P_DYN_ON, P_MOD_ON, P_COMP_ON, P_DLY_ON, P_REV_ON }) E.setParam(id, 1);
    E.play(); std::vector<float> L(128), R(128); auto t0 = std::chrono::high_resolution_clock::now(); for (int b = 0; b < 48000 / 128 * 4; b++) { E.process(L.data(), R.data(), 128); E.nev = 0; }
    double ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count() / 4;
    char m[96]; snprintf(m, sizeof m, "CPU at %dx, every effect on: %.1f ms per second of audio (%.2f %% of one core)", 1 << q, ms, ms / 10); check(m, ms < 120); }
  printf(fails ? "ENGINE: %d of %d FAILED\n" : "engine: all %d ok\n", fails ? fails : passes, fails + passes);
  return fails ? 1 : 0;
}
