#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstring>

namespace serenity {
const std::array<ParamSpec,parameterCount> specs={{
 {"master",0,1.5f,.8f},{"cutoff",0,1,0.85f},{"resonance",0,1,.14f},
 {"position",0,1,.5f},{"attack",.001f,10,.004f},{"decay",.001f,10,.45f},
 {"sustain",0,1,.7f},{"release",.001f,12,.35f},
 {"detune",0,1,.25f},{"spread",0,1,.7f},{"delayMix",0,1,.1f},
 {"reverbMix",0,1,.2f},{"drive",0,1,0},{"lfoRate",.01f,40,5},
 {"lfoDepth",0,1,0},
 {"oscALevel",0,1,.7f},{"oscBLevel",0,1,0},{"oscCLevel",0,1,0},{"oscDLevel",0,1,0},
 {"oscAWave",0,16,1},{"oscBWave",0,16,0},{"oscCWave",0,16,3},{"oscDWave",0,16,0},
 {"oscAUnison",1,16,1},{"oscBUnison",1,16,1},{"oscCUnison",1,16,1},{"oscDUnison",1,16,1},
 {"polyphony",1,32,16},
 {"filter2Cutoff",0,1,.55f},{"filter2Resonance",0,1,.2f},
 {"filter2Mix",0,1,1.f},{"filter2Enabled",0,1,0.f},{"filterRouting",0,1,0.f},
 {"macroBrightness",0,1,.5f},{"macroMovement",0,1,.5f},
 {"macroWarmth",0,1,.5f},{"macroSpace",0,1,.5f},
 {"macroWidth",0,1,.5f},{"macroTexture",0,1,.5f},
 {"macroEnergy",0,1,.5f},{"macroSerenity",0,1,.5f},
 {"oscAOctave",-3,3,0},{"oscBOctave",-3,3,0},{"oscCOctave",-3,3,0},{"oscDOctave",-3,3,0},
 {"oscASemi",-12,12,0},{"oscBSemi",-12,12,0},{"oscCSemi",-12,12,0},{"oscDSemi",-12,12,0},
 {"oscAFine",-100,100,0},{"oscBFine",-100,100,0},{"oscCFine",-100,100,0},{"oscDFine",-100,100,0},
 {"oscAPan",-1,1,0},{"oscBPan",-1,1,0},{"oscCPan",-1,1,0},{"oscDPan",-1,1,0},
 {"oscAWarp",-1,1,0},{"oscBWarp",-1,1,0},{"oscCWarp",-1,1,0},{"oscDWarp",-1,1,0},
 {"oscAShape",0,1,0},{"oscBShape",0,1,0},{"oscCShape",0,1,0},{"oscDShape",0,1,0},
 {"oscAWarpMode",0,3,0},{"oscBWarpMode",0,3,0},{"oscCWarpMode",0,3,0},{"oscDWarpMode",0,3,0},
 {"oscAShapeMode",0,3,0},{"oscBShapeMode",0,3,0},{"oscCShapeMode",0,3,0},{"oscDShapeMode",0,3,0},
 {"oscABlend",0,1,0},{"oscBBlend",0,1,0},{"oscCBlend",0,1,0},{"oscDBlend",0,1,0},
 {"oscAPhase",0,1,0},{"oscBPhase",0,1,0},{"oscCPhase",0,1,0},{"oscDPhase",0,1,0},
 {"oscARandPhase",0,1,0},{"oscBRandPhase",0,1,0},{"oscCRandPhase",0,1,0},{"oscDRandPhase",0,1,0},
 {"fmBA",0,1,0},
 {"fmCA",0,1,0},
 {"fmCB",0,1,0},
 {"ringAmount",0,1,0},
 {"fmMode",0,1,0},
 {"ringMode",0,1,0},
 {"env1Delay",0,5,0},
 {"env1Hold",0,5,0},
 {"env2Delay",0,5,0},
 {"env2Attack",.001f,10,.004f},
 {"env2Hold",0,5,0},
 {"env2Decay",.001f,10,.45f},
 {"env2Sustain",0,1,.7f},
 {"env2Release",.001f,12,.35f},
 {"env3Delay",0,5,0},
 {"env3Attack",.001f,10,.004f},
 {"env3Hold",0,5,0},
 {"env3Decay",.001f,10,.45f},
 {"env3Sustain",0,1,.7f},
 {"env3Release",.001f,12,.35f},
 {"env4Delay",0,5,0},
 {"env4Attack",.001f,10,.004f},
 {"env4Hold",0,5,0},
 {"env4Decay",.001f,10,.45f},
 {"env4Sustain",0,1,.7f},
 {"env4Release",.001f,12,.35f},
 {"env5Delay",0,5,0},
 {"env5Attack",.001f,10,.004f},
 {"env5Hold",0,5,0},
 {"env5Decay",.001f,10,.45f},
 {"env5Sustain",0,1,.7f},
 {"env5Release",.001f,12,.35f},
 {"env6Delay",0,5,0},
 {"env6Attack",.001f,10,.004f},
 {"env6Hold",0,5,0},
 {"env6Decay",.001f,10,.45f},
 {"env6Sustain",0,1,.7f},
 {"env6Release",.001f,12,.35f},
 {"lfo1Shape",0,4,0},
 {"lfo1Phase",0,1,0},
 {"lfo1Unipolar",0,1,0},
 {"lfo2Rate",.01f,40,5.90f},
 {"lfo2Shape",0,4,0},
 {"lfo2Phase",0,1,0},
 {"lfo2Unipolar",0,1,0},
 {"lfo3Rate",.01f,40,6.80f},
 {"lfo3Shape",0,4,0},
 {"lfo3Phase",0,1,0},
 {"lfo3Unipolar",0,1,0},
 {"lfo4Rate",.01f,40,7.70f},
 {"lfo4Shape",0,4,0},
 {"lfo4Phase",0,1,0},
 {"lfo4Unipolar",0,1,0},
 {"lfo5Rate",.01f,40,8.60f},
 {"lfo5Shape",0,4,0},
 {"lfo5Phase",0,1,0},
 {"lfo5Unipolar",0,1,0},
 {"lfo6Rate",.01f,40,9.50f},
 {"lfo6Shape",0,4,0},
 {"lfo6Phase",0,1,0},
 {"lfo6Unipolar",0,1,0},
 {"lfo7Rate",.01f,40,10.40f},
 {"lfo7Shape",0,4,0},
 {"lfo7Phase",0,1,0},
 {"lfo7Unipolar",0,1,0},
 {"lfo8Rate",.01f,40,11.30f},
 {"lfo8Shape",0,4,0},
 {"lfo8Phase",0,1,0},
 {"lfo8Unipolar",0,1,0},
 {"delayTime",.02f,1.9f,.38f},
 {"delayFeedback",0,.92f,.35f},
 {"delayTone",0,1,1},
 {"reverbFeedback",0,.94f,.76f},
 {"masterWidth",0,1.5f,1},
 {"tremRate",.05f,20,3},
 {"tremDepth",0,1,0},
 {"tremMode",0,1,0},
 {"fxRingFreq",20,2000,120},
 {"fxRingMix",0,1,0}
}};
}

