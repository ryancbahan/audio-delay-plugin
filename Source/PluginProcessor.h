#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>

class AudioDelayAudioProcessor : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener,
                                 public juce::Timer
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

  void parameterChanged(const juce::String &parameterID, float newValue) override;
  void timerCallback() override;

  juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

  void updateDelayTimeFromSync();

  enum TempoSync
  {
    Unsync,
    Whole,
    Half,
    Quarter,
    Eighth,
    Sixteenth,
    ThirtySecond,
    WholeTrip,
    HalfTrip,
    QuarterTrip,
    EighthTrip,
    SixteenthTrip,
    ThirtySecondTrip,
    WholeDot,
    HalfDot,
    QuarterDot,
    EighthDot,
    SixteenthDot,
    ThirtySecondDot
  };

private:
  juce::AudioProcessorValueTreeState parameters;
  juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
  juce::dsp::DryWetMixer<float> dryWetMixer;
  juce::dsp::Panner<float> panner;
  juce::dsp::Oscillator<float> lfo;

  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highpassFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowpassFilter;

  std::array<juce::dsp::IIR::Filter<float>, 2> dcBlocker;
  std::array<juce::dsp::IIR::Filter<float>, 2> finalDCBlocker;

  std::atomic<float> *delayParameter = nullptr;
  std::atomic<float> *feedbackParameter = nullptr;
  std::atomic<float> *mixParameter = nullptr;
  std::atomic<float> *bitcrushParameter = nullptr;
  std::atomic<float> *stereoWidthParameter = nullptr;
  std::atomic<float> *panParameter = nullptr;
  std::atomic<float> *highpassFreqParameter = nullptr;
  std::atomic<float> *lowpassFreqParameter = nullptr;
  std::atomic<float> *lfoFreqParameter = nullptr;
  std::atomic<float> *lfoAmountParameter = nullptr;
  std::atomic<float> *tempoSyncParameter = nullptr;

  double lastKnownBPM = 120.0;

  void updateFilterParameters();
  float applyBitcrushing(float sample, float bitcrushAmount);
  float applyLFO(float baseValue, float lfoAmount, float lfoValue, float minValue, float maxValue);
  float applyLFOToPan(float basePan, float lfoAmount, float lfoValue);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessor)
};