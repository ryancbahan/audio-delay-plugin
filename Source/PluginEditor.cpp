#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessorEditor::AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(700, 500);

  auto setupSwitch = [this](juce::ToggleButton &button, juce::Label &label, const juce::String &labelText)
  {
    addAndMakeVisible(button);
    button.setToggleState(false, juce::dontSendNotification);

    // Style the ToggleButton
    button.setColour(juce::ToggleButton::tickColourId, juce::Colours::red);
    button.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
    button.setColour(juce::ToggleButton::textColourId, juce::Colours::black);

    // Make the button larger and change its appearance
    button.setButtonText(labelText);
    button.changeWidthToFitText();
    button.setSize(button.getWidth() + 20, 30); // Add some padding

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(label);
  };

  setupSwitch(lfoBitcrushSwitch, lfoBitcrushLabel, "LFO Bitcrush");
  setupSwitch(lfoHighpassSwitch, lfoHighpassLabel, "LFO Highpass");
  setupSwitch(lfoLowpassSwitch, lfoLowpassLabel, "LFO Lowpass");
  setupSwitch(lfoPanSwitch, lfoPanLabel, "LFO Pan");

  lfoBitcrushAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      audioProcessor.getParameters(), "lfoBitcrush", lfoBitcrushSwitch);
  lfoHighpassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      audioProcessor.getParameters(), "lfoHighpass", lfoHighpassSwitch);
  lfoLowpassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      audioProcessor.getParameters(), "lfoLowpass", lfoLowpassSwitch);
  lfoPanAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      audioProcessor.getParameters(), "lfoPan", lfoPanSwitch);

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

  setupKnob(smearKnob, smearLabel, "Smear20", 0.0, 1.0, 0.01);

  setupKnob(delayKnob, delayLabel, "Delay", 0.0, 5000.0, 1.0);
  setupKnob(feedbackKnob, feedbackLabel, "Feedback", 0.0, 0.95, 0.01);
  setupKnob(mixKnob, mixLabel, "Dry/Wet", 0.0, 1.0, 0.01);
  setupKnob(bitcrushKnob, bitcrushLabel, "Bitcrush", 1.0, 16.0, 1.0);
  setupKnob(stereoWidthKnob, stereoWidthLabel, "Stereo Width", 0.0, 2.0, 0.01);
  setupKnob(panKnob, panLabel, "Pan", -1.0, 1.0, 0.01);
  setupKnob(highpassFreqKnob, highpassFreqLabel, "Highpass", 0.0, 1000.0, 1.0);
  setupKnob(lowpassFreqKnob, lowpassFreqLabel, "Lowpass", 2000.0, 20000.0, 1.0);
  setupKnob(lfoFreqKnob, lfoFreqLabel, "LFO Freq", 0.1, 20.0, 0.1);
  setupKnob(lfoAmountKnob, lfoAmountLabel, "LFO Amount", 0.0, 1.0, 0.01);

  smearAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "smear", smearKnob);
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
  tempoSyncBox.addItem("1/1T", 8);
  tempoSyncBox.addItem("1/2T", 9);
  tempoSyncBox.addItem("1/4T", 10);
  tempoSyncBox.addItem("1/8T", 11);
  tempoSyncBox.addItem("1/16T", 12);
  tempoSyncBox.addItem("1/32T", 13);
  tempoSyncBox.addItem("1/1D", 14);
  tempoSyncBox.addItem("1/2D", 15);
  tempoSyncBox.addItem("1/4D", 16);
  tempoSyncBox.addItem("1/8D", 17);
  tempoSyncBox.addItem("1/16D", 18);
  tempoSyncBox.addItem("1/32D", 19);
  addAndMakeVisible(tempoSyncBox);

  lfoTempoSyncBox.addItem("Free", 1);
  lfoTempoSyncBox.addItem("1/1", 2);
  lfoTempoSyncBox.addItem("1/2", 3);
  lfoTempoSyncBox.addItem("1/4", 4);
  lfoTempoSyncBox.addItem("1/8", 5);
  lfoTempoSyncBox.addItem("1/16", 6);
  lfoTempoSyncBox.addItem("1/32", 7);
  lfoTempoSyncBox.addItem("1/1T", 8);
  lfoTempoSyncBox.addItem("1/2T", 9);
  lfoTempoSyncBox.addItem("1/4T", 10);
  lfoTempoSyncBox.addItem("1/8T", 11);
  lfoTempoSyncBox.addItem("1/16T", 12);
  lfoTempoSyncBox.addItem("1/32T", 13);
  lfoTempoSyncBox.addItem("1/1D", 14);
  lfoTempoSyncBox.addItem("1/2D", 15);
  lfoTempoSyncBox.addItem("1/4D", 16);
  lfoTempoSyncBox.addItem("1/8D", 17);
  lfoTempoSyncBox.addItem("1/16D", 18);
  lfoTempoSyncBox.addItem("1/32D", 19);
  addAndMakeVisible(lfoTempoSyncBox);

  lfoTempoSyncLabel.setText("LFO Sync", juce::dontSendNotification);
  lfoTempoSyncLabel.setJustificationType(juce::Justification::centred);
  lfoTempoSyncLabel.setColour(juce::Label::textColourId, juce::Colours::black);
  addAndMakeVisible(lfoTempoSyncLabel);

  lfoTempoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.getParameters(), "lfoTempoSync", lfoTempoSyncBox);

  tempoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.getParameters(), "tempoSync", tempoSyncBox);

  updateDelayKnob();

  lfoBitcrushSwitch.toFront(false);
  lfoHighpassSwitch.toFront(false);
  lfoLowpassSwitch.toFront(false);
  lfoPanSwitch.toFront(false);
}

