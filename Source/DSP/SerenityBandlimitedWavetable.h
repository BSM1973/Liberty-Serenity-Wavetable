#pragma once
// SERENITY WAVETABLE — band-limited custom tables, C++17.
// Generation occurs on the editing/host-state thread; audio only reads snapshots.
// Original editable wavetable layout remains 4 oscillators x 8 frames x 512 samples.
#include "SerenitySpectralEditor.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <complex>
#include <vector>

namespace serenity {
constexpr int customMipCount=9; // harmonic ceilings: 256,128,64,...,2,1
constexpr size_t customFrameMipSamples=(size_t)customMipCount*editableSamples;
using CustomMipFrame=std::array<float,customFrameMipSamples>;

inline void buildCustomMipFrame(const spectral::Frame& frame,CustomMipFrame& output) noexcept {
 const auto original=spectral::analyzeBins(frame);
 for(int mip=0;mip<customMipCount;mip++) {
  auto bins=original;
  const int limit=1<<(customMipCount-1-mip);
  // DC is preserved; 256 (Nyquist bin) is discarded for deterministic real output.
  bins[(size_t)editableSamples/2]={0.f,0.f};
  for(int k=1;k<editableSamples/2;k++) {
   if(k>limit) {
    bins[(size_t)k]={0.f,0.f};
    bins[(size_t)(editableSamples-k)]={0.f,0.f};
   }
  }
  spectral::fft(bins,true);
  for(int i=0;i<editableSamples;i++){
   const auto value=bins[(size_t)i].real();
   output[(size_t)mip*editableSamples+i]=std::isfinite(value)?value:0.f;
  }
 }
}

class BandlimitedCustomWavetable {
 std::vector<float> values; // allocated only during construction, never during render
 static size_t index(int osc,int frame,int mip,int sample) noexcept {
  return ((((size_t)osc*editableFrames+(size_t)frame)*customMipCount+
            (size_t)mip)*editableSamples+(size_t)sample);
 }
 float point(int osc,int frame,int mip,int sample) const noexcept {
  return values[index(osc,frame,mip,sample)];
 }
public:
 BandlimitedCustomWavetable():
  values((size_t)editableOscillators*editableFrames*customFrameMipSamples,0.f){
  spectral::Frame sine{};
  for(int i=0;i<editableSamples;i++)
   sine[(size_t)i]=std::sin(spectral::tau*(float)i/editableSamples);
  CustomMipFrame mips{};
  buildCustomMipFrame(sine,mips);
  for(int o=0;o<editableOscillators;o++)
   for(int f=0;f<editableFrames;f++)setFrame(o,f,mips);
 }
 void setFrame(int osc,int frame,const CustomMipFrame& mips) noexcept {
  if(osc<0||osc>=editableOscillators||frame<0||frame>=editableFrames)return;
  for(int mip=0;mip<customMipCount;mip++)
   for(int i=0;i<editableSamples;i++)
    values[index(osc,frame,mip,i)]=mips[(size_t)mip*editableSamples+i];
 }
 static int levelFor(float hz,float sampleRate) noexcept {
  if(!std::isfinite(hz)||!std::isfinite(sampleRate)||hz<=0.f||
     sampleRate<8000.f||hz>=sampleRate*.48f)return -1;
  const int allowed=std::max(1,(int)(sampleRate*.48f/hz));
  int mip=0;
  while(mip<customMipCount-1 && (1<<(customMipCount-1-mip))>allowed)++mip;
  return mip;
 }
 float sample(int osc,float phase,float framePosition,float hz,float sampleRate) const noexcept {
  if(osc<0||osc>=editableOscillators || !std::isfinite(phase) ||
     !std::isfinite(framePosition)) return 0.f;
  const int mip=levelFor(hz,sampleRate);
  if(mip<0)return 0.f;
  const float f=std::clamp(framePosition,0.f,1.f)*(editableFrames-1);
  const int a=(int)f,b=std::min(a+1,editableFrames-1);
  const float frameBlend=f-(float)a;
  const float t=(phase-std::floor(phase))*editableSamples;
  const int i=((int)t)&(editableSamples-1),j=(i+1)&(editableSamples-1);
  const float phaseBlend=t-std::floor(t);
  const float x=point(osc,a,mip,i)+
                (point(osc,a,mip,j)-point(osc,a,mip,i))*phaseBlend;
  const float y=point(osc,b,mip,i)+
                (point(osc,b,mip,j)-point(osc,b,mip,i))*phaseBlend;
  return x+(y-x)*frameBlend;
 }
 // Test/debug path; exposes generated table at a given mip level, not GUI data.
 float levelPoint(int osc,int frame,int mip,int sample) const noexcept {
  if(osc<0||osc>=editableOscillators||frame<0||frame>=editableFrames||
     mip<0||mip>=customMipCount||sample<0||sample>=editableSamples)return 0.f;
  return point(osc,frame,mip,sample);
 }
};
} // namespace serenity
