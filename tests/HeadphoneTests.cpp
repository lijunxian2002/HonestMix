#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OfflineGuide.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <new>
static thread_local bool countingAudio=false;
static thread_local size_t audioNews=0;
void* operator new(std::size_t n){if(countingAudio)++audioNews;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}
static void check(bool v,const char* message){if(!v)throw std::runtime_error(message);}
static juce::var object(){return juce::var(new juce::DynamicObject());}
static void add(juce::var& v,const char* name,const juce::var& value){v.getDynamicObject()->setProperty(name,value);}
static juce::RangedAudioParameter* parameter(HonestMixAudioProcessor& p,const char* id){for(auto* item:p.getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(item))if(ranged->paramID==id)return ranged;throw std::runtime_error("Missing host parameter");}
static void automate(HonestMixAudioProcessor& p,const char* id,float value){auto* param=parameter(p,id);param->setValueNotifyingHost(param->convertTo0to1(value));}
static void runAudio(HonestMixAudioProcessor& p,juce::AudioBuffer<float>& buffer){juce::MidiBuffer midi;countingAudio=true;p.processBlock(buffer,midi);countingAudio=false;}
static void finite(const juce::AudioBuffer<float>& b){for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i)check(std::isfinite(b.getSample(ch,i)),"DSP emitted a nonfinite sample");}
struct Listener:juce::AudioProcessorListener{int changes=0;void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{++changes;}void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails&)override{}};
static juce::var impulseTests(){
 juce::Array<juce::var> cases;
 for(float degree:{0.f,100.f,200.f}){
  HonestMixAudioProcessor p;p.setTranslationDegree(degree);p.prepareToPlay(44100,256);juce::AudioBuffer<float> b(2,256);b.clear();b.setSample(0,0,1);b.setSample(1,0,.5f);runAudio(p,b);
  const auto* fir=p.getDatabase().getProfileFIR(0);double error=0;for(int i=0;i<256;++i){float expected=degree/100*fir[i]+(1-degree/100)*(i==0?1.f:0.f);error=juce::jmax(error,(double)std::abs(b.getSample(0,i)-expected));error=juce::jmax(error,(double)std::abs(b.getSample(1,i)-expected*.5f));}
  check(error<.0002,"Translation 0/100/200 must process the actual FIR and preserve stereo separation");check(p.getLatencySamples()==0,"The zero-latency convolver must not report FIR length as latency");
  auto c=object();add(c,"translation",degree);add(c,"maximumImpulseError",error);add(c,"passed",true);cases.add(c);
 }
 HonestMixAudioProcessor alternate;alternate.setProfileIndex(1);alternate.prepareToPlay(44100,256);juce::AudioBuffer<float> b(2,256);b.clear();b.setSample(0,0,1);runAudio(alternate,b);const auto* ir=alternate.getDatabase().getProfileFIR(1);double error=0,difference=0;for(int i=0;i<256;++i){error=juce::jmax(error,(double)std::abs(b.getSample(0,i)-ir[i]));difference+=std::abs(ir[i]-alternate.getDatabase().getProfileFIR(0)[i]);}check(error<.0002&&difference>.01,"Selecting a headphone must change actual FIR output");
 auto report=object();add(report,"translationCases",cases);add(report,"profileImpulseError",error);add(report,"profileDifference",difference);add(report,"passed",true);return report;
}
static juce::var configurationTests(){
 HonestMixAudioProcessor p;Listener listener;p.addListener(&listener);p.setTranslationDegree(73.5f);p.setListenMode(ListenCar);p.setProfileIndex(3);check(listener.changes>=3,"UI setters must notify host automation");p.removeListener(&listener);
 juce::MemoryBlock state;p.getStateInformation(state);HonestMixAudioProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());check(restored.getTranslationDegree()==73.5f&&restored.getListenMode()==ListenCar&&restored.getProfileName()==p.getProfileName(),"Preset must restore degree, listen mode and headphone together");

 // Real legacy XML state import, not setters on a modern round-trip state.
 int legacyCases=0;
 for(double value:{0.,37.5,73.7,100.,150.,200.,-20.,230.}){
  for(bool partial:{false,true}){
   juce::ValueTree tree("HonestMix");tree.setProperty("degree",value,nullptr);tree.setProperty("profile","Audio-Technica ATH-M50x",nullptr);
   if(partial){juce::ValueTree params("Parameters"),child("PARAM");child.setProperty("id","listenMode",nullptr);child.setProperty("value",0,nullptr);params.appendChild(child,nullptr);tree.appendChild(params,nullptr);}
   juce::MemoryBlock blob;juce::AudioProcessor::copyXmlToBinary(*tree.createXml(),blob);HonestMixAudioProcessor old;
   old.setStateInformation(blob.getData(),(int)blob.getSize());const float expected=parameter(old,"translation")->getNormalisableRange().snapToLegalValue((float)juce::jlimit(0.,200.,value));
   check(old.getTranslationDegree()==expected,"Legacy degree must restore exact legal correction, even with partial modern parameters");check(old.getProfileName()=="Audio-Technica ATH-M50x"&&old.getListenMode()==ListenNormal,"Legacy headphone and listen mode must restore to actual APVTS values");
   old.prepareToPlay(44100,256);juce::AudioBuffer<float> impulse(2,256);impulse.clear();impulse.setSample(0,0,1);impulse.setSample(1,0,.5f);runAudio(old,impulse);
   auto profile=old.getDatabase().findProfileByName(old.getProfileName());const auto* fir=old.getDatabase().getProfileFIR(profile);check(fir!=nullptr,"Imported actual profile needs FIR");
   for(int i=0;i<256;++i){const double wanted=expected/100.*fir[i]+(1-expected/100.)*(i==0?1.:0.);check(std::abs(impulse.getSample(0,i)-wanted)<.0002&&std::abs(impulse.getSample(1,i)-wanted*.5)<.0002,"Legacy correction readback must agree with actual stereo impulse audio");}
   juce::MemoryBlock modern;old.getStateInformation(modern);HonestMixAudioProcessor again;again.setStateInformation(modern.getData(),(int)modern.getSize());check(again.getTranslationDegree()==expected,"Imported legacy correction must survive modern save/restore");++legacyCases;
  }
 }
 for(const char* invalid:{"", "garbage", "37.5oops", "nan", "inf", "1e99999999999999999999", ".", "+", "-", "+."}){
  juce::ValueTree tree("HonestMix");tree.setProperty("degree",invalid,nullptr);juce::MemoryBlock blob;juce::AudioProcessor::copyXmlToBinary(*tree.createXml(),blob);HonestMixAudioProcessor old;old.setTranslationDegree(81.5f);old.setStateInformation(blob.getData(),(int)blob.getSize());check(old.getTranslationDegree()==81.5f,"Malformed legacy degree must not alter actual parameter");++legacyCases;
 }
 {auto xml=juce::AudioProcessor::getXmlFromBinary(state.getData(),(int)state.getSize());check(xml!=nullptr,"Real modern state required");xml->setAttribute("degree",12.5);juce::MemoryBlock blob;juce::AudioProcessor::copyXmlToBinary(*xml,blob);HonestMixAudioProcessor old;old.setStateInformation(blob.getData(),(int)blob.getSize());check(old.getTranslationDegree()==73.5f,"Modern translation must override stale legacy degree metadata");++legacyCases;}
 p.prepareToPlay(44100,128);automate(p,"translation",0);automate(p,"listenMode",1);juce::AudioBuffer<float> b(2,128);for(int k=0;k<16;++k){for(int i=0;i<128;++i){b.setSample(0,i,.2f);b.setSample(1,i,.6f);}runAudio(p,b);}for(int i=0;i<128;++i)check(std::abs(b.getSample(0,i)-.4f)<.0001&&std::abs(b.getSample(1,i)-.4f)<.0001,"Host automation must affect audio after its short smoothing interval without UI setter calls");
 HonestMixAudioProcessor::BusesLayout mono;mono.inputBuses.add(juce::AudioChannelSet::mono());mono.outputBuses.add(juce::AudioChannelSet::mono());check(p.isBusesLayoutSupported(mono),"Mono layout must be supported");check(p.setBusesLayout(mono),"Mono layout must apply");p.prepareToPlay(48000,64);b.setSize(1,127);b.clear();b.setSample(0,0,.5f);runAudio(p,b);finite(b);
 auto mixed=mono;mixed.inputBuses.set(0,juce::AudioChannelSet::stereo());check(!p.isBusesLayoutSupported(mixed),"Mixed channel counts must be rejected explicitly");auto surround=mono;surround.inputBuses.set(0,juce::AudioChannelSet::create5point1());surround.outputBuses.set(0,juce::AudioChannelSet::create5point1());check(!p.isBusesLayoutSupported(surround),"Unsupported surround layout must be rejected");
 auto r=object();add(r,"hostNotifications",listener.changes);add(r,"stateRoundTrip",true);add(r,"legacyDegreeCases",legacyCases);add(r,"legacyDegreeImport",true);add(r,"modernParameterAuthoritative",true);add(r,"hostAutomationAudio",true);add(r,"monoAndLayoutBoundaries",true);add(r,"passed",true);return r;
}
static juce::var stressTests(){
 juce::Array<juce::var> cases;
 for(double rate:{32000.,44100.,48000.,96000.,192000.})for(int block:{1,64,257,1024}){
  HonestMixAudioProcessor p;p.prepareToPlay(rate,block);juce::AudioBuffer<float> b(2,block+17);double energy=0;
  for(int mode=0;mode<4;++mode){p.setListenMode((ListenMode)mode);for(int iteration=0;iteration<12;++iteration){for(int ch=0;ch<2;++ch)for(int i=0;i<b.getNumSamples();++i)b.setSample(ch,i,.1f*(float)std::sin((iteration*b.getNumSamples()+i)*.1));if(iteration==3){b.setSample(0,0,std::numeric_limits<float>::quiet_NaN());b.setSample(1,0,std::numeric_limits<float>::infinity());}runAudio(p,b);finite(b);energy+=b.getMagnitude(0,b.getNumSamples());}}
  check(energy>0,"Rate/block/listen matrix must carry audio");p.releaseResources();p.prepareToPlay(rate,block);b.clear();runAudio(p,b);check(b.getMagnitude(0,b.getNumSamples())<1e-7,"Fresh silence must stay silent");auto r=object();add(r,"rate",rate);add(r,"preparedBlock",block);add(r,"actualBlock",block+17);add(r,"passed",true);cases.add(r);
 }
 auto report=object();add(report,"cases",cases);add(report,"passed",true);return report;
}
static juce::var listenTests(){
 auto rms=[](ListenMode mode){HonestMixAudioProcessor p;p.setTranslationDegree(0);p.setListenMode(mode);p.prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);double sum=0;size_t count=0;for(int k=0;k<40;++k){for(int i=0;i<256;++i){float v=.1f*(float)std::sin(juce::MathConstants<double>::twoPi*80*(k*256+i)/48000.0);b.setSample(0,i,v);b.setSample(1,i,0);}runAudio(p,b);if(k>10)for(int i=0;i<256;++i){sum+=b.getSample(0,i)*b.getSample(0,i);++count;}check(b.getMagnitude(1,0,256)<1e-8,"Listen filter channels must stay independent");}return std::sqrt(sum/count);};
 auto normal=rms(ListenNormal),phone=rms(ListeniPhone),car=rms(ListenCar);check(phone<normal*.4&&car>normal*1.3&&car<normal*2.1,"Phone/car modes must implement audible and bounded actual filters");auto r=object();add(r,"normalRms",normal);add(r,"phoneRms",phone);add(r,"carRms",car);add(r,"passed",true);return r;
}
static juce::var profileAutomationTest(){
 HonestMixAudioProcessor p;p.prepareToPlay(44100,256);automate(p,"profileIndex",1);juce::AudioBuffer<float> b(2,256);
 for(int k=0;k<120;++k){b.clear();runAudio(p,b);juce::Thread::sleep(1);}
 check(!p.isProfileLoading()&&p.getProfileName()==p.getDatabase().getProfileName(1),"Host profile automation must reach the audio engine");
 p.getCorrectionEngine().reset();b.clear();b.setSample(0,0,1);runAudio(p,b);double error=0;const auto* fir=p.getDatabase().getProfileFIR(1);for(int i=0;i<256;++i)error=juce::jmax(error,(double)std::abs(b.getSample(0,i)-fir[i]));check(error<.0002,"Asynchronously selected profile must alter the real impulse response");auto r=object();add(r,"maximumImpulseError",error);add(r,"passed",true);return r;
}
static juce::TextButton* buttonContaining(juce::Component& c,const juce::String& text){if(auto* button=dynamic_cast<juce::TextButton*>(&c))if(button->getButtonText().contains(text))return button;for(auto* child:c.getChildren())if(auto* button=buttonContaining(*child,text))return button;return nullptr;}
static juce::TextButton* buttonId(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return dynamic_cast<juce::TextButton*>(&c);for(auto* child:c.getChildren())if(auto* button=buttonId(*child,id))return button;return nullptr;}
template<typename T> static T* childOfType(juce::Component& c){for(auto* child:c.getChildren()){if(auto* match=dynamic_cast<T*>(child))return match;if(auto* match=childOfType<T>(*child))return match;}return nullptr;}
static void screenshot(juce::Component& editor,const juce::File& file){juce::Image image(juce::Image::ARGB,editor.getWidth(),editor.getHeight(),true);juce::Graphics graphics(image);editor.paintEntireComponent(graphics,true);juce::FileOutputStream stream(file);check(stream.openedOk()&&stream.setPosition(0)&&stream.truncate().wasOk()&&juce::PNGImageFormat().writeImageToStream(image,stream),"UI screenshot write failed");}
// 009 B 合入：来自 fidelity-fix（INTEGRATE.md「NativeSuite 加 4 种 rate 旁通连续历史与脉冲延迟」）。
// 每个采样率验证 reported latency == measured bypass latency，且正常/旁通回调共享同一 dry 延迟历史。
static juce::var bypassTests(){
 juce::Array<juce::var> cases;
 for(double rate:{44100.,48000.,96000.,192000.}){
  HonestMixAudioProcessor p;p.setTranslationDegree(0);p.prepareToPlay(rate,256);const int delay=p.getLatencySamples();juce::AudioBuffer<float> b(2,256);juce::MidiBuffer midi;double error=0;
  for(int block=0;block<12;++block){for(int i=0;i<256;++i){float value=(block*256+i+1)*.0001f;b.setSample(0,i,value);b.setSample(1,i,-value);}countingAudio=true;if(block>=3&&block<8)p.processBlockBypassed(b,midi);else p.processBlock(b,midi);countingAudio=false;
   for(int i=0;i<256;++i){int source=block*256+i-delay;float expected=source>=0?(source+1)*.0001f:0;error=juce::jmax(error,(double)std::abs(b.getSample(0,i)-expected));error=juce::jmax(error,(double)std::abs(b.getSample(1,i)+expected));}
  }
  check(error<1e-6,"Normal/bypassed callbacks must share the exact same dry-delay history");
  p.prepareToPlay(rate,256);int measured=-1;for(int block=0;block<5;++block){b.clear();if(block==0)b.setSample(0,0,1);countingAudio=true;p.processBlockBypassed(b,midi);countingAudio=false;for(int i=0;i<256;++i)if(b.getSample(0,i)>.5f)measured=block*256+i;}
  check(measured==delay,"Bypassed impulse latency must match host-reported latency");auto row=object();add(row,"rate",rate);add(row,"reportedLatency",delay);add(row,"measuredBypassLatency",measured);add(row,"transitionError",error);add(row,"passed",true);cases.add(row);
 }
 auto result=object();add(result,"cases",cases);add(result,"passed",true);return result;
}
static juce::TextEditor* inputNamed(juce::Component& c,const juce::String& name){
 if(auto* e=dynamic_cast<juce::TextEditor*>(&c))if(e->getName()==name)return e;
 for(auto* child:c.getChildren())if(auto* e=inputNamed(*child,name))return e;return nullptr;
}
struct TempoPlayHead final:juce::AudioPlayHead{
 double bpm=120;bool valid=true;
 juce::Optional<PositionInfo> getPosition()const override {PositionInfo p;if(valid)p.setBpm(bpm);return p;}
};
static juce::MouseEvent mouseEvent(juce::Component* c,float x,float y,float startY,int mods=juce::ModifierKeys::leftButtonModifier){
 return {juce::Desktop::getInstance().getMainMouseSource(),{x,y},juce::ModifierKeys(mods),1,0,0,0,0,c,c,juce::Time::getCurrentTime(),{x,startY},juce::Time::getCurrentTime(),1,true};
}
static juce::var grainMappingTests(){
 for(int scale:{1,2,4}){juce::Image image(juce::Image::ARGB,512*scale,64*scale,true);juce::Graphics g(image);g.fillAll(juce::Colour(0xff808080));g.addTransform(juce::AffineTransform::scale((float)scale));hm::skin::setDrawingScale((float)scale);hm::skin::paintBrushedGrain(g,{0,0,512,64},0xabc);long long periodicError=0,variation=0;for(int y=0;y<64*scale;y+=3)for(int x=0;x<128*scale;x+=3){auto a=image.getPixelAt(x,y),b=image.getPixelAt(x+128*scale,y);periodicError+=std::abs((int)a.getRed()-(int)b.getRed());variation+=std::abs((int)a.getRed()-128);}check(variation>0,"Grain mapping probe must contain actual texture");check(periodicError==0,"High-DPI grain period must remain 128 logical pixels without double scaling");}
 auto r=object();add(r,"physicalScales", "1/2/4");add(r,"logicalTilePeriod",128);add(r,"passed",true);return r;
}
static juce::var tempoMathTests(){
 for(double bpm:{1.,60.,90.,117.5,120.,123.456,300.,1000.}){
  const double q=60000.0/bpm;auto t=hm::tempo::calculate(bpm);
  check(std::abs(t.preShort-q/32)<=1e-12*std::abs(q),"Predelay must retain double precision");
  check(std::abs(t.hall.high-8*q)<=1e-12*std::abs(8*q),"Reverb pair must match beat multiples");
  for(int den:{2,4,8,16,32,64})for(auto feel:{hm::tempo::Feel::straight,hm::tempo::Feel::dotted,hm::tempo::Feel::triplet}){
   const double multiplier=feel==hm::tempo::Feel::dotted?1.5:feel==hm::tempo::Feel::triplet?2.0/3:1;
   const double expected=240000.0/(bpm*den)*multiplier;
   check(std::abs(hm::tempo::note(bpm,den,feel)-expected)<=std::abs(expected)*1e-12,"Independent whole-note duration oracle failed");
  }
 }
 check(hm::tempo::note(120,4)==500&&hm::tempo::note(120,2)==1000,"Quarter and half note labels must be musically correct");
 for(double bpm:{0.,-1.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()})check(!hm::tempo::valid(bpm)&&!std::isfinite(hm::tempo::quarter(bpm)),"Invalid tempo must not silently become 120");
 auto r=object();add(r,"doublePrecision",true);add(r,"quarter120ms",500);add(r,"half120ms",1000);add(r,"passed",true);return r;
}
static juce::var uiTests(const juce::File& folder){
 for(const char* product:{"Headphone","Connect"}){const auto guide=hm::support::writeOfflineGuideTo(folder.getChildFile("guide-export"),product,"test-"+juce::String(JucePlugin_VersionString));check(guide.existsAsFile(),"Embedded guide must export locally without a browser");const auto html=guide.loadFileAsString();check(html.contains("Content-Security-Policy")&&html.contains("default-src 'none'")&&html.contains(JucePlugin_VersionString)&&html.contains(product),"Embedded guide export must retain product/version and offline CSP");}
 TempoPlayHead host;HonestMixAudioProcessor p;p.setPlayHead(&host);p.prepareToPlay(48000,256);juce::AudioBuffer<float> buffer(2,256);buffer.clear();runAudio(p,buffer);
 std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());editor->getProperties().set("headlessTest",true);auto* skin=dynamic_cast<HonestMixAudioProcessorEditor*>(editor.get());check(skin,"Native editor must exist");
 for(int n=0;n<200;++n){buffer.clear();runAudio(p,buffer);skin->refreshFromProcessor();juce::Thread::sleep(1);}
 juce::Array<juce::var> scales;for(float scale:{1.f,1.25f,1.5f,2.f}){editor->setSize((int)(1000*scale),(int)(480*scale));screenshot(*editor,folder.getChildFile("ui-"+juce::String((int)(scale*100))+"pct.png"));scales.add(scale);}editor->setSize(900,432);screenshot(*editor,folder.getChildFile("ui-minimum.png"));check(editor->getWidth()==900&&editor->getHeight()==432,"Minimum native resize must be real");editor->setSize(1000,480);
 auto* compareMain=buttonId(*editor,"compare");auto* tempoMain=buttonId(*editor,"tempo");
 check(compareMain&&tempoMain&&!buttonId(*editor,"support"),"The retained TEMPO and COMPARE footer controls must have stable IDs, with GUIDE removed");
 for(auto* key:{compareMain,tempoMain})check(key->isVisible()&&key->getProperties().getWithDefault("secondaryLabel",juce::String()).toString().isNotEmpty(),"Each main action must retain its readable Chinese subtitle");
 int footerButtons=0;for(auto* child:compareMain->getParentComponent()->getChildren())if(auto* key=dynamic_cast<juce::TextButton*>(child))if(key->isVisible()&&key->getY()>=380)++footerButtons;
 check(footerButtons==2,"Normal view must have exactly two footer actions: TEMPO and COMPARE");
 auto options=skin->makeOptionsMenu();int legacyEntries=0;juce::PopupMenu::MenuItemIterator optionItem(options,true);while(optionItem.next()){const auto& item=optionItem.getItem();if(item.itemID>=1&&item.itemID<=4)++legacyEntries;}
 check(legacyEntries==4,"Legacy reference modes remain available in the context submenu");
 for(int id=1;id<=4;++id){skin->applyOption(id);check((int)p.getListenMode()==id-1,"Advanced legacy action must reach the retained host parameter");}skin->applyOption(1);
 auto* profile=buttonId(*editor,"profile");check(profile&&profile->onClick,"Profile browser must be wired");profile->onClick();
 auto* search=inputNamed(*editor,"profile-search");auto* list=childOfType<juce::ListBox>(*editor);auto* apply=buttonId(*editor,"apply");check(search&&list&&apply&&search->isVisible(),"Browser controls must actually become visible");
 search->setText("ZZZ_no_profile_20261016",false);search->onTextChange();check(!apply->isEnabled()&&list->getModel()->getNumRows()==0,"Empty search disables Apply");screenshot(*editor,folder.getChildFile("ui-search-empty.png"));
 search->setText("HD 600",false);search->onTextChange();check(apply->isEnabled()&&list->getModel()->getNumRows()>0,"Exact profile search must work");list->selectRow(0);apply->onClick();check(p.getProfileName().contains("HD 600"),"Apply must reach real parameter");
 p.setTranslationDegree(75.5f);skin->refreshFromProcessor();auto* compare=buttonId(*editor,"compare");check(compare&&compare->onClick,"A/B entry must exist");p.setListenMode(ListenCar);compare->onClick();check(p.getTranslationDegree()==0&&p.getListenMode()==ListenCar,"Compare must go to zero without silently clearing a legacy mode");compare->onClick();check(p.getTranslationDegree()==75.5f&&p.getListenMode()==ListenCar,"Compare must restore fractional amount while retaining legacy state");skin->applyOption(1);
 auto* tempo=buttonId(*editor,"tempo");check(tempo&&tempo->onClick,"Tempo page entry must exist");tempo->onClick();check(skin->isTempoView(),"Tempo page must open inside the editor");auto* input=inputNamed(*editor,"tempo-input");auto* source=buttonId(*editor,"tempo-source");check(input&&input->isVisible()&&input->isReadOnly()&&source,"HOST mode input must be visible and read-only");
 check(std::abs(skin->displayedTempo()-120)<1e-12,"Initial host tempo must be used");host.bpm=123.456;buffer.clear();runAudio(p,buffer);skin->refreshFromProcessor();check(std::abs(skin->displayedTempo()-123.456)<1e-10,"Host fractional tempo must update without reopening page");screenshot(*editor,folder.getChildFile("ui-tempo-host.png"));
 source->onClick();check(!input->isReadOnly(),"Manual source must enable editing");input->selectAll();input->insertTextAtCaret("117.5");input->onTextChange();check(skin->displayedTempo()==117.5,"Manual tempo must use edited value");input->selectAll();input->insertTextAtCaret("123.456789");input->onTextChange();source->onClick();source->onClick();check(skin->displayedTempo()==123.456789&&input->getText()=="123.456789","Source round trip must preserve precise manual text without filling the input with zeros");input->selectAll();input->insertTextAtCaret("117.5");input->onTextChange();input->setText("1.2.3",false);input->onTextChange();check(!std::isfinite(skin->displayedTempo()),"Malformed manual input must be rejected");input->setText("120",false);input->onTextChange();screenshot(*editor,folder.getChildFile("ui-tempo-manual.png"));
 auto* dotted=buttonId(*editor,"dotted");check(dotted&&dotted->onClick,"Dotted mode must be wired");dotted->onClick();check(dotted->getToggleState(),"Dotted selection must be visible");
 for(int i=0;i<3;++i){skin->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));check(!skin->isTempoView(),"Esc must leave the embedded page");tempo->onClick();check(skin->isTempoView(),"Repeated page opening must stay interactive");}
 skin->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));auto hover=mouseEvent(editor.get(),392,220,220,0);skin->mouseMove(hover);check(skin->hoveredHz()>20&&skin->hoveredHz()<20000,"Hover must map to real log-frequency");skin->advanceAnimation(.016);screenshot(*editor,folder.getChildFile("ui-main-hover.png"));
 
 {TempoPlayHead laterHost;HonestMixAudioProcessor manual;manual.prepareToPlay(32000,128);std::unique_ptr<juce::AudioProcessorEditor> laterEditor(manual.createEditor());auto* later=dynamic_cast<HonestMixAudioProcessorEditor*>(laterEditor.get());auto* open=buttonId(*laterEditor,"tempo");open->onClick();auto* field=inputNamed(*laterEditor,"tempo-input");check(!field->isReadOnly(),"Standalone should default to Manual");field->selectAll();field->insertTextAtCaret("117.5");field->onTextChange();manual.setPlayHead(&laterHost);juce::AudioBuffer<float> frame(2,128);frame.clear();runAudio(manual,frame);later->refreshFromProcessor();check(later->displayedTempo()==117.5&&!field->isReadOnly(),"Late host tempo must not overwrite explicit manual input");later->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));auto outside=mouseEvent(laterEditor.get(),705,220,220,0);later->mouseMove(outside);check(later->hoveredHz()==0,"Frequencies beyond 32k Nyquist must have no invented readout");manual.prepareToPlay(48000,128);later->refreshFromProcessor();later->mouseMove(outside);check(later->hoveredHz()>16000,"Higher rate must allow the same hover location");manual.prepareToPlay(32000,128);later->refreshFromProcessor();check(later->hoveredHz()==0,"Lower rate must clear out-of-band hover without a new mouse event");}
 auto r=object();add(r,"actualResizeScales",scales);add(r,"searchNoMatch",true);add(r,"profileSelection",true);add(r,"compareRestore",true);add(r,"mainFooterActions",2);add(r,"minimumWidth",900);add(r,"minimumHeight",432);add(r,"bilingualActionSubtitles",true);add(r,"advancedLegacyModes",true);add(r,"offlineGuidePureExport",true);add(r,"embeddedTempoInteractive",true);add(r,"hostTempoLiveDouble",true);add(r,"manualInvalidRejected",true);add(r,"nativeDialogShown",false);add(r,"thirdPartyDawUiTested",false);add(r,"tempoMath",tempoMathTests());add(r,"grainMapping",grainMappingTests());add(r,"passed",true);return r;
}
static juce::var faderMappingTest(){
 HonestMixAudioProcessor p;std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());auto* fader=dynamic_cast<MechFader*>(childOfType<juce::Slider>(*editor));check(fader,"Mechanical fader must be a real slider");
 check(fader->getMinimum()==0&&fader->getMaximum()==200&&fader->getInterval()==.5,"Range/step must match host parameter");
 fader->setValue(100,juce::dontSendNotification);float centre=fader->capY();auto down=mouseEvent(fader,95,120,120);fader->mouseDown(down);check(fader->getValue()==100,"Mouse-down must not jump the value");auto drag=mouseEvent(fader,95,100,120);fader->mouseDrag(drag);check(fader->getValue()==105&&p.getTranslationDegree()==105,"Damped relative drag must reach the real parameter");fader->mouseUp(drag);
 auto fineDown=mouseEvent(fader,95,120,120,juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::shiftModifier);fader->mouseDown(fineDown);auto fine=mouseEvent(fader,95,100,120,juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::shiftModifier);fader->mouseDrag(fine);check(fader->getValue()==106,"Shift drag must have fivefold precision");fader->mouseUp(fine);
 fader->setValue(100,juce::sendNotificationSync);fader->mouseDown(down);fader->mouseDrag(drag);auto sameShift=mouseEvent(fader,95,100,120,juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::shiftModifier);fader->mouseDrag(sameShift);check(fader->getValue()==105,"Shift toggle without movement must not jump");auto further=mouseEvent(fader,95,80,120,juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::shiftModifier);fader->mouseDrag(further);check(fader->getValue()==106,"Fine movement after a modifier toggle must accumulate correctly");auto sameNormal=mouseEvent(fader,95,80,120);fader->mouseDrag(sameNormal);check(fader->getValue()==106,"Releasing Shift without movement must not jump");fader->mouseUp(sameNormal);
 fader->keyPressed(juce::KeyPress(juce::KeyPress::homeKey));check(fader->getValue()==0&&fader->capY()>centre,"Home reaches Original at bottom");fader->keyPressed(juce::KeyPress(juce::KeyPress::endKey));check(fader->getValue()==200&&fader->capY()<centre,"End reaches 200 at top");
 fader->mouseDoubleClick(down);check(p.getTranslationDegree()==100,"Double click reaches actual 100");fader->keyPressed(juce::KeyPress(juce::KeyPress::upKey));check(p.getTranslationDegree()==100.5f,"Arrow reaches valid half-step");
 auto* label=childOfType<juce::Label>(*fader);check(label,"Fader must expose an editable value label");label->showEditor();auto* valueEditor=label->getCurrentTextEditor();check(valueEditor,"Fader value editor must open");valueEditor->selectAll();valueEditor->insertTextAtCaret("37.5");label->hideEditor(false);check(p.getTranslationDegree()==37.5f,"Actual fader label editing must reach exact parameter");

 int ignoredInputs=0;fader->setValue(73.5,juce::sendNotificationSync);
 for(int mods:std::array<int,3>{juce::ModifierKeys::rightButtonModifier,juce::ModifierKeys::middleButtonModifier,juce::ModifierKeys::leftButtonModifier|juce::ModifierKeys::rightButtonModifier}){
  auto start=mouseEvent(fader,95,120,120,mods),move=mouseEvent(fader,95,80,120,mods);fader->mouseDown(start);check(!fader->dragging(),"Non-primary input must not capture a correction drag");fader->mouseDrag(move);fader->mouseUp(move);fader->mouseDoubleClick(start);check(p.getTranslationDegree()==73.5f,"Non-primary drag/double-click must not change audible correction");++ignoredInputs;
 }
 fader->setEnabled(false);fader->mouseDoubleClick(down);fader->keyPressed(juce::KeyPress(juce::KeyPress::homeKey));juce::MouseWheelDetails wheel{};wheel.deltaY=1;fader->mouseWheelMove(down,wheel);check(p.getTranslationDegree()==73.5f,"Disabled fader must ignore discrete inputs");fader->setEnabled(true);
 fader->mouseDown(down);fader->mouseDown(down);fader->mouseDrag(drag);fader->mouseUp(drag);check(p.getTranslationDegree()==78.5f&&!fader->dragging(),"Duplicate press must not reset drag origin or leave capture active");
 auto r=object();add(r,"ignoredNonPrimaryInputs",ignoredInputs);add(r,"disabledInputsIgnored",true);add(r,"duplicatePressSafe",true);add(r,"range",200);add(r,"step",.5);add(r,"capSize","68x24");add(r,"relativeDragNoJump",true);add(r,"fineDragRatio",5);add(r,"endpointsAndKeyboard",true);add(r,"passed",true);return r;
}
static juce::var axisAlignmentTest(){
 HonestMixAudioProcessor p;std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());auto* skin=dynamic_cast<HonestMixAudioProcessorEditor*>(editor.get());juce::Rectangle<int> painted;auto image=skin->renderSurface(4,painted);auto plot=skin->plotBounds();
 for(double hz:{100.,1000.,10000.}){int x=(int)std::lround((72+std::log10(hz/20)/3*640)*4);int y=125*4;int bestX=x;int peak=0;for(int at=x-3;at<=x+3;++at){auto c=image.getPixelAt(at,y);int value=c.getRed()+c.getGreen()+c.getBlue();if(value>peak){peak=value;bestX=at;}}check(std::abs(bestX-x)<=3&&peak>0,"Native grid must be within one logical pixel of independent frequency mapping");}
 check(plot.getBottom()==342&&plot.getRight()==712,"Plot geometry must leave clear frequency axis space");auto r=object();add(r,"nativeRenderScale",4);add(r,"logAxisMapping",true);add(r,"passed",true);return r;
}
static juce::var measureStep(float before,float after,ListenMode from,ListenMode to,double rate=48000,int block=128){
 HonestMixAudioProcessor p;p.setTranslationDegree(before);p.setListenMode(from);p.prepareToPlay(rate,block);juce::AudioBuffer<float> b(2,block);float previous=0;
 auto fill=[&]{for(int i=0;i<block;++i){b.setSample(0,i,.25f);b.setSample(1,i,-.25f);}};
 const int warmBlocks=(int)std::ceil(juce::jmax(8192.,rate*.06)/block),transitionBlocks=(int)std::ceil(rate*.04/block)+1;
 for(int k=0;k<warmBlocks;++k){fill();runAudio(p,b);previous=b.getSample(0,block-1);}float baseline=previous,first=0,maximum=0;std::vector<float> captured;captured.reserve((size_t)transitionBlocks*block);
 automate(p,"translation",after);automate(p,"listenMode",(float)to);for(int k=0;k<transitionBlocks;++k){fill();runAudio(p,b);finite(b);if(k==0)first=b.getSample(0,0);for(int i=0;i<block;++i){auto value=b.getSample(0,i);maximum=juce::jmax(maximum,std::abs(value-previous));previous=value;captured.push_back(value);}}
 int settled=0;for(int i=0;i<(int)captured.size();++i)if(std::abs(captured[(size_t)i]-previous)>1e-5f)settled=i+1;
 auto r=object();add(r,"rate",rate);add(r,"block",block);add(r,"before",before);add(r,"after",after);add(r,"fromMode",(int)from);add(r,"toMode",(int)to);add(r,"beforeOutput",baseline);add(r,"firstOutput",first);add(r,"maximumStep",maximum);add(r,"finalOutput",previous);add(r,"settledSamples",settled);add(r,"settledMs",settled*1000./rate);return r;
}
static juce::var automationStepProbe(){
 auto report=object();add(report,"degreeStep",measureStep(0,200,ListenNormal,ListenNormal));juce::Array<juce::var> modes;for(auto mode:{ListenMono,ListeniPhone,ListenCar}){modes.add(measureStep(0,0,ListenNormal,mode));modes.add(measureStep(0,0,mode,ListenNormal));}add(report,"modeSteps",modes);return report;
}
static juce::var smoothingTests(){
 auto report=automationStepProbe();check((double)report["degreeStep"]["maximumStep"]<.0005,"Degree automation must not create a block-boundary click");
 for(auto item:*report["modeSteps"].getArray())check((double)item["maximumStep"]<.001,"Listen mode changes must crossfade without a hard step");
 juce::Array<juce::var> cases;for(double rate:{32000.,44100.,48000.,96000.,192000.})for(int block:{1,64,257,1024}){
  auto item=measureStep(0,200,ListenNormal,ListenNormal,rate,block);double step=(double)item["maximumStep"],settled=(double)item["settledMs"],total=std::abs((double)item["finalOutput"]-(double)item["beforeOutput"]);
  check(step<total/(rate*.015)+.000002,"Automation smoothing must follow sample rate independently of callback block size");check(settled>=19&&settled<=20.1,"Automation must reach its target in 20 ms, not one host block");add(item,"passed",true);cases.add(item);
 }
 add(report,"rateBlockCases",cases);add(report,"passed",true);return report;
}
static juce::var vst3Tests(const juce::String& path){
 juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,path);check(descriptions.size()==1&&descriptions[0]->name=="HONESTMIX Headphone","The new VST3 must scan with its independent product identity");
 juce::String error;auto instance=format.createInstanceFromDescription(*descriptions[0],44100,128,error);check(instance!=nullptr,error.toRawUTF8());instance->setPlayConfigDetails(2,2,44100,128);instance->prepareToPlay(44100,128);
 juce::AudioBuffer<float> b(2,128);b.clear();b.setSample(0,0,1);juce::MidiBuffer midi;instance->processBlock(b,midi);finite(b);HeadphoneDatabase db;const auto* fir=db.getProfileFIR(0);double maximumError=0;for(int i=0;i<128;++i)maximumError=juce::jmax(maximumError,(double)std::abs(b.getSample(0,i)-fir[i]));check(maximumError<.0002,"Loaded VST3 must process the actual default FIR");instance->releaseResources();auto r=object();add(r,"name",descriptions[0]->name);add(r,"version",descriptions[0]->version);add(r,"maximumImpulseError",maximumError);add(r,"passed",true);return r;
}
// ── Offline multi-scale render plan ─────────────────────────────────────────
// The skin draws itself with native vector work at whatever device scale the
// graphics context carries, so an integer-scale offscreen render is a real
// native-resolution render, not a bitmap upscale. This enumerates the shots;
// the harness renders, crops and digests them without opening a desktop window.
static juce::var renderPlan(){
 HonestMixAudioProcessor p;std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());editor->getProperties().set("headlessTest",true);
 auto* skin=dynamic_cast<HonestMixAudioProcessorEditor*>(editor.get());check(skin!=nullptr,"Editor must be the HONESTMIX Headphone editor");
 const auto design=HonestMixAudioProcessorEditor::designSize();
 p.prepareToPlay(48000,256);juce::AudioBuffer<float> warmup(2,256);for(int i=0;i<4;++i){warmup.clear();runAudio(p,warmup);}
 juce::Array<juce::var> shots;
 auto addShot=[&](const juce::String& state,int scale,const juce::String& name){auto s=object();add(s,"state",state);add(s,"scale",scale);add(s,"file",name);shots.add(s);};
 for(int scale:{1,2,3,4}){const juce::String tag("_x"+juce::String(scale));addShot("idle",scale,"ui_idle"+tag+".png");addShot("search",scale,"ui_search"+tag+".png");addShot("empty",scale,"ui_search_empty"+tag+".png");addShot("active",scale,"ui_active"+tag+".png");addShot("modes",scale,"ui_listen_modes"+tag+".png");}
 auto report=object();add(report,"designWidth",design.x);add(report,"designHeight",design.y);add(report,"shots",shots);add(report,"passed",true);return report;
}
// Executes one shot from the plan at its native resolution.
static juce::var renderOne(const juce::String& state,int scale,const juce::File& file){
 HonestMixAudioProcessor p;TempoPlayHead previewHost;if(state=="tempo-host")p.setPlayHead(&previewHost);std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());editor->getProperties().set("headlessTest",true);
 auto* skin=dynamic_cast<HonestMixAudioProcessorEditor*>(editor.get());check(skin!=nullptr,"Editor must be the HONESTMIX Headphone editor");
 const auto design=HonestMixAudioProcessorEditor::designSize();
 p.prepareToPlay(48000,256);juce::AudioBuffer<float> warmup(2,256);for(int i=0;i<4;++i){warmup.clear();runAudio(p,warmup);}
 // The chip reads "选择耳机 / 搜索型号" before any profile is resolved and
 // "<型号> / 更换耳机" afterwards; accept both so the shot is state driven.
 auto profileChip=[&]()->juce::TextButton*{return buttonId(*editor,"profile");};
 if(state=="search"||state=="empty"){
  auto* profile=profileChip();check(profile&&profile->onClick,"Profile browser button must be wired");profile->onClick();
  if(state=="empty"){auto* search=childOfType<juce::TextEditor>(*editor);check(search!=nullptr,"Search control must exist");search->setText("ZZZ_no_such_profile_20261016",false);search->onTextChange();}
 }
 if(state=="active"){
  auto* profile=profileChip();check(profile&&profile->onClick,"Profile browser button must be wired");profile->onClick();
  auto* search=childOfType<juce::TextEditor>(*editor);auto* list=childOfType<juce::ListBox>(*editor);auto* apply=buttonId(*editor,"apply");
  check(search&&list&&apply,"Search browser controls must exist");
  search->setText("HD 600",false);search->onTextChange();check(list->getModel()->getNumRows()>0,"Matching search must select a real profile");
  list->selectRow(0);apply->onClick();
  check(p.getProfileName().contains("HD 600"),"Applied profile must be the real parameter value");
  const auto requested=p.captureResponseState().requestedProfile;const auto settleEnd=juce::Time::getMillisecondCounterHiRes()+10000;
  while(juce::Time::getMillisecondCounterHiRes()<settleEnd){warmup.clear();runAudio(p,warmup);skin->refreshFromProcessor();auto current=p.captureResponseState();if(current.activeProfile==requested&&current.profilePhase==ProfilePhase::Stable)break;juce::Thread::sleep(1);}
  check(p.captureResponseState().activeProfile==requested&&p.captureResponseState().profilePhase==ProfilePhase::Stable,"Active render must await real IR adoption rather than show the requested profile as active");
  p.setTranslationDegree(75.0f);skin->refreshFromProcessor();skin->repaint();
  auto* chip=profileChip();check(chip!=nullptr&&chip->getButtonText().contains("HD 600"),"Applied profile must update the header chip text");
 }
 if(state=="modes"){p.setListenMode(ListeniPhone);skin->refreshFromProcessor();}
 if(state=="zero")p.setTranslationDegree(0);if(state=="over")p.setTranslationDegree(200);
 if(state=="tempo"||state=="tempo-host"){auto* t=buttonId(*editor,"tempo");check(t&&t->onClick,"Tempo entry");t->onClick();}
 for(int n=0;n<250;++n){warmup.clear();runAudio(p,warmup);skin->refreshFromProcessor();juce::Thread::sleep(1);}
 juce::Rectangle<int> painted;const auto image=skin->renderSurface(scale,painted);
 juce::FileOutputStream stream(file);check(stream.openedOk()&&stream.setPosition(0)&&stream.truncate().wasOk()&&juce::PNGImageFormat().writeImageToStream(image,stream),"UI render write failed");
 auto r=object();add(r,"state",state);add(r,"targetDegree",p.getTranslationDegree());add(r,"appliedDegree",p.captureResponseState().appliedDegree);add(r,"sampleRate",p.captureResponseState().sampleRate);add(r,"scale",scale);add(r,"width",painted.getWidth());add(r,"height",painted.getHeight());add(r,"designWidth",design.x);add(r,"designHeight",design.y);add(r,"passed",true);return r;
}
#include "PredictionUi.h"
#include "MotionUi.h"
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI gui;auto report=object();add(report,"version",JucePlugin_VersionString);add(report,"hardwareAudioOpened",false);add(report,"publicNetworkUsed",false);
 if(argc>2&&juce::String::fromUTF8(argv[1])=="--ui-only"){juce::File folder(juce::String::fromUTF8(argv[2]));folder.createDirectory();add(report,"uiPreviewOnly",true);try{add(report,"ui",uiTests(folder));add(report,"faderMapping",faderMappingTest());add(report,"axisAlignment",axisAlignmentTest());add(report,"presentationMotion",motionUiTests(folder));add(report,"passed",true);}catch(const std::exception& e){add(report,"passed",false);add(report,"error",e.what());std::cerr<<e.what()<<std::endl;}folder.getChildFile("ui-preview.json").replaceWithText(juce::JSON::toString(report));return (bool)report["passed"]?0:1;}
 if(argc>2&&juce::String::fromUTF8(argv[1])=="--ui-motion"){juce::File folder(juce::String::fromUTF8(argv[2]));folder.createDirectory();add(report,"presentationOnly",true);try{add(report,"presentationMotion",motionUiTests(folder));add(report,"passed",true);}catch(const std::exception& e){add(report,"passed",false);add(report,"error",e.what());std::cerr<<e.what()<<std::endl;}folder.getChildFile("ui-motion.json").replaceWithText(juce::JSON::toString(report));std::cout<<juce::JSON::toString(report)<<std::endl;return (bool)report["passed"]?0:1;}
 if(argc>1&&juce::String::fromUTF8(argv[1])=="--probe-automation"){std::cout<<juce::JSON::toString(automationStepProbe())<<std::endl;return 0;}
 if(argc>1&&juce::String::fromUTF8(argv[1])=="--ui-plan"){try{std::cout<<juce::JSON::toString(renderPlan())<<std::endl;return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
 if(argc>1&&juce::String::fromUTF8(argv[1])=="--fader-map"){try{std::cout<<juce::JSON::toString(faderMappingTest())<<std::endl;return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
 if(argc>1&&juce::String::fromUTF8(argv[1])=="--axis-align"){try{std::cout<<juce::JSON::toString(axisAlignmentTest())<<std::endl;return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
 if(argc>1&&juce::String::fromUTF8(argv[1])=="--grain-limits"){try{auto o=object();juce::Array<juce::var> perScale;bool withinBudget=true;for(int s:{1,2,3,4}){float w=0,b=0;check(hm::skin::measureGrainTile(s,w,b),"Grain tile must build");auto e=object();add(e,"scale",s);add(e,"maxWhiteAlpha",w);add(e,"maxBlackAlpha",b);perScale.add(e);if(w>hm::skin::GrainLimits::budgetBright||b>hm::skin::GrainLimits::budgetDark)withinBudget=false;}add(o,"perScale",perScale);add(o,"budgetBright",hm::skin::GrainLimits::budgetBright);add(o,"budgetDark",hm::skin::GrainLimits::budgetDark);add(o,"clampBright",hm::skin::GrainLimits::brightFibre);add(o,"clampDark",hm::skin::GrainLimits::darkFibre);add(o,"veil",hm::skin::GrainLimits::veil);add(o,"tileLogical",hm::skin::GrainLimits::tileLogical);add(o,"withinBudget",withinBudget);add(o,"passed",true);std::cout<<juce::JSON::toString(o)<<std::endl;return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
 if(argc>4&&juce::String::fromUTF8(argv[1])=="--ui-render"){try{std::cout<<juce::JSON::toString(renderOne(juce::String::fromUTF8(argv[2]),(int)juce::String::fromUTF8(argv[3]).getIntValue(),juce::File(juce::String::fromUTF8(argv[4]))))<<std::endl;return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
 try{add(report,"firAndTranslation",impulseTests());add(report,"stateAndAutomation",configurationTests());add(report,"rateBlockMatrix",stressTests());add(report,"listenFilters",listenTests());add(report,"profileAutomation",profileAutomationTest());add(report,"automationSmoothing",smoothingTests());add(report,"bypass",bypassTests());check(audioNews==0,"C++ new allocations occurred on DSP callback thread");add(report,"callbackCppNewCount",(juce::int64)audioNews);
  if(argc>2)add(report,"vst3",vst3Tests(juce::String::fromUTF8(argv[2])));
  if(argc>1){juce::File folder(juce::String::fromUTF8(argv[1]));folder.createDirectory();add(report,"faderMapping",faderMappingTest());add(report,"axisAlignment",axisAlignmentTest());add(report,"ui",uiTests(folder));add(report,"predictedHeadphoneUi",predictionUiTests(folder));add(report,"presentationMotion",motionUiTests(folder));add(report,"passed",true);folder.getChildFile("headphone-tests.json").replaceWithText(juce::JSON::toString(report));}
  std::cout<<juce::JSON::toString(report)<<"\nHEADPHONE_TESTS_OK\n";return 0;
 }catch(const std::exception& e){countingAudio=false;add(report,"passed",false);add(report,"error",e.what());if(argc>1){juce::File folder(juce::String::fromUTF8(argv[1]));folder.createDirectory();folder.getChildFile("headphone-tests.json").replaceWithText(juce::JSON::toString(report));}std::cerr<<juce::JSON::toString(report)<<std::endl;return 1;}
}

