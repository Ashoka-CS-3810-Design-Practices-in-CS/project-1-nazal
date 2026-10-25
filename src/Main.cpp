#include "ui/MainComponent.h"
class NazalApplication : public juce::JUCEApplication {
  public:
    const juce::String getApplicationName() override { return "Nazal Audio Editor"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    void initialise(const juce::String &arguments) override {
        window = std::make_unique<Window>();
        if (arguments.trim().isNotEmpty())
            window->content->openProjectFile(juce::File(arguments.trim().unquoted()));
    }
    void shutdown() override { window.reset(); }
    void systemRequestedQuit() override {
        if (window && window->content->canClose())
            quit();
    }

  private:
    class Window : public juce::DocumentWindow {
      public:
        Window() : DocumentWindow("Nazal Audio Editor", juce::Colour(0xff171b22), allButtons) {
            setUsingNativeTitleBar(true);
            content = new nazal::MainComponent();
            setContentOwned(content, true);
            setResizable(true, true);
            setResizeLimits(1000, 600, 2400, 1600);
            centreWithSize(1200, 760);
            setVisible(true);
        }
        void closeButtonPressed() override {
            if (content->canClose())
                juce::JUCEApplication::getInstance()->quit();
        }
        nazal::MainComponent *content = nullptr;
    };
    std::unique_ptr<Window> window;
};
START_JUCE_APPLICATION(NazalApplication)
