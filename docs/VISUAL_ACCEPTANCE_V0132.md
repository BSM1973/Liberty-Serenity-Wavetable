# v0.13.2 — reference comparison and stabilization

Compared all ten actual v0.13.1 English compact renders (1117 x 741) with the ten original reference PNGs (2048 x 1361). Layout broadly follows references, but graphic details still differ; different parameter states prevent a pixel equality assertion. Corrected visible deviations without new DSP or parameters:
- Filter response: logarithmic frequency grid, actual dB grid, filled combined linear transfer, both individual responses including inactive preview and cutoff markers. Series/parallel linear response follows current DSP mix formula at 48 kHz; excludes drive and per-voice modulations.
- Envelope and LFO fill, two-cycle preview like reference, actual synchronized LFO rate readout.
- Frequency, time, dB, ratio and discrete UI values display real units rather than normalized numbers.
- Pattern/sequence gradient bars, scales, actual pattern end indicators; matrix bipolar amount track and selector arrows.
- Settings: remove miswired hotspots for inactive tuning/velocity controls; align master level hit area.
- Native rendering, reference assets and complete sound engine unchanged. No new DSP functions; no new parameter IDs.
- Remaining: verify new captures after CI, compare French and English at actual DAW window size, further polish reference differences e.g. FX graphics, wavetable thumbnail depth and settings descriptions. Inactive controls must not be represented as working. No claim of pixel-perfect fidelity or release approval.
