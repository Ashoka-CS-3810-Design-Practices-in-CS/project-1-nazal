#include "MainComponent.h"
#include <cmath>
#include <cstdlib>
namespace nazal {
namespace {
const juce::Colour background{0xff171b22}, panel{0xff222832}, ink{0xffe7ebf0}, accent{0xff75c8c3};
constexpr int headerWidth = 218, rulerHeight = 36, rowHeight = 104, addRowHeight = 52;
constexpr int headerHeight = 118; // title row + transport row + seek bar
#if JUCE_MAC
constexpr int menuBarHeight = 0; // the system menu bar is used on macOS
#else
constexpr int menuBarHeight = 24; // in-window menu bar on Windows/Linux
#endif
juce::String seconds(double time) {
    return juce::String(time, 3);
}
// mm:ss.mmm, as in the transport mock-up (Assignment 3, Figure 3).
juce::String timeText(double time) {
    const auto ms = static_cast<long long>(std::llround(std::max(0.0, time) * 1000.0));
    return juce::String::formatted("%02lld:%02lld.%03lld", ms / 60000, (ms / 1000) % 60, ms % 1000);
}
// Vector icons for the transport and zoom controls; `r` is the square the icon fits in.
void drawIcon(juce::Graphics &g, Icon icon, juce::Rectangle<float> r, juce::Colour colour) {
    g.setColour(colour);
    const float w = r.getWidth(), h = r.getHeight();
    switch (icon) {
    case Icon::play: {
        juce::Path p;
        p.addTriangle(r.getX() + w * 0.12f, r.getY(), r.getX() + w * 0.12f, r.getBottom(),
                      r.getRight(), r.getCentreY());
        g.fillPath(p.createPathWithRoundedCorners(1.5f));
        break;
    }
    case Icon::pause:
        g.fillRoundedRectangle(r.getX() + w * 0.08f, r.getY(), w * 0.30f, h, 1.2f);
        g.fillRoundedRectangle(r.getX() + w * 0.62f, r.getY(), w * 0.30f, h, 1.2f);
        break;
    case Icon::stop:
        g.fillRoundedRectangle(r.reduced(w * 0.06f), 2.0f);
        break;
    case Icon::skipStart: {
        g.fillRoundedRectangle(r.getX(), r.getY(), w * 0.16f, h, 1.0f);
        juce::Path p;
        p.addTriangle(r.getRight(), r.getY(), r.getRight(), r.getBottom(), r.getX() + w * 0.3f,
                      r.getCentreY());
        g.fillPath(p.createPathWithRoundedCorners(1.5f));
        break;
    }
    case Icon::skipEnd: {
        g.fillRoundedRectangle(r.getRight() - w * 0.16f, r.getY(), w * 0.16f, h, 1.0f);
        juce::Path p;
        p.addTriangle(r.getX(), r.getY(), r.getX(), r.getBottom(), r.getRight() - w * 0.3f,
                      r.getCentreY());
        g.fillPath(p.createPathWithRoundedCorners(1.5f));
        break;
    }
    case Icon::search: {
        const float thick = std::max(1.6f, w * 0.12f);
        juce::Rectangle<float> lens(r.getX() + thick * 0.5f, r.getY() + thick * 0.5f,
                                    w * 0.68f - thick, h * 0.68f - thick);
        g.drawEllipse(lens, thick);
        const auto from = lens.getCentre() + juce::Point<float>(1, 1) * (lens.getWidth() * 0.3536f);
        const juce::Point<float> to(r.getRight() - thick * 0.5f, r.getBottom() - thick * 0.5f);
        juce::Path handle;
        handle.startNewSubPath(from);
        handle.lineTo(to);
        g.strokePath(handle, juce::PathStrokeType(thick, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        break;
    }
    }
}
} // namespace
IconButton::IconButton(Icon i, const juce::String &tooltip) : juce::Button(tooltip), icon(i) {
    setTooltip(tooltip);
    setTitle(tooltip);
}
void IconButton::paintButton(juce::Graphics &g, bool highlighted, bool down) {
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    auto fill = panel;
    if (isEnabled())
        fill = down ? panel.darker(0.3f) : highlighted ? panel.brighter(0.15f) : panel;
    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 5);
    g.setColour(juce::Colour(0xff39424f));
    g.drawRoundedRectangle(bounds, 5, 1);
    drawIcon(g, icon, bounds.withSizeKeepingCentre(14, 14),
             isEnabled() ? ink : ink.withAlpha(0.35f));
}
IconView::IconView(Icon i, const juce::String &tooltip) : icon(i) {
    setTooltip(tooltip);
}
void IconView::paint(juce::Graphics &g) {
    drawIcon(g, icon, getLocalBounds().toFloat().withSizeKeepingCentre(18, 18), ink);
}
Toast::Toast() {
    setAccessible(false); // the same text is also written to the status line
}
void Toast::show(const juce::String &message) {
    text = message;
    setVisible(true);
    toFront(false);
    repaint();
    startTimer(3500);
}
void Toast::timerCallback() {
    stopTimer();
    setVisible(false);
}
void Toast::mouseUp(const juce::MouseEvent &e) {
    if (e.position.x > static_cast<float>(getWidth() - 44))
        timerCallback(); // the close (x) area
}
void Toast::paint(juce::Graphics &g) {
    auto r = getLocalBounds().toFloat().reduced(1);
    g.setColour(panel);
    g.fillRoundedRectangle(r, 6);
    g.setColour(juce::Colour(0xff39424f));
    g.drawRoundedRectangle(r, 6, 1);
    juce::Rectangle<float> badge(14, r.getCentreY() - 11, 22, 22);
    g.setColour(accent);
    g.fillEllipse(badge);
    juce::Path tick;
    tick.startNewSubPath(badge.getX() + 6, badge.getCentreY());
    tick.lineTo(badge.getX() + 10, badge.getCentreY() + 4);
    tick.lineTo(badge.getX() + 16.5f, badge.getCentreY() - 4);
    g.setColour(background);
    g.strokePath(tick, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));
    g.setColour(ink);
    g.setFont(juce::Font(juce::FontOptions(15.0f)));
    g.drawText(text, 48, 0, getWidth() - 48 - 44, getHeight(), juce::Justification::centredLeft,
               true);
    juce::Rectangle<float> x(static_cast<float>(getWidth()) - 30.0f, r.getCentreY() - 6, 12, 12);
    g.setColour(ink.withAlpha(0.7f));
    g.drawLine(x.getX(), x.getY(), x.getRight(), x.getBottom(), 1.6f);
    g.drawLine(x.getRight(), x.getY(), x.getX(), x.getBottom(), 1.6f);
}
class TrackHeader : public juce::Component {
  public:
    TrackHeader(MainComponent &main, const Track &track) : owner(main), id(track.id) {
        addMouseListener(this, true);
        addAndMakeVisible(name);
        name.setText(track.name, juce::dontSendNotification);
        name.setEditable(false, true, false);
        name.setColour(juce::Label::textColourId, ink);
        name.onTextChange = [this] {
            owner.changed(owner.editor.renameTrack(id, name.getText()), "Track renamed.", false);
        };
        addAndMakeVisible(volume);
        volume.setRange(0, 2, 0.01);
        volume.setValue(track.volume, juce::dontSendNotification);
        volume.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 20);
        volume.setSliderStyle(juce::Slider::LinearHorizontal);
        volume.setTooltip("Track volume: 1.00 is original volume");
        volume.onValueChange = [this] {
            if (auto *t = owner.editor.track(id)) {
                t->volume = static_cast<float>(volume.getValue());
                owner.changed(juce::Result::ok(), "Track volume updated.", false);
            }
        };
        for (auto *b : std::array<juce::Button *, 3>{&mute, &solo, &remove})
            addAndMakeVisible(*b);
        mute.setToggleState(track.mute, juce::dontSendNotification);
        solo.setToggleState(track.solo, juce::dontSendNotification);
        mute.onClick = [this] {
            if (auto *t = owner.editor.track(id)) {
                t->mute = mute.getToggleState();
                owner.changed(juce::Result::ok(), "Mute updated.", false);
            }
        };
        solo.onClick = [this] {
            if (auto *t = owner.editor.track(id)) {
                t->solo = solo.getToggleState();
                owner.changed(juce::Result::ok(), "Solo updated.", false);
            }
        };
        // Defer deletion: rebuilding rows must not delete a button within its own callback.
        remove.onClick = [this] {
            auto safe = juce::Component::SafePointer<MainComponent>(&owner);
            auto trackId = id;
            juce::MessageManager::callAsync([safe, trackId] {
                if (safe)
                    safe->changed(safe->editor.deleteTrack(trackId), "Track deleted.");
            });
        };
        name.setTooltip("Double-click to rename this track");
        remove.setTooltip("Delete this track and its clips");
    }
    void resized() override {
        name.setBounds(10, 8, 194, 24);
        volume.setBounds(8, 35, 198, 24);
        mute.setBounds(10, 68, 57, 24);
        solo.setBounds(73, 68, 57, 24);
        remove.setBounds(146, 68, 58, 24);
    }
    void paint(juce::Graphics &g) override {
        g.fillAll(panel);
        g.setColour(juce::Colour(0xff39424f));
        g.drawRect(getLocalBounds());
        if (owner.selectedTrack == id) {
            g.setColour(accent);
            g.drawRect(getLocalBounds(), 2);
        }
    }
    void mouseDown(const juce::MouseEvent &) override { owner.select(id); }

