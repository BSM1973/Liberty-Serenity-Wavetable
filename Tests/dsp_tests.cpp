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
 // v1.2.7: never prepare fixed-pitch plans for modulated notes.
 // A bypassed sequencer must not disable the optimization.
 std::array<ModRoute,64> pitchRoutes{};
 assert(!hasRoutedPitchModulation(pitchRoutes));
 pitchRoutes[0]={14,1,.7f}; // Wavetable morph does not change frequency.
 assert(!hasRoutedPitchModulation(pitchRoutes));
 pitchRoutes[1]={0,0,.25f}; // Envelope -> pitch: dynamic for all generators.
 assert(hasRoutedPitchModulation(pitchRoutes));
 pitchRoutes[1].amount=0.f;
 assert(!hasRoutedPitchModulation(pitchRoutes));
 SequencePattern pitched=defaultSequence();
 assert(!sequencerMayTranspose(pitched)); // mode 0 bypass
 pitched.mode=2;
 assert(sequencerMayTranspose(pitched));
 for(auto& step:pitched.steps)step.pitch=0;
 assert(!sequencerMayTranspose(pitched));
 pitched.steps[0].pitch=7;
 assert(sequencerMayTranspose(pitched));
 Params fm;
 fm.fmMode=0;fm.fmBA=.5f;
 for(int gen=0;gen<generators;gen++)assert(!oscillatorHasFrequencyFm(fm,gen));
 fm.fmMode=1;
 assert(oscillatorHasFrequencyFm(fm,0));
 assert(!oscillatorHasFrequencyFm(fm,1));
 assert(!oscillatorHasFrequencyFm(fm,2));
 fm.fmBA=0.f;fm.fmCB=.35f;
 assert(!oscillatorHasFrequencyFm(fm,0));
 assert(oscillatorHasFrequencyFm(fm,1));
 assert(!oscillatorHasFrequencyFm(fm,2));
 fm.fmCB=0.f;fm.fmCA=.5f;
 assert(oscillatorHasFrequencyFm(fm,0));
 assert(!oscillatorHasFrequencyFm(fm,1));
 const auto &presets=factoryPresets();
 assert(presets.size()==100);
 std::map<std::string,int> counts;
 for(auto &p:presets) counts[p.category]++;
 assert(counts["Bass"]==12 && counts["Pads"]==15 && counts["Cinematic"]==10);
 assert(counts["Leads"]==12 && counts["Plucks"]==10 && counts["Keys"]==8);
 assert(counts["Arps"]==10 && counts["EDM"]==8 && counts["Ambient"]==8 && counts["Experimental"]==7);
 // Independent original SVF implementation, compared sample for sample
 // against the coefficient-sharing API at every filter mode and resonance.
 struct LegacyFilter {
  Svf a,b;
  float run(float x,float cutoff,float resonance,float drive,int mode,float sr){
   if(drive>0){float g=1.f+drive*5.f;x=std::tanh(x*g)/g;}
   float g=std::tan(pi*clamp(cutoff,10.f,sr*.45f)/sr);
   float k=2.f-1.85f*clamp(resonance,0.f,1.f);
   float y=a.tick(x,g,k,mode);
   return doubleFilter(mode)?b.tick(y,g,k,mode):y;
  }
 };
 for(int mode=0;mode<15;mode++)for(int cfg=0;cfg<8;cfg++){
  FilterChannel optimized;
  LegacyFilter original;
  const float sampleRate=cfg&1?96000.f:44100.f;
  const float resonance=(cfg%4)*.2f;
  const float drive=cfg>=4?.35f:0.f;
  for(int n=0;n<2048;n++){
   const float cutoff=60.f+14800.f*(.5f+.5f*std::sin(.0103f*n+.9f*cfg));
   const float value=.62f*std::sin(n*.127f)+.11f*std::cos(n*.71f);
   const float g=std::tan(pi*clamp(cutoff,10.f,sampleRate*.45f)/sampleRate);
   const float k=2.f-1.85f*clamp(resonance,0.f,1.f);
   const float x=original.run(value,cutoff,resonance,drive,mode,sampleRate);
   const float y=optimized.runPrepared(value,g,k,drive,mode);
   if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x-y)>1.e-6f){
    std::cerr<<"Filter coefficient sharing changed audio mode "<<mode<<" cfg "<<cfg<<" sample "<<n<<std::endl;
    return 91;
   }
  }
 }
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
