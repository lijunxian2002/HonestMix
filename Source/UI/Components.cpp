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
    // 底边线 — 红→蓝渐变（用户反馈）
    juce::ColourGradient lineGrad(juce::Colour::fromRGBA(220,60,50,60),0,0,
                                    juce::Colour::fromRGBA(50,100,210,60),(float)b.getWidth(),0,false);
    g.setGradientFill(lineGrad);
    g.fillRect(0.f,(float)(b.getBottom()-2),(float)b.getWidth(),2.f);
    // 品牌名 — 优雅字距 + 柔和微光（安静、不抢眼）
    juce::String brand = "H O N E S T M I X";
    float fs = 12.0f;
    float by = (float)b.getCentreY() - 6.0f;
    g.setFont (juce::FontOptions (fs));
    g.setColour (juce::Colour (0, 0, 0).withAlpha (0.25f));
    g.drawText (brand, 7, juce::roundToInt (by + 1.0f), 160, juce::roundToInt (fs), juce::Justification::centredLeft);
    g.setColour (juce::Colour (245, 245, 240).withAlpha (0.75f));
    g.drawText (brand, 6, juce::roundToInt (by), 160, juce::roundToInt (fs), juce::Justification::centredLeft);
    // 状态灯 — 固定4px，不缩放
    float cx = (float)b.getRight() - 14.f, cy = (float)b.getCentreY();
    juce::Colour dotCol = dotCol_.isTransparent() ? juce::Colour (100,98,94).withAlpha (0.70f) : dotCol_;
    // HTML box-shadow: 0 0 4px rgba(80,78,74,0.3)
    g.setColour (juce::Colour (80,78,74).withAlpha (0.30f));
    g.fillEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f);
    g.setColour (dotCol);
    g.fillEllipse (cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
}

// 02 BottomRow — 2 HoverButton 子组件（BPM + 诚·反馈）
BottomRow::BottomRow(){
    btnBPM_.setText(juce::String::fromUTF8(u8"BPM 120"));
    btnFeedback_.setText(juce::String::fromUTF8(u8"诚·反馈"));btnFeedback_.setAccent(true);
    addAndMakeVisible(btnBPM_);addAndMakeVisible(btnFeedback_);
    btnBPM_.onClick=[this]{if(onBPM)onBPM();};btnFeedback_.onClick=[this]{if(onFeedback)onFeedback();};
}
void BottomRow::setBPMText(const juce::String& t){btnBPM_.setText(t);}
void BottomRow::resized(){int w=getWidth(),bw=(w-8)/2;btnBPM_.setBounds(0,0,bw,getHeight());btnFeedback_.setBounds(bw+8,0,bw,getHeight());}

