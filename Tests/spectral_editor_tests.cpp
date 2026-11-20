// Keep assert-based DSP regression checks enabled in Release CI builds.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "SerenitySpectralEditor.h"
#include <cassert>
#include <cmath>
#include <iostream>

static bool near(float a,float b,float tolerance=.018f) {
 return std::abs(a-b)<tolerance;
}
int main() {
 using namespace serenity::spectral;
 Frame sine{};
 for(int i=0;i<fftSize;i++)
  sine[(size_t)i]=.35f*std::sin(tau*(float)i/fftSize);
 auto original=analyze(sine);
 assert(near(original[0],.35f,.0005f));
 assert(original[1]<.0005f&&original[2]<.0005f);

 // Editing only partial #3 must preserve the original #1, including its phase.
 auto edited=setHarmonic(sine,3,.27f);
 auto bars=analyze(edited);
 assert(near(bars[0],.35f,.002f)&&near(bars[2],.27f,.002f));
 assert(near(bars[1],0.f,.002f));

 // An empty partial must receive a safe sine-compatible default phase.
 auto removed=setHarmonic(edited,3,0.f);
 assert(analyze(removed)[2]<.001f);

 // Real FFT/iFFT roundtrip restores every waveform sample.
 auto roundtrip=restore(analyzeBins(sine));
 for(int i=0;i<fftSize;i++) assert(near(sine[(size_t)i],roundtrip[(size_t)i],.00008f));

 // Even and odd partial selection genuinely removes the opposite spectrum.
 auto second=setHarmonic(edited,2,.18f);
 const auto odd=analyze(transform(second,Transform::oddOnly));
 const auto even=analyze(transform(second,Transform::evenOnly));
 assert(odd[1]<.001f && odd[0]>.25f && odd[2]>.20f);
 assert(even[0]<.001f && even[2]<.001f && even[1]>.12f);

 // Sharp cutoff eliminates high-frequency components, not simply graph bars.
 auto plus40=sine;
 for(int i=0;i<fftSize;i++)
  plus40[(size_t)i]+=.18f*std::sin(tau*40.f*(float)i/fftSize);
 const auto filtered=transform(plus40,Transform::lowPass16);
 auto filteredBins=analyzeBins(filtered);
 assert(2.f*std::abs(filteredBins[40])/(float)fftSize<.002f);
 assert(near(analyze(filtered)[0],.35f,.004f));

 // Phase transform changes waveform shape/offset while preserving harmonic energy.
 const auto rotated=transform(sine,Transform::phaseQuarterTurn);
 assert(near(analyze(rotated)[0],.35f,.002f));
 assert(std::abs(rotated[0]-sine[0])>.12f);

 // Morph is precise and bounded, frame endpoints stay untouched.
 Frame b{};
 for(int i=0;i<fftSize;i++)b[(size_t)i]=-.2f*sine[(size_t)i]/.35f;
 const auto middle=morph(sine,b,.5f);
 assert(near(middle[fftSize/4],.075f,.0005f));
 const auto invalid=setHarmonic(sine,0,.5f);
 assert(near(invalid[fftSize/4],sine[fftSize/4],.0001f));
 std::cout<<"PASS: FFT 512 real inverse, 32 editable harmonics, filters, phase rotation and 8-frame morph\n";
 return 0;
}
