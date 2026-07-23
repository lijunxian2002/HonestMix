#include "UI/Components.h"
#include "Core/Design.h"

// ═══════════════════════════════════════════════════════
// 阶段 1：占位实现 —— 每个组件画彩色方块，验证布局
// ═══════════════════════════════════════════════════════

static juce::Colour shellCol (int i) {
    static juce::Colour cols[] = {
        juce::Colour::fromRGB(60,60,80),   // 01 TopBar 暗蓝紫
        juce::Colour::fromRGB(50,70,50),   // 02 BottomRow 暗绿
        juce::Colour::fromRGB(80,50,50),   // 03 Tooltip 暗红（浮层）
        juce::Colour::fromRGB(255,255,255),// 04 Breath 白点
        juce::Colour::fromRGB(40,50,60),   // 05 InfoRow 暗蓝灰
        juce::Colour::fromRGB(70,40,40),   // 06 VUPanel 暗红棕
        juce::Colour::fromRGB(90,80,60),   // 07 OverlayBPM 暗金
        juce::Colour::fromRGB(40,60,40),   // 08 OverlayMonitor 暗绿
        juce::Colour::fromRGB(50,40,60),   // 09 OverlayFeedback 暗紫
        juce::Colour::fromRGB(60,60,40),   // 10 Fader 暗黄
        juce::Colour::fromRGB(30,30,40),   // 11 Cockpit 深蓝黑
        juce::Colour::fromRGB(255,255,255),// 12 CurveGrid 白线
        juce::Colour::fromRGB(200,200,200),// 12b CurveLabels 灰标签
        juce::Colour::fromRGB(100,100,120),// 13a CurveCanvas 灰蓝
        juce::Colour::fromRGB(0,0,0),      // 13b StarField 黑
    };
    return cols[juce::jlimit(0,14,i)];
}

// 01 TopBar — 品牌名(左) + 状态灯(右) + 底边线
void TopBar::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    float s = (float) b.getWidth() / 558.0f;  // 响应式字号
    // 底边线
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.fillRect (0.0f, (float)(b.getBottom()-1), (float)b.getWidth(), 1.0f);
    // 品牌名
    g.setFont (juce::FontOptions (juce::jmax (9.0f, 9.0f * s)));
    g.setColour (juce::Colour (245, 245, 240).withAlpha (0.60f));
    g.drawText ("HONESTMIX", b.getX(), b.getY(), b.getWidth() - 40, b.getHeight(),
                juce::Justification::centredLeft);
    // 状态灯
    float cx = (float)b.getRight() - 14.f, cy = (float)b.getCentreY();
    float r = juce::jmax (2.0f, 4.0f * s * 0.7f);
    juce::Colour dotCol = dotCol_.isTransparent() ? juce::Colour (100,98,94).withAlpha (0.70f) : dotCol_;
    g.setColour (dotCol.withAlpha (dotCol.getAlpha() / 255.0f * 0.3f));
    g.fillEllipse (cx - r*2.0f, cy - r*2.0f, r*4.0f, r*4.0f);
    g.setColour (dotCol);
    g.fillEllipse (cx - r, cy - r, r*2.0f, r*2.0f);
}

