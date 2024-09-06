#pragma once

#include <juce_dsp/juce_dsp.h>

class DelayManager
{
public:
    DelayManager();
    void prepare(const juce::dsp::ProcessSpec &spec);
    void setDelay(float delayInSamples);
    float getDelay() const { return delayLine.getDelay(); }
    float popSample(int channel, float delayInSamples);
    void pushSample(int channel, float sample);
    float updateDelayTimeFromSync(float bpm, int syncMode);
    float getMaximumDelayInSeconds() const;

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine;
    float lastKnownBPM;
    float sampleRate;
};