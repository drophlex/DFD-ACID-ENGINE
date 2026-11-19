// DFD ACID ENGINE · original pattern generator, mutation and project helpers. No third-party code.
export const clamp = (v, a, b) => Math.max(a, Math.min(b, v));

// styles: each one is a genre's technique (rhythm grid, note palette, accent / slide habits), never a copied riff.
// rhythm: chance of a note on each 16th; pal: semitones from the root -> weight; acc: accent chance per 16th
const R = str => str.split(' ').map(Number);
export const MODES = {
  acid:       {name: 'ACID HOUSE / CLASSIC',  rhythm: R('.95 .45 .8 .5 .9 .4 .75 .55 .95 .45 .8 .5 .9 .4 .75 .6'), pal: {0: 6, 12: 3.5, 3: 1.4, 7: 1.4, 10: 1.1, 5: .6, '-12': .8, 1: .3},
                acc: R('.6 .1 .3 .2 .5 .1 .35 .2 .6 .1 .3 .2 .5 .1 .4 .3'), slide: .22, tie: .06, gate: [45, 70], ratchet: .02, motif: 8, range: [9, 14]},
  techno:     {name: 'ACID TECHNO / 3-3-2',    rhythm: R('.98 .85 .85 .98 .85 .85 .98 .85 .98 .85 .85 .98 .85 .85 .98 .85'), pal: {0: 8, 12: 3, 7: 1, 3: .9, 1: .5, 10: .5},
                acc: R('.9 0 0 .85 0 0 .8 0 .9 0 0 .85 0 0 .7 .1'), slide: .12, tie: .02, gate: [35, 55], ratchet: .03, motif: 4, range: [12, 16]},
  filter:     {name: 'FILTER HOUSE / FRENCH',  rhythm: R('.95 .1 .7 .35 .2 .5 .85 .2 .9 .1 .7 .35 .3 .55 .8 .4'), pal: {0: 5, 12: 4, 7: 1.6, 10: 1.6, 5: 1, 3: 1.1},
                acc: R('.5 0 .5 .2 0 .3 .6 0 .5 0 .5 .2 0 .3 .5 .2'), slide: .12, tie: .14, gate: [65, 95], ratchet: 0, motif: 8, range: [7, 11]},
  bigbeat:    {name: 'BIG BEAT / BREAKS',      rhythm: R('.98 .15 .3 .9 .1 .3 .85 .2 .4 .6 .2 .8 .15 .5 .7 .35'), pal: {0: 5, 3: 2, 5: 1.8, 6: .8, 7: 2, 10: 1.6, 12: 2.2},
                acc: R('.8 0 .1 .7 0 .1 .7 0 .2 .4 0 .6 0 .3 .5 .1'), slide: .18, tie: .1, gate: [50, 80], ratchet: .03, motif: 16, range: [7, 11]},
  rave:       {name: 'RAVE / HARDCORE',        rhythm: R('.95 .3 .6 .7 .5 .7 .6 .4 .9 .3 .7 .6 .5 .7 .65 .5'), pal: {0: 4, 3: 2, 7: 2.2, 10: 1.8, 12: 3, 15: 1, 5: .8},
                acc: R('.7 .1 .3 .5 .2 .5 .3 .1 .7 .1 .4 .4 .2 .5 .4 .3'), slide: .25, tie: .04, gate: [40, 65], ratchet: .06, motif: 8, range: [9, 13]},
  braindance: {name: 'BRAINDANCE / DRILL',     rhythm: R('.95 .4 .55 .5 .6 .35 .7 .5 .8 .45 .5 .55 .6 .4 .65 .6'), pal: {0: 3.5, 12: 2.2, 3: 1.2, 7: 1.2, 10: 1, 14: .8, '-12': 1, 1: .9, 6: .6, 8: .7},
                acc: R('.6 .2 .3 .4 .3 .2 .5 .3 .5 .3 .3 .4 .3 .3 .5 .4'), slide: .3, tie: .08, gate: [20, 80], ratchet: .28, motif: 16, range: [8, 14], probability: .5},
  electro:    {name: 'ELECTRO / PUNK',         rhythm: R('.98 .2 .9 .2 .95 .2 .9 .3 .98 .2 .9 .2 .95 .25 .9 .4'), pal: {0: 7, 12: 4, 7: 1.1, '-12': 1, 3: .7},
                acc: R('.7 0 .2 0 .6 0 .3 0 .7 0 .2 0 .6 0 .4 .2'), slide: .08, tie: .03, gate: [25, 45], ratchet: .05, motif: 4, range: [8, 12]},
  hypnotic:   {name: 'DEEP / HYPNOTIC',        rhythm: R('.95 .15 .35 .6 .2 .3 .55 .15 .7 .2 .4 .5 .2 .35 .5 .3'), pal: {0: 6, 12: 2, 3: 1.2, 10: 1.1, 7: 1.1, 5: .5},
                acc: R('.4 0 .2 .3 0 .1 .3 0 .4 0 .2 .3 0 .1 .3 .1'), slide: .28, tie: .2, gate: [60, 100], ratchet: 0, motif: 4, range: [5, 9]},
  dirty:      {name: 'ELECTRO HOUSE / DIRTY',  rhythm: R('.98 .35 .75 .55 .3 .8 .6 .4 .9 .4 .7 .6 .35 .8 .7 .5'), pal: {0: 6, 12: 5, 10: 1.4, 7: 1.4, 3: 1.2, '-12': 1.2, 5: .6},
                acc: R('.5 .2 .5 .3 .1 .6 .4 .2 .5 .2 .5 .3 .1 .6 .5 .3'), slide: .08, tie: .04, gate: [22, 45], ratchet: .04, motif: 8, range: [9, 13]},
  fidget:     {name: 'BLOGHOUSE / FIDGET',     rhythm: R('.95 .5 .3 .7 .4 .6 .2 .75 .9 .5 .35 .7 .45 .6 .3 .8'), pal: {0: 5, 12: 3, 7: 2, 10: 1.5, 3: 1.5, 15: 1, '-12': 1, 5: .8},
                acc: R('.6 .3 .1 .6 .2 .5 .1 .6 .6 .3 .1 .6 .2 .5 .1 .6'), slide: .1, tie: .03, gate: [15, 35], ratchet: .1, motif: 16, range: [8, 12]},
};
// DIRTY LAB: six riff families of filthy electro house (techniques, not copies). lab: true keeps them out of the main style list
Object.assign(MODES, {
  d_octave: {lab: true, name: 'OCTAVE PUMP',    rhythm: R('.98 .2 .95 .2 .98 .2 .95 .3 .98 .2 .95 .2 .98 .25 .95 .4'), pal: {0: 6, 12: 9, 10: .6, 7: .4}, acc: R('0 0 .9 0 0 0 .9 0 0 0 .9 0 0 0 .9 .2'), slide: .04, tie: 0, gate: [25, 40], ratchet: .03, motif: 4, range: [9, 12]},
  d_zap:    {lab: true, name: 'ZAP BOUNCE',     rhythm: R('.98 .1 .2 .85 .1 .3 .9 .15 .4 .7 .1 .85 .2 .3 .75 .4'), pal: {0: 6, 12: 3, 7: 1.5, 10: 1.5, 3: 1}, acc: R('.8 0 0 .7 0 .1 .8 0 .2 .6 0 .7 0 .1 .6 .2'), slide: .06, tie: .02, gate: [18, 32], ratchet: .05, motif: 8, range: [7, 10]},
  d_grind:  {lab: true, name: 'GRIND 16THS',    rhythm: R('.98 .9 .9 .9 .98 .9 .9 .9 .98 .9 .9 .9 .98 .9 .9 .95'), pal: {0: 9, 12: 2, '-12': 1, 1: .6, 3: .7}, acc: R('.7 .1 .1 .5 .2 .1 .5 .1 .7 .1 .1 .5 .2 .1 .5 .4'), slide: .1, tie: .02, gate: [30, 50], ratchet: .06, motif: 4, range: [13, 16]},
  d_call:   {lab: true, name: 'CALL & RESPONSE', rhythm: R('.98 .4 .7 .5 .6 .3 .5 .1 .98 .2 .3 .6 .7 .6 .8 .7'), pal: {0: 5, 12: 4, 10: 2, 7: 2, 3: 1.6, 5: 1}, acc: R('.7 .1 .4 .2 .3 .1 .2 0 .6 0 .1 .4 .5 .4 .6 .6'), slide: .14, tie: .05, gate: [25, 55], ratchet: .04, motif: 16, range: [9, 12]},
  d_stab:   {lab: true, name: 'STAB GATE',      rhythm: R('.2 .1 .98 .1 .2 .1 .98 .3 .2 .1 .98 .1 .3 .6 .98 .2'), pal: {0: 6, 12: 3, 7: 2, 3: 1.2}, acc: R('0 0 .9 0 0 0 .9 .2 0 0 .9 0 0 .4 .9 0'), slide: .02, tie: 0, gate: [15, 28], ratchet: .08, motif: 8, range: [4, 7]},
  d_drop:   {lab: true, name: 'DROP RIDE',      rhythm: R('.98 .5 .6 .7 .9 .5 .6 .7 .98 .6 .7 .8 .9 .8 .9 .95'), pal: {0: 6, 12: 4, '-12': 2, 7: 1.2, 10: 1}, acc: R('.8 .1 .2 .4 .7 .1 .2 .4 .8 .2 .3 .5 .7 .5 .6 .8'), slide: .2, tie: .02, gate: [28, 50], ratchet: .15, motif: 16, range: [11, 15]},
});
// DIRTY LAB sounds: voice + filth + pump presets, all original
export const DIRTY_SOUNDS = [
  {name: 'ROTTEN SAW',     snd: {voice: 1, detune: .55, fold: .45, cutoff: 1800, res: .35, envmod: .55, decay: .25, sub: .4, drive: .8, drive2: .55, pdrop: 5, pdec: .04, pump_on: 1, pump: .7, crush_on: 1, crush_bits: 10, crush_mix: .25}},
  {name: 'TEUTON GRIND',   snd: {voice: 1, detune: .3, fold: .75, cutoff: 1200, res: .5, envmod: .45, decay: .3, sub: .5, drive: 1, drive2: .8, pdrop: 0, pump_on: 1, pump: .8, tone: .3}},
  {name: 'ZAP CANNON',     snd: {voice: 2, detune: .45, fold: .3, cutoff: 2200, res: .4, envmod: .5, decay: .18, drive: .7, drive2: .5, pdrop: 14, pdec: .06, pump_on: 1, pump: .6}},
  {name: 'OCTAVE MONSTER', snd: {voice: 1, detune: .7, fold: .2, cutoff: 900, res: .3, envmod: .7, decay: .22, sub: .6, drive: .7, drive2: .4, pdrop: 2, pdec: .03, pump_on: 1, pump: .75}},
  {name: '8-BIT BRUISER',  snd: {voice: 0, wave: 1, pw: .25, cutoff: 3000, res: .3, envmod: .3, decay: .2, drive: .6, drive2: .6, crush_on: 1, crush_bits: 5, crush_rate: .55, crush_mix: .7, pump_on: 1, pump: .6}},
  {name: 'SQUEAL PIG',     snd: {voice: 0, wave: 0, cutoff: 700, res: .93, envmod: .85, decay: .3, drive: .9, drive2: .7, fold: .3, pump_on: 1, pump: .5, dly_mix: .18}},
  {name: 'BASS SLAB',      snd: {voice: 1, detune: .2, fold: .15, cutoff: 500, res: .25, envmod: .35, decay: .4, sub: .8, drive: .6, drive2: .3, pump_on: 1, pump: .85}},
  {name: 'GUTTER STAB',    snd: {voice: 1, detune: .4, fold: .55, cutoff: 2600, res: .45, envmod: .7, decay: .12, sub: .3, drive: .9, drive2: .6, pdrop: 7, pdec: .025, pump_on: 1, pump: .65, crush_on: 1, crush_bits: 8, crush_mix: .35}},
  {name: 'RAVE SIREN',     snd: {voice: 2, detune: .9, fold: .4, cutoff: 3500, res: .55, envmod: .4, decay: .5, drive: .5, drive2: .5, pdrop: 24, pdec: .25, pump_on: 1, pump: .5, rev_on: 1}},
];
export const DIRTY_MODES = ['d_octave', 'd_zap', 'd_grind', 'd_call', 'd_stab', 'd_drop'];
// each style's own sound: applied when you pick the style (the voice, the filter, the dirt, the pump)
export const SOUND_KEYS = ['voice', 'wave', 'pw', 'sub', 'detune', 'pdrop', 'pdec', 'fold', 'cutoff', 'res', 'envmod', 'decay', 'accent', 'drive', 'drive2', 'tone', 'pump_on', 'pump', 'crush_on', 'crush_bits', 'crush_mix', 'ring_on', 'dyn_on', 'mod_on', 'dly_mix', 'rev_on'];
export const STYLE_SOUND = {
  acid:       {voice: 0, wave: 0, cutoff: 430, res: .8, envmod: .62, decay: .42, drive: .35},
  techno:     {voice: 0, wave: 0, cutoff: 520, res: .84, envmod: .55, decay: .3, drive: .55, drive2: .3},
  filter:     {voice: 1, detune: .25, wave: .2, cutoff: 900, res: .45, envmod: .3, decay: .8, sub: .3, pump_on: 1, pump: .55},
  bigbeat:    {voice: 0, wave: .4, cutoff: 700, res: .6, envmod: .55, decay: .35, drive: .7, drive2: .4},
  rave:       {voice: 2, detune: .35, cutoff: 1400, res: .5, envmod: .4, decay: .35, drive: .45},
  braindance: {voice: 0, cutoff: 600, res: .86, envmod: .7, decay: .25, crush_on: 1, crush_bits: 9, crush_mix: .3},
  electro:    {voice: 0, wave: 1, pw: .4, cutoff: 800, res: .55, envmod: .5, decay: .2, drive: .5},
  hypnotic:   {voice: 0, cutoff: 360, res: .76, envmod: .45, decay: 1, dly_mix: .3},
  dirty:      {voice: 1, detune: .55, fold: .45, cutoff: 1800, res: .35, envmod: .55, decay: .25, sub: .4, drive: .8, drive2: .55, pdrop: 5, pdec: .04, pump_on: 1, pump: .7, crush_on: 1, crush_bits: 10, crush_mix: .25},
  fidget:     {voice: 2, detune: .4, cutoff: 1500, res: .55, envmod: .6, decay: .2, drive: .6, drive2: .4, pdrop: 12, pdec: .07, pump_on: 1, pump: .5},
};
const ALIAS = {broken: 'braindance', mutant: 'braindance', funk: 'filter', minimal: 'hypnotic'};
export const styleOf = m => MODES[m] || MODES[ALIAS[m]] || MODES.acid;
export const SCALES = {
  minor: [0, 2, 3, 5, 7, 8, 10], phrygian: [0, 1, 3, 5, 7, 8, 10], dorian: [0, 2, 3, 5, 7, 9, 10], harmonic: [0, 2, 3, 5, 7, 8, 11],
  blues: [0, 3, 5, 6, 7, 10], pentatonic: [0, 3, 5, 7, 10], locrian: [0, 1, 3, 5, 6, 8, 10], chromatic: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
};
// trig conditions, the same codes as the engine (engine/dfd_acid.h, enum Cond)
export const COND = {ALWAYS: 0, FILL: 1, NOFILL: 2, FIRST: 3, NOFIRST: 4, ab: (a, b) => 16 + (a - 1) * 8 + (b - 1)};
export function condName(c) {
  if (!c) return '—';
  if (c === 1) return 'FILL'; if (c === 2) return '!FILL'; if (c === 3) return '1ST'; if (c === 4) return '!1ST';
  if (c >= 16) { const a = ((c - 16) >> 3) + 1, b = ((c - 16) & 7) + 1; return a + ':' + b; }
  return '?';
}
export const COND_LIST = [0, 1, 2, 3, 4, COND.ab(1, 2), COND.ab(2, 2), COND.ab(1, 3), COND.ab(2, 3), COND.ab(3, 3), COND.ab(1, 4), COND.ab(2, 4), COND.ab(3, 4), COND.ab(4, 4), COND.ab(1, 8), COND.ab(8, 8)];