// 03 Tooltip
void Tooltip::show (int x,int y,float f,float d,int m){visible_=true;tx_=x;ty_=y;freq_=f;db_=d;mode_=m;repaint();}
void Tooltip::hide(){visible_=false;repaint();}
void Tooltip::paint(juce::Graphics& g){
    if(!visible_)return;
    auto b=juce::Rectangle<int>(tx_,ty_,130,24);
    g.setColour(juce::Colour::fromRGBA(26,26,24,static_cast<juce::uint8>(255*0.96f)));
    g.fillRoundedRectangle(b.toFloat(),4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(b.toFloat().reduced(0.5f),4.0f,1.0f);
    g.setColour(mode_==0?juce::Colours::lime:juce::Colours::grey);
    g.fillEllipse((float)b.getX()+6,(float)b.getCentreY()-2,4,4);
    g.setFont(juce::FontOptions(9.0f));
    g.setColour(juce::Colour(220,220,215).withAlpha(0.80f));
    g.drawText(mode_==0?"CORR":"RAW",b.getX()+14,b.getY(),32,b.getHeight(),juce::Justification::centredLeft);
    // HTML: Hz文字 rgba(220,220,215,0.85), dB文字 rgba(235,235,230,0.62)
    g.setFont(juce::FontOptions(11.0f));
    g.setColour(juce::Colour(220,220,215).withAlpha(0.85f));
    g.drawText(juce::String(freq_,freq_>=1000?1:0)+(freq_>=1000?" kHz":" Hz"),b.getX()+46,b.getY(),42,b.getHeight(),juce::Justification::centredLeft);
    g.setColour(juce::Colour(235,235,230).withAlpha(0.62f));
    g.drawText("|  "+juce::String(db_,1)+" dB",b.getX()+82,b.getY(),46,b.getHeight(),juce::Justification::centredLeft);
}

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

// 05 InfoRow — 3 ChipWidget 子组件
InfoRow::InfoRow(){
    hpChip_.setLabel("Headphone");cvChip_.setLabel("Target Curve");
    addAndMakeVisible(hpChip_);addAndMakeVisible(cvChip_);
    hpChip_.onOpen=[this]{if(onProfileClick)onProfileClick();};
    cvChip_.onOpen=[this]{if(onCurveClick)onCurveClick();};
}
void InfoRow::setNames(const juce::String& h,const juce::String& c){hpChip_.setValue(h);cvChip_.setValue(c);}
void InfoRow::resized(){int w=getWidth(),cw=(w-6)/2;hpChip_.setBounds(0,0,cw,getHeight());cvChip_.setBounds(cw+6,0,cw,getHeight());}

// 06 VUPanel — 电平表：暗轨道 + 频谱色填充条
void VUPanel::paint (juce::Graphics& g) {
    auto b = getLocalBounds();
    float s = (float)b.getWidth() / 440.0f;
    // 面板底 — HTML: box-shadow: inset 0 1px 0 rgba(255,255,255,0.01), inset 0 2px 4px rgba(0,0,0,0.4)
    g.setColour (juce::Colour::fromRGB (17, 18, 21));
    g.fillRoundedRectangle (b.toFloat(), 6.0f);
    g.setColour(juce::Colours::white.withAlpha(0.01f));g.fillRect(b.getX()+2.f,b.getY()+1.f,b.getWidth()-4.f,1.f);
    g.setColour(juce::Colour::fromRGBA(0,0,0,102));g.fillRect(b.getX()+2.f,b.getY()+2.f,b.getWidth()-4.f,3.f);
    g.setColour (juce::Colour::fromRGBA (0,0,0, static_cast<juce::uint8>(255*0.50f)));
    g.drawRoundedRectangle (b.toFloat().reduced (1.0f), 6.0f, 2.0f);
    // 轨道 — HTML: box-shadow: inset 0 1px 3px rgba(0,0,0,0.6)
    auto track = b.reduced (juce::roundToInt (10 * s), juce::roundToInt (6 * s));
    g.setColour (juce::Colour::fromRGB (13, 14, 18));
    g.fillRoundedRectangle (track.toFloat(), 2.0f);
    // inset shadow 模拟
    g.setColour(juce::Colour::fromRGBA(0,0,0,153));g.fillRect((float)track.getX(),(float)track.getY(),(float)track.getWidth(),1.f);
    g.setColour(juce::Colour::fromRGBA(0,0,0,80));
    juce::ColourGradient tsGrad(juce::Colour::fromRGBA(0,0,0,100),0,(float)track.getY(),
                                  juce::Colour::fromRGBA(0,0,0,0),0,(float)track.getY()+4.f,false);
    g.setGradientFill(tsGrad);g.fillRect((float)track.getX(),(float)track.getY(),(float)track.getWidth(),4.f);
    // 填充 — 静流式：头部亮、向后渐隐拖影（低饱和低透明度）
    auto sc = hm::spectrumColor (level_);
    float fillW = (float)track.getWidth() * level_;
    if (fillW > 0) {
        juce::ColourGradient fillGrad (sc.withAlpha (0.34f), (float)track.getX() + fillW, 0.f,
                                       sc.withAlpha (0.02f), (float)track.getX(), 0.f, false);
        g.setGradientFill (fillGrad);
        g.fillRoundedRectangle ((float)track.getX(), track.getY() - 1.0f, fillW, (float)track.getHeight() + 2.0f, 2.0f);
        // 头部亮斑 + 柔光晕
        g.setColour (sc.withAlpha (0.40f));
        g.fillRoundedRectangle ((float)track.getX() + fillW - 2.0f, track.getY() - 1.0f, 2.0f, (float)track.getHeight() + 2.0f, 1.0f);
        g.setColour (sc.withAlpha (0.10f));
        g.fillEllipse (track.getX() + fillW - 4.0f, track.getCentreY() - 6.0f, 12.0f, 12.0f);
    }
}

// 07 OverlayBPM
OverlayBPM::OverlayBPM(){
    addAndMakeVisible(input_);input_.setMultiLine(false);input_.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),32.f,juce::Font::plain)));
    input_.setColour(juce::TextEditor::textColourId,juce::Colour(220,220,215).withAlpha(0.7f));
    input_.setColour(juce::TextEditor::backgroundColourId,juce::Colours::transparentBlack);
    input_.setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);
    input_.setInputRestrictions(3,"0123456789");input_.setJustification(juce::Justification::centred);
    // 输入时实时更新数据表
    input_.onTextChange=[this]{int v=input_.getText().getIntValue();if(v>=40&&v<=240){bpm_=v;repaint();}};
    input_.onReturnKey=[this]{int v=input_.getText().getIntValue();if(v>=40&&v<=240){bpm_=v;if(onClose)onClose();}else{input_.setText(juce::String(bpm_),juce::dontSendNotification);}repaint();};
    input_.onFocusLost=[this]{int v=input_.getText().getIntValue();if(v>=40&&v<=240){bpm_=v;}input_.setText(juce::String(bpm_),juce::dontSendNotification);repaint();};
}
void OverlayBPM::setBPM(int b){bpm_=juce::jlimit(40,240,b);if(!input_.hasKeyboardFocus(false))input_.setText(juce::String(bpm_),juce::dontSendNotification);repaint();}
void OverlayBPM::handleTap(){
    auto now=juce::Time::getCurrentTime();taps_.add(now);if(taps_.size()>8)taps_.remove(0);
    if(taps_.size()>=3){auto ms=(taps_.getLast().toMilliseconds()-taps_.getFirst().toMilliseconds())/(taps_.size()-1);if(ms>0)setBPM(juce::jlimit(40,240,(int)(60000.0/(double)ms+0.5)));}
    tapFlashMs_=now.toMilliseconds();repaint();
}
void OverlayBPM::resized(){auto b=getLocalBounds().reduced(20);input_.setBounds(b.getCentreX()-50,b.getY()+38,100,36);}
void OverlayBPM::paint(juce::Graphics& g){
    auto b=getLocalBounds().reduced(20);
    g.setColour(juce::Colours::black.withAlpha(0.55f));g.fillAll();
    // 深空渐变底 — 主页面风格
    juce::ColourGradient bgGrad(juce::Colour::fromRGB(17,18,24),b.getX(),b.getY(),
                                  juce::Colour::fromRGB(14,15,19),b.getX(),b.getBottom(),false);
    bgGrad.addColour(0.5f,juce::Colour::fromRGB(16,17,22));
    g.setGradientFill(bgGrad);g.fillRoundedRectangle(b.toFloat(),10.f);
    g.setColour(juce::Colour::fromRGBA(20,22,26,242));g.drawRoundedRectangle(b.reduced(1.5f).toFloat(),10.f,3.f);
    g.setColour(juce::Colour::fromRGBA(50,55,62,38));g.drawRoundedRectangle(b.reduced(0.5f).toFloat(),10.f,1.f);
    // 标题区 + 红→蓝渐变底边线
    closeRect_={b.getRight()-32,b.getY()+4,24,24};tapRect_={b.getCentreX()+70,b.getY()+38,90,36};
    float s=b.getWidth()/556.f; auto f=[&](float pt){return pt*s;};
    auto cjkFont=[&](float sz){return juce::Font(juce::FontOptions("Microsoft YaHei",f(sz),juce::Font::plain));};
    g.setFont(cjkFont(11));g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.drawText("BPM CALCULATOR",b.getX()+20,b.getY()+8,200,f(20),juce::Justification::left);
    juce::ColourGradient lineGrad(juce::Colour::fromRGBA(220,60,50,60),0,0,
                                    juce::Colour::fromRGBA(50,100,210,60),(float)b.getWidth(),0,false);
    g.setGradientFill(lineGrad);g.fillRect((float)b.getX(),(float)(b.getY()+f(28)),(float)b.getWidth(),2.f);
    // 关闭按钮
    g.setColour(juce::Colour::fromRGBA(255,255,255,10));g.fillRoundedRectangle(closeRect_.toFloat(),4.f);
    g.setColour(juce::Colour::fromRGBA(255,255,255,20));g.drawRoundedRectangle(closeRect_.toFloat().reduced(0.5f),4.f,1.f);
    g.setFont(cjkFont(12));g.setColour(juce::Colour(200,200,200).withAlpha(0.3f));g.drawText(juce::String::fromUTF8(u8"×"),closeRect_,juce::Justification::centred);
    // 输入框底 — 底部亮线风格（无框，只有一条微光底线）
    auto ir=input_.getBounds();
    g.setColour(juce::Colour::fromRGBA(0,0,0,60));g.fillRoundedRectangle(ir.toFloat(),4.f);
    juce::ColourGradient inGrad(juce::Colour::fromRGBA(220,60,50,50),ir.getX(),0,
                                  juce::Colour::fromRGBA(50,100,210,50),(float)ir.getRight(),0,false);
    g.setGradientFill(inGrad);g.fillRect((float)ir.getX(),(float)(ir.getBottom()-1),(float)ir.getWidth(),1.f);
    // TAP 按钮
    bool flash=tapFlashMs_>0&&(juce::Time::getCurrentTime().toMilliseconds()-tapFlashMs_)<200;
    if(flash){
        g.setColour(juce::Colour::fromRGBA(200,160,80,38));g.fillRoundedRectangle(tapRect_.toFloat(),6.f);
        g.setColour(juce::Colour::fromRGBA(200,160,80,77));g.drawRoundedRectangle(tapRect_.toFloat(),6.f,1.f);
    }else{
        g.setColour(juce::Colour::fromRGBA(0,0,0,51));g.fillRoundedRectangle(tapRect_.toFloat(),6.f);
        g.setColour(juce::Colour::fromRGBA(0,0,0,107));g.drawRoundedRectangle(tapRect_.toFloat(),6.f,1.f);
    }
    g.setFont(cjkFont(12));g.setColour(flash?juce::Colour(220,180,100).withAlpha(0.6f):juce::Colours::white.withAlpha(0.4f));g.drawText("TAP TEMPO",tapRect_,juce::Justification::centred);

    BPMData bpmData(bpm_);
    // ═══ 数据表 — 双列布局：左列 PREDELAY+REVERB，右列 DELAY ═══
    const int colW = (b.getWidth() - 24) / 2;
    const int rowH = f(22);
    auto sec=[&](int y,int x,int colw,const char* t,auto&& rows){
        g.setFont(cjkFont(11));g.setColour(juce::Colour(220,220,215).withAlpha(0.5f));
        g.drawText(juce::String::fromUTF8(t),b.getX()+x,y,colw,f(18),juce::Justification::left);
        int r=0;rows(r);
    };
    auto row=[&](int y,int x,int colw,const char* l,const juce::String& v1){
        g.setFont(cjkFont(10));g.setColour(juce::Colour(235,235,230).withAlpha(0.55f));
        g.drawText(juce::String::fromUTF8(l),b.getX()+x,y,colw/4,f(18),juce::Justification::left);
        g.setFont(cjkFont(13));g.setColour(juce::Colour(240,240,235).withAlpha(0.8f));
        g.drawText(v1,b.getX()+x+colw/4,y,colw/3,f(18),juce::Justification::right);
    };
    auto row2=[&](int y,int x,int colw,const char* l,const juce::String& v1,const juce::String& v2){
        g.setFont(cjkFont(10));g.setColour(juce::Colour(235,235,230).withAlpha(0.55f));
        g.drawText(juce::String::fromUTF8(l),b.getX()+x,y,colw/8,f(18),juce::Justification::left);
        g.setFont(cjkFont(13));g.setColour(juce::Colour(240,240,235).withAlpha(0.8f));
        g.drawText(v1,b.getX()+x+colw/8,y,colw/4,f(18),juce::Justification::right);
        if(v2.isNotEmpty()){
            g.drawText(v2,b.getX()+x+colw/2,y,colw/4,f(18),juce::Justification::right);
        }
    };
    // 左列：PREDELAY + REVERB（只显示标签和数值，无描述文字）
    int ly=b.getY()+f(84);
    sec(ly,12,colW,u8"PREDELAY",[&](int&r){
        row2(ly+=rowH,12,colW,u8"短",BPMData::fmtMs(bpmData.preShort()),"");
        row2(ly+=rowH,12,colW,u8"中",BPMData::fmtMs(bpmData.preMedium()),"");
        row2(ly+=rowH,12,colW,u8"长",BPMData::fmtMs(bpmData.preLong()),"");
    });
    ly+=f(12);
    sec(ly,12,colW,u8"REVERB",[&](int&r){
        auto pR=bpmData.roomReverb(),pP=bpmData.plateReverb(),pH=bpmData.hallReverb();
        row2(ly+=rowH,12,colW,u8"房",BPMData::fmtMs(pR.a),BPMData::fmtMs(pR.b));
        row2(ly+=rowH,12,colW,u8"板",BPMData::fmtMs(pP.a),BPMData::fmtMs(pP.b));
        row2(ly+=rowH,12,colW,u8"厅",BPMData::fmtMs(pH.a),BPMData::fmtMs(pH.b));
    });
    // 右列：DELAY
    int ry=b.getY()+f(84);
    sec(ry,12+colW+12,colW,u8"DELAY",[&](int&r){
        row(ry+=rowH,12+colW+12,colW,"1/2",BPMData::fmtMs(bpmData.delayHalf()));
        row(ry+=rowH,12+colW+12,colW,"1/4",BPMData::fmtMs(bpmData.delayQuarter()));
        row(ry+=rowH,12+colW+12,colW,"1/8",BPMData::fmtMs(bpmData.delayEighth()));
        row(ry+=rowH,12+colW+12,colW,"1/16",BPMData::fmtMs(bpmData.delay16th()));
    });
}
void OverlayBPM::mouseDown(const juce::MouseEvent& e){
    auto p=e.getPosition();
    auto cardArea=getLocalBounds().reduced(20);
    if(!cardArea.contains(p)){if(onClose)onClose();return;}
    if(closeRect_.contains(p)&&onClose){onClose();return;}
    if(tapRect_.contains(p)){handleTap();return;}
}

