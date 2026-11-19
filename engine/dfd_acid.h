// DFD ACID ENGINE · the sound and the sequencer, one header for every build (web/WASM, VST3/AU, tests)
// DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO · GPL-3.0-or-later
//
// No libc, no allocation, no locks: everything is static inside Engine, sized once. process() is real-time safe.
// A monophonic acid voice (band-limited saw / square, sub, noise; a resonant 4-pole ladder with drive and gain
// compensation; accent, slide and tie the way an acid line needs them), oversampled 1x / 2x / 4x, then a chain of
// effects (crush, ring, stutter, dynamic filter, chorus / flanger / phaser, compressor, delay, reverb) and a limiter.
// The sequencer: up to 4096 steps, notes with gate, tie, slide, accent, velocity, ratchets 1-8, probability,
// conditions (A:B, FILL, !FILL, FIRST, !FIRST), micro-timing, swing, pattern A / B morph, host sync by PPQ.
// It writes the notes it plays (with slide as legato overlap and CC 102 / 103) into an event list for MIDI out.
#pragma once
#include <stdint.h>

namespace dfd {

// ------------------------------------------------------------------ small maths (no <cmath>: the WASM build has no libc)
static constexpr float PI = 3.14159265358979f, TAU = 6.28318530717959f;
inline float fabsf_(float x) { return x < 0 ? -x : x; }
inline float minf(float a, float b) { return a < b ? a : b; }
inline float maxf(float a, float b) { return a > b ? a : b; }
inline float clampf(float x, float a, float b) { return x < a ? a : x > b ? b : x; }
inline int clampi(int x, int a, int b) { return x < a ? a : x > b ? b : x; }
inline float floorf_(float x) { int i = (int)x; return (float)(i - (x < (float)i)); }
inline double floord(double x) { long long i = (long long)x; return (double)(i - (x < (double)i)); }
inline float exp2f_(float x) {   // 2^x: the integer part into the exponent, 2^f on [-0.5, 0.5] by a 6th-order series (~1e-7)
  x = clampf(x, -60.f, 60.f); const float fi = floorf_(x + 0.5f), f = (x - fi) * 0.69314718f;
  const float p = 1.f + f * (1.f + f * (0.5f + f * (0.16666667f + f * (0.04166667f + f * (0.00833333f + f * 0.00138889f)))));
  union { float f; int32_t i; } u; u.i = ((int32_t)fi + 127) << 23; return p * u.f;
}
inline float log2f_(float x) {   // accurate to ~1e-6 octave: log2(m) = 2 atanh((m-1)/(m+1)) / ln 2
  if (x <= 0) return -60.f;
  union { float f; int32_t i; } u; u.f = x;
  const float e = (float)(((u.i >> 23) & 255) - 127); u.i = (u.i & 0x007FFFFF) | 0x3F800000;
  const float m = u.f, t = (m - 1.f) / (m + 1.f), t2 = t * t;
  return e + 2.8853900818f * t * (1.f + t2 * (0.3333333f + t2 * (0.2f + t2 * (0.1428571f + t2 * 0.1111111f))));
}
inline float expf_(float x) { return exp2f_(x * 1.44269504f); }
inline float powf_(float a, float b) { return a <= 0 ? 0 : exp2f_(b * log2f_(a)); }
inline float sinf_(float x) {   // radians, any range
  x = x * 0.15915494f; x -= floorf_(x + 0.5f); float y = x * (8.f - 16.f * fabsf_(x)); return y * (0.776f + 0.224f * fabsf_(y));
}
inline float tanhf_(float x) { if (x > 4.5f) return 1.f; if (x < -4.5f) return -1.f; float x2 = x * x; return x * (27.f + x2) / (27.f + 9.f * x2); }
inline float sqrtf_(float x) { if (x <= 0) return 0; float r = x > 1 ? x : 1; for (int i = 0; i < 8; i++) r = 0.5f * (r + x / r); return r; }
inline float dbToGain(float db) { return exp2f_(db * 0.16609640f); }
inline float mtof(float n) { return 440.f * exp2f_((n - 69.f) / 12.f); }
inline float onepole(float hz, float sr) { return 1.f - expf_(-TAU * hz / sr); }
inline float polyblep(float t, float dt) {
  if (t < dt) { t /= dt; return t + t - t * t - 1.f; }
  if (t > 1.f - dt) { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
  return 0.f;
}
struct Rng { uint32_t s = 0x9E3779B9u; uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; } float uni() { return (next() >> 8) * (1.f / 16777216.f); } float bi() { return uni() * 2.f - 1.f; } };
inline uint32_t hash3(uint32_t a, uint32_t b, uint32_t c) { uint32_t h = a * 0x9E3779B1u ^ (b + 0x7F4A7C15u) * 0x85EBCA77u ^ (c + 0x165667B1u) * 0xC2B2AE3Du; h ^= h >> 15; h *= 0x2C1B3C6Du; h ^= h >> 12; h *= 0x297A2D39u; h ^= h >> 15; return h; }

// ------------------------------------------------------------------ the pattern
enum StepFlags : uint8_t { ST_ON = 1, ST_ACC = 2, ST_SLIDE = 4, ST_TIE = 8 };
enum Cond : uint8_t { C_ALWAYS = 0, C_FILL = 1, C_NOFILL = 2, C_FIRST = 3, C_NOFIRST = 4, C_AB = 16 };   // C_AB + (a-1)*8 + (b-1): play on loop a of every b (a,b 1..8)
struct Step {
  int8_t note = 36;      // MIDI note 0..127
  uint8_t flags = 0;     // StepFlags
  uint8_t gate = 55;     // % of the step 5..100 (100 = to the next step)
  uint8_t vel = 100;     // 1..127
  uint8_t ratchet = 1;   // 1..8 hits in the step
  uint8_t prob = 100;    // % chance 0..100
  uint8_t cond = 0;      // Cond
  int8_t micro = 0;      // -50..50 % of a step
};
static constexpr int MAXSTEPS = 4096;
struct Pattern { int len = 16; Step s[MAXSTEPS]; };

// ------------------------------------------------------------------ parameters (ids are permanent: automation and presets hang on them)
enum P : int {
  P_TUNE, P_OCT, P_WAVE, P_PW, P_SUB, P_NOISE,                       // oscillator: semitones, octave -2..2, saw 0 .. square 1, pulse width, sub level, noise level
  P_CUTOFF, P_RES, P_ENVMOD, P_DECAY, P_ACCENT, P_GLIDE,              // filter Hz, resonance 0..1, env mod 0..1, filter decay s, accent 0..1, slide time s
  P_DRIVE, P_DRIVE2, P_TONE, P_VCADECAY,                               // pre-filter drive 0..1, post drive 0..1, tone -1..1, amp decay s (gate held)
  P_CRUSH_ON, P_CRUSH_BITS, P_CRUSH_RATE, P_CRUSH_MIX,
  P_RING_ON, P_RING_FREQ, P_RING_MIX,
  P_STUT_ON, P_STUT_LEN,                                              // stutter: 0 1/4 1 1/8 2 1/16 3 1/32
  P_DYN_ON, P_DYN_RATE, P_DYN_DEPTH, P_DYN_RES,                        // dynamic filter: rate in steps per cycle 1..64
  P_MOD_ON, P_MOD_TYPE, P_MOD_RATE, P_MOD_DEPTH, P_MOD_FB, P_MOD_MIX,  // 0 chorus 1 flanger 2 phaser; rate Hz
  P_COMP_ON, P_COMP_THR, P_COMP_RATIO, P_COMP_MAKE,
  P_DLY_ON, P_DLY_TIME, P_DLY_FB, P_DLY_TONE, P_DLY_MIX, P_DLY_PING,   // time: 0 1/16 1 1/8 2 3/16 3 1/4 4 3/8 5 1/2 6 1/8T 7 1/4D
  P_REV_ON, P_REV_SIZE, P_REV_DAMP, P_REV_MIX,
  P_VOLUME, P_SWING, P_BPM, P_QUALITY, P_MORPH, P_SEED, P_HUMAN,     // dB, 0..0.75, internal BPM, 0 1x 1 2x 2 4x, A/B morph 0..1, seed, humanize 0..1
  P_TRANSPOSE, P_FILL, P_MUTE, P_AUDIO,                               // semitones -24..24, fill held, mute (notes + MIDI), audio out (0 = MIDI only)
  P_VOICE, P_DETUNE, P_PDROP, P_PDEC, P_FOLD, P_PUMP_ON, P_PUMP,      // voice 0 ACID 1 DIRTY 2 FIDGET; stack detune; pitch drop st, its decay s; wavefold; sidechain pump
  P_COUNT
};
struct ParamInfo { const char* id; const char* name; float lo, hi, def; int steps; };   // steps: 0 continuous, n = n choices
static constexpr ParamInfo PARAMS[P_COUNT] = {
  {"tune","Tune",-12,12,0,0},{"oct","Octave",-2,2,0,5},{"wave","Wave",0,1,0,0},{"pw","Pulse Width",0.05f,0.95f,0.5f,0},{"sub","Sub",0,1,0.15f,0},{"noise","Noise",0,1,0,0},
  {"cutoff","Cutoff",40,12000,520,0},{"res","Resonance",0,1,0.72f,0},{"envmod","Env Mod",0,1,0.6f,0},{"decay","Decay",0.05f,2.5f,0.45f,0},{"accent","Accent",0,1,0.65f,0},{"glide","Slide Time",0.01f,0.4f,0.06f,0},
  {"drive","Drive",0,1,0.25f,0},{"drive2","Overdrive",0,1,0.15f,0},{"tone","Tone",-1,1,0,0},{"vcadecay","Amp Decay",0.1f,6,3,0},
  {"crush_on","Crush",0,1,0,2},{"crush_bits","Crush Bits",2,16,8,0},{"crush_rate","Crush Rate",0,1,0.3f,0},{"crush_mix","Crush Mix",0,1,0.6f,0},
  {"ring_on","Ring Mod",0,1,0,2},{"ring_freq","Ring Freq",20,4000,330,0},{"ring_mix","Ring Mix",0,1,0.4f,0},
  {"stut_on","Stutter",0,1,0,2},{"stut_len","Stutter Length",0,3,2,4},
  {"dyn_on","Dynamic Filter",0,1,0,2},{"dyn_rate","Dyn Rate",1,64,16,0},{"dyn_depth","Dyn Depth",0,1,0.5f,0},{"dyn_res","Dyn Reso",0,1,0.4f,0},
  {"mod_on","Mod FX",0,1,0,2},{"mod_type","Mod Type",0,2,0,3},{"mod_rate","Mod Rate",0.02f,8,0.6f,0},{"mod_depth","Mod Depth",0,1,0.5f,0},{"mod_fb","Mod Feedback",0,0.95f,0.3f,0},{"mod_mix","Mod Mix",0,1,0.5f,0},
  {"comp_on","Compressor",0,1,1,2},{"comp_thr","Comp Threshold",-40,0,-14,0},{"comp_ratio","Comp Ratio",1,20,3,0},{"comp_make","Comp Makeup",0,24,4,0},
  {"dly_on","Delay",0,1,1,2},{"dly_time","Delay Time",0,7,2,8},{"dly_fb","Delay Feedback",0,0.95f,0.38f,0},{"dly_tone","Delay Tone",0,1,0.55f,0},{"dly_mix","Delay Mix",0,1,0.22f,0},{"dly_ping","Ping-Pong",0,1,1,2},
  {"rev_on","Reverb",0,1,0,2},{"rev_size","Reverb Size",0,1,0.6f,0},{"rev_damp","Reverb Damp",0,1,0.5f,0},{"rev_mix","Reverb Mix",0,1,0.18f,0},
  {"volume","Volume",-36,6,-6,0},{"swing","Swing",0,0.75f,0,0},{"bpm","Tempo",30,300,132,0},{"quality","Quality",0,2,1,3},{"morph","A/B Morph",0,1,0,0},{"seed","Seed",0,65535,1234,0},{"human","Humanize",0,1,0,0},
  {"transpose","Transpose",-24,24,0,0},{"fill","Fill",0,1,0,2},{"mute","Mute",0,1,0,2},{"audio","Audio Out",0,1,1,2},
  {"voice","Voice",0,2,0,3},{"detune","Detune",0,1,0.35f,0},{"pdrop","Pitch Drop",0,24,0,0},{"pdec","Drop Time",0.01f,0.5f,0.08f,0},{"fold","Filth",0,1,0,0},{"pump_on","Pump",0,1,0,2},{"pump","Pump Depth",0,1,0.6f,0},
};

// ------------------------------------------------------------------ building blocks
struct Biquad {     // transposed direct form II
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
  void lowpass(float fc, float q, float sr) { float w = TAU * clampf(fc, 10, sr * 0.49f) / sr, s = sinf_(w), c = sinf_(w + PI * 0.5f), al = s / (2 * q), a0 = 1 + al; b0 = (1 - c) * 0.5f / a0; b1 = (1 - c) / a0; b2 = b0; a1 = -2 * c / a0; a2 = (1 - al) / a0; }
  float run(float x) { float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
  void reset() { z1 = z2 = 0; }
};
struct Decimator {  // anti-alias low-pass before dropping samples (2 cascaded biquads = 4th-order Butterworth)
  Biquad a, b;
  void set(float sr_over, int factor) { a.lowpass(sr_over / factor * 0.45f, 0.5412f, sr_over); b.lowpass(sr_over / factor * 0.45f, 1.3066f, sr_over); }
  float run(float x) { return b.run(a.run(x)); }
  void reset() { a.reset(); b.reset(); }
};

// a 4-pole transistor-ladder low-pass, zero-delay feedback (TPT) with a tanh in the loop: stable at any setting,
// self-oscillates near full resonance, and makes up the bass it loses as the resonance rises
struct Ladder {
  float s[4] = {0, 0, 0, 0};
  float run(float x, float fc, float res, float sr) {
    const float w = clampf(fc, 20.f, sr * 0.45f) * PI / sr, g = sinf_(w) / sinf_(w + PI * 0.5f), G = g / (1 + g);   // g = tan(pi fc / sr)
    const float k = res * 4.2f, comp = 1.f + res * 1.6f;
    const float G2 = G * G, G4 = G2 * G2;
    const float S = (s[0] * G2 * G + s[1] * G2 + s[2] * G + s[3]) * (1 - G);   // the feedback estimate (states weighted)
    float u = (x * comp - k * S) / (1 + k * G4);
    u = tanhf_(u);
    for (int i = 0; i < 4; i++) { const float v = (u - s[i]) * G; const float y = v + s[i]; s[i] = y + v; u = y; }
    return u;
  }
  void reset() { s[0] = s[1] = s[2] = s[3] = 0; }
};

// ------------------------------------------------------------------ the engine
static constexpr int MAXOS = 4, MAXBLOCK = 1024, NEV = 512;
static constexpr int DLYMAX = 192000 * 3, REVMAX = 8192, MODMAX = 8192, STUTMAX = 192000 * 2;
struct MidiEv { int at; uint8_t st, d1, d2; };   // at = sample offset in the block

struct Engine {
  float sr = 48000;
  float params[P_COUNT];
  Pattern pat[2];                       // A and B (morph)
  // transport
  bool playing = false, hostSync = false;
  double pos = 0;                       // in 16th steps, since the pattern start
  double nextOnset = 0; long long nextK = 0;
  long long loopCount = 0;
  int curStep = -1;                     // for the page's playhead
  // voice
  double pumpFree = 0; float pumpG = 1;
  float stk[7] = {0.f, 0.13f, 0.29f, 0.41f, 0.57f, 0.71f, 0.87f}; float pEnv = 0;
  float phase = 0, subPhase = 0, freq = 110, freqTarget = 110, logF = 0, logT = 0;
  float meg = 0, vca = 0, vcaTarget = 0, accCap = 0, accNow = 0, velNow = 1;
  bool gateOn = false; double gateOff = -1; int noteNow = -1, hwNote = -1;
  float ampDecay = 1;
  Ladder lad; Decimator dec[2]; float hp = 0;
  Rng noise; float pinkB[3] = {0, 0, 0};
  int os = 2;
  // ratchet / scheduled hits inside the current step
  struct Hit { double at; Step st; bool slideIn; } hits[8]; int nHits = 0;
  // FX state
  float crushHold[2] = {0, 0}, crushPh = 0; float ringPh = 0;
  float dynPh = 0; float dynS[2][2] = {{0, 0}, {0, 0}};
  float modBuf[2][MODMAX]; int modW = 0; float modPh = 0, apZ[2][6] = {{0}}, modLast[2] = {0, 0};
  float compEnv = 0;
  float dly[2][DLYMAX]; int dlyW = 0; float dlyTone[2] = {0, 0}, dlyTime = 0;
  float rv[4][REVMAX]; int rvW = 0, rvLen[4] = {0, 0, 0, 0}; float rvLp[4] = {0, 0, 0, 0};
  float ap[2][1024]; int apW[2] = {0, 0}, apLen[2] = {0, 0};
  float stut[2][STUTMAX]; int stutW = 0, stutPos = 0, stutLen = 0; bool stutHeld = false;
  float lim = 1;
  // telemetry for the page: playing, step, loop, level, cutoff now, accent now
  float tele[8]; float scope[256]; int scopeW = 0;
  // MIDI out of the last process() call
  MidiEv ev[NEV]; int nev = 0; int procAt = 0; int midiCh = 0; bool midiOut = true;
  float lastLevel = 0, toneLp = 0;

