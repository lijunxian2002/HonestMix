#pragma once

//==============================================================================
/**
 * HonestMix 设计令牌 —— 全局唯一视觉/布局真源
 *
 * 来源：数据中心终局实现（PluginEditor 顶部 hm:: 命名空间）原样抽取。
 * 修 BUG 指引：任何颜色 / 字体 / 布局比例问题，只改这一个文件。
 */
#include <juce_gui_basics/juce_gui_basics.h>

namespace hm
{
    // ── 温香槟金 · 颜色令牌 ──────────────────────────────────
    inline juce::Colour whiteA (float a)   { return juce::Colours::white.withAlpha (a); }
    inline juce::Colour gold()             { return juce::Colour::fromRGB (200, 178, 148); }

    inline juce::Colour bg()               { return juce::Colour::fromRGB (43, 45, 50); }  // 专业灰（Studio One 色系）
    inline juce::Colour panelBg()          { return juce::Colour::fromRGBA (22, 22, 26, 140); }
    inline juce::Colour innerBg()          { return juce::Colour::fromRGBA (23, 23, 28, 180); }
    inline juce::Colour overlayBg()        { return juce::Colour::fromRGBA (10, 10, 12, 140); }

    inline juce::Colour textMain()         { return gold().withAlpha (0.88f); }
    inline juce::Colour textSec()          { return gold().withAlpha (0.65f); }
    inline juce::Colour textLabel()        { return gold().withAlpha (0.50f); }
    inline juce::Colour textDim()          { return gold().withAlpha (0.32f); }

    inline juce::Colour border()           { return whiteA (0.08f); }
    inline juce::Colour borderStrong()     { return whiteA (0.14f); }

    inline juce::Colour accent()           { return gold().withAlpha (0.40f); }
    inline juce::Colour accentHov()        { return gold().withAlpha (0.55f); }
    inline juce::Colour accentWarm()       { return gold().withAlpha (0.70f); }

    // ── 统一频谱色: 0灰(骗人)→蓝→青→100绿(12分)→150金→200灰红(危险) ──
    inline juce::Colour spectrumColor (float pct)
    {
        pct = juce::jlimit (0.0f, 1.0f, pct);
        struct Stop { float p; int r, g, b; };
        static constexpr Stop stops[] = {
            {0.00f, 90,90,92},    {0.15f, 60,100,150},   // 灰→钢蓝
            {0.30f, 40,140,165},   {0.42f, 30,185,155},   // 青→绿
            {0.50f, 20,220,110},                            // 100=纯绿 12分
            {0.65f, 100,190,105},  {0.78f, 185,155,85},    // 金
            {0.90f, 210,105,80},   {1.00f, 220,80,70}      // 灰红 危险
        };
        for (int i=1;i<9;++i) if(pct<=stops[i].p){
            float f=(pct-stops[i-1].p)/(stops[i].p-stops[i-1].p);
            return juce::Colour::fromFloatRGBA(
                (stops[i-1].r+(stops[i].r-stops[i-1].r)*f)/255.f,
                (stops[i-1].g+(stops[i].g-stops[i-1].g)*f)/255.f,
                (stops[i-1].b+(stops[i].b-stops[i-1].b)*f)/255.f,1.f);
        }
        return juce::Colour::fromFloatRGBA(0.86f,0.31f,0.27f,1.f);
    }

    // ── 字体 ────────────────────────────────────────────────
    inline juce::Font kaiTi (float size)   { return juce::Font (juce::FontOptions ("KaiTi", size, juce::Font::plain)); }
    inline juce::Font mono (float size)    { return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }

    // ── 布局基准（所有坐标 × scale_ 适配当前屏幕）────────────
    // 数据中心实现版：PW = 屏宽 28%，420×300 平常态 / 420×645 展开态
    // （项目文档 v0.4.0 写的 23% / 600 基准与实现有漂移，以此实现为准；
    //   若要切回文档规格，只改这三个常量 + kScreenWidthPercent）
    inline constexpr int kRefWidth        = 420;  // 设计基准宽
    inline constexpr int kRefHeightNormal = 300;  // 平常态基准高
    inline constexpr int kRefHeightExpand = 645;  // 展开态基准高
    inline constexpr int kScreenWidthPercent = 28; // 画布宽 = 屏宽 28%

    /** 玻璃卡片绘制（填充 + 1px 细边） */
    inline void drawCard (juce::Graphics& g, juce::Rectangle<float> b, float radius, juce::Colour fill)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (b, radius);
        g.setColour (border());
        g.drawRoundedRectangle (b.reduced (0.5f), radius, 1.0f);
    }

    /** Apple 毛玻璃底：对父组件指定区域做快照 + 高斯模糊（遮罩卡打开前调用） */
    inline juce::Image blurredBackdrop (juce::Component& parent, juce::Rectangle<int> area,
                                        float radius = 6.0f)
    {
        auto img = parent.createComponentSnapshot (area, true);
        if (img.isValid() && radius > 0.0f)
        {
            juce::ImageConvolutionKernel kernel (juce::roundToInt (radius * 2.5f) | 1); // 奇数核
            kernel.createGaussianBlur (radius);
            kernel.applyToImage (img, img, img.getBounds());
        }
        return img;
    }
}

//==============================================================================
/**
 * 全局 LookAndFeel —— 默认无衬线字体走雅黑（PingFang SC 的 Windows 对应）
 * 由 PluginEditor 持有一个实例并挂到根组件。
 */
class HonestLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HonestLookAndFeel() { setDefaultSansSerifTypefaceName ("Microsoft YaHei UI"); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HonestLookAndFeel)
};
