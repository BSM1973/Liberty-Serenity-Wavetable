cmake_minimum_required(VERSION 3.24)
project(SerenityWavetable VERSION 0.1.2 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target")
endif()
include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.10
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
set(SERENITY_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND SERENITY_FORMATS AU)
endif()
juce_add_plugin(SerenityWavetable
  COMPANY_NAME "Liberty Music Software"
  BUNDLE_ID "com.libertymusicsoftware.serenitywavetable"
  PLUGIN_MANUFACTURER_CODE LbMs
  PLUGIN_CODE SrWt
  PRODUCT_NAME "Serenity WAVETABLE"
  FORMATS ${SERENITY_FORMATS}
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD OFF
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE)
juce_generate_juce_header(SerenityWavetable)
target_sources(SerenityWavetable PRIVATE Source/Plugin.cpp)
target_compile_definitions(SerenityWavetable PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SerenityWavetable PRIVATE
  juce::juce_audio_utils juce::juce_audio_processors
  juce::juce_dsp juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags
  juce::juce_recommended_warning_flags)

# The startup regression test constructs the REAL editor used by all formats.
include(CTest)
if (BUILD_TESTING)
  add_executable(serenity_editor_startup_smoke Tests/EditorStartupSmoke.cpp)
  target_include_directories(serenity_editor_startup_smoke PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/SerenityWavetable_artefacts/JuceLibraryCode")
  target_link_libraries(serenity_editor_startup_smoke PRIVATE
    SerenityWavetable juce::juce_audio_utils)
  add_test(NAME serenity_editor_startup_smoke COMMAND serenity_editor_startup_smoke)
  set_tests_properties(serenity_editor_startup_smoke PROPERTIES TIMEOUT 60)
endif()
