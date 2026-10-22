#include "UI/StarField.h"
#include "Core/DesignTokens.h"
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 13b StarField — 逐行对标 13-CurveCanvas+StarField.html
// 45星 + 1追星 + 呼吸闪烁 + 鼠标视差偏移 + 心形聚集
// 参数全部来自 hm::tokens（design-tokens.md §8.3 自动生成）
// ═══════════════════════════════════════════════════════════

static float breathVal (juce::int64 ts)
{
    // HTML: getBreathValue — 1978ms golden ratio cycle
    // rise 38.2% to max 1.44, smoothstep ease
    using namespace hm::tokens;
    const float T = breath_period, rise = breath_rise_ratio,
                minB = breath_min_brightness, maxB = breath_max_brightness;
    float p = fmodf ((float) ts / T, 1.0f);
    if (p <= rise) {
        float t = p / rise;
        float v = t * t * (3.0f - 2.0f * t);  // smoothstep
        return minB + (maxB - minB) * v;
    } else {
        float t = (p - rise) / (1.0f - rise);
        float v = 1.0f - (t * t * (3.0f - 2.0f * t));
        return minB + (maxB - minB) * v;
    }
}

StarField::StarField()
{
    // NASA 深空底图（相对于可执行文件或桌面查找）
    juce::File imgPath = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                             .getSiblingFile ("Source/UI/stars_deep.jpg");
    if (!imgPath.existsAsFile())
        imgPath = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                      .getChildFile ("HonestMix_组件交付/stars_deep.jpg");
    bgImage_ = juce::ImageFileFormat::loadFrom (imgPath);

    // HTML: 45颗星均匀分布(7×7网格+随机偏移，避免聚类空洞)
    using namespace hm::tokens;
    int idx = 0;
    for (int row = 0; row < 7 && idx < star_count; ++row) {
        for (int col = 0; col < 7 && idx < star_count; ++col) {
            float x = (float)col / 6.0f * 100.0f + ((float)(rand()%30)-15.0f);
            float y = (float)row / 6.0f * 100.0f + ((float)(rand()%30)-15.0f);
            x = juce::jlimit (0.0f, 100.0f, x);
            y = juce::jlimit (0.0f, 100.0f, y);
            float sz = star_size_min + (float)(rand() % 130) / 100.0f;
            float op = star_opacity_min + (float)(rand() % 18) / 100.0f;
            juce::int64 off = (juce::int64)(rand() % (int)breath_period);
            stars_.add ({x, y, x, y, x, y, sz, off, op});
            ++idx;
        }
    }

    // HTML chase star: 3px white-blue glow
    chaseHomeX_ = chaseHomeY_ = 50.0f;
    chaseCurX_ = chaseCurY_ = 50.0f;
    chaseVX_ = chaseVY_ = 0.0f;

    t0_ = juce::Time::getCurrentTime().toMilliseconds();
    startTimerHz (60);
}

void StarField::mouseMove (const juce::MouseEvent& e)
{
    auto r = getLocalBounds();
    mx_ = juce::jlimit (0.0f, 100.0f, (float) e.x / r.getWidth() * 100.0f);
    my_ = juce::jlimit (0.0f, 100.0f, (float) e.y / r.getHeight() * 100.0f);
    mouseInside_ = true;
    stillStart_ = juce::Time::getCurrentTime().toMilliseconds();
    if (heartMode_) endHeart();
}

void StarField::mouseExit (const juce::MouseEvent&)
{
    mouseInside_ = false;
    if (heartMode_) endHeart();
}

void StarField::startHeart()
{
    if (heartMode_) return;
    heartMode_ = true;
    heartStars_.clear();

    // HTML: 15 closest stars to cursor
    struct Dist { int i; float d; };
    std::vector<Dist> dists;
    for (int i = 0; i < stars_.size(); ++i) {
        float dx = stars_[i].x - mx_, dy = stars_[i].y - my_;
        dists.push_back ({i, std::sqrt (dx*dx + dy*dy)});
    }
    std::sort (dists.begin(), dists.end(), [](auto& a, auto& b) { return a.d < b.d; });

    for (int j = 0; j < 15 && j < (int) dists.size(); ++j) {
        int idx = dists[j].i;
        heartStars_.push_back (idx);
        auto& s = stars_.getReference (idx);
        // HTML heart formula
        float t = (float) j / 15.0f * 6.283185f;
        float sn = 0.5f + 0.5f * (float) j / 15.0f;
        float hx = 10.0f * std::pow (std::sin (t), 3);
        float hy = 9.0f * std::cos (t) - 3.5f * std::cos (2*t) - 1.5f * std::cos (3*t) - 0.5f * std::cos (4*t);
        s.toX = mx_ + hx * sn;
        s.toY = my_ - hy * sn;
        s.scale = 1.6f;
        s.opacity = 0.95f;
        s.lerping = true;   // 开启 3.5s 过渡
        // snap to target (CSS 3.5s transition not yet applied per-frame coming in timer)
    }
}

