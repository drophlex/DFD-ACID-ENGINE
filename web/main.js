// DFD ACID ENGINE · the page. Sound comes from the shared C++ engine (WebAssembly in an AudioWorklet).
import {PRESETS, MODES, STYLE_SOUND, SOUND_KEYS, DIRTY_SOUNDS, DIRTY_MODES, generate, mutate, parseNoteName, downloadBytes, dnaHash, sanitize, blankStep, condName, COND_LIST, clamp} from './pattern.js';
import {LiveEngine, boot, ID, TABLE, render, eventsToMidi, wav24} from './engine.js';
import {TB3Link} from './tb3-midi.js';
import {applyLang, getLang, tr, PNAME_ES, PNAME_EN} from './i18n.js';

const VERSION = '1.1.0';
const $ = id => document.getElementById(id);
const eng = new LiveEngine();
let params = [], pats = {A: [], B: []}, edit = 'A', sel = 0, patternName = PRESETS[0].name, running = false;
const undo = [], redo = [];

const log = line => { $('monitor').textContent = '[' + new Date().toLocaleTimeString('es-MX', {hour12: false}) + '] ' + line; };
const steps = () => pats[edit];

// ------------------------------------------------------------------ parameters
const CHOICES = {
  stut_len: ['1/4', '1/8', '1/16', '1/32'], mod_type: ['CHORUS', 'FLANGER', 'PHASER'], dly_time: ['1/16', '1/8', '3/16', '1/4', '3/8', '1/2', '1/8T', '1/4D'],
  oct: ['−2', '−1', '0', '+1', '+2'], voice: ['ACID', 'DIRTY', 'FIDGET'], quality: ['ECO 1×', 'HQ 2×', 'ULTRA 4×'],
};
const LOG = new Set(['pdec', 'cutoff', 'ring_freq', 'decay', 'vcadecay', 'glide', 'dyn_rate', 'mod_rate']);
const EASY_SOUND = ['voice', 'wave', 'cutoff', 'res', 'envmod', 'decay', 'accent', 'drive', 'fold', 'sub', 'pdrop', 'volume'];
const EXPERT_SOUND = ['detune', 'pdec', 'tune', 'oct', 'pw', 'noise', 'glide', 'drive2', 'tone', 'vcadecay', 'human'];
const FX = [
  ['CRUSH', 'crush_on', ['crush_mix', 'crush_bits', 'crush_rate']], ['RING MOD', 'ring_on', ['ring_mix', 'ring_freq']], ['STUTTER', 'stut_on', ['stut_len']],
  ['DYN FILTER', 'dyn_on', ['dyn_depth', 'dyn_rate', 'dyn_res']], ['CHORUS · FLANGER · PHASER', 'mod_on', ['mod_mix', 'mod_type', 'mod_rate', 'mod_depth', 'mod_fb']],
  ['SIDECHAIN PUMP', 'pump_on', ['pump']], ['COMPRESSOR', 'comp_on', ['comp_thr', 'comp_ratio', 'comp_make']], ['DELAY', 'dly_on', ['dly_mix', 'dly_time', 'dly_fb', 'dly_tone', 'dly_ping']], ['REVERB', 'rev_on', ['rev_mix', 'rev_size', 'rev_damp']],
];
const P = id => TABLE[ID[id]];
const pname = id => getLang() === 'es' ? (PNAME_ES[id] || P(id).id) : (PNAME_EN[id] || engName(id));
const engName = id => ({tune: 'Tune', oct: 'Octave', wave: 'Saw ⇄ Square', pw: 'Pulse Width', sub: 'Sub', noise: 'Noise', cutoff: 'Cutoff', res: 'Resonance', envmod: 'Env Mod', decay: 'Decay', accent: 'Accent', glide: 'Slide Time', drive: 'Drive', drive2: 'Overdrive', tone: 'Tone', vcadecay: 'Amp Decay', volume: 'Volume', human: 'Humanize', voice: 'Voice', detune: 'Detune', pdrop: 'Pitch Drop', pdec: 'Drop Time', fold: 'Filth', pump: 'Depth'}[id] || id);
function fmt(id, v) {
  if (CHOICES[id]) return CHOICES[id][Math.round(v - P(id).lo)] ?? v;
  if (id === 'cutoff' || id === 'ring_freq') return v >= 1000 ? (v / 1000).toFixed(2) + ' kHz' : Math.round(v) + ' Hz';
  if (id === 'pdrop') return v < .05 ? 'OFF' : v.toFixed(1) + ' st';
  if (['decay', 'vcadecay', 'glide', 'pdec'].includes(id)) return v < 1 ? Math.round(v * 1000) + ' ms' : v.toFixed(2) + ' s';
  if (['volume', 'comp_thr', 'comp_make'].includes(id)) return v.toFixed(1) + ' dB';
  if (id === 'tune') return (v > 0 ? '+' : '') + v.toFixed(2) + ' st';
  if (id === 'comp_ratio') return v.toFixed(1) + ':1';
  if (id === 'crush_bits') return v.toFixed(1) + ' bit';
  if (id === 'dyn_rate') return v.toFixed(1) + ' steps';
  if (id === 'mod_rate') return v.toFixed(2) + ' Hz';
  if (id === 'tone') return (v > 0 ? '+' : '') + Math.round(v * 100);
  if (id === 'wave') return v < .02 ? 'SAW' : v > .98 ? 'SQUARE' : Math.round((1 - v) * 100) + '/' + Math.round(v * 100);
  return Math.round(v * 100) + '%';
}
const toSlider = (id, v) => { const p = P(id); return LOG.has(id) ? Math.round(1000 * Math.log(v / p.lo) / Math.log(p.hi / p.lo)) : Math.round(1000 * (v - p.lo) / (p.hi - p.lo)); };
const fromSlider = (id, x) => { const p = P(id); return LOG.has(id) ? p.lo * Math.pow(p.hi / p.lo, x / 1000) : p.lo + (p.hi - p.lo) * x / 1000; };