  void init(float sampleRate) {
    sr = sampleRate > 8000 ? sampleRate : 48000;
    for (int i = 0; i < P_COUNT; i++) params[i] = PARAMS[i].def;
    for (int p = 0; p < 2; p++) { pat[p].len = 16; for (int i = 0; i < MAXSTEPS; i++) pat[p].s[i] = Step(); }
    reset();
  }
  void reset() {
    phase = subPhase = 0; meg = vca = vcaTarget = accCap = 0; gateOn = false; gateOff = -1; noteNow = -1; nHits = 0;
    lad.reset(); dec[0].reset(); dec[1].reset(); hp = 0;
    for (int c = 0; c < 2; c++) { for (int i = 0; i < MODMAX; i++) modBuf[c][i] = 0; for (int i = 0; i < DLYMAX; i++) dly[c][i] = 0; for (int i = 0; i < 1024; i++) ap[c][i] = 0; for (int i = 0; i < 6; i++) apZ[c][i] = 0; dynS[c][0] = dynS[c][1] = 0; dlyTone[c] = 0; crushHold[c] = 0; modLast[c] = 0; }
    for (int k = 0; k < 4; k++) { for (int i = 0; i < REVMAX; i++) rv[k][i] = 0; rvLp[k] = 0; }
    static const float rvT[4] = {0.0297f, 0.0371f, 0.0411f, 0.0437f}; for (int k = 0; k < 4; k++) rvLen[k] = clampi((int)(rvT[k] * sr), 16, REVMAX - 1);
    apLen[0] = clampi((int)(0.0051f * sr), 8, 1023); apLen[1] = clampi((int)(0.0077f * sr), 8, 1023);
    modW = dlyW = rvW = stutW = 0; apW[0] = apW[1] = 0; compEnv = 0; lim = 1; stutPos = stutLen = 0; stutHeld = false;
    logF = logT = 6.78135971f; freq = freqTarget = 110;
    setQuality((int)params[P_QUALITY]);
  }
  void setQuality(int q) { os = q <= 0 ? 1 : q == 1 ? 2 : 4; dec[0].set(sr * os, os); dec[1].set(sr * os, os); }
  void setParam(int id, float v) {
    if (id < 0 || id >= P_COUNT) return; const ParamInfo& p = PARAMS[id];
    if (!(v == v)) v = p.def; v = clampf(v, p.lo, p.hi); if (p.steps > 1) v = (float)(int)(v + 0.5f);
    params[id] = v; if (id == P_QUALITY) setQuality((int)v);
  }
  float bpm() const { return params[P_BPM]; }
  double stepSamples(double bpmv) const { return sr * 60.0 / (bpmv > 1 ? bpmv : 120) / 4.0; }