// 08 OverlayMonitor — 风格对齐 HTML
juce::Rectangle<int> OverlayMonitor::modeButtonRect (int i) const {
    // 单一来源：paint 和 mouseDown 都用这个函数（高 22，间距 30）
    auto card = getLocalBounds().reduced (getWidth() / 6, getHeight() / 5);
    return card.withSizeKeepingCentre (card.getWidth() - 60, 22).translated (0, (i - 2) * 30);
}

void OverlayMonitor::paint(juce::Graphics& g){
    auto b=getLocalBounds();
    g.setColour(juce::Colours::black.withAlpha(0.55f));g.fillAll();
    auto card=b.reduced(b.getWidth()/6,b.getHeight()/5);
    g.setColour(juce::Colour::fromRGB(18,20,26));g.fillRoundedRectangle(card.toFloat(),10);
    g.setColour(juce::Colours::white.withAlpha(0.06f));g.drawRoundedRectangle(card.toFloat().reduced(0.5f),10,1);
    // 标题
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",11.f,juce::Font::plain)));
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.drawText(juce::String::fromUTF8(u8"MONITOR MODE"),card.withSizeKeepingCentre(card.getWidth()-40,20).translated(0,-40),juce::Justification::centred);
    // 副标题
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",8.f,juce::Font::plain)));
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawText(juce::String::fromUTF8(u8"Select monitoring mode"),card.withSizeKeepingCentre(card.getWidth()-40,16).translated(0,-24),juce::Justification::centred);
    // 5 模式按钮
    static const char* opts[]={u8"STEREO",u8"MONO",u8"L ONLY",u8"R ONLY",u8"SIDE"};
    for(int i=0;i<5;++i){
        auto r=modeButtonRect(i).toFloat();
        g.setColour(juce::Colours::white.withAlpha(0.03f));g.fillRoundedRectangle(r.toFloat(),5);
        g.setColour(juce::Colours::white.withAlpha(0.08f));g.drawRoundedRectangle(r.toFloat(),5,1);
        g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",8.f,juce::Font::plain)));
        g.setColour(juce::Colours::white.withAlpha(0.35f));
        g.drawText(juce::String::fromUTF8(opts[i]),r,juce::Justification::centred);
    }
}
void OverlayMonitor::mouseDown(const juce::MouseEvent& e){
    auto p=e.getPosition();
    int act=-1;
    for(int i=0;i<5;++i)
        if(modeButtonRect(i).contains(p)){act=i;break;}
    if(act>=0){if(onAction) onAction(act);}
    else if(onClose) onClose();
}

