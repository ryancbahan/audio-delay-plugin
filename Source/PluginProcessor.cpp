#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 2000.0f, 500.0f), std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f), std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f), std::make_unique<juce::AudioParameterFloat>("bitcrush", "Bitcrush", 1.0f, 16.0f, 16.0f), std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f), std::make_unique<juce::AudioParameterFloat>("highpassFreq", "Highpass Freq", 0.0f, 1000.0f, 0.0f), std::make_unique<juce::AudioParameterFloat>("lowpassFreq", "Lowpass Freq", 2000.0f, 20000.0f, 20000.0f), std::make_unique<juce::AudioParameterFloat>("lfoFreq", "LFO Frequency", 0.1f, 10.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("lfoAmount", "LFO Amount", 0.0f, 1.0f, 0.0f)})
{
    parameters.addParameterListener("highpassFreq", this);
    parameters.addParameterListener("lowpassFreq", this);
    lfo.initialise([](float x)
                   { return std::sin(x); }, 128);
}

AudioDelayAudioProcessor::~AudioDelayAudioProcessor()
{
    parameters.removeParameterListener("highpassFreq", this);
    parameters.removeParameterListener("lowpassFreq", this);
}

void AudioDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "highpassFreq" || parameterID == "lowpassFreq")
    {
        updateFilterParameters();
    }
}

const juce::String AudioDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioDelayAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AudioDelayAudioProcessor::producesMidi() const
{
    return false;
}

bool AudioDelayAudioProcessor::isMidiEffect() const
{
    return false;
}

double AudioDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioDelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioDelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String AudioDelayAudioProcessor::getProgramName(int index)
{
    return {};
}

void AudioDelayAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

void AudioDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(sampleRate * 2.0); // Max 2 seconds delay

    dryWetMixer.prepare(spec);
    dryWetMixer.reset();

    highpassFilter.prepare(spec);
    lowpassFilter.prepare(spec);

    updateFilterParameters();

    lfo.prepare(spec);
}

void AudioDelayAudioProcessor::releaseResources()
{
}

bool AudioDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void AudioDelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float delayTime = *parameters.getRawParameterValue("delay");
    float feedback = *parameters.getRawParameterValue("feedback");
    float mix = *parameters.getRawParameterValue("mix");
    float bitcrushAmount = *parameters.getRawParameterValue("bitcrush");
    float stereoWidth = *parameters.getRawParameterValue("stereoWidth");
    float pan = *parameters.getRawParameterValue("pan");
    float lfoFreq = *parameters.getRawParameterValue("lfoFreq");
    float lfoAmount = *parameters.getRawParameterValue("lfoAmount");

    delayLine.setDelay(delayTime / 1000.0f * getSampleRate());
    updateFilterParameters();

    lfo.setFrequency(lfoFreq);

    juce::AudioBuffer<float> dryBuffer(buffer);
    juce::AudioBuffer<float> wetBuffer(totalNumInputChannels, buffer.getNumSamples());

    // Process delay and apply effects to wet signal only
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *inputData = buffer.getReadPointer(channel);
        auto *wetData = wetBuffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float lfoValue = lfo.processSample(0.0f);
            float modifiedBitcrush = applyLFO(bitcrushAmount, lfoAmount, lfoValue, 1.0f, 16.0f);

            float inputSample = inputData[sample];
            float delaySample = delayLine.popSample(channel);

            // Apply bitcrushing to the delayed signal
            if (modifiedBitcrush < 16.0f)
            {
                delaySample = applyBitcrushing(delaySample, modifiedBitcrush);
            }

            delayLine.pushSample(channel, inputSample + (delaySample * feedback));
            wetData[sample] = delaySample;
        }
    }

    // Apply filters to the wet signal only
    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
    highpassFilter.process(wetContext);
    lowpassFilter.process(wetContext);

    // Apply stereo width to wet signal only
    if (totalNumInputChannels == 2 && stereoWidth != 1.0f)
    {
        float *left = wetBuffer.getWritePointer(0);
        float *right = wetBuffer.getWritePointer(1);

        for (int sample = 0; sample < wetBuffer.getNumSamples(); ++sample)
        {
            float mid = (left[sample] + right[sample]) * 0.5f;
            float side = (right[sample] - left[sample]) * 0.5f;

            left[sample] = mid - side * stereoWidth;
            right[sample] = mid + side * stereoWidth;
        }
    }

    // Apply panning to wet signal only
    float modifiedPan = applyLFOToPan(pan, lfoAmount, lfo.processSample(0.0f));
    juce::dsp::AudioBlock<float> wetPanBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> wetPanContext(wetPanBlock);
    panner.setPan(modifiedPan);
    panner.process(wetPanContext);

    // Mix dry and wet signals
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *wetData = wetBuffer.getReadPointer(channel);
        auto *outputData = buffer.getWritePointer(channel);
        auto *dryData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            outputData[sample] = dryData[sample] * (1 - mix) + wetData[sample] * mix;
        }
    }

    if (buffer.getNumSamples() > 0)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            DBG("Channel " << channel << " first sample: " << buffer.getSample(channel, 0));
        }
    }
}

void AudioDelayAudioProcessor::updateFilterParameters()
{
    auto sampleRate = getSampleRate();
    float highpassFreq = *parameters.getRawParameterValue("highpassFreq");
    float lowpassFreq = *parameters.getRawParameterValue("lowpassFreq");

    *highpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highpassFreq);
    *lowpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowpassFreq);
}

float AudioDelayAudioProcessor::applyBitcrushing(float sample, float bitcrushAmount)
{
    int bits = static_cast<int>(bitcrushAmount);
    float maxValue = std::pow(2, bits) - 1;
    return std::round(sample * maxValue) / maxValue;
}

float AudioDelayAudioProcessor::applyLFO(float baseValue, float lfoAmount, float lfoValue, float minValue, float maxValue)
{
    float range = maxValue - minValue;
    float modulation = lfoValue * lfoAmount * range * 0.5f;
    return juce::jlimit(minValue, maxValue, baseValue + modulation);
}

float AudioDelayAudioProcessor::applyLFOToPan(float basePan, float lfoAmount, float lfoValue)
{
    float modulation = lfoValue * lfoAmount;
    float modifiedPan = basePan + modulation;
    return juce::jlimit(-1.0f, 1.0f, modifiedPan);
}

bool AudioDelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *AudioDelayAudioProcessor::createEditor()
{
    return new AudioDelayAudioProcessorEditor(*this);
}

void AudioDelayAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioDelayAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioDelayAudioProcessor();
}