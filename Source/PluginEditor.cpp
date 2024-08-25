#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessorEditor::AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(700, 400); // Increased width to accommodate the new knob

  auto setupKnob = [this](juce::Slider &knob, juce::Label &label, const juce::String &labelText,
                          double rangeStart, double rangeEnd, double interval)
  {
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    knob.setRange(rangeStart, rangeEnd, interval);
    addAndMakeVisible(knob);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
  };

  setupKnob(delayKnob, delayLabel, "Delay", 0.0, 2000.0, 1.0);
  setupKnob(feedbackKnob, feedbackLabel, "Feedback", 0.0, 0.95, 0.01);
  setupKnob(mixKnob, mixLabel, "Dry/Wet", 0.0, 1.0, 0.01);
  setupKnob(bitcrushKnob, bitcrushLabel, "Bitcrush", 1.0, 16.0, 1.0);
  setupKnob(stereoWidthKnob, stereoWidthLabel, "Stereo Width", 0.0, 2.0, 0.01);
  setupKnob(panKnob, panLabel, "Pan", -1.0, 1.0, 0.01);
  setupKnob(reverbKnob, reverbLabel, "Reverb", 0.0, 1.0, 0.01);

  delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "delay", delayKnob);
  feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "feedback", feedbackKnob);
  mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "mix", mixKnob);
  bitcrushAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "bitcrush", bitcrushKnob);
  stereoWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "stereoWidth", stereoWidthKnob);
  panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "pan", panKnob);
  reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "reverbMix", reverbKnob);
}

AudioDelayAudioProcessorEditor::~AudioDelayAudioProcessorEditor()
{
}

void AudioDelayAudioProcessorEditor::paint(juce::Graphics &g)
{
  g.fillAll(juce::Colours::lightblue);
  g.setColour(juce::Colours::black);
  g.setFont(15.0f);
  g.drawFittedText("Audio Delay Plugin", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioDelayAudioProcessorEditor::resized()
{
  auto area = getLocalBounds().reduced(20);
  int width = area.getWidth() / 4; // Changed from 3 to 4 to accommodate the new knob
  int height = area.getHeight() / 2;

  auto layoutKnob = [this, width, height](juce::Slider &knob, juce::Label &label, int row, int col)
  {
    int x = width * col;
    int y = height * row;
    knob.setBounds(x, y, width, height - 20);
    label.setBounds(x, y + height - 20, width, 20);
  };

  layoutKnob(delayKnob, delayLabel, 0, 0);
  layoutKnob(feedbackKnob, feedbackLabel, 0, 1);
  layoutKnob(mixKnob, mixLabel, 0, 2);
  layoutKnob(bitcrushKnob, bitcrushLabel, 0, 3);
  layoutKnob(stereoWidthKnob, stereoWidthLabel, 1, 0);
  layoutKnob(panKnob, panLabel, 1, 1);
  layoutKnob(reverbKnob, reverbLabel, 1, 2);
}