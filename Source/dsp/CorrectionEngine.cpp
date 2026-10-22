#include "dsp/CorrectionEngine.h"
#include "dsp/IRConversion.h"
#include <stdexcept>
CorrectionEngine::CorrectionEngine(const HeadphoneDatabase& db):Thread("Headphone prepared IR loader"),db_(db){startThread();}
CorrectionEngine::~CorrectionEngine(){signalThreadShouldExit();notify();stopThread(-1);destroy(pending_.exchange(nullptr));destroy(incoming_);destroy(current_);reclaim();}
CorrectionEngine::Bundle* CorrectionEngine::makeBundle(std::uint64_t request,const juce::dsp::ProcessSpec& spec,std::uint64_t version){
 const int index=(int)(request&0xffff);auto samples=headphoneIR::convert(db_.getProfileFIR(index),HeadphoneDatabase::kFirLength,spec.sampleRate);
 auto model=std::make_shared<HostRateIR>();model->profile=index;model->sampleRate=spec.sampleRate;model->generation=nextToken_.fetch_add(1);model->requestGeneration=request>>16;model->specGeneration=version;model->latencySamples=headphoneIR::delay(spec.sampleRate);
 model->coefficients.assign(samples.getReadPointer(0),samples.getReadPointer(0)+samples.getNumSamples());
 for(auto& c:model->monitorCoefficients)c={1,0,0,0,0};
 const auto phone=juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate,(float)juce::jmin(400.0,spec.sampleRate*.2));
 const auto car=juce::dsp::IIR::Coefficients<float>::makeLowShelf(spec.sampleRate,(float)juce::jmin(100.0,spec.sampleRate*.1),.7f,juce::Decibels::decibelsToGain(6.f));
 std::copy_n(phone->getRawCoefficients(),5,model->monitorCoefficients[2].begin());std::copy_n(car->getRawCoefficients(),5,model->monitorCoefficients[3].begin());
 auto bundle=std::make_unique<Bundle>();bundle->response=model;bundle->request=request;bundle->specVersion=version;
 bundle->convolution.loadImpulseResponse(std::move(samples),spec.sampleRate,juce::dsp::Convolution::Stereo::no,juce::dsp::Convolution::Trim::no,juce::dsp::Convolution::Normalise::no);
 // JUCE prepare synchronously drains the queued load and builds the engine.
 // No instance is ever reloaded during audio processing, so JUCE's hidden
 // asynchronous engine adoption / 50ms crossover is not part of our state.
 bundle->convolution.prepare(spec);
 if(bundle->convolution.getLatency()!=0||bundle->convolution.getCurrentIRSize()!=(int)model->coefficients.size())throw std::runtime_error("Prepared convolution does not match its immutable IR");
 {std::lock_guard<std::mutex> lock(registryMutex_);registry_[model->generation]=model;}
 return bundle.release();
}
void CorrectionEngine::destroy(Bundle* bundle){if(!bundle)return;{std::lock_guard<std::mutex> lock(registryMutex_);registry_.erase(bundle->response->generation);}delete bundle;}
void CorrectionEngine::retire(Bundle* bundle)noexcept{if(!bundle)return;const auto w=retiredWrite_.load(std::memory_order_relaxed);jassert(w-retiredRead_.load(std::memory_order_acquire)<retired_.size());retired_[w%retired_.size()]=bundle;retiredWrite_.store(w+1,std::memory_order_release);}
void CorrectionEngine::reclaim(){auto r=retiredRead_.load(std::memory_order_relaxed),w=retiredWrite_.load(std::memory_order_acquire);while(r!=w){destroy(retired_[r%retired_.size()]);++r;}retiredRead_.store(r,std::memory_order_release);}
bool CorrectionEngine::readSpec(juce::dsp::ProcessSpec& spec,std::uint64_t& version)const noexcept{
 version=specVersion_.load(std::memory_order_acquire);if(version==0||(version&1))return false;spec={rate_.load(),block_.load(),channels_.load()};return version==specVersion_.load(std::memory_order_acquire);
}
void CorrectionEngine::run(){
 std::uint64_t publishedRequest=0,publishedSpec=0;
 while(!threadShouldExit()){
  reclaim();juce::dsp::ProcessSpec spec;std::uint64_t version=0;const auto request=request_.load();
  if(readSpec(spec,version)&&(request!=publishedRequest||version!=publishedSpec)){
   try{auto* bundle=makeBundle(request,spec,version);if(request==request_.load()&&version==specVersion_.load()){destroy(pending_.exchange(bundle));publishedRequest=request;publishedSpec=version;failedRequest_=0;}else destroy(bundle);}
   catch(const std::exception& e){{std::lock_guard<std::mutex> lock(registryMutex_);loadError_=juce::String::fromUTF8(e.what());}failedSpec_=version;failedRequest_=request;publishedRequest=request;publishedSpec=version;}
  }
  wait(5);
 }
 reclaim();
}
void CorrectionEngine::setProfile(int index){const auto next=(std::uint64_t)juce::jlimit(0,db_.getNumProfiles()-1,index);auto old=request_.load();while((old&0xffff)!=next&&!request_.compare_exchange_weak(old,((old>>16)+1)<<16|next)){} }
void CorrectionEngine::retryProfileLoad()noexcept{auto old=request_.load();if(failedRequest_.load()==old&&failedSpec_.load()==specVersion_.load())request_.compare_exchange_strong(old,((old>>16)+1)<<16|(old&0xffff));}
void CorrectionEngine::setProfileByName(const juce::String& name){auto index=db_.findProfileByName(name);setProfile(index<0?0:index);}
void CorrectionEngine::prepare(const juce::dsp::ProcessSpec& spec){
 prepared_=false;specVersion_.fetch_add(1,std::memory_order_acq_rel);rate_=spec.sampleRate;block_=spec.maximumBlockSize;channels_=spec.numChannels;const auto version=specVersion_.fetch_add(1,std::memory_order_release)+1;
 destroy(pending_.exchange(nullptr));destroy(incoming_);incoming_=nullptr;destroy(current_);current_=nullptr;
 scratch_.setSize((int)spec.numChannels,(int)spec.maximumBlockSize);scratch_.clear();fade_.reset(spec.sampleRate,.020);fade_.setCurrentAndTargetValue(0);warmupRemaining_=0;fading_=false;
 current_=makeBundle(request_.load(),spec,version);latency_=current_->response->latencySamples;tailSeconds_=current_->response->coefficients.size()/spec.sampleRate;prepared_=true;publish();
}
void CorrectionEngine::reset(){if(current_)current_->convolution.reset();if(incoming_)incoming_->convolution.reset();if(incoming_){warmupRemaining_=(int)incoming_->response->coefficients.size();fade_.setCurrentAndTargetValue(0);fading_=false;}publish();}
CorrectionEngine::ProcessState CorrectionEngine::processState()const noexcept{
 ProcessState state;state.request=request_.load();if(!prepared_||!current_)return state;
 state.activeToken=current_->response->generation;state.activeProfile=current_->response->profile;
 if(incoming_){state.incomingToken=incoming_->response->generation;state.incomingProfile=incoming_->response->profile;state.blend=fading_?fade_.getCurrentValue():0;state.phase=fading_?ProfilePhase::Fading:ProfilePhase::Warming;}
 else state.phase=state.activeProfile==(int)(state.request&0xffff)?ProfilePhase::Stable:ProfilePhase::Preparing;
 return state;
}
void CorrectionEngine::publish()noexcept{const auto state=processState();activeProfile_=state.activeProfile;phase_=(int)state.phase;}
void CorrectionEngine::process(juce::AudioBuffer<float>& buffer){
 if(!prepared_||!enabled_.load()||!current_)return;
 auto freeSlots=[&]{return retired_.size()-(retiredWrite_.load(std::memory_order_relaxed)-retiredRead_.load(std::memory_order_acquire));};
 if(incoming_&&!fading_&&incoming_->request!=request_.load()&&freeSlots()>=2){retire(incoming_);incoming_=nullptr;warmupRemaining_=0;}
 if(!incoming_&&freeSlots()>=2){if(auto* bundle=pending_.exchange(nullptr)){
  if(bundle->request==request_.load()&&bundle->specVersion==specVersion_.load()&&bundle->response->profile!=current_->response->profile){incoming_=bundle;warmupRemaining_=(int)bundle->response->coefficients.size();fading_=false;fade_.setCurrentAndTargetValue(0);}else retire(bundle);
 }}
 const int samples=buffer.getNumSamples(),channels=buffer.getNumChannels();
 if(incoming_)for(int ch=0;ch<channels;++ch)scratch_.copyFrom(ch,0,buffer,ch,0,samples);
 juce::dsp::AudioBlock<float> activeBlock(buffer);current_->convolution.process(juce::dsp::ProcessContextReplacing<float>(activeBlock));
 if(incoming_){juce::AudioBuffer<float> incomingBuffer(scratch_.getArrayOfWritePointers(),channels,0,samples);juce::dsp::AudioBlock<float> nextBlock(incomingBuffer);incoming_->convolution.process(juce::dsp::ProcessContextReplacing<float>(nextBlock));
  for(int sample=0;sample<samples;++sample){float mix=0;if(warmupRemaining_>0)--warmupRemaining_;else{if(!fading_){fading_=true;fade_.setTargetValue(1);}mix=fade_.getNextValue();}
   for(int ch=0;ch<channels;++ch)buffer.setSample(ch,sample,buffer.getSample(ch,sample)*(1-mix)+scratch_.getSample(ch,sample)*mix);
  }
  if(fading_&&!fade_.isSmoothing()){retire(current_);current_=incoming_;incoming_=nullptr;fading_=false;fade_.setCurrentAndTargetValue(0);}
 }
 publish();
}
std::shared_ptr<const HostRateIR> CorrectionEngine::responseIR(std::uint64_t token)const{std::lock_guard<std::mutex> lock(registryMutex_);const auto found=registry_.find(token);return found==registry_.end()?nullptr:found->second;}
juce::String CorrectionEngine::loadError()const{std::lock_guard<std::mutex> lock(registryMutex_);return loadError_;}
