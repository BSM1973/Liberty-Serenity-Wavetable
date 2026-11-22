#include "SerenityHdWavetable.h"
#include <iostream>
#include <limits>
using namespace serenity;
int main(){
 // A different amplitude at each of the 256 original frames.
 std::vector<float> source((size_t)hdMaxFrames*hdSamples);
 for(int f=0;f<hdMaxFrames;f++)for(int i=0;i<hdSamples;i++)
  source[(size_t)f*hdSamples+i]=(.15f+.7f*f/255.f)*std::sin(2*hdPi*i/hdSamples);
 auto pcm=hdConvert(source.data(),(int)source.size(),hdSamples);
 if(pcm!=source)return 1;
 auto bank=std::make_shared<const HdTable>(std::move(pcm));
 if(bank->frames!=256)return 2;
 for(int f=0;f<256;f++){
  float p=f/255.f,expected=.15f+.7f*p;
  if(std::abs(bank->preview(.25f,p)-expected)>1e-5f)return 3;
  if(std::abs(bank->sample(.25f,p,440,48000)-expected)>.001f)return 4;
 }
 if(std::abs(bank->sample(.25f,127.5f/255.f,440,48000)-.5f)>.001f)return 5;
 if(!hdConvert(source.data(),(int)source.size(),1024).empty())return 6;
 auto invalid=source;invalid[5]=std::numeric_limits<float>::quiet_NaN();if(!hdConvert(invalid.data(),(int)invalid.size(),hdSamples).empty())return 7;
 // A high harmonic is removed when its frequency would cross Nyquist.
 if(hdMipStride()!=4352||hdMipOffset(10)!=4288||hdMipLength(10)!=64)return 13;
 std::vector<float> high(hdSamples);
 for(int i=0;i<hdSamples;i++)high[i]=.8f*std::sin(2*hdPi*300*i/hdSamples);
 HdTable harmonics(high);
 double lowEnergy=0,highEnergy=0;
 for(int i=0;i<2048;i++){float phase=i/2048.f;
  float a=harmonics.sample(phase,0,40,48000),b=harmonics.sample(phase,0,1000,48000);lowEnergy+=a*a;highEnergy+=b*b;
 }
 if(lowEnergy<100||highEnergy>.001)return 8;
 // Exact source cycles resampled to 2048; fundamental remains calibrated.
 std::vector<float> shortCycle(512);
 for(int i=0;i<512;i++)shortCycle[i]=.7f*std::sin(2*hdPi*i/512);
 auto converted=hdConvert(shortCycle.data(),512,512);if(converted.size()!=2048||std::abs(converted[512]-.7f)>.001f)return 9;
 std::array<HdExchange,4> slots;
 for(int i=0;i<4;i++){auto data=source;for(float& v:data)v*=1.f-.1f*i;slots[i].publish(std::make_shared<const HdTable>(std::move(data)));}
 for(int i=0;i<4;i++){auto* p=slots[i].consume();if(!p||p->frames!=256||std::abs(p->preview(.25f,1)-.85f*(1.f-.1f*i))>.001f)return 10;}
 // Rapid publications must not invalidate the current audio front.
 auto* old=slots[0].consume();slots[0].publish(bank);slots[0].publish({});slots[0].publish(bank);
 if(std::abs(old->preview(.25f,1)-.85f)>.001f||slots[0].consume()!=bank.get())return 11;
 slots[0].publish({});if(slots[0].consume()!=nullptr||slots[1].consume()==nullptr)return 12;
 // Compare the optimized unison path to the original complete interpolation.
 for(int n=0;n<24000;n++){
  const float position=(float)((n*179u)%10007u)/10006.f;
  const float phase=((n*313u)%19111u)/19111.f*5.f-2.f;
  const float hz=20.f+(float)((n*101u)%12000u);
  const float rate=(n%3==0)?44100.f:(n%3==1)?48000.f:96000.f;
  const auto prepared=bank->prepareFrame(position);
  const float original=bank->sample(phase,position,hz,rate);
  const float optimized=bank->samplePrepared(phase,prepared,hz,rate);
  if(!std::isfinite(original)||!std::isfinite(optimized)||std::abs(original-optimized)>2.e-6f){
   std::cerr<<"HD prepared-frame interpolation differs: "<<n<<" "<<original<<" "<<optimized<<"\n";
   return 14;
  }
 }
 if(bank->samplePrepared(.2f,bank->prepareFrame(-1.f),0.f,48000.f)!=0.f)return 15;
 std::cout<<"HD: 256 x 2048, 4 banks, morph, anti-alias, resampling, publication OK\n";
 return 0;
}