// 02 BottomRow — 三按钮等宽：BPM · MONITOR · 诚·反馈
void BottomRow::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    // 顶边线
    g.setColour (juce::Colours::white.withAlpha (0.03f));
    g.fillRect ((float)b.getX(), (float)b.getY(), (float)b.getWidth(), 1.0f);
    // 按钮区（跳过顶边线 + padding-top 9px）
    auto row = b.withTrimmedTop (10);
    auto drawBtn = [&](juce::Rectangle<int> r, const juce::String& t, bool accent, bool hover) {
        // bg + border + shadow
        g.setColour (hover ? juce::Colours::white.withAlpha (accent ? 0.04f : 0.03f)
                           : juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.20f)));
        g.fillRoundedRectangle (r.toFloat(), 4.0f);
        g.setColour (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.42f)));
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 4.0f, 1.0f);
        // text
        float fs = juce::jmax (7.0f, 7.0f * (float) r.getWidth() / 176.0f);
        g.setFont (juce::FontOptions (fs));
        juce::Colour txtCol = hover ? juce::Colour (245,245,240).withAlpha (0.55f)
                                    : accent ? juce::Colour (255,255,250).withAlpha (0.50f)
                                             : juce::Colour (235,235,230).withAlpha (0.38f);
        g.setColour (txtCol);
        g.drawText (t, r, juce::Justification::centred);
    };
    drawBtn (btnBPM_,       juce::String::fromUTF8 (u8"BPM 120"), false, false);
    drawBtn (btnMonitor_,   "MONITOR",                              false, false);
    drawBtn (btnFeedback_,  juce::String::fromUTF8 (u8"诚·反馈"),  true,  false);
}
void BottomRow::resized() {
    int w = getWidth(), h = getHeight();
    int bw = (w - 16) / 3;
    btnBPM_ = {0, 0, bw, h}; btnMonitor_ = {bw+8, 0, bw, h}; btnFeedback_ = {(bw+8)*2, 0, bw, h};
}
void BottomRow::mouseDown (const juce::MouseEvent& e) {
    auto p = e.getPosition();
    if (btnBPM_.contains(p) && onBPM) onBPM();
    else if (btnMonitor_.contains(p) && onMonitor) onMonitor();
    else if (btnFeedback_.contains(p) && onFeedback) onFeedback();
}

// 03 Tooltip
void Tooltip::show (int x,int y,float f,float d,int m){visible_=true;tx_=x;ty_=y;freq_=f;db_=d;mode_=m;repaint();}
void Tooltip::hide(){visible_=false;repaint();}
void Tooltip::paint(juce::Graphics& g){if(!visible_)return;g.setColour(shellCol(2).withAlpha(0.9f));g.fillRoundedRectangle((float)tx_,(float)ty_,60,22,4);g.setColour(juce::Colours::white.withAlpha(0.6f));g.setFont(9);g.drawText(juce::String(freq_,0)+"Hz "+juce::String(db_,1)+"dB",tx_+4,ty_,56,22,juce::Justification::centredLeft);}

// 04 Breath — 呼吸灯（3px 白点 + 蓝光晕 + 脉冲动画，周期 2s）
Breath::Breath() { startTimerHz (30); }
void Breath::timerCallback() { phase_ += 0.064f; if (phase_ > 6.283f) phase_ -= 6.283f; repaint(); }
void Breath::paint (juce::Graphics& g) {
    float a = 0.30f + 0.70f * (std::sin (phase_) * 0.5f + 0.5f);  // 0.30~1.00
    float sc = 1.0f + 1.2f * (std::sin (phase_) * 0.5f + 0.5f);   // 1.0~2.2
    float r = 1.5f * sc;
    g.setColour (juce::Colour (200,220,255).withAlpha (a * 0.6f));
    g.fillEllipse (-r + 1.5f, -r + 1.5f, r * 2.0f, r * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (a * 0.9f));
    g.fillEllipse (-r * 0.4f + 1.5f, -r * 0.4f + 1.5f, r * 0.8f, r * 0.8f);
}

