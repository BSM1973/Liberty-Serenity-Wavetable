#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <memory>

int main(){
 juce::ScopedJuceInitialiser_GUI gui;
 SerenityAudioProcessor processor;
 processor.prepareToPlay(48000,256);
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
  editor->selectTabForSmokeTest(tab);
  auto snapshot=juce::Image(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);
  juce::Graphics painter(snapshot);
  editor->paint(painter);
  if(snapshot.isNull()) return 12;
  if(tab==0 || tab==1 || tab==8 || tab==9){
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
