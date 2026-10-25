#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>
#include <vector>
namespace nazal {
constexpr double maxTimelineSeconds = 86400;
struct WavePeak {
    float minimum = 0, maximum = 0;
};
struct AudioSource {
    juce::File file;
    double sampleRate = 44100;
    juce::AudioBuffer<float> samples;
    std::vector<WavePeak> waveform;
    double duration() const { return samples.getNumSamples() / sampleRate; }
};
struct Clip {
    juce::String id = juce::Uuid().toString();
    std::shared_ptr<const AudioSource> source;
    double start = 0, offset = 0, duration = 0, fadeIn = 0, fadeOut = 0;
    double end() const { return start + duration; }
};
struct Track {
    juce::String id = juce::Uuid().toString(), name = "Track";
    std::vector<Clip> clips;
    float volume = 1;
    bool mute = false, solo = false;
};
struct Project {
    juce::String name = "Untitled project";
    std::vector<Track> tracks;
    double duration() const {
        double result = 0;
        for (const auto &track : tracks)
            for (const auto &clip : track.clips)
                result = std::max(result, clip.end());
        return result;
    }
};
} // namespace nazal
