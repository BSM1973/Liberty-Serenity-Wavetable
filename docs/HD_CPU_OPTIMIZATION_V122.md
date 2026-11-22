# Serenity WAVETABLE HD v1.2.2 — CPU performance intervention

Status: build 130 of v1.2.1 passed automated functional and 23 sustained benchmark cases on both macOS ARM64 and Windows x64. Its reports flagged realtime overruns; the sound engine cannot yet be declared performance-qualified.

## Measurements from build 130 (GitHub-hosted runners)

Mean render-time % of theoretical block deadline; 32 simultaneous notes, four active oscillators:

| 48 kHz | macOS ARM64 | Windows x64 |
|---|---:|---:|
| HD unison x1, buffer 256 | 36.26% | 89.38% |
| HD unison x4, buffer 256 | 93.93% | 283.23% |
| HD unison x16, buffer 128 | 281.88% | 1004.41% |

At 96 kHz, unison x1 / 32 notes / 256 samples, HD mean budget: macOS 91.91%, Windows 174.15%.

Raw + mip payload: 25 MiB for four 256 × 2048 banks. Process peak RSS: 44.17 MiB macOS and 34.27 MiB Windows. Analytic waveform test: RMS error 0.001607, signal-to-error 45.75 dB, high partial rejection −172.64 dB. Zero ordinary C++ allocations detected during measured Engine render operations.

## Source changes in v1.2.2

- Constant-time mip-level spectral offsets and stride; no per-sample dynamic offset loop.
- Normalize oscillator phase once, compute both frame pointers once and avoid duplicate per-mip floor/phase normalization.
- Cache MIDI base frequency once per active voice per sample (instead of four times).
- Hoist FM pitch multiplication and detune exponential outside the unison iteration; use a multiplicative pitch progression over unison oscillator voices.
- 256-frame raw PCM, 2048-point cycles, spectral levels, filters, user-state format, synthesis voices and automation parameter indices unchanged.
- Existing HD fidelity, numerical, stress, and per-platform quality metrics remain mandatory.

No arbitrary CPU-limiting polyphony reduction or audible quality reduction. CI reports are diagnostic, not physical DAW realtime certification. Compare the v1.2.2 CSV against v1.2.1 on matched platforms after completion. Hardware tests and listening tests remain necessary.
