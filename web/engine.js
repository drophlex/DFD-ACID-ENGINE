// DFD ACID ENGINE · the browser side of the shared C++ engine (engine/dfd_acid.h, compiled to WebAssembly).
// The same code runs in the AudioWorklet (live), in an offline instance (WAV / stems / MIDI render) and in the plugin.
import {WASM_B64} from './dfd-wasm.js';

export const wasmBytes = () => Uint8Array.from(atob(WASM_B64), c => c.charCodeAt(0));

export async function instance() {
  const {instance: i} = await WebAssembly.instantiate(wasmBytes(), {});
  const X = i.exports; if (X.__wasm_call_ctors) X.__wasm_call_ctors();
  return X;
}

// the parameter table, read from the engine itself so the page never drifts from the C++ ids
export function paramTable(X) {
  const out = [], n = X.dfd_param_count();
  const cstr = p => { const m = new Uint8Array(X.memory.buffer); let s = ''; while (m[p]) s += String.fromCharCode(m[p++]); return s; };
  for (let i = 0; i < n; i++) out.push({i, id: cstr(X.dfd_param_id(i)), lo: X.dfd_param_lo(i), hi: X.dfd_param_hi(i), def: X.dfd_param_def(i), steps: X.dfd_param_steps(i)});
  return out;
}

// step objects -> the engine's 8-byte Step layout (note, flags, gate, vel, ratchet, prob, cond, micro), after an int32 length
export function patternBytes(steps) {
  const b = new Uint8Array(4 + steps.length * 8), dv = new DataView(b.buffer);
  dv.setInt32(0, steps.length, true);
  steps.forEach((s, k) => {
    const o = 4 + k * 8;
    b[o] = s.note & 127;
    b[o + 1] = (s.active ? 1 : 0) | (s.accent ? 2 : 0) | (s.slide ? 4 : 0) | (s.tie ? 8 : 0);
    b[o + 2] = Math.max(5, Math.min(100, s.gate ?? 55));
    b[o + 3] = Math.max(1, Math.min(127, s.vel ?? 100));
    b[o + 4] = Math.max(1, Math.min(8, s.ratchet || 1));
    b[o + 5] = Math.max(0, Math.min(100, s.prob ?? 100));
    b[o + 6] = s.cond | 0;
    b[o + 7] = Math.max(-50, Math.min(50, s.micro | 0)) & 255;
  });
  return b;
}

const WORKLET = `
class DFD extends AudioWorkletProcessor {
  constructor(o) {
    super(); this.X = null; this.q = []; this.tc = 0;
    this.port.onmessage = e => this.msg(e.data);
    WebAssembly.instantiate(o.processorOptions.bytes, {}).then(r => {
      const X = r.instance.exports; if (X.__wasm_call_ctors) X.__wasm_call_ctors();
      X.dfd_init(sampleRate); this.X = X; const q = this.q; this.q = []; for (const m of q) this.msg(m);
      this.port.postMessage({t: 'ready'});
    });
  }
  msg(m) {
    const X = this.X; if (!X) { this.q.push(m); return; }
    switch (m.t) {
      case 'p': X.dfd_param(m.id, m.v); break;
      case 'pat': new Uint8Array(X.memory.buffer, X.dfd_pattern(m.ab), m.bytes.length).set(m.bytes); break;
      case 'play': X.dfd_play(); break;
      case 'stop': X.dfd_stop(); break;
      case 'on': X.dfd_live_on(m.n, m.v); break;
      case 'off': X.dfd_live_off(m.n); break;
      case 'panic': X.dfd_panic(); break;
      case 'ch': X.dfd_midi_ch(m.ch); break;
    }
  }
  process(i, o) {
    const X = this.X, out = o[0]; if (!X) return true;
    const n = out[0].length; X.dfd_process(n);
    const b = X.memory.buffer;
    out[0].set(new Float32Array(b, X.dfd_out_l(), n)); if (out[1]) out[1].set(new Float32Array(b, X.dfd_out_r(), n));
    const ne = X.dfd_nev();
    if (ne) { const ev = new Int32Array(b, X.dfd_ev(), ne * 2), list = []; for (let k = 0; k < ne; k++) list.push(currentTime + ev[k * 2] / sampleRate, ev[k * 2 + 1]); X.dfd_clear_ev(); this.port.postMessage({t: 'ev', list}); }
    if ((this.tc += n) >= 1024) { this.tc = 0; this.port.postMessage({t: 'tele', tele: Array.from(new Float32Array(b, X.dfd_tele(), 8)), scope: new Float32Array(b, X.dfd_scope(), 256).slice(), w: X.dfd_scope_w()}); }
    return true;
  }
}
registerProcessor('dfd-acid', DFD);`;