// 09 OverlayFeedback
void OverlayFeedback::paint(juce::Graphics& g){
    auto b=getLocalBounds();
    g.setColour(juce::Colours::black.withAlpha(0.55f));g.fillAll();
    auto card=b.reduced(b.getWidth()/6,b.getHeight()/5);
    g.setColour(juce::Colour::fromRGB(18,20,26));g.fillRoundedRectangle(card.toFloat(),10);
    g.setColour(juce::Colours::white.withAlpha(0.06f));g.drawRoundedRectangle(card.toFloat().reduced(0.5f),10,1);
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",11.f,juce::Font::plain)));g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText(juce::String::fromUTF8(u8"翻译度反馈"),card.withTrimmedBottom(card.getHeight()-24).reduced(20,0),juce::Justification::centred);
    // HTML 副标题: "How does the correction sound?"
    g.setFont(juce::FontOptions(8.f));g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawText(juce::String::fromUTF8(u8"校正听起来怎么样？"),card.withTrimmedBottom(card.getHeight()-48).reduced(20,0).translated(0,22),juce::Justification::centred);
    static const char* bass[]={u8"低频刚好",u8"低频多了",u8"低频少了"};
    static const char* treble[]={u8"高频刚好",u8"高频亮了",u8"高频暗了"};
    for(int i=0;i<3;++i){
        auto rb=card.withSizeKeepingCentre(card.getWidth()-60,22).translated(-card.getWidth()/4+20,(-1+i)*28);
        g.setColour(bass_==i?juce::Colours::white.withAlpha(0.08f):juce::Colours::white.withAlpha(0.03f));
        g.fillRoundedRectangle(rb.toFloat(),5);
        g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",8.f,juce::Font::plain)));g.setColour(bass_==i?juce::Colours::white.withAlpha(0.55f):juce::Colours::white.withAlpha(0.25f));
        g.drawText(juce::String::fromUTF8(bass[i]),rb,juce::Justification::centred);
        auto rt=card.withSizeKeepingCentre(card.getWidth()-60,22).translated(card.getWidth()/4-20,(-1+i)*28);
        g.setColour(treble_==i?juce::Colours::white.withAlpha(0.08f):juce::Colours::white.withAlpha(0.03f));
        g.fillRoundedRectangle(rt.toFloat(),5);
        g.setColour(treble_==i?juce::Colours::white.withAlpha(0.55f):juce::Colours::white.withAlpha(0.25f));
        g.drawText(juce::String::fromUTF8(treble[i]),rt,juce::Justification::centred);
    }
    submitRect_=card.withSizeKeepingCentre(80,24).translated(-60,60);
    closeRect_=card.withSizeKeepingCentre(80,24).translated(60,60);
    g.setColour(juce::Colours::white.withAlpha(0.06f));g.fillRoundedRectangle(submitRect_.toFloat(),5);
    g.setColour(juce::Colours::white.withAlpha(0.06f));g.fillRoundedRectangle(closeRect_.toFloat(),5);
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",9.f,juce::Font::plain)));g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText(juce::String::fromUTF8(u8"提交"),submitRect_,juce::Justification::centred);
    g.drawText(juce::String::fromUTF8(u8"关闭"),closeRect_,juce::Justification::centred);
    // HTML 底部提示: "Feedback helps us improve"
    g.setFont(juce::FontOptions(9.f));g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawText(juce::String::fromUTF8(u8"反馈帮助我们改进"),card.withSizeKeepingCentre(160,14).translated(0,86),juce::Justification::centred);
}
void OverlayFeedback::mouseDown(const juce::MouseEvent& e){
    auto p=e.getPosition();
    if(submitRect_.contains(p)&&onSubmit){onSubmit(bass_,treble_);return;}
    if(closeRect_.contains(p)&&onClose){onClose();return;}
    // 点击低频/高频行
    auto card=getLocalBounds().reduced(getWidth()/6,getHeight()/5);
    for(int i=0;i<3;++i){
        auto rb=card.withSizeKeepingCentre(card.getWidth()-60,22).translated(-card.getWidth()/4+20,(-1+i)*28);
        auto rt=card.withSizeKeepingCentre(card.getWidth()-60,22).translated(card.getWidth()/4-20,(-1+i)*28);
        if(rb.contains(p)){bass_=i;repaint();return;}
        if(rt.contains(p)){treble_=i;repaint();return;}
    }
    if(onClose)onClose();
}