function setP(id, v, {ui = true} = {}) {
  const p = P(id); if (!p) return;
  v = clamp(v, p.lo, p.hi); if (p.steps > 1) v = Math.round(v);
  params[p.i] = v; eng.param(p.i, v);
  if (ui) syncCtl(id);
  scheduleMidiCache();
}
function syncCtl(id) {
  const v = params[ID[id]];
  document.querySelectorAll('[data-p="' + id + '"]').forEach(el => {
    if (el.tagName === 'BUTTON') el.classList.toggle('on', v > .5);
    else if (el.tagName === 'SELECT') el.value = String(Math.round(v));
    else el.value = toSlider(id, v);
  });
  document.querySelectorAll('[data-out="' + id + '"]').forEach(o => o.textContent = fmt(id, v));
}
function control(id, cls = '') {
  const p = P(id), lab = document.createElement('label'); lab.className = 'ctl ' + cls;
  const name = document.createElement('span'); name.className = 'pn'; name.dataset.pn = id; name.textContent = pname(id).toUpperCase();
  const out = document.createElement('output'); out.dataset.out = id;
  lab.append(name, ' ', out);
  if (CHOICES[id] || (p.steps > 2)) {
    const s = document.createElement('select'); s.dataset.p = id;
    const names = CHOICES[id] || Array.from({length: p.steps}, (_, k) => String(p.lo + k));
    names.forEach((n, k) => { const o = document.createElement('option'); o.value = String(p.lo + k); o.textContent = n; s.append(o); });
    s.onchange = () => setP(id, +s.value); lab.append(s);
  } else {
    const r = document.createElement('input'); r.type = 'range'; r.min = 0; r.max = 1000; r.dataset.p = id;
    r.oninput = () => setP(id, fromSlider(id, +r.value));
    r.ondblclick = () => setP(id, p.def);
    lab.append(r);
  }
  return lab;
}
function buildSound() {
  const root = $('soundCtrl'); root.textContent = '';
  for (const id of EASY_SOUND) root.append(control(id));
  for (const id of EXPERT_SOUND) root.append(control(id, 'x-only'));
}
function buildFX() {
  const root = $('fxCards'); root.textContent = '';
  for (const [title, on, list] of FX) {
    const card = document.createElement('div'); card.className = 'fx-card';
    const head = document.createElement('div'); head.className = 'fx-head';
    const b = document.createElement('button'); b.className = 'fx-on'; b.dataset.p = on; b.textContent = title;
    b.onclick = () => setP(on, params[ID[on]] > .5 ? 0 : 1);
    head.append(b); card.append(head);
    list.forEach((id, k) => card.append(control(id, k ? 'x-only' : '')));
    root.append(card);
  }
}
const syncAll = () => { TABLE.forEach(p => syncCtl(p.id)); if (document.querySelector('#dirtyMacros .macro')) syncMacros(); };
function relabel() { document.querySelectorAll('[data-pn]').forEach(el => el.textContent = pname(el.dataset.pn).toUpperCase()); document.querySelectorAll('#dirtyMacros .macro b').forEach((b, i) => b.textContent = getLang() === 'es' ? MACROS[i][1] : MACROS[i][0]); }

// ------------------------------------------------------------------ pattern + grid
function pushUndo() { undo.push(JSON.stringify(pats)); if (undo.length > 80) undo.shift(); redo.length = 0; }
function commit() { eng.pattern(0, pats.A); eng.pattern(1, pats.B); grid(); scheduleMidiCache(); }
function setLength(n) {
  for (const k of ['A', 'B']) { const s = pats[k]; if (n > s.length) pats[k] = [...s, ...Array.from({length: n - s.length}, (_, i) => structuredClone(s[i % s.length]))]; else pats[k] = s.slice(0, n); }
  sel = Math.min(sel, n - 1);
}
function metadata() {
  const s = steps(), act = s.filter(x => x.active).length;
  $('activeNotes').textContent = String(act).padStart(2, '0') + ' / ' + s.length;
  $('stepInfo').textContent = s.length + ' STEPS / ' + (s.length / 16).toFixed(s.length % 16 ? 2 : 0) + ' BAR' + (s.length > 16 ? 'S' : '');
  $('dna').textContent = dnaHash(s).replace(/^(.{4})/, '$1-');
  $('abTag').textContent = edit; $('abTag').className = 'abtag ' + edit.toLowerCase();
}
function grid() {
  const root = $('grid'); root.textContent = '';
  const f = document.createDocumentFragment(), s = steps();
  s.forEach((st, i) => {
    const w = document.createElement('div');
    w.className = 'step ' + (st.active ? 'active' : 'rest') + (st.tie ? ' tied' : '') + (i === sel ? ' selected' : '') + (i % 4 === 0 ? ' beat' : '');
    w.dataset.i = i;
    const num = document.createElement('button'); num.className = 'step-index'; num.textContent = String(i + 1).padStart(2, '0'); num.title = 'Inspect';
    num.onclick = () => { sel = i; grid(); inspector(); };
    w.append(num);
    const badges = [];
    if (st.ratchet > 1) badges.push('×' + st.ratchet);
    if (st.prob < 100) badges.push(st.prob + '%');
    if (st.cond) badges.push(condName(st.cond));
    if (badges.length) { const b = document.createElement('span'); b.className = 'step-num'; b.textContent = badges.join(' '); w.append(b); }
    const main = document.createElement('button'); main.className = 'step-main';
    const pitch = document.createElement('span'); pitch.className = 'step-note'; pitch.textContent = st.active ? parseNoteName(st.note) : '—'; main.append(pitch);
    const bar = document.createElement('i'); bar.className = 'gatebar'; bar.style.width = (st.active ? st.gate : 0) + '%'; main.append(bar);
    main.onclick = e => { pushUndo(); if (e.shiftKey) { st.note = Math.max(24, st.note - 1); st.active = true; } else st.active = !st.active; sel = i; commit(); inspector(); };
    main.oncontextmenu = e => { e.preventDefault(); pushUndo(); st.note = Math.min(96, st.note + 1); st.active = true; sel = i; commit(); inspector(); };
    main.onwheel = e => { if (!st.active) return; e.preventDefault(); st.note = clamp(st.note + (e.deltaY < 0 ? 1 : -1), 24, 96); commit(); };
    w.append(main);
    const modes = document.createElement('div'); modes.className = 'step-mode';
    for (const [txt, key] of [['A', 'accent'], ['S', 'slide'], ['T', 'tie'], ['R', 'ratchet']]) {
      const b = document.createElement('button'); b.textContent = txt; b.title = key;
      b.className = key + ((key === 'ratchet' ? st.ratchet > 1 : st[key]) ? ' on' : '');
      b.onclick = () => {
        pushUndo();
        if (key === 'ratchet') st.ratchet = st.ratchet >= 4 ? 1 : st.ratchet + 1; else st[key] = !st[key];
        if (key === 'tie' && st.tie) { st.slide = false; const prev = s[(i - 1 + s.length) % s.length]; st.note = prev.note; }
        if (key === 'slide' && st.slide) st.tie = false;
        if (st[key] || key === 'ratchet') st.active = true;
        sel = i; commit(); inspector();
      };
      modes.append(b);
    }
    w.append(modes); f.append(w);
  });
  root.append(f); metadata();
}
function inspector() {
  const st = steps()[sel]; if (!st) return;
  $('inspStep').textContent = 'STEP ' + String(sel + 1).padStart(2, '0') + ' · ' + edit;
  $('iNote').value = st.note; $('iNoteOut').textContent = parseNoteName(st.note);
  $('iGate').value = st.gate; $('iGateOut').textContent = st.gate + '%';
  $('iVel').value = st.vel; $('iVelOut').textContent = st.vel;
  $('iProb').value = st.prob; $('iProbOut').textContent = st.prob + '%';
  $('iMicro').value = st.micro; $('iMicroOut').textContent = (st.micro > 0 ? '+' : '') + st.micro + '%';
  $('iRat').value = st.ratchet; $('iRatOut').textContent = st.ratchet;
  $('iCond').value = String(st.cond);
}
function bindInspector() {
  COND_LIST.forEach(c => { const o = document.createElement('option'); o.value = String(c); o.textContent = c ? condName(c) : 'ALWAYS'; $('iCond').append(o); });
  for (const [id, key] of [['iNote', 'note'], ['iGate', 'gate'], ['iVel', 'vel'], ['iProb', 'prob'], ['iMicro', 'micro'], ['iRat', 'ratchet']]) {
    $(id).onpointerdown = pushUndo;
    $(id).oninput = () => { const st = steps()[sel]; st[key] = +$(id).value; if (key === 'note') st.active = true; commit(); inspector(); };
  }
  $('iCond').onchange = () => { pushUndo(); steps()[sel].cond = +$('iCond').value; commit(); };
}

