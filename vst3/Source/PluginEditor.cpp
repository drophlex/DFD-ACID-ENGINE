#include "PluginEditor.h"
#include <algorithm>

namespace
{
    const juce::Colour bg { 0xff111317 }, tile { 0xff282b31 }, acid { 0xffc0ff3b }, pale { 0xfff2f3ee }, grey { 0xff8e939d };
    void configure (juce::TextButton& b, juce::Colour fill = tile)
    {
        b.setColour (juce::TextButton::buttonColourId, fill);
        b.setColour (juce::TextButton::buttonOnColourId, acid);
        b.setColour (juce::TextButton::textColourOffId, fill == acid ? bg : pale);
        b.setColour (juce::TextButton::textColourOnId, bg);
    }
}

DFDAcidEngineEditor::DFDAcidEngineEditor (DFDAcidEngineProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (1050, 780);
    const std::array<juce::String, 8> keys { "cutoff", "resonance", "drive", "decay", "env", "sub", "delay", "glide" };
    const std::array<juce::String, 8> names { "CUTOFF", "RESONANCE", "DRIVE", "DECAY", "ENV MOD", "SUB", "DELAY", "GLIDE" };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& knob = knobs[i];
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
        knob.setColour (juce::Slider::rotarySliderFillColourId, acid);
        knob.setColour (juce::Slider::rotarySliderOutlineColourId, tile);
        knob.setColour (juce::Slider::textBoxTextColourId, pale);
        knob.setColour (juce::Slider::textBoxOutlineColourId, bg);
        addAndMakeVisible (knob);
        attachments[i] = std::make_unique<Attachment> (processor.params, keys[i], knob);
        auto& label = labels[i];
        label.setText (names[i], juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, grey);
        addAndMakeVisible (label);
    }
    for (int i = 0; i < 16; ++i)
    {
        auto& b = stepButtons[(size_t) i];
        configure (b);
        addAndMakeVisible (b);
        b.onClick = [this, i]
        {
            auto pat = processor.getPattern();
            const size_t index = (size_t) page * 16 + (size_t) i;
            if (index >= pat->size()) return;
            auto s = (*pat)[index];
            auto modifiers = juce::ModifierKeys::getCurrentModifiersRealtime();
            if (modifiers.isShiftDown()) s.note = std::min (127, s.note + 1);
            else if (modifiers.isAltDown()) s.accent = !s.accent;
            else if (modifiers.isCtrlDown() || modifiers.isCommandDown()) s.slide = !s.slide;
            else s.active = !s.active;
            processor.setStep (index, s);
            redrawSteps();
        };
    }
    for (auto* b : { &generateButton, &mutateButton, &exportButton, &prevPage, &nextPage, &lengthButton, &hardwareButton })
    {
        configure (*b);
        addAndMakeVisible (*b);
    }
    configure (generateButton, acid);
    lengthEditor.setText ("16");
    lengthEditor.setColour (juce::TextEditor::backgroundColourId, tile);
    lengthEditor.setColour (juce::TextEditor::textColourId, pale);
    addAndMakeVisible (lengthEditor);
    addAndMakeVisible (status);
    status.setColour (juce::Label::textColourId, grey);
    status.setText ("16 STEPS // SHIFT: NOTE+1 / ALT: ACCENT / CTRL: SLIDE", juce::dontSendNotification);
    generateButton.onClick = [this] { processor.generate ((int) juce::Time::getMillisecondCounter(), 1); redrawSteps(); };
    mutateButton.onClick = [this] { processor.mutate (.62f); redrawSteps(); };
    prevPage.onClick = [this] { page = std::max (0, page - 1); redrawSteps(); };
    nextPage.onClick = [this]
    {
        auto pattern = processor.getPattern();
        page = std::min ((int) ((pattern->size() - 1) / 16), page + 1);
        redrawSteps();
    };
    lengthButton.onClick = [this]
    {
        // No fixed musical sequence-length ceiling. Actual limit is available memory and JUCE's int.
        const int n = lengthEditor.getText().getIntValue();
        if (n > 0) { processor.setPatternLength (n); page = 0; redrawSteps(); }
    };
    hardwareButton.onClick = [this]
    {
        auto* param = processor.params.getParameter ("hardware");
        if (param)
        {
            const bool value = processor.params.getRawParameterValue ("hardware")->load() > .5f;
            param->setValueNotifyingHost (value ? 0.f : 1.f);
            hardwareButton.setButtonText (value ? "TB-3 OUT: OFF" : "TB-3 OUT: ON");
        }
    };
    exportButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Export DFD MIDI", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DFD_ACID.mid"), "*.mid");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            const auto f = fc.getResult();
            if (f != juce::File())
                status.setText (processor.exportMidi (f, 4) ? "MIDI EXPORTED: " + f.getFileName() : "MIDI EXPORT FAILED", juce::dontSendNotification);
        });
    };
    tb3Block.addItem ("PATCH", 1); tb3Block.addItem ("DISTORTION", 2);
    tb3Block.addItem ("FX1", 3); tb3Block.addItem ("FX2", 4);
    tb3Block.setSelectedId (3); addAndMakeVisible (tb3Block);
    tb3Param.setText ("1"); tb3Value.setText ("100");
    addAndMakeVisible (tb3Param); addAndMakeVisible (tb3Value);
    configure (applyTB3); addAndMakeVisible (applyTB3);
    applyTB3.onClick = [this]
    {
        const int block = tb3Block.getSelectedId() - 1;
        processor.queueTB3Parameter (block, tb3Param.getText().getIntValue(), tb3Value.getText().getIntValue());
        status.setText ("TB-3 SYS EX QUEUED / ROUTE VST3 MIDI OUT TO ROLAND TB-3", juce::dontSendNotification);
    };
    redrawSteps();
}

