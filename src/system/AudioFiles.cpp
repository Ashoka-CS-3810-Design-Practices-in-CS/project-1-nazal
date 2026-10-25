#include "AudioFiles.h"
#include "algorithms/AudioAlgorithms.h"
#include <cmath>
#include <limits>
namespace nazal {
AudioFiles::AudioFiles() {
    formats.registerBasicFormats();
}
juce::Result AudioFiles::load(const juce::File &file, std::shared_ptr<const AudioSource> &result) {
    if (!file.existsAsFile())
        return juce::Result::fail("Audio file is missing: " + file.getFullPathName());
    auto key = file.getFullPathName() + ":" +
               juce::String(file.getLastModificationTime().toMilliseconds());
    if (auto existing = cache[key].lock()) {
        result = existing;
        return juce::Result::ok();
    }
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if (!reader || reader->lengthInSamples <= 0 || !std::isfinite(reader->sampleRate) ||
        reader->sampleRate <= 0 || reader->numChannels < 1)
        return juce::Result::fail("This audio file could not be opened. Select a valid WAV, AIFF, "
                                  "MP3, FLAC or Ogg file.");
    // Decode once, share samples between clips. Bound allocation for malformed/very large files.
    if (reader->numChannels > 2 || reader->lengthInSamples > std::numeric_limits<int>::max() ||
        reader->lengthInSamples > (256LL * 1024 * 1024) / (reader->numChannels * sizeof(float)))
        return juce::Result::fail("This file is too large or has more than two channels. Use a "
                                  "shorter mono or stereo recording (decoded limit: 256 MB).");
    try {
        auto source = std::make_shared<AudioSource>();
        source->file = file;
        source->sampleRate = reader->sampleRate;
        source->samples.setSize(static_cast<int>(reader->numChannels),
                                static_cast<int>(reader->lengthInSamples));
        if (!reader->read(&source->samples, 0, source->samples.getNumSamples(), 0, true, true))
            return juce::Result::fail("The audio file is damaged or could not be read.");
        for (int ch = 0; ch < source->samples.getNumChannels(); ++ch)
            for (int i = 0; i < source->samples.getNumSamples(); ++i)
                if (!std::isfinite(source->samples.getSample(ch, i)))
                    return juce::Result::fail("This audio file contains invalid sample values.");
        source->waveform = generateWaveform(source->samples);
        result = source;
        cache[key] = source;
        return juce::Result::ok();
    } catch (const std::bad_alloc &) {
        return juce::Result::fail("Not enough memory to load this recording. Try a smaller file.");
    }
}
juce::Result exportWav(const Project &project, const juce::File &file, double rate, int bits,
                       std::atomic<double> *progress, const std::atomic<bool> *cancel) {
    if (file.isDirectory())
        return juce::Result::fail("Choose a file, rather than a folder, for the export.");
    if ((rate != 44100 && rate != 48000) || (bits != 16 && bits != 24) || project.duration() <= 0)
        return juce::Result::fail(
            "Add audio before exporting and choose 44.1 or 48 kHz, 16 or 24 bit WAV.");
    for (const auto &track : project.tracks)
        for (const auto &clip : track.clips)
            if (clip.source && clip.source->file == file)
                return juce::Result::fail(
                    "Choose a different output file to protect the original audio.");
    double total = std::ceil(project.duration() * rate);
    if (!std::isfinite(total) || total * 2 * (bits / 8) > 4290000000.0)
        return juce::Result::fail(
            "The mix exceeds the WAV size limit. Shorten the project before exporting.");
    juce::TemporaryFile temporary(file);
    std::unique_ptr<juce::OutputStream> stream = temporary.getFile().createOutputStream();
    if (!stream || static_cast<juce::FileOutputStream *>(stream.get())->failedToOpen())
        return juce::Result::fail("Cannot write here. Choose a writable folder.");
    juce::WavAudioFormat wav;
    auto writer = wav.createWriterFor(
        stream,
        juce::AudioFormatWriterOptions{}.withSampleRate(rate).withNumChannels(2).withBitsPerSample(
            bits));
    if (!writer)
        return juce::Result::fail("Cannot create the WAV file.");

    juce::AudioBuffer<float> buffer(2, 1024);
    auto frames = static_cast<int64_t>(total);
    for (int64_t offset = 0; offset < frames; offset += 1024) {
        if (cancel && cancel->load())
            return juce::Result::fail("Export cancelled.");
        int count = static_cast<int>(std::min<int64_t>(1024, frames - offset));
        render(project, buffer, 0, count, offset / rate, rate);
        if (!writer->writeFromAudioSampleBuffer(buffer, 0, count))
            return juce::Result::fail(
                "Export failed while writing audio. Check available disk space.");
        if (progress)
            progress->store(static_cast<double>(offset + count) / frames);
    }
    if (!writer->flush())
        return juce::Result::fail("Cannot finish writing the WAV file. Check disk space.");
    writer.reset();
    if (!temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail("Cannot finish the export. Check folder permissions.");
    return juce::Result::ok();
}
} // namespace nazal
