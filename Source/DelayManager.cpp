#include "DelayManager.h"

DelayManager::DelayManager()
    : lastKnownBPM(120.0f), sampleRate(44100.0f)
{
}

void DelayManager::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 5.0)); // 5 seconds maximum delay
}

void DelayManager::setDelay(float delayInSamples)
{
    delayLine.setDelay(delayInSamples);
}

float DelayManager::popSample(int channel, float delayInSamples)
{
    return delayLine.popSample(channel, delayInSamples);
}

void DelayManager::pushSample(int channel, float sample)
{
    delayLine.pushSample(channel, sample);
}

void DelayManager::updateDelayTimeFromSync(float bpm, int syncMode)
{
    lastKnownBPM = bpm;
    double beatsPerSecond = bpm / 60.0;
    double quarterNoteTime = 1000.0 / beatsPerSecond; // in milliseconds
    float delayTime = 0.0f;

    switch (syncMode)
    {
    case 0:     // Free
        return; // Don't change the delay time
    case 1:     // 1/1
        delayTime = static_cast<float>(quarterNoteTime * 4);
        break;
    case 2: // 1/2
        delayTime = static_cast<float>(quarterNoteTime * 2);
        break;
    case 3: // 1/4
        delayTime = static_cast<float>(quarterNoteTime);
        break;
    case 4: // 1/8
        delayTime = static_cast<float>(quarterNoteTime / 2);
        break;
    case 5: // 1/16
        delayTime = static_cast<float>(quarterNoteTime / 4);
        break;
    case 6: // 1/32
        delayTime = static_cast<float>(quarterNoteTime / 8);
        break;
    case 7: // 1/1T
        delayTime = static_cast<float>(quarterNoteTime * 4 * 2 / 3);
        break;
    case 8: // 1/2T
        delayTime = static_cast<float>(quarterNoteTime * 2 * 2 / 3);
        break;
    case 9: // 1/4T
        delayTime = static_cast<float>(quarterNoteTime * 2 / 3);
        break;
    case 10: // 1/8T
        delayTime = static_cast<float>(quarterNoteTime / 2 * 2 / 3);
        break;
    case 11: // 1/16T
        delayTime = static_cast<float>(quarterNoteTime / 4 * 2 / 3);
        break;
    case 12: // 1/32T
        delayTime = static_cast<float>(quarterNoteTime / 8 * 2 / 3);
        break;
    case 13: // 1/1D
        delayTime = static_cast<float>(quarterNoteTime * 4 * 1.5);
        break;
    case 14: // 1/2D
        delayTime = static_cast<float>(quarterNoteTime * 2 * 1.5);
        break;
    case 15: // 1/4D
        delayTime = static_cast<float>(quarterNoteTime * 1.5);
        break;
    case 16: // 1/8D
        delayTime = static_cast<float>(quarterNoteTime / 2 * 1.5);
        break;
    case 17: // 1/16D
        delayTime = static_cast<float>(quarterNoteTime / 4 * 1.5);
        break;
    case 18: // 1/32D
        delayTime = static_cast<float>(quarterNoteTime / 8 * 1.5);
        break;
    }

    float maxDelayTime = getMaximumDelayInSeconds() * 1000.0f; // Convert to milliseconds
    delayTime = juce::jlimit(0.0f, maxDelayTime, delayTime);

    float delayInSamples = delayTime / 1000.0f * sampleRate;
    setDelay(delayInSamples);
}

float DelayManager::getMaximumDelayInSeconds() const
{
    return static_cast<float>(delayLine.getMaximumDelayInSamples()) / sampleRate;
}