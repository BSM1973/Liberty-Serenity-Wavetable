# Serenity WAVETABLE 0.11.0 — Advanced Sound Engine

54 appended automation parameters. Factory presets reset new parameters to defaults. Existing parameter IDs preserved. Native reference layout retained.

- Six DAHDSR envelopes now include a continuous bipolar curvature control and velocity sensitivity; native envelope graph uses the same curve function as synthesis. Release begins from current value.
- Eight LFOs: independent host-tempo frequency synchronization, 12 straight/dotted/triplet divisions and smoothing. Uses host BPM with 120 BPM fallback; phase is continuous, not absolute transport-phase locking.
- MOD FX: selectable stereo chorus, flanger or six-stage phaser, rate, depth, signed feedback and wet mix. Delay buffers preallocated, fractional delay interpolation; no render-time allocation.
- Stereo three-band EQ: low shelf, parametric bell and high shelf, individually adjustable frequencies/gains, mid Q; RBJ biquads, coefficients calculated when parameter set is transferred. Native EQ response uses same coefficient equations at 48 kHz.
- Stereo-linked peak compressor: threshold, ratio, attack, release, makeup, parallel wet/dry. Static graph shows threshold/ratio transfer (before makeup and mix).
- Regression tests cover curve, tempo frequency, audible EQ/comp/mod FX differences and finite samples.
- New FX are neutral/bypassed by default. No redesign, no fixed images. Still pending: detailed visual comparison of all 10 pages, envelope looping, additional FX, host-specific DAW testing and signed installers.
