#pragma once
#include <JuceHeader.h>
#include "DSP/SerenityDSP.h"
#include "DSP/SerenitySpectralEditor.h"
#include <array>
#include <atomic>
#include <memory>

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
 // New parameters are appended: all existing parameter IDs and indices stay stable.
 oscAOctave,oscBOctave,oscCOctave,oscDOctave,
 oscASemi,oscBSemi,oscCSemi,oscDSemi,
 oscAFine,oscBFine,oscCFine,oscDFine,
 oscAPan,oscBPan,oscCPan,oscDPan,
 oscAWarp,oscBWarp,oscCWarp,oscDWarp,
 oscAShape,oscBShape,oscCShape,oscDShape,
 oscAWarpMode,oscBWarpMode,oscCWarpMode,oscDWarpMode,
 oscAShapeMode,oscBShapeMode,oscCShapeMode,oscDShapeMode,
 // v0.9.2: appended; older host automation IDs remain stable.
 oscABlend,oscBBlend,oscCBlend,oscDBlend,
 oscAPhase,oscBPhase,oscCPhase,oscDPhase,
 oscARandPhase,oscBRandPhase,oscCRandPhase,oscDRandPhase,
 // v0.10: append only; existing automation IDs remain stable.
 fmBA,
 fmCA,
 fmCB,
 ringAmount,
 fmMode,
 ringMode,
 env1Delay,
 env1Hold,
 env2Delay,
 env2Attack,
 env2Hold,
 env2Decay,
 env2Sustain,
 env2Release,
 env3Delay,
 env3Attack,
 env3Hold,
 env3Decay,
 env3Sustain,
 env3Release,
 env4Delay,
 env4Attack,
 env4Hold,
 env4Decay,
 env4Sustain,
 env4Release,
 env5Delay,
 env5Attack,
 env5Hold,
 env5Decay,
 env5Sustain,
 env5Release,
 env6Delay,
 env6Attack,
 env6Hold,
 env6Decay,
 env6Sustain,
 env6Release,
 lfo1Shape,
 lfo1Phase,
 lfo1Unipolar,
 lfo2Rate,
 lfo2Shape,
 lfo2Phase,
 lfo2Unipolar,
 lfo3Rate,
 lfo3Shape,
 lfo3Phase,
 lfo3Unipolar,
 lfo4Rate,
 lfo4Shape,
 lfo4Phase,
 lfo4Unipolar,
 lfo5Rate,
 lfo5Shape,
 lfo5Phase,
 lfo5Unipolar,
 lfo6Rate,
 lfo6Shape,
 lfo6Phase,
 lfo6Unipolar,
 lfo7Rate,
 lfo7Shape,
 lfo7Phase,
 lfo7Unipolar,
 lfo8Rate,
 lfo8Shape,
 lfo8Phase,
 lfo8Unipolar,
 delayTime,
 delayFeedback,
 delayTone,
 reverbFeedback,
 masterWidth,
 tremRate,
 tremDepth,
 tremMode,
 fxRingFreq,
 fxRingMix,
 env1Curve,
 env1Velocity,
 env2Curve,
 env2Velocity,
 env3Curve,
 env3Velocity,
 env4Curve,
 env4Velocity,
 env5Curve,
 env5Velocity,
 env6Curve,
 env6Velocity,
 lfo1Sync,
 lfo1Division,
 lfo1Smooth,
 lfo2Sync,
 lfo2Division,
 lfo2Smooth,
 lfo3Sync,
 lfo3Division,
 lfo3Smooth,
 lfo4Sync,
 lfo4Division,
 lfo4Smooth,
 lfo5Sync,
 lfo5Division,
 lfo5Smooth,
 lfo6Sync,
 lfo6Division,
 lfo6Smooth,
 lfo7Sync,
 lfo7Division,
 lfo7Smooth,
 lfo8Sync,
 lfo8Division,
 lfo8Smooth,
 modFxType,
 modFxRate,
 modFxDepth,
 modFxFeedback,
 modFxMix,
 eqLowGain,
 eqLowFreq,
 eqMidGain,
 eqMidFreq,
 eqMidQ,
 eqHighGain,
 eqHighFreq,
 compThreshold,
 compRatio,
 compAttack,
 compRelease,
 compMakeup,
 compMix,
 filter1Type,
 filter2Type,
 filter1Enabled,
 filter1Drive,
 filter2Drive,
 filter1Env,
 filter2Env,
 filter1Key,
 filter2Key,
 filter1Vel,
 filter2Vel,
 filter1Mix,
 reverbType,
 reverbDamping,
 reverbSize,
 reverbPreDelay,
 reverbLowCut,
 reverbWidth,
 env1Loop,
 env2Loop,
 env3Loop,
 env4Loop,
 env5Loop,
 env6Loop,
 lfo1Fade,
 lfo2Fade,
 lfo3Fade,
 lfo4Fade,
 lfo5Fade,
 lfo6Fade,
 lfo7Fade,
 lfo8Fade,
 parameterCount
};
struct ParamSpec { const char *id; float lo, hi, initial; };
extern const std::array<ParamSpec,parameterCount> specs;
inline int envParameter(int env,int control) noexcept {
 constexpr int ampIds[]={env1Delay,attack,env1Hold,decay,sustain,release};
 return env==0?ampIds[control]:env2Delay+(env-1)*6+control;
}
inline int lfoParameter(int lfo,int control) noexcept {
 constexpr int first[]={lfoRate,lfo1Shape,lfo1Phase,lfo1Unipolar};
 return lfo==0?first[control]:lfo2Rate+(lfo-1)*4+control;
}
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
 // Editable per-oscillator frames. GUI commits are atomic; DSP takes a consistent
 // bounded snapshot at audio block boundaries without heap allocations.
 void setCustomPoint(int oscillator,int frame,int sample,float value) noexcept;
 float getCustomPoint(int oscillator,int frame,int sample) const noexcept;
 // Read-only live preview: exact factory frames / editable samples.
 // No screenshot, approximation or audio-thread mutation.
 float getWavetablePreviewPoint(int oscillator,int wave,int frame,int sample) const noexcept;
 float getWavetablePreview(int oscillator,int wave,float position,float phase,
                           bool applyTransforms=true) const noexcept;
 void commitCustomFrame(int oscillator,int frame,const std::array<float,serenity::editableSamples>& points) noexcept;
 void normalizeCustomFrame(int oscillator,int frame) noexcept;
 void removeCustomDc(int oscillator,int frame) noexcept;
 void copyCustomFrame(int oscillator,int from,int to) noexcept;
 void resetCustomFrame(int oscillator,int frame) noexcept;
 void rebuildCustomFrame(int oscillator,int frame) noexcept;
 std::array<float,serenity::spectral::displayedHarmonics>
 getCustomHarmonics(int oscillator,int frame) const noexcept;
 void setCustomHarmonic(int oscillator,int frame,int harmonic,float amplitude) noexcept;
 void transformCustomFrame(int oscillator,int frame,serenity::spectral::Transform mode) noexcept;
 void morphCustomFrames(int oscillator,int first,int last) noexcept;
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
 float getChannelPeak(int channel)const noexcept{return channel==0?outputPeakLeft.load(std::memory_order_relaxed):outputPeakRight.load(std::memory_order_relaxed);}
 float getHostTempo()const noexcept{return hostTempoDisplay.load(std::memory_order_relaxed);}
 float getOutputScopeSample(int index) const noexcept {
  return index>=0&&index<256?outputScope[(size_t)index].load(std::memory_order_relaxed):0.f;
 }
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
 std::array<std::atomic<float>,serenity::editableTotal> customPoints{};
 // Published by editor/host thread, read atomically by the audio callback.
 // Per-frame seqlock: odd while writing; even when complete. Never block/spin on audio.
 // Allocated once at construction, never from the realtime callback.
 std::unique_ptr<std::atomic<float>[]> customMipPoints;
 std::array<std::atomic<unsigned>,serenity::editableOscillators*serenity::editableFrames> customMipVersions{};
 std::array<unsigned,serenity::editableOscillators*serenity::editableFrames> appliedMipVersions{};
 std::array<std::atomic<int>,serenity::parameterCount> controller{};
 std::array<std::atomic<float>,serenity::parameterCount> midiOverride{};
 std::atomic<int> learnTarget{-1},lastCC{-1},selectedPreset{0},voiceCount{0};
 std::atomic<float> outputMeter{0.f},outputPeakLeft{0.f},outputPeakRight{0.f},hostTempoDisplay{120.f};
 // Last rendered mono audio window, atomically published without allocation.
 std::array<std::atomic<float>,256> outputScope{};
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
