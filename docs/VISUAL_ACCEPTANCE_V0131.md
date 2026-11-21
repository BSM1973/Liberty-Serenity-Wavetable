# Visual acceptance audit — v0.13.1

Audited actual macOS v0.13.0 build 123 screenshots, ten FR compact pages at 1117 x 741. Build 123 passed both operating systems and published 30 PNGs and binaries.

Failure evidence:
- Filters: type selectors overlapped cutoff/resonance knobs.
- Effects: type/effect/mode selectors overlapped knobs; FX matrix final row too close to bottom.
- Sequencer: numeric step labels over bright bars; arp/seq values replaced by dashes.
- Settings: truncated long help text; English strings in FR on multiple pages.
- Header and macro areas: minor spacing corrections.
- Corrected selector/knob positions and hit areas, compact labels, step numbers, actual step/gate/swing display, 128 native labels mapped through UTF-8 French translation. Translation coverage is not yet exhaustive.
- DSP header, presets, parameter definitions and rendering algorithm unchanged. Existing regression tests remain active.
- Publish original ten reference PNGs separately alongside new native preview archive, enabling direct comparison without using reference images as runtime backgrounds.

Release acceptance still requires review of new screenshots (both languages), an actual comparison with all 10 original images, a host test on a laptop and auditory verification. Smoke tests do not prove absence of every collision or missing glyph. Do not call final until these pass.
