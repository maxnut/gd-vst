cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_subdirectory(JUCE)

project(gdvst VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_utils
    ole32
    oleaut32
    uuid
    winmm
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_PLUGINHOST_VST=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_LV2=0
    JUCE_PLUGINHOST_AU=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_BUILD_EXAMPLES=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WINSOCK_DEPRECATED_NO_WARNINGS
    STRICT
    _WIN2_WINNT
    _UNICODE
    UNICODE
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
