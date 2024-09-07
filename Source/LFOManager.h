#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

class LFOManager
{
public:
    LFOManager();
    void prepare(const juce::dsp::ProcessSpec &spec);
    void setFrequency(float frequency);
    void generateBlock(int numSamples);
    float getSample(int index) const;
    float updateFrequencyFromSync(float bpm, int syncMode);
    int getBufferSize() const { return static_cast<int>(lfoBuffer.size()); }
    bool isReady;

private:
    juce::dsp::Oscillator<float> lfo;
    float lastKnownBPM;
    std::vector<float> lfoBuffer;
    juce::SmoothedValue<float> smoother;
    float sampleRate;
};