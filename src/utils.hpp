#pragma once

#include "Geode/cocos/platform/win32/CCEGLView.h"

#include <filesystem>

#include <winuser.h>

namespace Utils {

inline std::filesystem::path getPluginDir() {
    return geode::Mod::get()->getSaveDir() / "plugins";
}

inline bool isExclusiveFullscreen() {
    auto view = cocos2d::CCEGLView::get();
    return view->getIsFullscreen() && !view->getIsBorderless();
}

inline void* getWindowHandle() {
    if(isExclusiveFullscreen()) return nullptr;
    return WindowFromDC(wglGetCurrentDC());
}

}