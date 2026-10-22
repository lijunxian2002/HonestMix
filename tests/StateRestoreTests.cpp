#include "PluginProcessor.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

// Offline regression against the real product SharedCode. No editor, device,
// network, priority change, normalisation, or substitute DSP implementation.
namespace {
int checks=0;
void require(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
juce::var object(){return juce::var(new juce::DynamicObject());}
void put(juce::var& v,const char* name,const juce::var& value){v.getDynamicObject()->setProperty(name,value);}
juce::MemoryBlock save(HonestMixAudioProcessor& p){juce::MemoryBlock b;p.getStateInformation(b);return b;}
std::unique_ptr<juce::XmlElement> xml(const juce::MemoryBlock& b){auto x=juce::AudioProcessor::getXmlFromBinary(b.getData(),(int)b.getSize());require(x!=nullptr,"Real saved processor XML required");return x;}
juce::XmlElement* parameter(juce::XmlElement& root,const char* id){
 auto* group=root.getChildByName("Parameters");require(group!=nullptr,"Real modern Parameters child required");
 juce::XmlElement* result=nullptr;int count=0;
 for(auto* c=group->getFirstChildElement();c;c=c->getNextElement())if(c->getStringAttribute("id")==id){result=c;++count;}
 require(count==1,"Exactly one real known parameter required");return result;
}
void restore(HonestMixAudioProcessor& p,const juce::XmlElement& x){juce::MemoryBlock b;juce::AudioProcessor::copyXmlToBinary(x,b);p.setStateInformation(b.getData(),(int)b.getSize());}
bool same(const juce::MemoryBlock& a,const juce::MemoryBlock& b){return a.getSize()==b.getSize()&&std::memcmp(a.getData(),b.getData(),a.getSize())==0;}
struct Audio {double rms=0,maxDifference=0;bool finite=true,allZero=true;int frames=0;};
Audio observe(HonestMixAudioProcessor& p,HonestMixAudioProcessor& control,int blocks,int& frame){
 juce::AudioBuffer<float> a(2,128),b(2,128);juce::MidiBuffer midi;Audio out;double sum=0;
 for(int n=0;n<blocks;++n){
  for(int i=0;i<128;++i){const double t=(frame+i)/48000.;a.setSample(0,i,(float)(.07*std::sin(2*juce::MathConstants<double>::pi*997*t)));a.setSample(1,i,(float)(.065*std::sin(2*juce::MathConstants<double>::pi*719*t)));}
  b.makeCopyOf(a);p.processBlock(a,midi);control.processBlock(b,midi);frame+=128;out.frames+=128;
  for(int ch=0;ch<2;++ch)for(int i=0;i<128;++i){const double v=a.getSample(ch,i),ref=b.getSample(ch,i);out.finite=out.finite&&std::isfinite(v)&&std::isfinite(ref);out.allZero=out.allZero&&v==0;if(std::isfinite(v)&&std::isfinite(ref)){sum+=v*v;out.maxDifference=std::max(out.maxDifference,std::abs(v-ref));}}
 }
 out.rms=std::sqrt(sum/(out.frames*2));return out;
}
juce::var describe(const Audio& a){auto v=object();put(v,"frames",a.frames);put(v,"allFinite",a.finite);put(v,"allZero",a.allZero);put(v,"rms",a.rms);put(v,"maximumDifferenceFromUnmodifiedProcessor",a.maxDifference);return v;}
void carriesAudio(const Audio& a){require(a.finite&&!a.allZero&&a.rms>.0001,"Safe generated input must retain finite nonzero output");}
void unchangedAudio(const Audio& a){carriesAudio(a);require(a.maxDifference<=1.e-7,"Rejected state must retain actual sample output against the untouched control");}
}
int main(int argc,char** argv){
 if(argc!=2)return 2;const juce::File output(juce::String::fromUTF8(argv[1]));if(output.exists()||output.createDirectory().failed())return 2;
 auto report=object();juce::Array<juce::var> rows;int code=2;
 put(report,"schema","honestmix.hp-state-restore-regression/1");put(report,"productVersion",JucePlugin_VersionString);
 put(report,"passed",false);put(report,"editorCreated",false);put(report,"deviceOpened",false);put(report,"networkUsed",false);
 put(report,"scope","Exact candidate SharedCode, 48k stereo/128 frames, generated tones; original bounded32+32+32 modern-nan sequence followed by malformed modern field cases and legal modern restore.");
 try{
  juce::ScopedJuceInitialiser_GUI init;
  {
   HonestMixAudioProcessor p,control;for(auto* processor:{&p,&control}){processor->setProfileByName("Audio-Technica ATH-M50x");processor->setTranslationDegree(73.5f);processor->prepareToPlay(48000,128);require(processor->getProfileName()=="Audio-Technica ATH-M50x","Exact M50x profile required");}
   int frame=0;const auto baseline=observe(p,control,32,frame);unchangedAudio(baseline);put(report,"baseline",describe(baseline));
   const auto saved=save(p);const auto originalTempo=p.getTempoPreferences();
   auto reject=[&](std::unique_ptr<juce::XmlElement> input,const juce::String& name,bool restoreOriginal){
    restore(p,*input);auto row=object();put(row,"case",name);
    const bool unchanged=same(saved,save(p))&&p.getTempoPreferences().generation==originalTempo.generation;
    put(row,"completeSavedStateUnchanged",unchanged);const auto audio=observe(p,control,32,frame);put(row,"audio",describe(audio));rows.add(row);
    require(unchanged,"Malformed modern state must be rejected before any metadata or parameter mutation");require(std::isfinite(p.getTranslationDegree())&&p.getTranslationDegree()==73.5f,"Malformed state must retain the legal raw degree");
    const auto response=p.captureResponseState();require(std::isfinite(response.targetDegree)&&std::isfinite(response.appliedDegree),"Response must remain finite after rejected state");unchangedAudio(audio);
    if(restoreOriginal){p.setStateInformation(saved.getData(),(int)saved.getSize());observe(p,control,32,frame);const auto recovered=observe(p,control,32,frame);put(report,"originalBoundedLegalRecovery",describe(recovered));unchangedAudio(recovered);}
   };
   // Same single mutation and same recovery intervals as the retained611
   // observation. Passing requires actual nonzero audio, not just finite zeros.
   {auto x=xml(saved);parameter(*x,"translation")->setAttribute("value","nan");reject(std::move(x),"original-single-modern-translation-nan",true);}
   // Restoring a legal state legitimately changes the tempo generation. Take
   // a new immutable baseline for the remaining independent rejected cases.
   const auto legalAfterRecovery=save(p);require(same(saved,legalAfterRecovery),"Untouched legal restoration must retain saved values");
   const auto tempoAfterRecovery=p.getTempoPreferences();
   auto rejectMore=[&](std::unique_ptr<juce::XmlElement> input,const juce::String& name){
    input->setAttribute("tempoManualText","123.45"); // valid unrelated metadata must not partially commit
    restore(p,*input);auto row=object();put(row,"case",name);
    const bool unchanged=same(legalAfterRecovery,save(p))&&p.getTempoPreferences().generation==tempoAfterRecovery.generation;
    put(row,"completeSavedStateUnchanged",unchanged);const auto a=observe(p,control,32,frame);put(row,"audio",describe(a));rows.add(row);
    require(unchanged,"Rejected modern state must not partially apply legal metadata");unchangedAudio(a);
   };
   for(const char* id:{"translation","curveIndex","profileIndex","listenMode"}){
    for(const char* bad:{"nan","inf","-inf","","12oops","1e999999","-1"}){
     if(juce::String(id)=="translation"&&juce::String(bad)=="nan")continue;
     auto x=xml(saved);parameter(*x,id)->setAttribute("value",bad);rejectMore(std::move(x),juce::String(id)+":"+bad);
    }
    {auto x=xml(saved);parameter(*x,id)->setAttribute("value","999999999");rejectMore(std::move(x),juce::String(id)+":out-of-range");}
    {auto x=xml(saved);parameter(*x,id)->setTagName("WRONG");rejectMore(std::move(x),juce::String(id)+":wrong-type");}
    {auto x=xml(saved);auto* c=parameter(*x,id);x->getChildByName("Parameters")->addChildElement(new juce::XmlElement(*c));rejectMore(std::move(x),juce::String(id)+":duplicate");}
    if(juce::String(id)!="translation"){auto x=xml(saved);parameter(*x,id)->setAttribute("value","0.25");rejectMore(std::move(x),juce::String(id)+":fractional-enum");}
   }
   {auto x=xml(saved);x->addChildElement(new juce::XmlElement(*x->getChildByName("Parameters")));rejectMore(std::move(x),"duplicate-Parameters");}
   // Real APVTS uses the declared default for an absent value property.
   // This occurs in its own saved state after a valid partial legacy import.
   juce::Array<juce::var> defaults;
   for(const char* id:{"translation","curveIndex","profileIndex","listenMode"}){
    auto x=xml(saved);parameter(*x,id)->removeAttribute("value");
    const auto previousGeneration=p.getTempoPreferences().generation;restore(p,*x);
    require(p.getTempoPreferences().generation==previousGeneration+1,"A legitimate missing-value state must be accepted rather than silently rejected");
    juce::RangedAudioParameter* actual=nullptr;for(auto* item:p.getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(item))if(ranged->paramID==id)actual=ranged;
    require(actual!=nullptr,"Actual declared parameter required");
    const float expected=actual->convertFrom0to1(actual->getDefaultValue()),value=actual->convertFrom0to1(actual->getValue());
    require(std::isfinite(value)&&value==expected,"Missing value must use the original finite declared default");
    const auto audio=observe(p,control,32,frame);carriesAudio(audio);auto row=object();put(row,"parameter",id);put(row,"expectedDefault",expected);put(row,"actual",value);put(row,"audio",describe(audio));defaults.add(row);
   }
   put(report,"legalMissingValueCases",defaults);
   struct Legal {const char* text;float expected;};
   for(const auto& legal: {Legal{"150.5",150.5f},Legal{"173.625",173.5f},Legal{" 1.5e2 ",150.f},Legal{"0",0.f},Legal{"200",200.f}}){
    auto x=xml(saved);parameter(*x,"translation")->setAttribute("value",legal.text);x->setAttribute("degree","12.5");restore(p,*x);
    require(p.getTranslationDegree()==legal.expected,"Legal modern input must retain JUCE's original legal-step mapping and override stale legacy metadata");
    const auto audio=observe(p,control,32,frame);carriesAudio(audio);auto row=object();put(row,"input",legal.text);put(row,"actual",p.getTranslationDegree());put(row,"audio",describe(audio));
    auto legalRows=report.getProperty("legalModernCases",juce::var(juce::Array<juce::var>()));legalRows.getArray()->add(row);put(report,"legalModernCases",legalRows);
   }
   p.setStateInformation(saved.getData(),(int)saved.getSize());observe(p,control,32,frame);const auto finalAudio=observe(p,control,32,frame);unchangedAudio(finalAudio);put(report,"afterLegalModernRoundTrip",describe(finalAudio));
   p.releaseResources();control.releaseResources();
  }
  put(report,"passed",true);code=0;
 }catch(const std::exception& e){put(report,"firstFailure",juce::String::fromUTF8(e.what()));}
 put(report,"cases",rows);put(report,"rejectedCaseCount",rows.size());put(report,"assertionCount",checks);put(report,"exitCode",code);
 const auto text=juce::JSON::toString(report,true);if(!output.getChildFile("state-restore.json").replaceWithText(text))return 3;std::cout<<text<<"\n";return code;
}
