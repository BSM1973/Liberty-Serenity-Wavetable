# Serenity WAVETABLE HD — Validation haute polyphonie, v1.2.3

This pass addresses actual worst-case CPU scenarios measured in build 131: 32 notes, all four oscillators, unison ×4 or ×16, without reducing the configured polyphony or changing spectral data.

## Why this optimization can preserve sound quality

- Each MIDI note's base frequency is invariant during its voice lifetime. Cache the exact original std::pow expression in Voice::start(), and verify all 128 notes against the original expression.
- Detune offset, multiplicative unison step, amplitude normalization, and static phase/random/blend parameters depend on a block-constant Params. Compute once at the beginning of Engine::render(). Keep per-voice phase, warped waveform, frequency clamp, oscillator shape, FM and ring processing unchanged.
- The global base cutoff is shared across all notes if no modulation-matrix filter route is active; retain the original per-voice calculation when filterMod differs from zero.
- The HD table frame pair only depends on a voice's morphing position, not on the 1–16 unison phase/frequency offsets. Resolve frame pointers once per generator per sample; compute the mip and frequency-specific anti-alias interpolation separately for each unison oscillator.
- No re-sampling of 256 HD frames, no reduction of 2048 samples, no reduction of the 11 spectral mip levels, no change to saved presets/DAW state or the 32-note maximum.
- The baseline HdTable::sample() remains intact as a numerical reference for 24,000 optimized sampling cases, varying note frequency, phase, frame morph and sample rate, tolerance 2e-6.

## Acceptance

Both GitHub Actions jobs (macOS ARM64 and Windows x64) must pass CTest and publish real downloadable plugin archives and HD quality CSV reports. Compare within each platform against the build 131 results, particularly 48 kHz, 32 notes × 4 oscillators, unison ×4 and ×16. Hosted runner latencies vary; individual timing comparisons must be interpreted with care. High audio deadline misses remain a blocker for real-time qualification, even if builds are green.

No new features or UI changes. DAW host, audio hardware, real-time safety under external load and subjective A/B listening require additional verification.
