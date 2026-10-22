#pragma once

// Presentation regression only. Silent offline callbacks below supply the real
// immutable prediction snapshot; this test makes no new audio-quality claim.
namespace motionqa {
static void advanceFor(HonestMixAudioProcessorEditor& ui,double seconds,double step=1.0/120.0){
 while(seconds>1e-10){const double dt=std::min(seconds,step);ui.advanceAnimation(dt);seconds-=dt;}
}
struct Model {
 std::vector<juce::Point<float>> vertices;
 std::array<double,5> db;
};
static constexpr std::array<double,5> modelHz{80.,200.,1000.,3000.,8000.};
static Model captureModel(const HonestMixAudioProcessorEditor& ui){
 check(ui.predictionReady()&&ui.plottedPrediction().size()>100,"Motion evidence requires an actual ready predicted-response curve");
 Model result;result.vertices=ui.plottedPrediction();
 for(size_t i=0;i<modelHz.size();++i){result.db[i]=ui.displayedResponseDb(modelHz[i]);check(std::isfinite(result.db[i]),"Motion comparison requires finite real supported response values");}
 return result;
}
static void sameModel(const HonestMixAudioProcessorEditor& ui,const Model& before){
 check(ui.predictionReady(),"Hiding a curve must not discard the ready response model");
 const auto& after=ui.plottedPrediction();check(after.size()==before.vertices.size(),"Presentation motion must not change prediction vertex count");
 for(size_t i=0;i<after.size();++i)check(after[i].x==before.vertices[i].x&&after[i].y==before.vertices[i].y,"Presentation motion must preserve every numeric prediction vertex exactly");
 for(size_t i=0;i<modelHz.size();++i)check(ui.displayedResponseDb(modelHz[i])==before.db[i],"Presentation motion must preserve exact numeric response values");
}
struct Capture {
 int scale=1;
 juce::Image away,hover,left,faded;
};
static juce::Image capture(HonestMixAudioProcessorEditor& ui,int scale,const juce::File& folder,const juce::String& state){
 juce::Rectangle<int> painted;auto image=ui.renderSurface(scale,painted);
 check(image.isValid()&&image.getWidth()==1000*scale&&image.getHeight()==480*scale&&painted==juce::Rectangle<int>(0,0,1000*scale,480*scale),"Motion screenshots must be actual native-scale editor renders");
 const auto file=folder.getChildFile("ui-motion-"+state+"-"+juce::String(scale*100)+"pct.png");
 auto stream=file.createOutputStream();check(stream&&juce::PNGImageFormat().writeImageToStream(image,*stream),"Native motion image write failed");return image;
}
struct PixelDifference { double mean=0;int samples=0;double greenA=0,greenB=0; };
// Probe only actual projected curve locations. Exclude exact-hover crosshairs
// and the follower region so their disappearance cannot pass a curve fade test.
static PixelDifference curveDifference(const juce::Image& a,const juce::Image& b,int scale,const Model& model,float hoverX,float hoverY){
 double sum=0,greenA=0,greenB=0;int count=0;const auto plot=HonestMixAudioProcessorEditor::plotBounds().reduced(4);
 float lastX=-1000;
 for(const auto point:model.vertices){
  if(!plot.contains(point)||point.x-lastX<4||std::abs(point.x-hoverX)<10||std::abs(point.y-hoverY)<8||juce::Rectangle<float>(290,155,155,125).contains(point))continue;
  lastX=point.x;const int cx=(int)std::lround(point.x*scale),cy=(int)std::lround(point.y*scale),radius=2*scale;
  for(int y=cy-radius;y<=cy+radius;++y)for(int x=cx-radius;x<=cx+radius;++x){
   const auto ca=a.getPixelAt(x,y),cb=b.getPixelAt(x,y);
   sum+=std::abs((int)ca.getRed()-(int)cb.getRed())+std::abs((int)ca.getGreen()-(int)cb.getGreen())+std::abs((int)ca.getBlue()-(int)cb.getBlue());++count;
   greenA+=std::max(0,(int)ca.getGreen()-std::max((int)ca.getRed(),(int)ca.getBlue()));greenB+=std::max(0,(int)cb.getGreen()-std::max((int)cb.getRed(),(int)cb.getBlue()));
  }
 }
 check(count>300,"Curve pixel comparison must sample a substantial real response path");return {sum/(3*count),count,greenA/count,greenB/count};
}
static juce::var followerTests(){
 juce::Array<juce::var> cases;std::array<juce::Point<float>,3> final{};size_t index=0;
 const juce::Point<float> target(680,300);
 for(int hz:{30,60,120}){
  HonestMixAudioProcessor p;std::unique_ptr<juce::AudioProcessorEditor> owned(p.createEditor());owned->getProperties().set("headlessTest",true);
  auto* ui=dynamic_cast<HonestMixAudioProcessorEditor*>(owned.get());check(ui,"Follower test requires the actual editor");
  if(ui->animationSnapshot().reducedMotion)ui->applyOption(10);
  const auto before=ui->animationSnapshot();auto move=mouseEvent(owned.get(),target.x,target.y,target.y,0);ui->mouseMove(move);advanceFor(*ui,.5,1.0/hz);const auto after=ui->animationSnapshot();
  const double remaining=after.follower.getDistanceFrom(target)/before.follower.getDistanceFrom(target);
  check(std::abs(remaining-std::exp(-2.4*.5))<.015,"Follower must use the slower elapsed-time response rather than the former fast chase");
  check(std::abs(after.elapsedSeconds-before.elapsedSeconds-.5)<1e-8,"Decorative elapsed time must follow the actual supplied duration");
  final[index++]=after.follower;auto row=object();add(row,"updatesPerSecond",hz);add(row,"elapsedSeconds",after.elapsedSeconds-before.elapsedSeconds);add(row,"remainingDistanceFraction",remaining);add(row,"x",after.follower.x);add(row,"y",after.follower.y);add(row,"passed",true);cases.add(row);
 }
 double maximumPositionDifference=0;for(const auto point:final)maximumPositionDifference=std::max(maximumPositionDifference,(double)point.getDistanceFrom(final[0]));
 check(maximumPositionDifference<.05,"Follower position at equal elapsed time must agree across 30/60/120 Hz updates");
 auto report=object();add(report,"cases",cases);add(report,"maximumPositionDifferenceLogicalPixels",maximumPositionDifference);add(report,"passed",true);return report;
}
}

