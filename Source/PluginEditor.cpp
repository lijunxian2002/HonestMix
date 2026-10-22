#include "PluginEditor.h"
#include "Updates/UpdateLink.h"
#include "UI/ResponsePlotModel.h"
#include <cmath>
#include <complex>
using namespace hm::skin;
namespace {
const juce::Colour accent(0xffcbdce7), warm(0xffd3b090);
const juce::Colour corrected(0xff8fbda2);
juce::Colour correctionColour(float degree){return degree>100?warm:degree==100?corrected:accent;}
float xFor(double hz){return 72.0f+(float)(std::log10(hz/20.0)/3.0)*640.0f;}
float yFor(double db){return 230.0f-(float)db*224.0f/48.0f;}
double gainDb(std::complex<double> z){return 20.0*std::log10(juce::jmax(1.0e-8,std::abs(z)));}
juce::String timeText(double ms){if(!std::isfinite(ms))return "--";return ms>=1000?juce::String(ms/1000.0,3)+" s":juce::String(ms,2)+" ms";}
const char* modes[]{"STEREO","MONO","HPF 400 Hz","LOW SHELF +6 dB"};
juce::String text8(const char* s){return juce::String::fromUTF8(s);}
}
void MechKey::paintButton(juce::Graphics& g,bool over,bool down){
 const auto r=getLocalBounds().toFloat();
 const bool shallow=(bool)getProperties().getWithDefault("shallow",false);
 if(shallow){
  g.setColour(juce::Colour(0xff11171b).withAlpha(over?.80f:.50f));g.fillRoundedRectangle(r,3);
  g.setColour(accent.withAlpha(over?.24f:.085f));g.drawRoundedRectangle(r.reduced(.5f),3,.7f);
 }else paintKey(g,r,getToggleState(),over,down);
 const auto secondary=getProperties().getWithDefault("secondaryLabel",juce::String()).toString();
 const int offset=down?1:0;
 if(secondary.isNotEmpty()&&getHeight()>=36){
  g.setFont(uiFont(13.5f).withExtraKerningFactor(.065f));g.setColour(isEnabled()?colour::textPrimary:colour::textDim);
  g.drawText(getButtonText(),6,6+offset,getWidth()-12,17,juce::Justification::centred);
  drawText(g,secondary,{6,25+offset,getWidth()-12,15},11.5f,colour::textSecondary,juce::Justification::centred);
 }else drawText(g,getButtonText(),getLocalBounds().reduced(12,2).translated(0,offset),12.5f,isEnabled()?colour::textPrimary:colour::textDim,juce::Justification::centred);
 if(getToggleState()&&!shallow){g.setColour(accent.withAlpha(.85f));g.fillEllipse(13,r.getCentreY()-1.5f,3,3);}
 if(hasKeyboardFocus(true)){g.setColour(accent.withAlpha(.65f));g.drawRoundedRectangle(r.reduced(2),3,.8f);}
}

MechFader::MechFader():juce::Slider(LinearVertical,TextBoxBelow){
 setRange(0,200,.5);setTextBoxStyle(TextBoxBelow,false,126,30);setTextValueSuffix(" %");
 setDoubleClickReturnValue(true,100);setWantsKeyboardFocus(true);setScrollWheelEnabled(true);
 setColour(textBoxTextColourId,colour::textPrimary);setColour(textBoxBackgroundColourId,juce::Colour(0xff090b0d));
 setColour(textBoxOutlineColourId,juce::Colour(0xff383e43));
 setTooltip({});
 setDescription("Correction amount: 0 original, 100 corrected, 100-200 overcorrection. Drag; Shift for fine adjustment. Double-click: 100.");
}
float MechFader::capY() const noexcept {return 20.0f+(1.0f-(float)getValue()/200.0f)*(getHeight()-80.0f);}
void MechFader::paint(juce::Graphics& g){
 const auto r=getLocalBounds().toFloat();const float cx=r.getCentreX(),top=20,travel=getHeight()-80.0f;
 const auto well=juce::Rectangle<float>(cx-51,4,102,getHeight()-44.0f);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff080b0d),well.getTopLeft(),juce::Colour(0xff151a1e),well.getBottomRight(),false));
 g.fillRoundedRectangle(well,6);
 g.setColour(juce::Colours::black.withAlpha(.85f));g.drawRoundedRectangle(well,6,1);
 g.setColour(accent.withAlpha(.08f));g.drawRoundedRectangle(well.reduced(1),5,.75f);
 const auto slot=juce::Rectangle<float>(cx-5.5f,top-6,11,travel+12);
 g.setColour(juce::Colour(0xff000102));g.fillRoundedRectangle(slot,5.5f);
 g.setColour(juce::Colour(0xff52616d).withAlpha(.4f));g.drawLine(cx+5.5f,top,cx+5.5f,top+travel,.7f);
 g.setColour(juce::Colour(0xff39454e).withAlpha(.4f));g.drawLine(cx-4.5f,top,cx-4.5f,top+travel,.6f);
 for(int v=0;v<=200;v+=10){
  const float y=top+(1-v/200.f)*travel;const bool major=v%50==0;
  g.setColour(v==100?corrected.withAlpha(.78f):colour::textSecondary.withAlpha(major?.64f:.25f));
  g.drawLine(cx-42-(major?5:0),y,cx-29,y,major?1.0f:.65f);
  g.drawLine(cx+29,y,cx+42+(major?5:0),y,major?1.0f:.65f);
  if(major)drawText(g,juce::String(v),{0,(int)y-8,37,16},10.5f,v==100?corrected:colour::textSecondary,juce::Justification::centredRight);
 }
 const auto cap=juce::Rectangle<float>(cx-34,capY()-12,68,24);
 g.setColour(juce::Colours::black.withAlpha(.80f));g.fillRoundedRectangle(cap.expanded(1).translated(0,3),4);
 juce::ColourGradient steel(juce::Colour(0xffb8bdc2),cap.getTopLeft(),juce::Colour(0xff33393f),cap.getBottomLeft(),false);
 steel.addColour(.18,juce::Colour(0xff858c92));steel.addColour(.48,juce::Colour(0xff697178));steel.addColour(.52,juce::Colour(0xff515a61));
 g.setGradientFill(steel);g.fillRoundedRectangle(cap,3);
 g.setColour(juce::Colour(0xffdfe9f0).withAlpha(.48f));g.drawRoundedRectangle(cap.reduced(.5f),2.5f,.75f);
 g.setColour(juce::Colours::black.withAlpha(.45f));g.drawLine(cap.getX()+4,cap.getBottom()-2,cap.getRight()-4,cap.getBottom()-2,.7f);
 for(float dy:{-3.f,0.f,3.f}){
  g.setColour(juce::Colour(0xff18232b).withAlpha(.7f));g.drawLine(cx-26,cap.getCentreY()+dy,cx+26,cap.getCentreY()+dy,.75f);
  g.setColour(juce::Colour(0xffe0eaf1).withAlpha(.16f));g.drawLine(cx-26,cap.getCentreY()+dy+.8f,cx+26,cap.getCentreY()+dy+.8f,.6f);
 }
 g.setColour(correctionColour((float)getValue()));g.fillRoundedRectangle(cx-9,cap.getCentreY()-.6f,18,1.2f,.5f);
 if(hasKeyboardFocus(true)){g.setColour(accent.withAlpha(.55f));g.drawRoundedRectangle(well.expanded(2),7,.8f);}
}

