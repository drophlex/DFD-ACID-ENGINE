#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <atomic>
#include "dfd_acid.h"

// DFD ACID ENGINE plug-in: the same header-only engine the web page runs as WebAssembly.
// Host parameters are the single source of truth for the sound; the page and the DAW both move them.
class DfdProcessor : public juce::AudioProcessor {
public:
  DfdProcessor();
  ~DfdProcessor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }
  const juce::String getName() const override { return "DFD ACID ENGINE"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return true; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 4.0; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return "DFD ACID ENGINE"; }
  void changeProgramName(int, const juce::String&) override {}
  void getStateInformation(juce::MemoryBlock&) override;
  void setStateInformation(const void*, int) override;

  // ---- message thread (the editor)
  void message(const juce::var& m);                 // {t:'p'|'pat'|'play'|'stop'|'on'|'off'|'panic'|'ch', ...}, the web worklet's shapes
  juce::var projectForPage();                       // {params:[...], A:b64, B:b64, ui:string, host:bool}
  void setUi(const juce::String& s) { const juce::SpinLock::ScopedLockType sl(uiLock); ui = s; }
  void snapshot(std::array<float, 8>& tele, std::array<float, 256>& scope, int& w, std::vector<float>& params);
  double rate() const { return sr; }

private:
  std::unique_ptr<dfd::Engine> eng;
  std::vector<juce::RangedAudioParameter*> hp;      // one per engine parameter, same order as dfd::PARAMS
  std::array<float, dfd::P_COUNT> applied {};       // value last given to the engine (audio thread)
  double sr = 48000;

  // patterns from the page: written on the message thread, copied in the audio thread when it can take the lock
  struct PatBuf { int len = 16; std::array<dfd::Step, dfd::MAXSTEPS> s; };
  PatBuf pending[2], current[2]; std::atomic<bool> patDirty[2] { false, false };
  juce::SpinLock patLock, uiLock, liveLock;
  juce::String ui;
  struct Live { int t, note, vel; }; std::vector<Live> live, liveAudio;   // keys and pads played on the page
  std::atomic<int> reqPlay { 0 };                   // 1 play, 2 stop, 3 panic
  std::atomic<int> midiCh { 0 };
  bool hostWasPlaying = false, ownPlay = false;

  void setFromPage(int id, float v);
  static juce::String b64(const PatBuf& p);
  static bool fromB64(PatBuf& p, const juce::String& s);
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DfdProcessor)
};
