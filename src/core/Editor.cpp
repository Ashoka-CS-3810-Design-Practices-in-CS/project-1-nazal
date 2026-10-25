#include "Editor.h"
#include <cmath>
namespace nazal {
Track *Editor::track(const juce::String &id) {
    for (auto &t : project.tracks)
        if (t.id == id)
            return &t;
    return nullptr;
}
Clip *Editor::clip(const juce::String &tid, const juce::String &cid) {
    if (auto *t = track(tid))
        for (auto &c : t->clips)
            if (c.id == cid)
                return &c;
    return nullptr;
}
juce::String Editor::addTrack(juce::String name) {
    Track t;
    t.name = name.isEmpty() ? "Track " + juce::String(project.tracks.size() + 1) : name;
    project.tracks.push_back(t);
    return t.id;
}
juce::Result Editor::renameTrack(const juce::String &id, const juce::String &name) {
    auto *t = track(id);
    if (!t || name.trim().isEmpty())
        return juce::Result::fail("Please enter a track name.");
    t->name = name.trim();
    return juce::Result::ok();
}
juce::Result Editor::deleteTrack(const juce::String &id) {
    auto it = std::find_if(project.tracks.begin(), project.tracks.end(),
                           [&](const Track &t) { return t.id == id; });
    if (it == project.tracks.end())
        return juce::Result::fail("Please select a track.");
    project.tracks.erase(it);
    return juce::Result::ok();
}
juce::Result Editor::addClip(const juce::String &id, std::shared_ptr<const AudioSource> source,
                             double start) {
    auto *t = track(id);
    if (!t || !source || source->duration() <= 0 || !std::isfinite(start) || start < 0 ||
        start + source->duration() > maxTimelineSeconds)
        return juce::Result::fail("Select a track and a valid audio file and position.");
    Clip c;
    c.source = std::move(source);
    c.start = start;
    c.duration = c.source->duration();
    t->clips.push_back(c);
    return juce::Result::ok();
}
juce::Result Editor::move(const juce::String &tid, const juce::String &cid, double start) {
    auto *c = clip(tid, cid);
    if (!c || !std::isfinite(start) || start < 0 || start + c->duration > maxTimelineSeconds)
        return juce::Result::fail("Place the clip within the 24-hour timeline.");
    c->start = start;
    return juce::Result::ok();
}
juce::Result Editor::trim(const juce::String &tid, const juce::String &cid, double begin,
                          double end) {
    auto *c = clip(tid, cid);
    if (!c || !std::isfinite(begin) || !std::isfinite(end) || begin < c->start || end > c->end() ||
        begin >= end)
        return juce::Result::fail("Choose a non-empty region inside the clip.");
    c->offset += begin - c->start;
    c->start = begin;
    c->duration = end - begin;
    c->fadeIn = std::min(c->fadeIn, c->duration);
    c->fadeOut = std::min(c->fadeOut, c->duration);
    return juce::Result::ok();
}
juce::Result Editor::split(const juce::String &tid, const juce::String &cid, double time) {
    auto *c = clip(tid, cid);
    if (!c || !std::isfinite(time) || time <= c->start || time >= c->end())
        return juce::Result::fail("Cannot split here. Place the cursor inside the clip.");
    Clip right = *c;
    double leftDuration = time - c->start;
    right.id = juce::Uuid().toString();
    right.start = time;
    right.offset += leftDuration;
    right.duration -= leftDuration;
    c->duration = leftDuration;
    c->fadeIn = std::min(c->fadeIn, c->duration);
    c->fadeOut = 0;
    right.fadeIn = 0;
    right.fadeOut = std::min(right.fadeOut, right.duration);
    track(tid)->clips.push_back(right);
    return juce::Result::ok();
}
juce::Result Editor::remove(const juce::String &tid, const juce::String &cid) {
    auto *t = track(tid);
    if (!t)
        return juce::Result::fail("Select a clip first.");
    auto it =
        std::find_if(t->clips.begin(), t->clips.end(), [&](const Clip &c) { return c.id == cid; });
    if (it == t->clips.end())
        return juce::Result::fail("Select a clip first.");
    t->clips.erase(it);
    return juce::Result::ok();
}
juce::Result Editor::fades(const juce::String &tid, const juce::String &cid, double in,
                           double out) {
    auto *c = clip(tid, cid);
    if (!c || !std::isfinite(in) || !std::isfinite(out) || in < 0 || out < 0 || in > c->duration ||
        out > c->duration)
        return juce::Result::fail("Fade lengths must be between zero and the clip length.");
    c->fadeIn = in;
    c->fadeOut = out;
    return juce::Result::ok();
}
} // namespace nazal
