# Serenity WAVETABLE v0.9.2 — Unison, phase and macro DSP wiring

This pass preserves the existing native JUCE layout and retains previous AU/VST3 parameter IDs.

## Connected to real synthesis
- OSCILLATORS / UNISON · PHASE: BLEND (outer-unison voice balance), PHASE (cyclic phase offset) and RAND PH (deterministic per-note phase offset).
- Independent control for A/B/C/D: 12 APVTS parameters appended without reordering existing host parameters.
- Existing knob coordinates are preserved, with parameter values, mouse editing, MIDI Learn and session-state persistence.
- All eight macros now affect audio: the previously inactive controls map WARMTH to drive, TEXTURE to wavetable position, ENERGY to master gain and SERENITY to reverb mix. 0.5 is neutral.
- Phase jitter is generated at note-on, not per sample. No additional allocations or locks inside render().

## Verification
- Dedicated C++ regression test checks that BLEND, PHASE and RAND PH alter samples and neutral settings preserve the original audio.
- GitHub Actions builds macOS ARM64 AU/VST3/Standalone and Windows x64 VST3/Standalone and publishes downloadable test ZIPs if tests pass.
- Binaries remain unsigned and unnotarized.

## Remaining work
Many FM/RING controls, multi-envelope/LFO editing controls and EFFECTS page widgets remain unconnected placeholders. Visual pixel-level fidelity, DAW testing and commercial signing must still be validated.
