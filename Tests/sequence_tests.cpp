// Keep assert-based DSP regression checks enabled in Release CI builds.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenitySequencer.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
 using namespace serenity;
 const auto original=defaultSequence();
 assert(original.length==8 && original.steps[2].pitch==12);
 StepClock clock;
 clock.prepare(48000);
 SequencePattern p=original;
 p.mode=2;p.length=8;p.tempo=120.f;p.rate=16;p.gate=.5f;
 p.steps[0].pitch=-5;
 p.steps[0].velocity=.8f;
 p.steps[0].modulation=.65f;
 p.steps[1].velocity=0.f; // rest
 clock.setPattern(p);
 assert(clock.currentStep()==0);
 for(int i=0;i<3000;i++){
  auto frame=clock.tick();
  assert(frame.index==0&&frame.pitch==-5&&frame.velocity==.8f);
  assert(frame.gate==1.f);
  assert(std::abs(frame.modulation-.65f)<1e-6f);
 }
 for(int i=0;i<3000;i++)assert(clock.tick().gate==0.f);
 assert(clock.currentStep()==1);
 for(int i=0;i<6000;i++){
  auto frame=clock.tick();
  assert(frame.index==1&&frame.velocity==0.f&&frame.gate==0.f);
 }
 assert(clock.currentStep()==2);
 for(int i=0;i<6000*6;i++)clock.tick();
 assert(clock.currentStep()==0); // loops after 8 steps
 p.swing=.25f;
 clock.reset();clock.setPattern(p);
 for(int i=0;i<7500;i++)clock.tick();
 assert(clock.currentStep()==1); // 1st step 1.25x duration
 for(int i=0;i<4500;i++)clock.tick();
 assert(clock.currentStep()==2); // 2nd step 0.75x duration
 p.length=64;p.mode=1;p.steps[63].pitch=24;p.steps[63].modulation=-1.f;
 clock.reset();clock.setPattern(p);
 int safety=0;
 while(clock.currentStep()!=63 && safety++<400000)clock.tick();
 assert(safety<400000 && clock.currentStep()==63);
 auto frame=clock.tick();
 assert(frame.pitch==24 && frame.modulation==0.f); // arpeggiator ignores mod lane
 p.mode=0;clock.setPattern(p);
 frame=clock.tick();
 assert(frame.index==-1&&frame.gate==1.f&&frame.velocity==1.f); // bypass
 // Independent v1.2.8 step clock: durationFor() was called on every
 // sample, while v1.2.9 caches the identical double result per parity.
 struct ReferenceClock {
  SequencePattern pattern{};
  double sr=48000.,sample=0.;
  int index=0;
  void prepare(double next){sr=std::max(8000.,next);sample=0.;index=0;}
  void set(const SequencePattern& next){
   pattern=next;
   pattern.length=std::clamp(pattern.length,1,64);
   pattern.gate=std::clamp(pattern.gate,0.f,1.f);
   pattern.swing=std::clamp(pattern.swing,0.f,.5f);
   pattern.tempo=std::clamp(pattern.tempo,20.f,300.f);
   pattern.mode=std::clamp(pattern.mode,0,2);
   index%=pattern.length;
  }
  int current()const{return pattern.mode==0?-1:index;}
  SequenceFrame tick(){
   if(pattern.mode==0)return {};
   const auto& step=pattern.steps[(size_t)index];
   const double bpm=std::max(20.,std::min(300.,(double)pattern.tempo));
   const int division=(pattern.rate==4||pattern.rate==8||
    pattern.rate==16||pattern.rate==32)?pattern.rate:16;
   const double nominal=sr*(240./bpm)/division;
   const double swing=std::max(0.,std::min(.5,(double)pattern.swing));
   const double duration=std::max(1.,nominal*((index%2==0)?(1.+swing):(1.-swing)));
   SequenceFrame f;
   f.pitch=std::clamp(step.pitch,-24,24);
   f.velocity=std::clamp(step.velocity,0.f,1.f);
   f.modulation=pattern.mode==2?std::clamp(step.modulation,-1.f,1.f):0.f;
   f.gate=(sample<duration*(double)pattern.gate&&f.velocity>0.f)?1.f:0.f;
   f.index=index;
   sample+=1.;
   if(sample>=duration){sample-=duration;index=(index+1)%pattern.length;}
   return f;
  }
 };
 for(double rate:{44100.,48000.,96000.}){
  StepClock optimized;
  ReferenceClock baseline;
  optimized.prepare(rate);baseline.prepare(rate);
  for(int config=0;config<6;config++){
   SequencePattern pat=defaultSequence();
   pat.length=5+config*7;pat.mode=1+config%2;
   pat.tempo=45.f+config*48.f;
   pat.rate=(config%4==0?4:config%4==1?8:config%4==2?16:32);
   pat.swing=config*.075f;pat.gate=.24f+config*.12f;
   for(int i=0;i<sequenceMaxSteps;i++){
    pat.steps[(size_t)i].pitch=(i%13)-6;
    pat.steps[(size_t)i].velocity=(i%5==0)?0.f:(.1f+i%8*.1f);
    pat.steps[(size_t)i].modulation=(i%9-4)*.21f;
   }
   optimized.setPattern(pat);baseline.set(pat);
   for(int n=0;n<9000;n++){
    const auto a=optimized.tick(),b=baseline.tick();
    assert(a.index==b.index&&a.pitch==b.pitch&&a.gate==b.gate);
    assert(a.velocity==b.velocity&&a.modulation==b.modulation);
    assert(optimized.currentStep()==baseline.current());
   }
  }
 }
 std::cout<<"PASS: 64 steps, note pitch, velocity/rest, mod, gate, 4 divisions, swing, and bypass\n";
}
