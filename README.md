# Serenity WAVETABLE — Liberty Music Software

**Serenity WAVETABLE** is a next-generation wavetable synthesizer project, built in C++17 / JUCE 8.0.10.

> **Development status: v0.1 source package prepared, not yet deployed as a validated macOS/Windows plugin.**
> A working source ZIP was prepared in the associated ChatGPT project conversation. GitHub source deployment is not complete yet; this README does **not** claim that the full plugin is present.

## Visual design

The 10 user-supplied screenshots are the strict UI reference:
1. OSCILLATORS
2. WAVETABLE EDITOR
3. FILTERS
4. MODULATION
5. EFFECTS
6. ARPEGGIATOR
7. SEQUENCER
8. MATRIX
9. PRESETS
10. SETTINGS

**Do not redesign or replace these screens with a different UI.** Preserve their exact layout hierarchy, night-blue / graphite palette, turquoise and violet accents, gold active states, panels and typography. Ensure no overlaps, corrupted glyphs or inaccessible controls.

## Architecture and targets

- Native JUCE/C++ audio instrument — **no PluginMaker.ai dependency**.
- macOS Apple Silicon: AU, VST3 and standalone targets.
- Windows x64: VST3 and standalone targets.
- FR/EN language selector, French by default, with correctly accented labels.
- MIDI Learn: map CC to supported parameters and persist mappings in plugin state.
- Factory presets: 100 target sounds across the specified 10 categories.
- Quality gates: actual DSP processing, usable UI, MIDI/audio tests and DAW validation.

## Phased development

A preliminary v0.1 native synthesis core has been developed and its headless DSP tests passed. This does **not** certify the JUCE build or satisfy all requested commercial features.

The requested end state includes three wavetable oscillators, a fourth flexible generator, custom wavetable editor, dual multimode filters, six envelopes, eight LFOs, 64 matrix routings, 30 effects, 64-step sequencer, arpeggiator, eight macros and 100 commercially validated presets.

These are **development targets**, not claims of implemented features.

## Source status

The complete v0.1 ZIP (including the ten UI reference images, JUCE files and DSP tests) is available as a ChatGPT conversation attachment. It is not currently mirrored here. The repository will not be represented as buildable until all required source and image assets have been uploaded.

Serenity WAVETABLE — Shape the Sound. Feel the Freedom.