  // ---------------------------------------------------------------- transport
  void play() { playing = true; hostSync = false; startAt(0); }
  void stop() { playing = false; releaseAll(); curStep = -1; }
  void startAt(double stepPos) { pos = stepPos; loopCount = 0; seekTo(stepPos); }
  void seekTo(double stepPos) {   // the next step to fire: the first whose onset is at or after stepPos
    const int L = pat[0].len < 1 ? 1 : pat[0].len;
    long long k = (long long)floord(stepPos) - 1; if (k < 0) k = 0;
    while (onset(k) < stepPos - 1e-9) k++;
    nextK = k; nextOnset = onset(k); loopCount = k / L; nHits = 0;
  }
  // when step k (absolute, counting loops) starts, in steps: swing on the odd 16ths, micro-timing per step
  double onset(long long k) const {
    const int L = pat[0].len < 1 ? 1 : pat[0].len; const Step& s = stepAt((int)(k % L));
    double t = (double)k + ((k & 1) ? params[P_SWING] * 0.5 : 0.0) + s.micro * 0.01;
    if (params[P_HUMAN] > 0.001f) t += (double)((hash3((uint32_t)k, 77u, (uint32_t)params[P_SEED]) >> 8) * (1.f / 16777216.f) - 0.5f) * params[P_HUMAN] * 0.06;
    return t;
  }
  // the step played at index i: A, or B where the morph says so (each step crosses at its own point)
  const Step& stepAt(int i) const {
    const float m = params[P_MORPH];
    if (m > 0.001f && i < pat[1].len) { const float h = (hash3((uint32_t)i, 0xAB, (uint32_t)params[P_SEED]) >> 8) * (1.f / 16777216.f); if (h < m) return pat[1].s[i]; }
    return pat[0].s[i];
  }
  bool condOk(const Step& s, long long loop) const {
    const bool fill = params[P_FILL] > 0.5f;
    switch (s.cond) {
      case C_ALWAYS: return true; case C_FILL: return fill; case C_NOFILL: return !fill;
      case C_FIRST: return loop == 0; case C_NOFIRST: return loop != 0;
      default: if (s.cond >= C_AB) { const int a = ((s.cond - C_AB) >> 3) + 1, b = ((s.cond - C_AB) & 7) + 1; return (int)(loop % b) == a - 1; } return true;
    }
  }

