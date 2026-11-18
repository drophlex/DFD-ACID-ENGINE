#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <random>

DFDAcidEngineProcessor::DFDAcidEngineProcessor()
 : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
   params (*this, nullptr, "DFD_STATE", createLayout())
{
    generate (123456u, 0);
}

juce::AudioProcessorValueTreeState::ParameterLayout DFDAcidEngineProcessor::createLayout()
{
    using F = juce::AudioParameterFloat;
    using B = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout x;
    x.add (std::make_unique<F> (juce::ParameterID { "cutoff", 1 }, "Cutoff", juce::NormalisableRange<float> (80.f, 9000.f, 1.f, .31f), 550.f));
    x.add (std::make_unique<F> (juce::ParameterID { "resonance", 1 }, "Resonance", 0.f, .99f, .78f));
    x.add (std::make_unique<F> (juce::ParameterID { "drive", 1 }, "Drive", 0.f, 1.f, .48f));
    x.add (std::make_unique<F> (juce::ParameterID { "decay", 1 }, "Decay", .04f, 1.5f, .32f));
    x.add (std::make_unique<F> (juce::ParameterID { "env", 1 }, "Env mod", 0.f, 1.f, .67f));
    x.add (std::make_unique<F> (juce::ParameterID { "sub", 1 }, "Sub", 0.f, 1.f, .34f));
    x.add (std::make_unique<F> (juce::ParameterID { "delay", 1 }, "Echo", 0.f, .85f, .24f));
    x.add (std::make_unique<F> (juce::ParameterID { "glide", 1 }, "Glide", 0.f, .4f, .12f));
    x.add (std::make_unique<F> (juce::ParameterID { "swing", 1 }, "Swing", 0.f, .7f, 0.f));
    x.add (std::make_unique<F> (juce::ParameterID { "output", 1 }, "Output", juce::NormalisableRange<float> (-30.f, 3.f, .1f), -8.f));
    x.add (std::make_unique<F> (juce::ParameterID { "bpm", 1 }, "Fallback BPM", 30.f, 300.f, 132.f));
    x.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "wave", 1 }, "Oscillator", juce::StringArray { "Saw", "Square", "Triangle" }, 0));
    x.add (std::make_unique<B> (juce::ParameterID { "hardware", 1 }, "Send MIDI to external TB-3", false));
    return x;
}

void DFDAcidEngineProcessor::prepareToPlay (double sr, int)
{
    sampleRateHz = sr; phase = subPhase = 0; envelope = 0; filterLow = filterBand = 0;
    delayLeft.assign ((size_t) std::max (1.0, sr * 2), 0.f);
    delayRight.assign (delayLeft.size(), 0.f);
    delayIndex = 0; fallbackSamples = 0; activePatternStep = -1; activeRatchet = -1;
}

void DFDAcidEngineProcessor::startNote (int note, bool accent, bool slide)
{
    targetPitch = 440.0 * std::pow (2.0, (note - 69) / 12.0);
    if (!slide || envelope < .015f) currentPitch = targetPitch;
    envelope = accent ? 1.15f : .76f;
    currentNote = note;
}

float DFDAcidEngineProcessor::processOscillator (double p) const
{
    const int wave = (int) get ("wave");
    if (wave == 1) return p < .5 ? 1.f : -1.f;
    if (wave == 2) return (float) (1.0 - 4.0 * std::abs (p - .5));
    return (float) (2.0 * p - 1.0);
}

void DFDAcidEngineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    auto pat = pattern.load();
    if (!pat || pat->empty()) return;
    const int n = buffer.getNumSamples();
    const double sr = sampleRateHz;
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;
    auto position = getPlayHead() != nullptr ? getPlayHead()->getPosition() : std::nullopt;
    const double tempo = position && position->getBpm() ? *position->getBpm() : (double) get ("bpm");
    const bool transportRunning = position && position->getIsPlaying();
    const double startingPPQ = position && position->getPpqPosition() ? *position->getPpqPosition() : ((double) fallbackSamples / sr) * (tempo / 60.0);
    juce::MidiBuffer outputMidi;
    if (!transportRunning) // Live MIDI keyboard when stopped, native instrument mode
      for (const auto m : midi)
      {
          auto msg = m.getMessage();
          if (msg.isNoteOn()) startNote (msg.getNoteNumber(), msg.getVelocity() >= 112, false);
          else if (msg.isAllNotesOff() || msg.isNoteOff()) envelope *= .3f;
      }
    else outputMidi.addEvents (midi, 0, n, 0);

    const float cutoff = get ("cutoff"), resonance = get ("resonance"), drive = get ("drive"),
      envMod = get ("env"), sub = get ("sub"), delayAmount = get ("delay"),
      decay = get ("decay"), glide = get ("glide"), outputGain = juce::Decibels::decibelsToGain (get ("output"));
    const bool sendHardware = get ("hardware") > .5f;
    if (sendHardware)
    {
        const auto command = tb3Command.exchange (0);
        if (command & 0x80000000u)
        {
            const int block = (int) ((command >> 16) & 0xff), parameter = (int) ((command >> 8) & 0xff), value = (int) (command & 0xff);
            const int base = block == 0 ? 0 : block == 1 ? 0x10 : block == 2 ? 0x20 : 0x30;
            const int offset = block >= 2 && parameter > 0 ? 2 + 2 * (parameter - 1) : parameter;
            std::vector<juce::uint8> data { 0x41, 0x10, 0, 0, 0x7b, 0x12, 0, 0x20, (juce::uint8) base, (juce::uint8) offset };
            if (block >= 2 && parameter > 0) { data.push_back ((juce::uint8) ((value >> 4) & 15)); data.push_back ((juce::uint8) (value & 15)); }
            else data.push_back ((juce::uint8) value);
            int sum = 0; for (size_t j = 6; j < data.size(); ++j) sum += data[j];
            data.push_back ((juce::uint8) ((128 - (sum % 128)) % 128));
            outputMidi.addEvent (juce::MidiMessage::createSysExMessage (data.data(), (int) data.size()), 0);
        }
    }
    const float freqDecay = std::exp (-1.f / (float) std::max (1.0, sr * (double) decay));
    const double hzStep = tempo / 60.0 * 4.0 / sr;
    for (int i = 0; i < n; ++i)
    {
        if (transportRunning)
        {
            const double beat = startingPPQ * 4.0 + (double) i * hzStep;
            const int64_t absoluteStep = (int64_t) std::floor (beat + 1.e-9);
            const size_t index = (size_t) ((absoluteStep % (int64_t) pat->size() + (int64_t) pat->size()) % (int64_t) pat->size());
            const AcidStep& step = (*pat)[index];
            const int subdivision = std::max (1, step.ratchet);
            const int ratchetIndex = std::min (subdivision - 1, (int) std::floor ((beat - std::floor (beat)) * subdivision));
            if ((int) index != activePatternStep || ratchetIndex != activeRatchet)
            {
                if (currentNote >= 0 && sendHardware) outputMidi.addEvent (juce::MidiMessage::noteOff (1, currentNote), i);
                activePatternStep = (int) index; activeRatchet = ratchetIndex;
                if (step.active)
                {
                    startNote (step.note, step.accent, step.slide && ratchetIndex == 0);
                    if (sendHardware)
                    {
                        outputMidi.addEvent (juce::MidiMessage::controllerEvent (1, 102, step.slide ? 127 : 0), i);
                        outputMidi.addEvent (juce::MidiMessage::controllerEvent (1, 103, step.accent ? 127 : 0), i);
                        outputMidi.addEvent (juce::MidiMessage::noteOn (1, step.note, (juce::uint8) (step.accent ? 125 : 100)), i);
                    }
                }
                else { envelope *= .15f; currentNote = -1; }
            }
        }
        const double glideCoefficient = std::exp (-1.0 / (sr * std::max (.0001, (double) glide)));
        currentPitch = glideCoefficient * currentPitch + (1.0 - glideCoefficient) * targetPitch;
        phase += currentPitch / sr; phase -= std::floor (phase);
        subPhase += currentPitch / (2.0 * sr); subPhase -= std::floor (subPhase);
        float x = processOscillator (phase) * .56f + std::sin (2.0 * juce::MathConstants<double>::pi * subPhase) * sub * .33f;
        const float cut = juce::jlimit (55.f, 12000.f, cutoff * (1.f + envelope * envMod * 3.5f));
        // Stable two-pole LP approximation with drive and resonant feedback.
        const float coef = 1.f - std::exp (-6.283185f * cut / (float) sr);
        filterBand += coef * (x - filterLow - filterBand * resonance * .8f);
        filterLow += coef * filterBand;
        x = std::tanh (filterLow * (1.f + drive * 25.f)) * envelope * .55f;
        envelope *= freqDecay;
        const size_t size = delayLeft.size();
        const size_t offset = (size_t) std::min ((double) size - 1.0, (60.0 / tempo) * .75 * sr);
        const size_t read = (delayIndex + size - offset) % size;
        const float dl = delayLeft[read], dr = delayRight[read];
        const float a = x + dl * delayAmount * .26f, b = x + dr * delayAmount * .28f;
        delayLeft[delayIndex] = x + dr * delayAmount * .48f;
        delayRight[delayIndex] = x + dl * delayAmount * .44f;
        delayIndex = (delayIndex + 1) % size;
        left[i] = std::tanh (a) * outputGain;
        right[i] = std::tanh (b) * outputGain;
    }
    fallbackSamples += n;
    midi.swapWith (outputMidi);
}

