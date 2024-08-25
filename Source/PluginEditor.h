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

  juce::Slider delayKnob;
  juce::Slider feedbackKnob;
  juce::Slider mixKnob;
  juce::Slider bitcrushKnob;
  juce::Slider stereoWidthKnob;
  juce::Slider panKnob;
  juce::Slider reverbKnob;

  juce::Label delayLabel;
  juce::Label feedbackLabel;
  juce::Label mixLabel;
  juce::Label bitcrushLabel;
  juce::Label stereoWidthLabel;
  juce::Label panLabel;
  juce::Label reverbLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitcrushAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessorEditor)
};