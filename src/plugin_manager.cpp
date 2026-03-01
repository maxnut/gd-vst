#include "plugin_manager.hpp"

PluginManager::PluginManager(double sampleRate, int bufferSize) : m_sampleRate(sampleRate), m_bufferSize(bufferSize) {
    juce::addDefaultFormatsToManager(m_formatManager);
}

void PluginManager::scan() {
    m_list.clear();
    for(auto& path : m_searchPaths) {
        for(auto& format : m_formatManager.getFormats()) {
            juce::PluginDirectoryScanner scanner(
                m_list,
                *format,
                juce::FileSearchPath(path.string()),
                true,
                juce::File(),
                true
            );

            juce::String name;
            while (scanner.scanNextFile(true, name)) {}

            for(auto& fail : scanner.getFailedFiles()) {
                geode::log::warn("Failed to open plugin: {}", fail.toStdString());
            }
        }
    }
}

geode::Result<juce::AudioPluginInstance*> PluginManager::createPluginInstance(size_t index) {
    if(m_list.getNumTypes() <= index) {
        return geode::Err("Invalid index");
    }

    juce::String error;
    auto plugin = m_formatManager.createPluginInstance(
        m_list.getTypes()[index],
        m_sampleRate,
        m_bufferSize,
        error
    );

    if(!plugin) return geode::Err(error.toStdString());

    int sampleRate = 0;
    int numBuffers = 0;
    unsigned int bufferLength = 0;
    auto system = FMODAudioEngine::sharedEngine()->m_system;
    system->getSoftwareFormat(&sampleRate, nullptr, nullptr);
    system->getDSPBufferSize(&bufferLength, &numBuffers);
    plugin->setRateAndBufferSizeDetails((double)sampleRate, bufferLength);
    plugin->prepareToPlay((double)sampleRate, bufferLength);

    auto ok = geode::Ok(plugin.get());
    m_plugins.push_back(std::move(plugin));
    return ok;
}