void DFDAcidEngineProcessor::setPatternLength (int length)
{
    if (length < 1) return;
    auto old = pattern.load();
    auto p = std::make_shared<std::vector<AcidStep>> (old ? *old : std::vector<AcidStep>{});
    p->resize ((size_t) length);
    pattern.store (p);
}

void DFDAcidEngineProcessor::queueTB3Parameter (int block, int parameter, int value)
{
    if (block < 0 || block > 3 || value < 0 || value > 255 || parameter < 0) return;
    if (block == 0 && !(parameter == 1 || parameter == 3 || parameter == 10 || parameter == 32)) return;
    if (block == 1 && parameter > 6) return;
    if (block == 2 && parameter > 12) return;
    if (block == 3 && parameter > 10) return;
    if (block < 2 && value > 127) return;
    if (block == 1 && parameter == 1 && value > 24) return;
    if (block == 2 && parameter == 0 && value > 10) return;
    if (block == 3 && parameter == 0 && value > 9) return;
    tb3Command.store (0x80000000u | ((uint32_t) block << 16) | ((uint32_t) parameter << 8) | (uint32_t) value);
}

void DFDAcidEngineProcessor::setStep (size_t index, AcidStep step)
{
    auto old = pattern.load();
    if (!old || index >= old->size()) return;
    auto p = std::make_shared<std::vector<AcidStep>> (*old);
    step.note = juce::jlimit (0, 127, step.note); step.ratchet = juce::jlimit (1, 32, step.ratchet);
    (*p)[index] = step; pattern.store (p);
}

void DFDAcidEngineProcessor::generate (int seed, int mode)
{
    std::mt19937 r ((unsigned) seed);
    std::uniform_real_distribution<float> u (0.f, 1.f);
    auto old = pattern.load(); const size_t count = old ? old->size() : 16;
    auto p = std::make_shared<std::vector<AcidStep>> (count);
    const int scale[] { 0, 2, 3, 5, 7, 8, 10 };
    int degree = 0;
    for (auto& s : *p)
    {
        degree = juce::jlimit (-10, 18, degree + (int) (u(r) * (mode == 0 ? 5 : 11)) - (mode == 0 ? 2 : 5));
        const int oct = degree >= 0 ? degree / 7 : (degree - 6) / 7;
        const int sc = ((degree % 7) + 7) % 7;
        s.note = juce::jlimit (24, 96, 36 + oct * 12 + scale[sc]);
        s.active = u(r) > (mode == 0 ? .12f : .24f);
        s.accent = u(r) < (mode == 2 ? .55f : .32f);
        s.slide = u(r) < (mode == 0 ? .33f : .2f);
        s.ratchet = u(r) < (mode == 0 ? .02f : .3f) ? 2 + (int) (u(r) * 3) : 1;
    }
    pattern.store (p);
}

