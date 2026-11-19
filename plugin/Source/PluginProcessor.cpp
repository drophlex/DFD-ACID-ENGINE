#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace dfd;
static_assert(sizeof(Step) == 8, "the page sends 8-byte steps");

static const char* const CHOICE_VOICE[] = {"ACID", "DIRTY", "FIDGET"};
static const char* const CHOICE_STUT[] = {"1/4", "1/8", "1/16", "1/32"};
static const char* const CHOICE_MOD[] = {"CHORUS", "FLANGER", "PHASER"};
static const char* const CHOICE_DLY[] = {"1/16", "1/8", "3/16", "1/4", "3/8", "1/2", "1/8T", "1/4D"};
static const char* const CHOICE_Q[] = {"ECO 1x", "HQ 2x", "ULTRA 4x"};

DfdProcessor::DfdProcessor()
  : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  eng = std::make_unique<Engine>(); eng->init((float)sr);
  live.reserve(256); liveAudio.reserve(256);
  // one host parameter per engine parameter: ids are the engine's permanent ids, so automation survives updates
  for (int i = 0; i < P_COUNT; i++) {
    const ParamInfo& p = PARAMS[i];
    const juce::ParameterID pid { p.id, 1 };
    juce::RangedAudioParameter* r = nullptr;
    auto choice = [&](const char* const* names, int n) { juce::StringArray a; for (int k = 0; k < n; k++) a.add(names[k]); return new juce::AudioParameterChoice(pid, p.name, a, (int)p.def); };
    if (i == P_VOICE) r = choice(CHOICE_VOICE, 3);
    else if (i == P_STUT_LEN) r = choice(CHOICE_STUT, 4);
    else if (i == P_MOD_TYPE) r = choice(CHOICE_MOD, 3);
    else if (i == P_DLY_TIME) r = choice(CHOICE_DLY, 8);
    else if (i == P_QUALITY) r = choice(CHOICE_Q, 3);
    else if (p.steps == 2) r = new juce::AudioParameterBool(pid, p.name, p.def > 0.5f);
    else if (p.steps > 2) r = new juce::AudioParameterInt(pid, p.name, (int)p.lo, (int)p.hi, (int)p.def);
    else {
      juce::NormalisableRange<float> nr(p.lo, p.hi);
      if (i == P_CUTOFF || i == P_RING_FREQ) nr.setSkewForCentre(i == P_CUTOFF ? 900.f : 400.f);
      r = new juce::AudioParameterFloat(pid, p.name, nr, p.def);
    }
    addParameter(r); hp.push_back(r); applied[(size_t)i] = p.def;
  }
  for (int a = 0; a < 2; a++) { current[a].len = 16; for (auto& s : current[a].s) s = Step(); pending[a] = current[a]; }
}

void DfdProcessor::prepareToPlay(double sampleRate, int) {
  sr = sampleRate;
  eng->init((float)sr);
  for (int a = 0; a < 2; a++) { eng->pat[a].len = current[a].len; std::memcpy(eng->pat[a].s, current[a].s.data(), sizeof(Step) * (size_t)current[a].len); }
  for (int i = 0; i < P_COUNT; i++) { applied[(size_t)i] = hp[(size_t)i]->convertFrom0to1(hp[(size_t)i]->getValue()); eng->setParam(i, applied[(size_t)i]); }
  hostWasPlaying = false;
}

