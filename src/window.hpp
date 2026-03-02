#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

class Window : public juce::Component {
public:
    Window(juce::Component* cmp);
    Window(std::unique_ptr<juce::Component> cmp);
    ~Window() { close(); }

    void resized() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void parentHierarchyChanged() override;

    void setContent(juce::Component* component);
    void setContent(std::unique_ptr<juce::Component> component);
    void close();
    void onClose(std::function<void()> func) { m_onClose = func; }

    juce::Component* getContent() const { return m_component; }
    bool isOwning() const { return m_owning != nullptr; }

private:
    juce::Component* m_component = nullptr;
    juce::Point<int> m_dragStart;
    juce::ShapeButton m_button{"cross", juce::Colours::red, juce::Colours::darkred, juce::Colours::orangered};

    std::unique_ptr<juce::Component> m_owning = nullptr;
    std::function<void()> m_onClose = nullptr;
};