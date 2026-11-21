#pragma once
// Liberty Music Software — custom wavetable store.
// Pure C++17; fixed 4 oscillators x 8 frames x 512 samples.
// Editing and loading happen outside the audio callback. Render never allocates.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace serenity {
constexpr int editableOscillators=4;
constexpr int editableFrames=8;
constexpr int editableSamples=512;
constexpr size_t editableTotal=(size_t)editableOscillators*editableFrames*editableSamples;
using ImportedWavetable=std::array<std::array<float,editableSamples>,editableFrames>;
// Offline cyclic resampling and interpolation along the source frame axis.
inline bool convertWavetable(const float* source,int count,int cycleLength,ImportedWavetable& out) noexcept {
 if(!source||count<2||count>2000000||cycleLength<2||cycleLength>count||count%cycleLength!=0)return false;
 for(int i=0;i<count;i++)if(!std::isfinite(source[i]))return false;
 int frames=count/cycleLength;constexpr double pi=3.14159265358979323846;
 auto sample=[&](int frame,int point){
  double pos=(double)point*cycleLength/editableSamples;int center=(int)std::floor(pos);
  auto at=[&](int n){int i=n%cycleLength;if(i<0)i+=cycleLength;return (double)source[frame*cycleLength+i];};
  if(cycleLength<=editableSamples){double frac=pos-center;return at(center)*(1-frac)+at(center+1)*frac;}
  double ratio=(double)editableSamples/cycleLength,fc=.475*ratio;
  int radius=std::min(512,std::max(16,(int)std::ceil(12./ratio)));
  double sum=0,weight=0;
  for(int j=-radius+1;j<=radius;j++){double x=pos-(center+j),z=2*pi*fc*x;
   double sinc=std::abs(z)<1e-12?1.:std::sin(z)/z,win=.5+.5*std::cos(pi*x/radius),k=2*fc*sinc*win;
   sum+=at(center+j)*k;weight+=k;}
  return std::abs(weight)>1e-12?sum/weight:0.;
 };
 for(int dst=0;dst<editableFrames;dst++){
  double f=(double)dst*(frames-1)/(editableFrames-1);int a=(int)f,b=std::min(a+1,frames-1);double blend=f-a;
  for(int i=0;i<editableSamples;i++){double v=sample(a,i)*(1-blend)+sample(b,i)*blend;out[dst][i]=(float)std::max(-1.,std::min(1.,v));}
 }
 return true;
}
struct EditableWavetable {
 std::array<float,editableTotal> values{};

 static size_t index(int oscillator,int frame,int sample) noexcept {
  return ((size_t)oscillator*editableFrames+(size_t)frame)*editableSamples+(size_t)sample;
 }
 float get(int oscillator,int frame,int sample) const noexcept {
  if(oscillator<0||oscillator>=editableOscillators||
     frame<0||frame>=editableFrames||sample<0||sample>=editableSamples) return 0.f;
  return values[index(oscillator,frame,sample)];
 }
 void put(int oscillator,int frame,int sample,float value) noexcept {
  if(oscillator<0||oscillator>=editableOscillators||
     frame<0||frame>=editableFrames||sample<0||sample>=editableSamples)return;
  values[index(oscillator,frame,sample)]=std::max(-1.f,std::min(1.f,value));
 }
 void resetSine() noexcept {
  for(int osc=0;osc<editableOscillators;osc++)
   for(int frame=0;frame<editableFrames;frame++)
    for(int point=0;point<editableSamples;point++)
     put(osc,frame,point,std::sin(6.2831853071795864769f*(float)point/editableSamples));
 }
 void normalize(int osc,int frame) noexcept {
  float peak=0.f;
  for(int i=0;i<editableSamples;i++)peak=std::max(peak,std::abs(get(osc,frame,i)));
  if(peak<0.000001f)return;
  for(int i=0;i<editableSamples;i++)put(osc,frame,i,get(osc,frame,i)/peak);
 }
 void removeDc(int osc,int frame) noexcept {
  float mean=0.f;
  for(int i=0;i<editableSamples;i++)mean+=get(osc,frame,i);
  mean/=editableSamples;
  for(int i=0;i<editableSamples;i++)put(osc,frame,i,get(osc,frame,i)-mean);
 }
 void copyFrame(int osc,int from,int to) noexcept {
  if(osc<0||osc>=editableOscillators||from<0||from>=editableFrames||to<0||to>=editableFrames)return;
  for(int i=0;i<editableSamples;i++)put(osc,to,i,get(osc,from,i));
 }
 float sample(int osc,float phase,float framePosition) const noexcept {
  if(osc<0||osc>=editableOscillators || !std::isfinite(phase) || !std::isfinite(framePosition))
   return 0.f;
  const float f=std::max(0.f,std::min(1.f,framePosition))*(editableFrames-1);
  const int frame0=(int)f,frame1=std::min(frame0+1,editableFrames-1);
  const float cross=f-frame0;
  const float p=(phase-std::floor(phase))*editableSamples;
  const int sample0=(int)p & (editableSamples-1),sample1=(sample0+1)&(editableSamples-1);
  const float fraction=p-std::floor(p);
  const float a=get(osc,frame0,sample0)*(1-fraction)+get(osc,frame0,sample1)*fraction;
  const float b=get(osc,frame1,sample0)*(1-fraction)+get(osc,frame1,sample1)*fraction;
  return a+(b-a)*cross;
 }
};
} // namespace serenity
