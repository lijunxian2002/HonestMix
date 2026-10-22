#pragma once
#include "UI/HeadphonePrediction.h"
// Independent CSV oracle: this test reads the actual pinned source files,
// searches their intervals sequentially, and uses natural log coordinates.
struct MeasuredCsv {
 std::vector<double> f,y;
 explicit MeasuredCsv(const juce::File& file){check(file.existsAsFile(),"Pinned measurement CSV required in portable source/delivery");juce::StringArray lines;lines.addLines(file.loadFileAsString());for(int i=1;i<lines.size();++i){const auto line=lines[i].trim();if(line.isEmpty())continue;const auto at=line.indexOfChar(',');check(at>0,"Malformed CSV");f.push_back(line.substring(0,at).getDoubleValue());y.push_back(line.substring(at+1).getDoubleValue());}check(f.size()==695,"All original raw measurement knots must be retained");}
 double at(double hz)const {
  if(hz<f.front()||hz>f.back()||!std::isfinite(hz))return std::numeric_limits<double>::quiet_NaN();
  if(hz==f.front())return y.front();
  for(std::size_t i=1;i<f.size();++i)if(hz<=f[i])return y[i-1]+(y[i]-y[i-1])*(std::log(hz)-std::log(f[i-1]))/(std::log(f[i])-std::log(f[i-1]));
  return y.back();
 }
 double relative(double hz)const {return at(hz)-at(1000.);}
};
static juce::File measurementInputs(){auto p=juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();for(int i=0;i<7;++i){auto folder=p.getChildFile("tools/profile-generation");if(folder.getChildFile("provenance.json").existsAsFile())return folder;p=p.getParentDirectory();}throw std::runtime_error("Actual pinned CSV inputs must travel with source and portable tools");}
static juce::var predictionTests(){
 const auto inputs=measurementInputs();const auto manifest=juce::JSON::parse(inputs.getChildFile("provenance.json"));const auto* profiles=manifest["profiles"].getArray();check(profiles&&profiles->size()==1573,"Independent CSV manifest has all1573 models");
 const MeasuredCsv targetCsv(inputs.getChildFile("target.csv"));const auto target=hm::measurement::target();HonestMixAudioProcessor names;std::size_t rawPoints=0;double rawError=0,targetError=0;
 check(hm::measurement::modelCount()==1573,"Compiled measurement count must match FIR database");
 for(int model=0;model<1573;++model){const auto& item=(*profiles)[model];check((int)item["index"]==model&&item["name"].toString()==names.getDatabase().getProfileName(model)&&item["name"].toString()==hm::measurement::modelName(model),"Measurement, FIR and profile-name identities must agree exactly");const MeasuredCsv csv(inputs.getChildFile(item["input"].toString()));const auto curve=hm::measurement::raw(model);
  for(std::size_t i=0;i<csv.f.size();++i){const double hz=csv.f[i],expected=csv.relative(hz);rawError=std::max(rawError,std::abs(curve.relativeDb(hz)-expected));check(rawError<1e-10,"Embedded unsmoothed measurement differs from pinned raw CSV");++rawPoints;
   if(i+1<csv.f.size()){const double between=std::sqrt(csv.f[i]*csv.f[i+1]);rawError=std::max(rawError,std::abs(curve.relativeDb(between)-csv.relative(between)));check(rawError<1e-10,"Raw log-frequency interpolation differs from independent CSV oracle");++rawPoints;}
  }
  check(std::isnan(curve.relativeDb(19.999))&&std::isnan(curve.relativeDb(20000))&&std::isnan(curve.relativeDb(std::numeric_limits<double>::quiet_NaN())),"Never extrapolate nonexistent headphone measurements");
 }
 for(double hz:{20.,47.,997.,1000.,3100.,10000.,19955.54}){targetError=std::max(targetError,std::abs(target.relativeDb(hz)-targetCsv.relative(hz)));check(targetError<1e-10,"Target must use original fixed1kHz reference, not an invented flat line");}
 juce::Array<juce::var> rows;double graphError=0,cachedError=0,audioError=0;std::size_t audioCases=0;
 for(int profile:{0,1,924,802,730,756,1572})for(double rate:{44100.,48000.,96000.,192000.}){
  HonestMixAudioProcessor p;p.setProfileIndex(profile);p.prepareToPlay(rate,256);const MeasuredCsv csv(inputs.getChildFile((*profiles)[profile]["input"].toString()));silence(p);const auto initial=p.captureResponseState();const auto basis=hm::prediction::build(initial);check(basis.measuredProfile==profile,"Prediction must bind measurement to actual active IR, not requested model");
  double rowZeroError=0,row100Variation=0;double min100=1e300,max100=-1e300;
  for(float degree:{0.f,25.f,50.f,75.f,100.f,125.f,150.f,175.f,200.f}){
   p.setTranslationDegree(degree);silence(p,(int)std::ceil(rate*.025/256)+2);const auto snapshot=p.captureResponseState();check(snapshot.isSteady()&&snapshot.appliedDegree==degree,"Prediction is compared only after real audio smoother has settled");
   for(double hz:{20.,47.,80.,997.,1135.,3100.,10000.,19937.}){
    const double raw=csv.relative(hz);const auto actualTransfer=expectedActualCorrection(*initial.activeIR,hz,degree);const double expected=raw+20*std::log10(std::abs(actualTransfer));const double actual=hm::prediction::evaluate(snapshot,hz);graphError=std::max(graphError,std::abs(actual-expected));check(graphError<=.01,"Predicted headphone frequency response must equal raw measurement plus actual DSP magnitude");
    if(degree==0){rowZeroError=std::max(rowZeroError,std::abs(actual-raw));check(rowZeroError<1e-10,"Zero must show original measured headphone, never identity gain");}
    if(degree==100){min100=std::min(min100,actual);max100=std::max(max100,actual);}
   }
   for(std::size_t i=0;i<basis.frequencies.size();i+=31){const double hz=basis.frequencies[i];if(hz>19955.54)continue;const double actual=hm::prediction::evaluate(snapshot,hz),cached=basis.evaluate(i,degree,0,false);cachedError=std::max(cachedError,std::abs(actual-cached));check(std::abs(actual-cached)<1e-6,"Dense rendered curve and exact hover must agree at the same applied degree");}
   for(double hz:{80.,997.,1135.,10000.,19937.}){
    const auto measuredPlugin=sine(p,rate,hz),exactTransfer=expectedActualCorrection(*initial.activeIR,hz,degree);if(std::abs(exactTransfer)>1e-4){const double observed=csv.relative(hz)+20*std::log10(std::abs(measuredPlugin));const double predicted=hm::prediction::evaluate(p.captureResponseState(),hz);audioError=std::max(audioError,std::abs(observed-predicted));check(audioError<=.02,"Actual sine audio magnitude plus measured headphone must equal displayed acoustic prediction");}else check(std::abs(measuredPlugin-exactTransfer)<2e-5,"Deep predicted audio null must retain old absolute-complex error gate");++audioCases;
   }
  }
  row100Variation=max100-min100;check(row100Variation>.05,"100% must retain computed target/residual structure rather than being forced flat");
  const auto bypass=sine(p,rate,997.,0,true);const auto state=p.captureResponseState();check(state.bypassed&&std::abs(hm::prediction::evaluate(state,997.)-csv.relative(997.))<1e-10&&std::abs(std::abs(bypass)-1)<2e-6,"Bypass must show original acoustic measurement regardless of saved slider200");
  auto row=object();add(row,"profile",profile);add(row,"sampleRate",rate);add(row,"degrees",9);add(row,"zeroOriginalMaximumDbError",rowZeroError);add(row,"actual100PercentVariationDb",row100Variation);add(row,"passed",true);rows.add(row);
 }
 auto result=object();add(result,"passed",true);add(result,"measurementProfiles",1573);add(result,"measurementKnotsPerProfile",695);add(result,"independentRawPoints",(juce::int64)rawPoints);add(result,"rawCsvDbError",rawError);add(result,"targetCsvDbError",targetError);add(result,"predictedGraphDbError",graphError);add(result,"denseVsHoverDbError",cachedError);add(result,"actualSinePredictedDbError",audioError);add(result,"actualSineCases",(juce::int64)audioCases);add(result,"rows",rows);add(result,"zeroMeansOriginalMeasurement",true);add(result,"fixedOriginal1kReferenceAtAllDegrees",true);add(result,"actualComplexMixNotDbMorph",true);add(result,"bypassMeansOriginalMeasurement",true);add(result,"noExtrapolation",true);add(result,"noForcedFlatOrInventedResidual",true);return result;
}
