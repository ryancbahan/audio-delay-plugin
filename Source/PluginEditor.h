#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class AudioDelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
  AudioDelayAudioProcessorEditor(AudioDelayAudioProcessor &);
  ~AudioDelayAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  AudioDelayAudioProcessor &audioProcessor;

  juce::Slider bitcrushSlider;
  juce::Slider delaySlider;
  juce::Slider feedbackSlider;
  juce::Slider mixSlider;

  juce::Label delayLabel;
  juce::Label feedbackLabel;
  juce::Label mixLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessorEditor)
};