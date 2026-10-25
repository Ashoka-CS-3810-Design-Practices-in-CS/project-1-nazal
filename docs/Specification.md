# Reconciled specification

Authoritative references: Team Nazal's Assignment 1 (Project Specification and Analysis), Assignment 2 (Technical Design), and Assignment 3 (Testing, UX, and Development Plan). All three were read before implementation. The eight wireframes embedded in Assignment 3 are the preliminary UI reference.

- Assignment 1 defines FR1–FR7: project/track management, import/arrangement, editing, synchronized playback, track mixing/fades, persistence, and export. ER1–ER9 require readable errors and unchanged project state on failed operations.
- Assignment 2 fixes C++/JUCE/CMake, Project → vector of Track → vector of Clip, source buffers, cached min/max waveform regions, layered separation, non-destructive editing, and referenced audio files.
- Assignment 3 confirms O(N) active-clip scanning for small/medium projects, cached O(S) waveform generation, shared playback/export mixing, background expensive operations, and acceptance scenarios T1–T8. This newer design controls any earlier suggestion of binary search. The supplied Assignment 2 also describes a linear scan.
- Implement only basic volume, mute, solo, fade-in, and fade-out. No EQ, compressor, automation, collaboration, or plugins.

## Concrete decisions where the documents leave details open

- Time is seconds in metadata; playback uses one global output-sample clock. Clips use half-open intervals [start,end). Overlapping clips, including on the same track, add together.
- Sources decode once into shared immutable JUCE float buffers. Cached min/max peaks are built on the worker thread. Stereo sources preserve channels; mono sources feed both output channels. Other channel counts are rejected.
- Different source sample rates use linear interpolation into the output device/export rate. This is adequate for the basic editor; high-quality resampling is a possible later refinement.
- Export is stereo PCM WAV at 44.1/48 kHz, 16/24 bit. Final values clamp to [-1,1], consistently in playback/export. Users should reduce track volumes if the mix clips.
- Muted tracks never contribute, including muted solo tracks. Any solo track excludes all non-solo tracks.
- Fades are linear clip-local ramps; overlapping fade-in/out gains multiply. Trim clamps fades to the new duration. Split keeps/clamps fades on the outer edges, removing fades at the new inner edges. This simple policy may change the envelope when splitting inside an existing fade.
- Trimming keeps a non-empty region inside the currently visible clip; the source file is never changed. There is no undo/restore-trim feature in core scope.
- Project format is versioned JSON (.nazal), storing relative source paths. Moving the project together with the same relative audio folder layout works; moving/removing only audio produces an explicit missing-file error. Reopen is all-or-nothing.
- Save/export use temporary files and replace the destination only after successful processing. Export refuses to overwrite a referenced source file.
- The initial version bounds a decoded source to 256 MB, mono/stereo, and project metadata to 16 MB. Projects are intended for small/medium sessions.
- A brief metadata snapshot swap is protected by a lock. The audio thread never waits for the lock and outputs silence for a contested block. Long operations never take that lock.

## Architecture

`model/Project.h`: Project, Track, Clip, shared AudioSource and WavePeak data.

`core/Editor`: validated student-readable editing and track operations.

`algorithms/AudioAlgorithms`: linear interval checks, min/max waveform calculation, fades and shared render/mix routine. The renderer scans all clips once per block and processes only those overlapping the block; it does not allocate active-clip lists in the device callback. The separate activeClips routine implements the documented O(N)/O(K) query for testing/other clients.

`system/AudioFiles`, `ProjectStore`, `Playback`: JUCE decoders/encoders, versioned filesystem storage, output-device callback and transport. No filesystem operations or decoding occur in the audio callback.

`ui/MainComponent`: desktop controls, timeline drawing/interaction, worker dispatch, readable status feedback. Selected clip metadata flows through Editor; playback/export use the same project representation.
