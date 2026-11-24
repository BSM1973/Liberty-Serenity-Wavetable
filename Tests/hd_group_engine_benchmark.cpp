// Full production Engine::render() path, three unison read modes:
// 0: scalar original, 1: 4-oscillator grouped scalar, 2: grouped NEON/SSE2.
// Never toggles modes in a shipped plugin. Strict sample parity is a gate.
#include "SerenityDSP.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>
#include <memory>
#include <numeric>
#include <string>
#include <stdexcept>
#include <vector>
using namespace serenity;
using Clock=std::chrono::steady_clock;
struct Case {const char* name;int sr,buffer,unison,kind,blocks;};
constexpr Case cases[]={
 {"x16_48k_b128_static",48000,128,16,0,48},
 {"x16_48k_b128_morph",48000,128,16,1,48},
 {"x16_48k_b128_sequence",48000,128,16,2,64},
 {"x16_96k_b256_static",96000,256,16,0,48},
 {"x4_48k_b256_static",48000,256,4,0,48},
 {"x16_48k_b128_dynamic_lfo",48000,128,16,3,48},
 {"x16_48k_b128_modulo_phase",48000,128,16,4,48},
 {"x12_48k_b128_tail",48000,128,12,1,48},
 {"x8_48k_b128_soa",48000,128,8,1,48}
};
struct Result{
 double mean=0,p99=0,peak=0;
 std::vector<float> audio;
 int overruns=0;
};
std::array<std::shared_ptr<const HdTable>,4> tables;
static Result renderMode(const Case& sc,int mode){
 auto engine=std::make_unique<Engine>();
 engine->prepare(sc.sr);
 engine->setHdUnisonGroupMode(mode);
 Params p;
 p.polyphony=32;p.attack=.001f;p.decay=.03f;p.sustain=.75f;p.release=.15f;
 p.delay=.12f;p.reverb=.12f;p.modFxMix=.12f;p.compMix=.2f;
 p.detune=.31f;p.spread=.55f;p.position=.37f;
 for(int g=0;g<generators;g++){
  p.oscLevel[g]=.18f;p.wave[g]=waveCount;p.unison[g]=sc.unison;
  engine->setHdTable(g,tables[(size_t)g].get());
 }
 if(sc.kind==4){p.oscWarp[0]=.75f;p.oscWarpMode[0]=3;p.detune=.95f;}
 engine->setParams(p);
 if(sc.kind==1){engine->route(0)={14,1,.37f};engine->route(1)={0,1,.21f};}
 if(sc.kind==3){engine->route(0)={6,0,.17f};engine->route(1)={0,1,.19f};}
 if(sc.kind==2){
  auto pattern=defaultSequence();pattern.mode=2;pattern.rate=32;
  pattern.tempo=300.f;pattern.length=8;engine->setSequence(pattern);
 }
 for(int n=0;n<32;n++)engine->noteOn(36+n,.76f);
 std::vector<float> l((size_t)sc.buffer),r((size_t)sc.buffer);
 for(int n=0;n<16;n++){
  std::fill(l.begin(),l.end(),0.f);std::fill(r.begin(),r.end(),0.f);
  engine->render(l.data(),r.data(),sc.buffer);
 }
 Result result;
 result.audio.reserve((size_t)sc.blocks*sc.buffer*2);
 std::vector<double> times;times.reserve((size_t)sc.blocks);
 for(int n=0;n<sc.blocks;n++){
  p.position=sc.kind==0?.37f:.4f+.35f*std::sin(n*.043f);
  engine->setParams(p);
  std::fill(l.begin(),l.end(),0.f);std::fill(r.begin(),r.end(),0.f);
  const auto begin=Clock::now();
  engine->render(l.data(),r.data(),sc.buffer);
  const double ms=std::chrono::duration<double,std::milli>(Clock::now()-begin).count();
  times.push_back(ms);
  result.overruns+=(ms>1000.*sc.buffer/sc.sr);
  if(engine->numVoices()!=32)throw std::runtime_error("32-note polyphony regression");
  for(int i=0;i<sc.buffer;i++){
   const float a=l[(size_t)i],b=r[(size_t)i];
   if(!std::isfinite(a)||!std::isfinite(b))throw std::runtime_error("non-finite grouped audio");
   result.audio.push_back(a);result.audio.push_back(b);
  }
 }
 result.mean=std::accumulate(times.begin(),times.end(),0.)/times.size();
 std::sort(times.begin(),times.end());
 result.p99=times[(size_t)std::ceil((times.size()-1)*.99)];
 result.peak=times.back();
 return result;
}
int main(int argc,char** argv){
 try{
  if(argc!=2&&argc!=3){std::cerr<<"usage: serenity_hd_group_engine_benchmark <report.csv>\n";return 2;}
  for(int g=0;g<generators;g++){
   std::vector<float> pcm((size_t)hdMaxFrames*hdSamples);
   for(int f=0;f<hdMaxFrames;f++)for(int i=0;i<hdSamples;i++){
    const double p=2*hdPi*i/hdSamples,amp=.17+.4*f/255.;
    pcm[(size_t)f*hdSamples+i]=(float)(amp*std::sin(p+g*.13)+.12*std::sin(3*p)+.03*std::sin(7*p));
   }
   tables[(size_t)g]=std::make_shared<const HdTable>(std::move(pcm));
  }
  std::ofstream out(argv[1]);if(!out)throw std::runtime_error("could not write report");
  out<<std::fixed<<std::setprecision(8);
  out<<"scenario,mode,mean_ms,p99_ms,max_ms,deadline_ms,mean_budget_pct,p99_budget_pct,overruns,"
        "max_delta,relative_rms_delta,samples,quality_pass\n";
  const bool endurance=argc==3&&std::string(argv[2])=="--endurance";
  const bool avx2=hdAvx2Available();
  std::cout<<"AVX2 available: "<<avx2<<"\n";
  const std::vector<int> modes=avx2?std::vector<int>{0,1,2,3,4}:std::vector<int>{0,1,2,3};
  const std::vector<int> longModes=avx2?std::vector<int>{0,3,4}:std::vector<int>{0,3};
  const auto& selected=endurance?longModes:modes;
  std::vector<int> orders=selected;
  if(!endurance)orders.insert(orders.end(),selected.rbegin(),selected.rend());
  const std::vector<Case> scenarios=endurance?std::vector<Case>{{"x16_48k_b128_endurance_60s",48000,128,16,0,22500}}:std::vector<Case>(std::begin(cases),std::end(cases));
  bool allGood=true;
  for(const auto& sc:scenarios){
   std::array<std::vector<double>,5> means{},p99s{};
   std::array<Result,5> first{};
   std::array<bool,5> recorded{};
   std::array<double,5> peaks{};
   std::array<int,5> overruns{};
   for(int mode:orders){
    auto result=renderMode(sc,mode);
    peaks[(size_t)mode]=std::max(peaks[(size_t)mode],result.peak);
    overruns[(size_t)mode]+=result.overruns;
    means[(size_t)mode].push_back(result.mean);
    p99s[(size_t)mode].push_back(result.p99);
    if(!recorded[(size_t)mode]){
     first[(size_t)mode]=std::move(result);
     recorded[(size_t)mode]=true;
    }
   }
   const double budget=1000.*sc.buffer/sc.sr;
   for(int mode:selected){
    std::sort(means[(size_t)mode].begin(),means[(size_t)mode].end());
    std::sort(p99s[(size_t)mode].begin(),p99s[(size_t)mode].end());
    const double mean=std::accumulate(means[(size_t)mode].begin(),means[(size_t)mode].end(),0.)/means[(size_t)mode].size();
    const double p99=*std::max_element(p99s[(size_t)mode].begin(),p99s[(size_t)mode].end());
    double error=0,energy=0,maxDelta=0;
    const auto& original=first[0].audio;
    const auto& current=first[(size_t)mode].audio;
    if(original.size()!=current.size())throw std::runtime_error("mismatched output size");
    for(size_t i=0;i<original.size();i++){
     const double delta=(double)original[i]-(double)current[i];
     error+=delta*delta;energy+=(double)original[i]*original[i];
     maxDelta=std::max(maxDelta,std::abs(delta));
    }
    const double rms=std::sqrt(error/std::max(energy,1.e-20));
    const bool good=(maxDelta<=2.e-6&&rms<=1.e-6);
    allGood&=good;
    out<<sc.name<<','<<mode<<','<<mean<<','<<p99<<','<<peaks[(size_t)mode]<<','
       <<budget<<','<<100.*mean/budget<<','<<100.*p99/budget<<','
       <<overruns[(size_t)mode]<<','<<maxDelta<<','<<rms<<','
       <<original.size()<<','<<(good?1:0)<<"\n";
    std::cout<<sc.name<<" mode "<<mode<<" mean "<<mean<<"ms p99 "<<p99
             <<"ms audio max delta "<<maxDelta<<" quality "<<good<<"\n";
   }
  }
  if(!allGood){std::cerr<<"Grouped full-engine audio fidelity FAILED\n";return 4;}
  std::cout<<"Grouped full-engine 32-voice quality PASS, reference/scalar/SIMD\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 3;}
 return 0;
}
