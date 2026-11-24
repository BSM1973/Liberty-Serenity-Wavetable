// A deliberately isolated SIMD feasibility study: never called by audio render.
// SSE2 and ARM NEON have arithmetic SIMD, but no cross-platform 4-way gather;
// loading four independent wavetable phases still costs scalar memory reads.
#include "SerenityHdWavetable.h"
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#if defined(__aarch64__) || defined(_M_ARM64)
 #include <arm_neon.h>
 #define SERENITY_STUDY_ISA "ARM NEON"
#elif defined(__SSE2__) || defined(_M_X64)
 #include <emmintrin.h>
 #define SERENITY_STUDY_ISA "SSE2"
#else
 #define SERENITY_STUDY_ISA "portable scalar"
#endif
using namespace serenity;
static void lerpFour(const float* lo,const float* hi,const float* t,float* result) noexcept {
#if defined(__aarch64__) || defined(_M_ARM64)
 const auto a=vld1q_f32(lo),b=vld1q_f32(hi),f=vld1q_f32(t);
 vst1q_f32(result,vaddq_f32(a,vmulq_f32(vsubq_f32(b,a),f)));
#elif defined(__SSE2__) || defined(_M_X64)
 const auto a=_mm_loadu_ps(lo),b=_mm_loadu_ps(hi),f=_mm_loadu_ps(t);
 _mm_storeu_ps(result,_mm_add_ps(a,_mm_mul_ps(_mm_sub_ps(b,a),f)));
#else
 for(int i=0;i<4;i++)result[i]=lo[i]+(hi[i]-lo[i])*t[i];
#endif
}
static std::array<float,4> atLevel(const HdTable::PreparedFrame& frame,
                                   int level,const std::array<float,4>& phase) noexcept {
 const int n=hdMipLength(level),offset=hdMipOffset(level);
 float a0[4],a1[4],b0[4],b1[4],frac[4],out[4],morphed[4];
 const float* a=frame.frameA+offset;
 const float* b=frame.frameB+offset;
 for(int i=0;i<4;i++){
  const float idx=phase[(size_t)i]*n;
  const int index=(int)idx,after=(index+1)&(n-1);
  frac[i]=idx-index;
  a0[i]=a[index];a1[i]=a[after];
  if(frame.blend>0.f){b0[i]=b[index];b1[i]=b[after];}
 }
 lerpFour(a0,a1,frac,out);
 if(frame.blend>0.f){
  lerpFour(b0,b1,frac,morphed);
  const float t[4]{frame.blend,frame.blend,frame.blend,frame.blend};
  float mid[4];lerpFour(out,morphed,t,mid);
  for(int i=0;i<4;i++)out[i]=mid[i];
 }
 return {out[0],out[1],out[2],out[3]};
}
static std::array<float,4> batchFour(const HdTable::PreparedFrame& frame,
                                     const HdTable::PreparedSpectral& spectral,
                                     const std::array<float,4>& phase) noexcept {
 auto a=atLevel(frame,spectral.mip,phase);
 if(spectral.mip==hdLevels-1||spectral.blend<=0.f)return a;
 const auto b=atLevel(frame,spectral.mip+1,phase);
 const float t[4]{spectral.blend,spectral.blend,spectral.blend,spectral.blend};
 float out[4];
 lerpFour(a.data(),b.data(),t,out);
 return {out[0],out[1],out[2],out[3]};
}
int main(){
 std::vector<float> samples((size_t)hdMaxFrames*hdSamples);
 for(int f=0;f<hdMaxFrames;f++)
  for(int i=0;i<hdSamples;i++){
   const float phase=(float)(2*hdPi*i/hdSamples);
   samples[(size_t)f*hdSamples+i]=.27f*std::sin(phase+.002f*f)
      +.13f*std::sin(3.f*phase)+.04f*std::sin(7.f*phase);
  }
 const HdTable table(std::move(samples));
 const auto frame=table.prepareFrame(.473f);
 const auto spectral=HdTable::prepareSpectral(700.f,48000.f);
 std::array<std::array<float,4>,256> phases{};
 for(size_t k=0;k<phases.size();k++)
  for(size_t i=0;i<4;i++)
   phases[k][i]=(float)((k*977u+i*137u)%10007u)/10007.f;
 float worst=0.f;
 for(const auto& row:phases){
  const auto y=batchFour(frame,spectral,row);
  for(int i=0;i<4;i++){
   const float ref=table.samplePreparedSpectralWrapped(row[(size_t)i],frame,spectral);
   worst=std::max(worst,std::abs(y[(size_t)i]-ref));
  }
 }
 if(!std::isfinite(worst)||worst>2.e-6f){
  std::cerr<<"SIMD interpolation parity failure: "<<worst<<"\n";return 2;
 }
 volatile float sink=0.f;
 using Clock=std::chrono::steady_clock;
 constexpr int iterations=320000;
 auto start=Clock::now();
 for(int n=0;n<iterations;n++){
  const auto& p=phases[(size_t)n%phases.size()];
  for(int i=0;i<4;i++)sink+=table.samplePreparedSpectralWrapped(p[(size_t)i],frame,spectral);
 }
 const double scalarMs=std::chrono::duration<double,std::milli>(Clock::now()-start).count();
 start=Clock::now();
 for(int n=0;n<iterations;n++){
  const auto& p=phases[(size_t)n%phases.size()];
  const auto v=batchFour(frame,spectral,p);
  for(auto x:v)sink+=x;
 }
 const double simdMs=std::chrono::duration<double,std::milli>(Clock::now()-start).count();
 std::ofstream out("hd-simd-study.txt");if(!out)return 3;
 out<<std::fixed<<std::setprecision(6);
 out<<"ISA: "<<SERENITY_STUDY_ISA<<"\n";
 out<<"Scalar 4-lane reference: "<<scalarMs<<" ms\n";
 out<<"SIMD arithmetic with scalar gathers: "<<simdMs<<" ms\n";
 out<<"SIMD/scalar runtime ratio: "<<simdMs/scalarMs<<"\n";
 out<<"Maximum absolute sample deviation: "<<worst<<"\n";
 out<<"Result: feasibility study only; no SIMD enabled in the audio engine.\n";
 out<<"Note: different wavetable phases require independent memory gathers; SSE2/NEON do not provide general gathered loads.\n";
 std::cout<<"HD SIMD feasibility: "<<SERENITY_STUDY_ISA
          <<" scalar "<<scalarMs<<" ms, vector-gather "<<simdMs
          <<" ms, max delta "<<worst<<", checksum "<<sink<<"\n";
 return 0;
}
