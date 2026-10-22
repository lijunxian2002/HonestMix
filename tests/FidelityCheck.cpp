#include "PluginProcessor.h"
#include "dsp/IRConversion.h"
#include <complex>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
using Complex=std::complex<double>;
static void check(bool v,const char* s){if(!v)throw std::runtime_error(s);}
static juce::var object(){return juce::var(new juce::DynamicObject());}
static void add(juce::var& v,const char* s,const juce::var& x){v.getDynamicObject()->setProperty(s,x);}
static Complex dtft(const float* h,int count,double rate,double frequency){Complex out{};for(int n=0;n<count;++n){double angle=-2*juce::MathConstants<double>::pi*frequency*n/rate;out+=(double)h[n]*Complex(std::cos(angle),std::sin(angle));}return out;}
static void process(HonestMixAudioProcessor& p,juce::AudioBuffer<float>& b){juce::MidiBuffer midi;p.processBlock(b,midi);}
static std::vector<float> impulse(HonestMixAudioProcessor& p,int length){std::vector<float> out;out.reserve((size_t)length+256);juce::AudioBuffer<float> b(2,256);for(int start=0;start<length;start+=256){b.clear();if(start==0)b.setSample(0,0,1);process(p,b);for(int i=0;i<256;++i)out.push_back(b.getSample(0,i));}return out;}
// Independent exact least-squares sine/cosine fit, no FFT or RMS-window bias.
static Complex sine(HonestMixAudioProcessor& p,double rate,double frequency){
 juce::AudioBuffer<float> b(2,256);const double w=2*juce::MathConstants<double>::pi*frequency/rate,amp=.0001;
 const int warm=(int)std::ceil(rate*.1/256)*256,total=warm+16384;double cc=0,ss=0,cs=0,yc=0,ys=0;
 for(int start=0;start<total;start+=256){for(int i=0;i<256;++i){auto value=(float)(amp*std::cos(w*(start+i)));b.setSample(0,i,value);b.setSample(1,i,value);}process(p,b);
  if(start>=warm)for(int i=0;i<256;++i){double c=std::cos(w*(start+i)),s=std::sin(w*(start+i)),y=b.getSample(0,i);cc+=c*c;ss+=s*s;cs+=c*s;yc+=y*c;ys+=y*s;}
 }
 const double d=cc*ss-cs*cs;return Complex((yc*ss-ys*cs)/d,-(ys*cc-yc*cs)/d)/amp;
}
int main(int argc,char** argv){auto report=object();add(report,"hardwareAudioOpened",false);add(report,"absoluteMagnitudeToleranceDb",.01);add(report,"phaseToleranceRadians",.001);juce::Array<juce::var> rows;double maxIR=0,maxSine=0,maxPhase=0,maxMix=0;
#ifdef FIDELITY_BASELINE
 constexpr bool evidenceOnly=true;add(report,"mode","frozen baseline evidence; failures retained");
#else
 constexpr bool evidenceOnly=false;add(report,"mode","patched hard-gate validation");
#endif
 try{
  for(int index:{0,1,924,802,730,756,1572})for(double rate:{44100.,48000.,96000.,192000.}){
   HonestMixAudioProcessor p;p.setProfileIndex(index);p.setTranslationDegree(100);p.prepareToPlay(rate,256);const int delay=evidenceOnly?p.getLatencySamples():headphoneIR::delay(rate),count=(int)std::ceil((1024+256)*rate/44100)+512;check(p.getLatencySamples()==delay,"Host latency must equal the explicit SRC delay");auto wet=impulse(p,count);const auto* h=p.getDatabase().getProfileFIR(index);
   auto row=object();add(row,"index",index);add(row,"rate",rate);add(row,"reportedLatency",delay);double rowIR=0,rowSine=0,rowPhase=0;
   add(row,"reportedTailSeconds",p.getTailLengthSeconds());const int expectedLength=rate==44100?1024:delay+(int)std::ceil(1151*rate/44100)+1;if(!evidenceOnly)check(std::abs(p.getTailLengthSeconds()-expectedLength/rate)<1e-9,"Reported tail must equal the complete converted IR duration");
   for(double frequency:{20.,100.,250.,1000.,1135.,4000.,10000.,16000.,20000.}){
    auto expected=dtft(h,1024,44100,frequency)*std::polar(1.,-2*juce::MathConstants<double>::pi*frequency*delay/rate);auto actual=dtft(wet.data(),(int)wet.size(),rate,frequency);
    rowIR=juce::jmax(rowIR,std::abs(20*std::log10(std::abs(actual/expected))));
    p.prepareToPlay(rate,256);auto measured=sine(p,rate,frequency);rowSine=juce::jmax(rowSine,std::abs(20*std::log10(std::abs(measured/expected))));rowPhase=juce::jmax(rowPhase,std::abs(std::arg(measured/expected)));
   }
   for(int point=0;point<384;++point){double frequency=20*std::pow(1000.,point/383.);auto expected=dtft(h,1024,44100,frequency)*std::polar(1.,-2*juce::MathConstants<double>::pi*frequency*delay/rate);auto actual=dtft(wet.data(),(int)wet.size(),rate,frequency);rowIR=juce::jmax(rowIR,std::abs(20*std::log10(std::abs(actual/expected))));}
   p.setTranslationDegree(0);p.prepareToPlay(rate,256);auto dry=impulse(p,count);int measuredDelay=-1;for(int n=0;n<(int)dry.size();++n)if(std::abs(dry[(size_t)n])>.5f){measuredDelay=n;break;}check(measuredDelay==delay,"Measured dry latency must equal host latency");for(int n=0;n<(int)dry.size();++n)check(std::abs(dry[(size_t)n]-(n==delay?1.f:0.f))<1e-6,"Dry path must be an exact integer delay");
   p.setTranslationDegree(200);p.prepareToPlay(rate,256);auto doubled=impulse(p,count);double mixError=0;for(int n=0;n<(int)wet.size();++n)mixError=juce::jmax(mixError,(double)std::abs(doubled[(size_t)n]-(2*wet[(size_t)n]-dry[(size_t)n])));
   if(rate==44100)for(int n=0;n<1024;++n)check(std::abs(wet[(size_t)n]-h[n])<.0002,"44.1kHz original impulse must remain unchanged");
   add(row,"measuredDryLatency",measuredDelay);add(row,"maxAbsoluteIrDbError",rowIR);add(row,"maxAbsoluteSineDbError",rowSine);add(row,"maxSinePhaseErrorRadians",rowPhase);add(row,"max200PercentMixError",mixError);add(row,"passed",rowIR<.01&&rowSine<.01&&rowPhase<.001&&mixError<.0002);rows.add(row);std::cout<<index<<" "<<rate<<" IR="<<rowIR<<" sine="<<rowSine<<" phase="<<rowPhase<<" mix="<<mixError<<std::endl;
   maxIR=juce::jmax(maxIR,rowIR);maxSine=juce::jmax(maxSine,rowSine);maxPhase=juce::jmax(maxPhase,rowPhase);maxMix=juce::jmax(maxMix,mixError);
   if(!evidenceOnly)check((bool)row["passed"],"Absolute magnitude, phase or dry/wet alignment fidelity failed");
  }
  add(report,"passed",maxIR<.01&&maxSine<.01&&maxPhase<.001&&maxMix<.0002);
 }catch(const std::exception& e){add(report,"passed",false);add(report,"error",e.what());}
 add(report,"denseDtftPoints",384);add(report,"sineFrequencies",9);add(report,"rows",rows);add(report,"maxAbsoluteIrDbError",maxIR);add(report,"maxAbsoluteSineDbError",maxSine);add(report,"maxSinePhaseErrorRadians",maxPhase);add(report,"max200PercentMixError",maxMix);if(argc>1){juce::File file(juce::String::fromUTF8(argv[1]));file.replaceWithText(juce::JSON::toString(report));}return evidenceOnly||((bool)report["passed"])?0:1;
}
