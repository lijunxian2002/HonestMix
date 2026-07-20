cmake_minimum_required(VERSION 3.22)
project(HonestMix VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── JUCE 框架 ────────────────────────────────────────────
add_subdirectory(third_party/JUCE)

# ── 插件定义 ─────────────────────────────────────────────
juce_add_plugin(HonestMix
    PLUGIN_MANUFACTURER_CODE  Hnst
    PLUGIN_CODE               Hnst
    BUNDLE_ID                 com.solarterra.HonestMix
    FORMATS                   VST3
    COMPANY_NAME              "SOLAR&TERRA DESIGN STUDIO"
    COMPANY_WEBSITE           "https://github.com/lijunxian2002/HonestMix"
    COPYRIGHT                 "Copyright 2026 SOLAR&TERRA DESIGN STUDIO"
    DESCRIPTION               "HonestMix 诚听 — 耳机频响校正工具"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    IS_AUDIO_SOURCE           FALSE
    IS_AUDIO_EFFECT           TRUE
    VST3_CATEGORY             "Fx|Eq"
)

# ── 源文件 ───────────────────────────────────────────────
target_sources(HonestMix
    PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/dsp/CorrectionEngine.cpp
    Source/dsp/CorrectionEngine.h
)

target_include_directories(HonestMix
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ── 链接 DSP 模块（卷积引擎需要） ─────────────────────
target_link_libraries(HonestMix PRIVATE juce::juce_dsp)

# ── 编译选项 ─────────────────────────────────────────────
target_compile_definitions(HonestMix PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)

target_compile_options(HonestMix PRIVATE /W3 /utf-8)
