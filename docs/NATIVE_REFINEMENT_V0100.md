# Serenity WAVETABLE 0.10.0 — Cross modulation, modulation and FX

- Cross modulation: B to A, C to A, C to B; phase or frequency modulation, ring and AM using the previous sample of enabled oscillator sources. No allocation inside the render loop.
- Six independently editable DAHDSR envelopes (ENV 1 controls amplitude; ENV 2–6 are modulation matrix sources).
- Eight independently adjustable free-running LFOs, five shapes, rate, phase, bipolar/unipolar output; real curves use the same shape function as DSP. LFO 1 also has direct modulation depth.
- Effects: delay time, feedback and tone; reverb feedback; stereo width; tremolo/autopan; ring effect frequency and mix. Corrected hit targets for delay/reverb mix and other supported knobs.
- 79 appended APVTS parameters; existing IDs and plugin layout kept. Factory presets reset new parameters to defaults. The older shared envelope and derived LFO behavior has been replaced, so existing modulated presets may sound different.
- Inactive knobs display a dash instead of an invented numeric value. Route count reflects the actual modulation matrix.
- Existing reference UI remains the design target; no generated or static images.
- Remaining: envelope curve/velocity/loop, LFO sync/fade/smoothing, advanced effects and detailed visual comparison of all pages; host tests and notarization.
- Regression tests extend the existing oscillator test to cover cross modulation, effects, delay/hold and matrix LFO behavior. CI publishes development artifacts only on passing builds.
