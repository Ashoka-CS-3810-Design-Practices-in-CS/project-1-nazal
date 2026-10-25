#pragma once
#include "model/Project.h"
#include <atomic>
#include <map>
namespace nazal {
class AudioFiles {
  public:
    AudioFiles();
    juce::Result load(const juce::File &, std::shared_ptr<const AudioSource> &);

  private:
    juce::AudioFormatManager formats;
    std::map<juce::String, std::weak_ptr<const AudioSource>> cache;
};
juce::Result exportWav(const Project &, const juce::File &, double rate, int bits,
                       std::atomic<double> *progress = nullptr,
                       const std::atomic<bool> *cancel = nullptr);
} // namespace nazal
