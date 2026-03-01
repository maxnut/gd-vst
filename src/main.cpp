#include "active_plugin_manager.hpp"
#include "plugin_manager.hpp"
#include "utils.hpp"

#include "fmod.hpp"
#include "juce_events/juce_events.h"

#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/AppDelegate.hpp>

#include <filesystem>
#include <mutex>

using namespace geode::prelude;

std::unique_ptr<PluginManager> manager = nullptr;
std::unique_ptr<ActivePluginManager> activeManager = nullptr;
std::unique_ptr<Window> activeManagerWindow = nullptr;
std::unique_ptr<juce::TooltipWindow> tooltipWindow = nullptr;
juce::AudioBuffer<float> buffer;
bool hasJuce = false;
std::mutex lock;

void showActiveManagerWindow() {
    if (!activeManagerWindow) {
        activeManagerWindow = std::make_unique<Window>(activeManager.get());
        activeManagerWindow->setTitle("Plugin Manager");
        activeManagerWindow->setVisible(true);
        activeManagerWindow->setOpaque(true);
        activeManagerWindow->setSize(activeManager->getWidth(), activeManager->getHeight());
        activeManagerWindow->onClose([]() {
            activeManager->closeAll();
        });
    }
    activeManagerWindow->addToDesktop(Utils::isExclusiveFullscreen() ? juce::ComponentPeer::windowAppearsOnTaskbar : 0, Utils::getWindowHandle());
    activeManagerWindow->toFront(true);
}

FMOD_RESULT F_CALLBACK PluginDSPCallback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length,
        int in_channels, int* out_channels) {
    std::lock_guard<std::mutex> guard(lock);
    
    *out_channels = in_channels;

    if(!activeManager) return FMOD_OK;
    auto plugins = activeManager->getActivePlugins().load(std::memory_order_acquire);
    if(plugins->empty()) return FMOD_OK;

    buffer.clear();
    for(int ch = 0; ch < in_channels; ++ch) {
        for(int i = 0; i < length; ++i) {
            buffer.setSample(ch, i, inbuffer[i * in_channels + ch]);
        }
    }

    juce::MidiBuffer midi;
    for(auto& entry : *plugins) {
        if(!entry->isEnabled()) continue;
        entry->getPlugin()->processBlock(buffer, midi);
    }

    for(int ch = 0; ch < in_channels; ++ch) {
        for(int i = 0; i < length; ++i) {
            outbuffer[i * in_channels + ch] = buffer.getSample(ch, i);
        }
    }

    return FMOD_OK;
}

void init() {
    juce::initialiseJuce_GUI();

    int sampleRate = 0;
    int numBuffers = 0;
    int numChannels = 0;
    unsigned int bufferLength = 0;
    auto system = FMODAudioEngine::sharedEngine()->m_system;
    system->getSoftwareFormat(&sampleRate, nullptr, nullptr);
    system->getSoftwareChannels(&numChannels);
    system->getDSPBufferSize(&bufferLength, &numBuffers);
    buffer = juce::AudioBuffer<float>(numChannels, bufferLength);

    manager = std::make_unique<PluginManager>((double)sampleRate, bufferLength);
    manager->addSearchPath(Utils::getPluginDir());
    manager->addSearchPath(Mod::get()->getResourcesDir());
    manager->scan();
    
    std::lock_guard<std::mutex> guard(lock);
    tooltipWindow = std::make_unique<juce::TooltipWindow>();
    activeManager = std::make_unique<ActivePluginManager>(manager.get());
    activeManager->setSize(400, 300);

    hasJuce = true;
}

void shutdown() {
    std::lock_guard<std::mutex> guard(lock);
    if(tooltipWindow) {
        if(tooltipWindow->isOnDesktop()) tooltipWindow->removeFromDesktop();
    }
    activeManagerWindow = nullptr;
    activeManager = nullptr;
    manager = nullptr;
    juce::shutdownJuce_GUI();
    hasJuce = false;
}

class $modify(PluginMenuLayer, MenuLayer) {
    static cocos2d::CCScene* scene(bool ivoo) {
        auto scene = MenuLayer::scene(ivoo);
        auto view = cocos2d::CCEGLView::get();
        if(!hasJuce) ::init();
        return scene;
    }
};

class $modify(PluginView, cocos2d::CCEGLView) {
    void pollEvents() {
        cocos2d::CCEGLView::pollEvents();
        if(hasJuce) juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
    }

    void toggleFullScreen(bool fullscreen, bool borderless, bool fix) {
        if(hasJuce) shutdown();
        cocos2d::CCEGLView::toggleFullScreen(fullscreen, borderless, fix);
    }
};

class $modify(CCTouchDispatcher) {
	void touches(CCSet* touches, CCEvent* event, unsigned int type) {
		if (!hasJuce || !touches)
			return CCTouchDispatcher::touches(touches, event, type);

		auto* touch = static_cast<CCTouch*>(touches->anyObject());
		if (!touch) return CCTouchDispatcher::touches(touches, event, type);
		if (juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse()) return;
        
        CCTouchDispatcher::touches(touches, event, type);
	}
};

class $modify(AppDelegate) {
    void applicationDidEnterBackground() {
        if(activeManagerWindow && activeManagerWindow->isOnDesktop()) return;
        AppDelegate::applicationDidEnterBackground();
    }
};

$on_mod(Loaded) {
    auto system = FMODAudioEngine::sharedEngine()->m_system;

    FMOD_DSP_DESCRIPTION dspDesc = {};
    strcpy_s(dspDesc.name, "PluginDSP");
    dspDesc.version = 0x00010000;
    dspDesc.numinputbuffers = 1;
    dspDesc.numoutputbuffers = 1;
    dspDesc.read = PluginDSPCallback;

    FMOD::DSP* dsp = nullptr;
    system->createDSP(&dspDesc, &dsp);

    FMOD::ChannelGroup* group;
    system->getMasterChannelGroup(&group);
    group->addDSP(0, dsp);

    listenForKeybindSettingPresses("keybind", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        if (!down || repeat) return;

        if(!activeManagerWindow || !activeManagerWindow->isOnDesktop()) {
            showActiveManagerWindow();
            return;
        }

        activeManagerWindow->close();
    });

    std::filesystem::create_directories(Utils::getPluginDir());
}

$on_mod(DataSaved) {
    if(hasJuce) shutdown();
}