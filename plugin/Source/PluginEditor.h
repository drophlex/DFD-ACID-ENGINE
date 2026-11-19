#pragma once
#include "PluginProcessor.h"

// The editor is the same page as the web version, in the system web view, talking to the native engine.
class DfdEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit DfdEditor(DfdProcessor&);
  ~DfdEditor() override;
  void resized() override;
  void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xff111216)); }
private:
  void timerCallback() override;
  juce::WebBrowserComponent::Options makeOptions();
  std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
  DfdProcessor& proc;
  std::unique_ptr<juce::FileChooser> chooser;
  std::unique_ptr<juce::WebBrowserComponent> web;
  std::array<float, 8> tele {}; std::array<float, 256> scope {}; std::vector<float> params;
  int frame = 0;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DfdEditor)
};
