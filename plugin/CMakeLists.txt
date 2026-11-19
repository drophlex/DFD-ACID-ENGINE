cmake_minimum_required(VERSION 3.22)
project(DFDACIDENGINE VERSION 1.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
include(FetchContent)
if(DEFINED JUCE_DIR)
  add_subdirectory(${JUCE_DIR} juce)
else()
  FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(DfdAcid
  COMPANY_NAME "Polaris Sound Lab"
  COMPANY_WEBSITE "https://drophlex.github.io/DFD-ACID-ENGINE/"
  BUNDLE_ID com.polarissoundlab.dfdacidengine
  PLUGIN_MANUFACTURER_CODE Plsl
  PLUGIN_CODE DfAe
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "DFD ACID ENGINE"
  DESCRIPTION "acid and dirty electro synthesizer, sequencer and pattern lab"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  NEEDS_WEB_BROWSER TRUE
  NEEDS_WEBVIEW2 TRUE
  VST3_CATEGORIES Instrument Synth
  AU_MAIN_TYPE kAudioUnitType_MusicDevice
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(DfdAcidData SOURCES ui/index.html)

target_sources(DfdAcid PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(DfdAcid PRIVATE ../engine)
target_compile_definitions(DfdAcid PUBLIC
  JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1)
target_link_libraries(DfdAcid PRIVATE DfdAcidData juce::juce_audio_utils juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