export class LiveEngine {
  constructor() { this.ctx = null; this.node = null; this.params = null; this.pending = []; this.onEvents = null; this.onTele = null; this.tele = [0, -1, 0, 0, 0, 0, -1, 0]; this.scope = new Float32Array(256); this.scopeW = 0; this.analyser = null; }
  async start() {
    if (this.ctx) { await this.ctx.resume(); return; }
    const ctx = new (window.AudioContext || window.webkitAudioContext)({latencyHint: 'interactive'});
    // blob: on http(s); a data: URL when the page is opened from disk (file:// has a null origin)
    try { await ctx.audioWorklet.addModule(URL.createObjectURL(new Blob([WORKLET], {type: 'application/javascript'}))); }
    catch (e) { await ctx.audioWorklet.addModule('data:text/javascript;base64,' + btoa(WORKLET)); }
    const node = new AudioWorkletNode(ctx, 'dfd-acid', {numberOfInputs: 0, outputChannelCount: [2], processorOptions: {bytes: wasmBytes().buffer}});
    node.connect(ctx.destination);
    node.port.onmessage = e => {
      const m = e.data;
      if (m.t === 'ev' && this.onEvents) this.onEvents(m.list);
      else if (m.t === 'tele') { this.tele = m.tele; this.scope = m.scope; this.scopeW = m.w; this.onTele && this.onTele(this.tele); }
    };
    this.ctx = ctx; this.node = node;
    for (const m of this.pending) node.port.postMessage(m); this.pending = [];
    await ctx.resume();
  }
  send(m) { if (this.node) this.node.port.postMessage(m); else this.pending.push(m); }
  param(id, v) { this.send({t: 'p', id, v}); }
  pattern(ab, steps) { this.send({t: 'pat', ab, bytes: patternBytes(steps)}); }
  play() { this.send({t: 'play'}); }
  stop() { this.send({t: 'stop'}); }
  noteOn(n, v) { this.send({t: 'on', n, v}); }
  noteOff(n) { this.send({t: 'off', n}); }
  panic() { this.send({t: 'panic'}); }
  channel(ch) { this.send({t: 'ch', ch: (ch - 1) & 15}); }
  // context seconds -> performance.now() milliseconds, aligned with what is heard
  perfTime(t) {
    if (!this.ctx) return performance.now();
    const ts = this.ctx.getOutputTimestamp ? this.ctx.getOutputTimestamp() : null;
    if (ts && ts.performanceTime) return ts.performanceTime + (t - ts.contextTime) * 1000;
    return performance.now() + (t - this.ctx.currentTime) * 1000 + (this.ctx.outputLatency || this.ctx.baseLatency || 0) * 1000;
  }
}

// offline: renders the same engine faster than real time. Returns {L, R, events:[[sample, st, d1, d2]...], sr}
export async function render({params, patA, patB, loops = 4, tail = 2, sr = 48000, overrides = {}}) {
  const X = await instance(); X.dfd_init(sr);
  params.forEach((v, i) => X.dfd_param(i, v));
  for (const [i, v] of Object.entries(overrides)) X.dfd_param(+i, v);
  const put = (ab, s) => { const b = patternBytes(s); new Uint8Array(X.memory.buffer, X.dfd_pattern(ab), b.length).set(b); };
  put(0, patA); put(1, patB || patA);
  const bpm = params[ID.bpm], stepS = sr * 60 / bpm / 4, total = Math.ceil(stepS * patA.length * loops), tailN = Math.ceil(tail * sr);
  const L = new Float32Array(total + tailN), R = new Float32Array(total + tailN), events = [];
  X.dfd_play();
  const B = 512;
  let stopped = false;
  for (let at = 0, n; at < L.length; at += n) {
    if (!stopped && at >= total) { X.dfd_stop(); stopped = true; }
    n = Math.min(B, L.length - at, at < total ? total - at : B); X.dfd_process(n);
    L.set(new Float32Array(X.memory.buffer, X.dfd_out_l(), n), at); R.set(new Float32Array(X.memory.buffer, X.dfd_out_r(), n), at);
    const ne = X.dfd_nev();
    if (ne) { const ev = new Int32Array(X.memory.buffer, X.dfd_ev(), ne * 2); for (let k = 0; k < ne; k++) { const p = ev[k * 2 + 1]; events.push([at + ev[k * 2], p & 255, (p >> 8) & 255, (p >> 16) & 255]); } X.dfd_clear_ev(); }
  }
  return {L, R, events, sr, stepS, total};
}

// parameter ids by name, filled at boot from the engine's own table
export const ID = {};
export let TABLE = [];
export async function boot() {
  const X = await instance(); TABLE = paramTable(X); for (const p of TABLE) ID[p.id] = p.i; return TABLE;
}

