#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Core/HeadphoneDatabase.h"
#include "CorrectionResponse.h"
#include <array>
#include <atomic>
#include <map>
#include <mutex>

class CorrectionEngine final : private juce::Thread {
public:
 struct ProcessState {std::uint64_t activeToken=0,incomingToken=0,request=0;int activeProfile=-1,incomingProfile=-1;double blend=0;ProfilePhase phase=ProfilePhase::Unprepared;};
 explicit CorrectionEngine(const HeadphoneDatabase&);
 ~CorrectionEngine() override;
 void prepare(const juce::dsp::ProcessSpec&);
 void reset();
 void process(juce::AudioBuffer<float>&);
 void setEnabled(bool value) noexcept {enabled_.store(value);}
 bool isEnabled()const noexcept{return enabled_.load();}
 void setProfile(int index);
 void retryProfileLoad()noexcept; // Explicit user retry; never from process().
 void setProfileByName(const juce::String&);
 int getCurrentProfile()const noexcept{return activeProfile_.load();}
 int getRequestedProfile()const noexcept{return (int)(request_.load()&0xffff);}
 std::uint64_t requestedToken()const noexcept{return request_.load();}
 bool isLoading()const noexcept{return !hasLoadError()&&(getRequestedProfile()!=activeProfile_.load()||phase_.load()==(int)ProfilePhase::Warming||phase_.load()==(int)ProfilePhase::Fading);}
 int getNumProfiles()const noexcept{return db_.getNumProfiles();}
 const char* getProfileName(int index)const{return db_.getProfileName(index);}
 int getLatencySamples()const noexcept{return latency_.load();}
 double getTailLengthSeconds()const noexcept{return tailSeconds_.load();}
 ProcessState processState()const noexcept; // Audio/prepare owner only.
 std::shared_ptr<const HostRateIR> responseIR(std::uint64_t token)const; // Non-audio only.
 bool hasLoadError()const noexcept{return failedRequest_.load()==request_.load()&&failedSpec_.load()==specVersion_.load();}
 juce::String loadError()const;
private:
 struct Bundle {juce::dsp::Convolution convolution;std::shared_ptr<const HostRateIR> response;std::uint64_t request=0,specVersion=0;};
 Bundle* makeBundle(std::uint64_t request,const juce::dsp::ProcessSpec&,std::uint64_t version);
 void destroy(Bundle*);
 void retire(Bundle*)noexcept;
 bool readSpec(juce::dsp::ProcessSpec&,std::uint64_t&)const noexcept;
 void run()override;
 void reclaim();
 void publish()noexcept;
 const HeadphoneDatabase& db_;
 std::atomic<bool> enabled_{false};
 std::atomic<std::uint64_t> request_{1ull<<16},specVersion_{0},nextToken_{1};
 std::atomic<double> rate_{44100};std::atomic<unsigned> block_{1},channels_{2};
 std::atomic<int> activeProfile_{-1},phase_{(int)ProfilePhase::Unprepared},latency_{0};
 std::atomic<double> tailSeconds_{HeadphoneDatabase::kFirLength/44100.0};
 std::atomic<std::uint64_t> failedRequest_{0},failedSpec_{0};
 std::atomic<Bundle*> pending_{nullptr};
 Bundle* current_=nullptr;Bundle* incoming_=nullptr;
 juce::AudioBuffer<float> scratch_;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> fade_{0};
 int warmupRemaining_=0;bool fading_=false,prepared_=false;
 std::array<Bundle*,32> retired_{};
 std::atomic<unsigned> retiredRead_{0},retiredWrite_{0};
 mutable std::mutex registryMutex_;
 std::map<std::uint64_t,std::shared_ptr<const HostRateIR>> registry_;
 juce::String loadError_;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CorrectionEngine)
};
