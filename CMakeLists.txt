cmake_minimum_required(VERSION 3.22)
project(HonestMixBETA2 VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ═══════════════════════════════════════════════════════════
# 编译选项
# ═══════════════════════════════════════════════════════════
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt)
    message(FATAL_ERROR "third_party/JUCE 缺失: 请重建目录联接")
endif()
add_subdirectory(third_party/JUCE)

juce_add_plugin(BETA2
    PLUGIN_MANUFACTURER_CODE  Hnst
    PLUGIN_CODE               Bet2
    BUNDLE_ID                 com.solarterra.HonestMix.BETA2
    FORMATS                   VST3 Standalone
    COMPANY_NAME              "SOLAR&TERRA DESIGN STUDIO"
    COMPANY_WEBSITE           "https://github.com/lijunxian2002/HonestMix"
    COPYRIGHT                 "Copyright 2026 SOLAR&TERRA DESIGN STUDIO"
    DESCRIPTION               "HonestMix — 耳机频响校正"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    IS_AUDIO_SOURCE           FALSE
    IS_AUDIO_EFFECT           TRUE
    VST3_CATEGORY             "Fx|Eq"
    PRODUCT_NAME              "BETA2"
    PLUGIN_NAME               "BETA2"
)

target_sources(BETA2 PRIVATE
    Source/PluginProcessor.h    Source/PluginProcessor.cpp
    Source/PluginEditor.h       Source/PluginEditor.cpp
    Source/Core/Design.h
    Source/Core/AppState.h      Source/Core/AppState.cpp
    Source/Core/HeadphoneDatabase.h Source/Core/HeadphoneDatabase.cpp
    Source/Core/BPMData.h       Source/Core/BPMData.cpp
    Source/DSP/CorrectionEngine.h Source/DSP/CorrectionEngine.cpp
    Source/DSP/WetDryMixer.h    Source/DSP/WetDryMixer.cpp
    Source/dsp/fir_data.h
    Source/Net/FeedbackClient.h Source/Net/FeedbackClient.cpp
    Source/UI/Components.h      Source/UI/Components.cpp
)

target_include_directories(BETA2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_link_libraries(BETA2 PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
)

target_compile_definitions(BETA2 PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
    _CRT_SECURE_NO_WARNINGS
)
target_compile_options(BETA2 PRIVATE /W3 /utf-8)