void StarField::endHeart()
{
    heartMode_ = false;
    for (int i : heartStars_) {
        auto& s = stars_.getReference (i);
        s.toX = s.homeX;
        s.toY = s.homeY;
        s.scale = 1.0f;
        s.lerping = true;   // 归位 3.5s 过渡
        // opacity falls back in timer (CSS 3.5s transition)
    }
    heartStars_.clear();
}

void StarField::timerCallback()
{
    auto now = juce::Time::getCurrentTime().toMilliseconds();
    float t = (float)(now - t0_) / 1000.0f;

    // ── HTML: 容器级视差漂移 (wave drift) ──
    parallaxDX_ = std::sin (t * 0.07f) * 3.0f + std::cos (t * 0.13f) * 2.0f;
    parallaxDY_ = std::cos (t * 0.09f) * 2.5f + std::sin (t * 0.11f) * 1.8f;

    // ── HTML: 追星物理 (spdC=0.00008, friction=0.95) ──
    float dCx = mx_ - chaseCurX_, dCy = my_ - chaseCurY_;
    float distC = std::sqrt (dCx*dCx + dCy*dCy);
    if (!heartMode_) {
        float spdC = 0.00008f * (1.0f + distC * 0.02f);  // HTML 原值
        chaseVX_ += dCx * spdC;  chaseVY_ += dCy * spdC;
        chaseVX_ *= 0.95f;        chaseVY_ *= 0.95f;
        chaseCurX_ += chaseVX_;   chaseCurY_ += chaseVY_;
    }

    // ── HTML: 心跳检测 (distC<4, still>1.2s, cooldown>10s) ──
    if (!heartMode_ && mouseInside_ && distC < 4.0f
        && now - lastHeartTime_ > 10000 && now - stillStart_ > 1200) {
        startHeart();
        lastHeartTime_ = now;
    }

    // ── HTML: 普通星呼吸 (opacity=baseOp * breathVal / 0.44) ──
    for (int i = 0; i < stars_.size(); ++i) {
        auto& s = stars_.getReference (i);
        bool inHeart = heartMode_ && std::find (heartStars_.begin(), heartStars_.end(), i) != heartStars_.end();
        if (!inHeart) {
            float v = breathVal (now + s.phaseOffset);
            // HTML: opacity = base * v / 0.44, scale = 0.7 + v * 0.35
            // （HTML 直接赋值，无过渡——因为 CSS transition 只有 left/top）
            s.opacity = s.baseOpacity * (v / 0.44f);
            s.scale   = 0.7f + v * 0.35f;
            // position goes to home (3.5s eased)
            s.toX = s.homeX;
            s.toY = s.homeY;
        }
        // ── 位置：心形过渡用 3.5s ease lerp，平时不动 ──
        if (s.lerping) {
            s.x += (s.toX - s.x) * 0.022f;
            s.y += (s.toY - s.y) * 0.022f;
            if (std::abs(s.toX - s.x) < 0.05f && std::abs(s.toY - s.y) < 0.05f) s.lerping = false;
        } else {
            s.x = s.toX; s.y = s.toY;  // 平时不动
        }
    }

    repaint();
}

