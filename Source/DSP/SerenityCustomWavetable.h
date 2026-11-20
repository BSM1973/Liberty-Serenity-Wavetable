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