  // ---------------------------------------------------------------- notes
  void midi(uint8_t st, uint8_t d1, uint8_t d2) { if (midiOut && nev < NEV) { ev[nev].at = procAt; ev[nev].st = st; ev[nev].d1 = d1; ev[nev].d2 = d2; nev++; } }
  void noteOn(const Step& s, bool slideIn) {
    const int n = clampi(s.note + (int)params[P_TRANSPOSE] + (int)params[P_OCT] * 12, 0, 127);
    const bool acc = (s.flags & ST_ACC) != 0;
    logT = 8.78135971f + ((float)n + params[P_TUNE] - 69.f) / 12.f; freqTarget = exp2f_(logT);   // log2(440) + semitones / 12
    if (!slideIn || !gateOn) { logF = logT; freq = freqTarget; meg = 1; vca = vca * 0.25f; pEnv = 1; }   // a new note: the filter envelope restarts
    accNow = acc ? 1.f : 0.f; velNow = s.vel / 127.f;
    if (acc) accCap = accCap + (1.f - accCap) * 0.55f;                                       // consecutive accents climb, like the original's capacitor
    gateOn = true; vcaTarget = 1;
    // MIDI out: slide = the previous note held until this one starts (legato), and the TB-3's CC 102 / 103
    const uint8_t ch = (uint8_t)(midiCh & 15);
    midi(0xB0 | ch, 102, slideIn ? 127 : 0); midi(0xB0 | ch, 103, acc ? 127 : 0);
    midi(0x90 | ch, (uint8_t)n, (uint8_t)clampi(acc ? 127 : (int)(s.vel * 0.85f), 1, 127));
    if (hwNote >= 0 && hwNote != n) midi(0x80 | ch, (uint8_t)hwNote, 0);
    hwNote = n; noteNow = n;
  }
  void noteOffNow() { gateOn = false; vcaTarget = 0; if (hwNote >= 0) { midi(0x80 | (midiCh & 15), (uint8_t)hwNote, 0); hwNote = -1; } }
  void releaseAll() { noteOffNow(); nHits = 0; gateOff = -1; midi(0xB0 | (midiCh & 15), 123, 0); }
  // a key or pad played live (not the sequencer)
  void liveNote(int note, int vel) { Step s; s.note = (int8_t)clampi(note, 0, 127); s.flags = ST_ON | (vel >= 112 ? ST_ACC : 0); s.vel = (uint8_t)clampi(vel, 1, 127); noteOn(s, gateOn); gateOff = -1; }
  void liveOff(int note) { if (note == noteNow || note < 0) noteOffNow(); }

