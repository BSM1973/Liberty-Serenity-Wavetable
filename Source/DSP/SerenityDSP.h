#pragma once
// Serenity WAVETABLE - Liberty Music Software
// Independent, allocation-free (in render) synthesis core.
// This is an actual playable foundation, not a claim that every planned engine is complete.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "SerenitySequencer.h"
#include "SerenityCustomWavetable.h"
#include "SerenityBandlimitedWavetable.h"
#include "SerenityOscTransform.h"

namespace serenity {
constexpr float pi=3.14159265358979323846f;
constexpr int maxVoices=32, generators=4, maxUnison=16, waveCount=16;
constexpr int frames=8, tableSize=512, mipCount=8;
inline float clamp(float x,float a,float b) noexcept { return std::max(a,std::min(x,b)); }
inline float mix(float a,float b,float t) noexcept {return a+(b-a)*t;}

struct Params {
 float master=0.65f, cutoff=0.85f, resonance=0.14f, position=0.50f;
 float cutoff2=0.55f,resonance2=.2f,filter2Mix=1.f;
 bool filter2On=false, parallelFilters=false;
 float attack=0.004f, decay=0.45f, sustain=0.70f, release=0.35f;
 float detune=0.20f, spread=0.70f, delay=0.10f, reverb=0.15f, drive=0.0f;
 float lfoRate=5.0f, lfoDepth=0.0f, wavemorph=0.0f;
 float oscLevel[4]={0.70f,0.0f,0.0f,0.0f};
 float oscPitch[4]={0,0,0,-12};
 float oscOctave[4]{},oscSemi[4]{},oscFine[4]{},oscPan[4]{};
 float oscWarp[4]{},oscShape[4]{};
 // Unison and phase controls default to neutral for backward compatibility.
 float oscBlend[4]{},oscPhase[4]{},oscRandPhase[4]{};
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
 enum Stage { idle, attack, decay, sustain, release } stage=idle;
 float value=0,releaseStep=0;
 void on() noexcept {stage=attack; if(value<0.0001f)value=0;}
 void off(float seconds,float sr) noexcept {stage=release; releaseStep=value/(std::max(.001f,seconds)*sr);}
 float next(const Params &p,float sr) noexcept {
  switch(stage) {
   case idle: return 0;
   case attack: value+=1.f/(std::max(.001f,p.attack)*sr);if(value>=1){value=1;stage=decay;}break;
   case decay: value-=(1-p.sustain)/(std::max(.001f,p.decay)*sr);if(value<=p.sustain){value=p.sustain;stage=sustain;}break;
   case sustain: value=p.sustain;break;
   case release: value-=releaseStep;if(value<=0){value=0;stage=idle;}break;
  }
  return clamp(value,0,1);
 }
};
struct Voice {
 int note=-1;bool active=false;
 float velocity=0,stepGate=0,filterL=0,filterR=0,filter2L=0,filter2R=0;
 float filter3L=0,filter3R=0,filter4L=0,filter4R=0;
 Envelope amp;
 std::array<Envelope,6> envelopes{};
 std::array<std::array<float,maxUnison>,generators> phase{};
 std::array<std::array<float,maxUnison>,generators> phaseJitter{};
 uint64_t age=0;
 void start(int n,float v,uint64_t when) noexcept {
  note=n;velocity=v;active=true;age=when;amp={};amp.on();
  for(auto &env:envelopes){env={};env.on();}
  filterL=filterR=filter2L=filter2R=0;filter3L=filter3R=filter4L=filter4R=0;
  for(auto &array:phase) array.fill(0);
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

class Engine {
 WavetableBank bank;
 EditableWavetable customTable{};
 BandlimitedCustomWavetable customBandlimited{};
 std::array<Voice,maxVoices> voices{};
 std::array<ModRoute,64> routes{};
 float sr=48000, lfoPhase=0,modWheel=0;
 StepClock sequence;
 int lastSequenceIndex=-1;
 uint64_t noteCounter=0;
 std::vector<float> delayL,delayR,reverbL,reverbR;
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
 void setCustomMipFrame(int oscillator,int frame,const CustomMipFrame& mips) noexcept {
  customBandlimited.setFrame(oscillator,frame,mips);
 }
 void prepare(double sampleRate) {
  sr=(float)std::max(8000.0,sampleRate);
  sequence.prepare(sampleRate);
  delayL.assign((size_t)(sr*2.0f)+1,0);delayR=delayL;
  reverbL.assign((size_t)(sr*.173f)+1,0);reverbR.assign((size_t)(sr*.193f)+1,0);
  delayIndex=reverbIndex=0;
  reset();
 }
 void reset() noexcept {for(auto &v:voices)v=Voice{};lfoPhase=0;activeCount=0;sequence.reset();lastSequenceIndex=-1;}
 void setParams(const Params &next) noexcept {p=next;p.polyphony=std::max(1,std::min(p.polyphony,maxVoices));}
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
    v.amp.off(p.release,sr);for(auto &env:v.envelopes)env.off(p.release,sr);
  }}
 void allNotesOff() noexcept {for(auto &v:voices) if(v.active){v.amp.off(.01f,sr);for(auto &env:v.envelopes)env.off(.01f,sr);}}
 void render(float *outL,float *outR,int count) noexcept {
  if(!outL||!outR||count<=0)return;
  for(int n=0;n<count;n++){
   float l=0,r=0;
   const SequenceFrame seq=sequence.tick();
   const bool retrigger=seq.index>=0 && seq.index!=lastSequenceIndex && seq.velocity>0.f;
   lastSequenceIndex=seq.index;
   float lfo=std::sin(2*pi*lfoPhase);
   std::array<float,8> lfos{};
   for(int k=0;k<8;k++) lfos[(size_t)k]=std::sin(2*pi*(lfoPhase*(1+k*.18f)+k*.125f));
   lfoPhase+=p.lfoRate/sr;if(lfoPhase>=1)lfoPhase-=1;
   int countActive=0;
   for(auto &v:voices){
    if(!v.active) continue;
    // Retrigger the envelope on every played step without reallocating voices.
    if(retrigger){v.amp.on();for(auto &envelope:v.envelopes)envelope.on();}
    float env=v.amp.next(p,sr);
    // Keep voice allocation intact while steps gate/retrigger the held note.
    // Short smoothing avoids clicks at step/rest boundaries.
    v.stepGate+=(seq.gate-v.stepGate)*clamp(1.f/(sr*.003f),0.f,1.f);
    if(v.amp.stage==Envelope::idle){v.active=false;continue;}
    countActive++;
    std::array<float,6> envs{};
    for(int k=0;k<6;k++)envs[(size_t)k]=v.envelopes[(size_t)k].next(p,sr);
    float pitchMod=0,frameMod=0,filterMod=0,volumeMod=0,panMod=0;
    for(const auto &route:routes){
     if(route.source<0||route.destination<0||route.amount==0)continue;
     float src=0;
     if(route.source<6)src=envs[(size_t)route.source];
     else if(route.source<14)src=lfos[(size_t)(route.source-6)];
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
    float vCutoff=25.f*std::pow(800.f,clamp(p.cutoff+filterMod+seq.modulation*.22f+p.lfoDepth*lfo*.18f,0,1));
    float vl=0,vr=0;
    for(int gen=0;gen<generators;gen++){
     float level=clamp(p.oscLevel[gen],0,1);
     if(level<.00001f)continue;
     int uni=std::max(1,std::min(maxUnison,p.unison[gen]));
     float base=noteHz(v.note)*std::pow(2.f,(p.oscPitch[gen]
      +p.oscOctave[gen]*12.f+p.oscSemi[gen]+p.oscFine[gen]*.01f
      +pitchMod+(float)seq.pitch)/12.f);
     float amp=level/(std::sqrt((float)uni));
     for(int u=0;u<uni;u++){
      float det=(uni==1)?0:((2.f*u)/(uni-1)-1.f)*p.detune*.19f;
      float freq=base*std::pow(2.f,det/12.f);
      float &ph=v.phase[gen][u];
      const float shifted=ph+clamp(p.oscPhase[gen],0.f,1.f)+
                          clamp(p.oscRandPhase[gen],0.f,1.f)*v.phaseJitter[(size_t)gen][(size_t)u];
      const float warped=oscillatorWarpPhase(shifted,p.oscWarp[gen],p.oscWarpMode[gen]);
      float x=(p.wave[gen]==waveCount ?
       customBandlimited.sample(gen,warped,clamp(p.position+frameMod,0,1),freq,sr) :
       bank.sample(p.wave[gen],warped,clamp(p.position+frameMod,0,1),freq,sr));
      const float outer=uni==1?0.f:std::abs((2.f*u)/(uni-1)-1.f);
      const float blendGain=1.f-.65f*clamp(p.oscBlend[gen],0.f,1.f)*outer;
      x=oscillatorShape(x,p.oscShape[gen],p.oscShapeMode[gen])*amp*blendGain;
      ph+=freq/sr;if(ph>=1.f)ph-=std::floor(ph);
      float pan=clamp(((uni==1)?0:((2.f*u)/(uni-1)-1.f)*p.spread)+panMod+
                       p.oscPan[gen],-1,1);
      vl+=x*(1-pan)*.5f;vr+=x*(1+pan)*.5f;
     }
    }
    // Two cascaded smoothed filters provide stable low-pass response.
    float a=lowpass(vl,v.filterL,vCutoff),b=lowpass(vr,v.filterR,vCutoff);
    a=lowpass(a,v.filter2L,vCutoff);b=lowpass(b,v.filter2R,vCutoff);
    if(p.filter2On){
     const float c2=25.f*std::pow(800.f,clamp(p.cutoff2,0.f,1.f));
     float f2l=lowpass(p.parallelFilters?vl:a,v.filter3L,c2);
     float f2r=lowpass(p.parallelFilters?vr:b,v.filter3R,c2);
     f2l=lowpass(f2l,v.filter4L,c2);f2r=lowpass(f2r,v.filter4R,c2);
     a=mix(a,f2l,p.parallelFilters?p.filter2Mix*.5f:p.filter2Mix);
     b=mix(b,f2r,p.parallelFilters?p.filter2Mix*.5f:p.filter2Mix);
    }
    float dry=1.f-p.resonance*.22f;
    const float seqGain=seq.velocity*v.stepGate;
    l+=a*env*v.velocity*dry*seqGain*clamp(1.f+volumeMod,0,2);
    r+=b*env*v.velocity*dry*seqGain*clamp(1.f+volumeMod,0,2);
   }
   activeCount=countActive;
   // Soft saturation.
   float gain=1.f+p.drive*7.f;
   l=std::tanh(l*gain)*.7f;r=std::tanh(r*gain)*.7f;
   if(!delayL.empty()){
    const size_t len=delayL.size(), offset=(size_t)(sr*.38f);
    const size_t read=(delayIndex+len-(offset%len))%len;
    const float dl=delayL[read],dr=delayR[read];
    delayL[delayIndex]=l+dl*.35f;delayR[delayIndex]=r+dr*.35f;
    if(++delayIndex>=len)delayIndex=0;
    l+=dl*p.delay*.5f;r+=dr*p.delay*.5f;
   }
   if(!reverbL.empty()){
    float wetL=reverbL[reverbIndex%reverbL.size()];
    float wetR=reverbR[reverbIndex%reverbR.size()];
    reverbL[reverbIndex%reverbL.size()]=l+wetL*.76f;
    reverbR[reverbIndex%reverbR.size()]=r+wetR*.75f;
    ++reverbIndex;
    l+=wetL*p.reverb*.45f;r+=wetR*p.reverb*.45f;
   }
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
   result.push_back(std::move(patch));idx++;
  }
  return result;
 }();
 return items;
}
} // namespace serenity
