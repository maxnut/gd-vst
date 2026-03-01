#include "active_plugin_manager.hpp"
#include "plugin_manager.hpp"
#include "plugin_selector.hpp"
#include "utils.hpp"

#include <algorithm>
#include <utility>

static juce::AudioProcessorEditor* getEditor(juce::AudioPluginInstance* plugin) {
    if(plugin->getActiveEditor()) return plugin->getActiveEditor();
    return plugin->createEditorIfNeeded();
}

ActivePluginEntry::ActivePluginEntry(ActivePluginManager* manager, juce::AudioPluginInstance* plugin, size_t idx)
        : m_manager(manager), m_plugin(plugin), m_idx(idx) {
    auto desc = plugin->getPluginDescription();
    m_icon.setTooltip(desc.descriptiveName + " (" + desc.version + ") - " + desc.manufacturerName);
    m_enableButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    m_enableButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    m_enableButton.setToggleState(true, juce::NotificationType::sendNotification);
    m_enableButton.onClick = [&]() {
        m_enabled = m_enableButton.getToggleState();
    };
    addAndMakeVisible(m_icon);
    addAndMakeVisible(m_enableButton);
}

void ActivePluginEntry::paint(juce::Graphics& g) {
    g.fillAll(m_idx % 2 == 0 ? juce::Colours::lightblue : juce::Colours::lightcyan);
    g.drawText(m_plugin->getName(), 8, 0, getWidth(), getHeight(), juce::Justification::left);
}

void ActivePluginEntry::resized() {
    m_icon.setBounds(getWidth() - 16, 4, 16, 16);
    m_enableButton.setBounds(getWidth() - 48, 0, 24, 24);
}

void ActivePluginEntry::mouseDown(const juce::MouseEvent& e) {
    if(e.mouseWasClicked() && e.mods.isRightButtonDown()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Remove Plugin");
        if(m_manager->getActivePlugins().load()->size() > m_idx - 1) {
            menu.addItem(2, "Move Up");
        }
        if(m_manager->getActivePlugins().load()->size() > m_idx + 1) {
            menu.addItem(3, "Move Down");
        }

        menu.showMenuAsync(juce::PopupMenu::Options(), [&](int result) {
            if(result == 0) return;
            if(result == 1) m_manager->removePlugin(m_idx);
            else if(result == 2) m_manager->movePlugin(m_idx, true);
            else if(result == 3) m_manager->movePlugin(m_idx, false);
        });
    }
    else if(e.mouseWasClicked() && e.getNumberOfClicks() == 2) {
        if(!m_pluginWindow) {
            m_pluginWindow = std::make_unique<Window>(getEditor(m_plugin));
            m_pluginWindow->setTitle(m_plugin->getName());
            m_pluginWindow->setVisible(true);
            m_pluginWindow->setOpaque(true);
            m_pluginWindow->onClose([&]() {
                auto* editor = m_plugin->getActiveEditor();
                if (editor) {
                    m_plugin->editorBeingDeleted(editor);
                    delete editor;
                }
            });
        }
        else if(m_pluginWindow->isOnDesktop()) return;
        else m_pluginWindow->setContent(getEditor(m_plugin));

        m_pluginWindow->addToDesktop(Utils::isExclusiveFullscreen() ? juce::ComponentPeer::windowAppearsOnTaskbar : 0, Utils::getWindowHandle());
        m_pluginWindow->toFront(true);
    }
}

ActivePluginManager::ActivePluginManager(PluginManager* manager) : m_manager(manager) {
    m_activePlugins.store(std::make_shared<std::vector<std::shared_ptr<ActivePluginEntry>>>());
    addAndMakeVisible(m_viewport);
    m_viewport.setViewedComponent(&m_dummy, false);
    m_viewport.setScrollBarPosition(true, false);
    m_viewport.setScrollBarsShown(true, false, true, false);
    m_viewport.setScrollBarThickness(6);
    m_viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::nonHover);
}

void ActivePluginManager::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
    if(m_activePlugins.load()->empty()) {
        g.drawFittedText("No Active Plugins. Right click to add a Plugin", getLocalBounds(), juce::Justification::centred, 4);
    }
}

