#include "acid_core.h"
#include <math.h>
#include <string.h>
static float limit(float a,float lo,float hi){return a<lo?lo:a>hi?hi:a;}
static float hz(int n){return 440.f*powf(2.f,((float)n-69.f)/12.f);}
void dfd_init(dfd_acid *a,uint32_t rate){
 memset(a,0,sizeof(*a));a->length=16;a->sample_rate=rate?rate:48000;
 a->bpm=132.f;a->cutoff=560.f;a->resonance=.78f;a->drive=.51f;a->decay=.32f;a->sub=.27f;
 a->pitch=a->target_pitch=110.f;
 const uint8_t notes[16]={36,36,43,36,39,36,41,48,36,36,43,39,36,41,34,36};
 for(int i=0;i<16;i++){a->step[i]=(dfd_step){notes[i],(uint8_t)(i%7!=2),(uint8_t)(i%4==0),(uint8_t)(i%5==2),1};}
 a->samples_until_step=1;
}
void dfd_set_midi(dfd_acid *a,dfd_midi_tx send,void *context,uint8_t channel){a->midi=send;a->midi_context=context;a->channel=(uint8_t)(channel&15);}
void dfd_note(dfd_acid *a,uint8_t note,uint8_t accent,uint8_t slide){
 a->target_pitch=hz(note);
 if(!slide||a->env<.03f)a->pitch=a->target_pitch;
 a->env=accent?1.f:.7f;
}
void dfd_tick(dfd_acid *a){
 if(a->samples_until_step>0){--a->samples_until_step;return;}
 if(a->length==0||a->length>DFD_FM1_STEPS)a->length=16;
 const dfd_step *s=&a->step[a->position];
 if(a->midi&&a->previous_active)a->midi(a->midi_context,(uint8_t)(0x80|a->channel),a->previous_note,0);
 if(s->active){
   dfd_note(a,s->note,s->accent,s->slide);
   if(a->midi){a->midi(a->midi_context,(uint8_t)(0xb0|a->channel),102,s->slide?127:0);
     a->midi(a->midi_context,(uint8_t)(0xb0|a->channel),103,s->accent?127:0);
     a->midi(a->midi_context,(uint8_t)(0x90|a->channel),s->note,s->accent?127:100);}
 }
 else a->env*=.1f;
 a->previous_active=s->active;a->previous_note=s->note;
 a->position=(uint16_t)((a->position+1)%a->length);
 a->step_length=(float)a->sample_rate*60.f/(limit(a->bpm,30.f,300.f)*4.f);
 a->samples_until_step=(uint32_t)limit(a->step_length,1.f,1000000.f);
}
float dfd_process_sample(dfd_acid *a){
 dfd_tick(a);
 float glide=limit(1.f/(a->sample_rate*.018f),0.00001f,1.f);
 a->pitch+=(a->target_pitch-a->pitch)*glide;
 a->phase+=a->pitch/a->sample_rate;if(a->phase>=1.f)a->phase-=1.f;
 a->subphase+=a->pitch/(a->sample_rate*2.f);if(a->subphase>=1.f)a->subphase-=1.f;
 float saw=2.f*a->phase-1.f,triangle=1.f-4.f*fabsf(a->subphase-.5f);
 float osc=.66f*saw+.28f*a->sub*triangle;
 float cf=limit(a->cutoff*(1.f+a->env*2.1f),50.f,a->sample_rate*.41f);
 float pole=limit(6.28318530718f*cf/a->sample_rate,.00001f,.8f);
 a->feedback=a->feedback*.98f+osc*.02f;
 a->filter+=pole*(osc-a->filter-a->resonance*a->feedback*.6f);
 float x=tanhf(a->filter*(1.f+a->drive*25.f))*.45f*a->env;
 a->env*=expf(-1.f/(a->sample_rate*limit(a->decay,.04f,1.5f)));
 return limit(x,-1.f,1.f);
}
