#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo()).withOutput("Output", juce::AudioChannelSet::stereo())),
      parameters(*this, nullptr, juce::Identifier("AudioDelay"),
                 {std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 2000.0f, 500.0f),
                  std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f),
                  std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f)})
{
    delayTimeParameter = parameters.getRawParameterValue("delay");
    feedbackParameter = parameters.getRawParameterValue("feedback");
    mixParameter = parameters.getRawParameterValue("mix");
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
    const int numChannels = getTotalNumInputChannels();
    const int delayBufferSize = 2 * (sampleRate + samplesPerBlock);
    delayBuffer.setSize(numChannels, delayBufferSize);
    delayBuffer.clear();

    delayWritePosition = 0;
}

void AudioDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void AudioDelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = delayBuffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        const float *bufferData = buffer.getReadPointer(channel);
        const float *delayBufferData = delayBuffer.getReadPointer(channel);
        float *dryBuffer = buffer.getWritePointer(channel);

        fillDelayBuffer(channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        getFromDelayBuffer(buffer, channel, bufferLength, delayBufferLength, bufferData, delayBufferData);

        // Apply dry/wet mix
        float mix = *mixParameter;
        for (int sample = 0; sample < bufferLength; ++sample)
        {
            float dry = bufferData[sample];
            float wet = dryBuffer[sample];
            dryBuffer[sample] = dry * (1.0f - mix) + wet * mix;
        }
    }

    delayWritePosition += bufferLength;
    delayWritePosition %= delayBufferLength;
}

void AudioDelayAudioProcessor::fillDelayBuffer(int channel, const int bufferLength, const int delayBufferLength, const float *bufferData, const float *delayBufferData)
{
    if (delayBufferLength > bufferLength + delayWritePosition)
    {
        delayBuffer.copyFromWithRamp(channel, delayWritePosition, bufferData, bufferLength, 0.8f, 0.8f);
    }
    else
    {
        const int bufferRemaining = delayBufferLength - delayWritePosition;
        delayBuffer.copyFromWithRamp(channel, delayWritePosition, bufferData, bufferRemaining, 0.8f, 0.8f);
        delayBuffer.copyFromWithRamp(channel, 0, bufferData + bufferRemaining, bufferLength - bufferRemaining, 0.8f, 0.8f);
    }
}

void AudioDelayAudioProcessor::getFromDelayBuffer(juce::AudioBuffer<float> &buffer, int channel, const int bufferLength, const int delayBufferLength, const float *bufferData, const float *delayBufferData)
{
    const float delayTime = *delayTimeParameter;
    const float feedback = *feedbackParameter;

    int delayTimeInSamples = static_cast<int>(delayTime * getSampleRate() / 1000);
    int delayReadPosition = delayWritePosition - delayTimeInSamples;
    if (delayReadPosition < 0)
        delayReadPosition += delayBufferLength;

    if (delayReadPosition + bufferLength < delayBufferLength)
    {
        buffer.addFromWithRamp(channel, 0, delayBufferData + delayReadPosition, bufferLength, feedback, feedback);
    }
    else
    {
        const int bufferRemaining = delayBufferLength - delayReadPosition;
        buffer.addFromWithRamp(channel, 0, delayBufferData + delayReadPosition, bufferRemaining, feedback, feedback);
        buffer.addFromWithRamp(channel, bufferRemaining, delayBufferData, bufferLength - bufferRemaining, feedback, feedback);
    }
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