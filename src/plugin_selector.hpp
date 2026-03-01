#pragma once

#include "help_icon.hpp"
#include "juce_gui_basics/juce_gui_basics.h"
#include "plugin_manager.hpp"

class PluginSelector;

class SelectorMenuModel : public juce::MenuBarModel {
public:
    SelectorMenuModel(PluginSelector* selector) : m_selector(selector) {}

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int index, const juce::String&) override;
    void menuItemSelected(int menuItemID, int) override;

private:
    PluginSelector* m_selector = nullptr;
};

class PluginEntry : public juce::Component {
public:
    PluginEntry(juce::PluginDescription plugin, size_t idx, std::function<void(size_t)> callback);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    size_t m_idx;
    juce::PluginDescription m_plugin;
    std::function<void(size_t)> m_callback = nullptr;
    
    HelpIcon m_icon;
};

class PluginSelector : public juce::Component {
public:
    PluginSelector(PluginManager* manager, std::function<void(size_t)> callback);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setup();

private:
    PluginManager* m_manager = nullptr;
    std::function<void(size_t)> m_callback = nullptr;
    std::vector<std::unique_ptr<PluginEntry>> m_entries;
    juce::Viewport m_viewport;
    juce::Component m_dummy;
    SelectorMenuModel m_model{this};
    juce::MenuBarComponent m_menuBar{&m_model};

friend class SelectorMenuModel;
};