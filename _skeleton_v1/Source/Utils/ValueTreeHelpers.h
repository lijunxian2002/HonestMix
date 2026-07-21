#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * ValueTree 辅助工具
 *
 * 提供对 JUCE ValueTree 的常用读写包装，
 * 减少重复的类型转换和默认值处理代码。
 *
 * 用法：
 *   auto val = ValueTreeHelpers::getInt(tree, "bpm", 120);
 *   ValueTreeHelpers::setFloat(tree, "wetDry", 0.5f);
 */
struct ValueTreeHelpers
{
    //==============================================================================
    // ——— 读取 ———

    static int         getInt  (const juce::ValueTree& tree,
                                const juce::Identifier& property,
                                int defaultValue = 0);

    static float       getFloat(const juce::ValueTree& tree,
                                const juce::Identifier& property,
                                float defaultValue = 0.0f);

    static double      getDouble(const juce::ValueTree& tree,
                                 const juce::Identifier& property,
                                 double defaultValue = 0.0);

    static bool        getBool (const juce::ValueTree& tree,
                                const juce::Identifier& property,
                                bool defaultValue = false);

    static juce::String getString(const juce::ValueTree& tree,
                                  const juce::Identifier& property,
                                  const juce::String& defaultValue = {});

    //==============================================================================
    // ——— 写入 ———

    static void setInt   (juce::ValueTree& tree,
                          const juce::Identifier& property,
                          int value);

    static void setFloat (juce::ValueTree& tree,
                          const juce::Identifier& property,
                          float value);

    static void setDouble(juce::ValueTree& tree,
                          const juce::Identifier& property,
                          double value);

    static void setBool  (juce::ValueTree& tree,
                          const juce::Identifier& property,
                          bool value);

    static void setString(juce::ValueTree& tree,
                          const juce::Identifier& property,
                          const juce::String& value);

    //==============================================================================
    // ——— 子结点查找 ———

    /** 获取或创建指定 ID 的子结点 */
    static juce::ValueTree getOrCreateChild(juce::ValueTree& parent,
                                            const juce::Identifier& childTag,
                                            int index = -1);

    /** 深拷贝 ValueTree（递归复制所有属性与子结点） */
    static juce::ValueTree deepCopy(const juce::ValueTree& source);

private:
    ValueTreeHelpers() = default; // 静态工具类
};
