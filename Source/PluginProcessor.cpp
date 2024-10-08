#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "LFOManager.h"
#include "DelayManager.h"

AudioDelayAudioProcessor::AudioDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout()),
      lastKnownBPM(120.0),
      lfoManager(),
      delayManager(),
      chorusRate(1.0f),
      chorusDepth(0.02f),
      chorusPhase(0.0f),
      chorusPhaseIncrement(0.0f)
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
    lfoBitcrushParameter = parameters.getRawParameterValue("lfoBitcrush");
    lfoHighpassParameter = parameters.getRawParameterValue("lfoHighpass");
    lfoLowpassParameter = parameters.getRawParameterValue("lfoLowpass");
    lfoPanParameter = parameters.getRawParameterValue("lfoPan");
    smearParameter = parameters.getRawParameterValue("smear");
    lfoTempoSyncParameter = parameters.getRawParameterValue("lfoTempoSync");
    lfoDelayParameter = parameters.getRawParameterValue("lfoDelay");
    waveshapeAmountParameter = parameters.getRawParameterValue("waveshapeAmount");

    parameters.addParameterListener("tempoSync", this);
    parameters.addParameterListener("delay", this);
    parameters.addParameterListener("lfoTempoSync", this);
    parameters.addParameterListener("lfoFreq", this);

    waveShaper.functionToUse = [](float x)
    {
        return juce::jlimit(float(-0.1), float(0.1), x); // [6]
    };

    DBG("AudioDelayAudioProcessor constructor completed");
}