// ------------------------------------------------------------------ generator
const lock = () => ({notes: $('lockNotes').checked, rhythm: $('lockRhythm').checked, accents: $('lockAcc').checked, slides: $('lockSlide').checked});
const genOpts = () => ({mode: $('mode').value, root: +$('root').value, scale: $('scale').value, length: +$('length').value, density: +$('density').value / 100, chaos: +$('chaos').value / 100,
  probability: +$('gProb').value / 100, conditions: +$('gCond').value / 100, gates: +$('gGate').value / 100, poly: +$('poly').value, transpose: +$('gTrans').value || 0, seed: +$('seed').value || 1});
function doGenerate(seed) {
  pushUndo();
  if (seed !== undefined) $('seed').value = seed;
  const L = lock(), any = L.notes || L.rhythm || L.accents || L.slides;
  pats[edit] = generate({...genOpts(), keep: any ? steps() : null, lock: L});
  patternName = 'DFD / ' + MODES[$('mode').value].name.split(' /')[0] + ' ' + String($('seed').value).padStart(5, '0');
  $('customName').value = patternName; commit(); inspector();
  log(tr('logGen', 'Pattern generated') + ' · ' + steps().length + ' steps · seed ' + $('seed').value);
}
function applyStyleSound(m) {
  const snd = STYLE_SOUND[m]; if (!snd) return;
  for (const k of SOUND_KEYS) if (ID[k] !== undefined) setP(k, P(k).def, {ui: false});
  for (const [k, v] of Object.entries(snd)) setP(k, v, {ui: false});
  syncAll(); log('Style sound: ' + MODES[m].name);
}
function loadPreset(p) {
  pushUndo();
  for (const id of ['mode', 'root', 'scale']) $(id).value = p[id];
  $('bpm').value = p.bpm; setP('bpm', p.bpm); $('seed').value = p.seed; $('length').value = 16;
  TABLE.forEach(t => { if (!['bpm', 'quality', 'volume', 'audio', 'morph', 'swing'].includes(t.id)) setP(t.id, t.def, {ui: false}); });
  for (const [k, v] of Object.entries(p.snd)) setP(k, v, {ui: false});
  const g = {mode: p.mode, root: p.root, scale: p.scale, length: 16, density: .77, chaos: .43, seed: p.seed};
  pats.A = generate(g); pats.B = mutate(generate({...g, seed: p.seed + 1}), .3, g);
  patternName = p.name; $('customName').value = p.name; syncAll(); commit(); inspector(); log('Loaded ' + p.name);
}

