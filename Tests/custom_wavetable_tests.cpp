// Keep assert-based DSP regression checks enabled in Release CI builds.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenityDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 using namespace serenity;
 EditableWavetable wave;
 wave.resetSine();
 assert(std::abs(wave.sample(2,.25f,0.f)-1.f)<.01f);
 assert(std::abs(wave.sample(2,.75f,0.f)+1.f)<.01f);
 for(int i=0;i<editableSamples;i++){
  wave.put(0,0,i,.25f);
  wave.put(0,1,i,-.25f);
 }
 assert(std::abs(wave.sample(0,.33f,0.f)-.25f)<1.e-6f);
 assert(std::abs(wave.sample(0,.33f,1.f/7.f)+.25f)<1.e-6f);
 assert(std::abs(wave.sample(0,.33f,1.f/14.f))<1.e-5f);
 wave.normalize(0,0);
 assert(std::abs(wave.get(0,0,30)-1.f)<1.e-6f);
 wave.removeDc(0,0);
 assert(std::abs(wave.get(0,0,30))<1.e-5f);
 wave.copyFrame(0,1,4);
 assert(std::abs(wave.get(0,4,0)+.25f)<1.e-6f);
 // Verify the custom 17th waveform is actually in the four-oscillator audio engine.
 Engine engine;
 engine.prepare(48000);
 Params params;
 params.wave[0]=waveCount;
 params.oscLevel[0]=.75f;
 params.position=.2f;
 for(int o=0;o<editableOscillators;o++)
  for(int f=0;f<editableFrames;f++)
   for(int i=0;i<editableSamples;i++)
    engine.setCustomSample(o,f,i,std::sin(6.2831853071795864769f*i/editableSamples));
 engine.setParams(params);
 engine.noteOn(60,.95f);
 float L[256]{},R[256]{};
 double energy=0;
 for(int block=0;block<20;block++){
  std::fill_n(L,256,0.f);std::fill_n(R,256,0.f);
  engine.render(L,R,256);
  for(int i=0;i<256;i++){
   assert(std::isfinite(L[i])&&std::isfinite(R[i]));
   energy+=(double)L[i]*L[i]+(double)R[i]*R[i];
  }
 }
 assert(energy>0.00001);
 std::cout<<"PASS: 4 x 8 x 512 custom frames, morph, normalize, DC removal and audible DSP\n";
}