AudioDelayAudioProcessor::~AudioDelayAudioProcessor()
{
    parameters.removeParameterListener("tempoSync", this);
    parameters.removeParameterListener("delay", this);
    parameters.removeParameterListener("highpassFreq", this);
    parameters.removeParameterListener("lowpassFreq", this);
    parameters.removeParameterListener("lfoTempoSync", this);
    parameters.removeParameterListener("lfoFreq", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioDelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("waveshapeAmount", "Waveshape Amount", 0.0f, 1.0f, 0.5f));

    // Delay
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delay", "Delay Time", 0.0f, 5000.0f, 500.0f));

    // Feedback
    params.push_back(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.95f, 0.5f));

    // Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet Mix", 0.0f, 1.0f, 0.5f));

    // Bitcrush
    params.push_back(std::make_unique<juce::AudioParameterFloat>("bitcrush", "Bitcrush", 1.0f, 16.0f, 16.0f));

    // Stereo Width
    params.push_back(std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 1.0f));

    // Pan
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f));

    // Highpass Frequency
    params.push_back(std::make_unique<juce::AudioParameterFloat>("highpassFreq", "Highpass Freq", 20.0f, 5000.0f, 20.0f));

    // Lowpass Frequency
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lowpassFreq", "Lowpass Freq", 200.0f, 20000.0f, 20000.0f));

    // LFO Frequency
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfoFreq", "LFO Frequency", 0.01f, 20.0f, 1.0f));

    // LFO Amount
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfoAmount", "LFO Amount", 0.0f, 1.0f, 0.0f));

    // Tempo Sync
    juce::StringArray tempoSyncOptions = {"Free", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/1T", "1/2T", "1/4T", "1/8T", "1/16T", "1/32T", "1/1D", "1/2D", "1/4D", "1/8D", "1/16D", "1/32D"};
    params.push_back(std::make_unique<juce::AudioParameterChoice>("tempoSync", "Tempo Sync", tempoSyncOptions, 0));

    // LFO Tempo Sync
    params.push_back(std::make_unique<juce::AudioParameterChoice>("lfoTempoSync", "LFO Tempo Sync", tempoSyncOptions, 0));

    // LFO Bitcrush
    params.push_back(std::make_unique<juce::AudioParameterBool>("lfoBitcrush", "LFO Bitcrush", false));

    // LFO Highpass
    params.push_back(std::make_unique<juce::AudioParameterBool>("lfoHighpass", "LFO Highpass", false));

    // LFO Lowpass
    params.push_back(std::make_unique<juce::AudioParameterBool>("lfoLowpass", "LFO Lowpass", false));

    // LFO Pan
    params.push_back(std::make_unique<juce::AudioParameterBool>("lfoPan", "LFO Pan", false));

    // Smear
    params.push_back(std::make_unique<juce::AudioParameterFloat>("smear", "Smear", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>("lfoDelay", "LFO Delay", false));

    return {params.begin(), params.end()};
}

void AudioDelayAudioProcessor::updateDelayTimeFromSync()
{
    int syncMode = static_cast<int>(tempoSyncParameter->load());
    float currentDelayTime = delayParameter->load();

    if (syncMode != 0) // Not in Free mode
    {
        float syncedDelayTime = delayManager.updateDelayTimeFromSync(static_cast<float>(lastKnownBPM), syncMode);

        // Only update the knob value if in sync mode and we got a valid delay time
        if (syncedDelayTime > 0.0f)
        {
            if (auto *param = parameters.getParameter("delay"))
            {
                float normalizedValue = param->convertTo0to1(syncedDelayTime);
                param->setValueNotifyingHost(normalizedValue);
            }
        }
    }

    // Always set the delay time from the knob value
    float delayInSamples = currentDelayTime / 1000.0f * getSampleRate();
    delayManager.setDelay(delayInSamples);
}

void AudioDelayAudioProcessor::updateDiffusionFilters()
{
    float smearAmount = smearParameter->load();
    float sampleRate = static_cast<float>(getSampleRate());

    // Adjust diffusion curve: start medium, then decrease as smear increases
    float diffusionCurve = 0.5f * (1.0f - std::pow(smearAmount, 0.5f));

    float delayTimes[4] = {0.0007f, 0.0011f, 0.0013f, 0.0017f}; // in seconds
    float maxFeedbackAmounts[4] = {0.3f, 0.35f, 0.4f, 0.45f};   // Maximum feedback amounts

    for (size_t i = 0; i < 4; ++i)
    {
        float delayTime = juce::jmax(0.00001f, delayTimes[i]);
        float feedback = maxFeedbackAmounts[i] * diffusionCurve;

        float frequency = juce::jlimit(20.0f, sampleRate * 0.49f, 1.0f / delayTime);
        float q = juce::jlimit(0.01f, 10.0f, 1.0f / (2.0f * (1.0f - feedback))); // Reduced maximum Q

        diffusionFilters[i].setCutoffFrequency(frequency);
        diffusionFilters[i].setResonance(q);
    }

    // Update pre and post diffusion lowpass filters
    float lowpassFreq = juce::jmap(smearAmount, 20000.0f, 10000.0f);
    preDiffusionLowpass.setCutoffFrequency(lowpassFreq);
    postDiffusionLowpass.setCutoffFrequency(lowpassFreq);

    // Update chorus parameters
    if (smearAmount > 0.0f)
    {
        // Slower increase in chorus rate
        chorusRate = 0.2f + smearAmount * 0.8f; // Chorus rate from 0.2 Hz to 1.0 Hz

        // More subtle increase in chorus depth
        chorusDepth = std::pow(smearAmount, 1.5f) * 0.005f; // Non-linear increase, up to 5 ms

        chorusPhaseIncrement = (chorusRate * juce::MathConstants<float>::twoPi) / sampleRate;

        // Update chorus lowpass filter
        float chorusCutoff = juce::jmap(smearAmount, 10000.0f, 15000.0f);
        *chorusLowpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, chorusCutoff);

        DBG("Chorus parameters updated - Rate: " << chorusRate << " Hz, Depth: " << chorusDepth << ", Cutoff: " << chorusCutoff << " Hz");
    }
    else
    {
        chorusRate = 0.0f;
        chorusDepth = 0.0f;
        chorusPhaseIncrement = 0.0f;
        DBG("Chorus disabled");
    }

    DBG("Diffusion filters updated - Smear Amount: " << smearAmount
                                                     << ", Diffusion Curve: " << diffusionCurve
                                                     << ", Pre/Post Lowpass Freq: " << lowpassFreq
                                                     << ", Diffusion Filter 0 Freq: " << diffusionFilters[0].getCutoffFrequency()
                                                     << ", Q: " << diffusionFilters[0].getResonance());
}

