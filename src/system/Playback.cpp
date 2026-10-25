#include "Playback.h"
#include "algorithms/AudioAlgorithms.h"
#include <cmath>
namespace nazal {
void Playback::setProject(const Project &project) {
    Project copy = project; // Metadata copy and allocations happen outside the audio lock.
    const juce::ScopedLock guard(lock);
    std::swap(snapshot, copy);
}
void Playback::play() {
    const juce::ScopedLock guard(lock);
    if (cursor.load() >= snapshot.duration())
        cursor.store(0);
    playing.store(snapshot.duration() > 0);
}
void Playback::pause() {
    const juce::ScopedLock guard(lock);
    playing.store(false);
}
void Playback::stop() {
    const juce::ScopedLock guard(lock);
    playing.store(false);
    cursor.store(0);
}
void Playback::seek(double time) {
    const juce::ScopedLock guard(lock);
    if (std::isfinite(time))
        cursor.store(juce::jlimit(0.0, snapshot.duration(), time));
}
void Playback::process(juce::AudioBuffer<float> &buffer, int first, int count) {
    // Never wait on the device thread. A concurrent metadata swap produces a silent block.
    const juce::ScopedTryLock guard(lock);
    if (!guard.isLocked() || !playing.load()) {
        buffer.clear(first, count);
        return;
    }
    double time = cursor.load();
    render(snapshot, buffer, first, count, time, rate);
    double next = std::min(snapshot.duration(), time + count / rate);
    cursor.store(next);
    if (next >= snapshot.duration())
        playing.store(false);
}
void Playback::audioDeviceIOCallbackWithContext(const float *const *, int, float *const *output,
                                                int channels, int samples,
                                                const juce::AudioIODeviceCallbackContext &) {
    juce::AudioBuffer<float> buffer(output, channels, samples);
    process(buffer, 0, samples);
}
void Playback::audioDeviceAboutToStart(juce::AudioIODevice *device) {
    const juce::ScopedLock guard(lock);
    rate = device->getCurrentSampleRate();
}
void Playback::audioDeviceError(const juce::String &error) {
    const juce::ScopedLock guard(lock);
    playing.store(false);
    deviceError = error;
}
juce::String Playback::takeError() {
    const juce::ScopedLock guard(lock);
    auto result = deviceError;
    deviceError.clear();
    return result;
}
} // namespace nazal
