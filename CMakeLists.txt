cmake_minimum_required(VERSION 3.22)
project(HonestMixHeadphone VERSION 0.6.12 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(HM_DEFAULT_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/deps/juce")
set(HM_JUCE "${HM_DEFAULT_JUCE}" CACHE PATH "Existing JUCE checkout")
if(NOT EXISTS "${HM_JUCE}/CMakeLists.txt")
 message(FATAL_ERROR "Set HM_JUCE to an existing JUCE 7 source checkout; no network download is performed.")
endif()
add_subdirectory("${HM_JUCE}" juce EXCLUDE_FROM_ALL)
juce_add_plugin(HonestMixHeadphone
 COMPANY_NAME HONESTMIX BUNDLE_ID cn.honestmix.headphone
 PLUGIN_MANUFACTURER_CODE HnMx PLUGIN_CODE HmH6
 PRODUCT_NAME "HONESTMIX Headphone" VERSION ${PROJECT_VERSION}
 FORMATS VST3 Standalone IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS TRUE VST3_CATEGORIES Fx EQ
 COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(HonestMixHeadphone)
target_sources(HonestMixHeadphone PRIVATE
 Source/PluginProcessor.cpp Source/PluginEditor.cpp
 Source/UI/Skin.cpp Source/UI/MeasuredHeadphone.cpp
 Source/Core/AppState.cpp Source/Core/HeadphoneDatabase.cpp
 Source/dsp/CorrectionEngine.cpp Source/dsp/WetDryMixer.cpp)
target_include_directories(HonestMixHeadphone PUBLIC Source "${CMAKE_CURRENT_BINARY_DIR}/HonestMixHeadphone_artefacts/JuceLibraryCode")
target_compile_definitions(HonestMixHeadphone PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0 JUCE_PLUGINHOST_VST3=1 NOMINMAX)
target_compile_options(HonestMixHeadphone PUBLIC /utf-8)
target_link_libraries(HonestMixHeadphone PUBLIC juce::juce_audio_utils juce::juce_dsp)
juce_add_console_app(HeadphoneTests PRODUCT_NAME "Headphone DSP Tests")
target_sources(HeadphoneTests PRIVATE tests/HeadphoneTests.cpp)
target_link_libraries(HeadphoneTests PRIVATE HonestMixHeadphone)
# 009 B 合入：独立保真度检查目标（来自 fidelity-fix，绝对幅度/相位硬门槛，不做 1k 归一化）
juce_add_console_app(FidelityCheck PRODUCT_NAME "Headphone Fidelity Check")
target_sources(FidelityCheck PRIVATE tests/FidelityCheck.cpp)
target_link_libraries(FidelityCheck PRIVATE HonestMixHeadphone)
juce_add_console_app(ResponseContractTests PRODUCT_NAME "Headphone Response Contract Tests")
target_sources(ResponseContractTests PRIVATE tests/ResponseContractTests.cpp)
target_link_libraries(ResponseContractTests PRIVATE HonestMixHeadphone)

juce_add_console_app(TempoRecallTests PRODUCT_NAME "Headphone Tempo Recall Tests")
target_sources(TempoRecallTests PRIVATE tests/TempoRecallTests.cpp)
target_link_libraries(TempoRecallTests PRIVATE HonestMixHeadphone)

# This JUCE revision does not list Info.txt in the generated resource edge.
# A version change must regenerate every PE resource without reusing old versions.
if(WIN32)
 foreach(hm_resource_target HonestMixHeadphone HeadphoneTests FidelityCheck ResponseContractTests TempoRecallTests)
  set(hm_resource_dir "${CMAKE_CURRENT_BINARY_DIR}/${hm_resource_target}_artefacts/JuceLibraryCode")
  add_custom_command(OUTPUT "${hm_resource_dir}/${hm_resource_target}_resources.rc"
   APPEND DEPENDS "${hm_resource_dir}/Info.txt")
 endforeach()
endif()


# Optional focused peak telemetry/UI regression; the four required QA suites remain unchanged.
option(HM_BUILD_PEAK_PROBE "Build focused offline output peak regression" OFF)
if(HM_BUILD_PEAK_PROBE)
 juce_add_console_app(PeakTelemetryProbe PRODUCT_NAME "Headphone Peak Telemetry Probe")
 target_sources(PeakTelemetryProbe PRIVATE tests/PeakTelemetryProbe.cpp)
 target_link_libraries(PeakTelemetryProbe PRIVATE HonestMixHeadphone)
 if(WIN32)
  set(hm_peak_resource_dir "${CMAKE_CURRENT_BINARY_DIR}/PeakTelemetryProbe_artefacts/JuceLibraryCode")
  add_custom_command(OUTPUT "${hm_peak_resource_dir}/PeakTelemetryProbe_resources.rc"
   APPEND DEPENDS "${hm_peak_resource_dir}/Info.txt")
 endif()
endif()

option(HM_BUILD_UPDATE_PROBES "Build offline website-update probes" OFF)
if(HM_BUILD_UPDATE_PROBES)
 include(tests/UpdateProbes.cmake)
endif()

include(tests/StateRestoreProbe.cmake)