// 05 InfoRow — 三芯片等宽：Headphone / Target Curve / Interface
void InfoRow::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    float s = (float)b.getWidth() / 560.0f;

    auto drawChip = [&](juce::Rectangle<int> r, const juce::String& label, const juce::String& val, bool hover) {
        // 芯片底
        g.setColour (hover ? juce::Colours::white.withAlpha (0.02f)
                           : juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.28f)));
        g.fillRoundedRectangle (r.toFloat(), 5.0f);
        g.setColour (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.28f)));
        g.drawRoundedRectangle (r.toFloat().reduced (0.75f), 5.0f, 1.5f);
        // 标签（6px, uppercase, letter-spacing 2px）
        float fl = juce::jmax (6.0f, 6.0f * s);
        g.setFont (juce::FontOptions (fl));
        g.setColour (juce::Colour (235,235,230).withAlpha (0.45f));
        g.drawText (label, r.getX(), r.getY() + 9, r.getWidth(), juce::roundToInt (fl * 1.3f),
                    juce::Justification::centred);
        // 值（9px, 居中）
        float fv = juce::jmax (9.0f, 9.0f * s);
        g.setFont (juce::FontOptions (fv));
        g.setColour (juce::Colour (240,240,235).withAlpha (0.68f));
        g.drawText (val, r.getX() + 4, r.getY() + juce::roundToInt (fl * 1.3f) + 3,
                    r.getWidth() - 8, r.getHeight() - juce::roundToInt (fl * 1.3f) - 3,
                    juce::Justification::centred);
    };

    drawChip (hpChip_,   "Headphone",    hpName_,  false);
    drawChip (cvChip_,   "Target Curve", cvName_,  false);
    drawChip (ifChip_,   "Interface",    ifName_,  false);
}
void InfoRow::setNames(const juce::String& h,const juce::String& c,const juce::String& i){hpName_=h;cvName_=c;ifName_=i;repaint();}
void InfoRow::resized(){int w=getWidth(),cw=(w-12)/3;hpChip_={0,0,cw,getHeight()};cvChip_={cw+6,0,cw,getHeight()};ifChip_={(cw+6)*2,0,cw,getHeight()};}
void InfoRow::mouseDown(const juce::MouseEvent& e){auto p=e.getPosition();if(hpChip_.contains(p)&&onProfileSelected)onProfileSelected(0);else if(cvChip_.contains(p)&&onCurveSelected)onCurveSelected(0);else if(ifChip_.contains(p)&&onInterfaceSelected)onInterfaceSelected(0);}

// 06 VUPanel — 电平表：暗轨道 + 频谱色填充条
void VUPanel::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    float s = (float)b.getWidth() / 440.0f;
    // 面板底
    g.setColour (juce::Colour::fromRGB (17, 18, 21));
    g.fillRoundedRectangle (b.toFloat(), 6.0f);
    g.setColour (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.50f)));
    g.drawRoundedRectangle (b.toFloat().reduced (1.0f), 6.0f, 2.0f);
    // 轨道
    auto track = b.reduced (juce::roundToInt (10 * s), juce::roundToInt (6 * s));
    g.setColour (juce::Colour::fromRGB (13, 14, 18));
    g.fillRoundedRectangle (track.toFloat(), 2.0f);
    // 填充
    auto sc = hm::spectrumColor (level_);
    float fillW = (float)track.getWidth() * level_;
    if (fillW > 0) {
        g.setColour (sc.withAlpha (0.75f));
        g.fillRoundedRectangle ((float)track.getX(), track.getY() - 1.0f, fillW, (float)track.getHeight() + 2.0f, 2.0f);
        // 光晕
        g.setColour (sc.withAlpha (0.20f));
        g.fillEllipse (track.getX() + fillW - 3.0f, track.getCentreY() - 4.0f, 10.0f, 8.0f);
    }
}

// 07 OverlayBPM
OverlayBPM::OverlayBPM():bpmData_(120){}
void OverlayBPM::setBPM(int b){bpmData_.setBPM(b);repaint();}
void OverlayBPM::paint(juce::Graphics& g){g.fillAll(shellCol(6));g.setColour(juce::Colours::white.withAlpha(0.5f));g.setFont(14);g.drawText("BPM "+juce::String(bpmData_.getBPM()),0,0,getWidth(),getHeight(),juce::Justification::centred);closeRect_={getWidth()-60,10,50,20};g.setColour(juce::Colours::white.withAlpha(0.3f));g.fillRoundedRectangle(closeRect_.toFloat(),4);g.setFont(10);g.drawText(juce::String::fromUTF8(u8"收起"),closeRect_,juce::Justification::centred);}
void OverlayBPM::mouseDown(const juce::MouseEvent& e){if(closeRect_.contains(e.getPosition())&&onClose)onClose();}