juce::AudioProcessorValueTreeState::ParameterLayout SerenityAudioProcessor::createParameterLayout(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.reserve(serenity::parameterCount);
 for(int i=0;i<serenity::parameterCount;++i){
  const auto& s=serenity::specs[(size_t)i];
  const bool discrete=(i>=serenity::oscAWave&&i<=serenity::polyphony)
    ||i==serenity::filter2Enabled||i==serenity::filterRouting
    ||(i>=serenity::oscAOctave&&i<=serenity::oscDSemi)
    ||(i>=serenity::oscAWarpMode&&i<=serenity::oscDShapeMode)
    ||i==serenity::fmMode||i==serenity::ringMode||i==serenity::tremMode
    ||i==serenity::lfo1Shape||i==serenity::lfo1Unipolar
    ||i==serenity::lfo2Shape||i==serenity::lfo2Unipolar
    ||i==serenity::lfo3Shape||i==serenity::lfo3Unipolar
    ||i==serenity::lfo4Shape||i==serenity::lfo4Unipolar
    ||i==serenity::lfo5Shape||i==serenity::lfo5Unipolar
    ||i==serenity::lfo6Shape||i==serenity::lfo6Unipolar
    ||i==serenity::lfo7Shape||i==serenity::lfo7Unipolar
    ||i==serenity::lfo8Shape||i==serenity::lfo8Unipolar;
  juce::NormalisableRange<float> r(s.lo,s.hi,discrete?1.0f:0.0001f);
  p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id,1},juce::String(s.id),r,s.initial));
 }
 return {p.begin(),p.end()};
}
SerenityAudioProcessor::SerenityAudioProcessor():
 juce::AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 apvts(*this,nullptr,"SerenityState",createParameterLayout()) {
 for(int i=0;i<serenity::parameterCount;i++){
  controller[(size_t)i].store(-1);midiOverride[(size_t)i].store(-1.0f);
 }
 for(int i=0;i<64;i++){
  modSources[(size_t)i].store(-1);modDestinations[(size_t)i].store(-1);
  modAmounts[(size_t)i].store(0);
 }
 for(auto &f:favorites)f.store(false);
 for(auto &sample:outputScope)sample.store(0.f,std::memory_order_relaxed);
 for(int o=0;o<serenity::editableOscillators;o++)
  for(int f=0;f<serenity::editableFrames;f++)
   for(int i=0;i<serenity::editableSamples;i++) {
    const float phase=6.2831853071795864769f*(float)i/serenity::editableSamples;
    customPoints[serenity::EditableWavetable::index(o,f,i)].store(std::sin(phase));
   }
 customMipPoints=std::make_unique<std::atomic<float>[]>(
   serenity::editableTotal*serenity::customMipCount);
 for(auto& version:customMipVersions)version.store(0);
 appliedMipVersions.fill(0);
 for(int o=0;o<serenity::editableOscillators;o++)
  for(int f=0;f<serenity::editableFrames;f++)
   rebuildCustomFrame(o,f);
 const auto defaults=serenity::defaultSequence();
 for(int b=0;b<serenity::sequenceBanks;b++)
  for(int i=0;i<serenity::sequenceMaxSteps;i++) {
   sequencePitch[(size_t)b][(size_t)i].store(defaults.steps[(size_t)i].pitch);
   sequenceVelocity[(size_t)b][(size_t)i].store(defaults.steps[(size_t)i].velocity);
   sequenceModulation[(size_t)b][(size_t)i].store(0.f);
  }
}
bool SerenityAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
}
void SerenityAudioProcessor::prepareToPlay(double rate,int){engine.prepare(rate);}
void SerenityAudioProcessor::releaseResources(){engine.reset();}
float SerenityAudioProcessor::getValue(int index) const noexcept {
 if(index<0||index>=serenity::parameterCount)return 0;
 const auto& spec=serenity::specs[(size_t)index];
 float ov=midiOverride[(size_t)index].load(std::memory_order_relaxed);
 if(ov>=0)return spec.lo+(spec.hi-spec.lo)*ov;
 auto *value=apvts.getRawParameterValue(spec.id);
 return value?value->load(std::memory_order_relaxed):spec.initial;
}
void SerenityAudioProcessor::setValue(int index,float physicalValue){
 if(index<0||index>=serenity::parameterCount)return;
 const auto& spec=serenity::specs[(size_t)index];
 if(auto *p=apvts.getParameter(spec.id)){
  midiOverride[(size_t)index].store(-1.f);
  p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(spec.lo,spec.hi,physicalValue)));
  p->endChangeGesture();
 }
}
serenity::Params SerenityAudioProcessor::buildParams() const noexcept {
 serenity::Params p;
 p.master=getValue(serenity::master);
 p.cutoff=getValue(serenity::cutoff);
 p.resonance=getValue(serenity::resonance);
 p.position=getValue(serenity::position);
 p.attack=getValue(serenity::attack);p.decay=getValue(serenity::decay);
 p.sustain=getValue(serenity::sustain);p.release=getValue(serenity::release);
 p.detune=getValue(serenity::detune);p.spread=getValue(serenity::spread);
 p.delay=getValue(serenity::delayMix);p.reverb=getValue(serenity::reverbMix);
 p.drive=getValue(serenity::drive);p.lfoRate=getValue(serenity::lfoRate);
 p.lfoDepth=getValue(serenity::lfoDepth);
 p.cutoff2=getValue(serenity::filter2Cutoff);
 p.resonance2=getValue(serenity::filter2Resonance);
 p.filter2Mix=getValue(serenity::filter2Mix);
 p.filter2On=getValue(serenity::filter2Enabled)>.5f;
 p.parallelFilters=getValue(serenity::filterRouting)>.5f;
 const int levels[]={serenity::oscALevel,serenity::oscBLevel,serenity::oscCLevel,serenity::oscDLevel};
 const int waves[]={serenity::oscAWave,serenity::oscBWave,serenity::oscCWave,serenity::oscDWave};
 const int unis[]={serenity::oscAUnison,serenity::oscBUnison,serenity::oscCUnison,serenity::oscDUnison};
 for(int i=0;i<4;i++){
  p.oscLevel[i]=getValue(levels[i]);p.wave[i]=(int)std::round(getValue(waves[i]));
  p.unison[i]=(int)std::round(getValue(unis[i]));
  p.oscOctave[i]=getValue(serenity::oscAOctave+i);
  p.oscSemi[i]=getValue(serenity::oscASemi+i);
  p.oscFine[i]=getValue(serenity::oscAFine+i);
  p.oscPan[i]=getValue(serenity::oscAPan+i);
  p.oscWarp[i]=getValue(serenity::oscAWarp+i);
  p.oscShape[i]=getValue(serenity::oscAShape+i);
  p.oscWarpMode[i]=(int)std::lround(getValue(serenity::oscAWarpMode+i));
  p.oscShapeMode[i]=(int)std::lround(getValue(serenity::oscAShapeMode+i));
  p.oscBlend[i]=getValue(serenity::oscABlend+i);
  p.oscPhase[i]=getValue(serenity::oscAPhase+i);
  p.oscRandPhase[i]=getValue(serenity::oscARandPhase+i);
 }
 p.polyphony=(int)getValue(serenity::polyphony);
 p.fmBA=getValue(serenity::fmBA);
 p.fmCA=getValue(serenity::fmCA);
 p.fmCB=getValue(serenity::fmCB);
 p.ringAmount=getValue(serenity::ringAmount);
 p.delayTime=getValue(serenity::delayTime);
 p.delayFeedback=getValue(serenity::delayFeedback);
 p.delayTone=getValue(serenity::delayTone);
 p.reverbFeedback=getValue(serenity::reverbFeedback);
 p.masterWidth=getValue(serenity::masterWidth);
 p.tremRate=getValue(serenity::tremRate);
 p.tremDepth=getValue(serenity::tremDepth);
 p.fxRingFreq=getValue(serenity::fxRingFreq);
 p.fxRingMix=getValue(serenity::fxRingMix);
 p.fmMode=(int)getValue(serenity::fmMode);
 p.ringMode=(int)getValue(serenity::ringMode);
 p.tremMode=(int)getValue(serenity::tremMode);
 p.envDelay=getValue(serenity::env1Delay);p.envHold=getValue(serenity::env1Hold);
 for(int k=1;k<6;k++){
  auto& e=p.modEnv[(size_t)k-1];
  e.delay=getValue(serenity::envParameter(k,0));
  e.attack=getValue(serenity::envParameter(k,1));
  e.hold=getValue(serenity::envParameter(k,2));
  e.decay=getValue(serenity::envParameter(k,3));
  e.sustain=getValue(serenity::envParameter(k,4));
  e.release=getValue(serenity::envParameter(k,5));
 }
 for(int k=0;k<8;k++){
  auto& f=p.lfos[(size_t)k];f.rate=getValue(serenity::lfoParameter(k,0));
  f.shape=(int)getValue(serenity::lfoParameter(k,1));f.phase=getValue(serenity::lfoParameter(k,2));f.unipolar=getValue(serenity::lfoParameter(k,3))>.5f;
 }
 // 8 macro parameters are host-automatable; map 4 useful defaults into DSP.
 p.cutoff=serenity::clamp(p.cutoff+(getValue(serenity::macroBrightness)-.5f)*.2f,0,1);
 p.lfoDepth=serenity::clamp(p.lfoDepth+(getValue(serenity::macroMovement)-.5f)*.14f,0,1);
 p.reverb=serenity::clamp(p.reverb+(getValue(serenity::macroSpace)-.5f)*.15f,0,1);
 p.spread=serenity::clamp(p.spread+(getValue(serenity::macroWidth)-.5f)*.2f,0,1);
 // Four additional macro mappings; 0.5 is neutral.
 p.drive=serenity::clamp(p.drive+(getValue(serenity::macroWarmth)-.5f)*.28f,0,1);
 p.position=serenity::clamp(p.position+(getValue(serenity::macroTexture)-.5f)*.30f,0,1);
 p.master=serenity::clamp(p.master+(getValue(serenity::macroEnergy)-.5f)*.25f,0,1.5f);
 p.reverb=serenity::clamp(p.reverb+(getValue(serenity::macroSerenity)-.5f)*.22f,0,1);
 return p;
}

