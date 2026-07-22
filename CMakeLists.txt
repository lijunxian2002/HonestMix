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

# ═══════════════════════════════════════════════════════════
# JUCE 框架（third_party/JUCE → 目录联接 → 数据中心仓库A）
# ═══════════════════════════════════════════════════════════
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt)
    message(FATAL_ERROR "third_party/JUCE 缺失: 请重建目录联接（见 拼接报告_BETA2.md）")
endif()
add_subdirectory(third_party/JUCE)

# ═══════════════════════════════════════════════════════════
# 插件定义
# ═══════════════════════════════════════════════════════════
juce_add_plugin(BETA2
    PLUGIN_MANUFACTURER_CODE  Hnst
    PLUGIN_CODE               Bet2
    BUNDLE_ID                 com.solarterra.HonestMix.BETA2
    FORMATS                   VST3
    COMPANY_NAME              "SOLAR&TERRA DESIGN STUDIO"
    COMPANY_WEBSITE           "https://github.com/lijunxian2002/HonestMix"
    COPYRIGHT                 "Copyright 2026 SOLAR&TERRA DESIGN STUDIO"
    DESCRIPTION               "HonestMix — 耳机频响校正（模块化骨架）"
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

# ═══════════════════════════════════════════════════════════
# 源文件 — 按模块分组
# ═══════════════════════════════════════════════════════════
target_sources(BETA2 PRIVATE

    # —— 入口（2 文件）—————————————————————————————————————
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp

    # —— Core：设计令牌 / 状态 / 数据（5 文件）—————————————
    Source/Core/Design.h
    Source/Core/AppState.h
    Source/Core/AppState.cpp
    Source/Core/HeadphoneDatabase.h
    Source/Core/HeadphoneDatabase.cpp

    # —— DSP：FIR 引擎 / 混合器 / 系数表（5 文件）———————
    Source/DSP/CorrectionEngine.h
    Source/DSP/CorrectionEngine.cpp
    Source/DSP/WetDryMixer.h
    Source/DSP/WetDryMixer.cpp
    Source/dsp/fir_data.h               # 25MB / 1603副 AutoEq FIR

    # —— Net：反馈服务（2 文件）————————————————————————————
    Source/Net/FeedbackClient.h
    Source/Net/FeedbackClient.cpp

    # —— UI：主界面组件（16 文件）—————————————————————————
    Source/UI/SnakeKnob.h
    Source/UI/SnakeKnob.cpp
    Source/UI/SealComponent.h
    Source/UI/SealComponent.cpp
    Source/UI/SetupCard.h
    Source/UI/SetupCard.cpp
    Source/UI/TransitionCard.h
    Source/UI/TransitionCard.cpp
    Source/UI/BpmPanel.h
    Source/UI/BpmPanel.cpp
    Source/UI/FeedbackCard.h
    Source/UI/FeedbackCard.cpp
    Source/UI/CheckCard.h
    Source/UI/CheckCard.cpp
    Source/UI/ShareCard.h
    Source/UI/ShareCard.cpp
)

target_include_directories(BETA2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

# ═══════════════════════════════════════════════════════════
# JUCE 模块依赖
# ═══════════════════════════════════════════════════════════
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

# ═══════════════════════════════════════════════════════════
# 编译定义
# ═══════════════════════════════════════════════════════════
target_compile_definitions(BETA2 PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
    _CRT_SECURE_NO_WARNINGS
)
target_compile_options(BETA2 PRIVATE /W3 /utf-8)
