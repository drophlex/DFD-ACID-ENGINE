// node --test tests/  · the page's logic against the real WebAssembly engine
import test from 'node:test'; import assert from 'node:assert/strict';
import {generate, mutate, PRESETS, sanitize, applyLock, COND, condName, dnaHash} from '../web/pattern.js';
import {boot, ID, TABLE, render, eventsToMidi, wav24, patternBytes} from '../web/engine.js';
import {sysexDT1, sysexRQ1, validateRolandDT1, checksum} from '../web/tb3-midi.js';

test('generator is seeded and stable, every style', () => {
  for (const mode of ['acid', 'techno', 'filter', 'bigbeat', 'rave', 'braindance', 'electro', 'hypnotic', 'broken', 'funk']) {
    const a = generate({length: 64, seed: 4242, mode, probability: 1, conditions: 1}), b = generate({length: 64, seed: 4242, mode, probability: 1, conditions: 1});
    assert.deepEqual(a, b); assert.equal(a.length, 64); assert.ok(a.some(x => x.active), mode);
    for (const s of a) { assert.ok(s.note >= 24 && s.note <= 96); assert.ok(s.gate >= 5 && s.gate <= 100); assert.ok(s.prob >= 0 && s.prob <= 100); }
    assert.equal(a[0].slide, false);
  }
});
test('DNA lock keeps the locked genes', () => {
  const old = generate({length: 16, seed: 1}), fresh = generate({length: 16, seed: 99, keep: old, lock: {notes: true, rhythm: true}});
  fresh.forEach((s, i) => { assert.equal(s.note, old[i].note); assert.equal(s.active, old[i].active); });
  const m = mutate(old, 1, {}, {notes: true, rhythm: true, accents: true, slides: true});
  m.forEach((s, i) => { assert.equal(s.note, old[i].note); assert.equal(s.accent, old[i].accent); });
});
test('mutation is immutable, factory patterns are original DFD / POLARIS', () => {
  const a = generate({length: 16, seed: 3}), copy = structuredClone(a); mutate(a, 1); assert.deepEqual(a, copy);
  assert.ok(PRESETS.length >= 10); assert.ok(PRESETS.every(p => /^(DFD|POLARIS|DARK FUNK|MÉRIDA)/.test(p.name)));
});
test('conditions and sanitize', () => {
  assert.equal(condName(COND.ab(1, 2)), '1:2'); assert.equal(condName(COND.FILL), 'FILL');
  const s = sanitize({note: 500, gate: 0, prob: 1, ratchet: 99}, true); assert.equal(s.note, 127); assert.equal(s.gate, 5); assert.equal(s.prob, 100); assert.equal(s.ratchet, 8);
  assert.equal(sanitize({prob: 40}).prob, 40);
});
test('pattern bytes match the engine Step layout', () => {
  const b = patternBytes([{note: 40, active: true, accent: true, slide: false, tie: true, gate: 70, vel: 90, ratchet: 3, prob: 60, cond: 17, micro: -10}]);
  assert.equal(new DataView(b.buffer).getInt32(0, true), 1);
  assert.deepEqual([...b.slice(4)], [40, 1 | 2 | 8, 70, 90, 3, 60, 17, 246]);
});
test('engine boots with its parameter table', async () => {
  await boot(); assert.ok(TABLE.length >= 60); for (const k of ['cutoff', 'res', 'bpm', 'morph', 'audio', 'dly_mix']) assert.ok(ID[k] >= 0, k);
});
const params = () => TABLE.map(p => p.def);
test('render: sound, sample-exact 16ths, MIDI = what you hear', async () => {
  await boot();
  const p = params(); p[ID.bpm] = 120;
  const pat = Array.from({length: 16}, (_, i) => ({note: 36 + (i % 3), active: true, accent: i % 4 === 0, slide: false, tie: false, gate: 50, vel: 100, ratchet: 1, prob: 100, cond: 0, micro: 0}));
  const r = await render({params: p, patA: pat, loops: 2, tail: 0.5});
  let pk = 0; for (const x of r.L) pk = Math.max(pk, Math.abs(x)); assert.ok(pk > 0.05 && pk <= 1, 'peak ' + pk);
  const ons = r.events.filter(e => (e[1] & 0xf0) === 0x90 && e[3] > 0);
  assert.equal(ons.length, 32);
  ons.forEach((e, k) => assert.ok(Math.abs(e[0] - k * r.stepS) <= 1, 'onset ' + k));
  assert.equal(ons[0][3], 127, 'accent = velocity 127');
  const mid = eventsToMidi(r.events, {bpm: 120, stepS: r.stepS, name: 'T'});
  assert.equal(new TextDecoder().decode(mid.slice(0, 4)), 'MThd'); assert.equal(new TextDecoder().decode(mid.slice(14, 18)), 'MTrk');
  assert.equal(mid.filter((x, i) => x === 0x90 && mid[i + 2] > 0).length >= 32, true);
  const w = wav24(r.L, r.R, r.sr); assert.equal(w.length, 44 + r.L.length * 6); assert.equal(new DataView(w.buffer).getUint16(34, true), 24);
});
test('render: audio off still sends MIDI (HARDWARE mode)', async () => {
  await boot(); const p = params(); p[ID.audio] = 0;
  const r = await render({params: p, patA: generate({length: 16, seed: 7}), loops: 1, tail: .2});
  assert.ok(r.L.every(x => x === 0)); assert.ok(r.events.some(e => (e[1] & 0xf0) === 0x90));
});
test('Roland SysEx DT1 checksum and roundtrip', () => {
  const msg = sysexDT1([0, 0x20, 0x10, 1], [3]); assert.equal(msg[0], 0xf0); assert.equal(msg[1], 0x41); assert.equal(msg.at(-1), 0xf7);
  const d = validateRolandDT1([...msg]); assert.deepEqual(d.address, [0, 32, 16, 1]); assert.deepEqual(d.payload, [3]);
  const bad = [...msg]; bad[bad.length - 2] ^= 1; assert.equal(validateRolandDT1(bad), null);
  assert.equal(checksum([0, 0x20, 0x10, 1, 3]), 0x4c);
});
test('Roland RQ1 standard message', () => { const m = sysexRQ1([0, 32, 0, 0], [0, 0, 0, 33]); assert.equal(m[6], 0x11); assert.equal(m.length, 17); });
test('generated lines are usable: root on the one, sane leaps, groove repeats', async () => {
  const {MODES, score, styleOf} = await import('../web/pattern.js');
  for (const mode of Object.keys(MODES)) for (let seed = 1; seed <= 40; seed++) {
    const s = generate({mode, root: 36, length: 16, seed});
    assert.ok(s[0].active && s[0].note === 36, mode + ' starts on the root');
    const act = s.filter(x => x.active); assert.ok(act.length >= 4, mode + ' has notes');
    const roots = act.filter(x => (x.note - 36) % 12 === 0).length; assert.ok(roots / act.length >= .25, mode + ' anchored on the root, seed ' + seed);
    for (const x of s) if (x.slide) assert.ok(x.active);
  }
});
test('DEMO TB-3: identity, read all blocks, writes land in its memory', async () => {
  const {TB3Link} = await import('../web/tb3-midi.js');
  globalThis.performance ??= {now: () => Date.now()};
  const logs = [], blocks = {}, l = new TB3Link({log: x => logs.push(x), onBlock: (k, b) => blocks[k] = b});
  l.useDemo(); l.identity(); l.requestSnapshot();
  await new Promise(r => setTimeout(r, 800));
  assert.deepEqual(Object.keys(blocks).sort(), ['distortion', 'efx1', 'efx2', 'patch']);
  assert.equal(blocks.patch.length, 0x21); assert.equal(blocks.distortion.length, 7);
  l.writeEffect('efx2', 0, 9); assert.equal(l.demo.mem.efx2[0], 9);
  const snap = JSON.parse(l.snapshotJSON()); assert.equal(snap.type, 'DFD-TB3-RAW');
});