float AudioDelayAudioProcessor::processDiffusionFilters(float input, int channel)
{
    float smearAmount = smearParameter->load();

    if (smearAmount <= 0.0f)
    {
        return input;
    }

    float output = preDiffusionLowpass.processSample(channel, input);

    // Improved chorus effect
    float chorusModulation = chorusDepth * (std::sin(chorusPhase + (channel * juce::MathConstants<float>::pi * 0.5f)) * 0.5f + 0.5f);
    if (channel == 0)
    {
        chorusPhase += chorusPhaseIncrement;
        if (chorusPhase >= juce::MathConstants<float>::twoPi)
            chorusPhase -= juce::MathConstants<float>::twoPi;
    }

    // Use smoother interpolation for chorus
    float delayInSamples = chorusModulation * getSampleRate();
    float chorusOutput = chorusDelayLine.popSample(channel, delayInSamples, true); // Use internal interpolation

    chorusDelayLine.pushSample(channel, input);

    // Apply lowpass filter to chorus output
    chorusOutput = chorusLowpass.processSample(chorusOutput);

    // Diffusion processing with smoother parameter changes
    for (size_t i = 0; i < diffusionFilters.size(); ++i)
    {
        float modulatedFrequency = diffusionFilters[i].getCutoffFrequency() * (1.0f + chorusModulation * 0.1f);
        diffusionFilters[i].setCutoffFrequency(modulatedFrequency);
        output = diffusionFilters[i].processSample(channel, output);
    }

    output = postDiffusionLowpass.processSample(channel, output);

    // Smooth mixing of dry, chorus, and diffused signals
    float wetAmount = smearAmount;
    float dryAmount = 1.0f - wetAmount;

    float mixedOutput = input * dryAmount + (chorusOutput * 0.6f + output * 0.4f) * wetAmount;

    return mixedOutput;
}

void AudioDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    chorusDelayLine.prepare(spec);
    chorusDelayLine.setMaximumDelayInSamples(getSampleRate() * 0.05f + 3); // 50 ms + 3 samples for cubic interpolation

    chorusLowpass.prepare(spec);
    chorusLowpass.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0f);

    convolution.prepare(spec);
    waveShaper.prepare(spec);

    delayManager.prepare(spec);
    dryWetMixer.prepare(spec);
    panner.prepare(spec);

    highpassFilter.prepare(spec);
    lowpassFilter.prepare(spec);

    lfoManager.prepare(spec);
    lfoManager.setFrequency(lfoFreqParameter->load());

    for (auto &filter : diffusionFilters)
    {
        filter.prepare(spec);
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setResonance(0.7f);
        filter.setCutoffFrequency(1000.0f); // Set a default cutoff frequency
    }
    preDiffusionLowpass.prepare(spec);
    preDiffusionLowpass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    preDiffusionLowpass.setCutoffFrequency(10000.0f);

    postDiffusionLowpass.prepare(spec);
    postDiffusionLowpass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    postDiffusionLowpass.setCutoffFrequency(10000.0f);

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

    chorusPhaseIncrement = (chorusRate * juce::MathConstants<float>::twoPi) / static_cast<float>(sampleRate);

    updateLFOFrequency();
    updateDiffusionFilters();
    updateDelayTimeFromSync();
    updateFilterParameters();

    // Initialize the delay time after the delayManager has been prepared
    float delayTime = delayParameter->load();
    float delayInSamples = delayTime / 1000.0f * sampleRate;
    delayManager.setDelay(delayInSamples);
}

void AudioDelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    DBG("-------- processBlock start --------");
    DBG("Buffer size: " << buffer.getNumSamples() << ", Input channels: " << totalNumInputChannels << ", Output channels: " << totalNumOutputChannels);

    float currentLFOFreq = lfoFreqParameter->load();
    DBG("Current LFO Frequency: " << currentLFOFreq << " Hz");

    float bitcrushAmount = bitcrushParameter->load();
    float waveshapeAmount = waveshapeAmountParameter->load();

    lfoManager.setFrequency(currentLFOFreq);
    lfoManager.generateBlock(buffer.getNumSamples());

    DBG("LFO block generated. Checking samples:");
    DBG("First LFO sample: " << lfoManager.getSample(0));
    DBG("Middle LFO sample: " << lfoManager.getSample(buffer.getNumSamples() / 2));
    DBG("Last LFO sample: " << lfoManager.getSample(buffer.getNumSamples() - 1));

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    DBG("Updating BPM if changed");
    updateBPMIfChanged();

    // Prepare dry and wet buffers
    juce::AudioBuffer<float> dryBuffer(buffer);
    juce::AudioBuffer<float> wetBuffer(totalNumInputChannels, buffer.getNumSamples());

    // Get current parameter values
    float feedback = feedbackParameter->load();
    float mix = mixParameter->load();
    float stereoWidth = stereoWidthParameter->load();
    float pan = panParameter->load();
    float lfoAmount = lfoAmountParameter->load();
    float smearAmount = smearParameter->load();

    DBG("Current parameters - Feedback: " << feedback << ", Mix: " << mix << ", Bitcrush: " << bitcrushAmount
                                          << ", Stereo Width: " << stereoWidth << ", Pan: " << pan << ", LFO Amount: " << lfoAmount << ", Smear: " << smearAmount);

    DBG("Updating diffusion filters");
    updateDiffusionFilters();

    DBG("Processing delay and effects");
    // Process delay and apply effects
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *inputData = buffer.getReadPointer(channel);
        auto *wetData = wetBuffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            processDelayAndEffects(channel, sample, inputData, wetData, feedback, bitcrushAmount, smearAmount, lfoAmount, waveshapeAmount);
        }
    }

    DBG("Applying filters to wet signal");
    applyFiltersToWetSignal(wetBuffer);

    DBG("Applying stereo width");
    applyStereoWidth(wetBuffer, stereoWidth);

    DBG("Applying panning");
    applyPanning(wetBuffer, pan, lfoAmount);

    DBG("Mixing dry and wet signals");
    mixDryWetSignals(buffer, dryBuffer, wetBuffer, mix);

    DBG("Applying final DC blocking");
    applyFinalDCBlocking(buffer);

    juce::ignoreUnused(midiMessages);

    DBG("-------- processBlock end --------");
}

