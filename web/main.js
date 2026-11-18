import {PRESETS,generate,mutate,midiBytes,parseNoteName,downloadBytes,audioBufferToWAV} from './pattern.js';
import {AcidAudio,renderOffline} from './audio.js';
import {TB3Link} from './tb3-midi.js';
const $=id=>document.getElementById(id);const audio=new AcidAudio();
let steps=[],playing=false,interval=null,nextTime=0,stepNumber=0,currentStep=-1,priorNote=null,patternName=PRESETS[0].name;
const output=(line)=>{$('monitor').textContent='['+new Date().toLocaleTimeString('es-MX',{hour12:false})+'] '+line;};
let loadedHardwareSnapshot=null;
const link=new TB3Link({log:output,onNote:e=>{if(e.type==='on')output('TB-3 IN  NOTE '+parseNoteName(e.note)+' · VEL '+e.velocity);},onCC:e=>{if([74,71,68,69].includes(e.cc))output('TB-3 IN  CC '+e.cc+' · '+e.value);},onBlock:(name,bytes)=>{if(name==='patch'&&bytes.length>=11){$('hwSaw').value=bytes[1];$('hwDecay').value=bytes[10];}}});
function settings(){return {bpm:Math.max(30,Math.min(300,+$('bpm').value||132)),swing:+$('swing').value/100,mode:$('mode').value,root:+$('root').value,scale:$('scale').value,density:+$('density').value/100,chaos:+$('chaos').value/100,cutoff:+$('cutoff').value,resonance:+$('resonance').value/100,drive:+$('drive').value/100,sub:+$('sub').value/100,delay:+$('delay').value/100,wave:$('wave').value};}
function setValue(id,value){if($(id))$(id).value=value;}
function hash(){let h=0x811c9dc5;for(const s of steps){h^=s.note+(s.active?211:0)+(s.accent?37:0)+(s.slide?97:0)+(s.ratchet||1);h=Math.imul(h,0x01000193)>>>0;}return h.toString(16).padStart(8,'0').toUpperCase();}
function metadata(){const act=steps.filter(x=>x.active).length;$('activeNotes').textContent=act+' / '+steps.length;$('stepInfo').textContent=steps.length+' STEPS / '+(steps.length/16)+' BARS';$('dna').textContent=hash().replace(/^(.{4})/,'$1-');}
function grid(){const root=$('grid');root.textContent='';const fragment=document.createDocumentFragment();steps.forEach((s,i)=>{
 const wrap=document.createElement('div');wrap.className='step '+(s.active?'active':'rest')+(currentStep===i?' playhead':'');wrap.dataset.i=i;
 const number=document.createElement('div');number.className='step-index';number.textContent=String(i+1).padStart(2,'0');wrap.append(number);
 if(s.ratchet>1){const n=document.createElement('span');n.className='step-num';n.textContent='x'+s.ratchet;wrap.append(n);}
 const button=document.createElement('button');button.className='step-main';button.title='Clic: activar/desactivar. Clic derecho: aumentar nota. Shift+Clic: bajar nota.';
 const pitch=document.createElement('span');pitch.className='step-note';pitch.textContent=s.active?parseNoteName(s.note):'—';button.append(pitch);
 button.addEventListener('click',e=>{if(e.shiftKey){s.note=Math.max(24,s.note-1);s.active=true;}else{s.active=!s.active;}grid();});
 button.addEventListener('contextmenu',e=>{e.preventDefault();s.note=Math.min(96,s.note+1);s.active=true;grid();});wrap.append(button);
 const modes=document.createElement('div');modes.className='step-mode';for(const [txt,key] of [['A','accent'],['S','slide'],['R','ratchet']]){
  const b=document.createElement('button');b.textContent=txt;b.title=key;b.className=key+(key==='ratchet'?s.ratchet>1?' on':'':s[key]?' on':'');b.addEventListener('click',()=>{if(key==='ratchet')s.ratchet=(s.ratchet%4)+1;else s[key]=!s[key];if(s[key]||key==='ratchet')s.active=true;grid();});modes.append(b);
 }wrap.append(modes);fragment.append(wrap);
 });root.append(fragment);metadata();}
