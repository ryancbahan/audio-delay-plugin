#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessorEditor::AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
  setSize(400, 300);

  auto setupSlider = [this](juce::Slider &slider, juce::Label &label, const juce::String &labelText,
                            double rangeStart, double rangeEnd, double interval)
  {
    addAndMakeVisible(slider);
    slider.setRange(rangeStart, rangeEnd, interval);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);

    addAndMakeVisible(label);
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::right);
  };

  setupSlider(delaySlider, delayLabel, "Delay (ms)", 0.0, 2000.0, 1.0);
  setupSlider(feedbackSlider, feedbackLabel, "Feedback", 0.0, 0.95, 0.01);
  setupSlider(mixSlider, mixLabel, "Dry/Wet", 0.0, 1.0, 0.01);
  setupSlider(bitcrushSlider, bitcrushLabel, "Bitcrush", 1.0, 16.0, 1.0);

  delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "delay", delaySlider);
  feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "feedback", feedbackSlider);
  mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "mix", mixSlider);
  bitcrushAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      audioProcessor.getParameters(), "bitcrush", bitcrushSlider);
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
  int sliderHeight = 20;
  int labelWidth = 100;
  int margin = 10;

  auto setupSliderWithLabel = [&](juce::Slider &slider, juce::Label &label)
  {
    auto sliderArea = area.removeFromTop(sliderHeight + margin);
    label.setBounds(sliderArea.removeFromLeft(labelWidth));
    slider.setBounds(sliderArea.reduced(margin, 0));
  };

  setupSliderWithLabel(delaySlider, delayLabel);
  setupSliderWithLabel(feedbackSlider, feedbackLabel);
  setupSliderWithLabel(mixSlider, mixLabel);
  setupSliderWithLabel(bitcrushSlider, bitcrushLabel);
}