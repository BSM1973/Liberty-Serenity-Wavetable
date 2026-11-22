// Same source is compiled against pinned v1.2.7 and candidate DSP headers.
// Runs on the same GitHub Actions runner in alternating A/B order.
#include "SerenityDSP.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace serenity;
using Clock=std::chrono::steady_clock;
struct Scenario {const char* name;int rate,block,unison;bool twoFilters;int blocks;int variant=0;};
constexpr Scenario scenarios[]={
 {"u4_48k_b256",48000,256,4,false,52},
 {"u16_48k_b128",48000,128,16,false,45},
 {"u16_96k_b256",96000,256,16,false,45},
 {"u4_48k_b256_f2",48000,256,4,true,52},
 {"u4_48k_b256_dynamic_fm",48000,256,4,false,42,1},
 {"u4_48k_b256_pitch_morph",48000,256,4,false,42,2},
 {"u16_48k_b128_dynamic_fm",48000,128,16,false,45,1},
 {"u16_48k_b128_pitch_morph",48000,128,16,false,45,2},
 {"u16_48k_b128_seq_pitch",48000,128,16,false,45,3},
 {"u16_48k_b128_lfo_pitch",48000,128,16,false,42,4},
 {"u16_48k_b128_key_pitch",48000,128,16,false,42,5}
};
static double quantile(std::vector<double> v,double p){
 std::sort(v.begin(),v.end());
 return v[std::min(v.size()-1,(size_t)std::ceil((v.size()-1)*p))];
}
int main(int argc,char** argv){
 if(argc!=2){std::cerr<<"usage: hd_ab_benchmark output_prefix\n";return 2;}
 const std::string prefix=argv[1];
 std::array<std::shared_ptr<const HdTable>,4> tables;
 for(int g=0;g<4;g++){
  std::vector<float> pcm((size_t)hdMaxFrames*hdSamples);
  for(int f=0;f<hdMaxFrames;f++)for(int i=0;i<hdSamples;i++){
   const double ph=2*hdPi*i/hdSamples,amp=.17+.4*f/255.;
   pcm[(size_t)f*hdSamples+i]=(float)(amp*std::sin(ph+g*.13)+.12*std::sin(3*ph)+.03*std::sin(7*ph));
  }
  tables[(size_t)g]=std::make_shared<const HdTable>(std::move(pcm));
 }
 std::ofstream csv(prefix+".csv");if(!csv)return 3;
 csv<<std::fixed<<std::setprecision(7);
 csv<<"scenario,rate,block,voices,unison,two_filters,mean_ms,p50_ms,p99_ms,max_ms,deadline_ms,overruns,blocks,rms,peak\n";
 for(const auto& sc:scenarios){
  auto engine=std::make_unique<Engine>();
  engine->prepare(sc.rate);
  Params p;p.polyphony=32;p.attack=.001f;p.decay=.03f;p.sustain=.75f;p.release=.15f;
  p.delay=.12f;p.reverb=.12f;p.modFxMix=.12f;p.compMix=.2f;
  p.detune=.31f;p.spread=.55f;p.position=.37f;
  if(sc.twoFilters){p.filter2On=true;p.filter2Mix=.65f;p.filter2Key=.15f;p.filter1Key=.21f;}
  if(sc.variant==1){p.fmMode=1;p.fmBA=.35f;p.fmCA=.22f;p.fmCB=.18f;}
  for(int g=0;g<4;g++){p.oscLevel[g]=.18f;p.wave[g]=waveCount;p.unison[g]=sc.unison;engine->setHdTable(g,tables[(size_t)g].get());}
  engine->setParams(p);
  if(sc.variant==2){engine->route(0)={14,0,.36f};engine->route(1)={0,1,.22f};}
  if(sc.variant==4){engine->route(0)={6,0,.12f};engine->route(1)={0,1,.18f};}
  if(sc.variant==5){engine->route(0)={15,0,.24f};engine->route(1)={14,0,.14f};}
  if(sc.variant==3){
   auto pattern=defaultSequence();pattern.mode=2;pattern.rate=32;
   pattern.tempo=200.f;pattern.length=8;
   engine->setSequence(pattern);
  }
  for(int n=0;n<32;n++)engine->noteOn(36+n,.76f);
  std::vector<float> l((size_t)sc.block),r((size_t)sc.block);
  // Warm up filters and LFO in exactly the same way in both versions.
  for(int n=0;n<16;n++){std::fill(l.begin(),l.end(),0.f);std::fill(r.begin(),r.end(),0.f);engine->render(l.data(),r.data(),sc.block);}
  std::ofstream audio(prefix+"-"+sc.name+".f32",std::ios::binary);if(!audio)return 4;
  double energy=0,peak=0;std::vector<double> timeMs;timeMs.reserve(sc.blocks);
  int over=0;
  for(int n=0;n<sc.blocks;n++){
   p.position=.4f+.35f*std::sin(n*.043f);
   std::fill(l.begin(),l.end(),0.f);std::fill(r.begin(),r.end(),0.f);
   auto begin=Clock::now();
   engine->setParams(p);engine->render(l.data(),r.data(),sc.block);
   double ms=std::chrono::duration<double,std::milli>(Clock::now()-begin).count();
   timeMs.push_back(ms);
   over+=(ms>1000.*sc.block/sc.rate);
   if(engine->numVoices()!=32){std::cerr<<"polyphony violation\n";return 5;}
   for(int i=0;i<sc.block;i++){
    const float x=l[(size_t)i],y=r[(size_t)i];
    if(!std::isfinite(x)||!std::isfinite(y)){std::cerr<<"non-finite audio\n";return 6;}
    energy+=(double)x*x+y*y;peak=std::max({peak,(double)std::abs(x),(double)std::abs(y)});
    {audio.write(reinterpret_cast<const char*>(&x),sizeof(x));audio.write(reinterpret_cast<const char*>(&y),sizeof(y));}
   }
  }
  double sum=0;for(auto ms:timeMs)sum+=ms;
  const double budget=1000.*sc.block/sc.rate;
  csv<<sc.name<<','<<sc.rate<<','<<sc.block<<",32,"<<sc.unison<<','<<(sc.twoFilters?1:0)<<','
    <<sum/sc.blocks<<','<<quantile(timeMs,.5)<<','<<quantile(timeMs,.99)<<','
    <<*std::max_element(timeMs.begin(),timeMs.end())<<','<<budget<<','<<over<<','<<sc.blocks<<','
    <<std::sqrt(energy/(2.*sc.blocks*sc.block))<<','<<peak<<'\n';
 }
 std::cout<<"A/B benchmark saved: "<<prefix<<".csv\n";
 return 0;
}