void ActivePluginManager::mouseDown(const juce::MouseEvent& e) {
    if(!e.mouseWasClicked() || !e.mods.isRightButtonDown()) return;

    juce::PopupMenu menu;
    menu.addItem(1, "Add Plugin");

    menu.showMenuAsync(juce::PopupMenu::Options(), [&](int result) {
        if(result == 0) return;
        
        if(!m_selectorWindow) {
            auto sel = std::make_unique<PluginSelector>(m_manager, [&](size_t idx) {
                newPlugin(idx);
                m_selectorWindow->close();
                toFront(true);
            });
            sel->setSize(getWidth() / 1.5f, getHeight() / 1.5f);
            m_selectorWindow = std::make_unique<Window>(std::move(sel));
            m_selectorWindow->setTitle("Plugin Selector");
            m_selectorWindow->setVisible(true);
            m_selectorWindow->setOpaque(true);
        }
        else if(m_selectorWindow->isOnDesktop()) return;
        m_selectorWindow->addToDesktop(Utils::isExclusiveFullscreen() ? juce::ComponentPeer::windowAppearsOnTaskbar : 0, Utils::getWindowHandle());
        m_selectorWindow->toFront(true);
    });
}

void ActivePluginManager::newPlugin(size_t idx) {
    auto plugin = getPlugin(idx);
    if(!plugin) return;

    auto oldList = m_activePlugins.load(std::memory_order_acquire);

    auto comp = std::make_shared<ActivePluginEntry>(this, plugin, oldList->size());
    addAndMakeVisible(comp.get());

    comp->setBounds(0, 24 * oldList->size(), m_dummy.getWidth(), 24);
    auto newList = std::make_shared<std::vector<std::shared_ptr<ActivePluginEntry>>>(*oldList);
    newList->push_back(comp);
    m_activePlugins.store(newList, std::memory_order_release);
    repaint();
}

void ActivePluginManager::removePlugin(size_t idx) {
    auto oldList = m_activePlugins.load(std::memory_order_acquire);
    if(oldList->size() <= idx) return;
    auto newList = std::make_shared<std::vector<std::shared_ptr<ActivePluginEntry>>>(*oldList);
    newList->erase(newList->begin()+idx);
    m_activePlugins.store(newList, std::memory_order_release);
    removeChildComponent(oldList->at(idx).get());
    repaint();
}

void ActivePluginManager::movePlugin(size_t idx, bool up) {
    auto oldList = m_activePlugins.load(std::memory_order_acquire);
    if(oldList->size() <= idx || oldList->size() <= idx + (up ? -1 : 1)) return;
    auto newList = std::make_shared<std::vector<std::shared_ptr<ActivePluginEntry>>>(*oldList);

    auto first = newList->at(idx);
    auto second = newList->at(idx + (up ? -1 : 1));
    first->setTopLeftPosition(first->getX(), first->getY() + (up ? -24 : 24));
    second->setTopLeftPosition(second->getX(), second->getY() + (up ? 24 : -24));
    std::swap(first->m_idx, second->m_idx);
    std::iter_swap(newList->begin() + idx, newList->begin() + idx + (up ? -1 : 1));
    m_activePlugins.store(newList, std::memory_order_release);
    repaint();
}

juce::AudioPluginInstance* ActivePluginManager::getPlugin(size_t idx) {
    const auto& desc = m_manager->getPluginList().getTypes()[idx];
    auto res = m_manager->createPluginInstance(idx);
    if(!res) {
        geode::log::error("{}", res.unwrapErr());
        return nullptr;
    }
    return res.unwrap();
}

void ActivePluginManager::resized() {
    auto list = m_activePlugins.load(std::memory_order_acquire);
    m_viewport.setBounds(getLocalBounds());
    m_dummy.setBounds(0, 0, getWidth() - (m_viewport.canScrollVertically() ? 6 : 0), 24 * list->size());
    juce::Rectangle<int> area = m_dummy.getLocalBounds();
    for(auto& entry : *list) {
        entry->setBounds(area.removeFromTop(24));
    }
}

void ActivePluginManager::closeAll() {
    if(m_selectorWindow) m_selectorWindow->close();
    auto list = m_activePlugins.load(std::memory_order_acquire);
    for(auto& entry : *list) {
        if(entry->m_pluginWindow) entry->m_pluginWindow->close();
    }
}