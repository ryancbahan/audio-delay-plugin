#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "LFOManager.h"
#include "DelayManager.h"

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
  LFOManager lfoManager;
  DelayManager delayManager;
  juce::dsp::DryWetMixer<float> dryWetMixer;
  juce::dsp::Panner<float> panner;

  juce::dsp::Convolution convolution;
  juce::AudioBuffer<float> impulseResponse;
  juce::dsp::WaveShaper<float> waveShaper;

  void createImpulseResponse();
  float customWaveshaper(float sample);

  std::atomic<float> *waveshapeAmountParameter = nullptr;

  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highpassFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowpassFilter;

  static const int NUM_DIFFUSION_FILTERS = 4;
  std::array<juce::dsp::StateVariableTPTFilter<float>, NUM_DIFFUSION_FILTERS> diffusionFilters;
  juce::dsp::StateVariableTPTFilter<float> preDiffusionLowpass;
  juce::dsp::StateVariableTPTFilter<float> postDiffusionLowpass;

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
  std::atomic<float> *lfoBitcrushParameter = nullptr;
  std::atomic<float> *lfoHighpassParameter = nullptr;
  std::atomic<float> *lfoLowpassParameter = nullptr;
  std::atomic<float> *lfoPanParameter = nullptr;
  std::atomic<float> *lfoTempoSyncParameter = nullptr;
  std::atomic<float> *smearParameter = nullptr;

  double lastKnownBPM;
  float chorusRate;
  float chorusDepth;
  float chorusPhase;
  float chorusPhaseIncrement;

  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  void updateLFOFrequency();
  void updateBPMIfChanged();
  void processDelayAndEffects(int channel, int sample, const float *inputData, float *wetData, float feedback, float bitcrushAmount, float smearAmount, float lfoAmount, float waveshapeAmount);
  float applyLFOToBitcrush(float bitcrushAmount, float lfoAmount, float smoothedLFO);
  void applyLFOToFilters(float smoothedLFO, float lfoAmount);
  float processDelaySample(int channel, float delayInSamples, float smearAmount, float lfoModulation);
  void updateChorusPhase();
  void applyFiltersToWetSignal(juce::AudioBuffer<float> &wetBuffer);
  void applyStereoWidth(juce::AudioBuffer<float> &wetBuffer, float stereoWidth);
  void applyPanning(juce::AudioBuffer<float> &wetBuffer, float pan, float lfoAmount);
  void mixDryWetSignals(juce::AudioBuffer<float> &buffer, const juce::AudioBuffer<float> &dryBuffer, const juce::AudioBuffer<float> &wetBuffer, float mix);
  void applyFinalDCBlocking(juce::AudioBuffer<float> &buffer);
  void updateFilterParameters();
  void updateDiffusionFilters();
  float processDiffusionFilters(float input);
  float applyBitcrushing(float sample, float bitcrushAmount, float waveshapeAmount);
  float applyLFO(float baseValue, float lfoAmount, float lfoValue, float minValue, float maxValue);
  float applyLFOToPan(float basePan, float lfoAmount, float lfoValue);
  void updateDelayTimeFromSync();
  std::atomic<float> *lfoDelayParameter = nullptr;
  float applyLFOToDelay(float delayInSamples, float lfoAmount, float smoothedLFO);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDelayAudioProcessor)
};