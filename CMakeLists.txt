cmake_minimum_required(VERSION 3.22)
project(NazalAudioEditor VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(POLICY CMP0135)
 cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)
set(JUCE_DIR "" CACHE PATH "Optional local JUCE source directory")
if(JUCE_DIR)
 add_subdirectory("${JUCE_DIR}" JUCE)
else()
 FetchContent_Declare(JUCE URL https://codeload.github.com/juce-framework/JUCE/tar.gz/refs/tags/8.0.12
  URL_HASH SHA256=dec1a8baee5aaec4502b717a421b850e257ab7e9c2efd03766185ec1957cf102)
 FetchContent_MakeAvailable(JUCE)
endif()
add_library(nazal_core STATIC src/algorithms/AudioAlgorithms.cpp src/core/Editor.cpp src/system/AudioFiles.cpp src/system/ProjectStore.cpp)
target_include_directories(nazal_core PUBLIC src)
target_compile_definitions(nazal_core PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_MP3AUDIOFORMAT=1 JUCE_MODAL_LOOPS_PERMITTED=0)
target_link_libraries(nazal_core PUBLIC juce::juce_audio_formats)
juce_add_gui_app(NazalEditor PRODUCT_NAME "Nazal Audio Editor" COMPANY_NAME "Team Nazal" BUNDLE_ID "edu.nazal.audioeditor")
target_sources(NazalEditor PRIVATE src/Main.cpp src/ui/MainComponent.cpp src/system/Playback.cpp)
target_link_libraries(NazalEditor PRIVATE nazal_core juce::juce_audio_utils juce::juce_recommended_config_flags)
juce_add_console_app(NazalTests PRODUCT_NAME "Nazal Tests")
target_sources(NazalTests PRIVATE tests/CoreTests.cpp src/system/Playback.cpp)
target_link_libraries(NazalTests PRIVATE nazal_core juce::juce_audio_devices)
enable_testing()
add_test(NAME core_integration COMMAND NazalTests)

set_tests_properties(core_integration PROPERTIES TIMEOUT 30)

# Apple Silicon executables need a valid signature for their complete app bundle.
# Ad-hoc signing is for local builds; it does not provide distribution notarization.
if(APPLE)
 find_program(NAZAL_CODESIGN codesign REQUIRED)
 add_custom_command(TARGET NazalEditor POST_BUILD
  COMMAND "${NAZAL_CODESIGN}" --force --deep --sign - "$<TARGET_BUNDLE_DIR:NazalEditor>"
  VERBATIM)
endif()
