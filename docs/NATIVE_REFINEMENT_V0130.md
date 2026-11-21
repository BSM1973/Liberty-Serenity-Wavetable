# Serenity WAVETABLE v0.13.0 — Visual Perfection

UI-only pass: DSP source and factory definitions unchanged. No new parameter IDs. Processor changes only expose true per-channel audio peaks and host BPM via atomics.
- Shared graphite/titanium gradient, higher text contrast, bounded text clipping, knob label/value cells, vector favorite stars, portable CMD labels.
- Independent left/right peak bar segments with GUI-only decay; real scope and spectrum retained, no full-window animation.
- Fits actual usable desktop rectangle with proportional scaling, no enforced minimum that can exceed desktop; existing fixed non-resizable/no scrolling UI.
- Correct selected oscillator label, harmonic graph label, matrix page/slot counts with display matching click targets, sequencer and ARP step page display and actual host BPM/division; macro badges separated from labels.
- Settings text spacing and status, preset glyphs, whole-source rail fits its own row. Hover no longer persists after exit.
- Tests calculate fit for 1280x800, 1440x900, 1024x768; render all 10 pages at default size and 20 compact French/English previews. Pixel tests are smoke tests, not pixel-exact screenshot comparisons, OCR, or global collision detection. All 10 source PNGs stay unchanged in Assets/UI; binary preview retrieval was unavailable in this session, so no claim of exact visual comparison or complete French translation.
- Native UI only (no screenshot skins used as backgrounds). CI publishes real rendered 30 preview PNGs on macOS after passing tests. Build artifacts only after validation. Real host/window chrome and Retina testing remain necessary.