AudioDelayAudioProcessorEditor::~AudioDelayAudioProcessorEditor()
{
}

void AudioDelayAudioProcessorEditor::updateDelayKnob()
{
  auto *delayParam = audioProcessor.getParameters().getParameter("delay");
  if (delayParam != nullptr)
  {
    float delayMs = delayParam->convertFrom0to1(delayParam->getValue());
    delayKnob.setValue(delayMs, juce::dontSendNotification);
  }
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
  int height = area.getHeight() / 3;

  tempoSyncBox.setBounds(width * 0, height * 0, width, 20);
  delayKnob.setBounds(width * 0, height * 0 + 20, width, height - 20);

  // Add LFO Tempo Sync ComboBox layout
  lfoTempoSyncBox.setBounds(width * 3, height * 1, width, 20);
  lfoTempoSyncLabel.setBounds(width * 3, height * 1 + 20, width, 20);

  // Adjust LFO Freq and Amount knob positions
  lfoFreqKnob.setBounds(width * 3, height * 1 + 40, width, height - 60);
  lfoAmountKnob.setBounds(width * 4, height * 1, width, height - 20);

  auto layoutSwitch = [this, width, height](juce::ToggleButton &button, juce::Label &label, int col)
  {
    int x = width * col;
    int y = height * 2;
    button.setBounds(x, y, width, 20);
    label.setBounds(x, y + 20, width, 20);
  };

  layoutSwitch(lfoBitcrushSwitch, lfoBitcrushLabel, 0);
  layoutSwitch(lfoHighpassSwitch, lfoHighpassLabel, 1);
  layoutSwitch(lfoLowpassSwitch, lfoLowpassLabel, 2);
  layoutSwitch(lfoPanSwitch, lfoPanLabel, 3);

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
  layoutKnob(smearKnob, smearLabel, 2, 4);

  // Ensure the switches are visible by setting their bounds directly
  lfoBitcrushSwitch.setBounds(width * 0, height * 2, width, 20);
  lfoHighpassSwitch.setBounds(width * 1, height * 2, width, 20);
  lfoLowpassSwitch.setBounds(width * 2, height * 2, width, 20);
  lfoPanSwitch.setBounds(width * 3, height * 2, width, 20);
}

void AudioDelayAudioProcessorEditor::setAllLabelsBlack()
{
  juce::Label *labels[] = {
      &delayLabel, &feedbackLabel, &mixLabel, &bitcrushLabel, &stereoWidthLabel,
      &panLabel, &highpassFreqLabel, &lowpassFreqLabel, &lfoFreqLabel, &lfoAmountLabel,
      &smearLabel // Add this line
  };

  for (auto *label : labels)
  {
    label->setColour(juce::Label::textColourId, juce::Colours::black);
  }
}

void AudioDelayAudioProcessorEditor::setAllKnobsBlack()
{
  juce::Slider *knobs[] = {
      &delayKnob, &feedbackKnob, &mixKnob, &bitcrushKnob, &stereoWidthKnob,
      &panKnob, &highpassFreqKnob, &lowpassFreqKnob, &lfoFreqKnob, &lfoAmountKnob,
      &smearKnob // Add this line
  };

  for (auto *knob : knobs)
  {
    knob->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    knob->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);
    knob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    knob->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    knob->setColour(juce::Slider::thumbColourId, juce::Colours::black);
  }
}