// ------------------------------------------------------------------ DIRTY LAB
const MACROS = [
  ['FILTH', 'SUCIEDAD', f => { setP('fold', f * .8); setP('drive', .3 + f * .7); setP('drive2', f * .85); setP('crush_on', f > .6 ? 1 : 0); setP('crush_mix', Math.max(0, f - .6) * 1.2); setP('crush_bits', 12 - f * 6); }, () => params[ID.drive2] / .85],
  ['PUMP', 'BOMBEO', v => { setP('pump_on', v > .02 ? 1 : 0); setP('pump', v); }, () => params[ID.pump_on] > .5 ? params[ID.pump] : 0],
  ['ZAP', 'ZAP', v => { setP('pdrop', v * 24); setP('pdec', .02 + v * .12); }, () => params[ID.pdrop] / 24],
  ['WIDTH', 'ANCHO', v => setP('detune', v), () => params[ID.detune]],
  ['SCREAM', 'GRITO', v => { setP('res', .2 + v * .75); setP('envmod', .3 + v * .65); }, () => (params[ID.res] - .2) / .75],
  ['BITE', 'MORDIDA', v => setP('cutoff', 200 * Math.pow(30, v)), () => Math.log(params[ID.cutoff] / 200) / Math.log(30)],
];
function buildLab() {
  const riffs = $('dirtyRiffs'), sounds = $('dirtySounds'), mac = $('dirtyMacros');
  for (const m of DIRTY_MODES) { const b = document.createElement('button'); b.textContent = MODES[m].name; b.onclick = () => labGenerate(m); riffs.append(b); }
  DIRTY_SOUNDS.forEach((d, i) => { const b = document.createElement('button'); b.textContent = d.name; b.onclick = () => labSound(i); sounds.append(b); });
  MACROS.forEach(([en, es], i) => { const l = document.createElement('label'); l.className = 'macro'; l.innerHTML = '<b data-en="' + en + '">' + (getLang() === 'es' ? es : en) + '</b><input type="range" min="0" max="1000"><output></output>';
    const r = l.querySelector('input'); r.oninput = () => { MACROS[i][2](+r.value / 1000); syncMacros(); }; mac.append(l); });
  syncMacros();
}
function syncMacros() { document.querySelectorAll('#dirtyMacros .macro').forEach((l, i) => { const v = clamp(MACROS[i][3](), 0, 1); l.querySelector('input').value = Math.round(v * 1000); l.querySelector('output').textContent = Math.round(v * 100); }); }
function labSound(i) {
  const d = DIRTY_SOUNDS[i]; for (const k of SOUND_KEYS) if (ID[k] !== undefined) setP(k, P(k).def, {ui: false});
  for (const [k, v] of Object.entries(d.snd)) setP(k, v, {ui: false});
  syncAll(); syncMacros(); document.querySelectorAll('#dirtySounds button').forEach((b, j) => b.classList.toggle('on', j === i)); log('DIRTY SOUND · ' + d.name);
}
function labGenerate(m, seed = Math.floor(Math.random() * 99999)) {
  pushUndo(); $('seed').value = seed;
  const g = {...genOpts(), mode: m, seed, chaos: Math.min(+$('chaos').value / 100, .55), probability: 0, conditions: 0};
  pats[edit] = generate({...g, keep: null}); pats[edit === 'A' ? 'B' : 'A'] = mutate(generate({...g, seed: seed + 1}), .25, g);
  patternName = 'DFD / DIRTY ' + MODES[m].name + ' ' + String(seed).padStart(5, '0'); $('customName').value = patternName;
  commit(); inspector(); document.querySelectorAll('#dirtyRiffs button').forEach((b, j) => b.classList.toggle('on', DIRTY_MODES[j] === m)); log('DIRTY RIFF · ' + MODES[m].name + ' · seed ' + seed + ' (B = a variation: ride the morph)');
}
function dirtyGo() {
  labSound(Math.floor(Math.random() * DIRTY_SOUNDS.length));
  labGenerate(DIRTY_MODES[Math.floor(Math.random() * DIRTY_MODES.length)]);
  if (!running) start();
}

// ------------------------------------------------------------------ transport
async function ensureAudio() {
  if (eng.ctx && eng.ctx.state === 'running') return true;
  try { await eng.start(); } catch (e) { log('Audio failed: ' + e.message); return false; }
  params.forEach((v, i) => eng.param(i, v)); commit(); eng.channel(+$('channel').value); applyHwMode();
  return true;
}
async function start() {
  if (!(await ensureAudio())) return;
  eng.play(); running = true;
  $('play').classList.add('on'); $('engineState').classList.add('live'); $('engineState').querySelector('b').textContent = tr('engineOn', 'PLAYING');
  log('ACID ENGINE RUNNING / ' + params[ID.bpm] + ' BPM');
}
function stop() {
  eng.stop(); running = false; if (link.output) link.allNotesOff();
  $('play').classList.remove('on'); $('engineState').classList.remove('live'); $('engineState').querySelector('b').textContent = tr('engineIdle', 'ENGINE READY');
  document.querySelectorAll('.step.playhead').forEach(e => e.classList.remove('playhead'));
  log('TRANSPORT STOPPED');
}
const toggle = () => running ? stop() : start();

// ------------------------------------------------------------------ exports
const fileBase = () => (patternName || 'DFD ACID').replace(/[^A-Za-z0-9ÁÉÍÓÚÑáéíóúñ _-]+/g, '').replace(/\s+/g, '_') + '_' + dnaHash(steps());
const loopsFor = () => Math.max(1, Math.min(16, Math.round(128 / pats.A.length)));
async function renderNow(over = {}, loops = loopsFor(), tail = 2.5) { return render({params, patA: pats.A, patB: pats.B, loops, tail, overrides: over}); }
let midiURL = null, midiTimer = 0;
function scheduleMidiCache() { clearTimeout(midiTimer); midiTimer = setTimeout(cacheMidi, 450); }
async function cacheMidi() {
  try {
    const r = await renderNow({[ID.audio]: 1}, Math.max(1, Math.round(64 / pats.A.length)), 0.2);
    const bytes = eventsToMidi(r.events, {bpm: params[ID.bpm], stepS: r.stepS, name: patternName});
    if (midiURL) URL.revokeObjectURL(midiURL);
    midiURL = URL.createObjectURL(new Blob([bytes], {type: 'audio/midi'}));
    return bytes;
  } catch (e) { log('MIDI render error: ' + e.message); }
}
async function exportMidi() { const b = await cacheMidi(); if (b) { downloadBytes(b, fileBase() + '.mid', 'audio/midi'); log('MIDI exported: notes, gates, ties as legato, CC 102 slide / CC 103 accent. What you hear is what you get.'); } }
async function busy(btn, fn) { const t = btn.innerHTML; btn.disabled = true; btn.textContent = '…'; try { await fn(); } catch (e) { log('Render error: ' + e.message); } finally { btn.disabled = false; btn.innerHTML = t; } }
async function exportWav() { const r = await renderNow({[ID.audio]: 1}); downloadBytes(wav24(r.L, r.R, r.sr), fileBase() + '.wav', 'audio/wav'); log('WAV 48 kHz / 24-bit rendered from the DFD engine (' + (r.L.length / r.sr).toFixed(1) + ' s). Virtual synth, not hardware audio.'); }
async function exportStems() {
  const dry = {[ID.audio]: 1}; for (const [, on] of FX) dry[ID[on]] = 0;
  const full = await renderNow({[ID.audio]: 1}), d = await renderNow(dry);
  const fxL = full.L.map((x, i) => x - d.L[i]), fxR = full.R.map((x, i) => x - d.R[i]);
  downloadBytes(wav24(d.L, d.R, d.sr), fileBase() + '_STEM_SYNTH_DRY.wav', 'audio/wav');
  setTimeout(() => downloadBytes(wav24(fxL, fxR, d.sr), fileBase() + '_STEM_FX.wav', 'audio/wav'), 400);
  setTimeout(() => downloadBytes(wav24(full.L, full.R, d.sr), fileBase() + '_STEM_FULL.wav', 'audio/wav'), 800);
  log('Stems: SYNTH DRY + FX (full minus dry) + FULL, sample-aligned, 48 kHz / 24-bit.');
}

