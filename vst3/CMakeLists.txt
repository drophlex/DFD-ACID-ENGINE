cmake_minimum_required(VERSION 3.22)
project(DFDAcidEngine VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.17-security GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(DFDAcidEngine COMPANY_NAME "Polaris Sound Lab" BUNDLE_ID "mx.polaris.dfdacidengine"
  PLUGIN_MANUFACTURER_CODE Plrs PLUGIN_CODE DfAe
  FORMATS VST3 Standalone IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT TRUE IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE COPY_PLUGIN_AFTER_BUILD TRUE
  PRODUCT_NAME "DFD ACID ENGINE")
target_sources(DFDAcidEngine PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(DFDAcidEngine PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(DFDAcidEngine PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp juce::juce_gui_extra PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