// original DFD / POLARIS factory patterns: a style, a key, a seed and a sound
const STYLE_SOUND_REF = (k, extra = {}) => ({...STYLE_SOUND[k], ...extra});
export const PRESETS = [
  {name: 'DFD / NIGHT SIGNAL',      mode: 'acid',     root: 36, scale: 'minor',     bpm: 132, seed: 617,   snd: {cutoff: 430, res: .79, envmod: .62, decay: .42, drive: .43, dly_mix: .2}},
  {name: 'POLARIS / GLASS TEETH',   mode: 'rave', root: 38, scale: 'phrygian',  bpm: 158, seed: 16044, snd: {cutoff: 890, res: .88, envmod: .5, decay: .25, drive: .51, crush_on: 1, crush_mix: .3}},
  {name: 'DFD / REACTION 04',       mode: 'bigbeat',  root: 41, scale: 'dorian',    bpm: 146, seed: 31471, snd: {cutoff: 720, res: .68, envmod: .75, decay: .5, drive: .73, drive2: .35}},
  {name: 'DARK FUNK / VIRUS',       mode: 'braindance', root: 33, scale: 'chromatic', bpm: 172, seed: 46898, snd: {cutoff: 520, res: .91, envmod: .7, decay: .3, drive: .82, ring_on: 1, ring_mix: .25}},
  {name: 'MÉRIDA / SUBSYSTEM',      mode: 'hypnotic', root: 29, scale: 'minor',     bpm: 124, seed: 62325, snd: {cutoff: 320, res: .73, envmod: .45, decay: .8, sub: .45, rev_on: 1, rev_mix: .22}},
  {name: 'POLARIS / STRANGE LOOP',  mode: 'techno',   root: 36, scale: 'dorian',    bpm: 138, seed: 77752, snd: {cutoff: 610, res: .84, envmod: .55, decay: .6, mod_on: 1, mod_type: 2, mod_mix: .4}},
  {name: 'DFD / HUESO NEGRO',       mode: 'filter',   root: 31, scale: 'blues',     bpm: 112, seed: 9031,  snd: {cutoff: 380, res: .66, envmod: .7, decay: .35, wave: 1, pw: .35, drive: .3}},
  {name: 'POLARIS / CENOTE',        mode: 'hypnotic', root: 34, scale: 'harmonic',  bpm: 120, seed: 4402,  snd: {cutoff: 280, res: .8, envmod: .4, decay: 1.2, rev_on: 1, rev_mix: .3, rev_size: .85, dly_mix: .3}},
  {name: 'DFD / MÁQUINA 808',       mode: 'electro',  root: 36, scale: 'phrygian',  bpm: 128, seed: 2718,  snd: {cutoff: 650, res: .6, envmod: .5, decay: .22, wave: 1, pw: .5, dyn_on: 1, dyn_rate: 8}},
  {name: 'DFD / FILTH MACHINE',      mode: 'dirty',    root: 33, scale: 'minor',     bpm: 128, seed: 6666,  snd: STYLE_SOUND_REF('dirty')},
  {name: 'POLARIS / ZAP ZAP',        mode: 'fidget',   root: 36, scale: 'dorian',    bpm: 130, seed: 3030,  snd: STYLE_SOUND_REF('fidget', {pdrop: 9})},
  {name: 'DARK FUNK / ÁCIDO RITUAL', mode: 'acid',    root: 38, scale: 'locrian',   bpm: 140, seed: 1999,  snd: {cutoff: 470, res: .9, envmod: .8, decay: .4, drive: .6, stut_on: 0}},
];

