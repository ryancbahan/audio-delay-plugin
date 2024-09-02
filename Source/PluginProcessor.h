#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AudioDelayAudioProcessor : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener
{
public:
  AudioDelayAudioProcessor();
  ~AudioDelayAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

  // Add this function to implement the Listener interface
  void parameterChanged(const juce::String &parameterID, float newValue) override;

  void loadAudioFile(const juce::File &file);
  void startPlaying();
  void stopPlaying();

private:
  juce::AudioProcessorValueTreeState parameters;

  juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
  juce::dsp::DryWetMixer<float> dryWetMixer;
  juce::dsp::Panner<float> panner;
  juce::Reverb reverb;
  juce::dsp::DryWetMixer<float> reverbMixer;
  float applyLFOToPan(float basePan, float lfoAmount, float lfoValue);

  juce::dsp::StateVariableTPTFilter<float> bandPassFilter;

  void updateDelayLineParameters();
  float applyBitcrushing(float sample, float bitcrushAmount);
  void updateFilterParameters();

  juce::dsp::Oscillator<float> lfo;
  float lfoPhase = 0.0f;

  void updateLFOParameters();
  float applyLFO(float baseValue, float lfoAmount, float lfoValue, float minValue, float maxValue);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessor)
};