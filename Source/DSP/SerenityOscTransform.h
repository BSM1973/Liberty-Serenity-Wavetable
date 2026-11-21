#pragma once
// Runtime-safe oscillator phase/shape transforms shared by DSP and native JUCE UI.
// No allocations, no locks, no UI assets, and no work outside scalar arithmetic.
#include <algorithm>
#include <cmath>

namespace serenity {
constexpr float oscTau=6.2831853071795864769f;
inline float oscillatorWarpPhase(float phase,float amount,int mode) noexcept {
 if(!std::isfinite(phase)||!std::isfinite(amount))return 0.f;
 phase-=std::floor(phase);
 amount=std::clamp(amount,-1.f,1.f);
 switch(mode) {
  case 1: { // bend: smooth non-linear phase remap
   const float warped=phase+amount*std::sin(oscTau*phase)/oscTau;
   return warped-std::floor(warped);
  }
  case 2: { // variable width, continuous and monotonic
   const float split=std::clamp(.5f+amount*.4f,.1f,.9f);
   return phase<split?(.5f*phase/split):
     (.5f+.5f*(phase-split)/(1.f-split));
  }
  case 3: { // synchronized double-wave phase blend
   const float doublePhase=2.f*phase-std::floor(2.f*phase);
   return (1.f-std::abs(amount))*phase+
          std::abs(amount)*doublePhase;
  }
  default:return phase;
 }
}
inline float oscillatorShape(float input,float amount,int mode) noexcept {
 if(!std::isfinite(input)||!std::isfinite(amount))return 0.f;
 input=std::clamp(input,-1.25f,1.25f);
 amount=std::clamp(amount,0.f,1.f);
 if(mode==0||amount<.00001f)return input;
 float altered=input;
 switch(mode) {
  case 1: { // soft saturation, normalized at +/-1
   const float drive=1.f+5.f*amount;
   altered=std::tanh(input*drive)/std::tanh(drive);
   break;
  }
  case 2: { // smooth wavefold
   altered=std::sin(input*(1.f+amount*2.f)*1.57079632679f);
   break;
  }
  case 3: { // quantization to 64..4 levels
   const float steps=std::max(4.f,64.f-60.f*amount);
   altered=std::round(input*steps)/steps;
   break;
  }
  default:return input;
 }
 return std::clamp(input*(1.f-amount)+altered*amount,-1.25f,1.25f);
}
} // namespace serenity
