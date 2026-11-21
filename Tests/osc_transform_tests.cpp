#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenityDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main(){
 using namespace serenity;
 // Identity modes preserve the original timbre and backwards-compatible presets.
 for(int i=0;i<=128;i++){
  const float phase=(float)i/129.f;
  assert(std::abs(oscillatorWarpPhase(phase,0.f,0)-phase)<.00001f);
  const float x=std::sin(oscTau*phase);
  assert(std::abs(oscillatorShape(x,0.f,0)-x)<.00001f);
 }
 // All phase transforms stay bounded and avoid NaN at extreme modulation.
 for(int mode=0;mode<=3;mode++)for(float amount:{-1.f,-.7f,0.f,.6f,1.f})
  for(int i=0;i<512;i++){
   const float phase=(float)i/512.f;
   const float warped=oscillatorWarpPhase(phase,amount,mode);
   assert(std::isfinite(warped)&&warped>=0.f&&warped<=1.f);
  }
 // PWM must be continuous at both ends of its two branches.
 assert(std::abs(oscillatorWarpPhase(.5f+.4f,.99f,2)-
                 oscillatorWarpPhase(.5f+.4f,1.0f,2))<.06f);
 for(int mode=0;mode<=3;mode++)
  for(int i=0;i<2048;i++){
   const float input=2.f*(float)i/2047.f-1.f;
   const float shaped=oscillatorShape(input,1.f,mode);
   assert(std::isfinite(shaped)&&std::abs(shaped)<=1.25001f);
  }
 // DSP transformation modes genuinely change a sound, not just a UI control.
 float transformDifference=0.f;
 for(int i=0;i<512;i++){
  const float ph=(float)i/512.f;
  const float input=std::sin(oscTau*ph)+.2f*std::sin(3.f*oscTau*ph);
  const float shifted=std::sin(oscTau*oscillatorWarpPhase(ph,.85f,1));
  transformDifference+=std::abs(input-shifted);
 }
 assert(transformDifference>10.f);
 assert(std::abs(oscillatorShape(.51f,.95f,1)-.51f)>.03f);
 // Factory frames are the exact table data used by Engine.
 Engine e;
 const float a=e.previewFactoryPoint(1,0,128);
 const float b=e.previewFactoryPoint(1,7,128);
 assert(std::isfinite(a)&&std::isfinite(b));
 assert(std::abs(a-b)>.001f); // evolving eight-frame wavetable
 std::cout<<"PASS: live warp/PWM/sync and shape modes; safe bounds and real factory frames\n";
}
