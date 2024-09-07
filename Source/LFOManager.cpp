#include "LFOManager.h"

LFOManager::LFOManager()
    : lastKnownBPM(120.0f), sampleRate(44100.0f)
{
    lfo.initialise([](float x)
                   { return std::sin(x); });
}

void LFOManager::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    lfo.prepare(spec);
    smoother.reset(sampleRate, 0.05f); // 50ms smoothing time
}

void LFOManager::setFrequency(float frequency)
{
    lfo.setFrequency(frequency);
}

void LFOManager::generateBlock(int numSamples)
{
    if (!isReady)
        return;

    lfoBuffer.resize(numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = lfo.processSample(0.0f);
        smoother.setTargetValue(sample);
        sample = smoother.getNextValue();
        lfoBuffer[i] = sample * 0.5f + 0.5f; // Convert from [-1, 1] to [0, 1] range
    }
}

float LFOManager::getSample(int index) const
{
    if (!isReady || index < 0 || index >= static_cast<int>(lfoBuffer.size()))
        return 0.0f;
    return lfoBuffer[index];
}

float LFOManager::updateFrequencyFromSync(float bpm, int syncMode)
{
    lastKnownBPM = bpm;
    double beatsPerSecond = bpm / 60.0;
    double quarterNoteTime = 1.0 / beatsPerSecond;
    float lfoFreq = 0.0f;

    switch (syncMode)
    {
    case 0:          // Free
        return 0.0f; // Don't change the frequency
    case 1:          // 1/1
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 4));
        break;
    case 2: // 1/2
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 2));
        break;
    case 3: // 1/4
        lfoFreq = static_cast<float>(1.0 / quarterNoteTime);
        break;
    case 4: // 1/8
        lfoFreq = static_cast<float>(2.0 / quarterNoteTime);
        break;
    case 5: // 1/16
        lfoFreq = static_cast<float>(4.0 / quarterNoteTime);
        break;
    case 6: // 1/32
        lfoFreq = static_cast<float>(8.0 / quarterNoteTime);
        break;
    case 7: // 1/1T
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 4 * 2 / 3));
        break;
    case 8: // 1/2T
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 2 * 2 / 3));
        break;
    case 9: // 1/4T
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 2 / 3));
        break;
    case 10: // 1/8T
        lfoFreq = static_cast<float>(2.0 / (quarterNoteTime * 2 / 3));
        break;
    case 11: // 1/16T
        lfoFreq = static_cast<float>(4.0 / (quarterNoteTime * 2 / 3));
        break;
    case 12: // 1/32T
        lfoFreq = static_cast<float>(8.0 / (quarterNoteTime * 2 / 3));
        break;
    case 13: // 1/1D
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 4 * 1.5));
        break;
    case 14: // 1/2D
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 2 * 1.5));
        break;
    case 15: // 1/4D
        lfoFreq = static_cast<float>(1.0 / (quarterNoteTime * 1.5));
        break;
    case 16: // 1/8D
        lfoFreq = static_cast<float>(2.0 / (quarterNoteTime * 1.5));
        break;
    case 17: // 1/16D
        lfoFreq = static_cast<float>(4.0 / (quarterNoteTime * 1.5));
        break;
    case 18: // 1/32D
        lfoFreq = static_cast<float>(8.0 / (quarterNoteTime * 1.5));
        break;
    }

    lfoFreq = juce::jlimit(0.01f, 20.0f, lfoFreq);
    return lfoFreq;
}