// 08 OverlayMonitor
void OverlayMonitor::paint(juce::Graphics& g){g.fillAll(shellCol(7));g.setColour(juce::Colours::white.withAlpha(0.5f));g.setFont(16);g.drawText(juce::String::fromUTF8(u8"抱歉打扰\n需要切换监听方式吗"),getLocalBounds().reduced(20),juce::Justification::centred);}
void OverlayMonitor::mouseDown(const juce::MouseEvent& e){if(onAction)onAction(3);}

// 09 OverlayFeedback
void OverlayFeedback::paint(juce::Graphics& g){g.fillAll(shellCol(8));g.setColour(juce::Colours::white.withAlpha(0.5f));g.setFont(14);g.drawText(juce::String::fromUTF8(u8"翻译度反馈"),getLocalBounds(),juce::Justification::centred);}
void OverlayFeedback::mouseDown(const juce::MouseEvent& e){if(onClose)onClose();}

// 10 Fader
Fader::Fader(){}
void Fader::setValue(float v){value_=juce::jlimit(0.f,200.f,v);repaint();}
void Fader::resized(){}
void Fader::updateFromMouse(int my){float frac=1.f-(float)(my-kCapH/2)/(float)(getHeight()-kCapH);value_=juce::jlimit(0.f,200.f,frac*200.f);if(onValueChanged)onValueChanged(value_);repaint();}
void Fader::mouseDown(const juce::MouseEvent& e){dragStartVal_=value_;dragStartY_=e.y;}
void Fader::mouseDrag(const juce::MouseEvent& e){float delta=(float)(dragStartY_-e.y)*kDamping;value_=juce::jlimit(0.f,200.f,dragStartVal_+delta);if(onValueChanged)onValueChanged(value_);repaint();}
void Fader::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& w){value_=juce::jlimit(0.f,200.f,value_+(w.deltaY>0?1.f:-1.f));if(onValueChanged)onValueChanged(value_);repaint();}
// 10 Fader — 垂直推子：暗轨 + 金属帽 + 金线 + 频谱色光晕
void Fader::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    float s = (float) b.getHeight() / 200.0f;
    // ── 轨道 ──
    int tx = (b.getWidth() - kTrackW) / 2;
    juce::ColourGradient trackGrad (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.55f)),
                                     (float)tx, 0, juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.28f)),
                                     (float)tx + kTrackW, 0, false);
    g.setGradientFill (trackGrad);
    g.fillRoundedRectangle ((float)tx, 0.0f, (float)kTrackW, (float)b.getHeight(), 3.0f);
    g.setColour (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.50f)));
    g.drawRoundedRectangle ((float)tx + 0.75f, 0.75f, (float)kTrackW - 1.5f, (float)b.getHeight() - 1.5f, 3.0f, 1.5f);

    // ── 频谱色填充 ──
    auto sc = hm::spectrumColor (value_ / 200.0f);
    float fillH = (float)b.getHeight() * value_ / 200.0f;
    if (fillH > 0) {
        g.setColour (sc.withAlpha (0.35f));
        g.fillRoundedRectangle ((float)tx, (float)b.getHeight() - fillH, (float)kTrackW, fillH, 3.0f);
    }

    // ── 推子帽 ──
    float capY = (float)b.getHeight() - fillH - kCapH / 2.0f;
    float cx = (float)(b.getWidth() - kCapW) / 2.0f;
    // 帽阴影
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (cx + 1.5f, capY + 1.5f, (float)kCapW, (float)kCapH, 6.0f);
    // 帽体渐变（深色金属）
    juce::ColourGradient capGrad (juce::Colour::fromRGB (42,40,37), cx, capY,
                                   juce::Colour::fromRGB (34,32,29), cx, capY + kCapH, false);
    capGrad.addColour (0.08f, juce::Colour::fromRGB (58,56,52));
    capGrad.addColour (0.20f, juce::Colour::fromRGB (78,76,72));
    capGrad.addColour (0.40f, juce::Colour::fromRGB (88,86,82));
    capGrad.addColour (0.60f, juce::Colour::fromRGB (66,64,60));
    capGrad.addColour (0.80f, juce::Colour::fromRGB (50,48,44));
    g.setGradientFill (capGrad);
    g.fillRoundedRectangle (cx, capY, (float)kCapW, (float)kCapH, 6.0f);
    // 帽边框
    g.setColour (juce::Colour::fromRGBA (20,18,15, static_cast<juce::uint8>(255*0.50f)));
    g.drawRoundedRectangle (cx + 0.5f, capY + 0.5f, (float)kCapW - 1.0f, (float)kCapH - 1.0f, 6.0f, 1.0f);
    // 金线
    g.setColour (juce::Colour (200,180,120).withAlpha (0.30f));
    g.fillRect (cx + kCapW / 2.0f - 4.5f, capY + 4.0f, 9.0f, (float)kCapH - 8.0f);
    // 光晕
    g.setColour (sc.withAlpha (0.12f));
    g.fillEllipse (cx - 6.0f, capY - 6.0f, (float)kCapW + 12.0f, (float)kCapH + 12.0f);
}

