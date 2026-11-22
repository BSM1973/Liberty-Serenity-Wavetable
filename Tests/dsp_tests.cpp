// Keep assert-based DSP regression checks enabled in Release CI builds.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenityDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
int main(){
 using namespace serenity;
 // Cached MIDI note frequency must exactly preserve the previous per-sample
 // fundamental at every legal MIDI note number.
 for(int note=0;note<128;note++){
  Voice voice;voice.start(note,.75f,1);
  const float before=440.f*std::pow(2.f,(note-69)/12.f);
  assert(std::isfinite(voice.baseFrequency)&&voice.baseFrequency==before);
 }
 const auto &presets=factoryPresets();
 assert(presets.size()==100);
 std::map<std::string,int> counts;
 for(auto &p:presets) counts[p.category]++;
 assert(counts["Bass"]==12 && counts["Pads"]==15 && counts["Cinematic"]==10);
 assert(counts["Leads"]==12 && counts["Plucks"]==10 && counts["Keys"]==8);
 assert(counts["Arps"]==10 && counts["EDM"]==8 && counts["Ambient"]==8 && counts["Experimental"]==7);
 Engine e;e.prepare(48000);Params p;e.setParams(p);
 float L[256]{},R[256]{};
 e.noteOn(60,.9f);
 double sum=0;
 for(int k=0;k<20;k++){std::fill_n(L,256,0);std::fill_n(R,256,0);e.render(L,R,256);
  for(int i=0;i<256;i++){assert(std::isfinite(L[i]) && std::isfinite(R[i]));sum+=L[i]*L[i];}}
 assert(sum>1.e-4);
 e.noteOff(60);
 for(int k=0;k<220;k++){std::fill_n(L,256,0);std::fill_n(R,256,0);e.render(L,R,256);}
 assert(e.numVoices()==0);
 for(double rate:{44100.,48000.,88200.,96000.,192000.}){
   e.prepare(rate);e.noteOn(64,1.f);std::fill_n(L,256,0);std::fill_n(R,256,0);e.render(L,R,256);
   for(float s:L)assert(std::isfinite(s));
 }
 std::cout<<"PASS: 100 factory definitions, output audio, release, finite samples at 5 rates\n";
}
