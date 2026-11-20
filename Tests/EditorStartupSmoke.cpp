#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <memory>
#include <cmath>

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
 processor.setCustomPoint(1,7,511,-.37f);
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
 processor.setSequenceMode(0);
 auto editor=std::make_unique<SerenityEditor>(processor);
 if(!editor->hasTenOriginalScreens()){
  std::cerr<<"ERROR: original 2048x1361 reference screenshots missing"<<std::endl;
  return 10;
 }
 if(editor->getWidth()<900 || editor->getHeight()<590){
  std::cerr<<"ERROR: impractical UI dimensions"<<std::endl;
  return 11;
 }
 for(int tab=0;tab<10;++tab){
  // Render actual sounding sequencer and arpeggiator states on original skins.
  if(tab==5)processor.setSequenceMode(1);
  if(tab==6)processor.setSequenceMode(2);
  editor->selectTabForSmokeTest(tab);
  auto snapshot=juce::Image(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);
  juce::Graphics painter(snapshot);
  editor->paint(painter);
  if(snapshot.isNull()) return 12;
  if(tab==0 || tab==1 || tab==5 || tab==6 || tab==8 || tab==9){
   auto destination=juce::File::getCurrentWorkingDirectory().getChildFile(
       juce::String("Serenity-UI-Preview-")+juce::String(tab)+".png");
   juce::PNGImageFormat png;
   auto stream=std::unique_ptr<juce::FileOutputStream>(destination.createOutputStream());
   if(stream)png.writeImageToStream(snapshot,*stream);
  }
 }
 editor.reset();
 processor.releaseResources();
 std::cout<<"SERENITY_TEN_ORIGINAL_SCREENS_EDITOR_SMOKE_OK"<<std::endl;
 return 0;
}
