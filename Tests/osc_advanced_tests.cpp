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
  for(int i=0;i<block;i++)total+=std::abs(a[(size_t)i]-b[(size_t)i]);
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
 std::cout<<"PASS: true oscillator BLEND / PHASE / RAND PH DSP controls\n";
 return 0;
}
