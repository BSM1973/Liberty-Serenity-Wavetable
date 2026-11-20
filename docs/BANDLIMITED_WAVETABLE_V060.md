# Serenity WAVETABLE — v0.6.0 | Wavetable anti-aliasing

## What is implemented
- Nine FFT-generated spectral mip levels for custom 512-sample wavetables, with harmonic ceilings 256, 128, 64, 32, 16, 8, 4, 2 and 1.
- Level selection for every oscillator and unison voice based on the effective note frequency and host sample rate, conservatively below Nyquist.
- Identical 8-frame morph and 4-oscillator architecture; all ten original GUI images unchanged.
- Precomputation from the edited signal occurs on the editor/host-state thread, never on the audio thread.
- Editing gestures publish their completed anti-aliased frame on mouse release. Menu transforms and WAV imports publish once per frame.
- Atomic, generation-checked handoff copies only changed frames at block boundaries; no spin locks, heap allocation, or FFT in the callback.
- Large mips and atomic buffers allocated in the processor constructor, avoiding 1-MB Windows stack overflow.
- Fixed memory bounded to 4 oscillators x 8 frames x 9 levels x 512 samples.
- Custom wavetable PCM persists in the host project/preset and is reconstructed into band-limited levels on state restore.

## Automated validation
- FFT harmonic suppression at different mip levels while retaining fundamental amplitude.
- Frequency-to-mip selection and high-note audible render in the real DSP engine.
- Existing MIDI sequencer/FFT wavetable tests and host state restore.
- JUCE editor constructor and all ten original screenshots in macOS/Windows CI.

## Scope and limits
- This is band-limited mip selection, not complete oversampling, nor yet an exhaustive alias/noise measurement across 44.1–192 kHz.
- A change in mip band at a harmonic threshold may be perceptible; smooth cross-band transitions and more advanced oversampling are future work.
- Waveform drawing updates the spectral sound on mouse release to keep the editor responsive and the audio thread safe.
- A signed or notarized installer is not included; artifacts are AU/VST3/Standalone developer builds.
- Preserve original skin images; no substitute drawings, no overlapping controls, and reliable French UTF-8 labels.