serenity::SequenceStep SerenityAudioProcessor::getSequenceStep(int bank,int index) const noexcept {
 if(bank<0||bank>=serenity::sequenceBanks||index<0||index>=serenity::sequenceMaxSteps) return {};
 return {
  sequencePitch[(size_t)bank][(size_t)index].load(std::memory_order_relaxed),
  sequenceVelocity[(size_t)bank][(size_t)index].load(std::memory_order_relaxed),
  sequenceModulation[(size_t)bank][(size_t)index].load(std::memory_order_relaxed)
 };
}
void SerenityAudioProcessor::setSequenceStep(int bank,int index,serenity::SequenceStep step) noexcept {
 if(bank<0||bank>=serenity::sequenceBanks||index<0||index>=serenity::sequenceMaxSteps)return;
 sequencePitch[(size_t)bank][(size_t)index].store(juce::jlimit(-24,24,step.pitch),std::memory_order_relaxed);
 sequenceVelocity[(size_t)bank][(size_t)index].store(juce::jlimit(0.f,1.f,step.velocity),std::memory_order_relaxed);
 sequenceModulation[(size_t)bank][(size_t)index].store(juce::jlimit(-1.f,1.f,step.modulation),std::memory_order_relaxed);
}
void SerenityAudioProcessor::setSequenceMode(int mode) noexcept {
 sequenceMode.store(juce::jlimit(0,2,mode),std::memory_order_relaxed);
}
void SerenityAudioProcessor::setSequenceRate(int rate) noexcept {
 if(rate==4||rate==8||rate==16||rate==32)
  sequenceRate.store(rate,std::memory_order_relaxed);
}