void AudioDelayAudioProcessor::applyPanning(juce::AudioBuffer<float> &buffer, float pan, float lfoAmount)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float lfoValue = lfoManager.getSample(sample);
        float modifiedPan = pan;

        if (lfoPanParameter->load() > 0.5f)
        {
            modifiedPan = applyLFOToPan(pan, lfoAmount, lfoValue);
        }

        // Convert pan [-1, 1] to gain [0, 1] for each channel
        float leftGain = 0.5f * (1.0f - modifiedPan);
        float rightGain = 0.5f * (1.0f + modifiedPan);

        buffer.setSample(0, sample, buffer.getSample(0, sample) * leftGain);
        buffer.setSample(1, sample, buffer.getSample(1, sample) * rightGain);
    }
}

void AudioDelayAudioProcessor::applyStereoWidth(juce::AudioBuffer<float> &buffer, float width)
{
    if (buffer.getNumChannels() < 2)
        return;

    float *left = buffer.getWritePointer(0);
    float *right = buffer.getWritePointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float mid = (left[sample] + right[sample]) * 0.5f;
        float side = (right[sample] - left[sample]) * 0.5f;

        left[sample] = mid - side * width;
        right[sample] = mid + side * width;
    }
}

void AudioDelayAudioProcessor::mixDryWetSignals(juce::AudioBuffer<float> &buffer, const juce::AudioBuffer<float> &dryBuffer, const juce::AudioBuffer<float> &wetBuffer, float mix)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto *outputData = buffer.getWritePointer(channel);
        auto *dryData = dryBuffer.getReadPointer(channel);
        auto *wetData = wetBuffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            outputData[sample] = dryData[sample] * (1 - mix) + wetData[sample] * mix;
        }
    }
}

void AudioDelayAudioProcessor::applyLFOToFilters(float smoothedLFO, float lfoAmount)
{
    float baseHighpassFreq = highpassFreqParameter->load();
    float baseLowpassFreq = lowpassFreqParameter->load();
    float modifiedHighpassFreq = baseHighpassFreq;
    float modifiedLowpassFreq = baseLowpassFreq;

    if (lfoHighpassParameter->load() > 0.5f)
    {
        // Increase the modulation range for highpass
        float highpassModDepth = juce::jmap(lfoAmount, 0.5f, 6.0f);
        modifiedHighpassFreq = juce::jlimit(
            20.0f,
            5000.0f,
            baseHighpassFreq * std::pow(2.0f, highpassModDepth * (smoothedLFO * 2.0f - 1.0f)));
    }

    if (lfoLowpassParameter->load() > 0.5f)
    {
        // Increase the modulation range for lowpass
        float lowpassModDepth = juce::jmap(lfoAmount, 0.5f, 4.0f);
        modifiedLowpassFreq = juce::jlimit(
            200.0f,
            20000.0f,
            baseLowpassFreq * std::pow(2.0f, lowpassModDepth * (smoothedLFO * 2.0f - 1.0f)));
    }

    *highpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), modifiedHighpassFreq);
    *lowpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), modifiedLowpassFreq);
}

