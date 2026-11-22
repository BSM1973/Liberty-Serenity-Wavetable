#pragma once
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace serenity {
constexpr int hdSamples=2048,hdMaxFrames=256,hdLevels=11;
constexpr double hdPi=3.1415926535897932384626433832795;
using HdBin=std::complex<float>;
inline void hdFft(HdBin* data,int n,bool inverse) noexcept {
 for(int i=1,j=0;i<n;i++){int bit=n>>1;while(j&bit){j^=bit;bit>>=1;}j^=bit;if(i<j)std::swap(data[i],data[j]);}
 for(int len=2;len<=n;len<<=1){
  const double a=(inverse?2.0:-2.0)*hdPi/len;
  for(int j=0;j<len/2;j++){
   const HdBin w((float)std::cos(a*j),(float)std::sin(a*j));
   for(int i=j;i<n;i+=len){auto u=data[i],v=data[i+len/2]*w;data[i]=u+v;data[i+len/2]=u-v;}
  }
 }
 if(inverse)for(int i=0;i<n;i++)data[i]/=(float)n;
}
// Constant-time offset lookup in the critical oscillator sample path.
inline constexpr std::array<int,hdLevels+1> hdMipOffsets={
 0,2048,3072,3584,3840,3968,4032,4096,4160,4224,4288,4352
};
inline constexpr int hdMipLength(int mip) noexcept {return std::max(64,hdSamples>>mip);}
inline constexpr int hdMipOffset(int mip) noexcept {return hdMipOffsets[(size_t)mip];}
inline constexpr int hdMipStride() noexcept {return hdMipOffsets[(size_t)hdLevels];}
static_assert(hdMipStride()==4352,"HD spectral stride must remain stable");

inline std::vector<float> hdConvert(const float* source,int count,int cycle){
 if(!source||count<2||cycle<2||cycle>count||cycle>4096||count%cycle||count/cycle>hdMaxFrames)return {};
 for(int i=0;i<count;i++)if(!std::isfinite(source[i]))return {};
 const int frames=count/cycle;
 std::vector<float> pcm((size_t)frames*hdSamples);
 for(int f=0;f<frames;f++){
  const float* in=source+(size_t)f*cycle;float* dst=pcm.data()+(size_t)f*hdSamples;
  if(cycle==hdSamples){for(int i=0;i<hdSamples;i++)dst[i]=std::clamp(in[i],-1.f,1.f);continue;}
  // Fourier-domain cyclic resampling for the supported power-of-two sizes.
  if((cycle&(cycle-1))==0){
   std::vector<HdBin> bins((size_t)cycle);for(int i=0;i<cycle;i++)bins[i]={std::clamp(in[i],-1.f,1.f),0.f};
   hdFft(bins.data(),cycle,false);
   std::array<HdBin,hdSamples> out{};const float scale=(float)hdSamples/cycle;
   out[0]=bins[0]*scale;
   int limit=std::min((cycle-1)/2,hdSamples/2-1);
   for(int k=1;k<=limit;k++){out[k]=bins[k]*scale;out[hdSamples-k]=std::conj(out[k]);}
   if(cycle<hdSamples){out[cycle/2]=bins[cycle/2]*scale*.5f;out[hdSamples-cycle/2]=std::conj(out[cycle/2]);}
   hdFft(out.data(),hdSamples,true);
   for(int i=0;i<hdSamples;i++)dst[i]=std::clamp(out[i].real(),-1.f,1.f);
  }else{
   const double ratio=(double)hdSamples/cycle,fc=.475*std::min(1.,ratio);
   const int radius=std::max(16,(int)std::ceil(12./std::min(1.,ratio)));
   for(int i=0;i<hdSamples;i++){
    const double pos=(double)i*cycle/hdSamples;int center=(int)std::floor(pos);double sum=0,w=0;
    for(int j=-radius+1;j<=radius;j++){double x=pos-center-j,z=2*hdPi*fc*x,k=2*fc*(std::abs(z)<1e-12?1.:std::sin(z)/z)*(.5+.5*std::cos(hdPi*x/radius));int idx=(center+j)%cycle;if(idx<0)idx+=cycle;sum+=in[idx]*k;w+=k;}
    dst[i]=(float)std::clamp(std::abs(w)>1e-12?sum/w:0.,-1.,1.);
   }
  }
 }
 return pcm;
}
class HdTable {
public:
 const int frames;
 const std::vector<float> pcm;
private:
 std::vector<float> mips;
 static float interp(const float* samples,int n,float phase) noexcept {
  const float x=(phase-std::floor(phase))*n;const int i=(int)x&(n-1),j=(i+1)&(n-1);
  return samples[i]+(samples[j]-samples[i])*(x-std::floor(x));
 }
 float at(int frame,int mip,float phase)const noexcept {
  return interp(mips.data()+(size_t)frame*hdMipStride()+hdMipOffset(mip),hdMipLength(mip),phase);
 }
public:
 explicit HdTable(std::vector<float> values):frames((int)(values.size()/hdSamples)),pcm(std::move(values)),mips((size_t)frames*hdMipStride()){
  std::array<HdBin,hdSamples> bins{},out{};
  for(int f=0;f<frames;f++){
   for(int i=0;i<hdSamples;i++)bins[i]={pcm[(size_t)f*hdSamples+i],0.f};
   hdFft(bins.data(),hdSamples,false);
   for(int mip=0;mip<hdLevels;mip++){
    const int n=hdMipLength(mip),cap=std::min(n/2-1,(hdSamples/2)>>mip);
    std::fill_n(out.begin(),n,HdBin{});
    // No DC: same convention as an AC-coupled oscillator; raw PCM is unchanged.
    const float scale=(float)n/hdSamples;
    for(int k=1;k<=cap;k++){out[k]=bins[k]*scale;out[n-k]=std::conj(out[k]);}
    hdFft(out.data(),n,true);
    float* dest=mips.data()+(size_t)f*hdMipStride()+hdMipOffset(mip);
    for(int i=0;i<n;i++)dest[i]=out[i].real();
   }
  }
 }
 float preview(float phase,float position)const noexcept {
  if(frames<1||!std::isfinite(phase)||!std::isfinite(position))return 0;
  float f=std::clamp(position,0.f,1.f)*(frames-1);int a=(int)f,b=std::min(a+1,frames-1);
  float x=interp(pcm.data()+(size_t)a*hdSamples,hdSamples,phase),y=interp(pcm.data()+(size_t)b*hdSamples,hdSamples,phase);
  return x+(y-x)*(f-a);
 }
 float sample(float phase,float position,float hz,float sr)const noexcept {
  if(frames<1||!std::isfinite(phase)||!std::isfinite(position)||!std::isfinite(hz)||!std::isfinite(sr)||hz<=0||sr<8000||hz>=sr*.48f)return 0;
  const float frequencyLimit=sr*.48f;
  int mip=0;
  while(mip<hdLevels-1&&hz>frequencyLimit/(float)((hdSamples/2)>>mip))++mip;
  const float framePos=std::clamp(position,0.f,1.f)*(frames-1);
  const int a=(int)framePos,b=std::min(a+1,frames-1);
  const float frameBlend=framePos-a;
  // Normalize phase and locate the two source frames exactly once per sample.
  const float cyclePhase=phase-std::floor(phase);
  const float* const frameA=mips.data()+(size_t)a*hdMipStride();
  const float* const frameB=mips.data()+(size_t)b*hdMipStride();
  auto atLevel=[&](int m)noexcept{
   const int len=hdMipLength(m);
   const float idx=cyclePhase*len;
   const int i=(int)idx,j=(i+1)&(len-1);
   const float blend=idx-i;
   const int offset=hdMipOffset(m);
   const float* const srcA=frameA+offset;
   const float x=srcA[i]+(srcA[j]-srcA[i])*blend;
   if(frameBlend<=0.f)return x;
   const float* const srcB=frameB+offset;
   const float y=srcB[i]+(srcB[j]-srcB[i])*blend;
   return x+(y-x)*frameBlend;
  };
  const float x=atLevel(mip);
  if(mip==hdLevels-1)return x;
  const float edge=frequencyLimit/(float)((hdSamples/2)>>mip);
  const float t0=std::clamp((hz-.8f*edge)/(.2f*edge),0.f,1.f);
  if(t0<=0.f)return x;
  const float t=t0*t0*(3.f-2.f*t0);
  return x+(atLevel(mip+1)-x)*t;
 }

};
// Single audio consumer; publishers serialized by writerMutex on non-audio
// threads. Producer owns back, consumer owns front, exchange owns middle.
// Consumer never allocates, locks, copies a table or changes a shared_ptr.
class HdExchange {
 std::array<std::shared_ptr<const HdTable>,3> slots{};
 std::shared_ptr<const HdTable> visible;
 mutable std::mutex writerMutex;
 std::atomic<unsigned> middle{1};
 unsigned back=2,front=0;
 static_assert(std::atomic<unsigned>::is_always_lock_free,"HD exchange requires lock-free index");
public:
 void publish(std::shared_ptr<const HdTable> next){
  std::lock_guard<std::mutex> lock(writerMutex);
  visible=next;slots[back]=std::move(next);
  back=middle.exchange(back|4u,std::memory_order_acq_rel)&3u;
 }
 std::shared_ptr<const HdTable> snapshot()const {
  std::lock_guard<std::mutex> lock(writerMutex);return visible;
 }
 const HdTable* consume()noexcept {
  if(middle.load(std::memory_order_acquire)&4u)front=middle.exchange(front,std::memory_order_acq_rel)&3u;
  return slots[front].get();
 }
};
} // namespace serenity
