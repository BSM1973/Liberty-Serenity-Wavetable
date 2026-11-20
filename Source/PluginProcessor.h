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
 // The sequencer uses atomics in the editor, copied into a fixed-size DSP snapshot per block.
 serenity::SequenceStep getSequenceStep(int bank,int index) const noexcept;
 void setSequenceStep(int bank,int index,serenity::SequenceStep step) noexcept;
 void setSequenceMode(int mode) noexcept;
 int getSequenceMode() const noexcept {return sequenceMode.load();}
 int getSequenceBank() const noexcept {return sequenceBank.load();}
 void setSequenceBank(int bank) noexcept {sequenceBank.store(juce::jlimit(0,3,bank));}
 int getSequenceLength() const noexcept {return sequenceLength.load();}
 void setSequenceLength(int length) noexcept {sequenceLength.store(juce::jlimit(1,64,length));}
 int getSequenceRate() const noexcept {return sequenceRate.load();}
 void setSequenceRate(int rate) noexcept;
 float getSequenceGate() const noexcept {return sequenceGate.load();}
 void setSequenceGate(float gate) noexcept {sequenceGate.store(juce::jlimit(0.f,1.f,gate));}
 float getSequenceSwing() const noexcept {return sequenceSwing.load();}
 void setSequenceSwing(float swing) noexcept {sequenceSwing.store(juce::jlimit(0.f,.5f,swing));}
 int getSequencePlayingStep() const noexcept {return sequencePlayingStep.load();}
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
 std::array<std::array<std::atomic<int>,serenity::sequenceMaxSteps>,serenity::sequenceBanks> sequencePitch{};
 std::array<std::array<std::atomic<float>,serenity::sequenceMaxSteps>,serenity::sequenceBanks> sequenceVelocity{},sequenceModulation{};
 std::atomic<int> sequenceMode{0},sequenceBank{0},sequenceLength{8},sequenceRate{16},sequencePlayingStep{-1};
 std::atomic<float> sequenceGate{.60f},sequenceSwing{0.f};
 serenity::Params buildParams() const noexcept;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerenityAudioProcessor)
};
