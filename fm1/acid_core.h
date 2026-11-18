#ifndef DFD_ACID_CORE_H
#define DFD_ACID_CORE_H
#include <stdint.h>
/* Portable realtime DSP / MIDI sequencer core for an eventual M-VAVE FM-1 firmware port.
 * NOT a .fwsc image. Device screen, keyscan, audio callback and build scripts still required. */
#ifdef __cplusplus
extern "C" {
#endif
#define DFD_FM1_STEPS 64

typedef struct { uint8_t note,active,accent,slide,ratchet; } dfd_step;
typedef void (*dfd_midi_tx)(void *context,uint8_t status,uint8_t data1,uint8_t data2);
typedef struct {
  dfd_step step[DFD_FM1_STEPS];
  uint16_t length,position; uint32_t sample_rate,samples_until_step;
  float bpm,cutoff,resonance,drive,decay,sub;
  float phase,subphase,env,filter,feedback,pitch,target_pitch,step_length;
  uint8_t previous_note,previous_active,channel;
  dfd_midi_tx midi;void *midi_context;
} dfd_acid;
void dfd_init(dfd_acid *a,uint32_t rate);
void dfd_tick(dfd_acid *a); /* call once per mono sample (48 kHz, etc.) */
float dfd_process_sample(dfd_acid *a);
void dfd_set_midi(dfd_acid *a,dfd_midi_tx send,void *context,uint8_t channel);
void dfd_note(dfd_acid *a,uint8_t note,uint8_t accent,uint8_t slide);
#ifdef __cplusplus
}
#endif
#endif
