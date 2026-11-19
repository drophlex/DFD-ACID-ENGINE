// USB TB-3 support. Roland TB-3 MIDI implementation v1.10; no undocumented writes.
// Physical audio is NOT accessible through Web MIDI. Capture via host USB audio / DAW.
export const TB3_BLOCKS={patch:{address:[0,0x20,0,0],size:[0,0,0,0x21]},distortion:{address:[0,0x20,0x10,0],size:[0,0,0,7]},efx1:{address:[0,0x20,0x20,0],size:[0,0,0,0x1a]},efx2:{address:[0,0x20,0x30,0],size:[0,0,0,0x16]}};
export const TB3_CONTROLS={cutoff:74,resonance:71,accent:16,effect:17,envX:12,envY:13,scatterType:68,scatterDepth:69,slide:102,patternAccent:103};
export function checksum(addressAndData){return (128-(addressAndData.reduce((a,b)=>a+b,0)%128))%128;}
export function sysexRQ1(address,size,device=0x10){const x=[...address,...size];return new Uint8Array([0xf0,0x41,device,0,0,0x7b,0x11,...x,checksum(x),0xf7]);}
export function sysexDT1(address,data,device=0x10){const x=[...address,...data];return new Uint8Array([0xf0,0x41,device,0,0,0x7b,0x12,...x,checksum(x),0xf7]);}
export function validateRolandDT1(bytes){if(bytes.length<14||bytes[0]!==0xf0||bytes.at(-1)!==0xf7||bytes[1]!==0x41||bytes[3]!==0||bytes[4]!==0||bytes[5]!==0x7b||bytes[6]!==0x12)return null;
 const address=bytes.slice(7,11),payload=bytes.slice(11,-2);if(checksum([...address,...payload])!==bytes.at(-2))return null;return {address,payload,device:bytes[2]};}
