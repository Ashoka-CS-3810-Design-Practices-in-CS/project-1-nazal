#pragma once
#include "model/Project.h"
#include <atomic>
#include <juce_audio_devices/juce_audio_devices.h>
namespace nazal {
class Playback : public juce::AudioIODeviceCallback {
  public:
    void setProject(const Project &);
    void play();
    void pause();
    void stop();
    void seek(double);
    double position() const { return cursor.load(); }
    bool isPlaying() const { return playing.load(); }
    void process(juce::AudioBuffer<float> &, int first, int count);
    void audioDeviceIOCallbackWithContext(const float *const *, int, float *const *, int, int,
                                          const juce::AudioIODeviceCallbackContext &) override;
    void audioDeviceAboutToStart(juce::AudioIODevice *) override;
    void audioDeviceStopped() override { pause(); }
    void audioDeviceError(const juce::String &) override;
    juce::String takeError();

  private:
    juce::CriticalSection lock;
    Project snapshot;
    double rate = 44100;
    std::atomic<double> cursor{0};
    std::atomic<bool> playing{false};
    juce::String deviceError;
};
} // namespace nazal
