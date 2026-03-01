#include "plugin_selector.hpp"
#include "Geode/utils/file.hpp"
#include "utils.hpp"

PluginEntry::PluginEntry(juce::PluginDescription plugin, size_t idx, std::function<void(size_t)> callback) : m_plugin(plugin), m_idx(idx), m_callback(callback) {
    m_icon.setTooltip(plugin.descriptiveName + " (" + plugin.version + ") - " + plugin.manufacturerName);
    addAndMakeVisible(m_icon);
}

void PluginEntry::paint(juce::Graphics& g) {
    g.fillAll(m_idx % 2 == 0 ? juce::Colours::lightblue : juce::Colours::lightcyan);
    g.drawText(m_plugin.name, 8, 0, getWidth(), getHeight(), juce::Justification::left);
}

void PluginEntry::resized() {
    m_icon.setBounds(getWidth() - 16, 4, 16, 16);
}

void PluginEntry::mouseDown(const juce::MouseEvent& e) {
    if(!e.mouseWasClicked() || e.getNumberOfClicks() != 2) return;
    m_callback(m_idx);
}

Pluginselector::Pluginselector(PluginManager* manager, std::function<void(size_t)> callback) : m_manager(manager), m_callback(callback) {
    addAndMakeVisible(m_viewport);
    m_viewport.setViewedComponent(&m_dummy, false);
    m_viewport.setScrollBarPosition(true, false);
    m_viewport.setScrollBarsShown(true, false, true, false);
    m_viewport.setScrollBarThickness(6);
    m_viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::nonHover);
    addAndMakeVisible(m_menuBar);
    setup();
}

void Pluginselector::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

void Pluginselector::resized() {
    m_menuBar.setBounds(0, 0, getWidth(), 24);
    m_viewport.setBounds(0, 24, getWidth(), getHeight() - 24);
    m_dummy.setBounds(0, 24, getWidth() - (m_viewport.canScrollVertically() ? 6 : 0), 24 * m_entries.size());
    juce::Rectangle<int> area = m_dummy.getLocalBounds();
    for(auto& entry : m_entries) {
        entry->setBounds(area.removeFromTop(24));
    }
}

void Pluginselector::setup() {
    for(auto& entry : m_entries) m_dummy.removeChildComponent(entry.get());
    m_entries.clear();

    for(size_t i = 0; i < m_manager->getPluginList().getNumTypes(); i++) {
        const auto& desc = m_manager->getPluginList().getTypes()[i];
        auto entry = std::make_unique<PluginEntry>(desc, i, m_callback);
        m_dummy.addAndMakeVisible(entry.get());
        m_entries.push_back(std::move(entry));
    }
    resized();
}

juce::StringArray SelectorMenuModel::getMenuBarNames() {
    return { "File" };
}

juce::PopupMenu SelectorMenuModel::getMenuForIndex(int index, const juce::String&) {
    juce::PopupMenu menu;
    if(index == 0) {
        menu.addItem(1, "Scan");
        menu.addItem(2, "Open Plugin Folder");
    }
    return menu;
}

void SelectorMenuModel::menuItemSelected(int menuItemID, int) {
    if(menuItemID == 1) {
        m_selector->m_manager->scan();
        m_selector->setup();
    }
    else if(menuItemID == 2) {
        geode::utils::file::openFolder(Utils::getPluginDir());
    }
}