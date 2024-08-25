#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 2000.0f, 500.0f), std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f), std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f), std::make_unique<juce::AudioParameterFloat>("bitcrush", "Bitcrush", 1.0f, 16.0f, 16.0f), std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f)})
{
}

AudioDelayAudioProcessor::~AudioDelayAudioProcessor()
{
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

    // Update parameters
    float delayTime = *parameters.getRawParameterValue("delay");
    float feedback = *parameters.getRawParameterValue("feedback");
    float mix = *parameters.getRawParameterValue("mix");
    float bitcrushAmount = *parameters.getRawParameterValue("bitcrush");
    float stereoWidth = *parameters.getRawParameterValue("stereoWidth");
    float pan = *parameters.getRawParameterValue("pan");

    delayLine.setDelay(delayTime / 1000.0f * getSampleRate());
    dryWetMixer.setWetMixProportion(mix);
    panner.setPan(pan);

    // Create an audio block
    juce::dsp::AudioBlock<float> block(buffer);

    // Save the dry signal
    dryWetMixer.pushDrySamples(block);

    // Process delay and feedback
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            float delaySample = delayLine.popSample(channel);

            float processedSample = delaySample;

            // Apply feedback
            delayLine.pushSample(channel, inputSample + (delaySample * feedback));

            // Apply bitcrushing if needed
            if (bitcrushAmount < 16.0f)
            {
                processedSample = applyBitcrushing(processedSample, bitcrushAmount);
            }

            channelData[sample] = processedSample;
        }
    }

    // Mix dry and wet signals
    dryWetMixer.mixWetSamples(block);

    // Apply stereo width
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

    // Apply panning
    juce::dsp::ProcessContextReplacing<float> context(block);
    panner.process(context);
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