void DfdProcessor::processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals nd;
  Engine& E = *eng;
  const int n = buf.getNumSamples();
  // host automation / page moves -> engine (only what changed)
  for (int i = 0; i < P_COUNT; i++) { const float v = hp[(size_t)i]->convertFrom0to1(hp[(size_t)i]->getValue()); if (v != applied[(size_t)i]) { applied[(size_t)i] = v; E.setParam(i, v); } }
  // new patterns from the page
  for (int a = 0; a < 2; a++) if (patDirty[a].load()) { const juce::SpinLock::ScopedTryLockType tl(patLock); if (tl.isLocked()) {
    current[a] = pending[a]; E.pat[a].len = juce::jlimit(1, MAXSTEPS, current[a].len); std::memcpy(E.pat[a].s, current[a].s.data(), sizeof(Step) * (size_t)E.pat[a].len); patDirty[a] = false; } }
  E.midiCh = midiCh.load();
  // keys and pads from the page
  { const juce::SpinLock::ScopedTryLockType tl(liveLock); if (tl.isLocked()) { liveAudio.swap(live); live.clear(); } }
  for (auto& l : liveAudio) { if (l.t == 1) E.liveNote(l.note, l.vel); else E.liveOff(l.note); }
  liveAudio.clear();
  if (const int r = reqPlay.exchange(0)) { if (r == 1) E.play(); else if (r == 2) E.stop(); else E.releaseAll(); }
  // the DAW's transport: when it plays, the engine follows its tempo and its bar
  bool hpl = false; double ppq = 0, bpm = 0;
  if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) { hpl = pos->getIsPlaying(); if (auto b = pos->getBpm()) bpm = *b; if (auto q = pos->getPpqPosition()) ppq = *q; }
  if (bpm > 1 && hpl) E.setParam(P_BPM, (float)bpm);
  if (hpl) E.hostPosition(ppq, true);
  else if (hostWasPlaying) { E.hostPosition(0, false); applied[P_BPM] = -1; }   // back to the plug-in's own tempo
  hostWasPlaying = hpl;
  // MIDI in from the DAW plays the voice (a keyboard, a clip)
  juce::MidiBuffer outMidi;
  float* L = buf.getWritePointer(0); float* R = buf.getNumChannels() > 1 ? buf.getWritePointer(1) : nullptr;
  float tmpR[MAXBLOCK];
  auto it = midi.cbegin(); const auto end = midi.cend();
  int done = 0;
  while (done < n) {
    int chunk = std::min(n - done, MAXBLOCK);
    // split at the next incoming MIDI event, so notes land on the sample
    while (it != end && (*it).samplePosition <= done) {
      const auto m = (*it).getMessage();
      if (m.isNoteOn()) E.liveNote(m.getNoteNumber(), m.getVelocity()); else if (m.isNoteOff()) E.liveOff(m.getNoteNumber());
      else if (m.isAllNotesOff() || m.isAllSoundOff()) E.releaseAll();
      ++it;
    }
    if (it != end) chunk = std::min(chunk, std::max(1, (*it).samplePosition - done));
    E.nev = 0;
    E.process(L + done, R ? R + done : tmpR, chunk);
    for (int k = 0; k < E.nev; k++) { const auto& e = E.ev[k]; outMidi.addEvent(juce::MidiMessage(e.st, e.d1, e.d2), done + juce::jlimit(0, chunk - 1, e.at)); }
    E.nev = 0;
    done += chunk;
  }
  for (int c = 2; c < buf.getNumChannels(); c++) buf.clear(c, 0, n);
  midi.swapWith(outMidi);
}

// ---- the page
void DfdProcessor::setFromPage(int id, float v) {
  if (id < 0 || id >= P_COUNT) return;
  auto* p = hp[(size_t)id]; const float nv = p->convertTo0to1(juce::jlimit(PARAMS[id].lo, PARAMS[id].hi, v));
  if (std::abs(nv - p->getValue()) < 1e-7f) return;
  p->beginChangeGesture(); p->setValueNotifyingHost(nv); p->endChangeGesture();
}

void DfdProcessor::message(const juce::var& m) {
  const juce::String t = m["t"].toString();
  if (t == "p") setFromPage((int)m["id"], (float)(double)m["v"]);
  else if (t == "pat") { const int a = (int)m["ab"] ? 1 : 0; const juce::SpinLock::ScopedLockType sl(patLock); if (fromB64(pending[a], m["b64"].toString())) patDirty[a] = true; }
  else if (t == "play") reqPlay = 1;
  else if (t == "stop") reqPlay = 2;
  else if (t == "panic") reqPlay = 3;
  else if (t == "ch") midiCh = juce::jlimit(0, 15, (int)m["ch"]);
  else if (t == "on" || t == "off") { const juce::SpinLock::ScopedLockType sl(liveLock); if (live.size() < 250) live.push_back({ t == "on" ? 1 : 0, (int)m["n"], m.hasProperty("v") ? (int)m["v"] : 0 }); }
}