void DFDAcidEngineProcessor::mutate (float amount)
{
    auto old = pattern.load(); if (!old) return;
    auto p = std::make_shared<std::vector<AcidStep>> (*old);
    std::mt19937 rng ((unsigned) juce::Time::getMillisecondCounter());
    std::uniform_real_distribution<float> u (0.f, 1.f);
    for (auto& s : *p)
    {
        if (u(rng) < amount * .4f) s.note = juce::jlimit (24, 96, s.note + (u(rng) < .5f ? -2 : 2));
        if (u(rng) < amount * .3f) s.active = !s.active;
        if (u(rng) < amount * .4f) s.accent = !s.accent;
        if (u(rng) < amount * .3f) s.slide = !s.slide;
    }
    pattern.store (p);
}

bool DFDAcidEngineProcessor::exportMidi (const juce::File& target, int repeats) const
{
    auto pat = pattern.load(); if (!pat || pat->empty()) return false;
    juce::MidiMessageSequence seq;
    constexpr int ppq = 960;
    seq.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / get ("bpm"))), 0.0);
    for (int r = 0; r < repeats; ++r) for (size_t i = 0; i < pat->size(); ++i)
    {
        auto& s = (*pat)[i]; if (!s.active) continue;
        double start = (double) ((r * pat->size() + i) * (ppq / 4));
        seq.addEvent (juce::MidiMessage::controllerEvent (1, 102, s.slide ? 127 : 0), start);
        seq.addEvent (juce::MidiMessage::controllerEvent (1, 103, s.accent ? 127 : 0), start);
        for (int j = 0; j < s.ratchet; ++j)
        {
            const double a = start + j * (ppq / 4.0 / s.ratchet);
            seq.addEvent (juce::MidiMessage::noteOn (1, s.note, (juce::uint8) (s.accent ? 125 : 96)), a);
            seq.addEvent (juce::MidiMessage::noteOff (1, s.note), a + (ppq / 4.0 / s.ratchet) * .82);
        }
    }
    seq.updateMatchedPairs();
    juce::MidiFile file; file.setTicksPerQuarterNote (ppq); file.addTrack (seq);
    target.deleteFile();
    auto stream = target.createOutputStream();
    return stream != nullptr && file.writeTo (*stream);
}

void DFDAcidEngineProcessor::getStateInformation (juce::MemoryBlock& data)
{
    auto v = params.copyState();
    juce::ValueTree sequence ("SEQUENCE");
    auto pat = pattern.load();
    if (pat) for (const auto& s : *pat)
    {
        juce::ValueTree node ("STEP");
        node.setProperty ("note", s.note, nullptr);
        node.setProperty ("active", s.active, nullptr);
        node.setProperty ("accent", s.accent, nullptr);
        node.setProperty ("slide", s.slide, nullptr);
        node.setProperty ("ratchet", s.ratchet, nullptr);
        sequence.addChild (node, -1, nullptr);
    }
    v.addChild (sequence, -1, nullptr);
    if (auto xml = v.createXml()) copyXmlToBinary (*xml, data);
}

void DFDAcidEngineProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size); if (!xml) return;
    auto tree = juce::ValueTree::fromXml (*xml); if (!tree.isValid()) return;
    auto seq = tree.getChildWithName ("SEQUENCE");
    if (seq.isValid() && seq.getNumChildren() > 0)
    {
        auto p = std::make_shared<std::vector<AcidStep>>();
        p->reserve ((size_t) seq.getNumChildren());
        for (auto node : seq)
        {
            AcidStep s;
            s.note = (int) node.getProperty ("note", 36);
            s.active = (bool) node.getProperty ("active", true);
            s.accent = (bool) node.getProperty ("accent", false);
            s.slide = (bool) node.getProperty ("slide", false);
            s.ratchet = juce::jlimit (1, 32, (int) node.getProperty ("ratchet", 1));
            p->push_back (s);
        }
        pattern.store (p);
        tree.removeChild (seq, nullptr);
    }
    params.replaceState (tree);
}

juce::AudioProcessorEditor* DFDAcidEngineProcessor::createEditor() { return new DFDAcidEngineEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DFDAcidEngineProcessor(); }
