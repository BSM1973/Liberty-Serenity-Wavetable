#pragma once
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <array>
#include <vector>

class SerenityEditor final: public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit SerenityEditor(SerenityAudioProcessor&);
 ~SerenityEditor() override = default;
 void paint(juce::Graphics&) override;
 void resized() override;
 // Regression checks, not a replacement UI: every tab uses an original screenshot.
 bool hasTenOriginalScreens() const;
 void selectTabForSmokeTest(int index);
 void mouseDown(const juce::MouseEvent&) override;
 void mouseDrag(const juce::MouseEvent&) override;
 void mouseUp(const juce::MouseEvent&) override;
 void mouseDoubleClick(const juce::MouseEvent&) override;
 void mouseMove(const juce::MouseEvent&) override;
 void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
private:
 struct Hotspot {float x,y,r;int parameter;};
 SerenityAudioProcessor& synth;
 std::array<juce::Image,10> skins;
 juce::TextEditor search;
 std::unique_ptr<juce::FileChooser> fileChooser;
 int tab=0,category=0,listOffset=0,matrixPage=0,selectedParameter=-1;
 int draggedParameter=-1,selectedOsc=0;
 int sequencePage=0,sequenceDragLane=-1;
 float startValue=0;
 juce::Point<float> downPos;
 juce::String status;
 int statusCountdown=0;
 std::vector<Hotspot> hotspots() const;
 int paramAt(juce::Point<float>)const;
 juce::Point<float> logical(juce::Point<float>) const;
 void timerCallback() override;
 void paintTop(juce::Graphics&);
 void paintSettings(juce::Graphics&);
 void paintPresets(juce::Graphics&);
 void paintMatrix(juce::Graphics&);
 void paintOscillators(juce::Graphics&);
 void paintEditor(juce::Graphics&);
 void paintSequencer(juce::Graphics&);
 bool editSequenceAt(juce::Point<float>,int lane);
 bool sequenceClick(juce::Point<float>);
 void showSequenceRateMenu();
 void showSequenceBankMenu();
 void showRouteMenu(int slot,bool source);
 void paintInteraction(juce::Graphics&);
 void paintTabTitle(juce::Graphics&,int,float,float,const char*,const char*,float=220);
 void drawText(juce::Graphics&,const juce::String&,float,float,float,float,
               juce::Colour color=juce::Colour(0xffaabbd0),float size=14,bool bold=false);
 juce::String tr(const char* french,const char* english)const;
 void selectPreset(int);
 void showParameterMenu(int);
 void showStatus(const juce::String&);
 juce::Rectangle<float> scaleRect(float,float,float,float) const;
 std::vector<int> filteredPresetIndices() const;
 void changeLanguage();
 void savePatch(bool import);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerenityEditor)
};
