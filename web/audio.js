import {clamp} from './pattern.js';
const frequency=n=>440*Math.pow(2,(n-69)/12);
function saturator(amount){const curve=new Float32Array(8192),k=1+amount*38;for(let i=0;i<curve.length;i++){let x=i*2/(curve.length-1)-1;curve[i]=(Math.tanh(x*k)/Math.tanh(k))*.72;}return curve;}
function createGraph(ctx,s){
 const input=ctx.createGain(),wave=ctx.createWaveShaper(),delay=ctx.createDelay(1.5),feedback=ctx.createGain(),dry=ctx.createGain(),wet=ctx.createGain(),comp=ctx.createDynamicsCompressor(),out=ctx.createGain();
 wave.curve=saturator(s.drive);wave.oversample='4x';
 dry.gain.value=1-s.delay*.22;delay.delayTime.value=60/s.bpm*.75;feedback.gain.value=.22+s.delay*.42;wet.gain.value=s.delay*.62;
 comp.threshold.value=-17;comp.ratio.value=3;comp.attack.value=.003;comp.release.value=.12;out.gain.value=.62;
 input.connect(wave);wave.connect(dry).connect(comp);wave.connect(delay);delay.connect(feedback).connect(delay);delay.connect(wet).connect(comp);comp.connect(out).connect(ctx.destination);
 return {input,out};
}
function createVoice(ctx,graph,s,step,t,duration,prevNote){
 const note=step.note??36,freq=frequency(note),startFreq=step.slide&&prevNote!==null?frequency(prevNote):freq;
 const osc=ctx.createOscillator(),sub=ctx.createOscillator(),subGain=ctx.createGain(),filter=ctx.createBiquadFilter(),amp=ctx.createGain();
 osc.type=s.wave||'sawtooth';sub.type='sine';subGain.gain.value=s.sub*.42;
 osc.frequency.setValueAtTime(startFreq,t);sub.frequency.setValueAtTime(startFreq*.5,t);
 if(step.slide&&prevNote!==null){osc.frequency.exponentialRampToValueAtTime(freq,t+.095);sub.frequency.exponentialRampToValueAtTime(freq*.5,t+.095);}
 else{osc.frequency.setValueAtTime(freq,t);sub.frequency.setValueAtTime(freq*.5,t);}
 filter.type='lowpass';filter.Q.value=.9+s.resonance*22;
 const brightness=clamp(step.cutoff||s.cutoff,70,10000)*(step.accent?1.55:1);
 filter.frequency.setValueAtTime(clamp(brightness*.48,50,12000),t);
 filter.frequency.exponentialRampToValueAtTime(clamp(brightness*3.1,50,15000),t+.008);
 filter.frequency.exponentialRampToValueAtTime(clamp(brightness*.42,50,15000),t+Math.min(.3,duration*.88));
 amp.gain.setValueAtTime(0,t);amp.gain.linearRampToValueAtTime(step.accent ? .34 : .23,t+.004);
 amp.gain.exponentialRampToValueAtTime(.0001,t+duration+.09);
 osc.connect(filter);sub.connect(subGain).connect(filter);filter.connect(amp).connect(graph.input);
 osc.start(t);sub.start(t);osc.stop(t+duration+.12);sub.stop(t+duration+.12);
 return {osc,sub,amp};
}
export class AcidAudio{
 constructor(){this.ctx=null;this.graph=null;this.settings=null;this.lastNote=null;}
 async init(settings){if(!this.ctx){this.ctx=new (window.AudioContext||window.webkitAudioContext)();}await this.ctx.resume();this.update(settings);}
 update(s){this.settings={...s};if(this.graph){try{this.graph.input.disconnect();this.graph.out.disconnect();}catch{}}this.graph=createGraph(this.ctx,this.settings);}
 play(step,t,duration,previous){if(!this.ctx||!this.graph||!step.active)return;createVoice(this.ctx,this.graph,this.settings,step,t,duration,previous);this.lastNote=step.note;}
 get now(){return this.ctx?.currentTime||0;}
 stop(){if(this.ctx){this.ctx.suspend();}}
 resume(){return this.ctx?.resume();}
}
export async function renderOffline(steps,settings,repeats=4){
 const seconds=60/settings.bpm/4,tail=2.2,duration=seconds*steps.length*repeats+tail;
 const ctx=new OfflineAudioContext(2,Math.ceil(48000*duration),48000),g=createGraph(ctx,settings);
 for(let turn=0;turn<repeats;turn++)for(let i=0;i<steps.length;i++){
  const step=steps[i];if(!step.active)continue;
  const t=.05+(turn*steps.length+i)*seconds+(i%2?seconds*(settings.swing||0)*.45:0),sub=Math.max(1,step.ratchet||1);
  for(let j=0;j<sub;j++)createVoice(ctx,g,settings,step,t+j*seconds/sub,seconds/sub*.86,steps[(i-1+steps.length)%steps.length]?.note??null);
 }
 return ctx.startRendering();
}
