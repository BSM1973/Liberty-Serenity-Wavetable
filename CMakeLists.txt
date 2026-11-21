cmake_minimum_required(VERSION 3.22)
project(SerenityWavetable VERSION 0.8.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Force UTF-8 compilation on Windows. Original French labels use UTF-8 literals.
if(MSVC)
  add_compile_options(/utf-8)
endif()

include(CTest)
add_library(serenity_dsp INTERFACE)
target_include_directories(serenity_dsp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP")
if(BUILD_TESTING)
  add_executable(serenity_dsp_tests Tests/dsp_tests.cpp)
  target_link_libraries(serenity_dsp_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_dsp_tests COMMAND serenity_dsp_tests)
  add_executable(serenity_sequence_tests Tests/sequence_tests.cpp)
  target_link_libraries(serenity_sequence_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_sequence_tests COMMAND serenity_sequence_tests)
  add_executable(serenity_custom_wavetable_tests Tests/custom_wavetable_tests.cpp)
  target_link_libraries(serenity_custom_wavetable_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_custom_wavetable_tests COMMAND serenity_custom_wavetable_tests)
  add_executable(serenity_spectral_editor_tests Tests/spectral_editor_tests.cpp)
  target_link_libraries(serenity_spectral_editor_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_spectral_editor_tests COMMAND serenity_spectral_editor_tests)
  add_executable(serenity_bandlimited_wavetable_tests Tests/bandlimited_wavetable_tests.cpp)
  target_link_libraries(serenity_bandlimited_wavetable_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_bandlimited_wavetable_tests COMMAND serenity_bandlimited_wavetable_tests)
  add_executable(serenity_smooth_mip_tests Tests/smooth_mip_tests.cpp)
  target_link_libraries(serenity_smooth_mip_tests PRIVATE serenity_dsp)
  add_test(NAME serenity_smooth_mip_tests COMMAND serenity_smooth_mip_tests)
  add_executable(serenity_render_demo Tests/render_demo.cpp)
  target_link_libraries(serenity_render_demo PRIVATE serenity_dsp)
endif()

option(SERENITY_BUILD_PLUGIN "Build AU/VST3/Standalone JUCE instrument" ON)
if(SERENITY_BUILD_PLUGIN)
  if(DEFINED JUCE_SOURCE_DIR AND EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/juce")
  else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
  endif()

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
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE)
  juce_generate_juce_header(SerenityWavetable)

  # IMPORTANT: the reference PNGs remain in Assets/UI for visual comparison
  # only; they are NOT linked or displayed by the plugin.
  # Every pixel is reconstructed at runtime from native JUCE Graphics.
  target_sources(SerenityWavetable PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SerenityNativeUI.cpp)
  target_include_directories(SerenityWavetable PRIVATE Source Source/DSP)
  target_compile_definitions(SerenityWavetable PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(SerenityWavetable PRIVATE
    serenity_dsp
    juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

  if(BUILD_TESTING)
    add_executable(serenity_editor_startup_smoke Tests/EditorStartupSmoke.cpp)
    target_include_directories(serenity_editor_startup_smoke PRIVATE
      Source "${CMAKE_CURRENT_BINARY_DIR}/SerenityWavetable_artefacts/JuceLibraryCode")
    target_link_libraries(serenity_editor_startup_smoke PRIVATE SerenityWavetable juce::juce_audio_utils)
    add_test(NAME serenity_editor_startup_smoke COMMAND serenity_editor_startup_smoke)
    set_tests_properties(serenity_editor_startup_smoke PROPERTIES TIMEOUT 60)
  endif()
endif()