  private:
    MainComponent &owner;
    juce::String id;
    juce::Label name;
    juce::Slider volume;
    juce::ToggleButton mute{"Mute"}, solo{"Solo"};
    juce::TextButton remove{"Delete"};
};
class Timeline : public juce::Component {
  public:
    explicit Timeline(MainComponent &main) : owner(main) {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
    double scale = 70;
    void rebuild() {
        headers.clear();
        for (const auto &track : owner.editor.project.tracks) {
            auto h = std::make_unique<TrackHeader>(owner, track);
            addAndMakeVisible(*h);
            headers.push_back(std::move(h));
        }
        layout();
    }
    void layout() {
        auto *view = findParentComponentOfClass<juce::Viewport>();
        int width = view ? view->getWidth() - 16 : 900;
        setSize(
            std::max(width, headerWidth +
                                static_cast<int>(
                                    (std::max(15.0, owner.editor.project.duration() + 3)) * scale)),
            std::max(view ? view->getHeight() - 16 : 300, addRowTop() + addRowHeight));
        for (size_t i = 0; i < headers.size(); ++i)
            headers[i]->setBounds(scrollX(), rulerHeight + static_cast<int>(i) * rowHeight,
                                  headerWidth, rowHeight);
        // "+ Add Track" sits directly below the last track, in the sticky header column.
        owner.addButton.setBounds(scrollX() + 10, addRowTop() + 10, headerWidth - 20, 32);
        repaint();
    }
    int addRowTop() const { return rulerHeight + rowHeight * static_cast<int>(headers.size()); }
    int scrollX() const {
        auto *v = findParentComponentOfClass<juce::Viewport>();
        return v ? v->getViewPositionX() : 0;
    }
    juce::Rectangle<float> bounds(const Clip &clip, int row) const {
        return {static_cast<float>(headerWidth + clip.start * scale),
                static_cast<float>(rulerHeight + row * rowHeight + 8),
                static_cast<float>(std::max(1.0, clip.duration * scale)), 88};
    }
    void paint(juce::Graphics &g) override {
        g.fillAll(background);
        auto visible = g.getClipBounds();
        double step = scale < 35 ? 5 : scale < 80 ? 2 : 1;
        double first =
            std::max(0.0, std::floor((visible.getX() - headerWidth) / scale / step) * step);
        for (double t = first; headerWidth + t * scale < visible.getRight(); t += step) {
            int x = static_cast<int>(headerWidth + t * scale);
            g.setColour(juce::Colour(0xff2c3340));
            g.drawVerticalLine(x, 0, static_cast<float>(getHeight()));
            g.setColour(ink.withAlpha(0.65f));
            g.drawText(juce::String(t, 0) + "s", x + 5, 4, 65, 25, juce::Justification::left);
        }
        for (size_t row = 0; row < owner.editor.project.tracks.size(); ++row) {
            const auto &track = owner.editor.project.tracks[row];
            g.setColour(juce::Colour(0xff303845));
            g.drawHorizontalLine(rulerHeight + static_cast<int>(row + 1) * rowHeight, 0,
                                 static_cast<float>(getWidth()));
            for (const auto &clip : track.clips) {
                auto rect = bounds(clip, static_cast<int>(row));
                if (!rect.intersects(visible.toFloat()))
                    continue;
                auto colour = juce::Colour::fromHSV(static_cast<float>(row % 6) * 0.11f + 0.40f,
                                                    0.40f, 0.74f, 1.0f);
                bool selected = clip.id == owner.selectedClip;
                g.setColour(colour.withAlpha(selected ? 0.37f : 0.20f));
                g.fillRoundedRectangle(rect, 5);
                g.setColour(selected ? accent : colour);
                g.drawRoundedRectangle(rect, 5, selected ? 2.0f : 1.0f);
                if (clip.source && !clip.source->waveform.empty()) {
                    const auto &source = *clip.source;
                    int left = std::max(static_cast<int>(rect.getX() + 2), visible.getX());
                    int right = std::min(static_cast<int>(rect.getRight() - 2), visible.getRight());
                    float center = rect.getY() + 53;
                    g.setColour(colour.brighter(0.3f));
                    for (int x = left; x < right; ++x) {
                        double sourceTime = clip.offset + (x - rect.getX()) / scale;
                        size_t index = std::min(
                            source.waveform.size() - 1,
                            static_cast<size_t>(std::max(0.0, sourceTime / source.duration() *
                                                                  source.waveform.size())));
                        size_t last =
                            std::min(source.waveform.size() - 1,
                                     static_cast<size_t>(std::max(
                                         0.0, (sourceTime + 1.0 / scale) / source.duration() *
                                                  source.waveform.size())));
                        auto peak = source.waveform[index];
                        for (size_t bin = index + 1; bin <= last; ++bin) {
                            peak.minimum = std::min(peak.minimum, source.waveform[bin].minimum);
                            peak.maximum = std::max(peak.maximum, source.waveform[bin].maximum);
                        }
                        g.drawVerticalLine(x, center - peak.maximum * 25,
                                           center - peak.minimum * 25 + 1);
                    }
                    g.setColour(ink);
                    g.drawText(source.file.getFileName(), rect.reduced(8, 4).withHeight(20),
                               juce::Justification::left, true);
                }
                g.setColour(accent.withAlpha(0.7f));
                if (clip.fadeIn > 0)
                    g.drawLine(rect.getX(), rect.getBottom() - 2,
                               rect.getX() + static_cast<float>(clip.fadeIn * scale),
                               rect.getY() + 25, 1);
                if (clip.fadeOut > 0)
                    g.drawLine(rect.getRight() - static_cast<float>(clip.fadeOut * scale),
                               rect.getY() + 25, rect.getRight(), rect.getBottom() - 2, 1);
                g.setColour(ink.withAlpha(0.5f));
                g.drawLine(rect.getX() + 4, rect.getY() + 32, rect.getX() + 4,
                           rect.getBottom() - 8);
                g.drawLine(rect.getRight() - 4, rect.getY() + 32, rect.getRight() - 4,
                           rect.getBottom() - 8);
            }
        }
        float cursor = static_cast<float>(headerWidth + owner.playback.position() * scale);
        g.setColour(accent);
        g.drawLine(cursor, 0, cursor, static_cast<float>(getHeight()), 2);
        g.setColour(panel);
        g.fillRect(scrollX(), 0, headerWidth, rulerHeight);
        g.setColour(ink);
        g.drawText("TRACKS  /  MIXER", scrollX() + 12, 0, headerWidth - 12, rulerHeight,
                   juce::Justification::left);
        juce::Rectangle<int> addCell(scrollX(), addRowTop(), headerWidth, addRowHeight);
        g.setColour(panel);
        g.fillRect(addCell);
        g.setColour(juce::Colour(0xff39424f));
        g.drawRect(addCell);
        if (headers.empty()) {
            g.setColour(ink.withAlpha(0.6f));
            g.drawText("Add a track, then import audio to begin.", headerWidth + 25, 70, 520, 40,
                       juce::Justification::left);
        }
    }
    void mouseDown(const juce::MouseEvent &e) override {
        if (owner.isBusy() || e.x < headerWidth + scrollX())
            return;
        if (e.mods.isPopupMenu()) {
            showClipMenu(e);
            return;
        }
        double t = std::max(0.0, (e.x - headerWidth) / scale);
        if (e.y < rulerHeight) {
            owner.playback.seek(t);
            repaint();
            return;
        }
        int row = (e.y - rulerHeight) / rowHeight;
        if (row < 0 || row >= static_cast<int>(owner.editor.project.tracks.size()))
            return;
        auto &track = owner.editor.project.tracks[static_cast<size_t>(row)];
        for (auto it = track.clips.rbegin(); it != track.clips.rend(); ++it)
            if (bounds(*it, row).contains(e.position)) {
                owner.select(track.id, it->id);
                original = *it;
                dragTrack = track.id;
                dragClip = it->id;
                downX = e.x;
                auto r = bounds(*it, row);
                mode = e.x - r.getX() < 8 ? 1 : r.getRight() - e.x < 8 ? 2 : 3;
                return;
            }
        owner.select(track.id);
        owner.playback.seek(t);
        repaint();
    }
    void mouseDrag(const juce::MouseEvent &e) override {
        if (mode == 0 || owner.isBusy())
            return;
        auto *clip = owner.editor.clip(dragTrack, dragClip);
        if (!clip)
            return;
        double delta = (e.x - downX) / scale;
        juce::Result result = juce::Result::ok();
        if (mode == 3)
            result = owner.editor.move(dragTrack, dragClip, std::max(0.0, original.start + delta));
        else {
            *clip = original;
            double begin = mode == 1
                               ? juce::jlimit(original.start,
                                              original.end() - 1.0 / original.source->sampleRate,
                                              original.start + delta)
                               : original.start;
            double end = mode == 2
                             ? juce::jlimit(original.start + 1.0 / original.source->sampleRate,
                                            original.end(), original.end() + delta)
                             : original.end();
            result = owner.editor.trim(dragTrack, dragClip, begin, end);
        }
        owner.changed(result, mode == 3 ? "Clip moved." : "Clip trimmed.", false);
    }
    void mouseUp(const juce::MouseEvent &) override {
        if (mode) {
            mode = 0;
            layout();
        }
    }

  private:
    // Right-click menu on a clip (Assignment 3, Figure 5). Edits use the playhead position.
    void showClipMenu(const juce::MouseEvent &e) {
        int row = (e.y - rulerHeight) / rowHeight;
        if (e.y < rulerHeight || row < 0 ||
            row >= static_cast<int>(owner.editor.project.tracks.size()))
            return;
        auto &track = owner.editor.project.tracks[static_cast<size_t>(row)];
        for (auto it = track.clips.rbegin(); it != track.clips.rend(); ++it) {
            if (!bounds(*it, row).contains(e.position))
                continue;
            owner.select(track.id, it->id);
            const double t = owner.playback.position();
            const bool inside = t > it->start && t < it->end(); // playhead strictly inside clip
            juce::PopupMenu menu;
            menu.addItem(1, "Split at Playhead", inside);
            menu.addItem(2, "Trim Start to Playhead", inside);
            menu.addItem(3, "Trim End to Playhead", inside);
            menu.addItem(4, "Fade In to Playhead", inside);
            menu.addItem(5, "Fade Out from Playhead", inside);
            menu.addSeparator();
            menu.addItem(6, "Delete");
            menu.showMenuAsync(juce::PopupMenu::Options(),
                               [safe = juce::Component::SafePointer<Timeline>(this),
                                trackId = track.id, clipId = it->id, t](int choice) {
                                   if (safe)
                                       safe->runClipCommand(choice, trackId, clipId, t);
                               });
            return;
        }
    }
    void runClipCommand(int choice, const juce::String &trackId, const juce::String &clipId,
                        double t) {
        auto *clip = owner.editor.clip(trackId, clipId);
        if (!clip || choice == 0)
            return;
        auto &ed = owner.editor;
        switch (choice) {
        case 1:
            owner.changed(ed.split(trackId, clipId, t), "Clip split successfully.");
            break;
        case 2:
            owner.changed(ed.trim(trackId, clipId, t, clip->end()), "Clip start trimmed.");
            break;
        case 3:
            owner.changed(ed.trim(trackId, clipId, clip->start, t), "Clip end trimmed.");
            break;
        case 4:
            owner.changed(ed.fades(trackId, clipId, t - clip->start, clip->fadeOut),
                          "Fade in applied.");
            break;
        case 5:
            owner.changed(ed.fades(trackId, clipId, clip->fadeIn, clip->end() - t),
                          "Fade out applied.");
            break;
        case 6: {
            auto result = ed.remove(trackId, clipId);
            if (result.wasOk())
                owner.selectedClip.clear();
            owner.changed(result, "Clip removed.");
            break;
        }
        default:
            break;
        }
    }
    MainComponent &owner;
    std::vector<std::unique_ptr<TrackHeader>> headers;
    Clip original;
    juce::String dragTrack, dragClip;
    int downX = 0, mode = 0;
};
MainComponent::MainComponent() {
    setSize(1200, 760);
    setWantsKeyboardFocus(true);
    commands.registerAllCommandsForTarget(this);
    commands.setFirstCommandTarget(this);
    addKeyListener(commands.getKeyMappings()); // Ctrl/Cmd + N, O, S, I, E
    setApplicationCommandManagerToWatch(&commands);
    for (auto *button : {&splitButton, &removeButton, &moveButton, &trimButton, &fadeButton}) {
        addAndMakeVisible(*button);
        button->setColour(juce::TextButton::buttonColourId, panel);
        button->setColour(juce::TextButton::textColourOffId, ink);
    }
    for (auto *button : std::array<juce::Button *, 5>{&skipStartButton, &playButton, &pauseButton,
                                                      &stopButton, &skipEndButton})
        addAndMakeVisible(*button);
    addAndMakeVisible(seek);
    seek.setSliderStyle(juce::Slider::LinearHorizontal);
    seek.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    seek.setRange(0, 1, 0.001);
    seek.setTooltip("Seek");
    seek.setTitle("Playback position");
    seek.setColour(juce::Slider::trackColourId, accent);
    seek.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2c3340));
    seek.setColour(juce::Slider::thumbColourId, ink);
    seek.onValueChange = [this] {
        if (updatingSeek)
            return;
        playback.seek(seek.getValue());
        timeline->repaint();
    };
    addChildComponent(toast);
    addAndMakeVisible(zoomIcon);
    zoomIcon.setAccessible(false); // decorative; the slider carries the "Zoom" label
