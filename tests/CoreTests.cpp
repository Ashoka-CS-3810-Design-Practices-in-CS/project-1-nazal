#include "algorithms/AudioAlgorithms.h"
#include "core/Editor.h"
#include "system/Playback.h"
#include "system/ProjectStore.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace nazal;
int checks = 0;
void require(bool ok, const char *text) {
    ++checks;
    if (!ok)
        throw std::runtime_error(text);
}
void near(double a, double b, const char *text, double tolerance = 0.0001) {
    require(std::abs(a - b) < tolerance, text);
}
int main() {
    try {
        auto dir = juce::File::getCurrentWorkingDirectory()
                       .getChildFile("test-output")
                       .getChildFile("nazal-test-" + juce::Uuid().toString());
        require(dir.createDirectory().wasOk(), "create test folder");
        struct Cleanup {
            juce::File file;
            ~Cleanup() { file.deleteRecursively(); }
        } cleanup{dir};
        auto wavFile = dir.getChildFile("tone.wav");
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> stream = wavFile.createOutputStream();
        auto writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions{}
                                                      .withSampleRate(44100)
                                                      .withNumChannels(1)
                                                      .withBitsPerSample(16));
        juce::AudioBuffer<float> tone(1, 44100);
        for (int i = 0; i < 44100; ++i)
            tone.setSample(0, i, 0.25f);
        require(writer && writer->writeFromAudioSampleBuffer(tone, 0, 44100), "fixture WAV");
        writer.reset();
        AudioFiles audio;
        std::shared_ptr<const AudioSource> source;
        require(audio.load(wavFile, source).wasOk(), "T1 valid WAV import");
        near(source->duration(), 1, "T1 duration");
        require(!source->waveform.empty(), "T1 waveform");
        near(source->waveform[0].maximum, 0.25, "T1 peak");
        std::shared_ptr<const AudioSource> cached;
        require(audio.load(wavFile, cached).wasOk() && cached == source, "shared audio cache");
        Editor editor;
        auto a = editor.addTrack("Drums"), b = editor.addTrack("Guitar");
        require(editor.addClip(a, source, 0).wasOk(), "clip placement");
        require(editor.addClip(b, source, 0.5).wasOk(), "second placement");
        auto id = editor.track(a)->clips[0].id;
        require(editor.split(a, id, 0.4).wasOk(), "T2 split");
        require(editor.track(a)->clips.size() == 2, "T2 two clips");
        near(editor.track(a)->clips[1].offset, 0.4, "T2 source offset");
        require(editor.track(a)->clips[1].source == source, "T2 non-destructive");
        require(activeClips(editor.project, 0.4).size() == 1, "half-open split boundary");
        require(activeClips(editor.project, 0.6).size() == 2, "T3 active clips");
        require(activeClips(editor.project, 1.5).empty(), "end boundary excluded");
        juce::AudioBuffer<float> mix(2, 44100);
        render(editor.project, mix, 0, 44100, 0, 44100);
        near(mix.getSample(0, 100), 0.25, "T3 first track");
        near(mix.getSample(1, 22050), 0.5, "T3 synchronized onset");
        near(mix.getSample(0, 17640), 0.25, "T2 split has no gap");
        editor.track(a)->volume = 0.5f;
        render(editor.project, mix, 0, 100, 0.6, 44100);
        near(mix.getSample(0, 1), 0.375, "T4 volume");
        editor.track(a)->mute = true;
        render(editor.project, mix, 0, 100, 0.6, 44100);
        near(mix.getSample(0, 1), 0.25, "T4 mute");
        editor.track(a)->mute = false;
        editor.track(a)->solo = true;
        render(editor.project, mix, 0, 100, 0.6, 44100);
        near(mix.getSample(0, 1), 0.125, "T4 solo");
        editor.track(a)->mute = true;
        render(editor.project, mix, 0, 100, 0.6, 44100);
        near(mix.getSample(0, 1), 0, "mute beats solo");
        editor.track(a)->mute = false;
        editor.track(a)->solo = false;
        require(editor.fades(b, editor.track(b)->clips[0].id, 0.2, 0.3).wasOk(), "fade settings");
        near(fadeGain(editor.track(b)->clips[0], 0.1), 0.5, "fade-in gain");
        near(fadeGain(editor.track(b)->clips[0], 0.85), 0.5, "fade-out gain");
        auto projectFile = dir.getChildFile("session.nazal");
        require(saveProject(editor.project, projectFile).wasOk(), "T5 save");
        Project restored;
        require(loadProject(projectFile, audio, restored).wasOk(), "T5 reopen");
        require(restored.tracks.size() == 2 && restored.tracks[0].clips.size() == 2,
                "T5 structure");
        near(restored.tracks[0].volume, 0.5, "T5 volume");
        near(restored.tracks[1].clips[0].fadeOut, 0.3, "T5 fade");
        require(restored.tracks[0].clips[0].source == restored.tracks[0].clips[1].source,
                "T5 source sharing");
        auto bad = dir.getChildFile("bad.wav");
        bad.replaceWithText("not audio");
        require(audio.load(bad, cached).failed(), "T6 invalid import");
        require(audio.load(dir.getChildFile("missing.wav"), cached).failed(), "missing import");
        auto before = editor.track(a)->clips[0];
        require(editor.split(a, id, before.start).failed(), "T7 split begin");
        require(editor.split(a, id, before.end()).failed(), "T7 split end");
        require(editor.split(a, id, -1).failed(), "T7 split outside");
        require(editor.trim(a, id, before.start, before.start).failed(), "T7 empty trim");
        require(editor.trim(a, id, -1, before.end()).failed(), "T7 outside trim");
        near(editor.track(a)->clips[0].duration, before.duration, "T7 state unchanged");
        require(editor.move(a, id, -1).failed(), "negative move");
        require(editor.fades(a, id, 5, 0).failed(), "invalid fade");
        auto out = dir.getChildFile("mix.wav");
        require(exportWav(editor.project, out, 44100, 24).wasOk(), "T8 export");
        std::shared_ptr<const AudioSource> exported;
        require(audio.load(out, exported).wasOk(), "T8 playable output");
        near(exported->duration(), 1.5, "T8 duration");
        render(editor.project, mix, 0, 44100, 0, 44100);
        for (int i : {0, 100, 17640, 22050, 30000, 44099})
            near(exported->samples.getSample(0, i), mix.getSample(0, i),
                 "T8 playback-export agreement");
        require(exportWav(editor.project, wavFile, 44100, 16).failed(), "protect source");
        require(exportWav(Project{}, out, 44100, 16).failed(), "empty export");
        Playback playback;
        playback.setProject(editor.project);
        playback.play();
        playback.process(mix, 0, 100);
        near(playback.position(), 100.0 / 44100, "transport advances");
        playback.pause();
        playback.process(mix, 0, 100);
        near(playback.position(), 100.0 / 44100, "pause retains position");
        near(mix.getSample(0, 0), 0, "pause silence");
        playback.seek(0.6);
        playback.play();
        playback.process(mix, 0, 100);
        near(playback.position(), 0.6 + 100.0 / 44100, "seek-resume");
        playback.stop();
        near(playback.position(), 0, "stop resets");
        require(!playback.isPlaying(), "stop status");
        auto right = editor.track(a)->clips[1].id;
        require(editor.trim(a, right, 0.5, 0.9).wasOk(), "valid trim");
        near(editor.clip(a, right)->offset, 0.5, "trim source offset");
        require(editor.move(a, right, 2).wasOk(), "valid move");
        near(editor.clip(a, right)->start, 2, "move position");
        require(editor.remove(a, right).wasOk(), "remove clip");
        require(editor.renameTrack(a, "Renamed").wasOk(), "rename track");
        require(editor.deleteTrack(b).wasOk(), "delete track");
        source->file.deleteFile();
        Project unchanged;
        unchanged.name = "Keep me";
        require(loadProject(projectFile, audio, unchanged).failed(), "missing source reopen");
        require(unchanged.name == "Keep me", "transactional failed reopen");
        bad.replaceWithText("{}");
        require(loadProject(bad, audio, unchanged).failed(), "invalid project");
        require(unchanged.name == "Keep me", "invalid project preserves state");
        auto stereo = std::make_shared<AudioSource>();
        stereo->sampleRate = 22050;
        stereo->samples.setSize(2, 4);
        for (int i = 0; i < 4; ++i) {
            stereo->samples.setSample(0, i, 0.1f * i);
            stereo->samples.setSample(1, i, -0.1f * i);
        }
        Project rates;
        Track rt;
        Clip rc;
        rc.source = stereo;
        rc.duration = 4.0 / 22050;
        rt.clips.push_back(rc);
        rates.tracks.push_back(rt);
        mix.clear();
        mix.setSample(0, 0, 0.8f);
        render(rates, mix, 2, 4, 0, 44100);
        near(mix.getSample(0, 3), 0.05, "resampling interpolation");
        near(mix.getSample(1, 3), -0.05, "stereo channels");
        near(mix.getSample(0, 0), 0.8, "render respects output offset");
        auto peaks = generateWaveform(stereo->samples, 2);
        near(peaks[1].maximum, 0.3, "waveform maximum");
        near(peaks[1].minimum, -0.3, "waveform minimum");
        require(editor.move(a, id, std::numeric_limits<double>::quiet_NaN()).failed(),
                "nonfinite move");
        require(editor.move(a, id, 86400).failed(), "timeline bound");
        auto validProject = dir.getChildFile("states.nazal");
        Project states = editor.project;
        states.tracks[0].mute = true;
        states.tracks[0].solo = true;
        states.tracks[0].clips[0].source = stereo;
        // Use an existing file-backed source for persistence testing.
        states.tracks[0].clips[0].source = exported;
        states.tracks[0].clips[0].offset = 0.2;
        require(saveProject(states, validProject).wasOk(), "persist nondefault settings");
        Project statesBack;
        require(loadProject(validProject, audio, statesBack).wasOk(), "reopen nondefault settings");
        require(statesBack.tracks[0].mute && statesBack.tracks[0].solo, "persist mute/solo");
        near(statesBack.tracks[0].clips[0].offset, 0.2, "persist source offset");
        auto invalid = juce::JSON::parse(validProject.loadFileAsString());
        invalid["tracks"].getArray()->getReference(0).getDynamicObject()->setProperty("volume", -1);
        bad.replaceWithText(juce::JSON::toString(invalid));
        require(loadProject(bad, audio, unchanged).failed(), "reject invalid stored volume");
        require(unchanged.name == "Keep me", "invalid settings preserve project");
        auto cancelOutput = dir.getChildFile("cancel.wav");
        cancelOutput.replaceWithText("previous output");
        std::atomic<bool> cancelled{true};
        require(exportWav(states, cancelOutput, 48000, 16, nullptr, &cancelled).failed(),
                "cancel export");
        require(cancelOutput.loadFileAsString() == "previous output",
                "cancel preserves prior file");
        auto otherRate = dir.getChildFile("48k.wav");
        require(exportWav(states, otherRate, 48000, 16).wasOk(), "48k 16bit export");
        std::shared_ptr<const AudioSource> rateSource;
        require(audio.load(otherRate, rateSource).wasOk(), "reimport 48k export");
        near(rateSource->sampleRate, 48000, "export rate");
        Project stress;
        for (int i = 0; i < 100; ++i) {
            Track t;
            t.volume = 0.002f;
            for (int j = 0; j < 4; ++j) {
                Clip c;
                c.source = exported;
                c.start = j * 0.1;
                c.duration = 1;
                t.clips.push_back(c);
            }
            stress.tracks.push_back(t);
        }
        auto startTime = std::chrono::steady_clock::now();
        for (int block = 0; block < 100; ++block)
            render(stress, mix, 0, 256, 0.4, 44100);
        auto elapsed =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime)
                .count();
        near(mix.getSample(0, 0), 0.1, "400 clip mix sums correctly");
        std::cout << "400 clips / 100 tracks: " << elapsed / 100
                  << " ms per 256-frame block (measurement only)\n";
        // Scaling measurements for the performance plan in Assignment 3: how mixing, waveform
        // generation and export time grow with project size (measurement only).
        auto elapsedMs = [](std::chrono::steady_clock::time_point since) {
            return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() -
                                                             since)
                .count();
        };
        for (int trackCount : {10, 50, 100, 200}) {
            Project scaled;
            for (int i = 0; i < trackCount; ++i) {
                Track t;
                t.volume = 0.002f;
                for (int j = 0; j < 4; ++j) {
                    Clip c;
                    c.source = exported;
                    c.start = j * 0.1;
                    c.duration = 1;
                    t.clips.push_back(c);
                }
                scaled.tracks.push_back(t);
            }
            auto since = std::chrono::steady_clock::now();
            for (int block = 0; block < 100; ++block)
                render(scaled, mix, 0, 256, 0.4, 44100);
            std::cout << trackCount * 4 << " clips / " << trackCount
                      << " tracks: " << elapsedMs(since) / 100
                      << " ms per 256-frame block (measurement only)\n";
        }
        juce::AudioBuffer<float> longTone(2, 44100 * 180);
        for (int channel = 0; channel < 2; ++channel)
            for (int i = 0; i < longTone.getNumSamples(); ++i)
                longTone.setSample(channel, i, 0.5f * std::sin(0.01f * static_cast<float>(i)));
        auto since = std::chrono::steady_clock::now();
        auto longPeaks = generateWaveform(longTone);
        require(!longPeaks.empty(), "waveform of a 3 minute stereo source");
        std::cout << "waveform, 3 min stereo source: " << elapsedMs(since)
                  << " ms (measurement only)\n";
        Project longProject;
        Track longTrack;
        for (int j = 0; j < 120; ++j) {
            Clip c;
            c.source = exported;
            c.start = j * 0.5;
            c.duration = 1;
            longTrack.clips.push_back(c);
        }
        longProject.tracks.push_back(longTrack);
        since = std::chrono::steady_clock::now();
        require(exportWav(longProject, dir.getChildFile("long.wav"), 44100, 16).wasOk(),
                "export of a one minute, 120 clip project");
        std::cout << "export, 60 s / 120 clips: " << elapsedMs(since) << " ms (measurement only)\n";
        std::cout << "PASS: " << checks
                  << " checks across Assignment 3 T1-T8, transport, boundary and failure cases\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << "\n";
        return 1;
    }
}