// 10 Fader
Fader::Fader(){}
void Fader::setValue(float v){value_=juce::jlimit(0.f,200.f,v);repaint();}
void Fader::resized(){}
void Fader::updateFromMouse(int my){float frac=1.f-(float)(my-kCapH/2)/(float)(getHeight()-kCapH);value_=juce::jlimit(0.f,200.f,frac*200.f);if(onValueChanged)onValueChanged(value_);repaint();}
void Fader::mouseDown(const juce::MouseEvent& e){dragStartVal_=value_;dragStartY_=e.y;}
void Fader::mouseDrag(const juce::MouseEvent& e){
    float damp = value_ < 100.f ? kDamping : kDamping * (200.f - value_) / 100.f; // ≥100 渐难
    float delta=(float)(dragStartY_-e.y)*juce::jmax(0.03f,damp);
    value_=juce::jlimit(0.f,200.f,dragStartVal_+delta);
    if(onValueChanged)onValueChanged(value_);repaint();
}
void Fader::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& w){value_=juce::jlimit(0.f,200.f,value_+(w.deltaY>0?5.f:-5.f));if(onValueChanged)onValueChanged(value_);repaint();}
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

    // ── 频谱色填充 — 自下而上渐隐（静流） ──
    auto sc = hm::spectrumColor (value_ / 200.0f);
    float fillH = (float)b.getHeight() * value_ / 200.0f;
    if (fillH > 0) {
        juce::ColourGradient fillGrad (sc.withAlpha (0.26f), (float)tx, (float)b.getHeight(),
                                       sc.withAlpha (0.02f), (float)tx, (float)b.getHeight() - fillH, false);
        g.setGradientFill (fillGrad);
        g.fillRoundedRectangle ((float)tx, (float)b.getHeight() - fillH, (float)kTrackW, fillH, 3.0f);
    }

    // ── 推子帽 ──
    // capY clamp：归零/拉满时帽体完整留在组件内（避免下半截被裁成"黑块"）
    float capY = juce::jlimit (0.0f, (float)b.getHeight() - kCapH,
                               (float)b.getHeight() - fillH - kCapH / 2.0f);
    float cx = (float)(b.getWidth() - kCapW) / 2.0f;
	    // 帽阴影 — 随帽宽收窄（两层右下偏移）
	    g.setColour (juce::Colours::black.withAlpha (0.42f));
	    g.fillRoundedRectangle (cx + 5.f, capY + 10.f, (float)kCapW - 10.f, (float)kCapH - 6.f, 6.0f);
	    g.setColour (juce::Colours::black.withAlpha (0.42f));
	    g.fillRoundedRectangle (cx + 3.f, capY + 5.f, (float)kCapW - 6.f, (float)kCapH - 3.f, 6.0f);
	    g.setColour (juce::Colours::black.withAlpha (0.42f));
	    g.fillRoundedRectangle (cx + 2.f, capY + 3.f, (float)kCapW - 4.f, (float)kCapH - 2.f, 6.0f);
		    // 帽顶高光 — HTML box-shadow: 0 1px 0 rgba(255,255,255,0.04)
		    g.setColour(juce::Colours::white.withAlpha(0.04f));
		    g.fillRoundedRectangle(cx+1.f,capY-1.f,(float)kCapW-2.f,2.f,3.f);
	    // 帽体渐变（深色金属） — HTML 7-stop
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
	    // 光晕 — 柔和弥散
	    g.setColour (sc.withAlpha (0.07f));
	    g.fillEllipse (cx - 6.0f, capY - 6.0f, (float)kCapW + 12.0f, (float)kCapH + 12.0f);

	    // ── 刻度标签 0 / 100(Harman) / 200 ── 双层绘制模拟辉光（与轨道留 2px 空隙）
	    float fh=(float)b.getHeight();int lxR=tx+kTrackW+4;
	    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",10.f,juce::Font::plain)));
	    g.setColour(juce::Colours::white.withAlpha(0.10f));
	    g.drawText("200",tx-26,0,24,12,juce::Justification::centredRight);
	    g.drawText("100",tx-26,(int)(fh*0.5f)-6,24,12,juce::Justification::centredRight);
	    g.drawText("0",  tx-26,(int)fh-14,24,12,juce::Justification::centredRight);
	    g.setColour(juce::Colours::white.withAlpha(0.40f));
	    g.drawText("200",tx-26,0,24,12,juce::Justification::centredRight);
	    g.drawText("100",tx-26,(int)(fh*0.5f)-6,24,12,juce::Justification::centredRight);
	    g.drawText("0",  tx-26,(int)fh-14,24,12,juce::Justification::centredRight);
	    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",8.f,juce::Font::plain)));g.setColour(juce::Colours::white.withAlpha(0.25f));
	    g.drawText(juce::String::fromUTF8(u8"过校正"),lxR,0,40,12,juce::Justification::left);
	    g.drawText(juce::String::fromUTF8(u8"Harman"),lxR,(int)(fh*0.5f)-6,40,12,juce::Justification::left);
	    g.drawText(juce::String::fromUTF8(u8"原声"),  lxR,(int)fh-14,40,12,juce::Justification::left);
	}

