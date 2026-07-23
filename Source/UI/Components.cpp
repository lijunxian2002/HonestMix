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
    // 底边线
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.fillRect ((float)b.getX(), (float)(b.getBottom()-1), (float)b.getWidth(), 1.0f);
    // 品牌名（9px, 字色 rgba(245,245,240,0.60)）
    g.setFont (juce::FontOptions (9.0f));
    g.setColour (juce::Colour (245, 245, 240).withAlpha (0.60f));
    g.drawText ("HONESTMIX", b.getX(), b.getY(), b.getWidth() - 40, b.getHeight(),
                juce::Justification::centredLeft);
    // 状态灯（4×4 圆，默认金属灰）
    float cx = (float)b.getRight() - 14.f, cy = (float)b.getCentreY();
    juce::Colour dotCol = dotCol_.isTransparent() ? juce::Colour (100,98,94).withAlpha (0.70f) : dotCol_;
    g.setColour (dotCol.withAlpha (dotCol.getAlpha() / 255.f * 0.3f));
    g.fillEllipse (cx - 4.f, cy - 4.f, 8.0f, 8.0f);
    g.setColour (dotCol);
    g.fillEllipse (cx - 2.f, cy - 2.f, 4.0f, 4.0f);
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
        g.setFont (juce::FontOptions (7.0f));
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

// 04 Breath
Breath::Breath(){startTimerHz(30);}
void Breath::timerCallback(){phase_+=0.05f;if(phase_>6.28f)phase_=0;repaint();}
void Breath::paint(juce::Graphics& g){float a=0.35f+0.65f*std::abs(std::sin(phase_));g.setColour(juce::Colours::white.withAlpha(a));g.fillEllipse(0,0,3,3);}

// 05 InfoRow
void InfoRow::setNames(const juce::String& h,const juce::String& c,const juce::String& i){hpName_=h;cvName_=c;ifName_=i;repaint();}
void InfoRow::resized(){int w=getWidth(),cw=(w-12)/3;hpChip_={0,0,cw,getHeight()};cvChip_={cw+6,0,cw,getHeight()};ifChip_={(cw+6)*2,0,cw,getHeight()};}
void InfoRow::paint(juce::Graphics& g){g.fillAll(shellCol(4));auto dc=[&](juce::Rectangle<int> r,const char* l,const juce::String& v){g.setColour(juce::Colours::white.withAlpha(0.2f));g.fillRoundedRectangle(r.toFloat(),5);g.setFont(6);g.setColour(juce::Colours::white.withAlpha(0.2f));g.drawText(juce::String::fromUTF8(l),r.getX(),r.getY(),r.getWidth(),12,juce::Justification::centred);g.setFont(9);g.setColour(juce::Colours::white.withAlpha(0.5f));g.drawText(v,r.getX(),r.getY()+12,r.getWidth(),r.getHeight()-12,juce::Justification::centred);};dc(hpChip_,"Headphone",hpName_);dc(cvChip_,"Target Curve",cvName_);dc(ifChip_,"Interface",ifName_);}
void InfoRow::mouseDown(const juce::MouseEvent& e){auto p=e.getPosition();if(hpChip_.contains(p)&&onProfileSelected)onProfileSelected(0);else if(cvChip_.contains(p)&&onCurveSelected)onCurveSelected(0);else if(ifChip_.contains(p)&&onInterfaceSelected)onInterfaceSelected(0);}

// 06 VUPanel
void VUPanel::paint(juce::Graphics& g){g.fillAll(shellCol(5));g.setColour(hm::spectrumColor(level_).withAlpha(0.6f));g.fillRoundedRectangle(4,(float)getHeight()*0.25f,(float)getWidth()-8,(float)getHeight()*0.5f*level_,2);}

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
void Fader::mouseDown(const juce::MouseEvent& e){updateFromMouse(e.y);}
void Fader::mouseDrag(const juce::MouseEvent& e){updateFromMouse(e.y);}
void Fader::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& w){value_=juce::jlimit(0.f,200.f,value_+(w.deltaY>0?5:-5));if(onValueChanged)onValueChanged(value_);repaint();}
void Fader::paint(juce::Graphics& g){g.fillAll(shellCol(9));int cx=(getWidth()-kTrackW)/2;g.setColour(juce::Colours::black.withAlpha(0.5f));g.fillRoundedRectangle((float)cx,0,(float)kTrackW,(float)getHeight(),3);float fh=getHeight()*value_/200.f;g.setColour(hm::spectrumColor(value_/200.f).withAlpha(0.5f));g.fillRoundedRectangle((float)cx,getHeight()-fh,(float)kTrackW,fh,3);float cy=getHeight()-fh-kCapH/2;g.setColour(juce::Colours::lightgrey);g.fillRoundedRectangle((float)(getWidth()-kCapW)/2,cy,(float)kCapW,(float)kCapH,5);}