export function rng(seed = Date.now()) { let s = (seed >>> 0) || 1; return () => { s = (Math.imul(1664525, s) + 1013904223) >>> 0; return s / 4294967296; }; }
export const blankStep = (note = 36) => ({note, active: false, accent: false, slide: false, tie: false, gate: 55, vel: 100, ratchet: 1, prob: 100, cond: 0, micro: 0});

function pick(r, pal) { const e = Object.entries(pal); let t = 0; for (const [, w] of e) t += w; let x = r() * t; for (const [k, w] of e) { if ((x -= w) <= 0) return +k; } return +e[0][0]; }
const fold = n => { while (n < 24) n += 12; while (n > 96) n -= 12; return n; };   // keep notes in range without leaving the key
function snap(semi, scale) { if (scale.length >= 12) return semi; const o = Math.floor(semi / 12), pc = ((semi % 12) + 12) % 12; let best = scale[0], d = 99; for (const n of scale) { const dd = Math.abs(n - pc); if (dd < d) { d = dd; best = n; } } return o * 12 + best; }

// one candidate: a motif, repeated across the pattern with small variations and a turnaround at the end of each bar
function candidate(st, o, r) {
  const scale = SCALES[o.scale] || SCALES.minor, M = Math.min(st.motif, o.length), chaos = o.chaos, dens = o.density / .75;
  const pal = {...st.pal}; if (chaos > .5) { pal[14] = (pal[14] || 0) + chaos; pal[-12] = (pal[-12] || 0) + chaos * .6; }
  const motif = [];
  for (let i = 0; i < M; i++) {
    const s = blankStep(), k = i % 16;
    s.active = k === 0 || r() < Math.min(1, st.rhythm[k] * dens);
    const semi = k === 0 ? 0 : snap(pick(r, pal), scale);
    s.note = fold(o.root + semi + o.transpose);
    s.accent = s.active && r() < st.acc[k];
    s.gate = Math.round(st.gate[0] + r() * (st.gate[1] - st.gate[0]) * (.5 + o.gates));
    s.vel = s.accent ? 127 : 96 + Math.round(r() * 14);
    motif.push(s);
  }
  const out = [];
  for (let i = 0; i < o.length; i++) {
    const s = structuredClone(motif[i % M]), k = i % 16, bar = Math.floor(i / 16), turn = k >= 12 && (bar % 2 === 1 || o.length <= 16);
    // variation: later repeats change a note now and then (more with chaos), the bar's last beat gets a turnaround
    if (i >= M && r() < chaos * .25 + (turn ? .35 : 0)) {
      if (r() < .6) s.note = fold(o.root + snap(pick(r, pal), scale) + o.transpose);
      else s.active = !s.active || k === 0;
    }
    out.push(s);
  }
  out.forEach((s, i) => {
    if (!s.active) return;
    const prev = out[(i - 1 + out.length) % out.length];
    if (i > 0 && prev.active && prev.note !== s.note && r() < st.slide) s.slide = true;          // slides only between different notes
    else if (i > 0 && prev.active && r() < st.tie) { s.tie = true; s.note = prev.note; }
    if (r() < st.ratchet * (.3 + chaos)) s.ratchet = [2, 2, 3, 4][Math.floor(r() * 4)];
    const pp = Math.max(o.probability, st.probability ? st.probability * chaos : 0);
    if (pp > 0 && i % 4 !== 0 && r() < pp * .4) s.prob = [50, 75, 75][Math.floor(r() * 3)];
    if (o.conditions > 0 && i % 4 === 3 && r() < o.conditions * .5) s.cond = [COND.FILL, COND.ab(2, 2), COND.ab(1, 2), COND.ab(4, 4)][Math.floor(r() * 4)];
    if (s.slide) s.gate = Math.max(s.gate, 60);
  });
  out[0].active = true; out[0].slide = false; out[0].tie = false;
  return out;
}
// how usable a line is: anchored on the root, singable leaps, a groove that repeats, a sane amount of accents and slides
export function score(steps, st, root) {
  let sc = 0; const n16 = steps.length / 16, act = steps.filter(s => s.active);
  if (steps[0].active && (steps[0].note - root) % 12 === 0) sc += 3;
  const per = act.length / n16; sc -= per < st.range[0] ? (st.range[0] - per) * 1.5 : per > st.range[1] ? (per - st.range[1]) * 1.5 : -2;
  const rootish = act.filter(s => ((s.note - root) % 12 + 12) % 12 === 0).length / Math.max(1, act.length); sc += rootish >= .3 && rootish <= .7 ? 2 : -Math.abs(rootish - .5) * 4;
  let prev = null; for (const s of act) { if (prev !== null) { const d = Math.abs(s.note - prev); if (d > 12 && d !== 24) sc -= 1.2; else if (d > 7 && d !== 12) sc -= .3; } prev = s.note; }
  const acc = act.filter(s => s.accent).length / n16; sc += acc >= 2 && acc <= 6 ? 1 : -1;
  const sl = act.filter(s => s.slide).length / n16; sc += st.slide > .1 ? (sl >= 1 && sl <= 4 ? 1 : -.8) : 0;
  const distinct = new Set(act.map(s => s.note)).size; sc += distinct >= 2 && distinct <= 6 ? 1 : -1;
  if (steps.length >= 16) { let same = 0; for (let i = 0; i < 8; i++) same += steps[i].active === steps[i + 8].active && (!steps[i].active || steps[i].note === steps[i + 8].note) ? 1 : 0; sc += same >= 4 ? 1.5 : 0; }
  return sc;
}