// 11 Cockpit — 移至 Source/UI/Cockpit.cpp

// 12 CurveGrid
void CurveGrid::paint(juce::Graphics& g){
    g.setColour(juce::Colour(180,200,230).withAlpha(0.03f));
    for(int i=1;i<=9;++i){float lx=getWidth()*i/10.f;g.fillRect(lx-0.5f,0.f,1.f,(float)getHeight());}
    g.setColour(juce::Colour(180,200,230).withAlpha(0.04f));
    for(int i=1;i<=7;++i){float ly=getHeight()*i/8.f;g.fillRect(0.f,ly-0.5f,(float)getWidth(),1.f);}
    // Harman 目标参考虚线 — HTML: rgba(190,185,175,0.15) 3/4 dash（±24dB 显示范围）
    juce::Path hp;bool fst=true;float w=(float)getWidth(),h=(float)getHeight();
    for(int i=0;i<256;++i){float f=20.f*std::pow(1000.f,(float)i/255.f);float hDb=hm::harmanDB(f);float x=std::log10(f/20.f)/std::log10(1000.f)*w;float y=(66.f-hDb*(66.f/24.f))/132.f*h;if(fst){hp.startNewSubPath(x,y);fst=false;}else hp.lineTo(x,y);}
    g.setColour(juce::Colour(190,185,175).withAlpha(0.15f));float dashes[2]={3.f,4.f};
    juce::Path dashedHp;juce::PathStrokeType(0.8f).createDashedStroke(dashedHp,hp,dashes,2);
    g.fillPath(dashedHp);
}

