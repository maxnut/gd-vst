#include "window.hpp"
#include "utils.hpp"

Window::Window(juce::Component* cmp) : m_component(cmp) {
    addAndMakeVisible(m_component);
    addAndMakeVisible(m_button);
    juce::Path path;
    path.addStar({0, 0}, 20.f, 1.f, 1.f);
    m_button.setShape(path, false, true, true);
    m_button.setSize(16, 16);
    m_button.onClick = [this]() {
        geode::queueInMainThread([this](){ close(); });
    };
    setSize(m_component->getWidth(), m_component->getHeight() + 24);
}

Window::Window(std::unique_ptr<juce::Component> cmp)
        : m_component(cmp.get()), m_owning(std::move(cmp)) {
    addAndMakeVisible(m_component);
    addAndMakeVisible(m_button);
    juce::Path path;
    path.addStar({0, 0}, 20.f, 1.f, 1.f);
    m_button.setShape(path, false, true, true);
    m_button.setSize(16, 16);
    m_button.onClick = [this]() {
        geode::queueInMainThread([this](){ close(); });
    };
    setSize(m_component->getWidth(), m_component->getHeight() + 24);
}

void Window::resized() {
    m_component->setBounds(0, 24, getWidth(), getHeight() - 24);
    m_button.setBounds(getWidth() - m_button.getWidth() - 4, 4, m_button.getWidth(), m_button.getHeight());
}

void Window::paint(juce::Graphics& g) {
    g.setColour({20, 20, 20});
    g.fillRect(0, 0, getWidth(), 24);
    g.setColour(juce::Colours::white);
    g.drawText(getTitle(), 8, 0, getWidth(), 24, juce::Justification::centredLeft);
}

void Window::mouseDown(const juce::MouseEvent& e) {
    m_dragStart = getPosition();
}

void Window::mouseDrag(const juce::MouseEvent& e) {
    auto delta = e.getOffsetFromDragStart();
    setTopLeftPosition(m_dragStart + delta);
}

void Window::setContent(juce::Component* component) {
    if(m_component) removeChildComponent(m_component);
    m_owning = nullptr;
    m_component = component;
    addAndMakeVisible(m_component);
    setSize(m_component->getWidth(), m_component->getHeight() + 24);
    m_component->setBounds(0, 24, getWidth(), getHeight() - 24);
}

void Window::setContent(std::unique_ptr<juce::Component> component) {
    setContent(component.get());
    m_owning = std::move(component);
}

void Window::close() {
    if(!isOnDesktop()) return;
    removeFromDesktop();
    if(m_onClose) m_onClose();
    juce::Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    juce::Desktop::getInstance().getMainMouseSource().triggerFakeMove();
}

void Window::parentHierarchyChanged() {
    if(!Utils::isOnWine() || !getPeer()) return;
    getPeer()->setCurrentRenderingEngine(0);
}