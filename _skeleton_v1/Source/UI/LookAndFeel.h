#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * HonestMix 自定义暗色主题
 *
 * 完整复刻 HTML 原型的暗色设计语言：
 *   - 背景: #0c0c0e (近乎纯黑)
 *   - 卡片底色: rgba(18,18,20,0.85) + backdrop blur
 *   - 文字主色: #f0f0f0
 *   - 边框: rgba(255,255,255,0.06) 极细描边
 *   - 强调色: #B85454 (暗红)
 *   - 圆角: 8–10px
 *
 * 继承 juce::LookAndFeel_V4 并覆写关键绘制方法。
 * 所有 UI 组件应统一使用此 LookAndFeel。
 */
class HonestMixLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==============================================================================
    HonestMixLookAndFeel();
    ~HonestMixLookAndFeel() override = default;

    //==============================================================================
    /** 全局单例实例（UI 组件通过此方法获取 LookAndFeel） */
    static HonestMixLookAndFeel& getInstance()
    {
        static HonestMixLookAndFeel instance;
        return instance;
    }

    //==============================================================================
    // ——— 全局颜色常量 ———

    static constexpr juce::Colour bgDark          { 0xFF0C0C0E }; ///< 主背景色
    static constexpr juce::Colour cardBg           { 0xD9121214 }; ///< 卡片底色 (alpha 0.85)
    static constexpr juce::Colour cardBorder       { 0x0FFFFFFF }; ///< 卡片边框 (alpha 0.06)
    static constexpr juce::Colour textPrimary      { 0xFFF0F0F0 }; ///< 主文字色
    static constexpr juce::Colour textMuted        { 0x4DFFFFFF }; ///< 次要文字 (alpha 0.3)
    static constexpr juce::Colour textSubtle       { 0x14FFFFFF }; ///< 极弱文字 (alpha 0.08)
    static constexpr juce::Colour accent           { 0xFFB85454 }; ///< 强调色 (暗红)
    static constexpr juce::Colour accentDim        { 0x40B85454 }; ///< 强调色弱化
    static constexpr juce::Colour optionBg         { 0x0AFFFFFF }; ///< 选项背景 (alpha 0.04)
    static constexpr juce::Colour optionBorder     { 0x14FFFFFF }; ///< 选项边框 (alpha 0.08)
    static constexpr juce::Colour divider          { 0x08FFFFFF }; ///< 分隔线 (alpha 0.03)
    static constexpr juce::Colour overlayBg        { 0x4C0A0A0C }; ///< 叠加层背景

    //==============================================================================
    // ——— 覆写绘制方法 ———

    // — 按钮 —
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOver, bool isDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isMouseOver, bool isDown) override;

    // — 滑块 / 旋钮 —
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider& slider) override;

    // — 文本框 —
    void drawTextEditorOutline(juce::Graphics& g, int w, int h,
                               juce::TextEditor& editor) override;

    void fillTextEditorBackground(juce::Graphics& g, int w, int h,
                                  juce::TextEditor& editor) override;

    // — 标签 —
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // — 组合框 —
    void drawComboBox(juce::Graphics& g, int w, int h,
                      bool isMouseDown, int buttonX, int buttonY,
                      int buttonW, int buttonH, juce::ComboBox& box) override;

    // — 默认字体 —
    juce::Font getTextButtonFont(juce::TextButton&, int height) override;
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

    //==============================================================================
    // ——— 辅助绘制工具方法 ———

    /** 绘制圆角矩形卡片背景 */
    static void drawCardBackground(juce::Graphics& g, juce::Rectangle<int> bounds,
                                    float cornerRadius = 10.0f);

    /** 绘制选项行 (radio 按钮样式) */
    static void drawOptionRow(juce::Graphics& g, juce::Rectangle<int> bounds,
                               bool selected, bool hovered);

    /** 绘制 radio 圆点 */
    static void drawRadioDot(juce::Graphics& g, juce::Rectangle<int> bounds,
                              bool selected);

private:
    //==============================================================================
    juce::Typeface::Ptr fontRegular;
    juce::Typeface::Ptr fontMono;

    void initFonts();
};
