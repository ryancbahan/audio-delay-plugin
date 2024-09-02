#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 2000.0f, 500.0f), std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f), std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f), std::make_unique<juce::AudioParameterFloat>("bitcrush", "Bitcrush", 1.0f, 16.0f, 16.0f), std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f), std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.0f), std::make_unique<juce::AudioParameterFloat>("filterFreq", "Filter Frequency", 20.0f, 20000.0f, 1000.0f), std::make_unique<juce::AudioParameterFloat>("filterQ", "Filter Q", 0.1f, 10.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("lfoFreq", "LFO Frequency", 0.1f, 10.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("lfoAmount", "LFO Amount", 0.0f, 1.0f, 0.0f)})
{
    parameters.addParameterListener("filterFreq", this);
    parameters.addParameterListener("filterQ", this);
    lfo.initialise([](float x)
                   { return std::sin(x); }, 128);
}

AudioDelayAudioProcessor::~AudioDelayAudioProcessor()
{
    parameters.removeParameterListener("filterFreq", this);
    parameters.removeParameterListener("filterQ", this);
}

void AudioDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "filterFreq" || parameterID == "filterQ")
    {
        DBG(parameterID << " changed to " << newValue);
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

    reverbMixer.prepare(spec);
    reverbMixer.reset();

    reverb.reset();
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = 1.0f;
    reverb.setParameters(reverbParams);

    bandPassFilter.prepare(spec);
    bandPassFilter.reset();
    bandPassFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
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
    float reverbMix = *parameters.getRawParameterValue("reverbMix");
    float lfoFreq = *parameters.getRawParameterValue("lfoFreq");
    float lfoAmount = *parameters.getRawParameterValue("lfoAmount");

    delayLine.setDelay(delayTime / 1000.0f * getSampleRate());
    updateFilterParameters();

    lfo.setFrequency(lfoFreq);

    juce::AudioBuffer<float> dryBuffer(buffer);
    juce::AudioBuffer<float> delayBuffer(totalNumInputChannels, buffer.getNumSamples());

    float averageModifiedPan = 0.0f;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *inputData = buffer.getReadPointer(channel);
        auto *delayData = delayBuffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float lfoValue = lfo.processSample(0.0f);
            float modifiedBitcrush = applyLFO(bitcrushAmount, lfoAmount, lfoValue, 1.0f, 16.0f);
            float modifiedPan = applyLFOToPan(pan, lfoAmount, lfoValue);

            averageModifiedPan += modifiedPan;

            float inputSample = inputData[sample];
            float delaySample = delayLine.popSample(channel);
            float filteredDelaySample = bandPassFilter.processSample(channel, delaySample);

            if (sample < 5)
            {
                DBG("Channel " << channel << ", Sample " << sample << ": Before filter: " << delaySample << ", After filter: " << filteredDelaySample);
            }

            if (modifiedBitcrush < 16.0f)
            {
                filteredDelaySample = applyBitcrushing(filteredDelaySample, modifiedBitcrush);
            }

            delayLine.pushSample(channel, inputSample + (delaySample * feedback));
            delayData[sample] = filteredDelaySample;
        }
    }

    averageModifiedPan /= (totalNumInputChannels * buffer.getNumSamples());

    juce::AudioBuffer<float> reverbBuffer(delayBuffer);
    reverb.processStereo(reverbBuffer.getWritePointer(0), reverbBuffer.getWritePointer(1), reverbBuffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *delayData = delayBuffer.getReadPointer(channel);
        auto *reverbData = reverbBuffer.getReadPointer(channel);
        auto *outputData = buffer.getWritePointer(channel);
        auto *dryData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float wetSample = delayData[sample] * (1 - reverbMix) + reverbData[sample] * reverbMix;
            outputData[sample] = dryData[sample] * (1 - mix) + wetSample * mix;
        }
    }

    if (totalNumInputChannels == 2 && stereoWidth != 1.0f)
    {
        float *left = buffer.getWritePointer(0);
        float *right = buffer.getWritePointer(1);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float mid = (left[sample] + right[sample]) * 0.5f;
            float side = (right[sample] - left[sample]) * 0.5f;

            left[sample] = mid - side * stereoWidth;
            right[sample] = mid + side * stereoWidth;
        }
    }

    juce::dsp::AudioBlock<float> panBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> panContext(panBlock);
    panner.setPan(averageModifiedPan); // Use the average modified pan value
    panner.process(panContext);

    if (buffer.getNumSamples() > 0)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            DBG("Channel " << channel << " first sample: " << buffer.getSample(channel, 0));
        }
    }
}

void AudioDelayAudioProcessor::updateDelayLineParameters()
{
    float delayTime = *parameters.getRawParameterValue("delay");
    delayLine.setDelay(delayTime / 1000.0f * getSampleRate());

    float mix = *parameters.getRawParameterValue("mix");
    dryWetMixer.setWetMixProportion(mix);
}

float AudioDelayAudioProcessor::applyBitcrushing(float sample, float bitcrushAmount)
{
    int bits = static_cast<int>(bitcrushAmount);
    float maxValue = std::pow(2, bits) - 1;
    return std::round(sample * maxValue) / maxValue;
}

void AudioDelayAudioProcessor::updateFilterParameters()
{
    float filterFreq = *parameters.getRawParameterValue("filterFreq");
    float filterQ = *parameters.getRawParameterValue("filterQ");
    bandPassFilter.setCutoffFrequency(filterFreq);
    bandPassFilter.setResonance(filterQ);

    DBG("Filter Frequency: " << filterFreq << " Hz, Q: " << filterQ);
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