// 11 Cockpit — 内部布局由本类自行管理
Cockpit::Cockpit(){}
void Cockpit::resized(){
    auto b = getLocalBounds().reduced (8);
    // 查找子组件并布局（按组装说明书 §3.3）
    for (auto* child : getChildren()) {
        auto name = child->getComponentID();
        if (name == "VUPanel")      child->setBounds (b.removeFromTop (22));
        // 其他子组件占满剩余区域
    }
    // 余下全给仪表盘子组件
    b.removeFromTop (6);
    for (auto* child : getChildren()) {
        auto name = child->getComponentID();
        if (name == "VUPanel") continue;
        if (name == "Breath") { child->setBounds (b.getRight()-20, b.getY()+10, 3, 3); continue; }
        if (name == "StarField") { child->setBounds (b.expanded(10)); continue; }
        if (name == "CurveCanvas") { child->setBounds (b.reduced(1)); continue; }
        child->setBounds (b);
    }
}
void Cockpit::paint(juce::Graphics& g){g.fillAll(shellCol(10));g.setColour(juce::Colours::white.withAlpha(0.1f));g.drawRect(getLocalBounds().reduced(3),3);}

// 12 CurveGrid
void CurveGrid::paint(juce::Graphics& g){g.setColour(juce::Colours::white.withAlpha(0.03f));for(int i=1;i<=3;++i){float lx=getWidth()*i/4.f;g.fillRect(lx-0.5f,0.f,1.f,(float)getHeight());float ly=getHeight()*i/4.f;g.fillRect(0.f,ly-0.5f,(float)getWidth(),1.f);}}

// 12b CurveLabels
void CurveLabels::paint(juce::Graphics& g){g.setColour(juce::Colours::white.withAlpha(0.3f));g.setFont(5);static const char* hz[]={"50","200","1k","5k","10k","20k"};for(int i=0;i<6;++i)g.drawText(juce::String::fromUTF8(hz[i]),i*getWidth()/6-10,getHeight()-10,24,8,juce::Justification::centred);static const char* db[]={"+12","+6","0","-6","-12"};for(int i=0;i<5;++i)g.drawText(juce::String::fromUTF8(db[i]),2,i*getHeight()/5,20,8,juce::Justification::centredLeft);}

// 13a CurveCanvas
void CurveCanvas::loadProfile(int){ /* TODO: FIR→rawYs */ repaint(); }
void CurveCanvas::setTargetCurve(int){ /* TODO */ repaint(); }
void CurveCanvas::paint(juce::Graphics& g){g.fillAll(juce::Colours::transparentBlack);}
void CurveCanvas::mouseMove(const juce::MouseEvent& e){if(onHover)onHover((float)e.x,(float)e.y);}
void CurveCanvas::mouseExit(const juce::MouseEvent&){if(onLeave)onLeave();}
void CurveCanvas::mouseDown(const juce::MouseEvent&){viewMode_=(viewMode_+1)%3;repaint();}

// 13b StarField
StarField::StarField(){for(int i=0;i<45;++i)stars_.add({(float)(rand()%100)/100.f,(float)(rand()%100)/100.f,(float)(rand()%3+1)*0.3f,(float)(rand()%100)/100.f,(float)(rand()%100)/100.f*6.28f});startTimerHz(30);}
void StarField::timerCallback(){for(auto& s:stars_){s.phase+=0.02f;if(s.phase>6.28f)s.phase=0;s.bright=0.2f+0.8f*std::abs(std::sin(s.phase));}repaint();}
void StarField::paint(juce::Graphics& g){for(auto& s:stars_){g.setColour(juce::Colours::white.withAlpha(s.bright*s.r*0.3f));float x=s.x*getWidth(),y=s.y*getHeight(),r=s.r;g.fillEllipse(x-r,y-r,r*2,r*2);}}
