cmake_minimum_required(VERSION 3.22)
project(HonestMixBETA2 VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 强制 UTF-8 编译（MSVC）────────────────────────────
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

# ── JUCE 框架（third_party/JUCE 为指向数据中心的目录联接）──
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt)
    message(FATAL_ERROR "third_party/JUCE 缺失：请重建目录联接（见 拼接报告_BETA2.md）")
endif()
add_subdirectory(third_party/JUCE)

# ── 插件定义 ─────────────────────────────────────────────
juce_add_plugin(BETA2
    PLUGIN_MANUFACTURER_CODE  Hnst
    PLUGIN_CODE               Bet2
    BUNDLE_ID                 com.solarterra.HonestMix.BETA2
    FORMATS                   VST3
    COMPANY_NAME              "SOLAR&TERRA DESIGN STUDIO"
    COMPANY_WEBSITE           "https://github.com/lijunxian2002/HonestMix"
    COPYRIGHT                 "Copyright 2026 SOLAR&TERRA DESIGN STUDIO"
    DESCRIPTION               "HonestMix 诚听 BETA2 — 耳机频响校正（模块化拼接版）"
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

# ── 源文件（模块化拼接：骨架结构 × 数据中心实现）──────────
target_sources(BETA2
    PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h

    Source/Core/Design.h
    Source/Core/AppState.cpp
    Source/Core/AppState.h
    Source/Core/HeadphoneDatabase.cpp
    Source/Core/HeadphoneDatabase.h

    Source/DSP/CorrectionEngine.cpp
    Source/DSP/CorrectionEngine.h
    Source/DSP/WetDryMixer.cpp
    Source/DSP/WetDryMixer.h
    Source/dsp/fir_data.h

    Source/Net/FeedbackClient.cpp
    Source/Net/FeedbackClient.h

    Source/UI/SealComponent.cpp
    Source/UI/SealComponent.h
    Source/UI/SnakeKnob.cpp
    Source/UI/SnakeKnob.h
    Source/UI/SetupCard.cpp
    Source/UI/SetupCard.h
    Source/UI/TransitionCard.cpp
    Source/UI/TransitionCard.h
    Source/UI/BpmPanel.cpp
    Source/UI/BpmPanel.h
    Source/UI/FeedbackCard.cpp
    Source/UI/FeedbackCard.h
    Source/UI/CheckCard.cpp
    Source/UI/CheckCard.h
    Source/UI/ShareCard.cpp
    Source/UI/ShareCard.h
)

target_include_directories(BETA2
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ── 链接 ─────────────────────────────────────────────────
target_link_libraries(BETA2 PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# ── 编译选项 ─────────────────────────────────────────────
target_compile_definitions(BETA2 PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
    _CRT_SECURE_NO_WARNINGS
)
target_compile_options(BETA2 PRIVATE /W3 /utf-8)