juce::String DfdProcessor::b64(const PatBuf& p) {
  juce::MemoryOutputStream mo; mo.writeInt(p.len); mo.write(p.s.data(), sizeof(Step) * (size_t)juce::jlimit(1, MAXSTEPS, p.len));
  return juce::Base64::toBase64(mo.getData(), mo.getDataSize());
}
bool DfdProcessor::fromB64(PatBuf& p, const juce::String& s) {
  juce::MemoryOutputStream mo; if (!juce::Base64::convertFromBase64(mo, s) || mo.getDataSize() < 12) return false;
  const auto* b = (const uint8_t*)mo.getData(); int len = (int)juce::ByteOrder::littleEndianInt(b);
  len = juce::jlimit(1, MAXSTEPS, std::min(len, (int)((mo.getDataSize() - 4) / sizeof(Step))));
  p.len = len; std::memcpy(p.s.data(), b + 4, sizeof(Step) * (size_t)len); return true;
}

juce::var DfdProcessor::projectForPage() {
  juce::DynamicObject::Ptr o = new juce::DynamicObject();
  juce::Array<juce::var> ps; for (int i = 0; i < P_COUNT; i++) ps.add(hp[(size_t)i]->convertFrom0to1(hp[(size_t)i]->getValue()));
  o->setProperty("params", ps);
  { const juce::SpinLock::ScopedLockType sl(patLock); o->setProperty("A", b64(patDirty[0] ? pending[0] : current[0])); o->setProperty("B", b64(patDirty[1] ? pending[1] : current[1])); }
  { const juce::SpinLock::ScopedLockType sl(uiLock); o->setProperty("ui", ui); }
  o->setProperty("sr", sr);
  return juce::var(o.get());
}

void DfdProcessor::snapshot(std::array<float, 8>& tele, std::array<float, 256>& scope, int& w, std::vector<float>& params) {
  // a racy but harmless read of plain floats, for the page's meters and playhead
  for (int i = 0; i < 8; i++) tele[(size_t)i] = eng->tele[i];
  for (int i = 0; i < 256; i++) scope[(size_t)i] = eng->scope[i];
  w = eng->scopeW;
  params.resize(P_COUNT); for (int i = 0; i < P_COUNT; i++) params[(size_t)i] = hp[(size_t)i]->convertFrom0to1(hp[(size_t)i]->getValue());
}

// ---- state: parameters by id, both patterns, the page's own settings
void DfdProcessor::getStateInformation(juce::MemoryBlock& dest) {
  juce::ValueTree v("DFDACID"); v.setProperty("version", 1, nullptr);
  for (int i = 0; i < P_COUNT; i++) v.setProperty(PARAMS[i].id, hp[(size_t)i]->convertFrom0to1(hp[(size_t)i]->getValue()), nullptr);
  { const juce::SpinLock::ScopedLockType sl(patLock); v.setProperty("patA", b64(patDirty[0] ? pending[0] : current[0]), nullptr); v.setProperty("patB", b64(patDirty[1] ? pending[1] : current[1]), nullptr); }
  { const juce::SpinLock::ScopedLockType sl(uiLock); v.setProperty("ui", ui, nullptr); }
  juce::MemoryOutputStream mo(dest, false); v.writeToStream(mo);
}
void DfdProcessor::setStateInformation(const void* data, int size) {
  auto v = juce::ValueTree::readFromData(data, (size_t)size);
  if (!v.isValid() || v.getType().toString() != "DFDACID") return;
  for (int i = 0; i < P_COUNT; i++) if (v.hasProperty(PARAMS[i].id)) { auto* p = hp[(size_t)i]; p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(PARAMS[i].lo, PARAMS[i].hi, (float)(double)v[PARAMS[i].id]))); }
  { const juce::SpinLock::ScopedLockType sl(patLock);
    if (fromB64(pending[0], v["patA"].toString())) patDirty[0] = true;
    if (fromB64(pending[1], v["patB"].toString())) patDirty[1] = true; }
  { const juce::SpinLock::ScopedLockType sl(uiLock); ui = v["ui"].toString(); }
}

juce::AudioProcessorEditor* DfdProcessor::createEditor() { return new DfdEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DfdProcessor(); }
