#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 5000.0f, 500.0f), std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f), std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f), std::make_unique<juce::AudioParameterFloat>("bitcrush", "Bitcrush", 1.0f, 16.0f, 16.0f), std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f), std::make_unique<juce::AudioParameterFloat>("highpassFreq", "Highpass Freq", 20.0f, 5000.0f, 20.0f), std::make_unique<juce::AudioParameterFloat>("lowpassFreq", "Lowpass Freq", 200.0f, 20000.0f, 20000.0f), std::make_unique<juce::AudioParameterFloat>("lfoFreq", "LFO Frequency", 0.1f, 10.0f, 1.0f), std::make_unique<juce::AudioParameterFloat>("lfoAmount", "LFO Amount", 0.0f, 1.0f, 0.0f), std::make_unique<juce::AudioParameterChoice>("tempoSync", "Tempo Sync", juce::StringArray{"Free", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/1T", "1/2T", "1/4T", "1/8T", "1/16D", "1/32D", "1/1D", "1/2D", "1/4D", "1/8D", "1/16D", "1/32D"}, 0)}),
      lastKnownBPM(120.0),
      delayLine(44100 * 5),
      lfo([](float x)
          { return std::sin(x); })
{
    DBG("AudioDelayAudioProcessor constructor called");
    delayParameter = parameters.getRawParameterValue("delay");
    feedbackParameter = parameters.getRawParameterValue("feedback");
    mixParameter = parameters.getRawParameterValue("mix");
    bitcrushParameter = parameters.getRawParameterValue("bitcrush");
    stereoWidthParameter = parameters.getRawParameterValue("stereoWidth");
    panParameter = parameters.getRawParameterValue("pan");
    highpassFreqParameter = parameters.getRawParameterValue("highpassFreq");
    lowpassFreqParameter = parameters.getRawParameterValue("lowpassFreq");
    lfoFreqParameter = parameters.getRawParameterValue("lfoFreq");
    lfoAmountParameter = parameters.getRawParameterValue("lfoAmount");
    tempoSyncParameter = parameters.getRawParameterValue("tempoSync");

    parameters.addParameterListener("tempoSync", this);
    parameters.addParameterListener("delay", this);

    // Initialize the LFO
    lfo.initialise([](float x)
                   { return std::sin(x); });

    DBG("AudioDelayAudioProcessor constructor completed");
}

