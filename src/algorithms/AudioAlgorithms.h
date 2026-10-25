#pragma once
#include "model/Project.h"
namespace nazal {
std::vector<const Clip *> activeClips(const Project &, double time);
std::vector<WavePeak> generateWaveform(const juce::AudioBuffer<float> &, int regions = 2048);
float fadeGain(const Clip &, double localTime);
// Both the device callback and WAV exporter use this exact rendering function.
void render(const Project &, juce::AudioBuffer<float> &output, int startSample, int count,
            double timelineTime, double outputRate);
} // namespace nazal