#if !JUCE_MAC
    addAndMakeVisible(menuBar);
#endif
    for (auto *label : {&title, &status, &clock, &selectedLabel}) {
        addAndMakeVisible(*label);
        label->setColour(juce::Label::textColourId, ink);
    }
    title.setFont(juce::Font(juce::FontOptions(22.0f)));
    title.setText("NAZAL  /  AUDIO EDITOR", juce::dontSendNotification);
    addAndMakeVisible(zoom);
    zoom.setTooltip("Zoom");
    zoom.setTitle("Zoom");
    zoom.setRange(20, 200, 1);
    zoom.setValue(70);
    zoom.setSliderStyle(juce::Slider::LinearHorizontal);
    zoom.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    timeline = std::make_unique<Timeline>(*this);
    timeline->addAndMakeVisible(addButton);
    addButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2c3340));
    addButton.setColour(juce::TextButton::textColourOffId, accent);
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(timeline.get(), false);
    zoom.onValueChange = [this] {
        timeline->scale = zoom.getValue();
        timeline->layout();
    };
    const char *labels[] = {"Position (seconds)", "Trim begin (timeline)", "Trim end (timeline)",
                            "Fade in (seconds)", "Fade out (seconds)"};
    for (size_t i = 0; i < fields.size(); ++i) {
        addAndMakeVisible(fieldLabels[i]);
        fieldLabels[i].setText(labels[i], juce::dontSendNotification);
        fieldLabels[i].setColour(juce::Label::textColourId, ink);
        addAndMakeVisible(fields[i]);
        fields[i].setInputRestrictions(20, "0123456789.-");
        fields[i].setColour(juce::TextEditor::backgroundColourId, background);
        fields[i].setColour(juce::TextEditor::textColourId, ink);
    }
    addButton.onClick = [this] {
        selectedTrack = editor.addTrack();
        selectedClip.clear();
        changed(juce::Result::ok(), "Track created successfully.");
    };
    playButton.onClick = [this] {
        if (editor.project.duration() <= 0) {
            message("Import audio before playback.", true, "Playback Error");
            return;
        }
        if (!devices.getCurrentAudioDevice()) {
            message("No audio output device is available.", true, "Playback Error");
            return;
        }
        playback.play();
        message("Playing.");
    };
    pauseButton.onClick = [this] {
        playback.pause();
        message("Paused. Play resumes from this position.");
    };
    stopButton.onClick = [this] {
        playback.stop();
        message("Stopped.");
    };
    // Keep the playhead on screen after jumping to the start or end.
    auto revealPlayhead = [this] {
        int x = static_cast<int>(headerWidth + playback.position() * timeline->scale);
        int left = viewport.getViewPositionX(), visible = viewport.getMaximumVisibleWidth();
        if (x < left + headerWidth || x > left + visible)
            viewport.setViewPosition(std::max(0, x - visible / 2), viewport.getViewPositionY());
        timeline->repaint();
    };
    skipStartButton.onClick = [this, revealPlayhead] {
        playback.seek(0);
        message("Moved to the start.");
        revealPlayhead();
    };
    skipEndButton.onClick = [this, revealPlayhead] {
        playback.seek(editor.project.duration());
        message("Moved to the end.");
        revealPlayhead();
    };
    splitButton.onClick = [this] {
        changed(editor.split(selectedTrack, selectedClip, playback.position()),
                "Clip split successfully.");
    };
    removeButton.onClick = [this] {
        auto result = editor.remove(selectedTrack, selectedClip);
        if (result.wasOk())
            selectedClip.clear();
        changed(result, "Clip removed.");
    };
    moveButton.onClick = [this] {
        std::vector<double> v;
        if (values({0}, v))
            changed(editor.move(selectedTrack, selectedClip, v[0]), "Clip moved.");
    };
    trimButton.onClick = [this] {
        std::vector<double> v;
        if (values({1, 2}, v))
            changed(editor.trim(selectedTrack, selectedClip, v[0], v[1]), "Clip trimmed.");
    };
    fadeButton.onClick = [this] {
        std::vector<double> v;
        if (values({3, 4}, v))
            changed(editor.fades(selectedTrack, selectedClip, v[0], v[1]), "Fades updated.");
    };
    auto error = devices.initialise(0, 2, nullptr, true);
    devices.addAudioCallback(&playback);
    newProject();
    if (error.isNotEmpty())
        message("Audio output could not start: " + error, true);