AudioDelayAudioProcessor::~AudioDelayAudioProcessor()
{
    parameters.removeParameterListener("tempoSync", this);
    parameters.removeParameterListener("delay", this);
    parameters.removeParameterListener("highpassFreq", this);
    parameters.removeParameterListener("lowpassFreq", this);
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
    DBG("prepareToPlay called with sampleRate: " << sampleRate << " and samplesPerBlock: " << samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    int maxDelaySamples = static_cast<int>(sampleRate * 5.0); // Max 5 seconds delay
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(maxDelaySamples);

    DBG("DelayLine prepared with maxDelaySamples: " << maxDelaySamples);
    DBG("Maximum delay in milliseconds: " << (maxDelaySamples * 1000.0 / sampleRate));

    dryWetMixer.prepare(spec);
    panner.prepare(spec);
    lfo.prepare(spec);

    highpassFilter.prepare(spec);
    lowpassFilter.prepare(spec);

    for (auto &filter : dcBlocker)
    {
        filter.prepare(spec);
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    }

    for (auto &filter : finalDCBlocker)
    {
        filter.prepare(spec);
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 5.0f);
    }

    // Update the delay parameter range
    if (auto *delayParam = dynamic_cast<juce::AudioParameterFloat *>(parameters.getParameter("delay")))
    {
        delayParam->range = juce::NormalisableRange<float>(0.0f, 5000.0f);
        DBG("Delay parameter range updated: 0.0f to 5000.0f");
    }

    updateDelayTimeFromSync();
    updateFilterParameters();

    DBG("prepareToPlay completed");
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
    static bool firstCall = true;
    if (firstCall)
    {
        DBG("First call to processBlock");
        firstCall = false;
    }

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Update BPM if changed
    auto playHead = getPlayHead();
    if (playHead != nullptr)
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                double currentBPM = *positionInfo->getBpm();
                if (std::abs(currentBPM - lastKnownBPM) > 0.01) // Check for significant change
                {
                    lastKnownBPM = currentBPM;
                    updateDelayTimeFromSync();
                }
            }
        }
    }

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float feedback = *feedbackParameter;
    float mix = *mixParameter;
    float bitcrushAmount = *bitcrushParameter;
    float stereoWidth = *stereoWidthParameter;
    float pan = *panParameter;
    float lfoFreq = *lfoFreqParameter;
    float lfoAmount = *lfoAmountParameter;
    float baseHighpassFreq = *highpassFreqParameter;
    float baseLowpassFreq = *lowpassFreqParameter;

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

            // Enhance filter modulation
            float highpassModDepth = juce::jmap(lfoAmount, 0.5f, 4.0f);
            float lowpassModDepth = juce::jmap(lfoAmount, 0.5f, 2.0f);

            float modifiedHighpassFreq = juce::jlimit(
                20.0f,
                5000.0f,
                baseHighpassFreq * std::pow(2.0f, highpassModDepth * (lfoValue * 2.0f - 1.0f)));

            float modifiedLowpassFreq = juce::jlimit(
                200.0f,
                20000.0f,
                baseLowpassFreq * std::pow(2.0f, lowpassModDepth * (lfoValue * 2.0f - 1.0f)));

            // Update filter coefficients for this sample
            *highpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), modifiedHighpassFreq);
            *lowpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), modifiedLowpassFreq);

            float inputSample = inputData[sample];
            float delaySample = delayLine.popSample(channel);

            // Apply bitcrushing to the delayed signal
            if (modifiedBitcrush < 16.0f)
            {
                delaySample = applyBitcrushing(delaySample, modifiedBitcrush);
            }

            // Apply DC blocking filter to the delayed sample
            delaySample = dcBlocker[static_cast<size_t>(channel)].processSample(delaySample);

            delayLine.pushSample(channel, inputSample + (delaySample * feedback));
            wetData[sample] = delaySample;
        }
    }

    // Apply filters to the wet signal
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

    // Final DC blocking on the output
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = finalDCBlocker[static_cast<size_t>(channel)].processSample(channelData[sample]);
        }
    }

    juce::ignoreUnused(midiMessages);
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

void AudioDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    DBG("Parameter changed: " << parameterID << " = " << newValue);

    if (parameterID == "tempoSync" || parameterID == "delay")
    {
        // Use a timer to debounce rapid parameter changes
        startTimerHz(30); // Will call timerCallback after ~33ms
    }
    else if (parameterID == "highpassFreq" || parameterID == "lowpassFreq")
    {
        updateFilterParameters();
    }
}

void AudioDelayAudioProcessor::timerCallback()
{
    stopTimer();
    updateDelayTimeFromSync();
}

