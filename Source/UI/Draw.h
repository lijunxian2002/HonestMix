#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/DesignTokens.h"

// ═══════════════════════════════════════════════════════════
// Draw 工具命名空间
// 让 JUCE 绘图像写 CSS 一样简单
// ═══════════════════════════════════════════════════════════

namespace Draw {

// ── 0. 阴影组渲染（消费 DesignTokens.h 的 hm::tokens::xxx_shadow 数组）──
// CSS:  box-shadow: 由外到内逐层;  调用一次画完整个阴影组
inline void shadowStack (juce::Graphics& g,
                         juce::Rectangle<float> r,
                         float cornerRadius,
                         const hm::tokens::ShadowLayer* layers,
                         int count)
{
    for (int i = 0; i < count; ++i)
    {
        const auto& L = layers[i];
        if (L.blur > 0.5f)
        {
            // 模糊投影：多层半透明膨胀模拟（blur 越大层越淡）
            int steps = juce::jlimit (2, 8, (int)(L.blur / 3.0f) + 1);
            auto base = r.translated (L.dx, L.dy).expanded (L.spread);
            for (int k = steps; k >= 1; --k)
            {
                float t = (float) k / (float) steps;
                g.setColour (L.col.withAlpha (L.col.getAlpha() / 255.0f * 0.25f * t));
                g.fillRoundedRectangle (base.expanded (L.blur * (1.0f - t)), cornerRadius + L.blur * (1.0f - t));
            }
        }
        else if (L.spread > 0.5f)
        {
            // spread 细描边（如 0 0 0 5px rgba(...)）：逐层描边
            g.setColour (L.col);
            g.drawRoundedRectangle (r.expanded (L.spread).translated (L.dx, L.dy),
                                    cornerRadius + L.spread, 1.0f);
        }
        else
        {
            // 普通偏移投影：一次填充
            g.setColour (L.col);
            g.fillRoundedRectangle (r.translated (L.dx, L.dy).expanded (L.spread),
                                    cornerRadius + L.spread);
        }
    }
}

// 便捷重载：直接传数组名（自动推导长度）
template <size_t N>
inline void shadowStack (juce::Graphics& g,
                         juce::Rectangle<float> r,
                         float cornerRadius,
                         const hm::tokens::ShadowLayer (&layers)[N])
{
    shadowStack (g, r, cornerRadius, layers, (int) N);
}

// ── 1. 圆角盒子 + 边框 ──────────────────────────────────
// CSS:  border-radius: r;  border: 1px solid c;
inline void roundedBox (juce::Graphics& g,
                        juce::Rectangle<float> r,
                        juce::Colour fill,
                        float cornerRadius,
                        juce::Colour stroke = juce::Colours::transparentBlack,
                        float strokeWidth = 0)
{
    g.setColour (fill);
    g.fillRoundedRectangle (r, cornerRadius);
    if (strokeWidth > 0 && stroke.isOpaqueOrTranslucent()) {
        g.setColour (stroke);
        g.drawRoundedRectangle (r.reduced (strokeWidth * 0.5f), cornerRadius, strokeWidth);
    }
}

// ── 2. 投影 ─────────────────────────────────────────────
// CSS:  box-shadow: dx dy blur color;
inline void dropShadow (juce::Graphics& g,
                        juce::Rectangle<float> r,
                        float cornerRadius,
                        juce::Colour shadowCol,
                        int dx = 0, int dy = 0, int blur = 8)
{
    auto shadow = r.translated ((float) dx, (float) dy);
    // 多层级半透明模拟模糊效果
    for (int i = 1; i <= blur/2; ++i) {
        auto alpha = shadowCol.getAlpha() * (1.0f - (float)i / (float)(blur/2));
        g.setColour (shadowCol.withAlpha (alpha * 0.15f));
        g.fillRoundedRectangle (shadow.expanded ((float)i * 1.2f), cornerRadius + (float)i);
    }
}

// ── 3. 发光文字 ─────────────────────────────────────────
// CSS:  text-shadow: 0 0 8px color;
inline void glowText (juce::Graphics& g,
                      const juce::String& text,
                      juce::Rectangle<float> area,
                      juce::Colour textCol,
                      juce::Colour glowCol,
                      float glowSize = 4.0f,
                      juce::Justification justify = juce::Justification::centred)
{
    // 发光层
    for (int i = 2; i <= (int)glowSize; i += 2) {
        g.setColour (glowCol.withAlpha (0.08f));
        g.drawText (text, area.reduced ((float)i), justify);
    }
    // 主文字层
    g.setColour (textCol);
    g.drawText (text, area, justify);
}

// ── 4. 渐变填充 ─────────────────────────────────────────
// CSS:  background: linear-gradient(to bottom, c1 at p1, c2 at p2);
enum class Dir { Horizontal, Vertical, Diagonal };

inline void gradient (juce::Graphics& g,
                      juce::Rectangle<float> r,
                      juce::Colour c1, float p1,   // 0..1
                      juce::Colour c2, float p2,
                      Dir dir = Dir::Vertical)
{
    juce::ColourGradient grad;
    if (dir == Dir::Vertical) {
        grad = juce::ColourGradient (c1, r.getX(), r.getY(),
                                     c2, r.getX(), r.getBottom(), false);
    } else if (dir == Dir::Horizontal) {
        grad = juce::ColourGradient (c1, r.getX(), r.getY(),
                                     c2, r.getRight(), r.getY(), false);
    } else {
        grad = juce::ColourGradient (c1, r.getX(), r.getY(),
                                     c2, r.getRight(), r.getBottom(), false);
    }
    grad.addColour (juce::jlimit (0.0, 1.0, p1), c1);
    grad.addColour (juce::jlimit (0.0, 1.0, p2), c2);
    g.setGradientFill (grad);
    g.fillRect (r);
}

// ── 5. 状态指示灯 ───────────────────────────────────────
// CSS:  div { width: w; height: w; border-radius: 50%; background: c; box-shadow: glow; }
inline void statusDot (juce::Graphics& g,
                       juce::Point<float> centre,
                       float diameter,
                       juce::Colour fill,
                       bool glowing = true)
{
    auto r = juce::Rectangle<float> (diameter, diameter).withCentre (centre);
    if (glowing) {
        for (int i = 2; i <= 6; i += 2) {
            g.setColour (fill.withAlpha (0.07f));
            g.fillEllipse (r.expanded ((float)i * 1.5f));
        }
        // 外层光晕
        g.setColour (fill.withAlpha (0.2f));
        g.fillEllipse (r.expanded (4.0f));
    }
    g.setColour (fill);
    g.fillEllipse (r);
}

// ── 6. Pill 药丸按钮 ───────────────────────────────────
// CSS:  border-radius: 9999px; 两端全圆
inline void pill (juce::Graphics& g,
                  juce::Rectangle<float> r,
                  juce::Colour fill,
                  juce::Colour textCol,
                  const juce::String& label,
                  float textSize,
                  juce::Justification justify = juce::Justification::centred)
{
    auto radius = r.getHeight() * 0.5f;
    g.setColour (fill);
    g.fillRoundedRectangle (r, radius);
    if (label.isNotEmpty()) {
        g.setFont (juce::FontOptions (textSize));
        g.setColour (textCol);
        g.drawText (label, r, justify);
    }
}

// ── 7. 毛玻璃面板 ──────────────────────────────────────
// CSS:  backdrop-filter: blur(12px); background: rgba(...);
inline void glassPane (juce::Graphics& g,
                       juce::Rectangle<float> r,
                       float cornerRadius,
                       juce::Colour tint,
                       float borderAlpha = 0.08f)
{
    // 底色
    g.setColour (tint);
    g.fillRoundedRectangle (r, cornerRadius);
    // 顶部高光线
    auto topLine = r.withHeight (1.0f).reduced (cornerRadius * 0.3f, 0);
    g.setColour (juce::Colours::white.withAlpha (borderAlpha * 0.6f));
    g.fillRoundedRectangle (topLine, 0.5f);
    // 底部投影
    g.setColour (juce::Colours::black.withAlpha (borderAlpha * 0.5f));
    g.fillRect (r.withTop (r.getBottom() - 0.5f).reduced (cornerRadius * 0.3f, 0));
}

// ── 8. 脉冲动画辅助 ────────────────────────────────────
// 在 Timer::timerCallback 中每帧调用 pulsePhase += 0.02f
// 返回 0-1  循环

// ── 9. 多段渐变 ────────────────────────────────────────
// CSS:  background: conic-gradient / multi-stop linear-gradient
struct Stop { juce::Colour colour; float position; };  // 0..1

inline void multiGradient (juce::Graphics& g,
                           juce::Rectangle<float> r,
                           const std::vector<Stop>& stops,
                           Dir dir = Dir::Vertical)
{
    if (stops.empty()) return;
    juce::ColourGradient grad;
    auto first = stops.front();
    auto last = stops.back();
    if (dir == Dir::Vertical)
        grad = juce::ColourGradient (first.colour, r.getX(), r.getY(),
                                     last.colour, r.getX(), r.getBottom(), false);
    else
        grad = juce::ColourGradient (first.colour, r.getX(), r.getY(),
                                     last.colour, r.getRight(), r.getY(), false);
    for (auto& s : stops)
        grad.addColour (juce::jlimit (0.0, 1.0, s.position), s.colour);
    g.setGradientFill (grad);
    g.fillRect (r);
}

// ── 10. 进度条（带光晕） ────────────────────────────────
inline void progressBar (juce::Graphics& g,
                         juce::Rectangle<float> track,
                         float fraction,
                         juce::Colour trackCol,
                         juce::Colour fillCol,
                         float cornerRadius = 0)
{
    // 轨道
    g.setColour (trackCol);
    g.fillRoundedRectangle (track, cornerRadius);
    // 填充
    auto fill = track.withWidth (track.getWidth() * juce::jlimit (0.0f, 1.0f, fraction));
    g.setColour (fillCol);
    g.fillRoundedRectangle (fill, cornerRadius);
    // 头部光晕
    if (fraction > 0.05f && fraction < 0.95f) {
        auto head = fill.withWidth (juce::jmin (6.0f, fill.getWidth()));
        g.setColour (fillCol.withAlpha (0.3f));
        g.fillRoundedRectangle (head.expanded (1, 0), cornerRadius);
    }
}

} // namespace Draw
