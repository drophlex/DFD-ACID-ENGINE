#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

struct AcidStep
{
    int note = 36;
    bool active = true, accent = false, slide = false;
    int ratchet = 1;
};

class DFDAcidEngineProcessor final : public juce::AudioProcessor
{
public:
    DFDAcidEngineProcessor();
    ~DFDAcidEngineProcessor() override = default;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {};
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DFD ACID ENGINE"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "DFD / USER"; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    void generate (int seed, int mode);
    void mutate (float amount);
    void setPatternLength (int requestedLength);
    void setStep (size_t index, AcidStep s);
    // Queue documented active-patch SysEx to be emitted by DAW MIDI output. Never writes user-bank memory.
    void queueTB3Parameter (int block, int parameter, int value);
    std::shared_ptr<const std::vector<AcidStep>> getPattern() const { return pattern.load(); }
    bool exportMidi (const juce::File& target, int repeats = 4) const;
    juce::AudioProcessorValueTreeState params;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
private:
    std::atomic<std::shared_ptr<const std::vector<AcidStep>>> pattern;
    std::atomic<uint32_t> tb3Command { 0 };
    double sampleRateHz = 44100.0, currentPitch = 110, targetPitch = 110, phase = 0, subPhase = 0;
    float envelope = 0, filterLow = 0, filterBand = 0;
    std::vector<float> delayLeft, delayRight;
    size_t delayIndex = 0;
    int currentNote = -1, activePatternStep = -1, activeRatchet = -1;
    int64_t fallbackSamples = 0;
    float get (const char* name) const { return params.getRawParameterValue (name)->load(); }
    void startNote (int note, bool accent, bool slide);
    float processOscillator (double phase) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DFDAcidEngineProcessor)
};