// 11 Cockpit — 内部布局由本类自行管理
Cockpit::Cockpit(){}
void Cockpit::resized(){}  // 子组件布局由 PluginEditor::resized() 跨级控制
// 11 Cockpit — 驾驶舱外壳：粗边框 + 内衬线 + 四角铆钉
void Cockpit::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    // 外层粗框（3px, dark）
    g.setColour (juce::Colour::fromRGBA (16,18,22, static_cast<juce::uint8>(255*0.90f)));
    g.drawRoundedRectangle (b.toFloat().reduced (1.5f), 10.0f, 3.0f);
    // 内衬线（inset 4px, 1px subtle）
    g.setColour (juce::Colours::white.withAlpha (0.008f));
    g.drawRoundedRectangle (b.toFloat().reduced (4.0f), 7.0f, 1.0f);
    // 四角铆钉（6px 圆，径向渐变金属色）
    static constexpr int kBolts = 4;
    int bx[kBolts] = {8, b.getRight()-14, 8, b.getRight()-14};
    int by[kBolts] = {8, 8, b.getBottom()-14, b.getBottom()-14};
    for (int i = 0; i < kBolts; ++i) {
        g.setColour (juce::Colour (150,155,165).withAlpha (0.40f));
        g.fillEllipse ((float)bx[i]-3, (float)by[i]-3, 6.0f, 6.0f);
        g.setColour (juce::Colour (130,135,145).withAlpha (0.30f));
        g.fillEllipse ((float)bx[i]-1.5f, (float)by[i]-1.5f, 3.0f, 3.0f);
    }
}

// 12 CurveGrid
void CurveGrid::paint(juce::Graphics& g){g.setColour(juce::Colour(180,200,230).withAlpha(0.03f));for(int i=1;i<=9;++i){float lx=getWidth()*i/10.f;g.fillRect(lx-0.5f,0.f,1.f,(float)getHeight());}g.setColour(juce::Colour(180,200,230).withAlpha(0.04f));for(int i=1;i<=7;++i){float ly=getHeight()*i/8.f;g.fillRect(0.f,ly-0.5f,(float)getWidth(),1.f);}}