void SerenityAudioProcessor::setCustomPoint(int oscillator,int frame,int sample,float value) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||
    frame<0||frame>=serenity::editableFrames||
    sample<0||sample>=serenity::editableSamples)return;
 const auto index=serenity::EditableWavetable::index(oscillator,frame,sample);
 customPoints[index].store(juce::jlimit(-1.f,1.f,value),std::memory_order_relaxed);
 // A drag writes to the editable raw frame. The GUI publishes the completed
 // anti-aliased spectrum on mouse release to avoid heavy FFT work per pixel.
}

float SerenityAudioProcessor::getWavetablePreviewPoint(int osc,int wave,int frame,int sample) const noexcept {
 if(osc<0||osc>=serenity::editableOscillators||frame<0||
    frame>=serenity::editableFrames||sample<0||sample>=serenity::editableSamples)return 0.f;
 if(wave==serenity::waveCount)return getCustomPoint(osc,frame,sample);
 if(wave<0||wave>=serenity::waveCount)return 0.f;
 return engine.previewFactoryPoint(wave,frame,sample);
}
float SerenityAudioProcessor::getWavetablePreview(int osc,int wave,float position,
                                                 float phase,bool transformed) const noexcept {
 if(osc<0||osc>=serenity::editableOscillators||!std::isfinite(position)||!std::isfinite(phase))return 0.f;
 if(transformed)
  phase=serenity::oscillatorWarpPhase(phase,getValue(serenity::oscAWarp+osc),
                                    (int)std::lround(getValue(serenity::oscAWarpMode+osc)));
 phase-=std::floor(phase);
 const float f=juce::jlimit(0.f,1.f,position)*7.f;
 const int a=(int)f,b=std::min(7,a+1);
 const float z=f-(float)a;
 const float x=phase*serenity::editableSamples;
 const int i=((int)x)&511,j=(i+1)&511;
 const float blend=x-std::floor(x);
 const float p0=juce::jmap(blend,
   getWavetablePreviewPoint(osc,wave,a,i),getWavetablePreviewPoint(osc,wave,a,j));
 const float p1=juce::jmap(blend,
   getWavetablePreviewPoint(osc,wave,b,i),getWavetablePreviewPoint(osc,wave,b,j));
 const float raw=juce::jmap(z,p0,p1);
 return transformed ? serenity::oscillatorShape(raw,getValue(serenity::oscAShape+osc),
                              (int)std::lround(getValue(serenity::oscAShapeMode+osc))):raw;
}
float SerenityAudioProcessor::getCustomPoint(int oscillator,int frame,int sample) const noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||
    frame<0||frame>=serenity::editableFrames||
    sample<0||sample>=serenity::editableSamples)return 0.f;
 return customPoints[serenity::EditableWavetable::index(oscillator,frame,sample)]
   .load(std::memory_order_relaxed);
}
void SerenityAudioProcessor::commitCustomFrame(int oscillator,int frame,
 const std::array<float,serenity::editableSamples>& points) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||
    frame<0||frame>=serenity::editableFrames)return;
 for(int i=0;i<serenity::editableSamples;i++) {
  customPoints[serenity::EditableWavetable::index(oscillator,frame,i)]
   .store(juce::jlimit(-1.f,1.f,points[(size_t)i]),std::memory_order_relaxed);
 }
 rebuildCustomFrame(oscillator,frame);
}
void SerenityAudioProcessor::normalizeCustomFrame(int oscillator,int frame) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||frame<0||frame>=serenity::editableFrames)return;
 std::array<float,serenity::editableSamples> points{};
 float peak=0;
 for(int i=0;i<serenity::editableSamples;i++){
  points[(size_t)i]=getCustomPoint(oscillator,frame,i);
  peak=std::max(peak,std::abs(points[(size_t)i]));
 }
 if(peak<.000001f)return;
 for(auto& value:points)value/=peak;
 commitCustomFrame(oscillator,frame,points);
}
void SerenityAudioProcessor::removeCustomDc(int oscillator,int frame) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||frame<0||frame>=serenity::editableFrames)return;
 std::array<float,serenity::editableSamples> points{};
 float mean=0;
 for(int i=0;i<serenity::editableSamples;i++){
  points[(size_t)i]=getCustomPoint(oscillator,frame,i);
  mean+=points[(size_t)i];
 }
 mean/=serenity::editableSamples;
 for(auto& value:points)value-=mean;
 commitCustomFrame(oscillator,frame,points);
}
void SerenityAudioProcessor::copyCustomFrame(int oscillator,int from,int to) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||from<0||from>=serenity::editableFrames||
    to<0||to>=serenity::editableFrames)return;
 std::array<float,serenity::editableSamples> points{};
 for(int i=0;i<serenity::editableSamples;i++)points[(size_t)i]=getCustomPoint(oscillator,from,i);
 commitCustomFrame(oscillator,to,points);
}
void SerenityAudioProcessor::resetCustomFrame(int oscillator,int frame) noexcept {
 std::array<float,serenity::editableSamples> points{};
 for(int i=0;i<serenity::editableSamples;i++)
  points[(size_t)i]=std::sin(6.2831853071795864769f*(float)i/serenity::editableSamples);
 commitCustomFrame(oscillator,frame,points);
}

