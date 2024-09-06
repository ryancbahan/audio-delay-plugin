#pragma once

#include <juce_dsp/juce_dsp.h>

class LFOManager
{
public:
    LFOManager();
    void prepare(const juce::dsp::ProcessSpec &spec);
    void setFrequency(float frequency);
    float getNextSample();
    float updateFrequencyFromSync(float bpm, int syncMode);

private:
    juce::dsp::Oscillator<float> lfo;
    float lastKnownBPM;
};