// a type-1 .mid from engine events: what you hear is exactly what you get (probability, conditions, morph, ratchets)
export function eventsToMidi(events, {bpm, stepS, name = 'DFD ACID'}) {
  const ppq = 480, tickPerSample = (ppq / 4) / stepS, body = [];
  const vlq = v => { v = Math.max(0, Math.round(v)); const b = [v & 127]; while ((v >>= 7)) b.unshift((v & 127) | 128); return b; };
  const mpqn = Math.round(60000000 / bpm);
  const meta = [0, 0xff, 0x03, ...vlq(name.length), ...new TextEncoder().encode(name), 0, 0xff, 0x51, 3, (mpqn >> 16) & 255, (mpqn >> 8) & 255, mpqn & 255];
  body.push(...meta);
  let prev = 0;
  for (const [s, st, d1, d2] of events) {
    if (st >= 0xf0) continue;
    const t = Math.round(s * tickPerSample); body.push(...vlq(t - prev), st, d1, d2); prev = t;
  }
  body.push(0, 0xff, 0x2f, 0);
  const chunk = (id, bytes) => [...new TextEncoder().encode(id), (bytes.length >>> 24) & 255, (bytes.length >>> 16) & 255, (bytes.length >>> 8) & 255, bytes.length & 255, ...bytes];
  return new Uint8Array([...chunk('MThd', [0, 0, 0, 1, (ppq >> 8) & 255, ppq & 255]), ...chunk('MTrk', body)]);
}

export function wav24(L, R, sr) {
  const n = L.length, bytes = new ArrayBuffer(44 + n * 6), v = new DataView(bytes); let o = 0;
  const str = s => { for (let i = 0; i < s.length; i++) v.setUint8(o++, s.charCodeAt(i)); }, u16 = x => { v.setUint16(o, x, true); o += 2; }, u32 = x => { v.setUint32(o, x, true); o += 4; };
  str('RIFF'); u32(bytes.byteLength - 8); str('WAVEfmt '); u32(16); u16(1); u16(2); u32(sr); u32(sr * 6); u16(6); u16(24); str('data'); u32(n * 6);
  for (let i = 0; i < n; i++) for (const ch of [L, R]) { const x = Math.max(-1, Math.min(1, ch[i])); const q = Math.round(x < 0 ? x * 8388608 : x * 8388607); v.setUint8(o++, q & 255); v.setUint8(o++, (q >> 8) & 255); v.setUint8(o++, (q >> 16) & 255); }
  return new Uint8Array(bytes);
}

// ------------------------------------------------------------------ inside the plug-in (JUCE web view): the native engine
export const PLUGIN = typeof window !== 'undefined' && !!(window.__JUCE__ && window.__JUCE__.backend);
let invokeId = 1;
export function invoke(name, ...args) {
  const B = window.__JUCE__.backend, id = invokeId++;
  return new Promise(res => { const tok = B.addEventListener('__juce__complete', e => { if (e.promiseId === id) { B.removeEventListener(tok); res(e.result); } }); B.emitEvent('__juce__invoke', {name, params: args, resultId: id}); });
}
export const toB64 = bytes => { let s = ''; for (let i = 0; i < bytes.length; i += 0x8000) s += String.fromCharCode.apply(null, bytes.subarray(i, i + 0x8000)); return btoa(s); };
export const fromB64 = s => Uint8Array.from(atob(s), c => c.charCodeAt(0));
// bytes (int32 length + 8-byte steps) -> step objects, the reverse of patternBytes
export function bytesToSteps(b) {
  const dv = new DataView(b.buffer, b.byteOffset, b.byteLength), len = Math.max(1, Math.min(4096, dv.getInt32(0, true), (b.length - 4) >> 3)), out = [];
  for (let k = 0; k < len; k++) { const o = 4 + k * 8, f = b[o + 1]; out.push({note: b[o], active: !!(f & 1), accent: !!(f & 2), slide: !!(f & 4), tie: !!(f & 8), gate: b[o + 2], vel: b[o + 3], ratchet: b[o + 4], prob: b[o + 5], cond: b[o + 6], micro: (b[o + 7] << 24) >> 24}); }
  return out;
}
export class NativeEngine {
  constructor() {
    this.ctx = {state: 'running'}; this.tele = [0, -1, 0, 0, 0, 0, -1, 0]; this.scope = new Float32Array(256); this.scopeW = 0; this.onEvents = null; this.onTele = null; this.onParams = null;
    window.__JUCE__.backend.addEventListener('tele', e => { this.tele = e.tele; this.scope = Float32Array.from(e.scope); this.scopeW = e.w; if (e.params && this.onParams) this.onParams(e.params); this.onTele && this.onTele(this.tele); });
  }
  async start() {}
  send(m) { invoke('msg', JSON.stringify(m)); }
  param(id, v) { this.send({t: 'p', id, v}); }
  pattern(ab, steps) { this.send({t: 'pat', ab, b64: toB64(patternBytes(steps))}); }
  play() { this.send({t: 'play'}); }
  stop() { this.send({t: 'stop'}); }
  noteOn(n, v) { this.send({t: 'on', n, v}); }
  noteOff(n) { this.send({t: 'off', n}); }
  panic() { this.send({t: 'panic'}); }
  channel(ch) { this.send({t: 'ch', ch: (ch - 1) & 15}); }
  perfTime() { return performance.now(); }
}