void MechFader::mouseDown(const juce::MouseEvent& e){if(!isEnabled()||!e.mods.isLeftButtonDown()||e.mods.isPopupMenu()||e.mods.isMiddleButtonDown()||dragging_)return;grabKeyboardFocus();startValue_=getValue();lastDragY_=0;dragging_=true;if(onDragStart)onDragStart();if(getPeer())e.source.enableUnboundedMouseMovement(true,false);}
void MechFader::mouseDrag(const juce::MouseEvent& e){if(!dragging_)return;double amount=e.mods.isShiftDown()?.05: .25;double y=e.getDistanceFromDragStartY();startValue_=juce::jlimit(0.0,200.0,startValue_-(y-lastDragY_)*amount);lastDragY_=y;setValue(startValue_,juce::sendNotificationSync);}
void MechFader::mouseUp(const juce::MouseEvent& e){if(!dragging_)return;dragging_=false;if(getPeer())e.source.enableUnboundedMouseMovement(false);if(onDragEnd)onDragEnd();}
void MechFader::mouseDoubleClick(const juce::MouseEvent& e){if(isEnabled()&&e.mods.isLeftButtonDown()&&!e.mods.isPopupMenu()&&!e.mods.isMiddleButtonDown())setValue(100,juce::sendNotificationSync);}
bool MechFader::keyPressed(const juce::KeyPress& k){if(!isEnabled())return false;double v=getValue();if(k.getKeyCode()==juce::KeyPress::homeKey)v=0;else if(k.getKeyCode()==juce::KeyPress::endKey)v=200;else if(k.getKeyCode()==juce::KeyPress::upKey||k.getKeyCode()==juce::KeyPress::rightKey)v+=.5;else if(k.getKeyCode()==juce::KeyPress::downKey||k.getKeyCode()==juce::KeyPress::leftKey)v-=.5;else return juce::Slider::keyPressed(k);setValue(v,juce::sendNotificationSync);return true;}
void MechFader::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& w){if(!isEnabled()||w.deltaY==0)return;setValue(getValue()+(w.deltaY>0?.5:-.5),juce::sendNotificationSync);}
void HonestMixAudioProcessorEditor::Look::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down){paintKey(g,b.getLocalBounds().toFloat(),b.getToggleState(),over,down);}
HonestMixAudioProcessorEditor::HonestMixAudioProcessorEditor(HonestMixAudioProcessor& p):AudioProcessorEditor(&p),proc_(p){
 // Start this editor's observation window without replaying a closed editor's peak.
 outputPeakResetGeneration_=proc_.getOutputPeakResetGeneration();proc_.consumeOutputPeak();
 setLookAndFeel(&look_);setWantsKeyboardFocus(true);setResizable(true,true);setResizeLimits(900,432,3000,1440);getConstrainer()->setFixedAspectRatio(1000.0/480.0);
 addAndMakeVisible(surface_);surface_.setSize(1000,480);
 for(auto* c:std::initializer_list<juce::Component*>{&profile_,&degree_,&compare_,&bpm_,&back_,&apply_,&source_,&straight_,&dotted_,&triplet_,&search_,&list_,&tempoInput_})surface_.addAndMakeVisible(c);
 auto identify=[](juce::Component& c,const char* id){c.setComponentID(id);c.setName(id);};
 identify(profile_,"profile");identify(degree_,"degree");identify(compare_,"compare");identify(bpm_,"tempo");identify(back_,"back");identify(apply_,"apply");identify(source_,"tempo-source");identify(straight_,"straight");identify(dotted_,"dotted");identify(triplet_,"triplet");
 compare_.getProperties().set("secondaryLabel",text8("原声对比"));bpm_.getProperties().set("secondaryLabel",text8("节拍时间"));
 back_.setButtonText(text8("BACK / 返回"));apply_.setButtonText(text8("USE / 使用此型号"));
 for(auto& point:starTrail_)point=chase_;
 profile_.getProperties().set("shallow",true);profile_.onClick=[this]{setView(View::browse);};profile_.setTooltip("Choose your exact headphone model");
 degree_.setValue(proc_.getTranslationDegree(),juce::dontSendNotification);
 degree_.onDragStart=[this]{proc_.beginTranslationGesture();processorGesture_=true;};
 degree_.onDragEnd=[this]{proc_.endTranslationGesture();processorGesture_=false;};
 degree_.onValueChange=[this]{proc_.setTranslationDegree((float)degree_.getValue());degree_.setValue(proc_.getTranslationDegree(),juce::dontSendNotification);refreshFromProcessor();};
 compare_.setTooltip(text8("Compare 0% with your saved correction amount. Legacy reference filters stay separate. / 对比0%与当前矫正强度。"));
 compare_.onClick=[this]{auto d=proc_.getTranslationDegree();if(d>0){previousDegree_=d;proc_.setTranslationDegree(0);}else proc_.setTranslationDegree(previousDegree_);refreshFromProcessor();};
 bpm_.onClick=[this]{setView(View::tempo);};
 back_.onClick=[this]{setView(View::normal);};apply_.onClick=[this]{selectProfile();};
 search_.setName("profile-search");search_.setMultiLine(false);search_.setTextToShowWhenEmpty("Search brand or exact model",colour::textDim);search_.onTextChange=[this]{filterProfiles();};search_.onReturnKey=[this]{selectProfile();};search_.setTitle("Headphone search");
 list_.setModel(this);list_.setRowHeight(32);list_.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff080a0c));list_.setColour(juce::ListBox::outlineColourId,colour::glassEdge);
 for(auto* input:{&search_,&tempoInput_}){input->setFont(uiFont(16));input->setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff080a0c));input->setColour(juce::TextEditor::textColourId,colour::textPrimary);input->setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff353d44));input->setColour(juce::TextEditor::focusedOutlineColourId,accent);}
 tempoInput_.setName("tempo-input");tempoInput_.setTitle("Manual BPM");tempoInput_.setInputRestrictions(12,"0123456789.");tempoInput_.setText("120",false);
 tempoInput_.onTextChange=[this]{if(!hostSync_)tempoSourceChosen_=true;auto t=tempoInput_.getText().trim();auto v=t.getDoubleValue();if(t.containsOnly("0123456789.")&&t.removeCharacters("0123456789").length()<=1&&hm::tempo::valid(v)){manualTempo_=v;manualText_=t;saveTempoPreferences();}surface_.repaint();};
 source_.onClick=[this]{hostSync_=!hostSync_;tempoSourceChosen_=true;if(!hostSync_)tempoInput_.setText(manualText_,false);saveTempoPreferences();refreshFromProcessor();};
 straight_.onClick=[this]{feel_=hm::tempo::Feel::straight;saveTempoPreferences();refreshFromProcessor();};dotted_.onClick=[this]{feel_=hm::tempo::Feel::dotted;saveTempoPreferences();refreshFromProcessor();};triplet_.onClick=[this]{feel_=hm::tempo::Feel::triplet;saveTempoPreferences();refreshFromProcessor();};
 restoreTempoPreferences();lastTick_=juce::Time::getMillisecondCounterHiRes();setSize(1000,480);setView(View::normal);refreshFromProcessor();startTimerHz(60);
}
HonestMixAudioProcessorEditor::~HonestMixAudioProcessorEditor(){stopTimer();if(processorGesture_)proc_.endTranslationGesture();list_.setModel(nullptr);setLookAndFeel(nullptr);}
void HonestMixAudioProcessorEditor::resized(){const float s=juce::jmin(getWidth()/1000.f,getHeight()/480.f);surface_.setTransform(juce::AffineTransform::scale(s));surface_.setTopLeftPosition(0,0);
 profile_.setBounds(42,70,688,30);degree_.setBounds(772,122,194,296);compare_.setBounds(keyBounds(1));
 bpm_.setBounds(keyBounds(0));
 back_.setBounds(34,70,100,30);apply_.setBounds(770,389,194,34);search_.setBounds(154,70,810,32);list_.setBounds(34,116,930,260);
 source_.setBounds(154,70,100,32);tempoInput_.setBounds(270,70,138,32);straight_.setBounds(558,70,124,32);dotted_.setBounds(694,70,124,32);triplet_.setBounds(830,70,134,32);
}
void HonestMixAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(colour::windowBackground);}
void HonestMixAudioProcessorEditor::paintSurfaceAtScale(juce::Graphics& g,int scale,juce::Rectangle<int>& painted){juce::Graphics::ScopedSaveState save(g);const float previous=renderScaleOverride_;renderScaleOverride_=(float)scale;setDrawingScale((float)scale);surface_.paintEntireComponent(g,true);renderScaleOverride_=previous;painted={0,0,1000*scale,480*scale};}
juce::Image HonestMixAudioProcessorEditor::renderSurface(int scale,juce::Rectangle<int>& painted){juce::Image image(juce::Image::ARGB,1000*scale,480*scale,true);juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale((float)scale));paintSurfaceAtScale(g,scale,painted);return image;}
void HonestMixAudioProcessorEditor::setView(View v){view_=v;pointerExit();graphOpacity_=0;const bool normal=v==View::normal,browse=v==View::browse,tempo=v==View::tempo;
 for(auto* c:std::initializer_list<juce::Component*>{&profile_,&degree_,&compare_,&bpm_})c->setVisible(normal);
 back_.setVisible(!normal);apply_.setVisible(browse);search_.setVisible(browse);list_.setVisible(browse);
 for(auto* c:std::initializer_list<juce::Component*>{&source_,&tempoInput_,&straight_,&dotted_,&triplet_})c->setVisible(tempo);
 if(browse){filterProfiles();search_.grabKeyboardFocus();}refreshFromProcessor();surface_.repaint();
}
bool HonestMixAudioProcessorEditor::keyPressed(const juce::KeyPress& k){if(k.getKeyCode()==juce::KeyPress::F10Key&&k.getModifiers().isShiftDown()){showListen();return true;}if(k.getKeyCode()==juce::KeyPress::escapeKey){setView(View::normal);return true;}return false;}
void HonestMixAudioProcessorEditor::mouseMove(const juce::MouseEvent& e){pointerMove(surface_.getLocalPoint(this,e.position));}
void HonestMixAudioProcessorEditor::mouseExit(const juce::MouseEvent&){pointerExit();}
void HonestMixAudioProcessorEditor::mouseDown(const juce::MouseEvent& e){surfaceClick(surface_.getLocalPoint(this,e.position),e.mods);}
void HonestMixAudioProcessorEditor::pointerMove(juce::Point<float> p){pointer_=p;pointerInside_=view_==View::normal&&juce::Rectangle<float>(34,62,704,322).contains(p);graphHovered_=view_==View::normal&&plotBounds().contains(p);if(reducedMotion_)graphOpacity_=graphHovered_?1.f:0.f;hoverHz_=plotBounds().contains(p)?20.0*std::pow(1000.0,(p.x-72)/640.0):0;if((responseState_.sampleRate>0&&hoverHz_>=responseState_.sampleRate*.5)||!hm::measurement::raw(responseState_.activeProfile).supports(hoverHz_))hoverHz_=0;surface_.repaint();}
void HonestMixAudioProcessorEditor::pointerExit(){hoverHz_=0;pointerInside_=false;graphHovered_=false;if(reducedMotion_)graphOpacity_=0;surface_.repaint();}
void HonestMixAudioProcessorEditor::surfaceClick(juce::Point<float> p,juce::ModifierKeys m){if(m.isPopupMenu())showListen();else if(m.isLeftButtonDown()&&view_==View::normal&&juce::Rectangle<float>(908,438,64,28).contains(p))showListen();}
void HonestMixAudioProcessorEditor::advanceAnimation(double dt){
 if(reducedMotion_){graphOpacity_=graphHovered_?1.f:0.f;return;}
 if(!std::isfinite(dt)||dt<=0)return;
 // Opacity belongs only to presentation. The immutable response and exact
 // frequency readout never depend on this animation or its visibility.
 const float opacityTarget=graphHovered_?1.f:0.f;
 graphOpacity_+=(opacityTarget-graphOpacity_)*(float)(1-std::exp(-dt/(graphHovered_?.18:.26)));
 if(std::abs(graphOpacity_-opacityTarget)<.0001f)graphOpacity_=opacityTarget;
 animationTime_+=dt;
 const auto target=pointerInside_?pointer_:juce::Point<float>(390,220);
 const auto goal=pointerInside_?(pointer_-juce::Point<float>(390,220))*.004f:juce::Point<float>();
 auto move=[&](double seconds){chase_+=(target-chase_)*(float)(1-std::exp(-seconds*2.4));parallax_+=(goal-parallax_)*(float)(1-std::exp(-seconds*1.8));};
 // Fixed-duration history samples keep the short trail independent of the
 // host repaint rate. A long stall skips only samples that will be replaced.
 constexpr double period=1.0/75.0;
 const double history=period*(starTrail_.size()+1);
 if(dt>history){const double skipped=std::floor((dt-history)/period)*period;move(skipped);dt-=skipped;}
 while(dt>0){
  const double step=juce::jmin(dt,period-trailSampleAccumulator_);move(step);dt-=step;trailSampleAccumulator_+=step;
  if(trailSampleAccumulator_>=period-1.0e-10){for(size_t i=starTrail_.size()-1;i>0;--i)starTrail_[i]=starTrail_[i-1];starTrail_[0]=chase_;trailSampleAccumulator_=0;}
 }
}
void HonestMixAudioProcessorEditor::timerCallback(){auto now=juce::Time::getMillisecondCounterHiRes();double dt=(now-lastTick_)/1000;lastTick_=now;if(!isShowing())return;advanceAnimation(dt);refreshAccumulator_+=dt;if(refreshAccumulator_>=.033){refreshAccumulator_=std::fmod(refreshAccumulator_,.033);refreshFromProcessor();}if(view_==View::normal&&!reducedMotion_)surface_.repaint(34,62,704,322);}
void HonestMixAudioProcessorEditor::paintStars(juce::Graphics& g){
 juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion({35,63,702,320});juce::Random r(0x5ab813);
 for(int i=0;i<126;++i){
  const float depth=.18f+r.nextFloat()*.82f;
  const float x=42+r.nextFloat()*686+(reducedMotion_?0:parallax_.x*depth*1.7f),y=66+r.nextFloat()*311+(reducedMotion_?0:parallax_.y*depth*1.7f);
  const float size=.55f+r.nextFloat()*1.15f;
  const float twinkle=reducedMotion_?.92f:.92f+.08f*std::sin((float)(animationTime_*(.18+depth*.13))+i*2.77f);
  const float alpha=(.22f+depth*.56f)*twinkle;
  g.setColour(accent.withAlpha(alpha));g.fillEllipse(x,y,size,size);
  if(i%23==0){g.setColour(accent.withAlpha(alpha*.45f));g.drawLine(x-2,y+.5f,x+3,y+.5f,.5f);g.drawLine(x+.5f,y-2,x+.5f,y+3,.5f);}
 }
 if(!reducedMotion_&&pointerInside_){
  auto previous=chase_;float remaining=8.f;
  for(size_t i=0;i<starTrail_.size()&&remaining>0;++i){
   const auto delta=starTrail_[i]-previous;const float distance=delta.getDistanceFromOrigin();if(distance<.001f)continue;
   const float length=juce::jmin(distance,remaining);const auto end=previous+delta*(length/distance);
   g.setColour(accent.withAlpha(.045f*(1-(float)i/starTrail_.size())));g.drawLine(previous.x,previous.y,end.x,end.y,.55f);
   remaining-=length;previous=end;
  }
  g.setColour(accent.withAlpha(.20f));g.fillEllipse(chase_.x-.65f,chase_.y-.65f,1.3f,1.3f);
 }
}

