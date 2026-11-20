#pragma once
// Serenity WAVETABLE — 512-sample FFT spectral editor.
// All transforms run only on the GUI/message thread, never in processBlock.
// Fixed-size FFT preserves phase and conjugate symmetry for real-valued output.
#include "SerenityCustomWavetable.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>

namespace serenity::spectral {
constexpr int fftSize=editableSamples;
constexpr int displayedHarmonics=32;
using Frame=std::array<float,fftSize>;
using Bins=std::array<std::complex<float>,fftSize>;
using Harmonics=std::array<float,displayedHarmonics>;
constexpr float tau=6.2831853071795864769f;

inline void fft(Bins& values,bool inverse=false) noexcept {
 for(int i=1,j=0;i<fftSize;i++){
  int bit=fftSize>>1;
  while(j&bit){j^=bit;bit>>=1;}
  j^=bit;
  if(i<j)std::swap(values[(size_t)i],values[(size_t)j]);
 }
 for(int len=2;len<=fftSize;len<<=1){
  const float angle=(inverse?tau:-tau)/(float)len;
  const auto root=std::complex<float>(std::cos(angle),std::sin(angle));
  for(int i=0;i<fftSize;i+=len){
   std::complex<float> w{1.f,0.f};
   for(int j=0;j<len/2;j++){
    auto u=values[(size_t)(i+j)];
    auto v=values[(size_t)(i+j+len/2)]*w;
    values[(size_t)(i+j)]=u+v;
    values[(size_t)(i+j+len/2)]=u-v;
    w*=root;
   }
  }
 }
 if(inverse)
  for(auto& c:values)c/=(float)fftSize;
}
inline Bins analyzeBins(const Frame& samples) noexcept {
 Bins bins{};
 for(int i=0;i<fftSize;i++)
  bins[(size_t)i]={std::isfinite(samples[(size_t)i])?samples[(size_t)i]:0.f,0.f};
 fft(bins);
 return bins;
}
inline Frame restore(Bins bins) noexcept {
 // Enforce exact real-conjugate symmetry before inverse FFT.
 bins[0]={bins[0].real(),0.f};
 bins[fftSize/2]={bins[fftSize/2].real(),0.f};
 for(int k=1;k<fftSize/2;k++)bins[(size_t)(fftSize-k)]=std::conj(bins[(size_t)k]);
 fft(bins,true);
 Frame values{};
 for(int i=0;i<fftSize;i++)
  values[(size_t)i]=std::clamp(bins[(size_t)i].real(),-1.f,1.f);
 return values;
}
inline Harmonics analyze(const Frame& frame) noexcept {
 auto bins=analyzeBins(frame);
 Harmonics harmonics{};
 for(int k=1;k<=displayedHarmonics;k++)
  harmonics[(size_t)k-1]=std::min(2.f,2.f*std::abs(bins[(size_t)k])/(float)fftSize);
 return harmonics;
}
inline Frame setHarmonic(const Frame& frame,int harmonic,float amplitude) noexcept {
 if(harmonic<1||harmonic>displayedHarmonics||!std::isfinite(amplitude))return frame;
 auto bins=analyzeBins(frame);
 auto c=bins[(size_t)harmonic];
 const float phase=std::abs(c)>.0001f?std::arg(c):-tau/4.f;
 bins[(size_t)harmonic]=std::polar((float)fftSize*.5f*std::clamp(amplitude,0.f,1.f),phase);
 bins[(size_t)(fftSize-harmonic)]=std::conj(bins[(size_t)harmonic]);
 return restore(bins);
}
enum class Transform : uint8_t {
 lowPass16, lowPass32, fundamental, oddOnly, evenOnly, phaseQuarterTurn,
 highPass4, smooth64
};
inline Frame transform(const Frame& frame,Transform mode) noexcept {
 auto bins=analyzeBins(frame);
 for(int k=1;k<fftSize/2;k++){
  bool keep=true;
  switch(mode){
   case Transform::lowPass16:keep=k<=16;break;
   case Transform::lowPass32:keep=k<=32;break;
   case Transform::fundamental:keep=k==1;break;
   case Transform::oddOnly:keep=(k%2)==1;break;
   case Transform::evenOnly:keep=(k%2)==0;break;
   case Transform::highPass4:keep=k>=5;break;
   case Transform::smooth64:keep=k<=64;break;
   case Transform::phaseQuarterTurn:break;
  }
  if(!keep)bins[(size_t)k]={0,0};
  else if(mode==Transform::phaseQuarterTurn)
   bins[(size_t)k]*=std::complex<float>(0.f,1.f);
 }
 if(mode!=Transform::phaseQuarterTurn)
  bins[fftSize/2]=(mode==Transform::evenOnly)?bins[fftSize/2]:std::complex<float>{0,0};
 for(int k=1;k<fftSize/2;k++)bins[(size_t)(fftSize-k)]=std::conj(bins[(size_t)k]);
 return restore(bins);
}
inline Frame morph(const Frame& from,const Frame& to,float t) noexcept {
 t=std::clamp(std::isfinite(t)?t:0.f,0.f,1.f);
 Frame out{};
 for(int i=0;i<fftSize;i++)
  out[(size_t)i]=(1.f-t)*from[(size_t)i]+t*to[(size_t)i];
 return out;
}
} // namespace serenity::spectral
