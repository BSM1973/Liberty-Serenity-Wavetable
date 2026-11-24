#pragma once
// Serenity WAVETABLE - Liberty Music Software
// Independent, allocation-free (in render) synthesis core.
// This is an actual playable foundation, not a claim that every planned engine is complete.
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <string>
#include <vector>
#include "SerenitySequencer.h"
#include "SerenityCustomWavetable.h"
#include "SerenityHdWavetable.h"
#include "SerenityBandlimitedWavetable.h"
#include "SerenityOscTransform.h"

namespace serenity {
constexpr float pi=3.14159265358979323846f;
constexpr int maxVoices=32, generators=4, maxUnison=16, waveCount=16;
constexpr int frames=8, tableSize=512, mipCount=8;
inline float clamp(float x,float a,float b) noexcept { return std::max(a,std::min(x,b)); }
inline float mix(float a,float b,float t) noexcept {return a+(b-a)*t;}

struct EnvelopeSettings { float delay=0,attack=.004f,hold=0,decay=.45f,sustain=.7f,release=.35f,curve=0,velocity=0; bool loop=false; };
struct LfoSettings { float rate=5,phase=0; int shape=0; bool unipolar=false; bool sync=false;int division=5;float smooth=0,fade=0; };
inline float envelopeCurve(float x,float curve) noexcept {x=clamp(x,0.f,1.f);return curve>=0.f?mix(x,x*x,curve):mix(x,1.f-(1.f-x)*(1.f-x),-curve);}
inline constexpr std::array<float,12> lfoBeats{{4.f,2.f,1.5f,1.f,2.f/3.f,.5f,.375f,1.f/3.f,.25f,1.f/6.f,.125f,.0625f}};
inline float lfoFrequency(const LfoSettings& f,float rate,float bpm) noexcept {return f.sync?clamp(bpm,20.f,400.f)/(60.f*lfoBeats[(size_t)std::max(0,std::min(11,f.division))]):rate;}
inline float lfoWave(float phase,int shape) noexcept {
 phase-=std::floor(phase);
 switch(shape){case 1:return 1.f-4.f*std::abs(phase-.5f);case 2:return 2.f*phase-1.f;case 3:return phase<.5f?1.f:-1.f;case 4:return 1.f-2.f*phase;default:return std::sin(2*pi*phase);}
}
struct Params {
 float master=0.65f, cutoff=0.85f, resonance=0.14f, position=0.50f;
 float cutoff2=0.55f,resonance2=.2f,filter2Mix=1.f;
 bool filter2On=false, parallelFilters=false,filter1Enabled=true;
 int filter1Type=0,filter2Type=1;
 float filter1Drive=0,filter2Drive=0,filter1Env=0,filter2Env=0,filter1Key=0,filter2Key=0,filter1Vel=0,filter2Vel=0,filter1Mix=1;
 int reverbType=1;float reverbDamping=.5f,reverbSize=1,reverbPreDelay=.015f,reverbLowCut=100,reverbWidth=1;
 bool envLoop=false;
 float attack=0.004f, decay=0.45f, sustain=0.70f, release=0.35f;
 float detune=0.20f, spread=0.70f, delay=0.10f, reverb=0.15f, drive=0.0f;
 float lfoRate=5.0f, lfoDepth=0.0f, wavemorph=0.0f;
 float oscLevel[4]={0.70f,0.0f,0.0f,0.0f};
 float oscPitch[4]={0,0,0,-12};
 float oscOctave[4]{},oscSemi[4]{},oscFine[4]{},oscPan[4]{};
 float oscWarp[4]{},oscShape[4]{};
 // Unison and phase controls default to neutral for backward compatibility.
 float oscBlend[4]{},oscPhase[4]{},oscRandPhase[4]{};
 float fmBA=0,fmCA=0,fmCB=0,ringAmount=0;
 int fmMode=0,ringMode=0;
 float envDelay=0,envHold=0,envCurve=0,envVelocity=0;
 std::array<EnvelopeSettings,5> modEnv{};
 std::array<LfoSettings,8> lfos{{{5,0,0,false},{5.9f,0,0,false},{6.8f,0,0,false},{7.7f,0,0,false},{8.6f,0,0,false},{9.5f,0,0,false},{10.4f,0,0,false},{11.3f,0,0,false}}};
 float delayTime=.38f,delayFeedback=.35f,delayTone=1,reverbFeedback=.76f,masterWidth=1;
 float tremRate=3,tremDepth=0,fxRingFreq=120,fxRingMix=0;
 int tremMode=0;
 int modFxType=0;float modFxRate=.6f,modFxDepth=.5f,modFxFeedback=0,modFxMix=0;
 float eqLowGain=0,eqLowFreq=120,eqMidGain=0,eqMidFreq=1000,eqMidQ=1,eqHighGain=0,eqHighFreq=6000;
 float compThreshold=-18,compRatio=4,compAttack=.015f,compRelease=.15f,compMakeup=0,compMix=0;
 EnvelopeSettings amplitude() const noexcept {return {envDelay,attack,envHold,decay,sustain,release,envCurve,envVelocity,envLoop};}
 int oscWarpMode[4]{},oscShapeMode[4]{};
 int wave[4]={1,0,3,0};
 int unison[4]={1,1,1,1};
 int polyphony=16;
};
struct Preset {
 std::string name,category,author="Liberty Music Software";
 Params p;
};

// Anti-aliased additive bank, with frequency-dependent harmonic caps (8 mip levels).
// Precomputed outside the real-time callback; phase and frame interpolation at render time.
class WavetableBank {
 std::vector<float> tables;
 size_t offset(int wave,int mip,int frame,int sample) const noexcept {
  return ((((size_t)wave*mipCount+mip)*frames+frame)*tableSize+sample);
 }
public:
 WavetableBank(): tables((size_t)waveCount*mipCount*frames*tableSize,0.0f) {
  for(int w=0;w<waveCount;++w) for(int mip=0;mip<mipCount;++mip)
   for(int frame=0;frame<frames;++frame) {
    int maxH=std::min(256,1<<(mipCount-1-mip));
    float brightness=(float)(frame+1)/frames;
    float mx=0;
    for(int n=0;n<tableSize;n++) {
     float x=2*pi*n/tableSize, sum=0;
     for(int h=1;h<=maxH;h++) {
      float amp=0, phase=0;
      switch(w) {
       case 0: amp=(h==1?1.0f:0.0f);break; // sine
       case 1: amp=1.0f/h;phase=(h&1)?0:pi;break; // saw
       case 2: amp=(h&1)?1.0f/(h*h):0;phase=(h%4==1)?0:pi;break; // triangle
       case 3: amp=(h&1)?1.0f/h:0;break; // square
       case 4: amp=(h%3==1)?1.0f/h:0;break; // hollow
       case 5: amp=(h%2==0)?1.0f/h:0;break; // even harmonics
       case 6: amp=std::exp(-0.22f*h);break; // soft
       case 7: amp=std::exp(-0.05f*h);break; // bright
       case 8: amp=std::sin(0.31f*h)*std::exp(-.08f*h);break;
       case 9: amp=std::cos(0.52f*h)*std::exp(-.07f*h);break;
       case 10: amp=(h==2||h==3||h==5||h==8)?1.0f/h:0;break;
       case 11: amp=std::exp(-0.03f*h)*std::sin(h*1.91f);break;
       case 12: amp=(h%4==1)?1.0f/std::sqrt((float)h):0;break;
       case 13: amp=std::exp(-0.09f*h)*std::cos(h*0.29f);break;
       case 14: amp=(h%5==0)?1.0f/h:0;break;
       default: amp=std::sin(h*.17f)/h;
      }
      amp*=std::pow(brightness,0.18f*h); // evolving 8-frame tables
      sum+=amp*std::sin(x*h+phase);
     }
     tables[offset(w,mip,frame,n)]=sum;
     mx=std::max(mx,std::abs(sum));
    }
    if(mx>0.000001f) for(int n=0;n<tableSize;n++) tables[offset(w,mip,frame,n)]/=mx;
   }
 }
 float previewPoint(int wave,int frame,int sampleIndex) const noexcept {
  if(wave<0||wave>=waveCount||frame<0||frame>=frames||
     sampleIndex<0||sampleIndex>=tableSize)return 0.f;
  return tables[offset(wave,0,frame,sampleIndex)];
 }
 float sample(int wave,float phase,float framePosition,float hz,float sr) const noexcept {
  if(!std::isfinite(phase)||!std::isfinite(hz)||sr<=0) return 0;
  wave=std::max(0,std::min(waveCount-1,wave));
  // Higher fundamental -> fewer harmonics. mip 0=128 harmonic, mip 7=1.
  int harmonicLimit=std::max(1,(int)(sr*.49f/std::max(hz,1.0f)));
  int mip=0;
  while(mip<mipCount-1 && (1<<(mipCount-1-mip))>harmonicLimit) ++mip;
  float f=clamp(framePosition,0,1)*(frames-1);
  int a=(int)f,b=std::min(a+1,frames-1);
  float ft=f-a;
  float pp=(phase-std::floor(phase))*tableSize;
  int i=(int)pp & (tableSize-1),j=(i+1)&(tableSize-1);
  float t=pp-std::floor(pp);
  return mix(mix(tables[offset(wave,mip,a,i)],tables[offset(wave,mip,a,j)],t),
             mix(tables[offset(wave,mip,b,i)],tables[offset(wave,mip,b,j)],t),ft);
 }
};

struct Envelope {
 enum Stage { idle, delay, attack, hold, decay, sustain, release } stage=idle;
 float value=0,elapsed=0,origin=0,releaseSeconds=.35f;
 void on() noexcept {stage=delay;elapsed=0;origin=value;if(value<.0001f)origin=value=0;}
 void off(float seconds,float) noexcept {stage=release;origin=value;elapsed=0;releaseSeconds=std::max(.001f,seconds);}
 float next(const EnvelopeSettings& p,float sr) noexcept {
  const float dt=1.f/sr;
  if(stage==delay){if(elapsed<p.delay){elapsed+=dt;value=0;origin=0;return 0;}stage=attack;elapsed=0;}
  if(stage==hold){if(elapsed<p.hold){elapsed+=dt;return 1;}stage=decay;elapsed=0;}
  switch(stage){
   case idle:return 0;
   case delay:case hold:break;
   case attack:{elapsed+=dt;float q=clamp(elapsed/std::max(.001f,p.attack),0.f,1.f);value=mix(origin,1.f,envelopeCurve(q,p.curve));if(q>=1.f){value=1;stage=hold;elapsed=0;}break;}
   case decay:{elapsed+=dt;float q=clamp(elapsed/std::max(.001f,p.decay),0.f,1.f);value=mix(1.f,p.sustain,envelopeCurve(q,p.curve));if(q>=1.f){value=p.sustain;if(p.loop){origin=value;stage=attack;elapsed=0;}else stage=sustain;}break;}
   case sustain:value=p.sustain;break;
   case release:{elapsed+=dt;float q=clamp(elapsed/releaseSeconds,0.f,1.f);value=origin*(1.f-envelopeCurve(q,p.curve));if(q>=1.f){value=0;stage=idle;}break;}
  }
  return clamp(value,0.f,1.f);
 }
};
// RBJ equalizer, transposed direct form II; coefficients calculated outside the sample loop.
struct EqCoefficients {float b0=1,b1=0,b2=0,a1=0,a2=0;};
inline EqCoefficients eqCoefficients(int type,float freq,float db,float q,float sr) noexcept {
 if(std::abs(db)<.00001f)return {};
 const float A=std::pow(10.f,db/40.f),omega=2*pi*clamp(freq,10.f,sr*.45f)/sr;
 const float c=std::cos(omega),sin=std::sin(omega);
 float b0,b1,b2,a0,a1,a2;
 if(type==1){float alpha=sin/(2.f*clamp(q,.3f,8.f));b0=1+alpha*A;b1=-2*c;b2=1-alpha*A;a0=1+alpha/A;a1=-2*c;a2=1-alpha/A;}
 else {const float alpha=sin*.70710678f,beta=2.f*std::sqrt(A)*alpha;
  if(type==0){b0=A*((A+1)-(A-1)*c+beta);b1=2*A*((A-1)-(A+1)*c);b2=A*((A+1)-(A-1)*c-beta);a0=(A+1)+(A-1)*c+beta;a1=-2*((A-1)+(A+1)*c);a2=(A+1)+(A-1)*c-beta;}
  else{b0=A*((A+1)+(A-1)*c+beta);b1=-2*A*((A-1)+(A+1)*c);b2=A*((A+1)+(A-1)*c-beta);a0=(A+1)-(A-1)*c+beta;a1=2*((A-1)-(A+1)*c);a2=(A+1)-(A-1)*c-beta;}
 }
 return {b0/a0,b1/a0,b2/a0,a1/a0,a2/a0};
}
inline float eqMagnitude(const EqCoefficients& k,float freq,float sr) noexcept {
 const float w=2*pi*freq/sr,c=std::cos(w),c2=std::cos(2*w),si=std::sin(w),s2=std::sin(2*w);
 const float nr=k.b0+k.b1*c+k.b2*c2,ni=k.b1*si+k.b2*s2,dr=1+k.a1*c+k.a2*c2,di=k.a1*si+k.a2*s2;
 return std::sqrt((nr*nr+ni*ni)/std::max(1.e-16f,dr*dr+di*di));
}
struct EqState {float z1=0,z2=0;float tick(float x,const EqCoefficients& c) noexcept {float y=c.b0*x+z1;z1=c.b1*x-c.a1*y+z2;z2=c.b2*x-c.a2*y;return y;}};
inline float compressorGainDb(float level,float threshold,float ratio) noexcept {return -std::max(0.f,level-threshold)*(1.f-1.f/std::max(1.f,ratio));}
// Topology-preserving state variable filters; each mode uses the same resonant core.
inline constexpr const char* filterNames[]={"LP 24","LP 12","HP 12","HP 24","BP 12","BP 24","Notch 12","Notch 24","All Pass","Peak","Low Shelf","High Shelf","Band Shelf","LP + BP","HP + BP"};
template<class T> inline T filterOutput(int mode,T x,T lo,T bp,T hi,float k) noexcept {
 switch(mode){case 2:case 3:return hi;case 4:case 5:return bp*k;
 case 6:case 7:return lo+hi;case 8:return lo+hi-bp*k;case 9:return lo-hi;
 case 10:return x+lo*.75f;case 11:return x+hi*.75f;case 12:return x+bp*k*.75f;
 case 13:return lo+bp*k*.5f;case 14:return hi+bp*k*.5f;default:return lo;}
}
inline bool doubleFilter(int mode) noexcept{return mode==0||mode==3||mode==5||mode==7;}
struct Svf {float ic1=0,ic2=0;
 float tick(float x,float g,float k,int mode) noexcept {
  float a=1.f/(1.f+g*(g+k)),v1=a*(ic1+g*(x-ic2)),v2=ic2+g*v1;
  ic1=2.f*v1-ic1;ic2=2.f*v2-ic2;
  return filterOutput(mode,x,v2,v1,x-k*v1-v2,k);
 }
};
inline float filterMag(int mode,float frequency,float cutoff,float resonance,float sr) noexcept{
 using Z=std::complex<float>;
 float r=std::tan(pi*clamp(frequency,10.f,sr*.45f)/sr)/std::tan(pi*clamp(cutoff,10.f,sr*.45f)/sr);
 float k=2.f-1.85f*clamp(resonance,0.f,1.f);
 Z z(0,r),den=1.f+k*z+z*z;
 Z h=filterOutput(mode,Z(1,0),Z(1,0)/den,z/den,z*z/den,k);
 float m=std::abs(h);return doubleFilter(mode)?m*m:m;
}
struct FilterChannel {Svf a,b;
 // Identical TPT transfer; g and k can be shared between both stereo
 // channels, and across all notes when the cutoff has no per-voice routing.
 float runPrepared(float x,float g,float k,float drive,int mode) noexcept {
  if(drive>0){float gain=1.f+drive*5.f;x=std::tanh(x*gain)/gain;}
  float y=a.tick(x,g,k,mode);return doubleFilter(mode)?b.tick(y,g,k,mode):y;
 }
 float run(float x,float cutoff,float resonance,float drive,int mode,float sr) noexcept {
  const float g=std::tan(pi*clamp(cutoff,10.f,sr*.45f)/sr),k=2.f-1.85f*clamp(resonance,0.f,1.f);
  return runPrepared(x,g,k,drive,mode);
 }
};
// Eight-line orthogonal feedback delay network, stereo decorrelated taps.
struct StereoReverb {
 std::array<std::vector<float>,8> lines;std::array<float,8> damp{};
 std::array<std::vector<float>,2> pre;std::array<float,2> low{};
 size_t pos=0;float sr=48000;
 void prepare(float rate){sr=rate;for(auto& v:lines)v.assign((size_t)(sr*.19f)+8,0);for(auto& v:pre)v.assign((size_t)(sr*.21f)+8,0);reset();}
 void reset() noexcept{pos=0;damp={};low={};for(auto& v:lines)std::fill(v.begin(),v.end(),0.f);for(auto& v:pre)std::fill(v.begin(),v.end(),0.f);}
 void tick(float& l,float& r,const Params& p) noexcept{
  if(lines[0].empty())return;
  const float x[]={l,r};float in[2]{};
  float hp=1.f-std::exp(-2*pi*clamp(p.reverbLowCut,20.f,1000.f)/sr);
  for(size_t j=0;j<2;j++){low[j]+=(x[j]-low[j])*hp;auto& buf=pre[j];size_t n=buf.size(),w=pos%n;
   buf[w]=x[j]-low[j];size_t d=(size_t)(sr*clamp(p.reverbPreDelay,0.f,.2f));in[j]=buf[(w+n-d%n)%n];}
  constexpr float times[]={.0297f,.0371f,.0411f,.0437f,.0531f,.0617f,.0719f,.0797f};
  constexpr float sizes[]={.65f,1.f,1.35f,.8f};
  const float size=clamp(p.reverbSize,.5f,1.5f)*sizes[std::max(0,std::min(3,p.reverbType))];
  float sum=0,wetL=0,wetR=0;
  for(size_t j=0;j<8;j++){auto& buf=lines[j];size_t n=buf.size(),d=(size_t)(sr*times[j]*size);
   float y=buf[(pos%n+n-d%n)%n];damp[j]+=(y-damp[j])*(1.f-.94f*clamp(p.reverbDamping,0.f,1.f));sum+=damp[j];
   wetL+=y*((j&1)?-1.f:1.f);wetR+=y*((j&2)?-1.f:1.f);}
  const float fb=clamp(p.reverbFeedback,0.f,.94f);
  for(size_t j=0;j<8;j++){auto& buf=lines[j];buf[pos%buf.size()]=.22f*(in[0]*((j&1)?-1.f:1.f)+in[1]*((j&2)?-1.f:1.f))+fb*(damp[j]-.25f*sum);}
  ++pos;float mid=.5f*(wetL+wetR),side=.5f*(wetL-wetR)*p.reverbWidth;
  l+=(mid+side)*p.reverb*.28f;r+=(mid-side)*p.reverb*.28f;
 }
};
struct Voice {
 int note=-1;bool active=false;
 // Immutable for the lifetime of a MIDI note: no std::pow per active sample.
 float baseFrequency=0.f;
 float velocity=0,stepGate=0,filterL=0,filterR=0,filter2L=0,filter2R=0;
 float filter3L=0,filter3R=0,filter4L=0,filter4R=0;
 std::array<std::array<FilterChannel,2>,2> filters{};
 float lfoAge=0;
 Envelope amp;
 std::array<Envelope,6> envelopes{};
 std::array<std::array<float,maxUnison>,generators> phase{};
 std::array<std::array<float,maxUnison>,generators> phaseJitter{};
 std::array<float,generators> modSignal{};
 uint64_t age=0;
 void start(int n,float v,uint64_t when) noexcept {
  note=n;velocity=v;active=true;age=when;
  baseFrequency=440.f*std::pow(2.f,(n-69)/12.f);
  amp={};amp.on();
  for(auto &env:envelopes){env={};env.on();}
  filters={};lfoAge=0;
  filterL=filterR=filter2L=filter2R=0;filter3L=filter3R=filter4L=filter4R=0;
  for(auto &array:phase) array.fill(0);
  modSignal.fill(0.f);
  // Stable random phase per note, not per sample (no allocation in render).
  for(int gen=0;gen<generators;gen++)for(int u=0;u<maxUnison;u++){
   uint32_t h=(uint32_t)n*0x9e3779b9u ^ (uint32_t)(when^(when>>32))*0x85ebca6bu ^
              (uint32_t)(gen+1)*0xc2b2ae35u ^ (uint32_t)(u+1)*0x27d4eb2fu;
   h^=h>>16;h*=0x7feb352du;h^=h>>15;h*=0x846ca68bu;h^=h>>16;
   phaseJitter[(size_t)gen][(size_t)u]=(float)(h>>8)/16777216.f;
  }
 }
};

struct ModRoute {
 int source=-1, destination=-1;
 float amount=0;
 // source: 0-5 envelopes; 6-13 LFO; 14 velocity, 15 key, 16 mod wheel
 // destination: 0 pitch, 1 wavetable, 2 cutoff, 3 amp, 4 pan
};
// The modulation matrix distinguishes sample-varying envelope/LFO pitch
// from note-constant key, velocity and current block mod-wheel offsets.
inline bool hasSampleVaryingPitchRoute(const std::array<ModRoute,64>& routes) noexcept {
 for(const auto& route:routes)
  if(route.source>=0&&route.source<14&&route.destination==0&&route.amount!=0.f)
   return true;
 return false;
}
inline float constantPitchForVoice(const std::array<ModRoute,64>& routes,
                                   const Voice& voice,float wheel) noexcept {
 float result=0.f;
 for(const auto& route:routes){
  if(route.destination!=0||route.amount==0.f)continue;
  float src=0.f;
  if(route.source==14)src=voice.velocity;
  else if(route.source==15)src=(voice.note-60)/60.f;
  else if(route.source==16)src=wheel;
  const float amount=src*route.amount;
  result+=amount*12.f;
 }
 return result;
}
// The eligibility check is block-constant, but it must inspect the complete
// routing and sequencer configuration, not the instantaneous modulation value.
// Otherwise computing hundreds of unused spectral plans each block penalizes
// the dynamic-FM and pitch-envelope workflows.
inline bool hasRoutedPitchModulation(const std::array<ModRoute,64>& routes) noexcept {
 for(const auto& route:routes)
  if(route.source>=0 && route.destination==0 && route.amount!=0.f)return true;
 return false;
}
inline bool sequencerMayTranspose(const SequencePattern& pattern) noexcept {
 if(pattern.mode==0)return false;
 const int length=std::clamp(pattern.length,1,sequenceMaxSteps);
 for(int i=0;i<length;i++)
  if(pattern.steps[(size_t)i].pitch!=0)return true;
 return false;
}
inline bool oscillatorHasFrequencyFm(const Params& p,int gen) noexcept {
 if(p.fmMode!=1)return false;
 if(gen==0)return p.fmBA!=0.f||p.fmCA!=0.f;
 if(gen==1)return p.fmCB!=0.f;
 return false;
}

class Engine {
 WavetableBank bank;
 EditableWavetable customTable{};
 BandlimitedCustomWavetable customBandlimited{};
 std::array<const HdTable*,generators> hdTables{};
 std::array<Voice,maxVoices> voices{};
 // Block-local note-frequency and spectral plans stay on the engine object,
 // rather than using ~40 KB of the host audio callback thread's stack.
 struct VoiceOscPlan {
  float frequency=0.f,phaseIncrement=0.f;
  HdTable::PreparedSpectral spectral{};
 };
 std::array<std::array<std::array<VoiceOscPlan,maxUnison>,generators>,maxVoices> pitchPlans{};
 // Persistent plans survive morph-only changes across blocks. Parameters
 // determining the exact pitch, sample-rate and HD state key invalidation.
 struct PitchPlanKey {
  uint64_t voiceAge=0;
  float semis=0.f,detune=0.f,mod=0.f,sampleRate=0.f;
  int sequencePitch=0,unison=0;
  bool hasHD=false,valid=false;
 };
 std::array<std::array<PitchPlanKey,generators>,maxVoices> pitchKeys{};


