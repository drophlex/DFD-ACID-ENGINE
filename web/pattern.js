// DFD ACID ENGINE - original sequencer and export utilities; no third-party dependencies.
export const MODES = {
  acid:{name:'ACID / 303',rests:.13,accents:.27,slides:.3,ratchets:.02,jump:.26},
  broken:{name:'BROKEN / IDM',rests:.27,accents:.34,slides:.24,ratchets:.34,jump:.73},
  rave:{name:'RAVE / ATTACK',rests:.09,accents:.48,slides:.19,ratchets:.11,jump:.48},
  mutant:{name:'MUTANT / GLITCH',rests:.31,accents:.48,slides:.43,ratchets:.44,jump:.92}
};
export const SCALES={minor:[0,2,3,5,7,8,10],phrygian:[0,1,3,5,7,8,10],dorian:[0,2,3,5,7,9,10],chromatic:[0,1,2,3,4,5,6,7,8,9,10,11],pentatonic:[0,3,5,7,10]};
export const PRESETS=[
 {name:'DFD / NIGHT SIGNAL',mode:'acid',root:36,scale:'minor',bpm:132,cutoff:430,resonance:.79,drive:.43,delay:.27},
 {name:'POLARIS / GLASS TEETH',mode:'broken',root:38,scale:'phrygian',bpm:158,cutoff:890,resonance:.88,drive:.51,delay:.16},
 {name:'DFD / REACTION 04',mode:'rave',root:41,scale:'dorian',bpm:146,cutoff:720,resonance:.68,drive:.73,delay:.33},
 {name:'DARK FUNK / VIRUS',mode:'mutant',root:33,scale:'chromatic',bpm:172,cutoff:520,resonance:.91,drive:.82,delay:.24},
 {name:'MERIDA / SUBSYSTEM',mode:'acid',root:29,scale:'minor',bpm:124,cutoff:320,resonance:.73,drive:.39,delay:.17},
 {name:'POLARIS / STRANGE LOOP',mode:'broken',root:36,scale:'dorian',bpm:184,cutoff:1100,resonance:.84,drive:.64,delay:.41}
];
export function rng(seed=Date.now()){let s=seed>>>0;return ()=>{s=(Math.imul(1664525,s)+1013904223)>>>0;return s/4294967296;};}
export const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
export function generate({mode='acid',root=36,scale='minor',length=16,density=.75,chaos=.45,seed=Date.now()}={}){
 const p=MODES[mode]||MODES.acid, notes=SCALES[scale]||SCALES.minor, r=rng(seed); const out=[];let degree=0;
 for(let i=0;i<length;i++){
  const active=r()<clamp((1-p.rests)*density+.13,0,1);
  if(r()<.2+chaos*.46) degree+=(r()-.5)*((p.jump*8+2)*chaos+1)|0;
  else degree+=r()<.46?-1:r()<.72?1:0;
  degree=clamp(degree,-9,15);
  const octave=Math.floor(degree/notes.length), index=((degree%notes.length)+notes.length)%notes.length;
  let note=clamp(root+12*octave+notes[index],24,96);
  if(r()<chaos*.13) note=clamp(note+(r()<.5?-12:12),24,96);
  out.push({note,active,accent:active&&r()<p.accents,slide:active&&i>0&&r()<p.slides,ratchet:active&&r()<p.ratchets*chaos?2+Math.floor(r()*3):1,prob:1,cutoff:null});
 }
 if(!out.some(s=>s.active))out[0].active=true;
 return out;
}
export function mutate(steps,amount=.38,config={}){
 const r=rng(Math.floor(Math.random()*0xffffffff)),sc=SCALES[config.scale||'minor']||SCALES.minor;
 return steps.map((s,i)=>{
  const n={...s};if(r()<amount*.58)n.note=clamp(n.note+sc[Math.floor(r()*sc.length)]-sc[Math.floor(r()*sc.length)],24,96);
  if(r()<amount*.32)n.active=!n.active;
  if(r()<amount*.58)n.accent=!n.accent;
  if(r()<amount*.43)n.slide=!n.slide;
  if(r()<amount*.22)n.ratchet=n.ratchet===1?2+Math.floor(r()*3):1;
  if(i===0)n.slide=false;
  return n;
 });
}
export function encodeVLQ(v){v=Math.max(0,Math.floor(v));let b=[v&127];while((v>>=7))b.unshift((v&127)|128);return b;}
function chunk(id,bytes){return [...new TextEncoder().encode(id),...(new Array(4).fill(0).map((_,i)=>(bytes.length >>> (24-i*8))&255)),...bytes];}
export function midiBytes(steps,{bpm=132,channel=1,repeats=4,swing=0}={}){
 // Type 0, one track, ticks per quarter = 480, includes TB-3 CC 102 slide & 103 accent.
 const division=480,tps=120,events=[],ch=clamp(channel|0,1,16)-1,base=[0x90|ch,0x80|ch],c=[0xb0|ch];
 const push=(tick,prio,...bytes)=>events.push({tick:Math.round(tick),prio,bytes});
 const mpqn=Math.round(60000000/clamp(bpm,30,300));
 push(0,0,0xff,0x51,3,(mpqn>>16)&255,(mpqn>>8)&255,mpqn&255);
 for(let turn=0;turn<repeats;turn++)for(let i=0;i<steps.length;i++){
  const s=steps[i];if(!s.active)continue;const start=(turn*steps.length+i)*tps+(i%2?Math.round(tps*swing*.45):0);
  const sub=Math.max(1,s.ratchet|0),part=tps/sub;
  push(start,1,...c,102,s.slide?127:0);push(start,1,...c,103,s.accent?127:0);
  for(let k=0;k<sub;k++){
    const onset=start+k*part, dur=part*(s.slide&&k===sub-1?1.01:.84);
    push(onset,2,...base.slice(0,1),clamp(s.note|0,0,127),s.accent?122:91);
    push(onset+dur,0,...base.slice(1),clamp(s.note|0,0,127),0);
  }
 }
 events.sort((a,b)=>a.tick-b.tick||a.prio-b.prio);let prev=0,body=[];
 for(const ev of events){body.push(...encodeVLQ(Math.max(0,ev.tick-prev)),...ev.bytes);prev=ev.tick;}
 body.push(0,0xff,0x2f,0);
 return new Uint8Array([...chunk('MThd',[0,0,0,1,(division>>8)&255,division&255]),...chunk('MTrk',body)]);
}
export function parseNoteName(note){const n=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];return n[(note+120)%12]+(Math.floor(note/12)-1);}
export function downloadBytes(bytes,name,mime){const url=URL.createObjectURL(new Blob([bytes],{type:mime}));const a=document.createElement('a');a.href=url;a.download=name;document.body.append(a);a.click();a.remove();setTimeout(()=>URL.revokeObjectURL(url),1000);}
export function audioBufferToWAV(buffer,bitDepth=24){
 if(![16,24].includes(bitDepth))throw Error('Use 16 or 24-bit PCM');
 const channels=Math.min(2,buffer.numberOfChannels),samples=buffer.length,bps=bitDepth/8;
 const bytes=new ArrayBuffer(44+samples*channels*bps),v=new DataView(bytes);let o=0;
 const str=s=>{for(let i=0;i<s.length;i++)v.setUint8(o++,s.charCodeAt(i));},u16=x=>{v.setUint16(o,x,true);o+=2;},u32=x=>{v.setUint32(o,x,true);o+=4;};
 str('RIFF');u32(bytes.byteLength-8);str('WAVEfmt ');u32(16);u16(1);u16(channels);u32(buffer.sampleRate);u32(buffer.sampleRate*channels*bps);u16(channels*bps);u16(bitDepth);str('data');u32(samples*channels*bps);
 const planes=Array.from({length:channels},(_,i)=>buffer.getChannelData(i));
 for(let i=0;i<samples;i++)for(let ch=0;ch<channels;ch++){
   const x=clamp(planes[ch][i],-1,1);
   if(bitDepth===16){v.setInt16(o,x<0?Math.round(x*32768):Math.round(x*32767),true);o+=2;}
   else{const n=x<0?Math.round(x*8388608):Math.round(x*8388607);v.setUint8(o++,n&255);v.setUint8(o++,(n>>8)&255);v.setUint8(o++,(n>>16)&255);}
 }
 return new Uint8Array(bytes);
}
