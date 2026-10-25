# Team Nazal — Interactive Multi-Track Audio Editor and Mixer

A C++17/JUCE desktop application implementing the core scope of Assignments 1–3. The main window follows the supplied DAW timeline wireframes: a Project menu with keyboard shortcuts, a transport bar (skip, play, pause, stop, seek bar and current/total time), a track mixer with an Add Track button, waveforms, a clip right-click menu and a global playhead.

## Setup, build and test

Install CMake 3.22+ and a C++17 compiler/toolchain (macOS: Xcode Command Line Tools; Windows: Visual Studio 2022 C++; Linux: GCC/Clang plus JUCE's ALSA, X11, FreeType and fontconfig development dependencies). Internet is required on first configuration to fetch the pinned JUCE 8.0.12 source.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 4
ctest --test-dir build -C Release --output-on-failure
```

For an existing JUCE checkout, add `-DJUCE_DIR=/absolute/path/to/JUCE` to the configure command. JUCE's license is supplied with its source; review the JUCE license before redistributing a binary.

Run on macOS:

```sh
open "build/NazalEditor_artefacts/Release/Nazal Audio Editor.app"
```

On Windows, run `build/NazalEditor_artefacts/Release/Nazal Audio Editor.exe`. On Linux, run `build/NazalEditor_artefacts/Release/Nazal Audio Editor`. Debug builds use `Debug` in place of `Release`. On macOS the build ad-hoc signs the complete bundle for local execution; this is not a notarized distribution. No microphone permission is required: the app opens output only.

## Using the editor

1. Create a project with **Project ▸ New Project** (Cmd/Ctrl+N); add tracks with **+ Add Track**, which sits below the last track in the track list. Click a track header to select it. Double-click its name to rename; **Delete** removes the track and its clips.
2. Click the ruler to set the insertion/playback position. **Project ▸ Import Audio…** (Cmd/Ctrl+I) adds audio to the selected track at that position. WAV, AIFF, MP3, FLAC and Ogg are supported by the enabled JUCE readers.
3. Select a clip. Drag its body to move it on the track; drag an edge inward to trim. Use the bottom **Position** / **Move** controls for precise placement. Enter a timeline **Trim begin/end** and click **Trim region** to retain that portion. **Remove clip** deletes the selected clip reference.
4. Click the ruler (or drag the seek bar) so the playhead is inside a clip, then use **Split at cursor** or right-click the clip for **Split at Playhead**, **Trim Start/End to Playhead**, **Fade In/Out to Playhead** and **Delete** (items that need the playhead inside the clip are greyed out otherwise). Splitting creates two references to the same recording.
5. Adjust each track's volume, mute and solo. Enter fade-in/out durations for the selected clip and click **Apply fades**.
6. The **Go to start** (⏮), **Play** (▶, also resumes), **Pause** (❚❚), **Stop** (■) and **Go to end** (⏭) icon buttons, the seek bar and ruler seeking control one synchronized timeline; the clock shows current / total time as mm:ss.mmm. The magnifier icon marks the zoom slider. Space toggles playback, Home/End jump to the start/end, and Delete removes the selected clip when the main window has focus.
7. **Project ▸ Save Project…** (Cmd/Ctrl+S) writes a .nazal project. **Project ▸ Open Project…** (Cmd/Ctrl+O) restores tracks, edits, settings, fades and audio references. Keep the referenced audio accessible. New/Open/Close offer a discard choice for unsaved changes. On macOS the Project menu is in the system menu bar; on Windows and Linux it appears at the top of the window.
8. **Project ▸ Export Audio…** (Cmd/Ctrl+E) lets you choose 44.1/48 kHz and 16/24 bit stereo WAV. The status bar shows progress. Successful actions also appear as a dismissible notification; failures open an OK dialog (for example Import Error or Invalid Operation) with a readable message.

## Design and tests

See [docs/Specification.md](docs/Specification.md) for reconciliation and architecture, and [docs/Testing.md](docs/Testing.md) for validation. Sources are separated into UI, core editing logic, model, algorithms and JUCE/system interfaces. Project/track/clip collections use vectors. Clip editing changes metadata without modifying source audio. Linear clip scanning and cached min/max waveforms follow Assignment 3. Playback and export share one mixing routine.

## Implemented and limitations

Implemented: new/open/save, track create/rename/delete, audio import, waveform timeline, select/move/trim/split/remove, synchronized play/pause/stop/resume/seek, volume/mute/solo, linear fades, referenced-source persistence, WAV export and success/error feedback. Expensive file work runs off the UI thread, and the device callback does no decoding or disk I/O.

Open `examples/Demo.nazal` for a three-track composition with generated beat, melody and pad recordings. You can also launch the executable with an absolute .nazal path as its single argument.

This is a small/medium-session student editor. The timeline is bounded to 24 hours. Sources decode into memory (256 MB maximum per mono/stereo file); no streaming, undo, cross-track clip drag, relink dialog, advanced effects, automation or collaboration. Trimming only keeps a region of the current clip. Split keeps fades at the outer edges and clamps their lengths. Resampling uses linear interpolation. Loud mixes hard-clip at full scale; lower track volumes when necessary. A concurrent metadata swap can cause one silent audio block. Missing source files abort reopen while preserving the current project. Only WAV export is offered. Native device behavior and cross-platform packaging must be checked on each target OS; this workspace validates macOS.