static juce::var motionUiTests(const juce::File& folder){
 HonestMixAudioProcessor p;p.setTranslationDegree(100);p.prepareToPlay(48000,256);
 std::unique_ptr<juce::AudioProcessorEditor> owned(p.createEditor());owned->getProperties().set("headlessTest",true);
 auto* ui=dynamic_cast<HonestMixAudioProcessorEditor*>(owned.get());check(ui,"Motion test requires the actual headphone editor");juce::AudioBuffer<float> b(2,256);
 const auto deadline=juce::Time::getMillisecondCounterHiRes()+5000;
 do{b.clear();runAudio(p,b);ui->refreshFromProcessor();const auto state=p.captureResponseState();if(ui->predictionReady()&&!state.degreeTransition&&state.profilePhase==ProfilePhase::Stable)break;juce::Thread::sleep(2);}while(juce::Time::getMillisecondCounterHiRes()<deadline);
 const auto readyState=p.captureResponseState();check(readyState.audioReady&&!readyState.degreeTransition&&readyState.profilePhase==ProfilePhase::Stable&&readyState.appliedDegree==100&&readyState.sampleRate==48000,"Motion captures must await the real settled 100 percent / 48 kHz response state");
 const auto model=motionqa::captureModel(*ui);if(ui->animationSnapshot().reducedMotion)ui->applyOption(10);
 auto awayEvent=mouseEvent(owned.get(),950,50,50,0);ui->mouseMove(awayEvent);ui->mouseExit(awayEvent);motionqa::advanceFor(*ui,2.6);
 const auto away=ui->animationSnapshot();check(!away.graphHovered&&away.graphOpacity<.001f&&ui->hoveredHz()==0,"Graph must settle hidden after the pointer leaves, while retaining its ready model");motionqa::sameModel(*ui,model);
 std::array<motionqa::Capture,2> images;images[0].scale=1;images[1].scale=2;
 for(auto& shot:images)shot.away=motionqa::capture(*ui,shot.scale,folder,"away");

 const float hoverX=340;auto hoverEvent=mouseEvent(owned.get(),hoverX,205,205,0);ui->mouseMove(hoverEvent);
 check(ui->animationSnapshot().graphHovered&&ui->hoveredHz()>20&&ui->hoveredHz()<20000,"Actual graph mouse movement must retain the supported exact frequency readout");
 const double hoveredHz=ui->hoveredHz(),hoveredDb=ui->displayedResponseDb(hoveredHz);const float hoverY=(float)(230-hoveredDb*224/48);
 motionqa::advanceFor(*ui,.18);const auto enter=ui->animationSnapshot();
 const double expectedEnter=1-(1-away.graphOpacity)*std::exp(-1.);
 check(std::abs(enter.graphOpacity-expectedEnter)<.015,"Graph fade-in must follow the 180 ms presentation time constant");
 motionqa::advanceFor(*ui,1.62);const auto visible=ui->animationSnapshot();check(visible.graphOpacity>.995f,"Sustained real hover must make the response curve visible");motionqa::sameModel(*ui,model);
 for(auto& shot:images)shot.hover=motionqa::capture(*ui,shot.scale,folder,"hover");

 ui->mouseExit(awayEvent);const auto left=ui->animationSnapshot();
 check(!left.graphHovered&&ui->hoveredHz()==0&&std::abs(left.graphOpacity-visible.graphOpacity)<1e-6f,"Actual mouse exit must clear the readout immediately and leave the curve to fade");motionqa::sameModel(*ui,model);
 for(auto& shot:images)shot.left=motionqa::capture(*ui,shot.scale,folder,"left");
 motionqa::advanceFor(*ui,.26);const auto leaving=ui->animationSnapshot();
 check(std::abs(leaving.graphOpacity-left.graphOpacity*std::exp(-1.))<.015,"Graph fade-out must follow the 260 ms presentation time constant");
 motionqa::advanceFor(*ui,2.34);const auto faded=ui->animationSnapshot();check(faded.graphOpacity<.001f,"Graph must return to hidden after its real leave fade");motionqa::sameModel(*ui,model);
 for(auto& shot:images)shot.faded=motionqa::capture(*ui,shot.scale,folder,"faded");

 juce::Array<juce::var> pixelCases;
 for(const auto& shot:images){
  const auto hovered=motionqa::curveDifference(shot.hover,shot.away,shot.scale,model,hoverX,hoverY);
  const auto justLeft=motionqa::curveDifference(shot.left,shot.away,shot.scale,model,hoverX,hoverY);
  const auto afterFade=motionqa::curveDifference(shot.faded,shot.away,shot.scale,model,hoverX,hoverY);
  const auto exitChange=motionqa::curveDifference(shot.hover,shot.left,shot.scale,model,hoverX,hoverY);
  check(hovered.mean>1&&justLeft.mean>1,"Native hover/left pixels must contain the real response stroke instead of only a changed opacity field");
  check(justLeft.mean>=hovered.mean*.80&&exitChange.mean<hovered.mean*.20+.25,"Mouse exit must retain the visible response stroke until elapsed fade time advances");
  check(afterFade.mean<hovered.mean*.25+.35,"Actual curve-region pixels must fade back toward the away render at each native scale");
  // The 100 percent response is green, while its dashed target is neutral.
  // This independent ink probe prevents a fading target alone from passing.
  check(hovered.greenA>.5&&hovered.greenB<hovered.greenA*.15+.05,"The actual green 100 percent response stroke must be visible on hover and absent while away");
  check(justLeft.greenA>=hovered.greenA*.90&&afterFade.greenA<hovered.greenA*.15+.05,"The response stroke itself must remain immediately after exit and then fade away");
  auto row=object();add(row,"scale",shot.scale);add(row,"curvePixelSamples",hovered.samples);add(row,"hoverVsAwayMeanRgbDifference",hovered.mean);add(row,"leftVsAwayMeanRgbDifference",justLeft.mean);add(row,"hoverVsLeftMeanRgbDifference",exitChange.mean);add(row,"fadedVsAwayMeanRgbDifference",afterFade.mean);add(row,"awayMeanGreenInk",hovered.greenB);add(row,"hoverMeanGreenInk",hovered.greenA);add(row,"leftMeanGreenInk",justLeft.greenA);add(row,"fadedMeanGreenInk",afterFade.greenA);add(row,"passed",true);pixelCases.add(row);
 }

 ui->applyOption(10);check(ui->animationSnapshot().reducedMotion&&ui->animationSnapshot().graphOpacity==0,"Reduce motion must snap an away graph to hidden");
 ui->mouseMove(hoverEvent);const auto reduced=ui->animationSnapshot();check(reduced.graphOpacity==1&&reduced.graphHovered&&ui->hoveredHz()==hoveredHz&&ui->displayedResponseDb(hoveredHz)==hoveredDb,"Reduced-motion pointer input must reveal the same exact response immediately");
 motionqa::advanceFor(*ui,.75);const auto frozen=ui->animationSnapshot();check(frozen.elapsedSeconds==reduced.elapsedSeconds&&frozen.follower==reduced.follower,"Reduce motion must freeze decorative time and follower movement");
 ui->mouseExit(awayEvent);check(ui->animationSnapshot().graphOpacity==0&&ui->hoveredHz()==0,"Reduced-motion exit must hide the graph immediately");motionqa::sameModel(*ui,model);
 check(p.getTranslationDegree()==100&&p.getListenMode()==ListenNormal,"Presentation input must not change the actual correction or listen-mode parameters");

 auto report=object();add(report,"scope","Presentation only; real response supplied by silent offline processor warmup, no new audio-quality acceptance");add(report,"appliedDegree",readyState.appliedDegree);add(report,"sampleRate",readyState.sampleRate);add(report,"readyPredictionVertexCount",(int)model.vertices.size());add(report,"nativeRenderCount",8);add(report,"nativeScalePixelComparisons",pixelCases);add(report,"awayOpacity",away.graphOpacity);add(report,"enter180msOpacity",enter.graphOpacity);add(report,"hoverOpacity",visible.graphOpacity);add(report,"immediatelyLeftOpacity",left.graphOpacity);add(report,"leave260msOpacity",leaving.graphOpacity);add(report,"fadedOpacity",faded.graphOpacity);add(report,"numericResponseAndVerticesUnchanged",true);add(report,"reducedMotionImmediateAndFrozen",true);add(report,"follower",motionqa::followerTests());add(report,"passed",true);return report;
}