#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu(this); // Project menu in the system menu bar
#endif
    startTimerHz(30);
}
MainComponent::~MainComponent() {
#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu(nullptr);
#endif
    setApplicationCommandManagerToWatch(nullptr); // before `commands` is destroyed
    commands.setFirstCommandTarget(nullptr);
    stopTimer();
    cancel.store(true);
    if (worker.joinable())
        worker.join();
    devices.removeAudioCallback(&playback);
    viewport.setViewedComponent(nullptr, false);
}
void MainComponent::paint(juce::Graphics &g) {
    g.fillAll(background);
    g.setColour(panel);
    g.fillRect(0, menuBarHeight, getWidth(), headerHeight);
    g.fillRect(0, getHeight() - 155, getWidth(), 155);
}
void MainComponent::resized() {
#if !JUCE_MAC
    menuBar.setBounds(0, 0, getWidth(), menuBarHeight);
#endif
    const int top = menuBarHeight;
    title.setBounds(18, top + 8, 430, 32);
    // Transport row: skip, play, pause, stop, skip; then clip actions, clock and zoom.
    const int row = top + 48;
    int x = 18;
    for (auto *b : std::array<juce::Button *, 5>{&skipStartButton, &playButton, &pauseButton,
                                                 &stopButton, &skipEndButton}) {
        b->setBounds(x, row, 44, 30);
        x += 52;
    }
    x += 8;
    for (auto *b : {&splitButton, &removeButton}) {
        b->setBounds(x, row, b == &splitButton ? 132 : 95, 30);
        x += b->getWidth() + 8;
    }
    clock.setBounds(x + 8, row, 210, 30);
    zoomIcon.setBounds(getWidth() - 194, row, 28, 30);
    zoom.setBounds(getWidth() - 162, row, 142, 30);
    seek.setBounds(18, top + 88, getWidth() - 36, 22);
    viewport.setBounds(12, top + headerHeight + 8, getWidth() - 24,
                       std::max(100, getHeight() - top - headerHeight - 171));
    toast.setBounds(getWidth() - 392, getHeight() - 219, 372, 48);
    int y = getHeight() - 148;
    selectedLabel.setBounds(18, y, getWidth() - 36, 24);
    int cell = (getWidth() - 36) / 5;
    for (size_t i = 0; i < fields.size(); ++i) {
        int fx = 18 + static_cast<int>(i) * cell;
        fieldLabels[i].setBounds(fx, y + 26, cell - 12, 21);
        fields[i].setBounds(fx, y + 49, cell - 14, 27);
    }
    moveButton.setBounds(18, y + 84, cell - 14, 26);
    trimButton.setBounds(18 + cell, y + 84, cell * 2 - 14, 26);
    fadeButton.setBounds(18 + cell * 3, y + 84, cell * 2 - 14, 26);
    status.setBounds(18, getHeight() - 30, getWidth() - 36, 26);
    if (timeline)
        timeline->layout();
}
void MainComponent::message(const juce::String &text, bool error, const juce::String &dialogTitle) {
    status.setColour(juce::Label::textColourId, error ? juce::Colour(0xffffaaa1) : accent);
    status.setText(text, juce::dontSendNotification);
    if (error && dialogTitle.isNotEmpty())
        juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                                         .withIconType(juce::MessageBoxIconType::WarningIcon)
                                         .withTitle(dialogTitle)
                                         .withMessage(text)
                                         .withButton("OK")
                                         .withAssociatedComponent(this),
                                     [](int) {});
}
void MainComponent::notify(const juce::String &text) {
    message(text);
    toast.show(text);
}
void MainComponent::select(const juce::String &track, const juce::String &clip) {
    selectedTrack = track;
    selectedClip = clip;
    updateInspector();
    timeline->repaint();
}
void MainComponent::changed(const juce::Result &result, const juce::String &success, bool rebuild) {
    if (result.failed()) {
        message(result.getErrorMessage(), true, "Invalid Operation");
        return;
    }
    dirty = true;
    playback.setProject(editor.project);
    refresh(rebuild);
    // Drags and sliders update continuously (rebuild == false); only discrete edits get a toast.
    if (rebuild)
        notify(success);
    else
        message(success);
}
void MainComponent::refresh(bool rebuild) {
    if (!editor.track(selectedTrack)) {
        selectedTrack =
            editor.project.tracks.empty() ? juce::String{} : editor.project.tracks.front().id;
        selectedClip.clear();
    }
    if (!editor.clip(selectedTrack, selectedClip))
        selectedClip.clear();
    if (rebuild)
        timeline->rebuild();
    else
        timeline->repaint();
    updateInspector();
    title.setText("NAZAL  /  " + editor.project.name + (dirty ? " *" : ""),
                  juce::dontSendNotification);
}
void MainComponent::updateInspector() {
    auto *clip = editor.clip(selectedTrack, selectedClip);
    auto *track = editor.track(selectedTrack);
    selectedLabel.setText(clip ? "Selected: " + clip->source->file.getFileName() + "  |  " +
                                     track->name
                               : "Select a clip. Drag its body to move; drag either edge to trim. "
                                 "Click the ruler to seek.",
                          juce::dontSendNotification);
    double v[] = {clip ? clip->start : 0, clip ? clip->start : 0, clip ? clip->end() : 0,
                  clip ? clip->fadeIn : 0, clip ? clip->fadeOut : 0};
    for (size_t i = 0; i < fields.size(); ++i) {
        fields[i].setEnabled(clip && !busy);
        if (!fields[i].hasKeyboardFocus(true))
            fields[i].setText(seconds(v[i]), false);
    }
    moveButton.setEnabled(clip && !busy);
    trimButton.setEnabled(clip && !busy);
    fadeButton.setEnabled(clip && !busy);
    splitButton.setEnabled(clip && !busy);
    removeButton.setEnabled(clip && !busy);
}
bool MainComponent::values(std::initializer_list<int> indices, std::vector<double> &out) {
    for (int index : indices) {
        auto text = fields[static_cast<size_t>(index)].getText().trim().toStdString();
        char *end = nullptr;
        double value = std::strtod(text.c_str(), &end);
        if (text.empty() || end != text.c_str() + text.size() || !std::isfinite(value)) {
            message("Enter a valid number of seconds.", true, "Invalid Operation");
            return false;
        }
        out.push_back(value);
    }
    return true;
}
void MainComponent::timerCallback() {
    const double total = editor.project.duration(), now = playback.position();
    clock.setText(timeText(now) + " / " + timeText(total), juce::dontSendNotification);
    clock.setColour(juce::Label::textColourId, playback.isPlaying() ? accent : ink);
    updatingSeek = true; // programmatic updates must not seek
    if (total != seekMax) {
        seekMax = total;
        seek.setRange(0, std::max(total, 0.001), 0.001);
    }
    seek.setEnabled(total > 0 && !busy);
    if (!seek.isMouseButtonDown())
        seek.setValue(std::min(now, std::max(total, 0.001)), juce::dontSendNotification);
    updatingSeek = false;
    timeline->layout();
    auto error = playback.takeError();
    if (error.isNotEmpty())
        message("Playback stopped: " + error, true, "Playback Error");
    if (busy && progress.load() > 0)
        message("Exporting audio... " + juce::String(static_cast<int>(progress.load() * 100)) +
                "%");
}
void MainComponent::newProject() {
    playback.stop();
    editor.project = Project{};
    selectedTrack = editor.addTrack();
    selectedClip.clear();
    projectFile = juce::File{};
    dirty = false;
    playback.setProject(editor.project);
    refresh();
    message("New project created. Import audio to begin.");
}
void MainComponent::confirmDiscard(std::function<void()> action) {
    if (!dirty) {
        action();
        return;
    }
    juce::AlertWindow::showAsync(
        juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Unsaved project")
            .withMessage("Discard your unsaved changes?")
            .withButton("Discard")
            .withButton("Cancel"),
        [safe = juce::Component::SafePointer<MainComponent>(this), action](int result) {
            if (safe && result == 1)
                action();
        });
}
bool MainComponent::canClose() {
    if (busy) {
        message("Please wait for the current operation to finish.", true);
        return false;
    }
    if (!dirty)
        return true;
    confirmDiscard([] { juce::JUCEApplication::getInstance()->quit(); });
    return false;
}
void MainComponent::runJob(juce::String label, juce::String errorTitle,
                           std::function<juce::Result()> work, std::function<void()> success) {
    if (busy)
        return;
    if (worker.joinable())
        worker.join();
    busy = true;
    progress.store(0);
    cancel.store(false);
    message(label);
    commands.commandStatusChanged(); // greys out the Project menu while busy
    timeline->setEnabled(false);     // also disables the Add Track button inside it
    updateInspector();
    auto safe = juce::Component::SafePointer<MainComponent>(this);
    worker = std::thread([safe, errorTitle, work = std::move(work), success = std::move(success)] {
        auto result = juce::Result::ok();
        try {
            result = work();
        } catch (const std::exception &) {
            result = juce::Result::fail(
                "The operation could not finish. Check the file and available memory.");
        }
        juce::MessageManager::callAsync([safe, result, success, errorTitle] {
            if (!safe)
                return;
            safe->busy = false;
            safe->commands.commandStatusChanged();
            safe->timeline->setEnabled(true);
            if (result.wasOk())
                success();
            else
                safe->message(result.getErrorMessage(), true, errorTitle);
            safe->refresh();
        });
    });
}
void MainComponent::chooseImport() {
    if (!editor.track(selectedTrack)) {
        message("Create or select a track before importing audio.", true, "Import Error");
        return;
    }
    chooser = std::make_unique<juce::FileChooser>("Import audio", juce::File{},
                                                  "*.wav;*.aif;*.aiff;*.mp3;*.flac;*.ogg");
    chooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<MainComponent>(this)](const juce::FileChooser &fc) {
            if (!safe || fc.getResult() == juce::File{})
                return;
            auto file = fc.getResult();
            auto source = std::make_shared<std::shared_ptr<const AudioSource>>();
            auto track = safe->selectedTrack;
            double start = safe->playback.position();
            safe->runJob(
                "Importing audio and generating waveform...", "Import Error",
                [safe, file, source] { return safe->audio.load(file, *source); },
                [safe, source, track, start] {
                    safe->changed(safe->editor.addClip(track, *source, start),
                                  "Audio imported successfully.");
                    if (auto *t = safe->editor.track(track))
                        safe->select(track, t->clips.back().id);
                });
        });
}
void MainComponent::chooseOpen() {
    chooser = std::make_unique<juce::FileChooser>("Open Nazal project", juce::File{}, "*.nazal");
    chooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<MainComponent>(this)](const juce::FileChooser &fc) {
            if (!safe || fc.getResult() == juce::File{})
                return;
            safe->openProjectFile(fc.getResult());
        });
}
void MainComponent::openProjectFile(const juce::File &file) {
    auto safe = juce::Component::SafePointer<MainComponent>(this);
    auto project = std::make_shared<Project>();
    runJob(
        "Opening project and referenced audio...", "Open Project Error",
        [safe, file, project] { return loadProject(file, safe->audio, *project); },
        [safe, file, project] {
            safe->playback.stop();
            safe->editor.project = std::move(*project);
            safe->projectFile = file;
            safe->dirty = false;
            safe->selectedTrack.clear();
            safe->selectedClip.clear();
            safe->playback.setProject(safe->editor.project);
            safe->notify("Project reopened successfully.");
        });
}
void MainComponent::chooseSave() {
    chooser = std::make_unique<juce::FileChooser>(
        "Save Nazal project",
        projectFile == juce::File{}
            ? juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile("Untitled.nazal")
            : projectFile,
        "*.nazal");
    chooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::warnAboutOverwriting,
        [safe = juce::Component::SafePointer<MainComponent>(this)](const juce::FileChooser &fc) {
            if (!safe || fc.getResult() == juce::File{})
                return;
            auto file = fc.getResult().withFileExtension("nazal");
            auto project = safe->editor.project;
            project.name = file.getFileNameWithoutExtension();
            safe->runJob(
                "Saving project...", "Save Project Error",
                [project, file] { return saveProject(project, file); },
                [safe, file, project] {
                    safe->projectFile = file;
                    safe->editor.project.name = project.name;
                    safe->dirty = false;
                    safe->notify("Project saved successfully.");
                });
        });
}
void MainComponent::chooseExport() {
    if (editor.project.duration() <= 0) {
        message("Add audio before exporting.", true, "Export Error");
        return;
    }
    auto *dialog = new juce::AlertWindow(
        "Export Audio", "Choose the WAV settings. Export uses the same mix as playback.",
        juce::MessageBoxIconType::NoIcon);
    dialog->addComboBox("rate", {"44100 Hz", "48000 Hz"}, "Sample rate");
    dialog->addComboBox("bits", {"16 bit", "24 bit"}, "Bit depth");
    dialog->addButton("Choose file", 1);
    dialog->addButton("Cancel", 0);
    dialog->enterModalState(
        true,
        juce::ModalCallbackFunction::create([safe =
                                                 juce::Component::SafePointer<MainComponent>(this),
                                             dialog](int answer) {
            if (!safe || answer != 1)
                return;
            double rate =
                dialog->getComboBoxComponent("rate")->getSelectedItemIndex() == 1 ? 48000 : 44100;
            int bits = dialog->getComboBoxComponent("bits")->getSelectedItemIndex() == 1 ? 24 : 16;
            safe->chooser = std::make_unique<juce::FileChooser>(
                "Export mixed audio",
                juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                    .getChildFile("Mix.wav"),
                "*.wav");
            safe->chooser->launchAsync(
                juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
                    juce::FileBrowserComponent::warnAboutOverwriting,
                [safe, rate, bits](const juce::FileChooser &fc) {
                    if (!safe || fc.getResult() == juce::File{})
                        return;
                    auto file = fc.getResult().withFileExtension("wav");
                    auto project = safe->editor.project;
                    safe->runJob(
                        "Exporting audio...", "Export Error",
                        [safe, project, file, rate, bits] {
                            return exportWav(project, file, rate, bits, &safe->progress,
                                             &safe->cancel);
                        },
                        [safe] { safe->notify("Audio exported successfully."); });
                });
        }),
        true);
}
juce::StringArray MainComponent::getMenuBarNames() {
    return {"Project"};
}
juce::PopupMenu MainComponent::getMenuForIndex(int, const juce::String &) {
    juce::PopupMenu menu;
    menu.addCommandItem(&commands, cmdNew);
    menu.addCommandItem(&commands, cmdOpen);
    menu.addCommandItem(&commands, cmdSave);
    menu.addSeparator();
    menu.addCommandItem(&commands, cmdImport);
    menu.addCommandItem(&commands, cmdExport);
    return menu;
}
void MainComponent::menuItemSelected(int, int) {} // command items are routed to perform()
juce::ApplicationCommandTarget *MainComponent::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}
void MainComponent::getAllCommands(juce::Array<juce::CommandID> &ids) {
    for (auto id : {cmdNew, cmdOpen, cmdSave, cmdImport, cmdExport})
        ids.add(id);
}
void MainComponent::getCommandInfo(juce::CommandID id, juce::ApplicationCommandInfo &info) {
    const auto dots = juce::String::fromUTF8("\xE2\x80\xA6");
    const auto cmd = juce::ModifierKeys::commandModifier; // Cmd on macOS, Ctrl elsewhere
    switch (id) {
    case cmdNew:
        info.setInfo("New Project", "Start a new project", "Project", 0);
        info.addDefaultKeypress('n', cmd);
        break;
    case cmdOpen:
        info.setInfo("Open Project" + dots, "Open a saved project", "Project", 0);
        info.addDefaultKeypress('o', cmd);
        break;
    case cmdSave:
        info.setInfo("Save Project" + dots, "Save the project", "Project", 0);
        info.addDefaultKeypress('s', cmd);
        break;
    case cmdImport:
        info.setInfo("Import Audio" + dots, "Add an audio file to the selected track", "Project",
                     0);
        info.addDefaultKeypress('i', cmd);
        break;
    case cmdExport:
        info.setInfo("Export Audio" + dots, "Mix the project to a WAV file", "Project", 0);
        info.addDefaultKeypress('e', cmd);
        break;
    default:
        return;
    }
    info.setActive(!busy);
}
bool MainComponent::perform(const InvocationInfo &info) {
    if (busy)
        return false;
    switch (info.commandID) {
    case cmdNew:
        confirmDiscard([this] { newProject(); });
        return true;
    case cmdOpen:
        confirmDiscard([this] { chooseOpen(); });
        return true;
    case cmdSave:
        chooseSave();
        return true;
    case cmdImport:
        chooseImport();
        return true;
    case cmdExport:
        chooseExport();
        return true;
    default:
        return false;
    }
}
bool MainComponent::keyPressed(const juce::KeyPress &key) {
    if (busy)
        return false;
    if (key.getKeyCode() == juce::KeyPress::spaceKey) {
        if (playback.isPlaying())
            pauseButton.triggerClick();
        else
            playButton.triggerClick();
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::homeKey) {
        skipStartButton.triggerClick();
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::endKey) {
        skipEndButton.triggerClick();
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::deleteKey ||
        key.getKeyCode() == juce::KeyPress::backspaceKey) {
        if (!selectedClip.isEmpty())
            removeButton.triggerClick();
        return true;
    }
    return false;
}
} // namespace nazal
