#pragma once
#include <JuceHeader.h>
#include "DSP/SerenityDSP.h"
#include <array>
#include <atomic>

namespace serenity {
// APVTS index: stable IDs matter for host automation and MIDI learn persistence.
enum Parameter : int {
 master, cutoff, resonance, position, attack, decay, sustain, release,
 detune, spread, delayMix, reverbMix, drive, lfoRate, lfoDepth,
 oscALevel, oscBLevel, oscCLevel, oscDLevel,
 oscAWave, oscBWave, oscCWave, oscDWave,
 oscAUnison, oscBUnison, oscCUnison, oscDUnison,
 polyphony,filter2Cutoff,filter2Resonance,filter2Mix,filter2Enabled,filterRouting,
 macroBrightness, macroMovement, macroWarmth, macroSpace,
 macroWidth, macroTexture, macroEnergy, macroSerenity,
 parameterCount
};
struct ParamSpec { const char *id; float lo, hi, initial; };
extern const std::array<ParamSpec,parameterCount> specs;
}

class SerenityAudioProcessor final : public juce::AudioProcessor {
public:
 SerenityAudioProcessor();
 ~SerenityAudioProcessor() override = default;
 void prepareToPlay(double sampleRate,int samplesPerBlock) override;
 void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "Serenity WAVETABLE";}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 3.0;}
 int getNumPrograms() override {return (int)serenity::factoryPresets().size();}
 int getCurrentProgram() override {return selectedPreset.load();}
 void setCurrentProgram(int index) override {applyPreset(index);}
 const juce::String getProgramName(int index) override;
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;

 juce::AudioProcessorValueTreeState apvts;
 float getValue(int index) const noexcept;
 void setValue(int index,float physicalValue);
 void applyPreset(int index);
 void armMidiLearn(int parameter) noexcept;
 void clearMapping(int parameter) noexcept;
 int mappedCC(int parameter) const noexcept;
 int currentlyLearning() const noexcept {return learnTarget.load();}
 int lastLearnedCC() const noexcept {return lastCC.load();}
 int currentPreset() const noexcept {return selectedPreset.load();}
 bool french() const noexcept {return frenchUI.load();}
 void setFrench(bool enabled) noexcept {frenchUI.store(enabled);}
 int getVoiceCount() const noexcept {return voiceCount.load();}
 float getOutputLevel() const noexcept {return outputMeter.load();}
 serenity::ModRoute getRoute(int slot) const noexcept;
 void setRoute(int slot,serenity::ModRoute next) noexcept;
 bool isFavorite(int preset) const noexcept;
 void toggleFavorite(int preset) noexcept;
 static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
 serenity::Engine engine;
 std::array<std::atomic<int>,serenity::parameterCount> controller{};
 std::array<std::atomic<float>,serenity::parameterCount> midiOverride{};
 std::atomic<int> learnTarget{-1},lastCC{-1},selectedPreset{0},voiceCount{0};
 std::atomic<float> outputMeter{0.f};
 std::atomic<bool> frenchUI{true};
 std::array<std::atomic<int>,64> modSources{},modDestinations{};
 std::array<std::atomic<float>,64> modAmounts{};
 std::array<std::atomic<bool>,100> favorites{};
 serenity::Params buildParams() const noexcept;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerenityAudioProcessor)
};