  // step k starts: decide what plays, queue its hits (ratchets), set the gate end
  void fire(long long k) {
    const int L = pat[0].len < 1 ? 1 : pat[0].len, i = (int)(k % L);
    const long long loop = k / L; if (loop != loopCount) loopCount = loop;
    curStep = i;
    const Step& s = stepAt(i);
    if (params[P_MUTE] > 0.5f) { if (gateOn) noteOffNow(); return; }
    if (!(s.flags & ST_ON)) { return; }                                              // a rest: the gate already ends by itself
    if (s.flags & ST_TIE) { gateOff = (double)(k + 1) + 0.999; return; }               // tie: the note keeps sounding through this step
    if (s.prob < 100 && (int)(hash3((uint32_t)k, (uint32_t)i, (uint32_t)params[P_SEED]) % 100u) >= s.prob) return;
    if (!condOk(s, loop)) return;
    const int r = clampi(s.ratchet, 1, 8); const double t0 = onset(k);
    // the gate: to the next step when it slides into it or is tied to it; else the step's own length
    const Step& nx = stepAt((i + 1) % L);
    const bool holdOn = (nx.flags & ST_ON) && (nx.flags & (ST_SLIDE | ST_TIE));
    nHits = 0;
    for (int j = 0; j < r; j++) { hits[nHits].at = t0 + (double)j / r; hits[nHits].st = s; hits[nHits].slideIn = j == 0 && (s.flags & ST_SLIDE) && gateOn; nHits++; }
    const double span = 1.0 / r;
    gateOff = holdOn && r == 1 ? (double)(k + 1) + 0.6 : t0 + (r - 1) * span + span * clampi(s.gate, 5, 100) * 0.01;
  }

