#pragma once

#include "window.hpp"
#include "help_icon.hpp"
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"

#include <memory>
#include <string>
#include <unordered_map>

class PluginManager;
class ActivePluginManager;

class ActivePluginEntry : public juce::Component {
public:
    ActivePluginEntry(ActivePluginManager* manager, juce::AudioPluginInstance* plugin, size_t m_idx);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    juce::AudioPluginInstance* getPlugin() const { return m_plugin; }
    bool isEnabled() const { return m_enabled; }

private:
    std::unique_ptr<Window> m_pluginWindow = nullptr;
    ActivePluginManager* m_manager = nullptr;
    juce::AudioPluginInstance* m_plugin = nullptr;
    size_t m_idx;
    bool m_enabled = true;
    
    HelpIcon m_icon;
    juce::ToggleButton m_enableButton;

friend class ActivePluginManager;
};

class ActivePluginManager : public juce::Component {
public:
    ActivePluginManager(PluginManager* manager);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    void newPlugin(size_t idx);
    void removePlugin(size_t idx);
    void movePlugin(size_t idx, bool up);

    void closeAll();

    juce::AudioPluginInstance* getPlugin(size_t idx);
    const std::atomic<std::shared_ptr<std::vector<std::shared_ptr<ActivePluginEntry>>>>& getActivePlugins() { return m_activePlugins; }

private:
    std::atomic<std::shared_ptr<std::vector<std::shared_ptr<ActivePluginEntry>>>> m_activePlugins;
    std::unique_ptr<Window> m_selectorWindow = nullptr;
    juce::Viewport m_viewport;
    juce::Component m_dummy;

    PluginManager* m_manager = nullptr;
};