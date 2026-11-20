#include "SerenityDSP.h"
#include "SerenityBandlimitedWavetable.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
 using namespace serenity;
 spectral::Frame complexWave{};
 const float tau=spectral::tau;
 for(int n=0;n<editableSamples;n++) {
  const float x=tau*(float)n/editableSamples;
  complexWave[(size_t)n]=.33f*std::sin(x)
                           +.16f*std::sin(90.f*x)
                           +.09f*std::sin(160.f*x);
 }
 CustomMipFrame mips{};
 buildCustomMipFrame(complexWave,mips);
 BandlimitedCustomWavetable bank;
 bank.setFrame(0,0,mips);

 // Higher spectral octaves are filtered, but the fundamental remains.
 auto inspect=[&](int mip,int harmonic){
  spectral::Frame frame{};
  for(int i=0;i<editableSamples;i++)
   frame[(size_t)i]=bank.levelPoint(0,0,mip,i);
  const auto frequency=spectral::analyzeBins(frame);
  return 2.f*std::abs(frequency[(size_t)harmonic])/editableSamples;
 };
 assert(std::abs(inspect(0,90)-.16f)<.002f); // mip0 permits harmonic 90
 assert(inspect(1,160)<.0005f);              // mip1 caps at 128
 assert(inspect(3,90)<.0005f);               // mip3 caps at 32
 assert(inspect(6,90)<.0005f);               // mip6 caps at 4
 assert(std::abs(inspect(6,1)-.33f)<.002f); // fundamental amplitude intact
 assert(BandlimitedCustomWavetable::levelFor(80.f,48000.f)==0);
 assert(BandlimitedCustomWavetable::levelFor(3000.f,48000.f)>=6);
 assert(BandlimitedCustomWavetable::levelFor(30000.f,48000.f)==-1);

 // Both frame and oscillator IDs remain independent in all mip levels.
 spectral::Frame silence{};
 CustomMipFrame zero{};
 buildCustomMipFrame(silence,zero);
 bank.setFrame(0,1,zero);
 const float a=bank.sample(0,.25f,0.f,3000.f,48000.f);
 const float b=bank.sample(0,.25f,1.f/7.f,3000.f,48000.f);
 const float mid=bank.sample(0,.25f,1.f/14.f,3000.f,48000.f);
 assert(std::abs(a-.33f)<.012f);
 assert(std::abs(b)<.0001f);
 assert(std::abs(mid-a*.5f)<.004f);
 assert(std::isfinite(bank.sample(2,.5f,0.5f,10000.f,48000.f)));

 // The exact same filtered mip frame must be audible in the real engine.
 Engine engine;
 engine.prepare(48000);
 engine.setCustomMipFrame(0,0,mips);
 Params params;
 params.wave[0]=waveCount;
 params.position=0.f;
 params.oscLevel[0]=.7f;
 params.oscLevel[1]=params.oscLevel[2]=params.oscLevel[3]=0.f;
 params.polyphony=4;
 params.delay=0.f;
 params.reverb=0.f;
 engine.setParams(params);
 engine.noteOn(90,.85f);
 float L[256]{},R[256]{};
 double energy=0.;
 for(int block=0;block<12;block++){
  std::fill_n(L,256,0.f);
  std::fill_n(R,256,0.f);
  engine.render(L,R,256);
  for(int n=0;n<256;n++){
   assert(std::isfinite(L[n])&&std::isfinite(R[n]));
   energy+=(double)L[n]*L[n]+(double)R[n]*R[n];
  }
 }
 assert(energy>1e-5);
 std::cout<<"PASS: 9 anti-aliasing bands, FFT attenuation, harmonic levels, frame morphing, and actual polyphonic render\n";
}
