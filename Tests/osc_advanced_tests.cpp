#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenityDSP.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
 using namespace serenity;
 Engine e;
 e.prepare(48000.0);
 Params p;
 p.master=.8f;
 p.drive=0.f;p.delay=0.f;p.reverb=0.f;
 p.oscLevel[0]=.7f;p.unison[0]=4;
 for(int gen=1;gen<4;gen++)p.oscLevel[gen]=0.f;
 constexpr int block=2048;
 using Samples=std::array<float,block>;
 auto render=[&](const Params& settings){
  e.reset();e.setParams(settings);e.noteOn(60,.95f);
  Samples left{},right{};
  e.render(left.data(),right.data(),block);
  for(float v:left)assert(std::isfinite(v));
  for(float v:right)assert(std::isfinite(v));
  return left;
 };
 auto difference=[](const Samples& a,const Samples& b){
  double total=0.0;
  for(size_t i=0;i<a.size();i++)total+=std::abs(a[i]-b[i]);
  return total;
 };
 const auto original=render(p);
 p.oscPhase[0]=.31f;
 assert(difference(original,render(p))>1.0);
 p.oscPhase[0]=0.f;
 p.oscRandPhase[0]=.85f;
 assert(difference(original,render(p))>1.0);
 p.oscRandPhase[0]=0.f;
 p.oscBlend[0]=1.f;
 assert(difference(original,render(p))>.01);
 p.oscBlend[0]=0.f;
 assert(difference(original,render(p))<.0001);
 // Cross-modulation is tested on two independently enabled oscillators.
 p.oscLevel[1]=.5f;p.oscSemi[1]=7.f;
 const auto twoOsc=render(p);
 p.fmBA=.85f;assert(difference(twoOsc,render(p))>1.0);
 p.fmMode=1;assert(difference(twoOsc,render(p))>1.0);
 p.fmBA=0;p.fmMode=0;p.ringAmount=.85f;assert(difference(twoOsc,render(p))>1.0);
 p.ringAmount=0;p.tremDepth=1;assert(difference(twoOsc,render(p))>1.0);
 p.tremDepth=0;p.fxRingMix=1;assert(difference(twoOsc,render(p))>1.0);
 p.fxRingMix=0;p.envDelay=.08f;
 const auto delayed=render(p);for(float v:delayed)assert(std::abs(v)<.00001f);
 p.envDelay=0;
 Envelope envelope;EnvelopeSettings cfg;cfg.delay=.01f;cfg.attack=.001f;cfg.hold=.02f;
 envelope.on();
 for(int i=0;i<400;i++)assert(envelope.next(cfg,48000.f)==0.f);
 for(int i=0;i<150;i++)envelope.next(cfg,48000.f);
 for(int i=0;i<500;i++)assert(envelope.next(cfg,48000.f)>.999f);
 for(int shape=0;shape<=4;shape++)for(int i=0;i<1000;i++){auto v=lfoWave(i*.01f,shape);assert(std::isfinite(v)&&std::abs(v)<=1.001f);}
 assert(lfoWave(.125f,0)!=lfoWave(.125f,3));
 e.route(0)={7,1,.75f};
 p.lfos[1].shape=0;
 const auto sineMod=render(p);
 p.lfos[1].shape=3;
 assert(difference(sineMod,render(p))>.01);
 e.route(0)={};
 p.lfos[1].shape=0;
 const auto dry=render(p);
 p.envCurve=.9f;assert(difference(dry,render(p))>.01);p.envCurve=0;
 LfoSettings sync;sync.sync=true;sync.division=5;
 assert(std::abs(lfoFrequency(sync,5,120)-4.f)<.001f);
 assert(std::abs(lfoFrequency(sync,5,90)-3.f)<.001f);
 p.eqLowGain=12;assert(difference(dry,render(p))>.01);p.eqLowGain=0;
 p.eqMidGain=-12;assert(difference(dry,render(p))>.01);p.eqMidGain=0;
 p.eqHighGain=12;assert(difference(dry,render(p))>.01);p.eqHighGain=0;
 p.compMix=1;p.compThreshold=-48;assert(difference(dry,render(p))>.01);p.compMix=0;
 for(int type=0;type<3;type++){p.modFxType=type;p.modFxMix=1;assert(difference(dry,render(p))>.01);}
 p.modFxMix=0;
 std::cout<<"PASS: curved envelopes, tempo divisions, EQ, compressor and stereo modulation FX\n";
 return 0;
}