std::array<float,serenity::spectral::displayedHarmonics>
SerenityAudioProcessor::getCustomHarmonics(int osc,int frame) const noexcept {
 serenity::spectral::Frame values{};
 if(osc<0||osc>=serenity::editableOscillators||frame<0||frame>=serenity::editableFrames)return {};
 for(int i=0;i<serenity::editableSamples;i++)
  values[(size_t)i]=getCustomPoint(osc,frame,i);
 return serenity::spectral::analyze(values);
}
void SerenityAudioProcessor::setCustomHarmonic(int osc,int frame,int harmonic,float amplitude) noexcept {
 if(osc<0||osc>=serenity::editableOscillators||frame<0||frame>=serenity::editableFrames||
    harmonic<1||harmonic>serenity::spectral::displayedHarmonics)return;
 serenity::spectral::Frame values{};
 for(int i=0;i<serenity::editableSamples;i++)values[(size_t)i]=getCustomPoint(osc,frame,i);
 commitCustomFrame(osc,frame,serenity::spectral::setHarmonic(values,harmonic,amplitude));
}
void SerenityAudioProcessor::transformCustomFrame(int osc,int frame,serenity::spectral::Transform transform) noexcept {
 if(osc<0||osc>=serenity::editableOscillators||frame<0||frame>=serenity::editableFrames)return;
 serenity::spectral::Frame values{};
 for(int i=0;i<serenity::editableSamples;i++)values[(size_t)i]=getCustomPoint(osc,frame,i);
 commitCustomFrame(osc,frame,serenity::spectral::transform(values,transform));
}
void SerenityAudioProcessor::morphCustomFrames(int osc,int first,int last) noexcept {
 if(osc<0||osc>=serenity::editableOscillators||
    first<0||last>=serenity::editableFrames||first>=last-1)return;
 serenity::spectral::Frame a{},b{};
 for(int i=0;i<serenity::editableSamples;i++){
  a[(size_t)i]=getCustomPoint(osc,first,i);
  b[(size_t)i]=getCustomPoint(osc,last,i);
 }
 for(int frame=first+1;frame<last;frame++) {
  const float t=(float)(frame-first)/(last-first);
  commitCustomFrame(osc,frame,serenity::spectral::morph(a,b,t));
 }
}

