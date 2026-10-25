#include "ProjectStore.h"
#include <cmath>
#include <set>
namespace nazal {
namespace {
juce::var object() {
    return juce::var(new juce::DynamicObject());
}
void set(juce::var &obj, const juce::Identifier &key, const juce::var &value) {
    obj.getDynamicObject()->setProperty(key, value);
}
bool number(const juce::var &obj, const char *field, double &value) {
    auto v = obj[field];
    if (!v.isInt() && !v.isInt64() && !v.isDouble())
        return false;
    value = static_cast<double>(v);
    return std::isfinite(value);
}
} // namespace
juce::Result saveProject(const Project &project, const juce::File &file) {
    if (file.isDirectory())
        return juce::Result::fail("Choose a file, rather than a folder, for the project.");
    for (const auto &track : project.tracks)
        for (const auto &clip : track.clips)
            if (clip.source && clip.source->file == file)
                return juce::Result::fail(
                    "Choose a different project file to protect source audio.");
    juce::var root = object();
    set(root, "format", "nazal-project");
    set(root, "version", 1);
    set(root, "name", project.name);
    juce::Array<juce::var> tracks;
    for (const auto &track : project.tracks) {
        auto t = object();
        set(t, "id", track.id);
        set(t, "name", track.name);
        set(t, "volume", track.volume);
        set(t, "mute", track.mute);
        set(t, "solo", track.solo);
        juce::Array<juce::var> clips;
        for (const auto &clip : track.clips) {
            if (!clip.source)
                return juce::Result::fail("A clip has no audio source. Remove it before saving.");
            auto c = object();
            set(c, "id", clip.id);
            set(c, "source", clip.source->file.getRelativePathFrom(file.getParentDirectory()));
            set(c, "start", clip.start);
            set(c, "offset", clip.offset);
            set(c, "duration", clip.duration);
            set(c, "fadeIn", clip.fadeIn);
            set(c, "fadeOut", clip.fadeOut);
            clips.add(c);
        }
        set(t, "clips", clips);
        tracks.add(t);
    }
    set(root, "tracks", tracks);
    juce::TemporaryFile temporary(file);
    if (!temporary.getFile().replaceWithText(juce::JSON::toString(root)) ||
        !temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail("Project could not be saved. Choose a writable folder.");
    return juce::Result::ok();
}
juce::Result loadProject(const juce::File &file, AudioFiles &audio, Project &output) {
    if (!file.existsAsFile() || file.getSize() > 16 * 1024 * 1024)
        return juce::Result::fail("This project is missing or too large to open.");
    juce::var root;
    if (juce::JSON::parse(file.loadFileAsString(), root).failed() ||
        root["format"].toString() != "nazal-project" ||
        (!root["version"].isInt() || static_cast<int>(root["version"]) != 1) ||
        !root["tracks"].isArray() || !root["name"].isString())
        return juce::Result::fail("This is not a supported Nazal project file.");
    Project candidate;
    candidate.name = root["name"].toString();
    std::set<juce::String> ids;
    for (const auto &t : *root["tracks"].getArray()) {
        Track track;
        double volume;
        track.id = t["id"].toString();
        track.name = t["name"].toString();
        if (track.id.isEmpty() || !ids.insert(track.id).second || track.name.trim().isEmpty() ||
            !number(t, "volume", volume) || volume < 0 || volume > 2 || !t["mute"].isBool() ||
            !t["solo"].isBool() || !t["clips"].isArray())
            return juce::Result::fail("This project contains invalid track settings.");
        track.volume = static_cast<float>(volume);
        track.mute = static_cast<bool>(t["mute"]);
        track.solo = static_cast<bool>(t["solo"]);
        for (const auto &c : *t["clips"].getArray()) {
            Clip clip;
            clip.id = c["id"].toString();
            if (clip.id.isEmpty() || !ids.insert(clip.id).second || !c["source"].isString() ||
                !number(c, "start", clip.start) || !number(c, "offset", clip.offset) ||
                !number(c, "duration", clip.duration) || !number(c, "fadeIn", clip.fadeIn) ||
                !number(c, "fadeOut", clip.fadeOut) || clip.start < 0 || clip.offset < 0 ||
                clip.duration <= 0 || clip.fadeIn < 0 || clip.fadeOut < 0 ||
                clip.fadeIn > clip.duration || clip.fadeOut > clip.duration ||
                !std::isfinite(clip.end()) || clip.end() > maxTimelineSeconds)
                return juce::Result::fail(
                    "This project contains invalid clip boundaries or fades.");
            juce::String path = c["source"].toString();
            auto sourceFile = juce::File::isAbsolutePath(path)
                                  ? juce::File(path)
                                  : file.getParentDirectory().getChildFile(path);
            auto loaded = audio.load(sourceFile, clip.source);
            if (loaded.failed())
                return loaded;
            if (clip.offset + clip.duration >
                clip.source->duration() + 0.5 / clip.source->sampleRate)
                return juce::Result::fail("A clip refers to audio outside its source file.");
            track.clips.push_back(clip);
        }
        candidate.tracks.push_back(track);
    }
    // Transactional load: leave the current project untouched if any validation fails.
    output = std::move(candidate);
    return juce::Result::ok();
}
} // namespace nazal
