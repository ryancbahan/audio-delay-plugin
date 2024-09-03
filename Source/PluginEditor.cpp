#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessorEditor::AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(700, 400);

  auto setupKnob = [this](juce::Slider &knob, juce::Label &label, const juce::String &labelText,
                          double rangeStart, double rangeEnd, double interval)
  {
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    knob.setRange(rangeStart, rangeEnd, interval);
    knob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);
    addAndMakeVisible(knob);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(label);
  };

  setupKnob(delayKnob, delayLabel, "Delay", 0.0, 2000.0, 1.0);
  setupKnob(feedbackKnob, feedbackLabel, "Feedback", 0.0, 0.95, 0.01);
  setupKnob(mixKnob, mixLabel, "Dry/Wet", 0.0, 1.0, 0.01);
  setupKnob(bitcrushKnob, bitcrushLabel, "Bitcrush", 1.0, 16.0, 1.0);
  setupKnob(stereoWidthKnob, stereoWidthLabel, "Stereo Width", 0.0, 2.0, 0.01);
  setupKnob(panKnob, panLabel, "Pan", -1.0, 1.0, 0.01);
  setupKnob(highpassFreqKnob, highpassFreqLabel, "Highpass", 0.0, 1000.0, 1.0);
  setupKnob(lowpassFreqKnob, lowpassFreqLabel, "Lowpass", 2000.0, 20000.0, 1.0);
  setupKnob(lfoFreqKnob, lfoFreqLabel, "LFO Freq", 0.1, 10.0, 0.1);
  setupKnob(lfoAmountKnob, lfoAmountLabel, "LFO Amount", 0.0, 1.0, 0.01);

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
  highpassFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "highpassFreq", highpassFreqKnob);
  lowpassFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "lowpassFreq", lowpassFreqKnob);
  lfoFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "lfoFreq", lfoFreqKnob);
  lfoAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "lfoAmount", lfoAmountKnob);

  tempoSyncBox.addItem("Free", 1);
  tempoSyncBox.addItem("1/1", 2);
  tempoSyncBox.addItem("1/2", 3);
  tempoSyncBox.addItem("1/4", 4);
  tempoSyncBox.addItem("1/8", 5);
  tempoSyncBox.addItem("1/16", 6);
  tempoSyncBox.addItem("1/32", 7);
  addAndMakeVisible(tempoSyncBox);

  tempoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.getParameters(), "tempoSync", tempoSyncBox);
}

AudioDelayAudioProcessorEditor::~AudioDelayAudioProcessorEditor()
{
}

void AudioDelayAudioProcessorEditor::paint(juce::Graphics &g)
{
  g.fillAll(juce::Colours::white);
  setAllLabelsBlack();
  setAllKnobsBlack();
}

void AudioDelayAudioProcessorEditor::resized()
{
  auto area = getLocalBounds().reduced(20);
  int width = area.getWidth() / 5;
  int height = area.getHeight() / 2;

  tempoSyncBox.setBounds(width * 0, height * 0, width, 20);
  delayKnob.setBounds(width * 0, height * 0 + 20, width, height - 20);

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
  layoutKnob(stereoWidthKnob, stereoWidthLabel, 0, 4);
  layoutKnob(panKnob, panLabel, 1, 0);
  layoutKnob(highpassFreqKnob, highpassFreqLabel, 1, 1);
  layoutKnob(lowpassFreqKnob, lowpassFreqLabel, 1, 2);
  layoutKnob(lfoFreqKnob, lfoFreqLabel, 1, 3);
  layoutKnob(lfoAmountKnob, lfoAmountLabel, 1, 4);
}

void AudioDelayAudioProcessorEditor::setAllLabelsBlack()
{
  juce::Label *labels[] = {
      &delayLabel, &feedbackLabel, &mixLabel, &bitcrushLabel, &stereoWidthLabel,
      &panLabel, &highpassFreqLabel, &lowpassFreqLabel, &lfoFreqLabel, &lfoAmountLabel};

  for (auto *label : labels)
  {
    label->setColour(juce::Label::textColourId, juce::Colours::black);
  }
}

void AudioDelayAudioProcessorEditor::setAllKnobsBlack()
{
  juce::Slider *knobs[] = {
      &delayKnob, &feedbackKnob, &mixKnob, &bitcrushKnob, &stereoWidthKnob,
      &panKnob, &highpassFreqKnob, &lowpassFreqKnob, &lfoFreqKnob, &lfoAmountKnob};

  for (auto *knob : knobs)
  {
    knob->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    knob->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);
    knob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    knob->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    knob->setColour(juce::Slider::thumbColourId, juce::Colours::black);
  }
}