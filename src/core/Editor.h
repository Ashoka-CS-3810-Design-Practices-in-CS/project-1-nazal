#pragma once
#include "model/Project.h"
namespace nazal {
class Editor {
  public:
    Project project;
    Track *track(const juce::String &id);
    Clip *clip(const juce::String &trackId, const juce::String &clipId);
    juce::String addTrack(juce::String name = {});
    juce::Result renameTrack(const juce::String &, const juce::String &);
    juce::Result deleteTrack(const juce::String &);
    juce::Result addClip(const juce::String &, std::shared_ptr<const AudioSource>, double start);
    juce::Result move(const juce::String &, const juce::String &, double start);
    juce::Result trim(const juce::String &, const juce::String &, double begin, double end);
    juce::Result split(const juce::String &, const juce::String &, double time);
    juce::Result remove(const juce::String &, const juce::String &);
    juce::Result fades(const juce::String &, const juce::String &, double in, double out);
};
} // namespace nazal