void StarField::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();

    // ── HTML: .stars bg (12% opacity × brightness 0.45 × contrast 0.5) ──
    if (bgImage_.isValid()) {
        g.setOpacity (0.06f);  // 12% × 0.45 brightness ≈ 5.4%
        g.drawImage (bgImage_, b.toFloat(), juce::RectanglePlacement::stretchToFit, false);
        g.setOpacity (1.0f);
    }
    // 暗色遮罩（模拟 contrast 0.5 压暗）
    g.setColour (juce::Colour::fromRGBA (8, 10, 16, static_cast<juce::uint8>(255*0.3f)));
    g.fillAll();
    // HTML .stars::after — 径向暗角 (50% 40% 蓝冷 + 25% 60% 暖色)
    juce::ColourGradient vignette1(juce::Colour::fromRGBA(100,150,200,8),b.getCentreX(),b.getY()+b.getHeight()*0.4f,
                                     juce::Colour::fromRGBA(0,0,0,0),b.getX()+b.getWidth()*0.55f,b.getY()+b.getHeight()*0.55f,true);
    g.setGradientFill(vignette1);g.fillAll();
    juce::ColourGradient vignette2(juce::Colour::fromRGBA(160,140,100,5),b.getX()+b.getWidth()*0.25f,b.getY()+b.getHeight()*0.6f,
                                     juce::Colour::fromRGBA(0,0,0,0),b.getX()+b.getWidth()*0.5f,b.getY()+b.getHeight()*0.5f,true);
    g.setGradientFill(vignette2);g.fillAll();
    juce::ColourGradient vignette3(juce::Colour::fromRGBA(0,0,0,107),b.getCentreX(),b.getCentreY(),
                                     juce::Colour::fromRGBA(0,0,0,0),b.getX(),b.getY(),true);
    g.setGradientFill(vignette3);g.fillAll();

    // ── HTML: container parallax (0.8s ease-out 过渡) ──
    float tgtDX = parallaxDX_, tgtDY = parallaxDY_;
    if (mouseInside_) {
        tgtDX += (mx_ - 50.0f) / 50.0f * 3.0f;
        tgtDY += (my_ - 50.0f) / 50.0f * 2.0f;
    }
    // ── HTML: 0.8s ease-out (mouse inside) / 1.5s (mouse left) ──
    float lerpSpd = mouseInside_ ? 0.022f : 0.012f;
    curDX_ += (tgtDX - curDX_) * lerpSpd;
    curDY_ += (tgtDY - curDY_) * lerpSpd;

    // ── 画普通星 ──
    for (auto& s : stars_) {
        if (s.opacity < 0.005f) continue;
        float sx = s.x + curDX_, sy = s.y + curDY_;
        if (sx < -10 || sx > 110 || sy < -10 || sy > 110) continue;
        g.setColour (juce::Colours::white.withAlpha (juce::jlimit (0.0f, 1.0f, s.opacity)));
        float px = sx / 100.0f * b.getWidth(), py = sy / 100.0f * b.getHeight();
        float r = s.size * s.scale;
        g.fillEllipse (px - r, py - r, r * 2.0f, r * 2.0f);
    }

    // ── HTML: 追星 3px核 + box-shadow 6px/14px (高斯光晕) ──
    {
        float cv = breathVal (juce::Time::getCurrentTime().toMilliseconds() + 600);
        float co = 0.3f + cv / 3.5f;
        float cs = 0.6f + cv * 0.35f;
        float cx = chaseCurX_ / 100.0f * b.getWidth();
        float cy = chaseCurY_ / 100.0f * b.getHeight();
        // 外层光晕 14px (HTML box-shadow 0 0 14px)
        juce::ColourGradient og (juce::Colour(120,180,240).withAlpha(co*0.25f), cx,cy, juce::Colours::transparentBlack, cx+8.0f*cs,cy, true);
        g.setGradientFill (og);
        g.fillEllipse (cx - 8.0f*cs, cy - 8.0f*cs, 16.0f*cs, 16.0f*cs);
        // 内层光晕 6px (HTML box-shadow 0 0 6px)  
        juce::ColourGradient ig (juce::Colour(150,200,255).withAlpha(co*0.50f), cx,cy, juce::Colours::transparentBlack, cx+4.0f*cs,cy, true);
        g.setGradientFill (ig);
        g.fillEllipse (cx - 4.0f*cs, cy - 4.0f*cs, 8.0f*cs, 8.0f*cs);
        // 核心 3px 实点 (HTML background rgba(180,220,255,0.9))
        g.setColour (juce::Colour(180,220,255).withAlpha(co*0.9f));
        g.fillEllipse (cx - 1.5f*cs, cy - 1.5f*cs, 3.0f*cs, 3.0f*cs);
    }
}