// the generator: tries several candidates from the seed and keeps the most usable one (deterministic per seed).
// lock = {notes, rhythm, accents, slides} keeps those genes from `keep` (DNA lock)
export function generate({mode = 'acid', root = 36, scale = 'minor', length = 16, density = .75, chaos = .45, seed = Date.now(),
  probability = 0, conditions = 0, gates = .3, poly = 0, transpose = 0, keep = null, lock = {}} = {}) {
  const st = styleOf(mode), r = rng(seed), o = {root, scale, length, density, chaos, probability, conditions, gates, transpose};
  let best = null, bs = -1e9;
  for (let c = 0; c < 16; c++) { const cand = candidate(st, o, rng(Math.floor(r() * 4294967295))), v = score(cand, st, root + transpose); if (v > bs) { bs = v; best = cand; } }
  // polyrhythm: an accent cycle of 3, 5 or 7 across the bar
  if (poly) { const cy = [0, 3, 5, 7][poly] || 3; best.forEach((s, i) => { if (s.active) { s.accent = i % cy === 0; s.vel = s.accent ? 127 : Math.min(s.vel, 110); } }); }
  return keep ? applyLock(best, keep, lock) : best;
}

export function applyLock(fresh, old, lock) {
  return fresh.map((s, i) => {
    const o = old[i % old.length]; if (!o) return s;
    const n = {...s};
    if (lock.notes) n.note = o.note;
    if (lock.rhythm) { n.active = o.active; n.gate = o.gate; n.ratchet = o.ratchet; n.tie = o.tie; }
    if (lock.accents) n.accent = o.accent;
    if (lock.slides) n.slide = o.slide;
    return n;
  });
}