function initialize(){for(const p of PRESETS){let o=document.createElement('option');o.value=p.name;o.textContent=p.name;$('preset').append(o);}loadPreset(PRESETS[0]);loadLibrary();updateReadouts();
 for(const id of ['density','chaos','mutation','swing','cutoff','resonance','drive','sub','delay'])$(id).addEventListener('input',()=>{updateReadouts();if(audio.ctx&&['cutoff','resonance','drive','sub','delay'].includes(id))audio.update(settings());});
 $('length').addEventListener('change',()=>{const n=+$('length').value;if(n>steps.length){steps=[...steps,...generate({...settings(),length:n-steps.length,seed:Date.now()})];}else steps=steps.slice(0,n);grid();});
 $('generate').onclick=()=>{steps=generate({...settings(),length:+$('length').value});patternName='DFD / GENERATED '+Date.now().toString(16).slice(-4).toUpperCase();$('customName').value=patternName;grid();output('Pattern generated · '+steps.length+' steps · '+settings().mode);};
 $('mutate').onclick=()=>{steps=mutate(steps,+$('mutation').value/100,settings());grid();output('Mutation complete · '+$('mutation').value+'%');};
 $('randomize').onclick=()=>{steps=mutate(steps,.95,settings());grid();};$('clear').onclick=()=>{steps=steps.map(s=>({...s,active:false,slide:false,accent:false,ratchet:1}));grid();};
 $('loadPreset').onclick=()=>loadPreset(PRESETS.find(p=>p.name===$('preset').value)||PRESETS[0]);
 $('play').onclick=start;$('stop').onclick=stop;
 $('exportMidi').onclick=()=>{downloadBytes(midiBytes(steps,{bpm:settings().bpm,swing:settings().swing,channel:link.channel,repeats:4}),'DFD_ACID_'+hash()+'.mid','audio/midi');output('MIDI exported (includes TB-3 slide/accent CCs).');};
 $('exportWav').onclick=exportWav;
 $('savePattern').onclick=saveLocal;$('loadSaved').onclick=loadLocal;
 $('exportProject').onclick=()=>{downloadBytes(new TextEncoder().encode(JSON.stringify({format:'DFD-ACID',version:1,name:patternName,settings:settings(),steps},null,2)),'DFD_PROJECT_'+hash()+'.json','application/json');};
 $('importProject').addEventListener('change',importProject);
 $('connect').onclick=connect;
 $('hardwareMode').onchange=()=>output('Hardware output mode: '+$('hardwareMode').value.toUpperCase());
 $('channel').onchange=()=>link.channel=+$('channel').value;
 $('capture').onclick=()=>{if(!link.input){output('Connect USB MIDI first.');return;}link.startCapture();$('capture').disabled=true;$('captureStop').disabled=false;};
 $('captureStop').onclick=()=>{const got=link.stopCapture({bpm:settings().bpm,length:steps.length});if(got.some(s=>s.active)){steps=got;grid();output('TB-3 MIDI pattern imported into sequencer.');}else output('Capture empty: check TB-3 MIDI transmit channel / mode.');$('capture').disabled=false;$('captureStop').disabled=true;};
 $('identity').onclick=()=>{try{link.identity();output('Sent Identity Request to TB-3.');}catch(e){output(e.message);}};
 $('readPatch').onclick=()=>{try{link.requestSnapshot();output('Requesting TB-3 patch / distortion / FX1 / FX2 blocks...');}catch(e){output(e.message);}};
 $('savePatch').onclick=()=>{if(Object.keys(link.rawBlocks).length===0){output('Read a patch first. No data to save.');return;}downloadBytes(new TextEncoder().encode(link.snapshotJSON()),'DFD_TB3_PATCH_'+Date.now()+'.json','application/json');};
 $('loadPatchFile').onchange=async e=>{const f=e.target.files?.[0];if(!f)return;try{const obj=JSON.parse(await f.text());if(obj.type!=='DFD-TB3-RAW'||!obj.blocks)throw Error('Not a DFD TB-3 snapshot');loadedHardwareSnapshot=obj.blocks;$('restorePatch').disabled=false;output('TB-3 snapshot loaded, ready for manual confirmation.');}catch(err){output('Patch file rejected: '+err.message);}e.target.value='';};
 $('restorePatch').onclick=()=>{if(!loadedHardwareSnapshot)return;if(!confirm('RESTORE ACTIVE TB-3 PATCH? This will modify current sound parameters on the connected TB-3. Back up first. Continue?'))return;try{link.restoreSnapshot(loadedHardwareSnapshot);output('Sending documented active-patch SysEx blocks to TB-3.');}catch(err){output(err.message);}};
 $('fxBlock').onchange=refreshFxOptions;refreshFxOptions();$('applyFxType').onclick=()=>{try{link.writeEffect($('fxBlock').value,0,+$('fxType').value);output('TB-3 FX type transmitted.');}catch(e){output(e.message);}};
 $('applyFxParameter').onclick=()=>{try{link.writeEffect($('fxBlock').value,+$('fxParameter').value,+$('fxValue').value);output('TB-3 FX parameter written.');}catch(e){output(e.message);}};
 for(const [id,control] of [['hwCutoff','cutoff'],['hwResonance','resonance'],['hwSaw','saw'],['hwDecay','decay']])$(id).oninput=e=>{try{link.setHardwareSound(control,+e.target.value);output('TB-3 SEND / '+control.toUpperCase()+' '+e.target.value);}catch(err){output(err.message);}};
 setInterval(()=>{$('clock').textContent=new Date().toLocaleTimeString('es-MX',{hour12:false});},1000);
}
function refreshFxOptions(){const b=$('fxBlock').value,parameter=$('fxParameter'),type=$('fxType');parameter.innerHTML='';for(let i=1;i<=(b==='efx1'?12:10);i++){const el=document.createElement('option');el.value=i;el.textContent='PARAMETER '+i;parameter.append(el);}type.innerHTML='';const names=b==='efx1'?['BYPASS','COMPRESSOR','RING MOD','BIT CRUSH','TREMOLO','CHORUS','FLANGER','PHASER','DELAY','PITCH SHIFT','EQ']:['BYPASS','COMPRESSOR','RING MOD','BIT CRUSH','TREMOLO','CHORUS','FLANGER','PHASER','DELAY','REVERB'];names.forEach((name,i)=>{const el=document.createElement('option');el.value=i;el.textContent=name;type.append(el);});}
function updateReadouts(){for(const [id,label,unit] of [['density','densityOut','%'],['chaos','chaosOut','%'],['mutation','mutationOut','%'],['swing','swingOut','%'],['resonance','resonanceOut','%'],['drive','driveOut','%'],['sub','subOut','%'],['delay','delayOut','%'],['cutoff','cutoffOut',' Hz']])$(label).textContent=$(id).value+unit;}
function loadPreset(p){for(const id of ['mode','root','scale','bpm','cutoff'])setValue(id,p[id]);for(const id of ['resonance','drive','delay'])setValue(id,Math.round(p[id]*100));setValue('length',16);patternName=p.name;setValue('customName',patternName);steps=generate({mode:p.mode,root:p.root,scale:p.scale,length:16,density:.77,chaos:.43,seed:PRESETS.indexOf(p)*15427+617});grid();updateReadouts();if(audio.ctx)audio.update(settings());output('Loaded '+p.name);}
function saveLocal(){patternName=$('customName').value.trim()||'DFD / UNTITLED';let db={};try{db=JSON.parse(localStorage.getItem('dfd-acid-library')||'{}');}catch{}db[patternName]={name:patternName,settings:settings(),steps:structuredClone(steps)};localStorage.setItem('dfd-acid-library',JSON.stringify(db));loadLibrary();output('Saved locally: '+patternName);}
function loadLibrary(){let db={};try{db=JSON.parse(localStorage.getItem('dfd-acid-library')||'{}');}catch{}const el=$('savedPatterns');el.innerHTML='<option value="">YOUR SAVED PATTERNS</option>';Object.keys(db).sort().forEach(key=>{let o=document.createElement('option');o.value=key;o.textContent=key;el.append(o);});}
function loadObject(project){if(!Array.isArray(project.steps)||!project.steps.length||project.steps.length>4096)throw Error('Invalid pattern step count');const s=project.settings||{};for(const id of ['bpm','mode','root','scale','cutoff','wave'])if(s[id]!==undefined)setValue(id,s[id]);for(const id of ['resonance','drive','sub','delay','density','chaos','swing'])if(s[id]!==undefined)setValue(id,Math.round(s[id]*100));let limit=Math.min(4096,project.steps.length);steps=project.steps.slice(0,limit).map(st=>({active:!!st.active,note:Math.min(96,Math.max(24,Number(st.note)||36)),accent:!!st.accent,slide:!!st.slide,ratchet:Math.min(4,Math.max(1,Number(st.ratchet)||1)),prob:1,cutoff:null}));setValue('length',[16,32,64,128].includes(limit)?limit:128);patternName=project.name||'DFD / IMPORTED';setValue('customName',patternName);updateReadouts();grid();if(audio.ctx)audio.update(settings());}
function loadLocal(){const key=$('savedPatterns').value;if(!key)return;try{const db=JSON.parse(localStorage.getItem('dfd-acid-library')||'{}');loadObject(db[key]);output('Recalled '+key);}catch(e){output('Cannot load: '+e.message);}}
async function importProject(e){const file=e.target.files?.[0];if(!file)return;try{const p=JSON.parse(await file.text());if(p.format!=='DFD-ACID')throw Error('Unknown format');loadObject(p);output('Imported '+file.name);}catch(err){output('Import rejected: '+err.message);}e.target.value='';}
async function exportWav(){const b=$('exportWav'),label=b.textContent;b.disabled=true;b.textContent='RENDERING…';try{const repeats=Math.max(1,Math.min(4,Math.floor(128/steps.length)));const buf=await renderOffline(steps,settings(),repeats);downloadBytes(audioBufferToWAV(buf),'DFD_ACID_'+hash()+'.wav','audio/wav');output('Rendered original virtual synth audio to WAV 48kHz / 24-bit. Not hardware audio.');}catch(e){output('WAV render error: '+e.message);}finally{b.disabled=false;b.textContent=label;}}
async function connect(){try{const status=await link.connect({sysex:true});$('midiStatus').textContent=status.output?'● '+status.output.toUpperCase():'● MIDI PERMITTED / NO OUTPUT';output('MIDI permission '+(status.sysex?'WITH':'WITHOUT')+' SysEx');}catch(e){output('MIDI connection failed: '+e.message);}}
function scheduleStep(stepIndex,t,stepSeconds){const s=steps[stepIndex];if(!s)return;const hw=$('hardwareMode').value,midiOn=(hw==='mirror'||hw==='hardware')&&!!link.output,audioOn=hw!=='hardware';
 if(midiOn){const ms=performance.now()+(t-audio.now)*1000;link.cc(102,s.slide?127:0,ms);link.cc(103,s.accent?127:0,ms);}
 if(s.active){const sub=Math.max(1,s.ratchet||1);for(let j=0;j<sub;j++){
 const onset=t+j*stepSeconds/sub,len=(stepSeconds/sub)*.86;if(audioOn)audio.play({...s},onset,len,priorNote);
 if(midiOn){const when=performance.now()+(onset-audio.now)*1000;link.noteOn(s.note,s.accent?125:95,when);link.noteOff(s.note,when+len*1000);}
 }priorNote=s.note;}
 const delay=Math.max(0,(t-audio.now)*1000);setTimeout(()=>{if(!playing)return;const old=document.querySelector('.step.playhead');if(old)old.classList.remove('playhead');const current=document.querySelector('.step[data-i="'+stepIndex+'"]');if(current)current.classList.add('playhead');currentStep=stepIndex;},delay);
}
function scheduler(){if(!playing)return;const s=settings(),seconds=60/s.bpm/4;while(nextTime<audio.now+.14){const i=stepNumber%steps.length;const swing=i%2?seconds*s.swing*.45:0;scheduleStep(i,nextTime+swing,seconds);nextTime+=seconds;stepNumber++;}}
async function start(){if(playing)return;try{await audio.init(settings());}catch(e){output('Audio failed: '+e.message);return;}playing=true;priorNote=null;stepNumber=0;nextTime=audio.now+.07;interval=setInterval(scheduler,20);scheduler();$('play').textContent='◉ PLAYING';output('ACID ENGINE RUNNING / '+settings().bpm+' BPM');}
function stop(){playing=false;clearInterval(interval);interval=null;if($('hardwareMode').value!=='off')link.allNotesOff();audio.stop();$('play').innerHTML='<span class="play-icon">▶</span> PLAY';document.querySelectorAll('.step.playhead').forEach(e=>e.classList.remove('playhead'));output('TRANSPORT STOPPED');}
initialize();
