#pragma once

#include "juce_audio_processors/juce_audio_processors.h"

#include <Geode/Result.hpp>
#include <filesystem>
#include <memory>
#include <vector>

class PluginManager {
public:
    PluginManager(double sampleRate, int bufferSize);

    void scan();
    geode::Result<juce::AudioPluginInstance*> createPluginInstance(size_t index);
    void removePluginInstance(juce::AudioPluginInstance* plugin);

    void addSearchPath(const std::filesystem::path& path) { m_searchPaths.push_back(path); }
    void removeSearchPath(size_t index) { m_searchPaths.erase(m_searchPaths.begin() + index); }
    const std::vector<std::filesystem::path>& getSearchPaths() const { return m_searchPaths; }
    const std::vector<std::unique_ptr<juce::AudioPluginInstance>>& getPluginInstances() const { return m_plugins; }
    const juce::KnownPluginList& getPluginList() const { return m_list; }

private:
    std::vector<std::filesystem::path> m_searchPaths;
    std::vector<std::unique_ptr<juce::AudioPluginInstance>> m_plugins;
    juce::AudioPluginFormatManager m_formatManager;
    juce::KnownPluginList m_list;
    double m_sampleRate;
    int m_bufferSize;
};