void SerenityAudioProcessor::rebuildCustomFrame(int oscillator,int frame) noexcept {
 if(oscillator<0||oscillator>=serenity::editableOscillators||
    frame<0||frame>=serenity::editableFrames)return;
 serenity::spectral::Frame raw{};
 for(int i=0;i<serenity::editableSamples;i++)
  raw[(size_t)i]=getCustomPoint(oscillator,frame,i);
 serenity::CustomMipFrame mips{};
 serenity::buildCustomMipFrame(raw,mips);
 const auto slot=(size_t)oscillator*serenity::editableFrames+(size_t)frame;
 customMipVersions[slot].fetch_add(1,std::memory_order_acq_rel); // odd: writing
 const size_t base=slot*serenity::customFrameMipSamples;
 for(size_t i=0;i<serenity::customFrameMipSamples;i++)
  customMipPoints[base+i].store(mips[i],std::memory_order_relaxed);
 customMipVersions[slot].fetch_add(1,std::memory_order_release); // even: ready
}
void SerenityAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;
 buffer.clear();
 if(buffer.getNumChannels()<2)return;
 engine.setParams(buildParams());
 // Only changed frames are copied at block boundaries. Do not spin on a writer.
 // Snapshot into fixed stack storage; if a UI edit occurs midway, discard it
 // and retain the previous completed spectrum until the following block.
 for(int o=0;o<serenity::editableOscillators;o++)
  for(int f=0;f<serenity::editableFrames;f++){
   const size_t slot=(size_t)o*serenity::editableFrames+(size_t)f;
   const auto begin=customMipVersions[slot].load(std::memory_order_acquire);
   if((begin&1u)!=0u || begin==appliedMipVersions[slot])continue;
   serenity::CustomMipFrame scratch{};
   const size_t offset=slot*serenity::customFrameMipSamples;
   for(size_t i=0;i<serenity::customFrameMipSamples;i++)
    scratch[i]=customMipPoints[offset+i].load(std::memory_order_relaxed);
   const auto end=customMipVersions[slot].load(std::memory_order_acquire);
   if(begin!=end || (end&1u)!=0u)continue;
   engine.setCustomMipFrame(o,f,scratch);
   appliedMipVersions[slot]=end;
  }
 serenity::SequencePattern sequence;
 const int bank=sequenceBank.load(std::memory_order_relaxed);
 for(int i=0;i<serenity::sequenceMaxSteps;i++)
  sequence.steps[(size_t)i]=getSequenceStep(bank,i);
 sequence.length=sequenceLength.load(std::memory_order_relaxed);
 sequence.rate=sequenceRate.load(std::memory_order_relaxed);
 sequence.gate=sequenceGate.load(std::memory_order_relaxed);
 sequence.swing=sequenceSwing.load(std::memory_order_relaxed);
 sequence.mode=sequenceMode.load(std::memory_order_relaxed);
 if(auto* playhead=getPlayHead())
  if(auto position=playhead->getPosition())
   if(auto bpm=position->getBpm())
    sequence.tempo=(float)*bpm;
 engine.setSequence(sequence);
 for(int i=0;i<64;i++) engine.route((size_t)i)=getRoute(i);
 float *l=buffer.getWritePointer(0),*r=buffer.getWritePointer(1);
 int prev=0;
 // Segment by MIDI event timestamp; note-ons and CC take effect at sample position.
 for(const auto meta : midi){
  int sample=juce::jlimit(prev,buffer.getNumSamples(),meta.samplePosition);
  engine.render(l+prev,r+prev,sample-prev);prev=sample;
  const auto& message=meta.getMessage();
  if(message.isNoteOn())engine.noteOn(message.getNoteNumber(),message.getFloatVelocity());
  else if(message.isNoteOff())engine.noteOff(message.getNoteNumber());
  else if(message.isAllNotesOff()||message.isAllSoundOff())engine.allNotesOff();
  else if(message.isController()){
   const int cc=message.getControllerNumber();
   const float norm=message.getControllerValue()/127.f;
   if(cc==1)engine.setModWheel(norm);
   int armed=learnTarget.exchange(-1);
   if(armed>=0&&armed<serenity::parameterCount){
    controller[(size_t)armed].store(cc);lastCC.store(cc);
   }
   for(int i=0;i<serenity::parameterCount;i++)
    if(controller[(size_t)i].load(std::memory_order_relaxed)==cc)
     midiOverride[(size_t)i].store(norm,std::memory_order_relaxed);
   engine.setParams(buildParams());
  }
 }
 engine.render(l+prev,r+prev,buffer.getNumSamples()-prev);
 voiceCount.store(engine.numVoices(),std::memory_order_relaxed);
 sequencePlayingStep.store(engine.sequenceStep(),std::memory_order_relaxed);
 float peak=0;
 for(int n=0;n<buffer.getNumSamples();n++) peak=std::max(peak,std::max(std::abs(l[n]),std::abs(r[n])));
 outputMeter.store(peak,std::memory_order_relaxed);
 const int numSamples=buffer.getNumSamples();
 if(numSamples>0){
  for(int k=0;k<256;k++){
   const int index=(k*(numSamples-1))/255;
   outputScope[(size_t)k].store(.5f*(l[index]+r[index]),
                                std::memory_order_relaxed);
  }
 }
}
bool SerenityAudioProcessor::isFavorite(int p) const noexcept {
 return p>=0&&p<100?favorites[(size_t)p].load():false;
}
void SerenityAudioProcessor::toggleFavorite(int p) noexcept {
 if(p>=0&&p<100) favorites[(size_t)p].store(!favorites[(size_t)p].load());
}
serenity::ModRoute SerenityAudioProcessor::getRoute(int slot) const noexcept {
 if(slot<0||slot>=64)return {};
 return {modSources[(size_t)slot].load(),modDestinations[(size_t)slot].load(),
         modAmounts[(size_t)slot].load()};
}
void SerenityAudioProcessor::setRoute(int slot,serenity::ModRoute next) noexcept {
 if(slot<0||slot>=64)return;
 modSources[(size_t)slot].store(juce::jlimit(-1,16,next.source));
 modDestinations[(size_t)slot].store(juce::jlimit(-1,4,next.destination));
 modAmounts[(size_t)slot].store(juce::jlimit(-1.f,1.f,next.amount));
}
void SerenityAudioProcessor::armMidiLearn(int parameter) noexcept {
 if(parameter<0||parameter>=serenity::parameterCount){learnTarget.store(-1);return;}
 learnTarget.store(parameter);
}
void SerenityAudioProcessor::clearMapping(int parameter) noexcept {
 if(parameter<0||parameter>=serenity::parameterCount)return;
 controller[(size_t)parameter].store(-1);midiOverride[(size_t)parameter].store(-1);
}
int SerenityAudioProcessor::mappedCC(int parameter) const noexcept {
 return (parameter>=0&&parameter<serenity::parameterCount)?controller[(size_t)parameter].load():-1;
}
void SerenityAudioProcessor::applyPreset(int index){
 const auto& all=serenity::factoryPresets();
 if(index<0||index>=(int)all.size())return;
 const auto& p=all[(size_t)index].p;
 const std::pair<int,float> values[]={
  {serenity::master,p.master},{serenity::cutoff,p.cutoff},{serenity::resonance,p.resonance},
  {serenity::position,p.position},{serenity::attack,p.attack},{serenity::decay,p.decay},
  {serenity::sustain,p.sustain},{serenity::release,p.release},{serenity::detune,p.detune},
  {serenity::spread,p.spread},{serenity::delayMix,p.delay},{serenity::reverbMix,p.reverb},
  {serenity::drive,p.drive},{serenity::lfoRate,p.lfoRate},{serenity::lfoDepth,p.lfoDepth},
  {serenity::oscALevel,p.oscLevel[0]},{serenity::oscBLevel,p.oscLevel[1]},
  {serenity::oscCLevel,p.oscLevel[2]},{serenity::oscDLevel,p.oscLevel[3]},
  {serenity::oscAWave,(float)p.wave[0]},{serenity::oscBWave,(float)p.wave[1]},
  {serenity::oscCWave,(float)p.wave[2]},{serenity::oscDWave,(float)p.wave[3]},
  {serenity::oscAUnison,(float)p.unison[0]},{serenity::oscBUnison,(float)p.unison[1]},
  {serenity::oscCUnison,(float)p.unison[2]},{serenity::oscDUnison,(float)p.unison[3]},
  {serenity::polyphony,(float)p.polyphony},
  {serenity::filter2Cutoff,p.cutoff2},{serenity::filter2Resonance,p.resonance2},
  {serenity::filter2Mix,p.filter2Mix},{serenity::filter2Enabled,p.filter2On?1.f:0.f},
  {serenity::filterRouting,p.parallelFilters?1.f:0.f}
 };
 for(const auto& v:values) setValue(v.first,v.second);
 for(int osc=0;osc<4;osc++){
  setValue(serenity::oscAOctave+osc,p.oscOctave[osc]);
  setValue(serenity::oscASemi+osc,p.oscSemi[osc]);
  setValue(serenity::oscAFine+osc,p.oscFine[osc]);
  setValue(serenity::oscAPan+osc,p.oscPan[osc]);
  setValue(serenity::oscAWarp+osc,p.oscWarp[osc]);
  setValue(serenity::oscAShape+osc,p.oscShape[osc]);
  setValue(serenity::oscAWarpMode+osc,(float)p.oscWarpMode[osc]);
  setValue(serenity::oscAShapeMode+osc,(float)p.oscShapeMode[osc]);
  setValue(serenity::oscABlend+osc,p.oscBlend[osc]);
  setValue(serenity::oscAPhase+osc,p.oscPhase[osc]);
  setValue(serenity::oscARandPhase+osc,p.oscRandPhase[osc]);
 }
 // Factory definitions predating v0.10 reset added modules to their neutral defaults.
 for(int i=serenity::fmBA;i<serenity::parameterCount;i++)setValue(i,serenity::specs[(size_t)i].initial);
 selectedPreset.store(index);
}
const juce::String SerenityAudioProcessor::getProgramName(int index){
 const auto& all=serenity::factoryPresets();
 return index>=0&&index<(int)all.size()?juce::String(all[(size_t)index].name):juce::String();
}
void SerenityAudioProcessor::getStateInformation(juce::MemoryBlock& dest){
 auto state=apvts.copyState();
 state.setProperty("serenityLanguage",frenchUI.load()?"FR":"EN",nullptr);
 state.setProperty("serenityPreset",selectedPreset.load(),nullptr);
 for(int i=0;i<serenity::parameterCount;i++)
  state.setProperty("midiCC"+juce::String(i),controller[(size_t)i].load(),nullptr);
 for(int i=0;i<64;i++){
  const auto route=getRoute(i);
  state.setProperty("routeSrc"+juce::String(i),route.source,nullptr);
  state.setProperty("routeDest"+juce::String(i),route.destination,nullptr);
  state.setProperty("routeAmt"+juce::String(i),route.amount,nullptr);
 }
 juce::String fave;
 for(const auto& f:favorites)fave+=(f.load()?"1":"0");
 state.setProperty("favorites",fave,nullptr);
 // Cross-platform project state: every user wavetable is embedded in its project/preset.
 // Native little-endian float PCM, base64 within the existing APVTS XML.
 juce::MemoryBlock waveBytes;
 for(size_t i=0;i<serenity::editableTotal;i++){
  const float value=customPoints[i].load(std::memory_order_relaxed);
  waveBytes.append(&value,sizeof(value));
 }
 state.setProperty("customWavetablePCM",waveBytes.toBase64Encoding(),nullptr);
 state.setProperty("sequenceMode",sequenceMode.load(),nullptr);
 state.setProperty("sequenceBank",sequenceBank.load(),nullptr);
 state.setProperty("sequenceLength",sequenceLength.load(),nullptr);
 state.setProperty("sequenceRate",sequenceRate.load(),nullptr);
 state.setProperty("sequenceGate",sequenceGate.load(),nullptr);
 state.setProperty("sequenceSwing",sequenceSwing.load(),nullptr);
 for(int b=0;b<serenity::sequenceBanks;b++)for(int i=0;i<serenity::sequenceMaxSteps;i++) {
  const juce::String prefix="seq"+juce::String(b)+"_"+juce::String(i);
  const auto step=getSequenceStep(b,i);
  state.setProperty(prefix+"P",step.pitch,nullptr);
  state.setProperty(prefix+"V",step.velocity,nullptr);
  state.setProperty(prefix+"M",step.modulation,nullptr);
 }
 if(auto xml=state.createXml())copyXmlToBinary(*xml,dest);
}
void SerenityAudioProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size)){
  juce::ValueTree state=juce::ValueTree::fromXml(*xml);
  if(!state.isValid())return;
  frenchUI.store(state.getProperty("serenityLanguage","FR").toString()=="FR");
  selectedPreset.store((int)state.getProperty("serenityPreset",0));
  for(int i=0;i<serenity::parameterCount;i++){
   controller[(size_t)i].store((int)state.getProperty("midiCC"+juce::String(i),-1));
   midiOverride[(size_t)i].store(-1.0f);
  }
  for(int i=0;i<64;i++)setRoute(i,{
   (int)state.getProperty("routeSrc"+juce::String(i),-1),
   (int)state.getProperty("routeDest"+juce::String(i),-1),
   (float)state.getProperty("routeAmt"+juce::String(i),0.f)
  });
  const auto fave=state.getProperty("favorites","").toString();
  for(int i=0;i<100;i++)favorites[(size_t)i].store(i<fave.length()&&fave[i]=='1');
  setSequenceMode((int)state.getProperty("sequenceMode",0));
  setSequenceBank((int)state.getProperty("sequenceBank",0));
  setSequenceLength((int)state.getProperty("sequenceLength",8));
  setSequenceRate((int)state.getProperty("sequenceRate",16));
  setSequenceGate((float)state.getProperty("sequenceGate",.6f));
  setSequenceSwing((float)state.getProperty("sequenceSwing",0.f));
  for(int b=0;b<serenity::sequenceBanks;b++)for(int i=0;i<serenity::sequenceMaxSteps;i++) {
   const juce::String prefix="seq"+juce::String(b)+"_"+juce::String(i);
   const auto defaults=getSequenceStep(b,i);
   setSequenceStep(b,i,{
     (int)state.getProperty(prefix+"P",defaults.pitch),
     (float)state.getProperty(prefix+"V",defaults.velocity),
     (float)state.getProperty(prefix+"M",defaults.modulation)
   });
  }
  const juce::String encoded=state.getProperty("customWavetablePCM","").toString();
  juce::MemoryBlock waveBytes;
  if(waveBytes.fromBase64Encoding(encoded) &&
     waveBytes.getSize()==serenity::editableTotal*sizeof(float)){
   const auto* bytes=static_cast<const unsigned char*>(waveBytes.getData());
   for(size_t i=0;i<serenity::editableTotal;i++){
    float value=0.f;
    std::memcpy(&value,bytes+i*sizeof(float),sizeof(value));
    customPoints[i].store(std::isfinite(value)?juce::jlimit(-1.f,1.f,value):0.f,
                         std::memory_order_relaxed);
   }
   for(int o=0;o<serenity::editableOscillators;o++)
    for(int f=0;f<serenity::editableFrames;f++)
     rebuildCustomFrame(o,f);
  }
  apvts.replaceState(state);
 }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SerenityAudioProcessor();}
