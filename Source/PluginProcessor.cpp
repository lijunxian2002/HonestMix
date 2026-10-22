#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/TempoModel.h"
#include <cmath>
#include <charconv>
HonestMixAudioProcessor::HonestMixAudioProcessor()
 :AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 apvts_(*this,nullptr,"Parameters",createParameterLayout(headphoneDB_.getNumProfiles())){
 translationParameter_=apvts_.getRawParameterValue("translation");profileParameter_=apvts_.getRawParameterValue("profileIndex");listenParameter_=apvts_.getRawParameterValue("listenMode");
}
HonestMixAudioProcessor::~HonestMixAudioProcessor(){endTranslationGesture();}
juce::AudioProcessorValueTreeState::ParameterLayout HonestMixAudioProcessor::createParameterLayout(int profiles){
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 layout.add(std::make_unique<juce::AudioParameterFloat>("translation","Translation",juce::NormalisableRange<float>(0,200,.5f),100));
 // Retained as compatibility metadata. Only the embedded Harman2018 target is available.
 layout.add(std::make_unique<juce::AudioParameterInt>("curveIndex","Legacy target metadata",0,3,0));
 layout.add(std::make_unique<juce::AudioParameterInt>("profileIndex","Headphone",0,profiles-1,0));
 layout.add(std::make_unique<juce::AudioParameterChoice>("listenMode","Listen mode",juce::StringArray{"Normal","Mono","Phone","Car"},0));return layout;
}
bool HonestMixAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout)const{
 auto input=layout.getMainInputChannelSet(),output=layout.getMainOutputChannelSet();
 return input==output&&(output==juce::AudioChannelSet::mono()||output==juce::AudioChannelSet::stereo());
}
void HonestMixAudioProcessor::prepareToPlay(double sampleRate,int block){
 prepared_=false;publishHostBpm(0,false,juce::Time::getMillisecondCounterHiRes());
 sampleRate_=sampleRate>0&&std::isfinite(sampleRate)?sampleRate:44100;maximumBlock_=juce::jmax(1,block);
 auto channels=juce::jlimit(1,2,getTotalNumOutputChannels());
 correctionEngine_.setProfile((int)profileParameter_->load());
 correctionEngine_.prepare({sampleRate_,(juce::uint32)maximumBlock_,(juce::uint32)channels});correctionEngine_.setEnabled(true);
 setLatencySamples(correctionEngine_.getLatencySamples());mixer_.setTranslation(getTranslationDegree());mixer_.prepare(channels,maximumBlock_,sampleRate_,getLatencySamples());updateSimulationFilters();simulationState_={};previousListenMode_=juce::jlimit(0,3,(int)listenParameter_->load());
 for(int mode=0;mode<4;++mode){listenWeights_[(size_t)mode].reset(sampleRate_,0.020);listenWeights_[(size_t)mode].setCurrentAndTargetValue(mode==previousListenMode_?1.f:0.f);}
 outputPeak_=0;outputPeakSinceRead_.store(0,std::memory_order_relaxed);outputPeakResetGeneration_.fetch_add(1,std::memory_order_relaxed);prepared_=true;publishResponseSnapshot(false);
}
void HonestMixAudioProcessor::releaseResources(){prepared_=false;correctionEngine_.reset();simulationState_={};outputPeak_=0;outputPeakSinceRead_.store(0,std::memory_order_relaxed);outputPeakResetGeneration_.fetch_add(1,std::memory_order_relaxed);publishHostBpm(0,false,juce::Time::getMillisecondCounterHiRes());publishResponseSnapshot(false);}
void HonestMixAudioProcessor::updateSimulationFilters(){
 if(auto model=correctionEngine_.responseIR(correctionEngine_.processState().activeToken))simulationCoefficients_=model->monitorCoefficients;
}
float HonestMixAudioProcessor::processChunk(juce::AudioBuffer<float>& buffer,bool bypassed){
 const int channels=buffer.getNumChannels(),samples=buffer.getNumSamples();
 for(int ch=0;ch<channels;++ch){auto* p=buffer.getWritePointer(ch);for(int s=0;s<samples;++s)if(!std::isfinite(p[s]))p[s]=0;}
 correctionEngine_.setProfile((int)profileParameter_->load());mixer_.setTranslation(getTranslationDegree());mixer_.captureDry(buffer);correctionEngine_.process(buffer);mixer_.mixInPlace(buffer);
 const int mode=juce::jlimit(0,3,(int)listenParameter_->load());if(mode!=previousListenMode_){for(int m=0;m<4;++m)listenWeights_[(size_t)m].setTargetValue(m==mode?1.f:0.f);previousListenMode_=mode;}
 // Keep the two reference filters warm even while inaudible. Crossfading
 // one-hot weights preserves channel independence and avoids cold-state jumps.
 for(int i=0;i<samples;++i){
  float input[2]{buffer.getSample(0,i),channels==2?buffer.getSample(1,i):buffer.getSample(0,i)};
  float weights[4];for(int m=0;m<4;++m)weights[m]=listenWeights_[(size_t)m].getNextValue();
  for(int ch=0;ch<channels;++ch){float outputs[4]{input[ch],(input[0]+input[1])*.5f,0,0};
   for(int m=ListeniPhone;m<=ListenCar;++m){const auto& c=simulationCoefficients_[(size_t)m];auto& st=simulationState_[(size_t)m][(size_t)ch];float value=c[0]*input[ch]+st.z1;st.z1=c[1]*input[ch]-c[3]*value+st.z2;st.z2=c[2]*input[ch]-c[4]*value;outputs[m]=value;}
   float value=0;for(int m=0;m<4;++m)value+=weights[m]*outputs[m];buffer.setSample(ch,i,value);
  }
 }
 if(bypassed)mixer_.copyDelayedDryTo(buffer);
 float peak=0;for(int ch=0;ch<channels;++ch){auto* p=buffer.getWritePointer(ch);for(int i=0;i<samples;++i){if(!std::isfinite(p[i]))p[i]=0;peak=juce::jmax(peak,std::abs(p[i]));}}return peak;
}
void HonestMixAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 processAudio(buffer,false);
}
void HonestMixAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 processAudio(buffer,true);
}
void HonestMixAudioProcessor::processAudio(juce::AudioBuffer<float>& buffer,bool bypassed){
 juce::ScopedNoDenormals noDenormals;
 if(!prepared_){buffer.clear();outputPeak_=0;return;}
 double tempo=0;bool validTempo=false;
 if(auto* ph=getPlayHead())if(auto pos=ph->getPosition())if(auto bpm=pos->getBpm();bpm.hasValue()&&std::isfinite(*bpm)&&*bpm>0){tempo=*bpm;validTempo=true;}
 publishHostBpm(tempo,validTempo,juce::Time::getMillisecondCounterHiRes());
 const int channels=juce::jmin(2,juce::jmin(getTotalNumInputChannels(),getTotalNumOutputChannels()));
 for(int ch=channels;ch<buffer.getNumChannels();++ch)buffer.clear(ch,0,buffer.getNumSamples());
 if(channels<=0){outputPeak_=0;publishResponseSnapshot(bypassed);return;}
 // Unexpectedly large callbacks are processed in prepared-size slices; never resize dry storage here.
 float peak=0;for(int start=0;start<buffer.getNumSamples();start+=maximumBlock_){juce::AudioBuffer<float> chunk(buffer.getArrayOfWritePointers(),channels,start,juce::jmin(maximumBlock_,buffer.getNumSamples()-start));peak=juce::jmax(peak,processChunk(chunk,bypassed));}outputPeak_.store(peak);
 // Telemetry only: silence must not erase an overload before the UI polls it.
 auto pendingPeak=outputPeakSinceRead_.load(std::memory_order_relaxed);
 while(peak>pendingPeak&&!outputPeakSinceRead_.compare_exchange_weak(pendingPeak,peak,std::memory_order_relaxed)){}
 publishResponseSnapshot(bypassed);
}
juce::AudioProcessorEditor* HonestMixAudioProcessor::createEditor(){return new HonestMixAudioProcessorEditor(*this);}
static void notifyParameter(juce::AudioProcessorValueTreeState& tree,const char* id,float value){auto* p=tree.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}
void HonestMixAudioProcessor::beginTranslationGesture(){if(!translationGesture_.exchange(true))apvts_.getParameter("translation")->beginChangeGesture();}
void HonestMixAudioProcessor::endTranslationGesture(){if(translationGesture_.exchange(false))apvts_.getParameter("translation")->endChangeGesture();}
void HonestMixAudioProcessor::setTranslationDegree(float degree){
 if(!std::isfinite(degree))return;auto* p=apvts_.getParameter("translation");
 const auto value=p->getNormalisableRange().snapToLegalValue(juce::jlimit(0.f,200.f,degree));
 const bool discrete=!translationGesture_.load();if(discrete)p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));if(discrete)p->endChangeGesture();
}
float HonestMixAudioProcessor::getTranslationDegree()const noexcept{return translationParameter_->load();}
void HonestMixAudioProcessor::publishHostBpm(double bpm,bool valid,double timestamp)noexcept{
 bpmSequence_.fetch_add(1);hostBpm_.store(bpm);hostBpmValid_.store(valid);hostBpmTimestamp_.store(timestamp);bpmSequence_.fetch_add(1);
}
HostBpmSnapshot HonestMixAudioProcessor::getHostBpmSnapshot(double maximumAgeMs)const noexcept{
 HostBpmSnapshot result;for(;;){const auto seq=bpmSequence_.load();if(seq&1)continue;result.bpm=hostBpm_.load();result.valid=hostBpmValid_.load();result.updatedAtMs=hostBpmTimestamp_.load();if(seq==bpmSequence_.load()){result.generation=seq;break;}}
 const double age=juce::Time::getMillisecondCounterHiRes()-result.updatedAtMs;
 result.valid=result.valid&&std::isfinite(result.bpm)&&result.bpm>0;
 result.fresh=result.valid&&age>=0&&age<=juce::jmax(0.0,maximumAgeMs);
 result.calculable=result.fresh&&result.bpm>=1&&result.bpm<=1000;return result;
}
double HonestMixAudioProcessor::getHostBpm()const noexcept{const auto s=getHostBpmSnapshot();return s.valid&&s.fresh?s.bpm:0;}
void HonestMixAudioProcessor::publishResponseSnapshot(bool bypassed)noexcept{
 const auto state=correctionEngine_.processState();auto& p=responsePublication_;p.sequence.fetch_add(1);
 p.active=state.activeToken;p.incoming=state.incomingToken;p.blend=state.blend;p.phase=(int)state.phase;
 p.degree=mixer_.appliedTranslation();p.degreeTransition=mixer_.isTransitioning();bool moving=false;
 for(std::size_t i=0;i<listenWeights_.size();++i){p.modes[i]=listenWeights_[i].getCurrentValue();moving=moving||listenWeights_[i].isSmoothing();}
 p.modeTransition=moving;p.ready=prepared_.load();p.bypassed=bypassed;p.sequence.fetch_add(1);
}
CorrectionResponseSnapshot HonestMixAudioProcessor::captureResponseState()const{
 CorrectionResponseSnapshot s;const auto& p=responsePublication_;
 // The registry can retire an old token between the atomic publication read
 // and acquiring its shared model. Retry instead of substituting a new IR.
 for(int attempt=0;attempt<64;++attempt){
  const auto seq=p.sequence.load();if(seq&1)continue;
  s.generation=p.active.load();s.incomingGeneration=p.incoming.load();s.profileBlend=p.blend.load();s.profilePhase=(ProfilePhase)p.phase.load();s.appliedDegree=p.degree.load();s.degreeTransition=p.degreeTransition.load();s.modeTransition=p.modeTransition.load();s.audioReady=p.ready.load();s.bypassed=p.bypassed.load();
  for(std::size_t i=0;i<s.appliedModes.size();++i)s.appliedModes[i]=p.modes[i].load();
  if(seq!=p.sequence.load())continue;
  s.activeIR=correctionEngine_.responseIR(s.generation);s.incomingIR=correctionEngine_.responseIR(s.incomingGeneration);
  if((s.generation&&!s.activeIR)||(s.incomingGeneration&&!s.incomingIR))continue;
  s.revision=seq;s.audioReady=s.audioReady&&prepared_.load();
  if(s.activeIR){s.activeProfile=s.activeIR->profile;s.sampleRate=s.activeIR->sampleRate;s.latencySamples=s.activeIR->latencySamples;s.monitorCoefficients=s.activeIR->monitorCoefficients;}
  if(s.incomingIR)s.incomingProfile=s.incomingIR->profile;
  const auto request=correctionEngine_.requestedToken();s.requestedGeneration=request>>16;s.requestedProfile=juce::jlimit(0,headphoneDB_.getNumProfiles()-1,(int)profileParameter_->load());
  if(s.audioReady&&s.profilePhase==ProfilePhase::Stable&&s.activeProfile!=s.requestedProfile)s.profilePhase=ProfilePhase::Preparing;
  if(!s.audioReady)s.profilePhase=ProfilePhase::Unprepared;
  s.targetDegree=getTranslationDegree();s.degreeTransition=s.degreeTransition||std::abs(s.targetDegree-s.appliedDegree)>1.e-5;
  const auto targetMode=(int)getListenMode();for(int i=0;i<4;++i)s.modeTransition=s.modeTransition||std::abs(s.appliedModes[(std::size_t)i]-(i==targetMode?1.f:0.f))>1.e-6f;
  s.profileError=correctionEngine_.hasLoadError();if(s.profileError)s.profileErrorMessage=correctionEngine_.loadError();return s;
 }
 return {}; // A transient unavailable snapshot is explicit, never a wrong IR.
}
void HonestMixAudioProcessor::setListenMode(ListenMode mode){notifyParameter(apvts_,"listenMode",(float)juce::jlimit(0,3,(int)mode));}
void HonestMixAudioProcessor::setProfileIndex(int index){index=juce::jlimit(0,headphoneDB_.getNumProfiles()-1,index);notifyParameter(apvts_,"profileIndex",(float)index);correctionEngine_.setProfile(index);correctionEngine_.retryProfileLoad();}
void HonestMixAudioProcessor::setProfileByName(const juce::String& name){auto index=headphoneDB_.findProfileByName(name);setProfileIndex(index<0?0:index);}
static bool validManualTempoText(const juce::String& text){return text.isNotEmpty()&&text.length()<=12&&text.containsOnly("0123456789.")&&text.removeCharacters("0123456789").length()<=1&&hm::tempo::valid(text.getDoubleValue());}
HonestMixAudioProcessor::TempoPreferences HonestMixAudioProcessor::getTempoPreferences() const {
 std::lock_guard<std::mutex> lock(stateMutex_);return tempoPreferences_;
}
HonestMixAudioProcessor::TempoPreferences HonestMixAudioProcessor::setTempoPreferences(const TempoPreferences& next){
 std::lock_guard<std::mutex> lock(stateMutex_);
 if(!validManualTempoText(next.manualText)||next.source< -1||next.source>1||next.feel<0||next.feel>2)return tempoPreferences_;
 if(next.manualText!=tempoPreferences_.manualText||next.source!=tempoPreferences_.source||next.feel!=tempoPreferences_.feel){auto generation=tempoPreferences_.generation+1;tempoPreferences_=next;tempoPreferences_.generation=generation;}
 return tempoPreferences_;
}
void HonestMixAudioProcessor::getStateInformation(juce::MemoryBlock& dest){
 std::lock_guard<std::mutex> lock(stateMutex_);
 auto state=appState_.serialize();state.setProperty("tempoManualText",tempoPreferences_.manualText,nullptr);state.setProperty("tempoSource",tempoPreferences_.source,nullptr);state.setProperty("tempoFeel",tempoPreferences_.feel,nullptr);state.setProperty("profile",getProfileName(),nullptr);state.setProperty("listen",(int)getListenMode(),nullptr);state.setProperty("degree",getTranslationDegree(),nullptr);state.removeAllChildren(nullptr);state.appendChild(apvts_.copyState(),nullptr);if(auto xml=state.createXml())copyXmlToBinary(*xml,dest);
}
// A malformed modern state must not reach APVTS's permissive var-to-float
// conversion. In particular NaN survives its normalisation and can poison the
// mixer and the continuously warmed reference filters. Validate the whole
// known-parameter submission before changing either metadata or DSP requests.
static bool validModernParameters(const juce::ValueTree& root,juce::AudioProcessorValueTreeState& apvts){
 int parameterTrees=0;for(int i=0;i<root.getNumChildren();++i)if(root.getChild(i).hasType("Parameters"))++parameterTrees;
 if(parameterTrees>1)return false;
 const auto parameters=root.getChildWithName("Parameters");if(!parameters.isValid())return true;
 const char* ids[]{"translation","curveIndex","profileIndex","listenMode"};
 int counts[4]{};
 for(int i=0;i<parameters.getNumChildren();++i){
  const auto child=parameters.getChild(i);const auto id=child.getProperty("id").toString();
  for(int k=0;k<4;++k)if(id==ids[k]){
   if(++counts[k]>1||!child.hasType("PARAM"))return false;
   // JUCE can legitimately save a default PARAM without a value after
   // restoring a partial legacy tree. Its defined fallback is the finite
   // parameter default, not the permissive conversion of an empty string.
   if(!child.hasProperty("value"))continue;
   const auto value=child.getProperty("value");const auto text=value.toString().trim();
   const auto* first=text.toRawUTF8();const auto* last=first+text.getNumBytesAsUTF8();double number=0;
   const auto parsed=std::from_chars(first,last,number);const auto* p=apvts.getParameter(ids[k]);
   if(value.isBool()||parsed.ec!=std::errc{}||parsed.ptr!=last||!std::isfinite(number)||p==nullptr)return false;
   const auto& range=p->getNormalisableRange();
   if(number<range.start||number>range.end||(k!=0&&std::floor(number)!=number))return false;
  }
 }
 return true;
}
void HonestMixAudioProcessor::setStateInformation(const void* data,int size){
 std::lock_guard<std::mutex> lock(stateMutex_);
 if(data==nullptr||size<=0||size>1024*1024)return;if(auto xml=getXmlFromBinary(data,size)){auto tree=juce::ValueTree::fromXml(*xml);if(!tree.hasType("HonestMix")||!validModernParameters(tree,apvts_))return;
  TempoPreferences restored;
  const auto manual=tree.getProperty("tempoManualText"),tempoSource=tree.getProperty("tempoSource"),tempoFeel=tree.getProperty("tempoFeel");
  if(manual.isString()&&validManualTempoText(manual.toString()))restored.manualText=manual.toString();
  // ValueTree XML attributes restore as strings. Accept only canonical bounded
  // enum representations; do not truncate arbitrary doubles or malformed text.
  const auto sourceText=tempoSource.toString(),feelText=tempoFeel.toString();
  if(sourceText=="-1"||sourceText=="0"||sourceText=="1")restored.source=sourceText.getIntValue();
  if(feelText=="0"||feelText=="1"||feelText=="2")restored.feel=feelText.getIntValue();
  restored.generation=tempoPreferences_.generation+1;tempoPreferences_=restored;
  appState_.deserialize(tree);const auto parameters=tree.getChildWithName("Parameters");
  // Inspect the submitted state before JUCE fills missing child nodes. A
  // direct AudioParameter::setValue does not update APVTS's raw atomics.
  auto hasParameter=[&](const char* id){for(int i=0;i<parameters.getNumChildren();++i)if(parameters.getChild(i).getProperty("id").toString()==id)return true;return false;};
  auto restoredParameters=parameters.isValid()?parameters.createCopy():apvts_.copyState();
  auto importParameter=[&](const char* id,float value){
   for(int i=0;i<restoredParameters.getNumChildren();++i){auto child=restoredParameters.getChild(i);if(child.getProperty("id").toString()==id){child.setProperty("value",value,nullptr);return;}}
   juce::ValueTree child("PARAM");child.setProperty("id",id,nullptr);child.setProperty("value",value,nullptr);restoredParameters.appendChild(child,nullptr);
  };
  // Modern parameter entries are authoritative. Import legacy metadata only
  // where the corresponding parameter is absent from the submitted state.
  if(!hasParameter("translation")&&tree.hasProperty("degree")){
   const auto value=tree.getProperty("degree");const auto text=value.toString().trim();const auto* start=text.toRawUTF8();const auto* end=start+text.getNumBytesAsUTF8();
   double legacy=0;const auto parsed=std::from_chars(start,end,legacy);
   if(!value.isBool()&&parsed.ec==std::errc{}&&parsed.ptr==end&&std::isfinite(legacy)){
    auto* p=apvts_.getParameter("translation");const float bounded=(float)juce::jlimit(0.0,200.0,legacy);
    importParameter("translation",p->getNormalisableRange().snapToLegalValue(bounded));
   }
  }
  if(!hasParameter("profileIndex")){auto index=headphoneDB_.findProfileByName(appState_.getProfileName());importParameter("profileIndex",(float)juce::jmax(0,index));}
  if(!hasParameter("listenMode"))importParameter("listenMode",(float)juce::jlimit(0,3,appState_.getListenMode()));
  apvts_.replaceState(restoredParameters);
  correctionEngine_.setProfile((int)profileParameter_->load());
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new HonestMixAudioProcessor();}