  // ---------------------------------------------------------------- the sound
  inline float voiceSample(float sro) {
    // pitch: an exponential glide in octaves (a constant-time slide, as on the original)
    const float gl = params[P_GLIDE]; logF += (logT - logF) * (1.f - expf_(-1.f / (gl * sro * 0.35f + 1.f)));
    // pitch drop: each new note starts up to two octaves high and falls (the fidget / electro "zap")
    pEnv *= expf_(-1.f / (params[P_PDEC] * sro));
    freq = exp2f_(logF + params[P_PDROP] * pEnv * (1.f / 12.f));
    const float dt = clampf(freq / sro, 0.f, 0.45f);
    phase += dt; if (phase >= 1) phase -= 1;
    subPhase += dt * 0.5f; if (subPhase >= 1) subPhase -= 1;
    const float pw = params[P_PW], w = params[P_WAVE];
    auto osc = [&](float ph, float d) { float sa = 2 * ph - 1 - polyblep(ph, d); float q = ph < pw ? 1.f : -1.f; q += polyblep(ph, d); float t = ph - pw; if (t < 0) t += 1; q -= polyblep(t, d); return sa * (1 - w) + q * w; };
    float sq = subPhase < 0.5f ? 1.f : -1.f; sq += polyblep(subPhase, dt * 0.5f); { float t = subPhase - 0.5f; if (t < 0) t += 1; sq -= polyblep(t, dt * 0.5f); }
    const int vmode = (int)params[P_VOICE];
    float x;
    if (vmode == 0) x = osc(phase, dt);
    else {
      // DIRTY: seven detuned oscillators (a wall of saws). FIDGET: two hard-detuned ones and one an octave up
      static const float spread7[7] = {0.f, -0.11f, 0.11f, -0.32f, 0.32f, -0.6f, 0.6f}, spread3[3] = {-0.5f, 0.5f, 12.f};
      const float det = params[P_DETUNE];
      const int n = vmode == 1 ? 7 : 3; float sum = 0;
      for (int j = 0; j < n; j++) {
        const float semi = vmode == 1 ? spread7[j] * det : (j < 2 ? spread3[j] * det : 12.f);
        const float dj = clampf(dt * exp2f_(semi * (1.f / 12.f)), 0.f, 0.45f);
        stk[j] += dj; if (stk[j] >= 1) stk[j] -= 1;
        sum += osc(stk[j], dj) * (vmode == 2 && j == 2 ? 0.6f : 1.f);
      }
      x = sum * (vmode == 1 ? 0.42f : 0.62f);
    }
    x += sq * params[P_SUB] * 0.7f;
    if (params[P_FOLD] > 0.001f) { const float f = params[P_FOLD]; const float y = sinf_(clampf(x * (1.f + f * 6.f), -8.f, 8.f) * PI * 0.5f); x += (y - x) * f; }
    if (params[P_NOISE] > 0.001f) { float wn = noise.bi(); pinkB[0] = 0.997f * pinkB[0] + 0.029591f * wn; pinkB[1] = 0.985f * pinkB[1] + 0.032534f * wn; pinkB[2] = 0.95f * pinkB[2] + 0.048056f * wn; x += (pinkB[0] + pinkB[1] + pinkB[2] + wn * 0.1848f) * params[P_NOISE] * 1.5f; }
    x *= 0.5f * (1.f + params[P_DRIVE] * 5.f);
    // filter: cutoff from the knob, the decaying envelope (shorter and stronger on an accent) and the accent "sweep"
    const float acc = params[P_ACCENT] * accNow;
    const float envAmt = params[P_ENVMOD] * (1.f + acc * 0.6f);
    const float oct = envAmt * 5.5f * meg + acc * 2.2f * accCap * meg;
    const float fc = clampf(params[P_CUTOFF] * exp2f_(oct), 30.f, minf(18000.f, sro * 0.45f));
    float y = lad.run(x, fc, params[P_RES] * (1.f - acc * 0.12f), sro);
    return y;
  }

