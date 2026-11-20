// Release builds must execute the audio quality checks, not compile them away.
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
 constexpr std::array<float,5> sampleRates={44100.f,48000.f,88200.f,96000.f,192000.f};
 spectral::Frame source{};
 for(int n=0;n<editableSamples;n++){
  const float x=spectral::tau*(float)n/editableSamples;
  source[(size_t)n]=.36f*std::sin(x)+.19f*std::sin(5.f*x)
                     +.11f*std::sin(24.f*x)+.13f*std::sin(90.f*x);
 }
 CustomMipFrame filtered{};
 buildCustomMipFrame(source,filtered);
 BandlimitedCustomWavetable bank;
 bank.setFrame(0,0,filtered);
 for(float sr:sampleRates) {
  // Each discontinuous old mip boundary is now a smooth pre-Nyquist transition.
  for(int mip=0;mip<customMipCount-1;mip++) {
   const int cap=1<<(customMipCount-1-mip);
   const float threshold=sr*.48f/(float)cap;
   const auto start=BandlimitedCustomWavetable::blendFor(threshold*.79f,sr);
   const auto half=BandlimitedCustomWavetable::blendFor(threshold*.90f,sr);
   const auto end=BandlimitedCustomWavetable::blendFor(threshold,sr);
   const auto above=BandlimitedCustomWavetable::blendFor(threshold*1.00005f,sr);
   assert(start.bright==mip && start.darkWeight==0.f);
   assert(half.bright==mip && half.dark==mip+1);
   assert(half.darkWeight>.35f && half.darkWeight<.65f);
   assert(end.dark==mip+1 && end.darkWeight>.999f);
   assert(above.bright==mip+1);
   // At the exact boundary, both branches must agree for every sampled phase.
   for(int k=0;k<64;k++){
    const float phase=(float)k/64.f;
    const float before=bank.sample(0,phase,0.f,threshold*.999995f,sr);
    const float after=bank.sample(0,phase,0.f,threshold*1.000005f,sr);
    assert(std::isfinite(before)&&std::isfinite(after));
    assert(std::abs(before-after)<.002f);
   }
   // Fade weight is strictly monotonic in the safe-side transition interval.
   float last=0.f;
   for(int k=0;k<=100;k++){
    const float hz=threshold*(.8f+.2f*(float)k/100.f);
    const auto selection=BandlimitedCustomWavetable::blendFor(hz,sr);
    assert(selection.bright==mip);
    assert(selection.darkWeight>=last-.00001f);
    assert(selection.darkWeight<=1.f+.00001f);
    last=selection.darkWeight;
   }
  }
  // An out-of-band 90th harmonic must not leak back above Nyquist.
  const float highFreq=sr*.48f/80.f;
  spectral::Frame waveform{};
  for(int k=0;k<editableSamples;k++)
   waveform[(size_t)k]=bank.sample(0,(float)k/editableSamples,0.f,highFreq,sr);
  const auto bins=spectral::analyzeBins(waveform);
  assert(2.f*std::abs(bins[90])/editableSamples<.001f);
  // The low-frequency fundamental survives under all host sample rates.
  assert(std::abs(bank.sample(0,.25f,0.f,150.f,sr))>.30f);
  for(int note=24;note<=127;note++){
   const float hz=440.f*std::pow(2.f,((float)note-69.f)/12.f);
   if(hz>sr*.48f)continue;
   const auto output=bank.sample(0,.124f,.46f,hz,sr);
   assert(std::isfinite(output));
   assert(std::abs(output)<1.0001f);
  }
 }
 // Frame interpolation must remain linear even inside a mip crossfade.
 spectral::Frame empty{};
 CustomMipFrame muted{};
 buildCustomMipFrame(empty,muted);
 bank.setFrame(0,1,muted);
 for(float sr:sampleRates){
  const float boundary=sr*.48f/64.f;
  const float frequency=.9f*boundary;
  const float a=bank.sample(0,.25f,0.f,frequency,sr);
  const float b=bank.sample(0,.25f,1.f/7.f,frequency,sr);
  const float center=bank.sample(0,.25f,1.f/14.f,frequency,sr);
  assert(std::abs(b)<.0001f);
  assert(std::abs(center-.5f*a)<.0005f);
 }
 std::cout<<"PASS: phase-coherent smooth mip transitions across 44.1,48,88.2,96,192 kHz; "
             "monotonic fade, safe harmonic suppression, frame morph and 104 MIDI pitches\n";
}
