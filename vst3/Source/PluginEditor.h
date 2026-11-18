#pragma once
#include "PluginProcessor.h"
#include <array>

class DFDAcidEngineEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DFDAcidEngineEditor (DFDAcidEngineProcessor&);
    ~DFDAcidEngineEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    DFDAcidEngineProcessor& processor;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<juce::Slider, 8> knobs;
    std::array<juce::Label, 8> labels;
    std::array<std::unique_ptr<Attachment>, 8> attachments;
    std::array<juce::TextButton, 16> stepButtons;
    juce::TextButton generateButton { "GENERATE NEW DNA" }, mutateButton { "MUTATE" },
        exportButton { "EXPORT MIDI" }, prevPage { "< PAGE" }, nextPage { "PAGE >" },
        lengthButton { "SET STEPS" }, hardwareButton { "TB-3 OUT: OFF" };
    juce::TextEditor lengthEditor;
    juce::ComboBox tb3Block;
    juce::TextEditor tb3Param, tb3Value;
    juce::TextButton applyTB3 { "SEND ACTIVE PATCH SYS EX" };
    juce::Label status;
    int page = 0;
    std::unique_ptr<juce::FileChooser> chooser;
    void redrawSteps();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DFDAcidEngineEditor)
};
