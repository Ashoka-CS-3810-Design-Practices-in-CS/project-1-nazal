#pragma once
#include "AudioFiles.h"
namespace nazal {
juce::Result saveProject(const Project &, const juce::File &);
juce::Result loadProject(const juce::File &, AudioFiles &, Project &);
} // namespace nazal
