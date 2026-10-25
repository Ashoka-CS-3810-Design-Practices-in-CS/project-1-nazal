#pragma once
#include "core/Editor.h"
#include "system/Playback.h"
#include "system/ProjectStore.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <thread>
namespace nazal {
class Timeline;
enum class Icon { play, pause, stop, skipStart, skipEnd, search };
// Square icon button for the transport bar (graphic replacement for text buttons).
class IconButton : public juce::Button {
  public:
    IconButton(Icon, const juce::String &tooltip);
    void paintButton(juce::Graphics &, bool highlighted, bool down) override;

  private:
    Icon icon;
};
// Non-interactive icon (used for the zoom magnifier).
class IconView : public juce::Component, public juce::SettableTooltipClient {
  public:
    IconView(Icon, const juce::String &tooltip);
    void paint(juce::Graphics &) override;

  private:
    Icon icon;
};
// Dismissible success notification shown in the corner of the window.
class Toast : public juce::Component, private juce::Timer {
  public:
    Toast();
    void show(const juce::String &text);
    void paint(juce::Graphics &) override;
    void mouseUp(const juce::MouseEvent &) override;

  private:
    void timerCallback() override;
    juce::String text;
};
class MainComponent : public juce::Component,
                      public juce::MenuBarModel,
                      public juce::ApplicationCommandTarget,
                      private juce::Timer {
  public:
    MainComponent();
    ~MainComponent() override;
    void paint(juce::Graphics &) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress &) override;
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int, const juce::String &) override;
    void menuItemSelected(int, int) override;
    juce::ApplicationCommandTarget *getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID> &) override;
    void getCommandInfo(juce::CommandID, juce::ApplicationCommandInfo &) override;
    bool perform(const InvocationInfo &) override;
    bool canClose();
    void openProjectFile(const juce::File &);
    Editor editor;
    Playback playback;
    juce::String selectedTrack, selectedClip;
    void select(const juce::String &, const juce::String & = {});
    void changed(const juce::Result &, const juce::String &, bool rebuild = true);
    bool isBusy() const { return busy; }
    void refresh(bool rebuild = true);

  private:
    friend class Timeline;
    enum CommandId { cmdNew = 0x7001, cmdOpen, cmdSave, cmdImport, cmdExport };
    juce::ApplicationCommandManager commands; // Project menu items and their shortcuts
    std::unique_ptr<Timeline> timeline;
    juce::Viewport viewport;
    juce::AudioDeviceManager devices;
    AudioFiles audio;
    juce::TooltipWindow tooltipWindow{nullptr, 600};
#if !JUCE_MAC
    juce::MenuBarComponent menuBar{this}; // macOS uses the system menu bar instead
#endif
    juce::TextButton addButton{"+ Add Track"}; // lives at the bottom of the track list
    IconButton skipStartButton{Icon::skipStart, "Go to start (Home)"},
        playButton{Icon::play, "Play / Resume (Space)"}, pauseButton{Icon::pause, "Pause (Space)"},
        stopButton{Icon::stop, "Stop"}, skipEndButton{Icon::skipEnd, "Go to end (End)"};
    juce::TextButton splitButton{"Split at cursor"}, removeButton{"Remove clip"};
    IconView zoomIcon{Icon::search, "Zoom"};
    juce::Label title, status, clock, selectedLabel;
    juce::Slider zoom, seek;
    Toast toast;
    double seekMax = -1;
    bool updatingSeek = false;
    std::array<juce::Label, 5> fieldLabels;
    std::array<juce::TextEditor, 5> fields;
    juce::TextButton moveButton{"Move"}, trimButton{"Trim region"}, fadeButton{"Apply fades"};
    std::unique_ptr<juce::FileChooser> chooser;
    std::thread worker;
    std::atomic<double> progress{0};
    std::atomic<bool> cancel{false};
    bool busy = false, dirty = false;
    juce::File projectFile;
    void timerCallback() override;
    // With a dialog title, errors also open an OK dialog (Assignment 3, Figure 8).
    void message(const juce::String &, bool error = false, const juce::String &dialogTitle = {});
    void notify(const juce::String &); // status line + dismissible success toast
    void chooseImport();
    void chooseOpen();
    void chooseSave();
    void chooseExport();
    void newProject();
    void confirmDiscard(std::function<void()>);
    void runJob(juce::String, juce::String errorTitle, std::function<juce::Result()>,
                std::function<void()>);
    bool values(std::initializer_list<int>, std::vector<double> &);
    void updateInspector();
};
} // namespace nazal
