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
  void setAllLabelsBlack();
  void setAllKnobsBlack();

private:
  void updateDelayKnob();

  AudioDelayAudioProcessor &audioProcessor;

  juce::ToggleButton lfoBitcrushSwitch;
  juce::ToggleButton lfoHighpassSwitch;
  juce::ToggleButton lfoLowpassSwitch;
  juce::ToggleButton lfoPanSwitch;

  juce::Slider delayKnob;
  juce::Slider feedbackKnob;
  juce::Slider mixKnob;
  juce::Slider bitcrushKnob;
  juce::Slider stereoWidthKnob;
  juce::Slider panKnob;
  juce::Slider lfoFreqKnob;
  juce::Slider lfoAmountKnob;
  juce::Slider highpassFreqKnob;
  juce::Slider lowpassFreqKnob;

  juce::ComboBox tempoSyncBox;
  juce::ComboBox lfoTempoSyncBox;
  juce::Label lfoTempoSyncLabel;

  juce::Label highpassFreqLabel;
  juce::Label lowpassFreqLabel;
  juce::Label delayLabel;
  juce::Label feedbackLabel;
  juce::Label mixLabel;
  juce::Label bitcrushLabel;
  juce::Label stereoWidthLabel;
  juce::Label panLabel;
  juce::Label filterFreqLabel;
  juce::Label filterQLabel;
  juce::Label lfoFreqLabel;
  juce::Label lfoAmountLabel;
  juce::Label lfoBitcrushLabel;
  juce::Label lfoHighpassLabel;
  juce::Label lfoLowpassLabel;
  juce::Label lfoPanLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoBitcrushAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoHighpassAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoLowpassAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoPanAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoTempoSyncAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highpassFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowpassFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitcrushAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoFreqAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoAmountAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tempoSyncAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessorEditor)
};