  // the effect chain on one stereo frame at the output rate
  inline void fx(float& L, float& R, double stepLen) {
    // bit crusher
    if (params[P_CRUSH_ON] > 0.5f) {
      const float rate = params[P_CRUSH_RATE]; crushPh += 1.f - rate * 0.985f; if (crushPh >= 1.f) { crushPh -= 1.f; crushHold[0] = L; crushHold[1] = R; }
      const float lv = exp2f_(params[P_CRUSH_BITS] - 1), m = params[P_CRUSH_MIX];
      const float a = (float)(int)(crushHold[0] * lv) / lv, b = (float)(int)(crushHold[1] * lv) / lv; L += (a - L) * m; R += (b - R) * m;
    }
    // ring modulator
    if (params[P_RING_ON] > 0.5f) { ringPh += params[P_RING_FREQ] / sr; if (ringPh >= 1) ringPh -= 1; const float c = sinf_(ringPh * TAU), m = params[P_RING_MIX]; L += (L * c - L) * m; R += (R * c - R) * m; }
    // stutter: when it turns on, the last slice (1/4 .. 1/32) loops until it turns off
    { const bool on = params[P_STUT_ON] > 0.5f;
      if (!stutHeld) { stut[0][stutW] = L; stut[1][stutW] = R; if (++stutW >= STUTMAX) stutW = 0; }
      if (on && !stutHeld) { static const float lens[4] = {4, 2, 1, 0.5f}; stutLen = clampi((int)(stepLen * lens[clampi((int)params[P_STUT_LEN], 0, 3)]), 64, STUTMAX - 1); stutPos = 0; stutHeld = true; }
      if (!on) stutHeld = false;
      if (stutHeld) { int r = stutW - stutLen + stutPos; while (r < 0) r += STUTMAX; L = stut[0][r]; R = stut[1][r]; if (++stutPos >= stutLen) stutPos = 0; } }
    // dynamic filter: a low-pass that sweeps with the tempo (a slow LFO in steps per cycle)
    if (params[P_DYN_ON] > 0.5f) {
      dynPh += (float)(1.0 / (stepLen * params[P_DYN_RATE])); if (dynPh >= 1) dynPh -= 1;
      const float l = 0.5f - 0.5f * sinf_(dynPh * TAU + PI * 0.5f), fc = 200.f * exp2f_(l * params[P_DYN_DEPTH] * 6.5f + 1.5f * (1 - params[P_DYN_DEPTH]));
      const float f = 2.f * sinf_(PI * minf(fc, sr * 0.2f) / sr), q = 1.f - params[P_DYN_RES] * 0.9f;
      float* io[2] = {&L, &R}; for (int c = 0; c < 2; c++) { float& lo = dynS[c][0]; float& bp = dynS[c][1]; const float hi = *io[c] - lo - q * bp; bp += f * hi; lo += f * bp; *io[c] = lo; }
    }
    // chorus / flanger / phaser
    if (params[P_MOD_ON] > 0.5f) {
      const int type = (int)params[P_MOD_TYPE]; modPh += params[P_MOD_RATE] / sr; if (modPh >= 1) modPh -= 1;
      const float d = params[P_MOD_DEPTH], fb = params[P_MOD_FB], mix = params[P_MOD_MIX];
      float in[2] = {L, R}, out[2];
      if (type < 2) {
        for (int c = 0; c < 2; c++) {
          const float lfo = 0.5f + 0.5f * sinf_((modPh + c * 0.25f) * TAU);
          const float ms = type == 0 ? (7.f + 12.f * d * lfo) : (0.3f + 6.f * d * lfo);
          float rp = (float)modW - ms * 0.001f * sr; while (rp < 0) rp += MODMAX; int i0 = (int)rp; float fr = rp - i0; int i1 = i0 + 1; if (i1 >= MODMAX) i1 = 0;
          const float wv = modBuf[c][i0] + (modBuf[c][i1] - modBuf[c][i0]) * fr; out[c] = wv;
          modBuf[c][modW] = in[c] + wv * (type == 1 ? fb : fb * 0.3f);
        }
        if (++modW >= MODMAX) modW = 0;
      } else {   // phaser: six first-order all-passes swept together
        for (int c = 0; c < 2; c++) {
          const float lfo = 0.5f + 0.5f * sinf_((modPh + c * 0.25f) * TAU), fc = 200.f * exp2f_(lfo * d * 4.5f);
          const float t = PI * minf(fc, sr * 0.45f) / sr, a = (1.f - t) / (1.f + t);
          float x = in[c] + modLast[c] * fb;
          for (int s = 0; s < 6; s++) { const float y = -a * x + apZ[c][s]; apZ[c][s] = x + a * y; x = y; }
          modLast[c] = x; out[c] = x;
        }
      }
      L += (out[0] - L) * mix; R += (out[1] - R) * mix;
    }
    // compressor (stereo linked)
    if (params[P_COMP_ON] > 0.5f) {
      const float a = maxf(fabsf_(L), fabsf_(R)); compEnv += (a - compEnv) * (a > compEnv ? 0.08f : 0.0004f);
      const float thr = dbToGain(params[P_COMP_THR]); float g = 1;
      if (compEnv > thr) g = powf_(compEnv / thr, 1.f / params[P_COMP_RATIO] - 1.f);
      g *= dbToGain(params[P_COMP_MAKE]); L *= g; R *= g;
    }
    // delay, synced, ping-pong, darker each repeat
    if (params[P_DLY_ON] > 0.5f) {
      static const float mult[8] = {1, 2, 3, 4, 6, 8, 4.f / 3.f, 6};
      const float want = minf((float)(stepLen * mult[clampi((int)params[P_DLY_TIME], 0, 7)]), (float)(DLYMAX - 2));
      dlyTime += (want - dlyTime) * 0.0005f; if (dlyTime < 1) dlyTime = want;
      float rp = (float)dlyW - dlyTime; while (rp < 0) rp += DLYMAX; int i0 = (int)rp; float fr = rp - i0; int i1 = i0 + 1; if (i1 >= DLYMAX) i1 = 0;
      const float yl = dly[0][i0] + (dly[0][i1] - dly[0][i0]) * fr, yr = dly[1][i0] + (dly[1][i1] - dly[1][i0]) * fr;
      const float tc = 0.05f + params[P_DLY_TONE] * 0.9f; dlyTone[0] += (yl - dlyTone[0]) * tc; dlyTone[1] += (yr - dlyTone[1]) * tc;
      const float fb = params[P_DLY_FB]; const bool ping = params[P_DLY_PING] > 0.5f;
      const float inM = (L + R) * 0.5f;
      if (ping) { dly[0][dlyW] = inM + dlyTone[1] * fb; dly[1][dlyW] = dlyTone[0] * fb; }
      else { dly[0][dlyW] = L + dlyTone[0] * fb; dly[1][dlyW] = R + dlyTone[1] * fb; }
      if (++dlyW >= DLYMAX) dlyW = 0;
      const float m = params[P_DLY_MIX]; L += dlyTone[0] * m; R += dlyTone[1] * m;
    }
    // reverb: two diffusing all-passes into four damped combs (a small FDN)
    if (params[P_REV_ON] > 0.5f) {
      float x = (L + R) * 0.35f;
      for (int k = 0; k < 2; k++) { const int n = apLen[k]; int r = apW[k] - n; if (r < 0) r += 1024; const float y = ap[k][r] - 0.6f * x; ap[k][apW[k]] = x + 0.6f * y; if (++apW[k] >= 1024) apW[k] = 0; x = y; }
      const float fb = 0.70f + params[P_REV_SIZE] * 0.27f, damp = 0.15f + params[P_REV_DAMP] * 0.7f; float o[4];
      for (int k = 0; k < 4; k++) { int r = rvW - rvLen[k]; if (r < 0) r += REVMAX; const float y = rv[k][r]; rvLp[k] += (y - rvLp[k]) * (1.f - damp); o[k] = rvLp[k]; }
      const float hd = 0.5f; // Householder-ish mix
      const float s = (o[0] + o[1] + o[2] + o[3]) * hd;
      for (int k = 0; k < 4; k++) rv[k][rvW] = x + (o[k] - s) * fb;
      if (++rvW >= REVMAX) rvW = 0;
      const float m = params[P_REV_MIX]; L += (o[0] + o[2]) * m; R += (o[1] + o[3]) * m;
    }
  }

