import test from 'node:test';import assert from 'node:assert/strict';import {generate,mutate,midiBytes,encodeVLQ,PRESETS} from '../web/pattern.js';
test('generates seeded stable 64-step pattern',()=>{let a=generate({length:64,seed:4242,mode:'broken'}),b=generate({length:64,seed:4242,mode:'broken'});assert.deepEqual(a,b);assert.equal(a.length,64);assert.ok(a.some(x=>x.active));});
test('midi export contains header, track, tempo and note',()=>{const x=midiBytes(generate({length:16,seed:1}),{bpm:147,repeats:2});assert.equal(new TextDecoder().decode(x.slice(0,4)),'MThd');assert.equal(new TextDecoder().decode(x.slice(14,18)),'MTrk');assert.ok(x.includes(0x90));assert.ok(x.length>70);});
test('VLQ encodes typical delta',()=>{assert.deepEqual(encodeVLQ(128),[0x81,0]);});
test('presets include DFD, pattern mutation is immutable',()=>{let a=generate({length:16,seed:3});let b=mutate(a,1);assert.notEqual(a,b);assert.equal(a.length,b.length);assert.ok(PRESETS.some(p=>p.name.includes('DFD')));});
import {audioBufferToWAV} from '../web/pattern.js';
import {sysexDT1,sysexRQ1,validateRolandDT1,checksum} from '../web/tb3-midi.js';
test('Roland SysEx DT1 checksum and roundtrip validate',()=>{
 const msg=sysexDT1([0,0x20,0x10,1],[3]);
 assert.equal(msg[0],0xf0);assert.equal(msg[1],0x41);assert.equal(msg.at(-1),0xf7);
 const decoded=validateRolandDT1([...msg]);assert.deepEqual(decoded.address,[0,32,16,1]);assert.deepEqual(decoded.payload,[3]);
 const tampered=[...msg];tampered[tampered.length-2]^=1;assert.equal(validateRolandDT1(tampered),null);
 assert.equal(checksum([0,0x20,0x10,1,3]),0x4c);
});
test('Roland RQ1 standard message',()=>{const m=sysexRQ1([0,32,0,0],[0,0,0,33]);assert.equal(m[6],0x11);assert.equal(m.length,17);});
test('24 bit WAV conforms to PCM RIFF header',()=>{const fake={numberOfChannels:2,length:3,sampleRate:48000,getChannelData:ch=>ch===0?new Float32Array([0,.5,-1]):new Float32Array([1,-.5,0])};const wav=audioBufferToWAV(fake);assert.equal(new TextDecoder().decode(wav.slice(0,4)),'RIFF');assert.equal(wav.length,44+2*3*3);const d=new DataView(wav.buffer);assert.equal(d.getUint16(34,true),24);assert.equal(d.getUint32(24,true),48000);});