// ------------------------------------------------------------------ projects
function projectJSON() { const p = {}; TABLE.forEach(t => p[t.id] = params[t.i]); return {format: 'DFD-ACID', version: 2, app: VERSION, name: patternName, params: p, gen: genOpts(), A: pats.A, B: pats.B}; }
function loadProject(o) {
  if (o.format !== 'DFD-ACID') throw Error('Unknown format');
  const A = o.A || o.steps, B = o.B || A;
  if (!Array.isArray(A) || !A.length || A.length > 4096) throw Error('Invalid step count');
  pushUndo();
  const legacy = !o.version || o.version < 2; pats.A = A.map(x => sanitize(x, legacy)); pats.B = B.slice(0, A.length).map(x => sanitize(x, legacy)); while (pats.B.length < pats.A.length) pats.B.push(structuredClone(pats.A[pats.B.length]));
  if (o.params) for (const [k, v] of Object.entries(o.params)) if (ID[k] !== undefined && k !== 'audio') setP(k, +v, {ui: false});
  if (o.settings) { const s = o.settings; if (s.bpm) setP('bpm', s.bpm, {ui: false}); if (s.cutoff) setP('cutoff', s.cutoff, {ui: false}); if (s.resonance !== undefined) setP('res', s.resonance, {ui: false}); if (s.drive !== undefined) setP('drive', s.drive, {ui: false}); if (s.swing !== undefined) setP('swing', s.swing * .75, {ui: false}); }
  if (o.gen) for (const [k, id] of [['mode', 'mode'], ['root', 'root'], ['scale', 'scale']]) if (o.gen[k] !== undefined) $(id).value = o.gen[k];
  patternName = o.name || 'DFD / IMPORTED'; $('customName').value = patternName;
  $('bpm').value = params[ID.bpm]; $('length').value = [8, 16, 24, 32, 48, 64, 128, 256].includes(pats.A.length) ? pats.A.length : 256;
  $('swing').value = Math.round(params[ID.swing] * 100); sel = 0;
  syncAll(); readouts(); commit(); inspector();
}
const LIB = 'dfd-acid-library';
const readLib = () => { try { return JSON.parse(localStorage.getItem(LIB) || '{}'); } catch { return {}; } };
function saveLocal() { patternName = $('customName').value.trim() || 'DFD / UNTITLED'; const db = readLib(); db[patternName] = projectJSON(); try { localStorage.setItem(LIB, JSON.stringify(db)); log('Saved in this browser: ' + patternName); } catch (e) { log('Cannot save here: ' + e.message); } loadLibrary(); }
function loadLibrary() { const db = readLib(), el = $('savedPatterns'); el.innerHTML = '<option value="">—</option>'; Object.keys(db).sort().forEach(k => { const o = document.createElement('option'); o.value = k; o.textContent = k; el.append(o); }); }

// ------------------------------------------------------------------ TB-3 link
let snapshot = null;
const link = new TB3Link({
  log,
  onNote: e => {
    if (hwMode() === 'virtual') { if (e.type === 'on') { ensureAudio().then(() => eng.noteOn(e.note, e.velocity)); } else eng.noteOff(e.note); }
    if (e.type === 'on') log('TB-3 IN  NOTE ' + parseNoteName(e.note) + ' · VEL ' + e.velocity);
  },
  onCC: e => {
    if (hwMode() === 'virtual') { if (e.cc === 74) setP('cutoff', fromSlider('cutoff', e.value / 127 * 1000)); if (e.cc === 71) setP('res', e.value / 127); }
    if ([74, 71, 68, 69, 102, 103].includes(e.cc)) log('TB-3 IN  CC ' + e.cc + ' · ' + e.value);
  },
  onBlock: (name, bytes) => { if (name === 'patch' && bytes.length >= 11) { $('hwSaw').value = bytes[1]; $('hwDecay').value = bytes[10]; } },
});
const hwMode = () => $('hardwareMode').value;
function applyHwMode() {
  const m = hwMode();
  setP('audio', m === 'hardware' ? 0 : 1);
  if (m === 'off' || m === 'virtual') link.allNotesOff?.();
  log('TWIN LINK: ' + m.toUpperCase() + (m !== 'off' && !link.output ? ' (connect USB MIDI or DEMO first)' : ''));
}
eng.onEvents = list => {
  const m = hwMode(); if ((m !== 'hardware' && m !== 'twin') || !link.output) return;
  for (let k = 0; k < list.length; k += 2) { const p = list[k + 1], st = p & 255; if (st >= 0xf0) continue; link.send([st, (p >> 8) & 255, (p >> 16) & 255], eng.perfTime(list[k])); }
};
function fxTypes() {
  const efx1 = ['BYPASS', 'COMPRESSOR', 'RING MOD', 'BIT CRUSH', 'TREMOLO', 'CHORUS', 'FLANGER', 'PHASER', 'DELAY', 'PITCH SHIFT', 'EQ'];
  const efx2 = ['BYPASS', 'COMPRESSOR', 'RING MOD', 'BIT CRUSH', 'TREMOLO', 'CHORUS', 'FLANGER', 'PHASER', 'DELAY', 'REVERB'];
  const b = $('fxBlock').value, names = b === 'efx1' ? efx1 : efx2, t = $('fxType'), par = $('fxParameter');
  t.innerHTML = ''; names.forEach((n, i) => { const o = document.createElement('option'); o.value = i; o.textContent = n; t.append(o); });
  par.innerHTML = ''; for (let i = 1; i <= (b === 'efx1' ? 12 : 10); i++) { const o = document.createElement('option'); o.value = i; o.textContent = 'PARAMETER ' + i; par.append(o); }
}