void AudioDelayAudioProcessor::updateDelayTimeFromSync()
{
    static bool isUpdating = false;
    if (isUpdating)
        return;
    isUpdating = true;
    DBG("updateDelayTimeFromSync called");

    float delayTime = delayParameter->load();
    int syncMode = static_cast<int>(tempoSyncParameter->load());

    DBG("Initial delay time: " << delayTime << ", Sync mode: " << syncMode << ", BPM: " << lastKnownBPM);

    double beatsPerSecond = lastKnownBPM / 60.0;
    double quarterNoteTime = 1000.0 / beatsPerSecond; // in milliseconds

    if (syncMode != TempoSync::Unsync)
    {
        switch (syncMode)
        {
        case TempoSync::Whole:
            delayTime = static_cast<float>(quarterNoteTime * 4);
            break;
        case TempoSync::Half:
            delayTime = static_cast<float>(quarterNoteTime * 2);
            break;
        case TempoSync::Quarter:
            delayTime = static_cast<float>(quarterNoteTime);
            break;
        case TempoSync::Eighth:
            delayTime = static_cast<float>(quarterNoteTime / 2);
            break;
        case TempoSync::Sixteenth:
            delayTime = static_cast<float>(quarterNoteTime / 4);
            break;
        case TempoSync::ThirtySecond:
            delayTime = static_cast<float>(quarterNoteTime / 8);
            break;
        case TempoSync::WholeTrip:
            delayTime = static_cast<float>(quarterNoteTime * 4 * 2 / 3);
            break;
        case TempoSync::HalfTrip:
            delayTime = static_cast<float>(quarterNoteTime * 2 * 2 / 3);
            break;
        case TempoSync::QuarterTrip:
            delayTime = static_cast<float>(quarterNoteTime * 2 / 3);
            break;
        case TempoSync::EighthTrip:
            delayTime = static_cast<float>(quarterNoteTime / 2 * 2 / 3);
            break;
        case TempoSync::SixteenthTrip:
            delayTime = static_cast<float>(quarterNoteTime / 4 * 2 / 3);
            break;
        case TempoSync::ThirtySecondTrip:
            delayTime = static_cast<float>(quarterNoteTime / 8 * 2 / 3);
            break;
        case TempoSync::WholeDot:
            delayTime = static_cast<float>(quarterNoteTime * 4 * 1.5);
            break;
        case TempoSync::HalfDot:
            delayTime = static_cast<float>(quarterNoteTime * 2 * 1.5);
            break;
        case TempoSync::QuarterDot:
            delayTime = static_cast<float>(quarterNoteTime * 1.5);
            break;
        case TempoSync::EighthDot:
            delayTime = static_cast<float>(quarterNoteTime / 2 * 1.5);
            break;
        case TempoSync::SixteenthDot:
            delayTime = static_cast<float>(quarterNoteTime / 4 * 1.5);
            break;
        case TempoSync::ThirtySecondDot:
            delayTime = static_cast<float>(quarterNoteTime / 8 * 1.5);
            break;
        }
    }

    DBG("Calculated delay time: " << delayTime);

    // Ensure the delay time is within the valid range
    float maxDelayTime = 5000.0f; // 5 seconds maximum delay
    delayTime = juce::jlimit(0.0f, maxDelayTime, delayTime);

    float delayInSamples = delayTime / 1000.0f * getSampleRate();

    // Ensure the delay in samples doesn't exceed the maximum
    int maxDelaySamples = delayLine.getMaximumDelayInSamples();
    DBG("Max delay samples from DelayLine: " << maxDelaySamples);
    delayInSamples = juce::jlimit(1.0f, static_cast<float>(maxDelaySamples), delayInSamples);

    DBG("Delay in samples: " << delayInSamples << " (Max: " << maxDelaySamples << ")");

    delayLine.setDelay(delayInSamples);

    // Update the delay parameter to reflect the new delay time
    delayTime = (delayInSamples / getSampleRate()) * 1000.0f;
    delayParameter->store(delayTime);

    DBG("Final delay time: " << delayTime);

    // Update the AudioProcessorValueTreeState parameter
    if (auto *param = parameters.getParameter("delay"))
    {
        float normalizedValue = param->convertTo0to1(delayTime);
        DBG("Normalized value: " << normalizedValue);
        param->setValueNotifyingHost(normalizedValue);
    }

    DBG("updateDelayTimeFromSync completed");
    isUpdating = false;
}

void AudioDelayAudioProcessor::updateFilterParameters()
{
    auto sampleRate = getSampleRate();
    float highpassFreq = *highpassFreqParameter;
    float lowpassFreq = *lowpassFreqParameter;

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

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioDelayAudioProcessor();
}