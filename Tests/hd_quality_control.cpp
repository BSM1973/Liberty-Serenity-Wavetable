// WAVETABLE HD quality gate: offline deterministic numerical and realtime-budget checks.
// Hosted runner timing is diagnostic, not a guarantee of realtime DAW performance.
#include "SerenityDSP.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <new>
#include <thread>
#include <atomic>
#include <limits>
#if defined(_WIN32)
 #define NOMINMAX
 #include <windows.h>
 #include <psapi.h>
#elif defined(__APPLE__)
 #include <mach/mach.h>
 #include <sys/resource.h>
#else
 #include <sys/resource.h>
 #include <unistd.h>
#endif

using namespace serenity;
using Clock=std::chrono::steady_clock;
// Count ordinary C++ heap allocations on this thread only; does not count direct malloc.
static thread_local bool watchAlloc=false;
static thread_local size_t allocations=0;
void* operator new(size_t n){if(watchAlloc)++allocations;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,size_t)noexcept{std::free(p);}
void operator delete[](void* p,size_t)noexcept{std::free(p);}

struct Memory {double rss=0,peak=0;};
static Memory memory(){
 constexpr double mib=1024.*1024.;
#if defined(_WIN32)
 PROCESS_MEMORY_COUNTERS c{};
 if(GetProcessMemoryInfo(GetCurrentProcess(),&c,sizeof(c)))return {c.WorkingSetSize/mib,c.PeakWorkingSetSize/mib};
#elif defined(__APPLE__)
 mach_task_basic_info_data_t info{};mach_msg_type_number_t count=MACH_TASK_BASIC_INFO_COUNT;
 struct rusage r{};getrusage(RUSAGE_SELF,&r);
 if(task_info(mach_task_self(),MACH_TASK_BASIC_INFO,(task_info_t)&info,&count)==KERN_SUCCESS)return {info.resident_size/mib,r.ru_maxrss/mib};
#else
 struct rusage r{};getrusage(RUSAGE_SELF,&r);std::ifstream f("/proc/self/statm");long total=0,res=0;f>>total>>res;return {res*sysconf(_SC_PAGESIZE)/mib,r.ru_maxrss/1024.};
#endif
 return {};
}
static double elapsed(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
static double quantile(const std::vector<double>& sorted,double q){return sorted[(size_t)std::min<double>(sorted.size()-1,std::ceil(q*sorted.size())-1)];}
static int fail(std::ostream& out,const char* message){out<<"FAIL: "<<message<<std::endl;std::cerr<<message<<std::endl;return 1;}

int main(){
 std::ofstream csv("hd-quality.csv"),report("hd-quality-summary.txt");
 if(!csv||!report)return 2;
 report<<std::fixed<<std::setprecision(6);
 csv<<std::fixed<<std::setprecision(6);
 csv<<"bank,rate,block,voices,unison,rendered_seconds,mean_ms,p50_ms,p95_ms,p99_ms,max_ms,deadline_ms,mean_budget_pct,p99_budget_pct,overruns,blocks,allocations,rms,peak,rss_mib,peak_rss_mib\n";
 auto baseline=memory();report<<"Process initial RSS MiB: "<<baseline.rss<<" peak "<<baseline.peak<<"\n";
 auto engine=std::make_unique<Engine>();engine->prepare(48000);
 auto engineMem=memory();report<<"Legacy engine RSS MiB: "<<engineMem.rss<<"\n";
 std::array<HdExchange,4> exchanges;
 std::array<std::shared_ptr<const HdTable>,4> banks;
 size_t bytes=0;double buildMs=0,peakPreview=0,peakMorph=0;
 // Four independent maximum-size tables with distinct phase/amplitude and 3 harmonics.
 for(int o=0;o<4;o++){
  std::vector<float> pcm((size_t)hdMaxFrames*hdSamples);
  for(int f=0;f<hdMaxFrames;f++)for(int i=0;i<hdSamples;i++){
   double a=2*hdPi*i/hdSamples,amp=.2+.4*f/255.;
   pcm[(size_t)f*hdSamples+i]=(float)(amp*std::sin(a+o*.2)+.13*std::sin(3*a)+.06*std::sin(7*a));
  }
  auto start=Clock::now();auto copy=hdConvert(pcm.data(),(int)pcm.size(),hdSamples);
  if(copy!=pcm)return fail(report,"2048-sample source PCM changed");
  banks[o]=std::make_shared<const HdTable>(std::move(copy));buildMs+=elapsed(start);
  bytes+=(banks[o]->pcm.size()+(size_t)banks[o]->frames*hdMipStride())*sizeof(float);
  for(int f=0;f<hdMaxFrames;f++)for(int k=0;k<hdSamples;k+=17){
   float pos=f/255.f,phase=k/(float)hdSamples,expected=pcm[(size_t)f*hdSamples+k];
   peakPreview=std::max(peakPreview,(double)std::abs(banks[o]->preview(phase,pos)-expected));
   if(f<hdMaxFrames-1){float target=(expected+pcm[(size_t)(f+1)*hdSamples+k])*.5f;
    peakMorph=std::max(peakMorph,(double)std::abs(banks[o]->preview(phase,(f+.5f)/255.f)-target));}
  }
  exchanges[o].publish(banks[o]);engine->setHdTable(o,exchanges[o].consume());
 }
 auto loaded=memory();
 report<<"Four 256 x 2048 tables, raw+spectral payload MiB: "<<bytes/(1024.*1024.)<<"\n"
       <<"Four-bank conversion+FFT ms: "<<buildMs<<"\n"
       <<"RSS after four banks MiB: "<<loaded.rss<<" peak "<<loaded.peak<<"\n"
       <<"Exact-grid preview peak error: "<<peakPreview<<"\n"
       <<"Half-frame PCM morph peak error: "<<peakMorph<<"\n";
 if(peakPreview>2e-5||peakMorph>2e-5)return fail(report,"PCM or morph numerical error");
 // Compare safe harmonic reconstruction against analytic reference at low and mid notes.
 double error2=0,ref2=0,maxError=0;size_t samples=0;
 for(int o=0;o<4;o++)for(float pos:{0.f,.371f,1.f})for(float hz:{55.f,440.f,880.f})for(int i=0;i<2048;i++){
  double a=2*hdPi*i/2048.,amp=.2+.4*pos;
  double expected=amp*std::sin(a+o*.2)+.13*std::sin(3*a)+.06*std::sin(7*a);
  double error=banks[o]->sample(i/2048.f,pos,hz,48000.f)-expected;
  error2+=error*error;ref2+=expected*expected;maxError=std::max(maxError,std::abs(error));++samples;
 }
 const double rmsError=std::sqrt(error2/samples),snr=10*std::log10(ref2/std::max(1e-30,error2));
 report<<"Bandlimited analytic RMS error: "<<rmsError<<" max "<<maxError<<" SNR dB "<<snr<<"\n";
 // Mip interpolation uses linear phase interpolation, not an ideal sinc reconstruction.
 if(maxError>.006||snr<42)return fail(report,"bandlimited analytic fidelity");
 // FFT cap test: retained partial at low pitch, suppressed at high pitch.
 std::vector<float> high(hdSamples);for(int i=0;i<hdSamples;i++)high[i]=(float)(.8*std::sin(2*hdPi*300*i/hdSamples));
 HdTable probe(std::move(high));double low=0,alias=0;
 for(int i=0;i<2048;i++){float p=i/2048.f;double a=probe.sample(p,0,30,48000),b=probe.sample(p,0,1000,48000);low+=a*a;alias+=b*b;}
 double reject=10*std::log10(std::max(alias,1e-30)/std::max(low,1e-30));report<<"Rejected partial relative power dB: "<<reject<<"\n";
 if(low<100||reject>-60)return fail(report,"high partial rejection");
 bool realtime=true;size_t cases=0;
 // Comparable legacy/HD workloads; 32 MIDI notes, four active generators.
 // Timing render+parameter update+pointer publication; excludes test scans and DAW overhead.
 auto run=[&](bool hd,int rate,int block,int voices,int unison,double seconds)->bool{
  engine->prepare(rate);
  Params p;p.polyphony=voices;p.attack=.001f;p.decay=.01f;p.sustain=.8f;p.release=.025f;
  p.delay=.12f;p.reverb=.12f;p.modFxMix=.12f;p.compMix=.2f;
  for(int o=0;o<4;o++){p.oscLevel[o]=.18f;p.wave[o]=waveCount;p.unison[o]=unison;engine->setHdTable(o,hd?exchanges[o].consume():nullptr);}
  engine->setParams(p);
  for(int n=0;n<voices;n++)engine->noteOn(36+n,.72f);
  std::vector<float> l((size_t)block),r((size_t)block);
  for(int i=0;i<8;i++){std::fill(l.begin(),l.end(),0);std::fill(r.begin(),r.end(),0);engine->render(l.data(),r.data(),block);}
  const int blocks=std::max(32,(int)std::ceil(seconds*rate/block));const double budget=1000.*block/rate;
  std::vector<double> times;times.reserve((size_t)blocks);double total=0,energy=0,peak=0;size_t overruns=0;allocations=0;
  for(int b=0;b<blocks;b++){
   std::fill(l.begin(),l.end(),0);std::fill(r.begin(),r.end(),0);
   p.position=(float)(.5+.49*std::sin(b*.031));
   watchAlloc=true;auto start=Clock::now();
   engine->setParams(p);for(int o=0;o<4;o++)engine->setHdTable(o,hd?exchanges[o].consume():nullptr);
   engine->render(l.data(),r.data(),block);double ms=elapsed(start);watchAlloc=false;
   times.push_back(ms);total+=ms;overruns+=(ms>budget);
   if(engine->numVoices()!=voices)return false;
   for(int i=0;i<block;i++){if(!std::isfinite(l[i])||!std::isfinite(r[i]))return false;energy+=(double)l[i]*l[i]+(double)r[i]*r[i];peak=std::max(peak,(double)std::max(std::abs(l[i]),std::abs(r[i])));}
  }
  std::sort(times.begin(),times.end());double avg=total/blocks,p99=quantile(times,.99);auto mem=memory();
  csv<<(hd?"HD":"legacy")<<','<<rate<<','<<block<<','<<voices<<','<<unison<<','<<(double)blocks*block/rate<<','<<avg<<','<<quantile(times,.5)<<','<<quantile(times,.95)<<','<<p99<<','<<times.back()<<','<<budget<<','<<100*avg/budget<<','<<100*p99/budget<<','<<overruns<<','<<blocks<<','<<allocations<<','<<std::sqrt(energy/(2.*blocks*block))<<','<<peak<<','<<mem.rss<<','<<mem.peak<<'\n';csv.flush();
  if(hd&&overruns)realtime=false;
  ++cases;
  engine->allNotesOff();for(int i=0;i<64&&engine->numVoices()>0;i++){std::fill(l.begin(),l.end(),0);std::fill(r.begin(),r.end(),0);engine->render(l.data(),r.data(),block);}
  return allocations==0&&energy>1e-5&&peak<=1.001&&engine->numVoices()==0;
 };
 for(int rate:{44100,48000,96000})for(int block:{64,256,512}){
  for(bool hd:{false,true})if(!run(hd,rate,block,32,1,.4))return fail(report,"32-note render or allocation check");
 }
 for(bool hd:{false,true})if(!run(hd,48000,256,32,4,2.))return fail(report,"32-note unison-4 sustained render");
 for(bool hd:{false,true})if(!run(hd,48000,128,32,16,.5))return fail(report,"maximum unison stress");
 if(!run(true,48000,256,32,1,10.))return fail(report,"10-second HD sustained render");
 // Concurrent publications: audio consumes immutable front while producer rotates slots.
 std::atomic<bool> producerDone{false},bad{false};std::atomic<int> observed{0};
 auto& x=exchanges[0];x.publish(banks[0]);
 std::thread writer([&]{for(int n=0;n<20000;n++)x.publish(banks[n%4]);producerDone.store(true,std::memory_order_release);});
 do {auto* front=x.consume();if(!front||front->frames!=256||!std::isfinite(front->sample(.123f,.541f,440,48000)))bad=true;else ++observed;}while(!producerDone.load(std::memory_order_acquire));
 writer.join();if(bad||observed==0)return fail(report,"concurrent HD publication");
 report<<"Concurrent publications: 20000; observations: "<<observed.load()<<"\n";
 auto end=memory();report<<"Final RSS MiB: "<<end.rss<<" peak "<<end.peak<<"\n";
 report<<"Functional quality gate: PASS, "<<cases<<" render cases, no ordinary C++ allocations in measured path.\n";
 report<<"Hosted-runner realtime budgets: "<<(realtime?"no observed HD overrun":"one or more HD overruns: REVIEW REQUIRED")<<"\n";
 report<<"Timing is not a DAW certification; inspect CSV, test physical audio hardware and listen.\n";
 report.flush();std::cout<<"HD QUALITY: "<<cases<<" cases; realtime budgets "<<(realtime?"PASS":"REVIEW")<<"; reports saved\n";
 return 0;
}
