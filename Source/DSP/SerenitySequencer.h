#pragma once
// Serenity WAVETABLE - sample-accurate, fixed-memory step pattern playback.
// This is intentionally independent of JUCE and runs allocation-free on the audio thread.
#include <algorithm>
#include <array>
#include <cmath>

namespace serenity {
constexpr int sequenceBanks = 4;
constexpr int sequenceMaxSteps = 64;

struct SequenceStep {
 int pitch = 0;       // relative to held MIDI key: -24 to +24 semitones
 float velocity = 1.f; // 0 = rest
 float modulation = 0.f; // -1 to +1 modulation lane
};
struct SequencePattern {
 std::array<SequenceStep, sequenceMaxSteps> steps{};
 int length = 8;            // 1..64
 int rate = 16;             // 4, 8, 16 or 32 subdivisions
 float gate = 0.60f;        // 0..1
 float swing = 0.f;         // 0..0.5, pair duration remains invariant
 float tempo = 120.f;       // host tempo or local fallback
 int mode = 0;             // 0 bypass, 1 arpeggiator, 2 sequencer
};
struct SequenceFrame {
 int pitch = 0;
 float velocity = 1.f;
 float modulation = 0.f;
 float gate = 1.f;
 int index = -1;
};
class StepClock {
 SequencePattern pattern{};
 double sampleRate = 48000.;
 double sampleInStep = 0.;
 int stepIndex = 0;
 // The step duration changes only when the pattern or sample rate changes.
 // Keep the original IEEE double calculation, precomputed for even/odd swing.
 std::array<double,2> cachedDurations{{1.,1.}};

 double durationFor(int step) const noexcept {
   const double bpm = std::max(20.,std::min(300.,(double)pattern.tempo));
   const int division = (pattern.rate == 4 || pattern.rate == 8 ||
                         pattern.rate == 16 || pattern.rate == 32) ? pattern.rate : 16;
   const double nominal = sampleRate * (240. / bpm) / division;
   const double swing = std::max(0.,std::min(.5,(double)pattern.swing));
   return std::max(1.,nominal * ((step % 2 == 0) ? (1. + swing) : (1. - swing)));
 }
 void refreshDurations() noexcept {
  cachedDurations[0]=durationFor(0);
  cachedDurations[1]=durationFor(1);
 }
public:
 void prepare(double sr) noexcept {
   sampleRate = std::max(8000.,sr);
   refreshDurations();
   reset();
 }
 void reset() noexcept { sampleInStep = 0.;stepIndex=0; }
 void setPattern(const SequencePattern& next) noexcept {
   pattern=next;
   pattern.length=std::max(1,std::min(64,pattern.length));
   pattern.gate=std::max(0.f,std::min(1.f,pattern.gate));
   pattern.swing=std::max(0.f,std::min(.5f,pattern.swing));
   pattern.tempo=std::max(20.f,std::min(300.f,pattern.tempo));
   pattern.mode=std::max(0,std::min(2,pattern.mode));
   stepIndex %= pattern.length;
   refreshDurations();
 }
 const SequencePattern& getPattern() const noexcept {return pattern;}
 int currentStep() const noexcept {return pattern.mode==0 ? -1 : stepIndex;}
 SequenceFrame tick() noexcept {
   if (pattern.mode == 0) return {};
   const auto& step=pattern.steps[(size_t)stepIndex];
   const double duration=cachedDurations[(size_t)(stepIndex&1)];
   SequenceFrame frame;
   frame.pitch=std::max(-24,std::min(24,step.pitch));
   frame.velocity=std::max(0.f,std::min(1.f,step.velocity));
   frame.modulation=pattern.mode==2 ? std::max(-1.f,std::min(1.f,step.modulation)) : 0.f;
   frame.gate=(sampleInStep < duration*(double)pattern.gate && frame.velocity>0.f)?1.f:0.f;
   frame.index=stepIndex;
   sampleInStep+=1.;
   if(sampleInStep>=duration) {
     sampleInStep-=duration;
     stepIndex=(stepIndex+1)%pattern.length;
   }
   return frame;
 }
};
inline SequencePattern defaultSequence() noexcept {
 SequencePattern p;
 constexpr int notes[16]={0,0,12,7,0,0,12,5,0,0,12,7,0,3,7,12};
 for(int i=0;i<sequenceMaxSteps;i++) {
   p.steps[(size_t)i].pitch=notes[i%16];
   p.steps[(size_t)i].velocity=(i%4==1)?0.70f:1.f;
 }
 return p;
}
} // namespace serenity