// 12b CurveLabels
void CurveLabels::paint(juce::Graphics& g){float s=juce::jmax(1.f,getWidth()/430.f);float fs=juce::jmax(5.f,5.f*s);g.setFont(juce::FontOptions(fs));g.setColour(juce::Colour(210,225,248).withAlpha(0.75f));static const char* hz[]={"50","200","1k","5k","10k","20k"};for(int i=0;i<6;++i)g.drawText(juce::String::fromUTF8(hz[i]),i*getWidth()/6-14,getHeight()-10,28,8,juce::Justification::centred);static const char* db[]={"+12","+6","0","-6","-12"};for(int i=0;i<5;++i)g.drawText(juce::String::fromUTF8(db[i]),2,i*getHeight()/5,20,8,juce::Justification::centredLeft);}

// 13a CurveCanvas
void CurveCanvas::setRawCurve (const float* pts, int n) {
    rawYs_.clear();
    for (int i=0;i<n;++i) rawYs_.add(pts[i]);
    repaint();
}
// 13a CurveCanvas — 曲线画布
void CurveCanvas::paint (juce::Graphics& g) {
    if (rawYs_.isEmpty()) return;
    auto b=getLocalBounds();
    int w=b.getWidth(),h=b.getHeight(),n=rawYs_.size();
    float tgtY=66.0f; // 0dB 中心线
    juce::Path rawPath,corrPath;
    rawPath.startNewSubPath(0.0f,rawYs_[0]/132.0f*h);
    corrPath.startNewSubPath(0.0f,(tgtY+(rawYs_[0]-tgtY)*juce::jmax(0.0f,1.0f-degree_/100.0f))/132.0f*h);
    for(int i=1;i<n;++i){float x=(float)i/(n-1)*w;float ry=rawYs_[i]/132.0f*h;float cy=(tgtY+(rawYs_[i]-tgtY)*juce::jmax(0.0f,1.0f-degree_/100.0f))/132.0f*h;rawPath.lineTo(x,ry);corrPath.lineTo(x,cy);}
    if(viewMode_!=1){juce::Path rf(rawPath);rf.lineTo(w,(float)h);rf.lineTo(0,(float)h);rf.closeSubPath();g.setColour(juce::Colour(80,78,74).withAlpha(0.08f));g.fillPath(rf);g.setColour(juce::Colour(80,78,74).withAlpha(0.4f));g.strokePath(rawPath,juce::PathStrokeType(1.6f));}
    if(viewMode_!=2){auto sc=hm::spectrumColor(degree_/200.0f);juce::Path cf(corrPath);cf.lineTo(w,(float)h);cf.lineTo(0,(float)h);cf.closeSubPath();g.setColour(sc.withAlpha(0.10f));g.fillPath(cf);g.setColour(sc.withAlpha(0.6f));g.strokePath(corrPath,juce::PathStrokeType(2.0f));}
}
void CurveCanvas::mouseMove(const juce::MouseEvent& e){if(onHover)onHover((float)e.x,(float)e.y);}
void CurveCanvas::mouseExit(const juce::MouseEvent&){if(onLeave)onLeave();}
void CurveCanvas::mouseDown(const juce::MouseEvent&){viewMode_=(viewMode_+1)%3;repaint();}

// 13b StarField
StarField::StarField(){for(int i=0;i<45;++i)stars_.add({(float)(rand()%100)/100.f,(float)(rand()%100)/100.f,(float)(rand()%3+1)*0.3f,(float)(rand()%100)/100.f,(float)(rand()%100)/100.f*6.28f});startTimerHz(30);}
void StarField::timerCallback(){for(auto& s:stars_){s.phase+=0.018f;if(s.phase>6.283f)s.phase-=6.283f;s.bright=0.15f+0.85f*(std::sin(s.phase)*0.5f+0.5f);}repaint();}
void StarField::paint(juce::Graphics& g){for(auto& s:stars_){g.setColour(juce::Colours::white.withAlpha(s.bright*s.r*0.3f));float x=s.x*getWidth(),y=s.y*getHeight(),r=s.r;g.fillEllipse(x-r,y-r,r*2,r*2);}}