  // ---------------------------------------------------------------- one block
  // The events of this call are appended to ev[] (those sent between calls, like a stop, land at 0): the caller reads
  // them and sets nev = 0.
  void process(float* outL, float* outR, int n) {
    const double stepLen = stepSamples(bpm());
    const float sro = sr * (float)os;
    const float megK = expf_(-1.f / (maxf(0.01f, (accNow > 0.5f ? 0.2f : params[P_DECAY])) * sro));
    const float vcaDecK = expf_(-1.f / (params[P_VCADECAY] * sro));
    const float vAtk = onepole(400.f, sro), vRel = onepole(120.f, sro);
    const float vol = params[P_AUDIO] > 0.5f ? dbToGain(params[P_VOLUME]) : 0.f;
    for (int i = 0; i < n; i++) {
      procAt = i;
      // the sequencer, at sample resolution
      if (playing) {
        while (pos + 1e-9 >= nextOnset) { fire(nextK); nextK++; nextOnset = onset(nextK); }
        while (nHits > 0 && pos + 1e-9 >= hits[0].at) { noteOn(hits[0].st, hits[0].slideIn); for (int j = 1; j < nHits; j++) hits[j - 1] = hits[j]; nHits--; }
        if (gateOn && gateOff >= 0 && pos >= gateOff && nHits == 0) { noteOffNow(); gateOff = -1; }
        pos += 1.0 / stepLen;
      }
      // the voice, oversampled
      float acc = 0;
      for (int o = 0; o < os; o++) {
        const float v = voiceSample(sro);
        meg *= megK; if (!gateOn) accCap *= 0.99995f;
        if (gateOn) { vca += (vcaTarget - vca) * vAtk; vcaTarget *= vcaDecK; } else vca += (0 - vca) * vRel;
        float y = v * vca * (0.55f + 0.45f * velNow) * (1.f + params[P_ACCENT] * accNow * 0.9f);
        acc = os > 1 ? dec[0].run(y) : y;
      }
      float x = acc;
      // post drive with tone, a gentle DC block
      const float d2 = params[P_DRIVE2];
      if (d2 > 0.001f) { const float g = 1.f + d2 * 14.f; x = tanhf_(x * g + 0.15f * d2) - tanhf_(0.15f * d2); x /= (1.f + d2 * 1.5f) * 0.5f + 0.5f; }
      { const float t = params[P_TONE]; if (t != 0) { toneLp += (x - toneLp) * 0.18f; x = t > 0 ? x + (x - toneLp) * t * 1.5f : x + (toneLp - x) * (-t); } }
      hp += (x - hp) * onepole(18.f, sr); x -= hp;
      // sidechain pump: ducks on every beat and breathes back, locked to the sequencer (free-running when stopped)
      if (params[P_PUMP_ON] > 0.5f) {
        const double beat = playing ? pos * 0.25 : (pumpFree += 1.0 / (stepLen * 4.0));
        const float t = (float)(beat - (double)(long long)beat), u = 1.f - t;
        const float duck = params[P_PUMP] * (t < 0.012f ? t / 0.012f : u * u * u) ;
        pumpG += ((1.f - duck) - pumpG) * 0.25f; x *= pumpG;
      }
      float L = x, R = x;
      fx(L, R, stepLen);
      L *= vol; R *= vol;
      // a soft limiter: never past full scale
      const float pk = maxf(fabsf_(L), fabsf_(R)); if (pk * lim > 0.98f) lim = 0.98f / pk; else lim += (1.f - lim) * 0.00008f;
      L = tanhf_(L * lim * 1.02f) * 0.98f; R = tanhf_(R * lim * 1.02f) * 0.98f;
      if (!(L == L)) L = 0; if (!(R == R)) R = 0;
      outL[i] = L; outR[i] = R;
      lastLevel = maxf(lastLevel * 0.9995f, fabsf_(L));
      if ((i & 3) == 0) { scope[scopeW] = L; if (++scopeW >= 256) scopeW = 0; }
    }
    procAt = 0;
    tele[0] = playing ? 1.f : 0.f; tele[1] = (float)curStep; tele[2] = (float)loopCount; tele[3] = lastLevel;
    tele[4] = params[P_CUTOFF] * exp2f_(params[P_ENVMOD] * 5.5f * meg); tele[5] = accCap; tele[6] = (float)noteNow; tele[7] = (float)pos;
  }

  // the plug-in: follow the host's song position (PPQ = quarters), loops and jumps included
  void hostPosition(double ppq, bool running) {
    if (!running) { if (playing) stop(); hostSync = false; return; }
    const double want = ppq * 4.0;
    if (!playing || !hostSync) { playing = true; hostSync = true; pos = want; seekTo(want); return; }
    if (want < pos - 0.25 || want > pos + 0.5) { releaseAll(); pos = want; seekTo(want); }   // a jump or a loop in the host
    else pos = want;                                                                       // re-anchor every block (no drift)
  }
};

}  // namespace dfd
