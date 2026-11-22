# WAVETABLE HD QUALITY CONTROL / v1.2.1

Scope: tests and reporting only. No modification to synthesis, UI, presets or parameter identifiers.

Baseline v1.2.0 build 129 succeeded on macOS ARM64 and Windows x64, including the functional HD tests and four-bank storage checks. The new test `serenity_hd_quality` measures the full Engine render path (not a DAW or physical audio-device callback).

- Four independent tables at 256 × 2048: exact raw in-range PCM, numerical interpolation against adjacent frames, analytical partial reference, spectral rejection at high pitch.
- 32 active notes with four generators on legacy and HD banks; 44.1/48/96 kHz and 64/256/512 samples. Sustained 2 seconds with four unison oscillators per generator, short maximum-unison (16 per generator) stress, 10 seconds HD at 32 notes and unison 1. Duration means rendered audio, not elapsed wall time.
- Mean, median, 95th and 99th percentile, worst block, deadline, budget percentage and count over deadline. Runtime timings on shared CI runners are diagnostic; no guarantee of actual hardware real-time behavior.
- OS working set/RSS and peak, raw + mip payload, preparation time. Memory readings include the entire process, not just HD. Four maximal raw+spectral payloads are 25 MiB before container/control overhead.
- Count ordinary C++ heap allocations in the measured render path. Direct malloc/aligned allocations are not instrumented.
- Nonfinite output, wrong voice count, silent output, runaway output, ordinary render allocation, loss of preserved source samples or low numeric fidelity fail the tests. Concurrent exchange stress uses 20,000 publications.
- Per-block overruns are explicitly reported as REVIEW REQUIRED, not hidden or treated as a build failure due to shared-runner scheduling.
- Physical DAW tests, playing during import, long-session memory retention including undo history, MIDI timing, audible A/B listening and maximum-polyphony performance on target hardware still require separate verification.

Artifacts per platform: hd-quality.csv, hd-quality-summary.txt and LastTest.log. Review actual measurements before declaring performance validated. No further features authorized by this change.
