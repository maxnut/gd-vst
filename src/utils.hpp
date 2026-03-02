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

inline bool isOnWine() {
    HMODULE ntdll = GetModuleHandleW(geode::utils::string::utf8ToWide("ntdll.dll").c_str());
	typedef void (*wine_get_host_version)(const char **sysname, const char **release);
	return (wine_get_host_version)GetProcAddress(ntdll, "wine_get_host_version") != nullptr;
}

inline void* getWindowHandle() {
    if(isExclusiveFullscreen()) return nullptr;
    return WindowFromDC(wglGetCurrentDC());
}

}