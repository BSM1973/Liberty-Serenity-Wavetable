#include "SerenityDSP.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
static void w16(std::ofstream &f,uint16_t v){f.put((char)(v&255));f.put((char)(v>>8));}
static void w32(std::ofstream &f,uint32_t v){w16(f,(uint16_t)(v&65535));w16(f,(uint16_t)(v>>16));}
int main(int argc,char **argv){
 if(argc<2){std::cerr<<"Usage: serenity_render_demo output.wav\n";return 2;}
 constexpr int sr=48000, segment=sr*2, count=6;
 serenity::Engine synth;synth.prepare(sr);
 const int indices[]={0,12,27,37,49,75};
 const int pitches[]={36,60,48,72,60,48};
 std::vector<int16_t> pcm;pcm.reserve((size_t)segment*count*2);
 for(int j=0;j<count;j++){
  synth.allNotesOff();synth.setParams(serenity::factoryPresets()[(size_t)indices[j]].p);
  synth.noteOn(pitches[j],.7f);
  for(int start=0;start<segment;start+=256){
   float l[256]{},r[256]{};
   if(start>=sr&&start<sr+256)synth.noteOff(pitches[j]);
   int n=std::min(256,segment-start);
   synth.render(l,r,n);
   for(int k=0;k<n;k++){
    pcm.push_back((int16_t)(serenity::clamp(l[k]*.75f,-.999f,.999f)*32767.f));
    pcm.push_back((int16_t)(serenity::clamp(r[k]*.75f,-.999f,.999f)*32767.f));
   }
  }
 }
 std::ofstream f(argv[1],std::ios::binary);if(!f)return 2;
 const uint32_t dataBytes=(uint32_t)(pcm.size()*2);
 f.write("RIFF",4);w32(f,36+dataBytes);f.write("WAVEfmt ",8);
 w32(f,16);w16(f,1);w16(f,2);w32(f,sr);w32(f,sr*4);w16(f,4);w16(f,16);
 f.write("data",4);w32(f,dataBytes);
 f.write(reinterpret_cast<const char*>(pcm.data()),dataBytes);
 return f?0:1;
}