void AudioDelayAudioProcessor::updateChorusPhase()
{
    chorusPhase += chorusPhaseIncrement;
    if (chorusPhase >= juce::MathConstants<float>::twoPi)
        chorusPhase -= juce::MathConstants<float>::twoPi;
}

float AudioDelayAudioProcessor::applyLFOToBitcrush(float bitcrushAmount, float lfoAmount, float smoothedLFO)
{
    if (lfoBitcrushParameter->load() > 0.5f)
    {
        return applyLFO(bitcrushAmount, lfoAmount, smoothedLFO, 1.0f, 16.0f);
    }
    return bitcrushAmount;
}

float AudioDelayAudioProcessor::processDelaySample(int channel, float delayInSamples, float smearAmount, float lfoModulation)
{
    // Apply LFO modulation to delay time
    float lfoModulatedDelay = delayInSamples * (1.0f + lfoModulation);

    // Apply additional chorusing based on smear amount
    float chorusModulation = chorusDepth * std::sin(chorusPhase) * smearAmount;
    float totalModulatedDelay = lfoModulatedDelay * (1.0f + chorusModulation);

    // Get the delayed sample
    float delaySample = delayManager.popSample(channel, totalModulatedDelay);

    // Apply diffusion if smear is active
    if (smearAmount > 0.0f)
    {
        float diffusedSample = processDiffusionFilters(delaySample, channel);
        delaySample = juce::jmap(smearAmount, delaySample, diffusedSample);
    }

    return delaySample;
}

void AudioDelayAudioProcessor::applyFinalDCBlocking(juce::AudioBuffer<float> &buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto *channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = finalDCBlocker[static_cast<size_t>(channel)].processSample(channelData[sample]);
        }
    }
}

void AudioDelayAudioProcessor::applyFiltersToWetSignal(juce::AudioBuffer<float> &wetBuffer)
{
    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
    highpassFilter.process(wetContext);
    lowpassFilter.process(wetContext);
}

void AudioDelayAudioProcessor::processDelayAndEffects(int channel, int sample, const float *inputData, float *wetData, float feedback, float bitcrushAmount, float smearAmount, float lfoAmount, float waveshapeAmount)
{
    float smoothedLFO = lfoManager.getSample(sample);

    // Apply a global LFO depth control
    float globalLFODepth = 1.5f; // Adjust this value to increase overall LFO impact
    float enhancedLFOAmount = lfoAmount * globalLFODepth;

    applyLFOToFilters(smoothedLFO, enhancedLFOAmount);

    float modifiedBitcrush = applyLFOToBitcrush(bitcrushAmount, enhancedLFOAmount, smoothedLFO);

    float delayTime = delayParameter->load();
    float delayInSamples = delayTime / 1000.0f * getSampleRate();

    // Get the input sample
    float inputSample = inputData[sample];

    // Calculate LFO modulation for delay
    float lfoModulation = 0.0f;
    if (lfoDelayParameter->load() > 0.5f)
    {
        lfoModulation = smoothedLFO * enhancedLFOAmount * 0.2f;
    }

    // Process delay sample with both LFO modulation and smear
    float delaySample = processDelaySample(channel, delayInSamples, smearAmount, lfoModulation);

    // Apply smear (diffusion and chorus) to the delayed signal
    delaySample = processDiffusionFilters(delaySample, channel);

    // Apply bitcrushing to the delayed signal
    if (modifiedBitcrush < 16.0f)
    {
        delaySample = applyBitcrushing(delaySample, modifiedBitcrush, waveshapeAmount);
    }

    // Apply DC blocking filter to the delayed sample
    delaySample = dcBlocker[static_cast<size_t>(channel)].processSample(delaySample);

    delayManager.pushSample(channel, inputSample + (delaySample * feedback));
    wetData[sample] = delaySample;

    updateChorusPhase();

    // Debug output (if needed)
    if (sample == 0)
    {
        DBG("Channel " << channel << " - Sample 0:");
        DBG("  Input: " << inputSample << ", Output: " << delaySample);
        DBG("  LFO: " << smoothedLFO << ", Enhanced Amount: " << enhancedLFOAmount);
        DBG("  Smear Amount: " << smearAmount << ", LFO Modulation: " << lfoModulation);
    }
}

void AudioDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{

    if (parameterID == "delay")
    {
        float delayInSamples = newValue / 1000.0f * getSampleRate();
        delayManager.setDelay(delayInSamples);
    }
    else if (parameterID == "tempoSync")
    {
        updateDelayTimeFromSync();
    }
    if (parameterID == "smear")
    {
        updateDiffusionFilters();
    }
    else if (parameterID == "lfoTempoSync")
    {
        updateLFOFrequency();
    }
    else if (parameterID == "lfoFreq")
    {
        lfoManager.setFrequency(newValue);
        updateLFOFrequency();
    }
    else if (parameterID == "highpassFreq" || parameterID == "lowpassFreq" ||
             parameterID == "lfoHighpass" || parameterID == "lfoLowpass" ||
             parameterID == "lfoAmount")
    {
        updateFilterParameters();
    }
}

void AudioDelayAudioProcessor::updateLFOFrequency()
{
    DBG("updating lfo freq");
    int syncMode = static_cast<int>(lfoTempoSyncParameter->load());

    if (syncMode != 0) // Not in Free mode
    {
        float syncedFreq = lfoManager.updateFrequencyFromSync(static_cast<float>(lastKnownBPM), syncMode);

        // Only update the knob value if in sync mode
        if (syncedFreq > 0.0f)
        {
            if (auto *param = parameters.getParameter("lfoFreq"))
            {
                float normalizedValue = param->convertTo0to1(syncedFreq);
                param->setValueNotifyingHost(normalizedValue);
            }
        }
    }

    // Always set the LFO frequency from the knob value
    lfoManager.setFrequency(lfoFreqParameter->load());
}

void AudioDelayAudioProcessor::updateBPMIfChanged()
{
    auto playHead = getPlayHead();
    if (playHead != nullptr)
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                double currentBPM = *positionInfo->getBpm();
                if (std::abs(currentBPM - lastKnownBPM) > 0.01)
                {
                    lastKnownBPM = currentBPM;
                    updateDelayTimeFromSync();
                    updateLFOFrequency();
                }
            }
        }
    }
}

void AudioDelayAudioProcessor::timerCallback()
{
    stopTimer();
    updateDelayTimeFromSync();
}

void AudioDelayAudioProcessor::updateFilterParameters()
{
    float lfoAmount = lfoAmountParameter->load();

    // We can't use a specific sample index here, so we'll use the middle of the buffer
    int middleSample = lfoManager.getBufferSize() / 2;
    float smoothedLFO = lfoManager.getSample(middleSample);

    applyLFOToFilters(smoothedLFO, lfoAmount);
}

float AudioDelayAudioProcessor::applyBitcrushing(float sample, float bitcrushAmount, float waveshapeAmount)
{
    int bits = static_cast<int>(bitcrushAmount);
    float maxValue = std::pow(2, bits) - 1;
    float crushedSample = std::round(sample * maxValue) / maxValue;

    float shapedSample = waveShaper.processSample(crushedSample);
    // Mix between crushed and shaped sample
    // return crushedSample * (1.0f - waveshapeAmount) + shapedSample * waveshapeAmount;

    return shapedSample;
}

float AudioDelayAudioProcessor::applyLFO(float baseValue, float lfoAmount, float lfoValue, float minValue, float maxValue)
{
    float range = maxValue - minValue;
    float modulation = lfoValue * lfoAmount * range * 0.5f;
    return juce::jlimit(minValue, maxValue, baseValue + modulation);
}

float AudioDelayAudioProcessor::applyLFOToPan(float basePan, float lfoAmount, float lfoValue)
{
    // Assume lfoValue is in the range [0, 1]
    // Convert it to the range [-1, 1]
    float bipolarLFO = 2.0f * lfoValue - 1.0f;

    // Calculate the modulation, scaling it by lfoAmount
    float modulation = bipolarLFO * lfoAmount;

    // Apply the modulation to the base pan
    // This ensures that when basePan is 0 (center), the LFO modulates equally in both directions
    float modifiedPan = basePan + (1.0f - std::abs(basePan)) * modulation;

    // Limit the result to the valid range [-1, 1]
    return juce::jlimit(-1.0f, 1.0f, modifiedPan);
}

float AudioDelayAudioProcessor::applyLFOToDelay(float delayInSamples, float lfoAmount, float smoothedLFO)
{
    if (lfoDelayParameter->load() > 0.5f)
    {
        return delayInSamples * (1.0f + smoothedLFO * lfoAmount * 0.2f);
    }
    return delayInSamples;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioDelayAudioProcessor();
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