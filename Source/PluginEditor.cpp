#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessorEditor::AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(400, 300);

  bitcrushSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
  delaySlider.setRange(0.0, 2000.0, 1.0);
  delaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  addAndMakeVisible(&bitcrushSlider);

  delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
  delaySlider.setRange(0.0, 2000.0, 1.0);
  delaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  addAndMakeVisible(&delaySlider);

  feedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  feedbackSlider.setRange(0.0, 0.95, 0.01);
  feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  addAndMakeVisible(&feedbackSlider);

  mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  mixSlider.setRange(0.0, 1.0, 0.01);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  addAndMakeVisible(&mixSlider);

  delayLabel.setText("Delay Time (ms)", juce::dontSendNotification);
  delayLabel.attachToComponent(&delaySlider, true);
  addAndMakeVisible(&delayLabel);

  feedbackLabel.setText("Feedback", juce::dontSendNotification);
  feedbackLabel.attachToComponent(&feedbackSlider, true);
  addAndMakeVisible(&feedbackLabel);

  mixLabel.setText("Dry/Wet Mix", juce::dontSendNotification);
  mixLabel.attachToComponent(&mixSlider, true);
  addAndMakeVisible(&mixLabel);

  delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delay", delaySlider);
  feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "feedback", feedbackSlider);
  mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixSlider);
}

AudioDelayAudioProcessorEditor::~AudioDelayAudioProcessorEditor()
{
}

void AudioDelayAudioProcessorEditor::paint(juce::Graphics &g)
{
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioDelayAudioProcessorEditor::resized()
{
  auto area = getLocalBounds();

  delaySlider.setBounds(area.removeFromTop(100).reduced(50, 25));
  feedbackSlider.setBounds(area.removeFromTop(100).reduced(50, 25));
  mixSlider.setBounds(area.reduced(50, 25));
}