export class TB3Link{
 constructor({log=()=>{},onNote=()=>{},onCC=()=>{},onBlock=()=>{}}={}){this.log=log;this.onNote=onNote;this.onCC=onCC;this.onBlock=onBlock;this.access=null;this.output=null;this.input=null;this.sysex=false;this.channel=1;this.capture=false;this.notes=[];this.startMS=0;this.rawBlocks={};this.pend=new Map();}
 async connect({sysex=true}={}){
  if(!navigator.requestMIDIAccess)throw Error('Web MIDI no disponible. Abre Chrome/Edge en localhost o HTTPS.');
  try{this.access=await navigator.requestMIDIAccess({sysex});this.sysex=sysex;}
  catch(e){if(!sysex)throw e;this.access=await navigator.requestMIDIAccess({sysex:false});this.sysex=false;this.log('MIDI conectado sin SysEx: permisos limitados.');}
  this.refresh();this.access.onstatechange=()=>this.refresh();
  if(!this.output&&!this.input)this.log('No se detectó un puerto TB-3. Puedes elegir puertos MIDI compatibles.');
  return {output:this.output?.name,input:this.input?.name,sysex:this.sysex};
 }
 refresh(){const outputs=[...this.access.outputs.values()],inputs=[...this.access.inputs.values()],pick=arr=>arr.find(p=>/TB[- ]?3\b/i.test(p.name))||null;
  this.output=pick(outputs);if(this.input)this.input.onmidimessage=null;this.input=pick(inputs);if(this.input)this.input.onmidimessage=e=>this.receive(e);this.log('MIDI OUT: '+(this.output?.name||'sin puerto')+' · IN: '+(this.input?.name||'sin puerto'));
 }
 send(data,at){if(!this.output)return false;this.output.send([...data],at);return true;}
 cc(cc,v,at){return this.send([0xb0+this.channel-1,cc,Math.max(0,Math.min(127,Math.round(v)))],at);}
 noteOn(note,velocity=100,at){return this.send([0x90+this.channel-1,note,velocity],at);}
 noteOff(note,at){return this.send([0x80+this.channel-1,note,0],at);}
 allNotesOff(){this.cc(123,0);}
 identity(){if(!this.sysex)throw Error('SysEx no permitido');this.send([0xf0,0x7e,0x7f,6,1,0xf7]);}
 requestBlock(key){const b=TB3_BLOCKS[key];if(!b||!this.sysex)throw Error('Bloque inválido o SysEx no autorizado');return this.send(sysexRQ1(b.address,b.size));}
 requestSnapshot(){if(!this.sysex)throw Error('Necesitas autorizar SysEx');Object.keys(TB3_BLOCKS).forEach((name,i)=>setTimeout(()=>this.requestBlock(name),i*150));}
 // Byte-safe explicit write only: no undocumented sequencer pattern writes and no auto storage.
 writeParameter(address,data){if(!this.sysex)throw Error('SysEx no autorizado');if(!this.output)throw Error('Ninguna TB-3 MIDI de salida conectada');return this.send(sysexDT1(address,data));}
 writeEffect(type,index,value){const b=TB3_BLOCKS[type];if(!b||!['efx1','efx2'].includes(type))throw Error('Bloque FX inválido');if(index===0)return this.writeParameter(b.address,[Math.min(type==='efx1'?10:9,Math.max(0,value|0))]);const max=type==='efx1'?12:10;if(index<1||index>max)throw Error('FX parameter out of range');const offset=2+2*(index-1);return this.writeParameter([0,0x20,type==='efx1'?0x20:0x30,offset],[(value>>4)&15,value&15]);}
 restoreSnapshot(blocks){if(!this.sysex||!this.output)throw Error('TB-3 SysEx connection needed');for(const [key,data] of Object.entries(blocks)){const b=TB3_BLOCKS[key];if(!b||!Array.isArray(data)||data.length!==b.size[3]||data.some(v=>!Number.isInteger(v)||v<0||v>127))throw Error('Invalid snapshot block: '+key);}Object.entries(blocks).forEach(([key,data],i)=>setTimeout(()=>this.writeParameter(TB3_BLOCKS[key].address,data),i*170));}
 // Controller morphing: CC safe; complex patch SysEx morph is avoided because FX enum values aren't continuous.
 setHardwareSound(name,value){if(!this.output)throw Error('Conecta primero la TB-3 por USB MIDI');const addresses={saw:[0,0x20,0,1],square:[0,0x20,0,3],decay:[0,0x20,0,0x0a],distDrive:[0,0x20,0x10,2],distTone:[0,0x20,0x10,4]};
  if(TB3_CONTROLS[name]!==undefined)this.cc(TB3_CONTROLS[name],value);
  else if(addresses[name])this.writeParameter(addresses[name],[Math.max(0,Math.min(127,Math.round(value)))]);
  else throw Error('Parámetro no documentado: '+name);
 }
 receive(event){const d=Array.from(event.data),status=d[0]&0xf0;
  if(status===0x90&&d[2]>0){this.onNote({note:d[1],velocity:d[2],time:event.timeStamp,type:'on'});if(this.capture)this.notes.push({note:d[1],velocity:d[2],at:event.timeStamp-this.startMS});}
  else if(status===0x80||(status===0x90&&d[2]===0))this.onNote({note:d[1],velocity:0,time:event.timeStamp,type:'off'});
  else if(status===0xb0)this.onCC({cc:d[1],value:d[2],time:event.timeStamp});
  else if(d[0]===0xf0){if(d[1]===0x7e&&d[3]===6&&d[4]===2)this.log('Identidad MIDI recibida: '+d.map(x=>x.toString(16).padStart(2,'0')).join(' '));
    const dt=validateRolandDT1(d);if(dt){const key=Object.keys(TB3_BLOCKS).find(k=>TB3_BLOCKS[k].address.every((a,i)=>a===dt.address[i]));if(key){this.rawBlocks[key]=dt.payload;this.onBlock(key,dt.payload);this.log('Bloque '+key+' recibido ('+dt.payload.length+' bytes).');}}}
 }
 startCapture(){this.notes=[];this.startMS=performance.now();this.capture=true;this.log('Grabando notas MIDI entrantes de '+(this.input?.name||'la TB-3')+'…');}
 stopCapture({bpm=132,length=16}={}){this.capture=false;const stepMS=60000/bpm/4;const steps=Array.from({length},()=>({note:36,active:false,accent:false,slide:false,ratchet:1,prob:1,cutoff:null}));for(const n of this.notes){const i=Math.round(n.at/stepMS)%length;if(i<0)continue;steps[i]={...steps[i],active:true,note:n.note,accent:n.velocity>110};}this.log(this.notes.length+' notas capturadas; cuantizadas a 1/16.');return steps;}
 // DEMO: a simulated TB-3 that answers like the real one (identity, RQ1 -> DT1, DT1 writes), so everything can be tried without hardware
 useDemo(){const demo=new DemoTB3(d=>this.receive({data:d,timeStamp:performance.now()}),this.log);this.demo=demo;this.output={name:'DEMO TB-3 (simulated)',send:(d,at)=>demo.send(d,at)};this.input={name:'DEMO TB-3 (simulated)'};this.sysex=true;this.log('DEMO TB-3 connected: nothing leaves this browser.');return {output:this.output.name,input:this.input.name,sysex:true};}
 disconnect(){if(this.input&&this.input.onmidimessage!==undefined)this.input.onmidimessage=null;this.output=null;this.input=null;this.demo=null;this.log('MIDI disconnected.');}
 snapshotJSON(){return JSON.stringify({type:'DFD-TB3-RAW',version:1,source:'TB-3 MIDI v1.10',blocks:Object.fromEntries(Object.entries(this.rawBlocks).map(([k,v])=>[k,[...v]]))},null,2);}
}

// the simulated TB-3: plausible factory-like values in the four documented blocks
export class DemoTB3{
 constructor(reply,log=()=>{}){this.reply=reply;this.log=log;this.mem={patch:[1,100,0,0,0,0,0,0,0,0,68,...new Array(21).fill(0),100],distortion:[1,3,40,50,50,90,100],efx1:[0,0,...new Array(24).fill(0)],efx2:[8,0,...new Array(20).fill(0)]};}
 send(d){d=[...d];const at=()=>new Promise(r=>setTimeout(r,25));
  if(d[0]===0xf0&&d[1]===0x7e&&d[4]===1){at().then(()=>this.reply([0xf0,0x7e,0x10,6,2,0x41,0x7b,0x02,0,0,0,0,0,0,0xf7]));return;}
  if(d[0]===0xf0&&d[1]===0x41&&d[5]===0x7b){const addr=d.slice(7,11),key=Object.keys(TB3_BLOCKS).find(k=>TB3_BLOCKS[k].address.slice(0,3).every((a,i)=>a===addr[i]));if(!key)return;
   if(d[6]===0x11){at().then(()=>this.reply([...sysexDT1(TB3_BLOCKS[key].address,this.mem[key])]));}
   if(d[6]===0x12){const data=d.slice(11,-2),off=addr[3];data.forEach((v,i)=>{if(off+i<this.mem[key].length)this.mem[key][off+i]=v;});this.log('DEMO TB-3 stored '+data.length+' byte(s) in '+key+' @'+off);}
   return;}
  const st=d[0]&0xf0;if(st===0x90&&d[2]>0)this.last=d[1];
 }
}
