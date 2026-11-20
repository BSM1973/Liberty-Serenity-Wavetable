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
 std::cout<<"PASS: 64 steps, note pitch, velocity/rest, mod, gate, 4 divisions, swing, and bypass\n";
}
