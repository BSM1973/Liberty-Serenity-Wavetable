#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace serenity {
const std::array<ParamSpec,parameterCount> specs={{
 {"master",0,1.5f,.8f},{"cutoff",0,1,0.85f},{"resonance",0,1,.14f},
 {"position",0,1,.5f},{"attack",.001f,10,.004f},{"decay",.001f,10,.45f},
 {"sustain",0,1,.7f},{"release",.001f,12,.35f},
 {"detune",0,1,.25f},{"spread",0,1,.7f},{"delayMix",0,1,.1f},
 {"reverbMix",0,1,.2f},{"drive",0,1,0},{"lfoRate",.01f,40,5},
 {"lfoDepth",0,1,0},
 {"oscALevel",0,1,.7f},{"oscBLevel",0,1,0},{"oscCLevel",0,1,0},{"oscDLevel",0,1,0},
 {"oscAWave",0,15,1},{"oscBWave",0,15,0},{"oscCWave",0,15,3},{"oscDWave",0,15,0},
 {"oscAUnison",1,16,1},{"oscBUnison",1,16,1},{"oscCUnison",1,16,1},{"oscDUnison",1,16,1},
 {"polyphony",1,32,16},
 {"filter2Cutoff",0,1,.55f},{"filter2Resonance",0,1,.2f},
 {"filter2Mix",0,1,1.f},{"filter2Enabled",0,1,0.f},{"filterRouting",0,1,0.f},
 {"macroBrightness",0,1,.5f},{"macroMovement",0,1,.5f},
 {"macroWarmth",0,1,.5f},{"macroSpace",0,1,.5f},
 {"macroWidth",0,1,.5f},{"macroTexture",0,1,.5f},
 {"macroEnergy",0,1,.5f},{"macroSerenity",0,1,.5f}
}};
}

juce::AudioProcessorValueTreeState::ParameterLayout SerenityAudioProcessor::createParameterLayout(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.reserve(serenity::parameterCount);
 for(int i=0;i<serenity::parameterCount;++i){
  const auto& s=serenity::specs[(size_t)i];
  const bool discrete=(i>=serenity::oscAWave&&i<=serenity::polyphony)||i==serenity::filter2Enabled||i==serenity::filterRouting;
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
 }
 p.polyphony=(int)getValue(serenity::polyphony);
 // 8 macro parameters are host-automatable; map 4 useful defaults into DSP.
 p.cutoff=serenity::clamp(p.cutoff+(getValue(serenity::macroBrightness)-.5f)*.2f,0,1);
 p.lfoDepth=serenity::clamp(p.lfoDepth+(getValue(serenity::macroMovement)-.5f)*.14f,0,1);
 p.reverb=serenity::clamp(p.reverb+(getValue(serenity::macroSpace)-.5f)*.15f,0,1);
 p.spread=serenity::clamp(p.spread+(getValue(serenity::macroWidth)-.5f)*.2f,0,1);
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
void SerenityAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;
 buffer.clear();
 if(buffer.getNumChannels()<2)return;
 engine.setParams(buildParams());
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
  apvts.replaceState(state);
 }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SerenityAudioProcessor();}