// ------------------------------------------------------------------ perform
const KEYS = 'zsxdcvgbhnjm,';
const held = new Set();
function buildKeys() {
  const root = $('keys'); root.textContent = '';
  for (let k = 0; k < 13; k++) { const b = document.createElement('button'); const black = [1, 3, 6, 8, 10].includes(k % 12); b.className = 'key' + (black ? ' black' : ''); b.dataset.k = k; b.innerHTML = '<small>' + KEYS[k].toUpperCase() + '</small>';
    b.onpointerdown = e => { e.preventDefault(); keyOn(k); }; b.onpointerup = b.onpointerleave = () => keyOff(k); root.append(b); }
}
const keyNote = k => +$('root').value + k;
async function keyOn(k) { if (held.has(k)) return; held.add(k); if (!(await ensureAudio())) return; eng.noteOn(keyNote(k), 110); document.querySelector('.key[data-k="' + k + '"]')?.classList.add('down'); }
function keyOff(k) { if (!held.has(k)) return; held.delete(k); eng.noteOff(keyNote(k)); document.querySelector('.key[data-k="' + k + '"]')?.classList.remove('down'); }
const saved = {};
function padDown(pad) {
  if (pad === 'fill') setP('fill', 1);
  if (pad === 'stut') { saved.stut = params[ID.stut_on]; setP('stut_on', 1); }
  if (pad === 'acid') { saved.acid = [params[ID.cutoff], params[ID.res], params[ID.envmod]]; setP('cutoff', Math.min(4000, params[ID.cutoff] * 2.2)); setP('res', .95); setP('envmod', 1); }
  if (pad === 'mute') setP('mute', params[ID.mute] > .5 ? 0 : 1);
  document.querySelector('.pad[data-pad="' + pad + '"]')?.classList.toggle('on', pad === 'mute' ? params[ID.mute] > .5 : true);
}
function padUp(pad) {
  if (pad === 'fill') setP('fill', 0);
  if (pad === 'stut' && saved.stut !== undefined) { setP('stut_on', saved.stut); delete saved.stut; }
  if (pad === 'acid' && saved.acid) { setP('cutoff', saved.acid[0]); setP('res', saved.acid[1]); setP('envmod', saved.acid[2]); delete saved.acid; }
  if (pad !== 'mute') document.querySelector('.pad[data-pad="' + pad + '"]')?.classList.remove('on');
}
function bindXY() {
  const xy = $('xy'), dot = $('xyDot');
  const move = e => { const r = xy.getBoundingClientRect(), x = clamp((e.clientX - r.left) / r.width, 0, 1), y = clamp((e.clientY - r.top) / r.height, 0, 1);
    setP('morph', x); $('morph').value = Math.round(x * 100); $('morphOut').textContent = Math.round(x * 100) + '%';
    setP('cutoff', fromSlider('cutoff', (1 - y) * 1000)); placeDot(); };
  xy.onpointerdown = e => { xy.setPointerCapture(e.pointerId); move(e); xy.onpointermove = move; };
  xy.onpointerup = xy.onpointercancel = () => { xy.onpointermove = null; };
}
function placeDot() { $('xyDot').style.left = params[ID.morph] * 100 + '%'; $('xyDot').style.top = (1 - toSlider('cutoff', params[ID.cutoff]) / 1000) * 100 + '%'; }

// ------------------------------------------------------------------ visuals
let lastStep = -2;
function frame() {
  const t = eng.tele, cur = running ? (t[1] | 0) : -1;
  if (cur !== lastStep) {
    document.querySelectorAll('.step.playhead').forEach(e => e.classList.remove('playhead'));
    if (cur >= 0) document.querySelector('.step[data-i="' + cur + '"]')?.classList.add('playhead');
    lastStep = cur;
  }
  $('fNow').textContent = eng.ctx ? (t[4] >= 1000 ? (t[4] / 1000).toFixed(2) + ' kHz' : Math.round(t[4]) + ' Hz') : '— Hz';
  const lvl = Math.min(1, t[3] * 1.4);
  $('orbC').style.transform = 'scale(' + (1 + lvl * .35).toFixed(3) + ')'; $('orbC').style.boxShadow = '0 0 ' + Math.round(40 + lvl * 140) + 'px #c1ff3a' + (lvl > .5 ? 'aa' : '55');
  const c = $('scope'), g = c.getContext('2d'); g.clearRect(0, 0, c.width, c.height);
  if (eng.ctx) {
    const s = eng.scope, w = eng.scopeW, N = 256; g.lineWidth = 3; g.strokeStyle = '#c1ff3a'; g.shadowColor = '#c1ff3a'; g.shadowBlur = 12; g.beginPath();
    for (let k = 0; k < N; k++) { const v = s[(w + k) % N]; const x = k / (N - 1) * c.width, y = c.height / 2 - v * c.height * .42; k ? g.lineTo(x, y) : g.moveTo(x, y); }
    g.stroke(); g.shadowBlur = 0;
    g.fillStyle = '#a799ff'; g.font = '700 18px Helvetica, Arial'; g.fillText((t[6] >= 0 ? parseNoteName(t[6] | 0) : '—') + '  ·  ' + Math.round(t[4]) + ' Hz', 22, c.height - 26);
  }
  requestAnimationFrame(frame);
}

// ------------------------------------------------------------------ wiring
function readouts() { for (const [id, out, u] of [['density', 'densityOut', '%'], ['chaos', 'chaosOut', '%'], ['mutation', 'mutationOut', '%'], ['swing', 'swingOut', '%'], ['gProb', 'gProbOut', '%'], ['gCond', 'gCondOut', '%'], ['gGate', 'gGateOut', '%'], ['morph', 'morphOut', '%']]) $(out).textContent = $(id).value + u; }
function setLayer(x) { document.body.classList.toggle('expert', x); document.body.classList.toggle('easy', !x); $('layerExpert').classList.toggle('on', x); $('layerEasy').classList.toggle('on', !x); try { localStorage.setItem('dfd-acid-layer', x ? 'expert' : 'easy'); } catch {} }
function setLangUI(l) { applyLang(l); const ml = $('manLink'); if (ml) ml.href = '../manual/DFD-ACID-ENGINE-Web-Manual-' + (l === 'es' ? 'ES' : 'EN') + '.pdf'; $('langEN').classList.toggle('on', l !== 'es'); $('langES').classList.toggle('on', l === 'es'); relabel(); $('engineState').querySelector('b').textContent = running ? tr('engineOn', 'PLAYING') : tr('engineIdle', 'ENGINE READY'); }
function setEdit(k) { edit = k; $('editA').classList.toggle('on', k === 'A'); $('editB').classList.toggle('on', k === 'B'); grid(); inspector(); }

