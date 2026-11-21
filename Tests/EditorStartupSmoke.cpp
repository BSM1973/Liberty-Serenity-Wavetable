#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <memory>
#include <cmath>
#include <cstdint>
#include <array>

int main(){
 juce::ScopedJuceInitialiser_GUI gui;
 SerenityAudioProcessor processor;
 processor.prepareToPlay(48000,256);
 // Simulate real MIDI input through the complete JUCE processor and 64-step engine.
 processor.setSequenceMode(2);
 processor.setSequenceLength(8);
 processor.setSequenceRate(16);
 processor.setSequenceGate(.6f);
 processor.setSequenceStep(0,0,{12,.8f,.3f});
 juce::AudioBuffer<float> audio(2,256);
 juce::MidiBuffer midi;
 midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);
 double energy=0.0;
 for(int block=0;block<40;block++){
  audio.clear();
  processor.processBlock(audio,midi);
  midi.clear();
  for(int i=0;i<audio.getNumSamples();i++){
   const float x=audio.getSample(0,i),y=audio.getSample(1,i);
   if(!std::isfinite(x)||!std::isfinite(y))return 20;
   energy+=x*x+y*y;
  }
 }
 if(energy<1.e-6 || processor.getSequencePlayingStep()<1){
  std::cerr<<"ERROR: Sequencer generated no valid audible steps"<<std::endl;
  return 21;
 }
 // Verify the custom wavetable is a real audio generator, not a static UI drawing.
 processor.setSequenceMode(0);
 processor.setValue(serenity::oscAWave,(float)serenity::waveCount);
 std::array<float,serenity::editableSamples> smoothWave{};
 for(int i=0;i<serenity::editableSamples;i++)
  smoothWave[(size_t)i]=.73f*std::sin(6.2831853071795864769f*(float)i/serenity::editableSamples);
 processor.commitCustomFrame(0,4,smoothWave);
 const float realFrame=processor.getWavetablePreview(0,serenity::waveCount,4.f/7.f,.25f,false);
 if(std::abs(realFrame-.73f)>.002f){
  std::cerr<<"ERROR: native preview does not read real custom wavetable PCM"<<std::endl;
  return 31;
 }
 processor.setValue(serenity::oscAWarp,.72f);
 processor.setValue(serenity::oscAWarpMode,1.f);
 processor.setValue(serenity::oscAShape,.55f);
 processor.setValue(serenity::oscAShapeMode,1.f);
 processor.setValue(serenity::oscAOctave,1.f);
 processor.setValue(serenity::oscASemi,-2.f);
 processor.setValue(serenity::oscAFine,15.f);
 processor.setValue(serenity::oscAPan,.55f);
 const float modifiedFrame=processor.getWavetablePreview(0,serenity::waveCount,4.f/7.f,.25f,true);
 if(!std::isfinite(modifiedFrame)||std::abs(modifiedFrame-realFrame)<.03f){
  std::cerr<<"ERROR: WARP/SHAPE controls do not transform the real rendered waveform"<<std::endl;
  return 32;
 }
 processor.setCustomPoint(1,7,511,-.37f);
 processor.rebuildCustomFrame(1,7); // Publish the completed audio-safe mip snapshot.
 std::array<float,serenity::editableSamples> quietSine{};
 for(int i=0;i<serenity::editableSamples;i++)
  quietSine[(size_t)i]=.28f*std::sin(6.2831853071795864769f*(float)i/serenity::editableSamples);
 processor.commitCustomFrame(2,3,quietSine);
 processor.setCustomHarmonic(2,3,5,.21f);
 const auto partials=processor.getCustomHarmonics(2,3);
 if(std::abs(partials[4]-.21f)>.025f){
   std::cerr<<"ERROR: FFT harmonic editor did not update real wavetable samples"<<std::endl;
   return 26;
 }
 audio.clear();
 midi.clear();
 midi.addEvent(juce::MidiMessage::noteOn(1,72,(juce::uint8)110),0);
 processor.processBlock(audio,midi);
 double customEnergy=0;
 for(int n=0;n<audio.getNumSamples();n++){
  const float value=audio.getSample(0,n);
  if(!std::isfinite(value))return 23;
  customEnergy+=value*value;
 }
 if(customEnergy<1.e-9)return 24;
 // Verify all 64 steps in all four banks can survive AU/VST3 project state roundtrips.
 processor.setSequenceMode(2);
 processor.setSequenceBank(3);
 processor.setSequenceLength(64);
 processor.setSequenceStep(3,63,{-17,.42f,-.65f});
 juce::MemoryBlock savedState;
 processor.getStateInformation(savedState);
 SerenityAudioProcessor restored;
 restored.setStateInformation(savedState.getData(),(int)savedState.getSize());
 const auto last=restored.getSequenceStep(3,63);
 if(restored.getSequenceBank()!=3 || restored.getSequenceLength()!=64 ||
    restored.getSequenceMode()!=2 || last.pitch!=-17 ||
    std::abs(last.velocity-.42f)>.001f || std::abs(last.modulation+.65f)>.001f) {
  std::cerr<<"ERROR: Sequencer state did not survive project reload"<<std::endl;
  return 22;
 }
 if(std::abs(restored.getCustomPoint(0,4,128)-.73f)>.00001f ||
    std::abs(restored.getCustomPoint(1,7,511)+.37f)>.00001f ||
    (int)std::lround(restored.getValue(serenity::oscAWave))!=serenity::waveCount){
   std::cerr<<"ERROR: user wavetable frames did not survive project save/load"<<std::endl;
   return 25;
 }
 if(std::abs(restored.getCustomHarmonics(2,3)[4]-.21f)>.025f){
   std::cerr<<"ERROR: FFT harmonic editing lost after preset/project reload"<<std::endl;
   return 27;
 }
 if(std::abs(restored.getValue(serenity::oscAWarp)-.72f)>.001f||
    std::abs(restored.getValue(serenity::oscAShape)-.55f)>.001f||
    std::abs(restored.getValue(serenity::oscAOctave)-1.f)>.001f||
    std::abs(restored.getValue(serenity::oscAPan)-.55f)>.001f){
  std::cerr<<"ERROR: native oscillator controls lost after AU/VST3 project reload"<<std::endl;
  return 33;
 }
 processor.setSequenceMode(0);
 auto editor=std::make_unique<SerenityEditor>(processor);
 // Native JUCE renderer is the only visual source: no skin PNG dependency.
 // Compare pixels after changing DSP parameters; a flattened screenshot would fail.
 auto pixelDigest=[](const juce::Image& img)->std::uint64_t {
   std::uint64_t digest=1469598103934665603ULL;
   for(int y=2;y<img.getHeight();y+=5)
    for(int x=2;x<img.getWidth();x+=7){
     const std::uint64_t argb=(std::uint64_t)img.getPixelAt(x,y).getARGB();
     digest=(digest^argb)*1099511628211ULL;
    }
   return digest;
 };
 if(editor->getWidth()<900 || editor->getHeight()<590){
  std::cerr<<"ERROR: impractical UI dimensions"<<std::endl;
  return 11;
 }
 std::array<std::uint64_t,10> pageDigests{};
 for(int tab=0;tab<10;++tab){
  // Render actual sounding sequencer and arpeggiator states on original skins.
  if(tab==5)processor.setSequenceMode(1);
  if(tab==6)processor.setSequenceMode(2);
  editor->selectTabForSmokeTest(tab);
  auto snapshot=juce::Image(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);
  {
   juce::Graphics painter(snapshot);
   editor->paint(painter);
  } // Flush platform-specific Graphics state before reading pixels on Windows.
  if(snapshot.isNull()) return 12;
  pageDigests[(size_t)tab]=pixelDigest(snapshot);
  if(pageDigests[(size_t)tab]==0)return 13;
  if(tab==0){
   const auto baseline=pageDigests[0];
   const float original=processor.getValue(serenity::oscALevel);
   const float target=original>=.5f?0.f:1.f;
   processor.setValue(serenity::oscALevel,target);
   if(std::abs(processor.getValue(serenity::oscALevel)-original)<.20f){
    std::cerr<<"ERROR: OSC A parameter did not change on this platform"<<std::endl;
    return 30;
   }
   auto changed=juce::Image(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);
   {
    juce::Graphics other(changed);
    editor->paint(other);
   } // Finish the rasterizer before measuring a deterministic pixel checksum.
   if(pixelDigest(changed)==baseline){
    std::cerr<<"ERROR: editing OSC A did not alter native rendered UI"<<std::endl;
    return 28;
   }
   processor.setValue(serenity::oscALevel,original);
  }
  if(tab==0 || tab==1 || tab==5 || tab==6 || tab==8 || tab==9){
   auto destination=juce::File::getCurrentWorkingDirectory().getChildFile(
       juce::String("Serenity-UI-Preview-")+juce::String(tab)+".png");
   juce::PNGImageFormat png;
   auto stream=std::unique_ptr<juce::FileOutputStream>(destination.createOutputStream());
   if(stream)png.writeImageToStream(snapshot,*stream);
  }
 }
 for(int a=0;a<10;a++)for(int b=a+1;b<10;b++)
  if(pageDigests[(size_t)a]==pageDigests[(size_t)b]){
   std::cerr<<"ERROR: two native pages have identical renders: "<<a<<","<<b<<std::endl;
   return 29;
  }
 editor.reset();
 processor.releaseResources();
 std::cout<<"SERENITY_TEN_NATIVE_UI_PAGES_AND_LIVE_CONTROLS_OK"<<std::endl;
 return 0;
}
