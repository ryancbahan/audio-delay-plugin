#include "LFOManager.h"

LFOManager::LFOManager()
    : lastKnownBPM(120.0f)
{
    lfo.initialise([](float x)
                   { return std::sin(x); });
}

void LFOManager::prepare(const juce::dsp::ProcessSpec &spec)
{
    lfo.prepare(spec);
}

void LFOManager::setFrequency(float frequency)
{
    lfo.setFrequency(frequency);
}

float LFOManager::getNextSample()
{
    return lfo.processSample(0.0f);
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