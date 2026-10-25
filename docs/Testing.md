# Validation

`NazalTests` is an executable integration/unit suite registered with CTest. It generates a known mono WAV fixture and temporary project files, checks outcomes numerically, and removes its fixtures. Run with `ctest --test-dir build --output-on-failure`.

| Assignment 3 case | Automated coverage |
| --- | --- |
| T1 valid import | WAV decoding, duration, waveform peaks, clip placement and shared source cache |
| T2 split | Two regions, correct source offset, shared source identity, no gap at boundary |
| T3 synchronized playback | Different track start times, half-open active intervals, exact mixed onset in both channels |
| T4 volume/mute/solo | Volume scaling, mute, solo filtering, muted solo precedence |
| T5 save/reopen | Track/clip counts, settings, fades, shared sources and relative references |
| T6 invalid import | Corrupted WAV, missing file, readable rejection |
| T7 boundary edit | Split at start/end/outside, empty/outside trim, negative move, invalid fade, unchanged state |
| T8 export | Playable stereo WAV, duration, numerical agreement with playback renderer, source protection, empty project rejection |

Additional transport coverage checks advancement, pause silence and retained position, seek/resume, stop reset. Additional editor checks cover valid trim, offset adjustment, move, remove, rename and track deletion. Persistence failure checks verify missing sources/invalid project data never replace the current project.

## Manual desktop acceptance checklist

- Launch the app and check all controls at minimum and normal window size.
- Create/select/rename/delete tracks; import into the selected track at the cursor.
- Confirm waveforms and selection; move and edge-trim clips; split at a ruler position.
- Play/pause/resume/seek/stop, change volume/mute/solo, verify cursor and output.
- Save/reopen, export with both available rates/bit depths; verify status feedback.
- Reject an invalid import and invalid split; confirm current state remains usable.
- Check discard prompts for New/Open/Close and responsive UI while exporting.
- Project menu: New Project, Open Project…, Save Project…, Import Audio… and Export Audio… work from the menu bar and with Cmd/Ctrl+N, O, S, I and E; the items are greyed out while an import, save or export is running.
- Transport: skip to start/end buttons (and Home/End), play, pause and stop icons, the seek bar (click and drag) and the `mm:ss.mmm / mm:ss.mmm` clock stay in sync.
- Right-click a clip: Split at Playhead, Trim Start/End to Playhead, Fade In/Out to Playhead and Delete; items are disabled when the playhead is outside the clip.
- Feedback: success actions show a dismissible notification; an invalid import or split opens an OK dialog (Import Error / Invalid Operation).
- Opening a project or saving it leaves the title without an unsaved (`*`) marker until a real edit is made.

## Results on 12 October 2026

- Apple Silicon macOS, AppleClang 17, CMake 4.4.4, JUCE 8.0.12.
- Debug and Release application/test builds succeeded. Debug CTest passed; Release suite passed all 89 checks in approximately 0.7 seconds.
- 100 tracks / 400 shared-source clips: approximately 0.25 ms average per 256-frame block over 100 blocks in the Release renderer (measurement, not a portable performance guarantee).
- Additional checks cover stereo preservation, linear sample-rate conversion, output buffer offsets, waveform min/max values, nonfinite edits, timeline bounds, persisted mute/solo/offsets, malformed stored settings, cancelled export preserving prior output, and 48 kHz/16-bit export.
- Fixed and verified local macOS bundle signing: `codesign --verify --deep --strict` reports valid on disk.
- The standard configuration successfully fetched JUCE from the pinned, SHA-256 verified release archive.
- Native macOS GUI/device interaction was not validated on 12 October (desktop launch was blocked by approval review); the macOS acceptance record below is used for that. Automated transport tests use the actual Playback controller with deterministic buffers, not a physical output device.

The original assignments, including the preliminary wireframes, are retained unchanged under `docs/assignments/`.

## UI revision: results and scaling measurements

The UI was revised to match Assignment 3's figures (Project menu with shortcuts, transport with skip buttons / seek bar / current-total time, clip right-click menu, success notifications and error dialogs, Add Track below the last track) and a bug was fixed where reopening or saving a project immediately marked it as modified.

- Core suite after the revision: **91 checks passed** (two new checks cover waveform generation for a 3-minute stereo source and export of a one-minute, 120-clip project).
- The revised window was smoke-tested on Linux under a virtual display (open the demo project, skip to end, seek bar click, right-click Split at Playhead, Project menu and Ctrl+N shortcut, Playback Error and Invalid Operation dialogs, success notification). This is not a substitute for the macOS record below.
- Scaling measurements printed by `NazalTests` (Release build; Linux x86-64 sandbox with a single slow core, so absolute times are conservative; run the suite on your own machine for local numbers). A 256-frame block at 44.1 kHz is a 5.8 ms real-time budget.

| Project size | Mixing time per 256-frame block |
| --- | --- |
| 10 tracks / 40 clips | 0.06 ms |
| 50 tracks / 200 clips | 0.31 ms |
| 100 tracks / 400 clips | 0.60 ms |
| 200 tracks / 800 clips | 1.27 ms |

| Operation | Time |
| --- | --- |
| Waveform generation, 3-minute stereo source | 34 ms |
| Export, 60 s project with 120 clips | 43 ms |

Mixing time grows linearly with the number of clips, matching the O(T × S) analysis in Assignment 3.

## macOS acceptance record (complete before submission)

Tick each item after running it on the submitted `.app`, and note the macOS version and date.

- [ ] App launches; Project menu appears in the system menu bar with the five items and shortcuts
- [ ] Create tracks, import audio, play/pause/stop/skip, seek bar and clock
- [ ] Right-click clip menu: split, trim, fade and delete
- [ ] Save, reopen (no `*` in the title), export WAV and play the result
- [ ] Invalid import and invalid split show dialogs; the app stays usable
- Tested by / macOS version / date: ______________________