export function mutate(steps, amount = .38, config = {}, lock = {}) {
  const r = rng(Math.floor(Math.random() * 0xffffffff)), sc = SCALES[config.scale || 'minor'] || SCALES.minor;
  return steps.map((s, i) => {
    const n = {...s};
    // new notes come from the style's palette around the root, so a mutation stays in the genre
    if (!lock.notes && i % 16 !== 0 && r() < amount * .5) n.note = fold((config.root ?? 36) + (config.transpose || 0) + snap(pick(r, styleOf(config.mode).pal), sc));
    if (!lock.rhythm && i % 16 !== 0 && r() < amount * .3) n.active = !n.active;
    if (!lock.accents && r() < amount * .58) n.accent = !n.accent;
    if (!lock.slides && r() < amount * .3) n.slide = !n.slide && i > 0 && steps[i - 1].active && steps[i - 1].note !== n.note;
    if (!lock.rhythm && r() < amount * .22) n.ratchet = n.ratchet === 1 ? 2 + Math.floor(r() * 3) : 1;
    if (r() < amount * .2) n.gate = clamp(n.gate + Math.round((r() - .5) * 40), 10, 100);
    if (i === 0) { n.slide = false; n.tie = false; }
    return n;
  });
}

export function sanitize(st, legacy = false) {
  const s = blankStep();
  s.note = clamp(Number(st.note) || 36, 0, 127); s.active = !!st.active; s.accent = !!st.accent; s.slide = !!st.slide; s.tie = !!st.tie;
  s.gate = clamp(Number(st.gate ?? 55), 5, 100); s.vel = clamp(Number(st.vel ?? 100), 1, 127); s.ratchet = clamp(Number(st.ratchet) || 1, 1, 8);
  s.prob = clamp(Number(st.prob ?? 100), 0, 100); s.cond = COND_LIST.includes(Number(st.cond)) || Number(st.cond) >= 16 ? Number(st.cond) | 0 : 0; s.micro = clamp(Number(st.micro) || 0, -50, 50);
  // version-1 projects (and the capture) stored probability as 0..1
  if (legacy) s.prob = st.prob === undefined ? 100 : clamp(Math.round(Number(st.prob) * 100), 0, 100);
  return s;
}

export function parseNoteName(note) { const n = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']; return n[(note + 120) % 12] + (Math.floor(note / 12) - 1); }
export function downloadBytes(bytes, name, mime) { const url = URL.createObjectURL(new Blob([bytes], {type: mime})); const a = document.createElement('a'); a.href = url; a.download = name; document.body.append(a); a.click(); a.remove(); setTimeout(() => URL.revokeObjectURL(url), 4000); }
export function dnaHash(steps) { let h = 0x811c9dc5; for (const s of steps) { h ^= s.note + (s.active ? 211 : 0) + (s.accent ? 37 : 0) + (s.slide ? 97 : 0) + (s.tie ? 13 : 0) + (s.ratchet || 1) * 7 + s.prob + s.cond * 3; h = Math.imul(h, 0x01000193) >>> 0; } return h.toString(16).padStart(8, '0').toUpperCase(); }
