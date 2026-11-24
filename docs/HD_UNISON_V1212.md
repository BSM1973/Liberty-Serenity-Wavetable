# HD Unison Acceleration v1.2.12

## Baseline evidence (v1.2.11, run 45369582252)
At 48 kHz / 128 samples, 32 notes / 4 generators / x16, mean audio budget: macOS scalar 93.11%, grouped 118.51%, NEON 123.28%; Windows scalar 317.42%, grouped 329.40%, SSE2 324.89%. Grouped and SIMD were slower. Previous "static" case also changed morph each block, and mixed first-trial peaks with average p99; v1.2.12 corrects both issues.

## Implementation
Modes 0/1/2 remain; mode 3 caches SoA lengths, masks, offsets and spectral blend at block preparation, and reads eight independent phases with exact interpolation order. Mode 4 uses isolated AVX2 gathers only after CPUID and OSXSAVE/XGETBV validation. No global AVX2 compiler flags. Dynamic pitch and remainder lanes use proven scalar path. All modes preserve 32 notes, four HD tables, x16 unison, all mips, and original accumulation order. Phase-dependent fractional indices are never reused incorrectly.

## Acceptance
Short comparison uses forward/reverse trials, worst-trial p99 and maximum peak, total overruns. New x8 and x12 tail cases. Separate endurance renders 60 seconds per mode 0 and 3 (and 4 when supported). Strict sample max delta <=2e-6, relative RMS <=1e-6. Performance is advisory on shared runners; no automatic activation. Compare each architecture independently, then verify real DAW callback deadlines on hardware. No release claim before reports and binaries exist.