async function init() {
  await boot();
  params = TABLE.map(p => p.def);
  $('ver').textContent = VERSION;
  for (const [k, m] of Object.entries(MODES)) { if (m.lab) continue; const o = document.createElement('option'); o.value = k; o.textContent = m.name; $('mode').append(o); }
  for (const p of PRESETS) { const o = document.createElement('option'); o.value = p.name; o.textContent = p.name; $('preset').append(o); }
  for (let c = 1; c <= 16; c++) { const o = document.createElement('option'); o.value = c; o.textContent = 'CH ' + c; $('channel').append(o); }
  buildSound(); buildFX(); buildLab(); buildKeys(); bindInspector(); bindXY(); fxTypes();
  pats.A = Array.from({length: 16}, () => blankStep()); pats.B = structuredClone(pats.A);
  loadPreset(PRESETS[0]); undo.length = 0; loadLibrary(); readouts(); placeDot();

  $('dirtyGo').onclick = dirtyGo;
  $('play').onclick = start; $('heroPlay').onclick = start; $('stop').onclick = stop;
  $('panic').onclick = () => { eng.panic(); link.allNotesOff?.(); held.clear(); log('PANIC: all notes off (engine + TB-3).'); };
  $('bpm').onchange = () => { const v = clamp(+$('bpm').value || 132, 30, 300); $('bpm').value = v; setP('bpm', v); };
  $('swing').oninput = () => { setP('swing', +$('swing').value / 100); readouts(); };
  $('morph').oninput = () => { setP('morph', +$('morph').value / 100); readouts(); placeDot(); };
  $('quality').onchange = () => setP('quality', +$('quality').value);
  for (const id of ['density', 'chaos', 'mutation', 'gProb', 'gCond', 'gGate']) $(id).oninput = readouts;
  $('length').onchange = () => { pushUndo(); setLength(+$('length').value); commit(); inspector(); };
  $('editA').onclick = () => setEdit('A'); $('editB').onclick = () => setEdit('B');
  $('copyAB').onclick = () => { pushUndo(); pats.B = structuredClone(pats.A); commit(); log('A copied to B. Mutate B, then morph.'); };
  $('generate').onclick = () => doGenerate(Math.floor(Math.random() * 99999));
  $('dice').onclick = () => { $('seed').value = Math.floor(Math.random() * 99999); };
  $('seed').onchange = () => doGenerate();
  $('mode').onchange = () => { applyStyleSound($('mode').value); doGenerate(Math.floor(Math.random() * 99999)); };
  $('mutate').onclick = () => { pushUndo(); pats[edit] = mutate(steps(), +$('mutation').value / 100, genOpts(), lock()); commit(); inspector(); log('Mutation ' + $('mutation').value + '%'); };
  $('randomize').onclick = () => { pushUndo(); pats[edit] = mutate(steps(), .95, genOpts(), lock()); commit(); };
  $('clear').onclick = () => { pushUndo(); pats[edit] = steps().map(s => ({...s, active: false, accent: false, slide: false, tie: false, ratchet: 1, prob: 100, cond: 0, micro: 0})); commit(); inspector(); };
  $('shiftL').onclick = () => { pushUndo(); const s = steps(); s.push(s.shift()); commit(); };
  $('shiftR').onclick = () => { pushUndo(); const s = steps(); s.unshift(s.pop()); commit(); };
  $('loadPreset').onclick = () => loadPreset(PRESETS.find(p => p.name === $('preset').value) || PRESETS[0]);
  $('exportMidi').onclick = () => busy($('exportMidi'), exportMidi);
  $('exportWav').onclick = () => busy($('exportWav'), exportWav);
  $('exportStems').onclick = () => busy($('exportStems'), exportStems);
  $('dragMidi').ondragstart = e => { if (!midiURL) { e.preventDefault(); return; } e.dataTransfer.setData('DownloadURL', 'audio/midi:' + fileBase() + '.mid:' + midiURL); e.dataTransfer.setData('text/uri-list', midiURL); };
  $('dragMidi').onclick = () => busy($('exportMidi'), exportMidi);
  $('savePattern').onclick = saveLocal;
  $('loadSaved').onclick = () => { const k = $('savedPatterns').value; if (!k) return; try { loadProject(readLib()[k]); log('Recalled ' + k); } catch (e) { log('Cannot load: ' + e.message); } };
  $('exportProject').onclick = () => downloadBytes(new TextEncoder().encode(JSON.stringify(projectJSON(), null, 1)), fileBase() + '.dfdacid.json', 'application/json');
  $('importProject').onchange = async e => { const f = e.target.files?.[0]; if (!f) return; try { loadProject(JSON.parse(await f.text())); log('Imported ' + f.name); } catch (err) { log('Import rejected: ' + err.message); } e.target.value = ''; };
  $('layerEasy').onclick = () => setLayer(false); $('layerExpert').onclick = () => setLayer(true);
  $('langEN').onclick = () => setLangUI('en'); $('langES').onclick = () => setLangUI('es');

  // hardware
  $('connect').onclick = async () => { try { const st = await link.connect({sysex: true}); $('midiStatus').textContent = st.output ? '● ' + st.output.toUpperCase() : '● MIDI OK / NO OUTPUT'; log('MIDI ' + (st.sysex ? 'WITH' : 'WITHOUT') + ' SysEx'); } catch (e) { log('MIDI connection failed: ' + e.message); } };
  $('demo').onchange = () => { if ($('demo').checked) { link.useDemo(); $('midiStatus').textContent = tr('demoOn', '● DEMO TB-3 (SIMULATED)'); } else { link.disconnect(); $('midiStatus').textContent = '● NOT CONNECTED'; } };
  $('hardwareMode').onchange = applyHwMode;
  $('channel').onchange = () => { link.channel = +$('channel').value; eng.channel(+$('channel').value); };
  $('capture').onclick = () => { if (!link.input) { log('Connect USB MIDI first.'); return; } link.startCapture(); $('capture').disabled = true; $('captureStop').disabled = false; };
  $('captureStop').onclick = () => { const got = link.stopCapture({bpm: params[ID.bpm], length: steps().length}); if (got.some(s => s.active)) { pushUndo(); pats[edit] = got.map(x => sanitize(x, true)); commit(); log('TB-3 notes captured into pattern ' + edit + '.'); } else log('Capture empty: check the TB-3 MIDI transmit channel.'); $('capture').disabled = false; $('captureStop').disabled = true; };
  $('identity').onclick = () => { try { link.identity(); log('Identity Request sent.'); } catch (e) { log(e.message); } };
  $('readPatch').onclick = () => { try { link.requestSnapshot(); log('Reading patch / distortion / EFX1 / EFX2 blocks…'); } catch (e) { log(e.message); } };
  $('savePatch').onclick = () => { if (!Object.keys(link.rawBlocks).length) { log('Read the patch first: nothing to save yet.'); return; } downloadBytes(new TextEncoder().encode(link.snapshotJSON()), 'DFD_TB3_BACKUP_' + new Date().toISOString().replace(/[:.]/g, '-') + '.json', 'application/json'); };
  $('loadPatchFile').onchange = async e => { const f = e.target.files?.[0]; if (!f) return; try { const o = JSON.parse(await f.text()); if (o.type !== 'DFD-TB3-RAW' || !o.blocks) throw Error('not a DFD TB-3 backup'); snapshot = o.blocks; $('restorePatch').disabled = false; log('Backup loaded. Restore asks before writing.'); } catch (err) { log('Rejected: ' + err.message); } e.target.value = ''; };
  // restore: confirm -> read the current active patch -> download it as a safety backup -> only then write
  $('restorePatch').onclick = async () => {
    if (!snapshot) return;
    if (!confirm('Restore the ACTIVE TB-3 patch from this backup?\n\nFirst the current sound is read and downloaded as a safety backup. Then the backup is written to the active (edit) patch only, never to a saved user slot.')) return;
    try {
      link.rawBlocks = {}; link.requestSnapshot(); log('Reading the current patch for the safety backup…');
      await new Promise(r => setTimeout(r, 1200));
      if (Object.keys(link.rawBlocks).length < 4) { log('Safety backup failed (the TB-3 did not answer all 4 blocks). Nothing was written.'); return; }
      downloadBytes(new TextEncoder().encode(link.snapshotJSON()), 'DFD_TB3_SAFETY_BACKUP_' + new Date().toISOString().replace(/[:.]/g, '-') + '.json', 'application/json');
      link.restoreSnapshot(snapshot); log('Safety backup saved. Restoring the active patch from your backup.');
    } catch (err) { log(err.message); }
  };
  $('fxBlock').onchange = fxTypes;
  $('applyFxType').onclick = () => { if (!confirm('Write the FX type to the TB-3 active patch?')) return; try { link.writeEffect($('fxBlock').value, 0, +$('fxType').value); log('FX type written.'); } catch (e) { log(e.message); } };
  $('applyFxParameter').onclick = () => { try { link.writeEffect($('fxBlock').value, +$('fxParameter').value, +$('fxValue').value); log('FX parameter written.'); } catch (e) { log(e.message); } };
  for (const [id, c] of [['hwCutoff', 'cutoff'], ['hwResonance', 'resonance'], ['hwSaw', 'saw'], ['hwDecay', 'decay']]) $(id).oninput = e => { try { link.setHardwareSound(c, +e.target.value); } catch (err) { log(err.message); } };

  // perform
  document.querySelectorAll('.pad[data-pad]').forEach(b => { const p = b.dataset.pad; b.onpointerdown = async e => { e.preventDefault(); await ensureAudio(); padDown(p); }; b.onpointerup = b.onpointerleave = () => padUp(p); });
  document.querySelectorAll('.pad.tr').forEach(b => b.onclick = () => { setP('transpose', +b.dataset.tr); document.querySelectorAll('.pad.tr').forEach(x => x.classList.toggle('on', x === b)); });
  addEventListener('keydown', e => {
    if (e.target.closest('input[type=text],input[type=number],select')) return;
    if (e.code === 'Space') { e.preventDefault(); toggle(); return; }
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'z') { e.preventDefault(); const src = e.shiftKey ? redo : undo, dst = e.shiftKey ? undo : redo; if (src.length) { dst.push(JSON.stringify(pats)); pats = JSON.parse(src.pop()); commit(); inspector(); log(e.shiftKey ? 'Redo' : 'Undo'); } return; }
    if (e.ctrlKey || e.metaKey || e.altKey || e.repeat) return;
    const k = KEYS.indexOf(e.key.toLowerCase()); if (k >= 0) keyOn(k);
    if (e.key === 'f' || e.key === 'F') padDown('fill');
  });
  addEventListener('keyup', e => { const k = KEYS.indexOf(e.key.toLowerCase()); if (k >= 0) keyOff(k); if (e.key === 'f' || e.key === 'F') padUp('fill'); });

  let lang = 'en', layer = 'easy'; try { lang = localStorage.getItem('dfd-acid-lang') || 'en'; layer = localStorage.getItem('dfd-acid-layer') || 'easy'; } catch {}
  setLangUI(lang); setLayer(layer === 'expert');
  setInterval(() => { $('clock').textContent = new Date().toLocaleTimeString('es-MX', {hour12: false}); }, 1000);
  requestAnimationFrame(frame);
  window.DFD = {eng, get params() { return params; }, get pats() { return pats; }, ID, render: renderNow, eventsToMidi, setP, start, stop};
  log('DFD SYSTEM READY · ENGINE ' + VERSION + ' · ' + TABLE.length + ' PARAMETERS. PRESS PLAY.');
}
init().catch(e => { console.error(e); log('Startup error: ' + e.message); });
