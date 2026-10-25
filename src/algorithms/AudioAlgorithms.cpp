#include "AudioAlgorithms.h"
#include <cmath>
namespace nazal {
std::vector<const Clip *> activeClips(const Project &project, double time) {
    std::vector<const Clip *> result;
    for (const auto &track : project.tracks)
        for (const auto &clip : track.clips)
            if (clip.start <= time && time < clip.end())
                result.push_back(&clip);
    return result;
}
std::vector<WavePeak> generateWaveform(const juce::AudioBuffer<float> &samples, int regions) {
    int size = std::min(std::max(1, regions), samples.getNumSamples());
    std::vector<WavePeak> result(static_cast<size_t>(size));
    for (int region = 0; region < size; ++region) {
        auto &peak = result[static_cast<size_t>(region)];
        peak.minimum = 1;
        peak.maximum = -1;
        int first = static_cast<int>(static_cast<int64_t>(region) * samples.getNumSamples() / size);
        int last =
            static_cast<int>(static_cast<int64_t>(region + 1) * samples.getNumSamples() / size);
        for (int channel = 0; channel < samples.getNumChannels(); ++channel)
            for (int sample = first; sample < last; ++sample) {
                float value = samples.getSample(channel, sample);
                peak.minimum = std::min(peak.minimum, value);
                peak.maximum = std::max(peak.maximum, value);
            }
    }
    return result;
}
float fadeGain(const Clip &clip, double time) {
    double gain = 1;
    if (clip.fadeIn > 0)
        gain *= juce::jlimit(0.0, 1.0, time / clip.fadeIn);
    if (clip.fadeOut > 0)
        gain *= juce::jlimit(0.0, 1.0, (clip.duration - time) / clip.fadeOut);
    return static_cast<float>(gain);
}
void render(const Project &project, juce::AudioBuffer<float> &output, int first, int count,
            double time, double rate) {
    output.clear(first, count);
    bool solo = false;
    for (const auto &track : project.tracks)
        solo = solo || track.solo;
    // Linear scan of all clips per block. Only overlapping clips enter the sample loop.
    for (const auto &track : project.tracks) {
        if (track.mute || (solo && !track.solo))
            continue;
        for (const auto &clip : track.clips) {
            if (!clip.source || clip.end() <= time || clip.start >= time + count / rate)
                continue;
            const auto &source = *clip.source;
            for (int sample = 0; sample < count; ++sample) {
                double local = time + sample / rate - clip.start;
                if (local < 0 || local >= clip.duration)
                    continue;
                double sourceSample = (clip.offset + local) * source.sampleRate;
                int index = static_cast<int>(sourceSample);
                if (index < 0 || index >= source.samples.getNumSamples())
                    continue;
                float fraction = static_cast<float>(sourceSample - index);
                float gain = track.volume * fadeGain(clip, local);
                for (int channel = 0; channel < output.getNumChannels(); ++channel) {
                    int sourceChannel = std::min(channel, source.samples.getNumChannels() - 1);
                    float a = source.samples.getSample(sourceChannel, index);
                    float b = source.samples.getSample(
                        sourceChannel, std::min(index + 1, source.samples.getNumSamples() - 1));
                    output.addSample(channel, first + sample, (a + fraction * (b - a)) * gain);
                }
            }
        }
    }
    // Match the device's normalized range; summed loud tracks hard-clip rather than wrap.
    for (int channel = 0; channel < output.getNumChannels(); ++channel)
        for (int sample = first; sample < first + count; ++sample)
            output.setSample(channel, sample,
                             juce::jlimit(-1.0f, 1.0f, output.getSample(channel, sample)));
}
} // namespace nazal
