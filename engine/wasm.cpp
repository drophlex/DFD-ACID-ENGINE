// DFD ACID ENGINE · the WebAssembly entry (no libc): the web's AudioWorklet and its WAV render call these
#include "dfd_acid.h"
extern "C" {
void* memset(void* d, int c, unsigned long n) { unsigned char* p = (unsigned char*)d; while (n--) *p++ = (unsigned char)c; return d; }
void* memcpy(void* d, const void* s, unsigned long n) { unsigned char* a = (unsigned char*)d; const unsigned char* b = (const unsigned char*)s; while (n--) *a++ = *b++; return d; }
void* memmove(void* d, const void* s, unsigned long n) { unsigned char* a = (unsigned char*)d; const unsigned char* b = (const unsigned char*)s; if (a < b) while (n--) *a++ = *b++; else { a += n; b += n; while (n--) *--a = *--b; } return d; }
}
using namespace dfd;
static Engine E;
static float outL[MAXBLOCK], outR[MAXBLOCK];
#define EXPORT extern "C" __attribute__((visibility("default")))
EXPORT void dfd_init(float sr) { E.init(sr); }
EXPORT void dfd_process(int n) { E.process(outL, outR, n > MAXBLOCK ? MAXBLOCK : n); }
EXPORT float* dfd_out_l() { return outL; }
EXPORT float* dfd_out_r() { return outR; }
EXPORT float* dfd_params() { return E.params; }
EXPORT int dfd_param_count() { return P_COUNT; }
EXPORT void dfd_param(int id, float v) { E.setParam(id, v); }
EXPORT unsigned char* dfd_pattern(int ab) { return (unsigned char*)&E.pat[ab ? 1 : 0]; }
EXPORT int dfd_pattern_size() { return (int)sizeof(Pattern); }
EXPORT int dfd_max_steps() { return MAXSTEPS; }
EXPORT void dfd_play() { E.play(); }
EXPORT void dfd_stop() { E.stop(); }
EXPORT void dfd_live_on(int note, int vel) { E.liveNote(note, vel); }
EXPORT void dfd_live_off(int note) { E.liveOff(note); }
EXPORT void dfd_panic() { E.releaseAll(); }
EXPORT float* dfd_tele() { return E.tele; }
EXPORT float* dfd_scope() { return E.scope; }
EXPORT int dfd_scope_w() { return E.scopeW; }
EXPORT int dfd_nev() { return E.nev; }
EXPORT void dfd_clear_ev() { E.nev = 0; }
EXPORT int* dfd_ev() { return (int*)E.ev; }          // [at, st | d1 << 8 | d2 << 16] pairs, as int32
EXPORT int dfd_ev_size() { return (int)sizeof(MidiEv); }
EXPORT void dfd_midi_ch(int ch) { E.midiCh = ch & 15; }
EXPORT int dfd_step_now() { return E.curStep; }
EXPORT const char* dfd_param_id(int i) { return i >= 0 && i < P_COUNT ? PARAMS[i].id : ""; }
EXPORT float dfd_param_lo(int i) { return PARAMS[i].lo; }
EXPORT float dfd_param_hi(int i) { return PARAMS[i].hi; }
EXPORT float dfd_param_def(int i) { return PARAMS[i].def; }
EXPORT int dfd_param_steps(int i) { return PARAMS[i].steps; }