 std::array<ModRoute,64> routes{};
 float sr=48000,modWheel=0;
 std::array<float,8> lfoPhases{},lfoSmooth{};
 float hostTempo=120,modFxPhase=0,compGain=1,compAtk=0,compRel=0,compMakeupGain=1;
 std::array<EqCoefficients,3> eqCoeffs{};
 std::array<std::array<EqState,3>,2> eqState{};
 std::array<std::array<float,6>,2> phaserState{};
 std::array<float,2> phaserFeedback{};
 std::array<std::vector<float>,2> modDelay;
 size_t modIndex=0;
 void modulationFx(float& l,float& r) noexcept {
  if(modDelay[0].empty())return;
  const float wet=clamp(p.modFxMix,0.f,1.f),depth=clamp(p.modFxDepth,0.f,1.f),fb=clamp(p.modFxFeedback,-.85f,.85f);
  float* ch[]={&l,&r};
  for(size_t k=0;k<2;k++){
   const float x=*ch[k],lf=.5f+.5f*std::sin(2*pi*(modFxPhase+(k==0?0.f:.25f)));
   float y=x;auto& buf=modDelay[k];const size_t len=buf.size();
   if(p.modFxType==2){const float f=200.f+depth*lf*3200.f;const float g=std::tan(pi*clamp(f,20.f,sr*.4f)/sr),coef=(1.f-g)/(1.f+g);
    y=x+fb*phaserFeedback[k];for(float& z:phaserState[k]){float v=-coef*y+z;z=y+coef*v;y=v;}phaserFeedback[k]=y;
   }else{float off=sr*(p.modFxType==0?(.012f+.014f*depth*lf):(.0007f+.007f*depth*lf));size_t whole=(size_t)off,rd=(modIndex+len-whole%len)%len;
    y=mix(buf[rd],buf[(rd+len-1)%len],off-(float)whole);}
   buf[modIndex]=x+(p.modFxType==2?0.f:fb*y);*ch[k]=mix(x,.5f*(x+y),wet);
  }
  modIndex=(modIndex+1)%modDelay[0].size();modFxPhase+=p.modFxRate/sr;modFxPhase-=std::floor(modFxPhase);
 }
 void equalizeCompress(float& l,float& r) noexcept {
  float* ch[]={&l,&r};for(size_t k=0;k<2;k++)for(size_t j=0;j<3;j++)*ch[k]=eqState[k][j].tick(*ch[k],eqCoeffs[j]);
  if(p.compMix>0.f){const float peak=std::max(std::abs(l),std::abs(r));float db=20.f*std::log10(std::max(peak,1.e-9f));float target=std::pow(10.f,compressorGainDb(db,p.compThreshold,p.compRatio)/20.f);
   compGain=target+(compGain-target)*(target<compGain?compAtk:compRel);float g=mix(1.f,compGain*compMakeupGain,p.compMix);l*=g;r*=g;}
  else compGain=1.f;
 }
 float tremPhase=0,ringPhase=0,delaySmoothed=.38f,delayToneL=0,delayToneR=0;
 StepClock sequence;
 int lastSequenceIndex=-1;
 uint64_t noteCounter=0;
 std::vector<float> delayL,delayR;
 StereoReverb reverbFx;
 size_t delayIndex=0, reverbIndex=0;
 Params p;
 int activeCount=0;
 float lowpass(float x,float &state,float freq) noexcept {
  float coeff=1.0f-std::exp(-2.0f*pi*clamp(freq,10.0f,sr*.45f)/sr);
  state+=(x-state)*coeff;return state;
 }
 static float noteHz(int note) noexcept {return 440.f*std::pow(2.f,(note-69)/12.f);}
public:
 Engine(){customTable.resetSine();}
 void setCustomSample(int oscillator,int frame,int sample,float value) noexcept {
  customTable.put(oscillator,frame,sample,value);
 }
 float customSample(int oscillator,int frame,int sample) const noexcept {
  return customTable.get(oscillator,frame,sample);
 }
 // Factory bank is immutable after construction; this read is safe from the UI.
 float previewFactoryPoint(int wave,int frame,int index) const noexcept {
  return bank.previewPoint(wave,frame,index);
 }
 // Replaces one immutable fixed-size frame snapshot at a block boundary.
 // FFT and allocation are deliberately absent from this API and render().
 void setHdTable(int osc,const HdTable* table) noexcept {if(osc>=0&&osc<generators)hdTables[(size_t)osc]=table;}
 void setCustomMipFrame(int oscillator,int frame,const CustomMipFrame& mips) noexcept {
  customBandlimited.setFrame(oscillator,frame,mips);
 }
 void prepare(double sampleRate) {
  sr=(float)std::max(8000.0,sampleRate);
  sequence.prepare(sampleRate);
  delayL.assign((size_t)(sr*2.0f)+1,0);delayR=delayL;
  reverbFx.prepare(sr);
  delayIndex=reverbIndex=0;
  for(auto& buf:modDelay)buf.assign((size_t)(sr*.04f)+4,0.f);
  reset();setParams(p);
 }
 void reset() noexcept {for(auto &v:voices)v=Voice{};reverbFx.reset();std::fill(delayL.begin(),delayL.end(),0.f);std::fill(delayR.begin(),delayR.end(),0.f);delayIndex=0;lfoPhases.fill(0.f);lfoSmooth.fill(0.f);modFxPhase=0;modIndex=0;eqState={};phaserState={};phaserFeedback={};compGain=1;for(auto& buf:modDelay)std::fill(buf.begin(),buf.end(),0.f);tremPhase=ringPhase=0;delayToneL=delayToneR=0;delaySmoothed=p.delayTime;activeCount=0;sequence.reset();lastSequenceIndex=-1;}
 void setParams(const Params &next) noexcept {p=next;p.polyphony=std::max(1,std::min(p.polyphony,maxVoices));
  eqCoeffs={eqCoefficients(0,p.eqLowFreq,p.eqLowGain,1.f,sr),eqCoefficients(1,p.eqMidFreq,p.eqMidGain,p.eqMidQ,sr),eqCoefficients(2,p.eqHighFreq,p.eqHighGain,1.f,sr)};
  compAtk=std::exp(-1.f/(sr*std::max(.001f,p.compAttack)));compRel=std::exp(-1.f/(sr*std::max(.02f,p.compRelease)));compMakeupGain=std::pow(10.f,p.compMakeup/20.f);
 }
 void setHostTempo(float bpm) noexcept {hostTempo=std::isfinite(bpm)?clamp(bpm,20.f,400.f):120.f;}
 void setModWheel(float value) noexcept {modWheel=clamp(value,0,1);}
 void setSequence(const SequencePattern& pattern) noexcept {
  if(pattern.mode==0)lastSequenceIndex=-1;
  sequence.setPattern(pattern);
 }
 int sequenceStep() const noexcept { return sequence.currentStep(); }
 const Params &params() const noexcept {return p;}
 ModRoute &route(size_t i) noexcept {return routes[std::min(i,routes.size()-1)];}
 int numVoices() const noexcept {return activeCount;}
 void noteOn(int note,float vel) noexcept {
  if(note<0||note>127||vel<=0) return;
  Voice *pick=nullptr;
  for(int k=0;k<p.polyphony;k++) if(!voices[(size_t)k].active){pick=&voices[(size_t)k];break;}
  if(!pick){pick=&voices[0];for(int k=1;k<p.polyphony;k++) if(voices[(size_t)k].age<pick->age)pick=&voices[(size_t)k];}
  if(activeCount==0) sequence.reset();
  pick->start(note,clamp(vel,0,1),++noteCounter);
 }
 void noteOff(int note) noexcept {for(auto &v:voices) if(v.active&&v.note==note&&v.amp.stage!=Envelope::release){
    v.amp.off(p.release,sr);for(size_t k=0;k<v.envelopes.size();k++)v.envelopes[k].off(k==0?p.release:p.modEnv[k-1].release,sr);
  }}
 void allNotesOff() noexcept {for(auto &v:voices) if(v.active){v.amp.off(.01f,sr);for(auto &env:v.envelopes)env.off(.01f,sr);}}
 void render(float *outL,float *outR,int count) noexcept {
  if(!outL||!outR||count<=0)return;
  // p is block-constant. Cache all unison factors, amplitude normalization
  // and static parameters once per render call, not per sample or voice.
  struct UnisonState {
   int count=1;
   float start=1.f,step=1.f,amp=0.f,relativeStep=0.f;
   float phase=0.f,random=0.f,blend=0.f;
   float baseSemis=0.f;
   std::array<float,maxUnison> pitch{};
   std::array<float,maxUnison> blendGain{};
   std::array<float,maxUnison> relative{};
   std::array<float,maxUnison> pan{};
  };
  std::array<UnisonState,generators> unisonStates{};
  for(int gen=0;gen<generators;gen++){
   auto& state=unisonStates[(size_t)gen];
   state.count=std::max(1,std::min(maxUnison,p.unison[gen]));
   const float detune=state.count>1?p.detune*.19f/12.f:0.f;
   state.start=state.count>1?std::exp2(-detune):1.f;
   state.step=state.count>1?std::exp2(2.f*detune/(state.count-1)):1.f;
   state.amp=clamp(p.oscLevel[gen],0.f,1.f)/std::sqrt((float)state.count);
   state.relativeStep=state.count>1?2.f/(state.count-1):0.f;
   state.phase=clamp(p.oscPhase[gen],0.f,1.f);
   state.random=clamp(p.oscRandPhase[gen],0.f,1.f);
   state.blend=.65f*clamp(p.oscBlend[gen],0.f,1.f);
   state.baseSemis=p.oscPitch[gen]+p.oscOctave[gen]*12.f
      +p.oscSemi[gen]+p.oscFine[gen]*.01f;
   float pitch=state.start;
   for(int u=0;u<state.count;u++){
    // Exact same recurrence and order as the v1.2.4 per-sample loop.
    state.pitch[(size_t)u]=pitch;
    pitch*=state.step;
    const float relative=state.count==1?0.f:(state.relativeStep*u-1.f);
    state.relative[(size_t)u]=relative;
    state.blendGain[(size_t)u]=1.f-state.blend*std::abs(relative);
    state.pan[(size_t)u]=clamp(relative*p.spread+p.oscPan[gen],-1.f,1.f);
   }
  }
  // A cached frequency is immutable for a single note, oscillator,
  // modulation offset and sequence step. Envelope/LFO pitch routes remain
  // sample-dynamic; constant key/velocity/wheel routes can be prepared.
  const bool varyingPitch=hasSampleVaryingPitchRoute(routes);
  const auto& pattern=sequence.getPattern();
  // Preserve insertion order of the 64 modulation slots, but do not visit
  // unused slots 32 times per sample. Modulation math and order are unchanged.
  std::array<const ModRoute*,64> activeRoutes{};
  size_t activeRouteCount=0;
  for(const auto& route:routes)
   if(route.source>=0&&route.destination>=0&&route.amount!=0.f)
    activeRoutes[activeRouteCount++]=&route;
  // A pitch LFO without note-specific fade has the same signal for all notes.
  // It is safe to share the exponent across 32 voices; LFOs with fade or
  // envelope/key/velocity pitch routes still take the original per-voice path.
  bool sharedLfoPitch=false,sharedPitchEligible=true;
  for(size_t ri=0;ri<activeRouteCount;ri++){
   const auto& route=*activeRoutes[ri];
   if(route.destination!=0)continue;
   if(route.source>=6&&route.source<14){
    const size_t index=(size_t)(route.source-6);
    if(p.lfos[index].fade>0.f)sharedPitchEligible=false;
    else sharedLfoPitch=true;
   }else if(route.source!=16)sharedPitchEligible=false;
  }
  sharedLfoPitch &= sharedPitchEligible;
  const int currentStep=sequence.currentStep();
  const int cachedSequencePitch=currentStep>=0 ?
    std::clamp(pattern.steps[(size_t)currentStep].pitch,-24,24):0;
  std::array<bool,generators> canCachePitch{};
  bool anyCachedGenerator=false;
  for(int gen=0;gen<generators;gen++){
   canCachePitch[(size_t)gen]=!varyingPitch &&
     !oscillatorHasFrequencyFm(p,gen) && p.oscLevel[gen]>=.00001f;
   anyCachedGenerator|=canCachePitch[(size_t)gen];
  }
  // Profiling v1.2.9 build #140 showed that throttling plan rebuilds
  // forces too many expensive dynamic table reads through the next blocks.
  // Complete the ready plans at block start instead: dense unison ×16
  // sequences benefit from the fast spectral path throughout the callback.
  std::array<std::array<bool,generators>,maxVoices> readyPlans{};
  if(anyCachedGenerator){
   for(size_t vi=0;vi<voices.size();++vi){
    const auto& voice=voices[vi];if(!voice.active)continue;
    const float voiceMod=constantPitchForVoice(routes,voice,modWheel);
    for(int gen=0;gen<generators;gen++){
     if(!canCachePitch[(size_t)gen])continue;
     const auto& us=unisonStates[(size_t)gen];
     const HdTable* const hd=p.wave[gen]==waveCount?hdTables[(size_t)gen]:nullptr;
     auto& key=pitchKeys[vi][(size_t)gen];
     const bool same=key.valid && key.voiceAge==voice.age &&
       key.semis==us.baseSemis && key.detune==p.detune &&
       key.mod==voiceMod && key.sampleRate==sr &&
       key.sequencePitch==cachedSequencePitch &&
       key.unison==us.count && key.hasHD==(hd!=nullptr);
     if(same){readyPlans[vi][(size_t)gen]=true;continue;}
     const float staticPitch=std::exp2(
       (us.baseSemis+voiceMod+(float)cachedSequencePitch)/12.f);
     const float base=voice.baseFrequency*staticPitch;
     for(int u=0;u<us.count;u++){
      const float freq=clamp(base*us.pitch[(size_t)u],1.f,sr*.45f);
      auto& plan=pitchPlans[vi][(size_t)gen][(size_t)u];
      plan.frequency=freq;
      plan.phaseIncrement=freq/sr;
      plan.spectral=hd?HdTable::prepareSpectral(freq,sr):HdTable::PreparedSpectral{};
     }
     key.voiceAge=voice.age;key.semis=us.baseSemis;
     key.detune=p.detune;key.mod=voiceMod;
     key.sampleRate=sr;key.sequencePitch=cachedSequencePitch;
     key.unison=us.count;key.hasHD=hd!=nullptr;key.valid=true;
     readyPlans[vi][(size_t)gen]=true;
    }
   }
  }
  // LFO sync, rate and smoothing are block-invariant even when a note moves.
  std::array<float,8> lfoStep{},lfoAlpha{};
  for(size_t k=0;k<lfoStep.size();k++){
   const auto& cfg=p.lfos[k];
   lfoStep[k]=lfoFrequency(cfg,k==0?p.lfoRate:cfg.rate,hostTempo)/sr;
   lfoAlpha[k]=cfg.smooth>0.f?clamp(1.f/(sr*(.0001f+cfg.smooth*.08f)),0.f,1.f):1.f;
  }
  // When no frame modulation is present a whole block shares these HD frame
  // pointers. Voice-specific frame modulation still uses prepareFrame().
  std::array<HdTable::PreparedFrame,generators> sharedFrames{};
  for(int g=0;g<generators;g++){
   const auto* hd=hdTables[(size_t)g];
   if(p.wave[g]==waveCount&&hd)sharedFrames[(size_t)g]=hd->prepareFrame(clamp(p.position,0.f,1.f));
  }
  const auto ampSettings=p.amplitude();
  const float filter1K=2.f-1.85f*clamp(p.resonance,0.f,1.f);
  const float filter2K=2.f-1.85f*clamp(p.resonance2,0.f,1.f);
  const bool sharedFilter1=p.filter1Enabled&&p.filter1Key==0.f&&p.filter1Env==0.f&&p.filter1Vel==0.f;
  const bool sharedFilter2=p.filter2On&&p.filter2Key==0.f&&p.filter2Env==0.f&&p.filter2Vel==0.f;
  const float filter2Base=25.f*std::pow(800.f,clamp(p.cutoff2,0.f,1.f));
  const float filter2SharedG=sharedFilter2?std::tan(pi*clamp(filter2Base,10.f,sr*.45f)/sr):0.f;
  // A sequencer pitch changes at step boundaries, not every audio sample.
  // Reuse exactly the same common oscillator exponents until it changes.
  std::array<float,generators> commonPitch{},sharedLfoFactors{};
  int lastCommonPitch=0;
  bool commonPitchReady=false;
  for(int n=0;n<count;n++){
   float l=0,r=0;
   const SequenceFrame seq=sequence.tick();
   const bool retrigger=seq.index>=0 && seq.index!=lastSequenceIndex && seq.velocity>0.f;
   lastSequenceIndex=seq.index;
   std::array<float,8> lfos{};
   for(size_t k=0;k<lfos.size();k++){
    const auto& cfg=p.lfos[k];float& ph=lfoPhases[k];
    float x=lfoWave(ph+cfg.phase,cfg.shape);
    if(cfg.smooth>0.f){lfoSmooth[k]+=(x-lfoSmooth[k])*lfoAlpha[k];x=lfoSmooth[k];}else lfoSmooth[k]=x;
    lfos[k]=cfg.unipolar?.5f*(x+1.f):x;
    ph+=lfoStep[k];ph-=std::floor(ph);
   }
   const float lfo=lfos[0];
   // Filter cutoff without per-voice routing is common to all active notes.
   const float globalCutoff=25.f*std::pow(800.f,
     clamp(p.cutoff+seq.modulation*.22f+p.lfoDepth*lfo*.18f,0.f,1.f));
   const float globalFilterG=sharedFilter1?std::tan(pi*clamp(globalCutoff,10.f,sr*.45f)/sr):0.f;
   if(!commonPitchReady||lastCommonPitch!=seq.pitch){
    for(int gen=0;gen<generators;gen++)
     if(p.oscLevel[gen]>=.00001f)
      commonPitch[(size_t)gen]=std::exp2(
       (unisonStates[(size_t)gen].baseSemis+(float)seq.pitch)/12.f);
    lastCommonPitch=seq.pitch;
    commonPitchReady=true;
   }
   float sharedPitchMod=0.f;
   if(sharedLfoPitch){
    for(size_t ri=0;ri<activeRouteCount;ri++){
     const auto& route=*activeRoutes[ri];
     if(route.destination!=0)continue;
     float src=0.f;
     if(route.source>=6&&route.source<14){
      const size_t k=(size_t)(route.source-6);
      src=lfos[k]*(p.lfos[k].fade>0.f?0.f:1.f);
     }else if(route.source==16)src=modWheel;
     const float amount=src*route.amount;
     sharedPitchMod+=amount*12.f;
    }
    if(sharedPitchMod!=0.f)
     for(int gen=0;gen<generators;gen++)
      if(p.oscLevel[gen]>=.00001f)
       sharedLfoFactors[(size_t)gen]=std::exp2(
        (unisonStates[(size_t)gen].baseSemis+sharedPitchMod+(float)seq.pitch)/12.f);
   }
   int countActive=0;
   for(size_t voiceIndex=0;voiceIndex<voices.size();++voiceIndex){
    auto &v=voices[voiceIndex];
    if(!v.active) continue;
    // Retrigger the envelope on every played step without reallocating voices.
    if(retrigger){v.amp.on();for(auto &envelope:v.envelopes)envelope.on();}
    v.lfoAge+=1.f/sr;
    float env=v.amp.next(ampSettings,sr)*mix(1.f,v.velocity,ampSettings.velocity);
    // Keep voice allocation intact while steps gate/retrigger the held note.
    // Short smoothing avoids clicks at step/rest boundaries.
    v.stepGate+=(seq.gate-v.stepGate)*clamp(1.f/(sr*.003f),0.f,1.f);
    if(v.amp.stage==Envelope::idle){v.active=false;continue;}
    countActive++;
    std::array<float,6> envs{};
    envs[0]=env;
    for(size_t k=1;k<envs.size();k++){const auto& cfg=p.modEnv[k-1];envs[k]=v.envelopes[k].next(cfg,sr)*mix(1.f,v.velocity,cfg.velocity);}
    float pitchMod=0,frameMod=0,filterMod=0,volumeMod=0,panMod=0;
    for(size_t ri=0;ri<activeRouteCount;ri++){
     const auto &route=*activeRoutes[ri];
     float src=0;
     if(route.source<6)src=envs[(size_t)route.source];
     else if(route.source<14){size_t k=(size_t)(route.source-6);src=lfos[k]*(p.lfos[k].fade>0.f?clamp(v.lfoAge/p.lfos[k].fade,0.f,1.f):1.f);}
     else if(route.source==14)src=v.velocity;
     else if(route.source==15)src=(v.note-60)/60.f;
     else if(route.source==16)src=modWheel;
     float amount=src*route.amount;
     switch(route.destination){
      case 0:pitchMod+=amount*12.f;break;
      case 1:frameMod+=amount;break;
      case 2:filterMod+=amount*.4f;break;
      case 3:volumeMod+=amount;break;
      case 4:panMod+=amount;break;
      default:break;
     }
    }
    float vCutoff=filterMod==0.f?globalCutoff:
     25.f*std::pow(800.f,clamp(p.cutoff+filterMod+seq.modulation*.22f+p.lfoDepth*lfo*.18f,0.f,1.f));
    float vl=0,vr=0;
    const float noteBaseHz=v.baseFrequency;
    const auto prior=v.modSignal;v.modSignal.fill(0.f);
    const float framePosition=clamp(p.position+frameMod,0.f,1.f);
    for(int gen=0;gen<generators;gen++){
     float level=clamp(p.oscLevel[gen],0,1);
     if(level<.00001f)continue;
     const auto& us=unisonStates[(size_t)gen];
     const int uni=us.count;
     const bool fixedPitch=canCachePitch[(size_t)gen] &&
       readyPlans[voiceIndex][(size_t)gen] &&
       pitchMod==pitchKeys[voiceIndex][(size_t)gen].mod &&
       seq.pitch==cachedSequencePitch;
     float base=0.f;
     const float mod=gen==0?prior[1]*p.fmBA+prior[2]*p.fmCA:gen==1?prior[2]*p.fmCB:0.f;
     if(!fixedPitch){
      base=noteBaseHz*(pitchMod==0.f?commonPitch[(size_t)gen]:
       (sharedLfoPitch&&pitchMod==sharedPitchMod?
         sharedLfoFactors[(size_t)gen]:
         std::exp2((us.baseSemis+pitchMod+(float)seq.pitch)/12.f)));
      if(p.fmMode==1)base*=std::exp2(clamp(mod,-1.f,1.f)*2.f);
     }
     const float phaseMod=p.fmMode==0?mod*.5f:0.f;
     const float amp=us.amp;
     const HdTable* const hd=p.wave[gen]==waveCount?hdTables[(size_t)gen]:nullptr;
     const HdTable::PreparedFrame prepared=hd?(frameMod==0.f?
       sharedFrames[(size_t)gen]:hd->prepareFrame(framePosition)):HdTable::PreparedFrame{};
     for(int u=0;u<uni;u++){
      auto& plan=pitchPlans[voiceIndex][(size_t)gen][(size_t)u];
      const float freq=fixedPitch?plan.frequency:
       clamp(base*us.pitch[(size_t)u],1.f,sr*.45f);
      float &ph=v.phase[gen][u];
      const float shifted=ph+phaseMod+us.phase+
                          us.random*v.phaseJitter[(size_t)gen][(size_t)u];
      const float warped=oscillatorWarpPhase(shifted,p.oscWarp[gen],p.oscWarpMode[gen]);
      // Warp guarantees finite normalized phase: the HD reader can skip
      // floor() while retaining the exact frame and mip interpolations.
      float x=0.f;
      if(p.wave[gen]==waveCount){
       if(hd){
        // Same stateless spectral selection as v1.2.9 for dynamic LFO/FM.
        // The first v1.2.10 benchmark proved stateful mip tracking slower
        // (up to +40% at x16): keep the precise normalized-phase fast path
        // but avoid thousands of per-sample cache writes.
        const auto spectral=fixedPitch?plan.spectral:
         HdTable::prepareSpectral(freq,sr);
        x=hd->samplePreparedSpectralWrapped(warped,prepared,spectral);
       }else x=customBandlimited.sample(gen,warped,framePosition,freq,sr);
      }else x=bank.sample(p.wave[gen],warped,framePosition,freq,sr);
      const float blendGain=us.blendGain[(size_t)u];
      x=oscillatorShape(x,p.oscShape[gen],p.oscShapeMode[gen]);
      v.modSignal[(size_t)gen]+=x*level/(float)uni;
      if(gen==0&&p.ringAmount>0.f){const float carrier=clamp(prior[1]+prior[2],-1.f,1.f);x*=mix(1.f,p.ringMode==0?carrier:.5f*(carrier+1.f),p.ringAmount);}
      x*=amp*blendGain;
      ph+=fixedPitch?plan.phaseIncrement:freq/sr;
      if(ph>=1.f)ph-=std::floor(ph);
      float pan=panMod==0.f?us.pan[(size_t)u]:
       clamp(us.relative[(size_t)u]*p.spread+panMod+p.oscPan[gen],-1.f,1.f);
      vl+=x*(1-pan)*.5f;vr+=x*(1+pan)*.5f;
     }
    }
    const float key=(v.note-60)/12.f,ev=envs[1];
    float a=vl,b=vr;
    if(p.filter1Enabled){
     const float cutoffA=sharedFilter1?vCutoff:
      vCutoff*std::pow(2.f,key*p.filter1Key+5.f*ev*p.filter1Env+2.f*v.velocity*p.filter1Vel);
     const float gA=sharedFilter1&&filterMod==0.f?globalFilterG:
      std::tan(pi*clamp(cutoffA,10.f,sr*.45f)/sr);
     a=mix(vl,v.filters[0][0].runPrepared(vl,gA,filter1K,p.filter1Drive,p.filter1Type),p.filter1Mix);
     b=mix(vr,v.filters[0][1].runPrepared(vr,gA,filter1K,p.filter1Drive,p.filter1Type),p.filter1Mix);
    }
    if(p.filter2On){
     const float cutoffB=sharedFilter2?filter2Base:
      filter2Base*std::pow(2.f,key*p.filter2Key+5.f*ev*p.filter2Env+2.f*v.velocity*p.filter2Vel);
     const float gB=sharedFilter2?filter2SharedG:std::tan(pi*clamp(cutoffB,10.f,sr*.45f)/sr);
     float inL=p.parallelFilters?vl:a,inR=p.parallelFilters?vr:b;
     float f2l=v.filters[1][0].runPrepared(inL,gB,filter2K,p.filter2Drive,p.filter2Type);
     float f2r=v.filters[1][1].runPrepared(inR,gB,filter2K,p.filter2Drive,p.filter2Type);
     a=mix(a,f2l,p.parallelFilters?p.filter2Mix*.5f:p.filter2Mix);
     b=mix(b,f2r,p.parallelFilters?p.filter2Mix*.5f:p.filter2Mix);
    }
    const float dry=1.f;
    const float seqGain=seq.velocity*v.stepGate;
    l+=a*env*v.velocity*dry*seqGain*clamp(1.f+volumeMod,0,2);
    r+=b*env*v.velocity*dry*seqGain*clamp(1.f+volumeMod,0,2);
   }
   activeCount=countActive;
   // Soft saturation.
   float gain=1.f+p.drive*7.f;
   l=std::tanh(l*gain)*.7f;r=std::tanh(r*gain)*.7f;
   equalizeCompress(l,r);modulationFx(l,r);
   if(!delayL.empty()){
    const size_t len=delayL.size();
    delaySmoothed+=(clamp(p.delayTime,.02f,1.9f)-delaySmoothed)*.002f;
    const float offset=sr*delaySmoothed;const size_t whole=(size_t)offset;
    const size_t read=(delayIndex+len-(whole%len))%len,prev=(read+len-1)%len;
    const float fraction=offset-(float)whole;
    const float dl=mix(delayL[read],delayL[prev],fraction),dr=mix(delayR[read],delayR[prev],fraction);
    const float tone=clamp(p.delayTone,0.f,1.f);
    delayToneL+=(dl-delayToneL)*(.02f+.98f*tone);delayToneR+=(dr-delayToneR)*(.02f+.98f*tone);
    delayL[delayIndex]=l+delayToneL*clamp(p.delayFeedback,0.f,.92f);delayR[delayIndex]=r+delayToneR*clamp(p.delayFeedback,0.f,.92f);
    if(++delayIndex>=len)delayIndex=0;
    l+=dl*p.delay*.5f;r+=dr*p.delay*.5f;
   }
   reverbFx.tick(l,r,p);
   const float trem=std::sin(2*pi*tremPhase);tremPhase+=p.tremRate/sr;tremPhase-=std::floor(tremPhase);
   const float depth=clamp(p.tremDepth,0.f,1.f);
   if(p.tremMode==0){const float gain=1.f-depth*.5f*(1.f+trem);l*=gain;r*=gain;}
   else{l*=1.f-depth*.5f*(1.f+trem);r*=1.f-depth*.5f*(1.f-trem);}
   const float ring=std::sin(2*pi*ringPhase);ringPhase+=p.fxRingFreq/sr;ringPhase-=std::floor(ringPhase);
   l*=mix(1.f,ring,p.fxRingMix);r*=mix(1.f,ring,p.fxRingMix);
   const float mid=.5f*(l+r),side=.5f*(l-r)*p.masterWidth;l=mid+side;r=mid-side;
   outL[n]+=std::tanh(l*clamp(p.master,0,1.5f));
   outR[n]+=std::tanh(r*clamp(p.master,0,1.5f));
  }
 }
};

inline const std::vector<Preset>& factoryPresets() {
 static const std::vector<Preset> items=[] {
  const std::array<std::pair<const char *,std::vector<const char*>>,10> groups={{
   {"Bass",{"Metallic Pulse","Velvet Low End","Rubber Wire","Basalt FM","Night Rail","Bronze Sub","Sub Gravity","Obsidian Growl","Titan Foundation","Deep Current","Gravel Sweep","Pressure Drop"}},
   {"Pads",{"Serenity Awakening","Velvet Universe","Aurora Motion","Glass Cathedral","Silk Meridian","Opal Drift","Amber Hours","Breathing Marble","Liquid Dreams","Celestial Waves","Warm Tide","Lunar Chorale","Harbour Light","Winter Garden","Soft Orbit"}},
   {"Cinematic",{"Midnight Reflections","Dust of Empires","Last Horizon","Tectonic Rise","Cold Machine","Ancestral Drone","Crystal Horizon","Infinite Atmosphere","Ocean of Light","Ethereal Memories"}},
   {"Leads",{"Digital Emotions","Golden Frequency","Quantum Drift","Future Harmony","Comet Line","Mercury Voice","Blue Voltage","Iron Ribbon","Satellite Signal","Northern Glow","Neon Prophet","Harmonic Edge"}},
   {"Plucks",{"Crystalline Rain","Porcelain Bell","Polar Droplets","Luminous Marimba","Frozen Glass","Ceramic Keys","Starlight Pluck","Shimmering Reed","Silver Spark","Moonlit Mallet"}},
   {"Keys",{"Aurora Keys","Digital Rhodes","Ice Piano","Amber Electric","Soft Circuit","Crystal Keys","Gentle Current","Vintage Photon"}},
   {"Arps",{"Orbit Pattern","Running Lights","Solstice Steps","Amber Sequence","Northern Pulse","Chromatic Tide","Celestial Arp","Spiral Motion","Neon Eighths","Transient Bloom"}},
   {"EDM",{"Festival Dawn","Hyperdrive","Impact Drive","Glitter Saw","Voltage Rise","Supernova Stack","Electric Vortex","Peak Motion"}},
   {"Ambient",{"Quiet Nebula","Still Water","Ocean Breath","Ethereal Memories II","Infinite Space","Soft Eclipse","Meditation Cloud","Slow Horizon"}},
   {"Experimental",{"Ghost Harmonics","Broken Spectrum","Metallic Fog","Neural Glitch","Grain Shift","Alien Glass","Fractal Noise"}}
  }};
  std::vector<Preset> result;result.reserve(100);
  int idx=0;
  for(const auto& group:groups) for(const auto *name:group.second){
   Preset patch;patch.name=name;patch.category=group.first;
   Params &p=patch.p;
   const float v=(idx*37%101)/100.f;
   p.cutoff=clamp(.25f+.68f*v,.05f,.99f);
   p.position=(idx*23%100)/100.f;
   p.detune=.03f+.45f*((idx*13%97)/100.f);
   p.lfoRate=.08f+(idx*17%35)*.17f;
   p.lfoDepth= (idx%4==0)? .08f:0;
   p.reverb=(idx%5)*.047f; p.delay=(idx%4)*.05f;
   p.wave[0]=idx%waveCount;p.wave[1]=(idx+7)%waveCount;
   p.oscLevel[0]=.62f;p.oscLevel[1]=.13f+(idx%4)*.07f;
   p.polyphony=16;p.unison[0]=1+(idx%3);
   p.cutoff2=p.cutoff*.75f;
   if(patch.category=="Bass") {p.oscPitch[1]=-12;p.attack=.003f;p.decay=.18f;p.sustain=.57f;p.release=.16f;p.polyphony=8;}
   if(patch.category=="Pads"||patch.category=="Ambient") {p.attack=.12f+(idx%7)*.17f;p.decay=1.1f;p.sustain=.78f;p.release=1.6f+(idx%4);p.reverb=.30f;}
   if(patch.category=="Plucks"||patch.category=="Keys") {p.attack=.003f;p.decay=.24f;p.sustain=.12f;p.release=.31f;}
   if(patch.category=="Leads"||patch.category=="EDM") {p.unison[0]=3;p.detune=.24f;p.attack=.004f;p.sustain=.80f;}
   if(patch.category=="Cinematic"||patch.category=="Experimental") {p.oscLevel[2]=.20f;p.wave[2]=(idx+5)%waveCount;p.reverb=.25f;p.release=.8f;}
   // Category-specific sound design using the new DSP; preserved names and ordering.
   p.resonance=.10f+.19f*v;p.filter1Drive=.05f*v;p.envCurve=(v-.5f)*.6f;
   p.envVelocity=.20f;p.eqLowGain=-2.f+v*4.f;p.eqHighGain=(v-.5f)*4.f;
   p.modFxRate=.18f+.8f*v;p.modFxDepth=.25f+.45f*v;p.reverbDamping=.4f+.35f*v;
   p.reverbSize=.75f+.5f*v;p.reverbPreDelay=.008f+.03f*v;p.delayFeedback=.22f+.25f*v;
   p.compMix=.18f;p.compThreshold=-16.f;p.compRatio=2.5f;
   if(patch.category=="Bass"){p.oscOctave[1]=-1;p.filter1Type=idx%3==0?1:0;p.filter1Env=.20f+.25f*v;p.modEnv[0].decay=.12f+.25f*v;p.modEnv[0].sustain=0;p.reverb=.04f;p.modFxMix=.02f;}
   else if(patch.category=="Pads"||patch.category=="Ambient"){p.modFxType=0;p.modFxMix=.24f+.22f*v;p.reverbType=2;p.reverb=.32f+.16f*v;p.oscPan[0]=-.18f;p.oscPan[1]=.18f;p.filter1Type=idx%3==0?13:0;p.lfos[0].fade=.4f;p.lfoDepth=.07f;}
   else if(patch.category=="Cinematic"||patch.category=="Experimental"){p.filter1Type=idx%15;p.filter2On=true;p.parallelFilters=true;p.filter2Type=6;p.filter2Mix=.30f;p.fmBA=.06f+.22f*v;p.modFxType=idx%3;p.modFxMix=.24f;p.reverbType=2;p.reverb=.36f;p.lfoDepth=.12f;}
   else if(patch.category=="Leads"||patch.category=="EDM"){p.filter1Type=idx%2;p.filter1Env=.12f;p.drive=.08f+.13f*v;p.modFxMix=.15f;p.compMix=.35f;p.fmBA=.08f*v;p.reverb=.12f;}
   else if(patch.category=="Plucks"||patch.category=="Keys"){p.filter1Type=1;p.filter1Env=.30f;p.modEnv[0].decay=.08f+.22f*v;p.modEnv[0].sustain=0;p.envCurve=.25f;p.modFxMix=.12f;p.reverb=.20f;}
   else {p.lfos[0].sync=true;p.lfos[0].division=5+idx%4;p.lfoDepth=.16f;p.filter1Type=4;p.delayTime=.20f;p.delay=.25f;p.modFxMix=.12f;}
   result.push_back(std::move(patch));idx++;
  }
  return result;
 }();
 return items;
}
} // namespace serenity