void HonestMixAudioProcessorEditor::paintSurface(juce::Graphics& g){setDrawingScale(renderScaleOverride_>0?renderScaleOverride_:(float)getWidth()/1000);paintWindowBackdrop(g,{0,0,1000,480});paintChassisPlate(g,{8,8,984,460},false);paintBezel(g,{8,8,984,460});
 hLine(g,26,974,52,.7f,juce::Colours::black.withAlpha(.85f));hLine(g,26,974,53,.6f,accent.withAlpha(.08f));
 hLine(g,26,974,440,.7f,juce::Colours::black.withAlpha(.85f));hLine(g,26,974,441,.6f,accent.withAlpha(.07f));
 g.setColour(colour::textPrimary);g.setFont(uiFont(16.0f).withExtraKerningFactor(.16f));g.drawText("HONESTMIX",34,19,300,26,juce::Justification::centredLeft);
 g.setColour(colour::textSecondary);g.setFont(uiFont(12.5f).withExtraKerningFactor(.13f));g.drawText("HEADPHONE",758,19,206,26,juce::Justification::centredRight);
 if(juce::Time::getMillisecondCounterHiRes()<updateNoticeUntil_)drawText(g,updateNotice_,{340,21,380,22},10.5f,colour::textSecondary,juce::Justification::centred);
 if(view_==View::normal){paintGlass(g,{26,62,720,322});paintStars(g);paintPlot(g);
 const juce::Rectangle<float> faderBay(766,62,208,373);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff101214),faderBay.getTopLeft(),juce::Colour(0xff080a0c),faderBay.getBottomRight(),false));g.fillRoundedRectangle(faderBay,3);
 paintInsetSeam(g,faderBay);hLine(g,770,970,62.7f,.7f,accent.withAlpha(.15f));
 const auto d=proc_.getTranslationDegree();
 drawText(g,JucePlugin_VersionString,{916,445,48,16},10,colour::textDim,juce::Justification::centredRight);
 auto mode=proc_.getListenMode();drawText(g,text8("ESTIMATED / 预计频响"),{34,442,260,20},10.5f,colour::textSecondary);
 juce::String state=responseState_.bypassed?"BYPASSED":!proc_.isAudioReady()?"AUDIO INACTIVE":responseState_.profileError?"PROFILE LOAD FAILED":(responseState_.requestedProfile<0||responseError_.isNotEmpty())?"RESPONSE UNAVAILABLE":basisJob_.valid()?"BUILDING RESPONSE":responseState_.profilePhase!=ProfilePhase::Stable?"LOADING PROFILE":responseState_.degreeTransition?"ADJUSTING":d>100?"OVER 100%": "TARGET: HARMAN 2018";if(mode!=ListenNormal)state=juce::String("LEGACY FILTER: ")+modes[(int)mode];drawText(g,state,{314,442,460,20},10.5f,colour::textSecondary,juce::Justification::centred);
 drawText(g,heldOutputPeak_>1?"OUT PK +"+juce::String(20*std::log10(heldOutputPeak_),1)+" dBFS":"",{738,442,160,20},10.5f,warm,juce::Justification::centredRight);
 }else if(view_==View::tempo)paintTempo(g);else{paintGlass(g,{26,110,948,274});drawText(g,selectionName_,{36,390,714,32},13,colour::textSecondary);drawText(g,results_.empty()?"No matching model. Choose an exact match.":"HEADPHONE LIBRARY",{34,442,850,20},11,colour::textSecondary);}
}
void HonestMixAudioProcessorEditor::paintPlot(juce::Graphics& g){auto p=plotBounds();
 for(int db=-24;db<=24;db+=6){float y=yFor(db);g.setColour(juce::Colours::white.withAlpha(db==0?.17f:.055f));g.drawLine(p.getX(),y,p.getRight(),y,db==0?1:.6f);drawText(g,juce::String(db),{32,(int)y-7,30,14},10,colour::textSecondary,juce::Justification::centredRight);}
 const int ticks[]{20,50,100,200,500,1000,2000,5000,10000,20000};for(auto hz:ticks){float x=xFor(hz);g.setColour(juce::Colours::white.withAlpha(.055f));g.drawLine(x,p.getY(),x,p.getBottom(),.6f);juce::String s=hz>=1000?juce::String(hz/1000)+"k":juce::String(hz);drawText(g,s,{(int)x-18,350,36,17},10,colour::textSecondary,juce::Justification::centred);}
 if(responseReady_&&graphOpacity_>0&&targetCurve_.size()>1){juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(p.toNearestInt());juce::Path target;target.startNewSubPath(targetCurve_[0]);for(size_t i=1;i<targetCurve_.size();++i)target.lineTo(targetCurve_[i]);g.setColour(colour::textSecondary.withAlpha(.32f*graphOpacity_));const float dashes[]{3.f,4.f};juce::Path dashed;juce::PathStrokeType(.8f).createDashedStroke(dashed,target,dashes,2);g.fillPath(dashed);}
 if(responseReady_&&(curveBelowScale_||curveAboveScale_))drawText(g,curveBelowScale_?(curveAboveScale_?"OUTSIDE +/-24 dB":"BELOW -24 dB"):"ABOVE +24 dB",{76,322,220,16},9.5f,colour::textSecondary);
 if(responseReady_&&graphOpacity_>0&&curve_.size()>1){juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(p.expanded(1).toNearestInt());juce::Path path;path.startNewSubPath(curve_[0]);for(size_t i=1;i<curve_.size();++i)path.lineTo(curve_[i]);g.setColour(correctionColour(appliedDegree_).withAlpha(graphOpacity_));g.strokePath(path,juce::PathStrokeType(1.25f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
 if(hoverHz_>0&&responseReady_){/* exact response readout is populated from the same immutable snapshot as the curve */
 const double db=displayedResponseDb(hoverHz_);float x=xFor(hoverHz_),y=juce::jlimit(p.getY(),p.getBottom(),yFor(db));g.setColour(accent.withAlpha(.35f));g.drawLine(x,p.getY(),x,p.getBottom(),.7f);g.drawLine(p.getX(),y,p.getRight(),y,.7f);g.setColour(accent);g.fillEllipse(x-2.5f,y-2.5f,5,5);juce::String text=(hoverHz_>=1000?juce::String(hoverHz_/1000,2)+" kHz":juce::String(hoverHz_,1)+" Hz")+"  /  "+(std::isinf(db)?juce::String("-inf"):juce::String(db,2))+" dB est.";drawText(g,text,{452,101,260,16},11,accent,juce::Justification::centredRight);}
 drawText(g,"HEADPHONE dB / original 1 kHz ref.",{76,101,360,16},10,colour::textSecondary);
 drawText(g,"dB",{40,101,30,16},9.5f,colour::textDim);
}
double HonestMixAudioProcessorEditor::displayedTempo() const noexcept {if(hostSync_){auto host=proc_.getHostBpmSnapshot();return host.valid&&host.fresh&&host.calculable?host.bpm:std::numeric_limits<double>::quiet_NaN();}auto t=tempoInput_.getText().trim();return t.isNotEmpty()&&t.containsOnly("0123456789.")&&t.removeCharacters("0123456789").length()<=1&&hm::tempo::valid(t.getDoubleValue())?t.getDoubleValue():std::numeric_limits<double>::quiet_NaN();}
void HonestMixAudioProcessorEditor::paintTempo(juce::Graphics& g){
 const auto bpm=displayedTempo();auto t=hm::tempo::calculate(bpm);paintGlass(g,{26,118,948,304});
 auto title=[&](const juce::String& s,int x,int y,int w){drawText(g,s,{x,y,w,23},12.5f,accent,juce::Justification::centredLeft,true);g.setColour(colour::textSecondary.withAlpha(.18f));g.drawLine((float)x,(float)y+27,(float)(x+w),(float)y+27,.7f);};
 auto row=[&](const juce::String& label,const juce::String& value,int x,int y,int w){drawText(g,label,{x,y,84,23},13.5f,colour::textSecondary);drawText(g,value,{x+80,y,w-80,23},14.0f,colour::textPrimary,juce::Justification::centredRight);};
 auto pair=[&](hm::tempo::Pair v){return timeText(v.low)+" / "+timeText(v.high);};
 title("PRE DELAY",46,132,278);row("Short",timeText(t.preShort),46,166,278);row("Medium",timeText(t.preMedium),46,194,278);row("Long",timeText(t.preLong),46,222,278);
 title("REVERB",46,266,278);row("Room",pair(t.room),46,300,278);row("Plate",pair(t.plate),46,329,278);row("Hall",pair(t.hall),46,358,278);
 title("COMP RELEASE",364,132,278);row("Fast",pair(t.fast),364,166,278);row("Medium",pair(t.medium),364,204,278);row("Slow",pair(t.slow),364,242,278);
 title("DELAY",686,132,262);const int den[]{2,4,8,16,32,64};for(int i=0;i<6;++i)row("1/"+juce::String(den[i]),timeText(hm::tempo::note(bpm,den[i],feel_)),686,166+i*37,262);
 auto snap=proc_.getHostBpmSnapshot();juce::String status=hostSync_?(!snap.valid?"NO HOST TEMPO":!snap.fresh?"HOST TEMPO STALE":!snap.calculable?"HOST TEMPO OUT OF RANGE":"HOST SYNC  /  "+juce::String(bpm,3)+" BPM"):std::isfinite(bpm)?"MANUAL  /  "+juce::String(bpm,3)+" BPM":"ENTER BPM 1-1000";
 drawText(g,status,{34,442,520,20},11,colour::textSecondary);drawText(g,"Tempo-based suggestions",{540,442,424,20},10,colour::textDim,juce::Justification::centredRight);
}
juce::PopupMenu HonestMixAudioProcessorEditor::makeOptionsMenu()const{
 juce::PopupMenu menu,legacy;
 legacy.addItem(1,text8("Normal stereo / 正常立体声"),true,proc_.getListenMode()==ListenNormal);
 legacy.addItem(2,text8("Mono / 单声道"),true,proc_.getListenMode()==ListenMono);
 legacy.addItem(3,text8("Legacy 400 Hz high-pass / 旧版高通"),true,proc_.getListenMode()==ListeniPhone);
 legacy.addItem(4,text8("Legacy +6 dB low shelf / 旧版低架"),true,proc_.getListenMode()==ListenCar);
 menu.addSubMenu(text8("Legacy reference filters / 旧版参考滤镜"),legacy);
 menu.addItem(10,text8("Reduce motion / 减少动态"),true,reducedMotion_);
 menu.addItem(20,text8("Check for updates / 检查更新"));
 menu.addItem(21,text8("Report a problem / 报告问题"));return menu;
}
void HonestMixAudioProcessorEditor::applyOption(int id){
 if(id==20){openUpdateWebsite();return;}
 if(id==21){openFeedbackWebsite();return;}
 if(id>=1&&id<=4)proc_.setListenMode((ListenMode)(id-1));
 if(id==10){reducedMotion_=!reducedMotion_;graphOpacity_=graphHovered_?1.f:0.f;parallax_={};chase_=pointerInside_?pointer_:juce::Point<float>(390,220);starTrail_.fill(chase_);trailSampleAccumulator_=0;}refreshFromProcessor();
}
void HonestMixAudioProcessorEditor::openUpdateWebsite(){
 if((bool)getProperties().getWithDefault("headlessTest",false))return;
 const bool opened=hm::updates::openWebsite("headphone",JucePlugin_VersionString);
 updateNotice_=opened?text8("Update page opened / 已打开更新页"):text8("Open HONESTMIX website / 请访问官网");
 updateNoticeUntil_=juce::Time::getMillisecondCounterHiRes()+4000;surface_.repaint();
}
void HonestMixAudioProcessorEditor::openFeedbackWebsite(){
 if((bool)getProperties().getWithDefault("headlessTest",false))return;
 const bool opened=hm::updates::openFeedbackWebsite("headphone",JucePlugin_VersionString);
 updateNotice_=opened?text8("Feedback page requested / 已请求打开反馈页"):text8("Open HONESTMIX feedback / 请访问官网反馈页");
 updateNoticeUntil_=juce::Time::getMillisecondCounterHiRes()+4000;surface_.repaint();
}
void HonestMixAudioProcessorEditor::showListen(){
 auto menu=makeOptionsMenu();juce::Component::SafePointer<HonestMixAudioProcessorEditor> safe(this);
 menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&surface_).withMousePosition(),[safe](int id){if(safe&&id>0)safe->applyOption(id);});
}
void HonestMixAudioProcessorEditor::filterProfiles(){results_.clear();auto q=search_.getText().trim();auto& db=proc_.getDatabase();if(q.isEmpty()){for(int i=0;i<db.getNumProfiles();++i)results_.push_back(i);}else db.search(q,results_);list_.deselectAllRows();list_.updateContent();if(!results_.empty())list_.selectRow(0);apply_.setEnabled(!results_.empty());selectionName_=results_.empty()?"No matching headphone":db.getProfileName(results_[0]);surface_.repaint();}
void HonestMixAudioProcessorEditor::selectedRowsChanged(int row){if(row>=0&&row<(int)results_.size())selectionName_=proc_.getDatabase().getProfileName(results_[(size_t)row]);surface_.repaint();}
void HonestMixAudioProcessorEditor::selectProfile(){auto row=list_.getSelectedRow();if(row<0||row>=(int)results_.size())return;proc_.setProfileIndex(results_[(size_t)row]);setView(View::normal);}
void HonestMixAudioProcessorEditor::paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected){if(row<0||row>=(int)results_.size())return;if(selected)g.fillAll(juce::Colour(0xff202a31));drawText(g,proc_.getDatabase().getProfileName(results_[(size_t)row]),{10,0,w-20,h},13,selected?colour::textPrimary:colour::textSecondary);}
void HonestMixAudioProcessorEditor::rebuildResponse(){
 curve_.clear();targetCurve_.clear();curveBelowScale_=curveAboveScale_=false;responseReady_=responseState_.audioReady&&(responseState_.bypassed||(proc_.getListenMode()==ListenNormal&&!responseState_.modeTransition));appliedDegree_=responseState_.appliedDegree;
 if(!responseReady_)return;
 if(basisJob_.valid()&&basisJob_.wait_for(std::chrono::seconds(0))==std::future_status::ready){try{basis_=basisJob_.get();responseError_.clear();}catch(const std::exception& e){responseError_=e.what();responseReady_=false;return;}}
 const bool matched=basis_.generation==responseState_.generation&&basis_.incomingGeneration==responseState_.incomingGeneration&&basis_.sampleRate==responseState_.sampleRate;
 if(!matched&&!basisJob_.valid()&&(responseError_.isEmpty()||basisGeneration_!=responseState_.generation||basisIncoming_!=responseState_.incomingGeneration||basisRequestRate_!=responseState_.sampleRate)){auto snapshot=responseState_;basisGeneration_=snapshot.generation;basisIncoming_=snapshot.incomingGeneration;basisRequestRate_=snapshot.sampleRate;responseError_.clear();basisJob_=std::async(std::launch::async,[snapshot]{return hm::prediction::build(snapshot);});}
 responseReady_=matched&&!basis_.frequencies.empty();if(!responseReady_)return;
 curve_.reserve(basis_.frequencies.size());targetCurve_.reserve(basis_.frequencies.size());for(size_t i=0;i<basis_.frequencies.size();++i){const double db=basis_.evaluate(i,responseState_.appliedDegree,responseState_.profileBlend,responseState_.bypassed);if(std::isnan(db))continue;curveBelowScale_|=db< -24;curveAboveScale_|=db>24;curve_.push_back({xFor(basis_.frequencies[i]),yFor(std::isfinite(db)?db:-300)});targetCurve_.push_back({xFor(basis_.frequencies[i]),yFor(basis_.target[i])});}
 // Preserve first/last and both extrema in every quarter-pixel column. Data
 // values are never averaged; exact hover still evaluates the immutable IR.
 std::vector<juce::Point<float>> envelope;envelope.reserve(9000);
 for(size_t i=0;i<curve_.size();){size_t end=i+1,lo=i,hi=i;const int column=(int)std::floor(curve_[i].x*4);while(end<curve_.size()&&(int)std::floor(curve_[end].x*4)==column){if(curve_[end].y<curve_[lo].y)lo=end;if(curve_[end].y>curve_[hi].y)hi=end;++end;}std::array<size_t,4> indices{{i,lo,hi,end-1}};std::sort(indices.begin(),indices.end());size_t previous=~size_t(0);for(auto at:indices)if(at!=previous){envelope.push_back(curve_[at]);previous=at;}i=end;}
 curve_.swap(envelope);
}
void HonestMixAudioProcessorEditor::saveTempoPreferences(){
 HonestMixAudioProcessor::TempoPreferences p;p.manualText=manualText_;p.source=tempoSourceChosen_?(hostSync_?1:0):-1;p.feel=(int)feel_;
 tempoPreferencesGeneration_=proc_.setTempoPreferences(p).generation;
}
void HonestMixAudioProcessorEditor::restoreTempoPreferences(){
 const auto p=proc_.getTempoPreferences();if(p.generation==tempoPreferencesGeneration_)return;
 manualText_=p.manualText;manualTempo_=manualText_.getDoubleValue();tempoSourceChosen_=p.source>=0;
 hostSync_=p.source<0?proc_.getHostBpmSnapshot().valid:p.source==1;feel_=(hm::tempo::Feel)p.feel;
 if(!hostSync_)tempoInput_.setText(manualText_,false);tempoPreferencesGeneration_=p.generation;
}
void HonestMixAudioProcessorEditor::refreshFromProcessor(){
 restoreTempoPreferences();
 const auto peakNow=juce::Time::getMillisecondCounterHiRes();
 const auto peakResetGeneration=proc_.getOutputPeakResetGeneration();
 if(peakResetGeneration!=outputPeakResetGeneration_){heldOutputPeak_=0;outputPeakHoldUntil_=0;outputPeakResetGeneration_=peakResetGeneration;}
 const auto observedPeak=proc_.consumeOutputPeak();
 if(!proc_.isAudioReady()){heldOutputPeak_=0;outputPeakHoldUntil_=0;}
 else{
  if(peakNow>=outputPeakHoldUntil_)heldOutputPeak_=0;
  if(observedPeak>1&&observedPeak>=heldOutputPeak_){heldOutputPeak_=observedPeak;outputPeakHoldUntil_=peakNow+1500;}
 }
 auto target=proc_.getTranslationDegree();if(!degree_.dragging())degree_.setValue(target,juce::dontSendNotification);
 degree_.setColour(juce::Slider::textBoxTextColourId,target==100?corrected:colour::textPrimary);
 auto state=proc_.captureResponseState();responseState_=state;if((state.sampleRate>0&&hoverHz_>=state.sampleRate*.5)||!hm::measurement::raw(state.activeProfile).supports(hoverHz_))hoverHz_=0;if(!tempoSourceChosen_&&proc_.getHostBpmSnapshot().valid)hostSync_=true;
 // The response state owns immutable IR data, keeping paint/hover on one coherent snapshot.
 if(!responseReady_||(int)proc_.getListenMode()!=lastListenMode_||target!=lastDegree_||state.appliedDegree!=appliedDegree_||state.sampleRate!=lastRate_||state.activeProfile!=lastProfile_||state.bypassed!=lastBypassed_||state.audioReady!=lastAudioReady_||state.profileBlend!=lastProfileBlend_||state.profilePhase!=ProfilePhase::Stable||basisJob_.valid()||basis_.generation!=state.generation||basis_.incomingGeneration!=state.incomingGeneration){
   lastListenMode_=(int)proc_.getListenMode();lastDegree_=target;lastRate_=state.sampleRate;lastProfile_=state.activeProfile;lastBypassed_=state.bypassed;lastAudioReady_=state.audioReady;lastProfileBlend_=state.profileBlend;rebuildResponse();
 }
 auto actualName=state.activeProfile>=0?juce::String(proc_.getDatabase().getProfileName(state.activeProfile)):proc_.getProfileName();profile_.setTooltip(actualName+(state.profileError?"\n"+state.profileErrorMessage:"\nChoose your exact headphone model"));profile_.setButtonText(actualName+(state.profileError?text8("   /   加载失败 · 更换"):state.profilePhase!=ProfilePhase::Stable&&state.audioReady?text8("   /   加载中"):text8("   /   更换型号")));compare_.setToggleState(target==0,juce::dontSendNotification);
 if(view_==View::tempo){source_.setButtonText(hostSync_?"HOST":"MANUAL");source_.setToggleState(hostSync_,juce::dontSendNotification);tempoInput_.setReadOnly(hostSync_);if(hostSync_){auto b=proc_.getHostBpmSnapshot();tempoInput_.setText(b.valid?juce::String(b.bpm,3):"",false);}else if(tempoInput_.isReadOnly())tempoInput_.setText(manualText_,false);straight_.setToggleState(feel_==hm::tempo::Feel::straight,juce::dontSendNotification);dotted_.setToggleState(feel_==hm::tempo::Feel::dotted,juce::dontSendNotification);triplet_.setToggleState(feel_==hm::tempo::Feel::triplet,juce::dontSendNotification);}
 surface_.repaint();
}