// 12b CurveLabels
void CurveLabels::paint(juce::Graphics& g){float fs=juce::jmax(8.f,8.f*getWidth()/430.f);g.setFont(juce::FontOptions(fs));
// glow层偏移模拟text-shadow
static const char* glowDB[]={"+24","+12","0","-12","-24"};g.setColour(juce::Colour(160,190,220).withAlpha(0.15f));for(int i=0;i<5;++i)g.drawText(juce::String::fromUTF8(glowDB[i]),4,i*getHeight()/5+1,24,11,juce::Justification::centredLeft);
// 主层 — 更亮
g.setColour(juce::Colour(210,230,245).withAlpha(0.80f));static const char* hz[]={"50","200","1k","5k","10k","20k"};for(int i=0;i<6;++i)g.drawText(juce::String::fromUTF8(hz[i]),i*getWidth()/6-14,getHeight()-12,32,10,juce::Justification::centred);static const char* db[]={"+24","+12","0","-12","-24"};for(int i=0;i<5;++i)g.drawText(juce::String::fromUTF8(db[i]),2,i*getHeight()/5,24,11,juce::Justification::centredLeft);
// 标题：偏离Harman目标（顶部居中，避开左右刻度）
g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",juce::jmax(9.f,9.f*getWidth()/430.f),juce::Font::plain)));g.setColour(juce::Colour(180,195,220).withAlpha(0.65f));g.drawText(juce::String::fromUTF8(u8"偏离Harman目标"),0,0,getWidth(),12,juce::Justification::centred);}

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
    float tgtY=66.0f; // 0dB 中心线（1kHz归零后Harman目标在此）
    // HTML: 0dB 参考虚线 rgba(190,185,175,0.15) stroke-dasharray:3,4
    float centerY = tgtY / 132.0f * h;
    g.setColour(juce::Colour(190,185,175).withAlpha(0.15f));
    float dashes[2] = {3.f, 4.f};
    juce::Path refLine;
    refLine.startNewSubPath(0.f, centerY);
    refLine.lineTo((float)w, centerY);
    juce::Path dashedRef;
    juce::PathStrokeType(0.8f).createDashedStroke(dashedRef, refLine, dashes, 2);
    g.fillPath(dashedRef);
    juce::Path rawPath,corrPath;
    rawPath.startNewSubPath(0.0f,rawYs_[0]/132.0f*h);
    float corrMult = degree_<=100.f ? (1.0f-degree_/100.f) : -(degree_-100.f)/400.f;
    corrPath.startNewSubPath(0.0f,(tgtY+(rawYs_[0]-tgtY)*corrMult)/132.0f*h);
    for(int i=1;i<n;++i){float x=(float)i/(n-1)*w;float ry=rawYs_[i]/132.0f*h;float cy=(tgtY+(rawYs_[i]-tgtY)*corrMult)/132.0f*h;rawPath.lineTo(x,ry);corrPath.lineTo(x,cy);}
    if(viewMode_!=1){juce::Path rf(rawPath);rf.lineTo(w,(float)h);rf.lineTo(0,(float)h);rf.closeSubPath();g.setColour(juce::Colour(80,78,74).withAlpha(0.08f));g.fillPath(rf);g.setColour(juce::Colour(80,78,74).withAlpha(0.4f));g.strokePath(rawPath,juce::PathStrokeType(1.6f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
    if(viewMode_!=2){auto sc=hm::spectrumColor(degree_/200.0f);juce::Path cf(corrPath);cf.lineTo(w,(float)h);cf.lineTo(0,(float)h);cf.closeSubPath();g.setColour(sc.withAlpha(0.05f));g.fillPath(cf);g.setColour(sc.withAlpha(0.35f));g.strokePath(corrPath,juce::PathStrokeType(2.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
    // 视图模式指示器 — 小卡片按钮（点击切换）
    static const char* modes[]={u8"BOTH",u8"CORR",u8"RAW"};
    auto modeCard=juce::Rectangle<int>(w-46,4,42,16);
    g.setColour(juce::Colour::fromRGBA(255,255,255,10));g.fillRoundedRectangle(modeCard.toFloat(),4.f);
    g.setColour(juce::Colour::fromRGBA(255,255,255,18));g.drawRoundedRectangle(modeCard.toFloat().reduced(0.5f),4.f,1.f);
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",9.f,juce::Font::plain)));
    g.setColour(juce::Colours::white.withAlpha(0.55f));
    g.drawText(juce::String::fromUTF8(modes[viewMode_]),modeCard,juce::Justification::centred);
}
void CurveCanvas::mouseMove(const juce::MouseEvent& e){if(onHover)onHover((float)e.x,(float)e.y);}
void CurveCanvas::mouseExit(const juce::MouseEvent&){if(onLeave)onLeave();}
void CurveCanvas::mouseDown(const juce::MouseEvent&){viewMode_=(viewMode_+1)%3;lastClickMs_=juce::Time::getCurrentTime().toMilliseconds();repaint();if(!isTimerRunning())startTimerHz(5);}
void CurveCanvas::timerCallback(){if(juce::Time::getCurrentTime().toMilliseconds()-lastClickMs_>5000){viewMode_=0;repaint();stopTimer();}}

// HeadphoneSearch
HeadphoneSearch::HeadphoneSearch(){
    addAndMakeVisible(input_);
    input_.setMultiLine(false); input_.setFont(juce::FontOptions(14.f));
    input_.setColour(juce::TextEditor::textColourId,juce::Colours::white.withAlpha(0.8f));
    input_.setColour(juce::TextEditor::backgroundColourId,juce::Colour::fromRGBA(30,32,40,200));
    input_.setColour(juce::TextEditor::outlineColourId,juce::Colours::white.withAlpha(0.1f));
    input_.onTextChange=[this]{
        if(!db_)return;
        juce::String f=input_.getText().toLowerCase(); results_.clear();
        if(f.isEmpty()){for(int i=0,n=juce::jmin(50,db_->getNumProfiles());i<n;++i)results_.push_back(i);}
        else db_->search(f,results_,60);
        repaint();
    };
}
void HeadphoneSearch::open(const HeadphoneDatabase& db,int cur){db_=&db;currentIdx_=cur;input_.clear();results_.clear();for(int i=0,n=juce::jmin(50,db_->getNumProfiles());i<n;++i)results_.push_back(i);setVisible(true);input_.grabKeyboardFocus();}
void HeadphoneSearch::resized(){input_.setBounds(8,8,getWidth()-16,30);}
void HeadphoneSearch::paint(juce::Graphics& g){
    int w=getWidth();
    g.setColour(juce::Colour::fromRGBA(18,20,26,250));g.fillRoundedRectangle(getLocalBounds().toFloat(),8.f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f),8.f,1.f);
    g.setFont(juce::Font(juce::FontOptions("Microsoft YaHei",11.f,juce::Font::plain)));
    int iy=46;
    for(int i=0;i<(int)results_.size()&&i<kVisRows;++i){
        int idx=results_[(size_t)i];
        auto r=juce::Rectangle<int>(12,iy,w-24,kRowH-2);
        if(idx==currentIdx_){g.setColour(juce::Colours::white.withAlpha(0.06f));g.fillRoundedRectangle(r.toFloat(),4.f);}
        g.setColour(idx==currentIdx_?juce::Colours::white.withAlpha(0.7f):juce::Colours::white.withAlpha(0.35f));
        g.drawText(juce::String(db_->getProfileName(idx)),r.getX()+8,r.getY(),r.getWidth()-16,r.getHeight(),juce::Justification::centredLeft);
        iy+=kRowH;
    }
}
void HeadphoneSearch::mouseDown(const juce::MouseEvent& e){
    auto p=e.getPosition(); int iy=46;
    for(int i=0;i<(int)results_.size()&&i<kVisRows;++i){
        if(juce::Rectangle<int>(12,iy,getWidth()-24,kRowH-2).contains(p)){if(onSelect)onSelect(results_[(size_t)i]);return;}
        iy+=kRowH;
    }
    if(!getLocalBounds().contains(p)&&onClose)onClose();
}


// 13a CurveCanvas — 曲线画布（RAW + CORR + 点击切换）