void DFDAcidEngineEditor::redrawSteps()
{
    auto pat = processor.getPattern(); if (!pat) return;
    for (size_t i = 0; i < stepButtons.size(); ++i)
    {
        const size_t index = (size_t) page * 16 + i;
        auto& b = stepButtons[i];
        const bool available = index < pat->size();
        b.setEnabled (available);
        if (available)
        {
            const auto& s = (*pat)[index];
            b.setButtonText (juce::String ((int) index + 1).paddedLeft ('0', 2) + "\n" + (s.active ? juce::MidiMessage::getMidiNoteName (s.note, true, true, 3) : "REST") + (s.accent ? " A" : "") + (s.slide ? " S" : ""));
            b.setColour (juce::TextButton::buttonColourId, s.active ? juce::Colour (0xff384b28) : tile);
            b.setColour (juce::TextButton::textColourOffId, s.active ? acid : grey);
        }
    }
    status.setText (juce::String ((int) pat->size()) + " STEPS / PAGE " + juce::String (page + 1) + " / SHIFT: NOTE · ALT: ACCENT · CTRL: SLIDE", juce::dontSendNotification);
}

void DFDAcidEngineEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (juce::Colour (0xff1e2026));
    g.fillRect (16, 18, getWidth() - 32, 180);
    g.setColour (acid);
    g.setFont (juce::Font (juce::FontOptions ("Helvetica Neue", 74.f, juce::Font::bold)));
    g.drawText ("ACID ENGINE", 40, 32, 730, 95, juce::Justification::centredLeft);
    g.setFont (juce::Font (juce::FontOptions ("Helvetica Neue", 13.f, juce::Font::bold)));
    g.drawText ("DARK FUNK DURCAL DESIGN // DEVELOPED AT POLARIS SOUND LAB / MÉRIDA, YUCATÁN", 45, 132, 720, 34, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xff978cff));
    g.drawEllipse (825, 22, 160, 160, 17.f);
    g.setColour (acid);
    g.fillEllipse (896, 88, 23, 23);
    g.setColour (grey);
    g.drawText ("01 / SOUND DESIGN", 31, 202, 210, 25, juce::Justification::centredLeft);
    g.drawText ("02 / SEQUENCER", 31, 408, 210, 25, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xff32353c));
    g.drawLine (31.f, 400.f, 1020.f, 400.f, 1.f);
    g.drawLine (31.f, 582.f, 1020.f, 582.f, 1.f);
    g.setColour (grey);
    g.drawText ("03 / TB-3 DEEP CONTROL // SYS EX (HOST MIDI SUPPORT REQUIRED)", 31, 676, 600, 22, juce::Justification::centredLeft);
    g.setFont (juce::Font (juce::FontOptions ("Helvetica Neue", 11.f, juce::Font::plain)));
    g.drawText ("BLOCK: PATCH 0, DISTORTION 1, FX1 2, FX2 3    /    PARAM: 0 = TYPE, 1..12 = DETAIL", 32, 750, 900, 22, juce::Justification::centredLeft);
}

void DFDAcidEngineEditor::resized()
{
    const int margin = 31;
    const int available = getWidth() - 2 * margin;
    const int cell = available / 8;
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int x = margin + (int) i * cell;
        labels[i].setBounds (x, 235, cell, 22);
        knobs[i].setBounds (x, 260, cell, 133);
    }
    const int w = (available - 15 * 5) / 16;
    for (size_t i = 0; i < stepButtons.size(); ++i)
        stepButtons[i].setBounds (margin + (int) i * (w + 5), 442, w, 96);
    generateButton.setBounds (margin, 597, 180, 54);
    mutateButton.setBounds (221, 597, 120, 54);
    exportButton.setBounds (353, 597, 135, 54);
    prevPage.setBounds (510, 597, 98, 54);
    nextPage.setBounds (617, 597, 98, 54);
    lengthEditor.setBounds (728, 602, 76, 44);
    lengthButton.setBounds (812, 597, 100, 54);
    hardwareButton.setBounds (922, 597, 105, 54);
    status.setBounds (margin, 539, available, 35);
    tb3Block.setBounds (32, 706, 165, 35);
    tb3Param.setBounds (208, 706, 85, 35);
    tb3Value.setBounds (304, 706, 95, 35);
    applyTB3.setBounds (411, 704, 266, 39);
}
