# Room reference tuning — 0.2.1

Independently implemented Room DSP, tuned using the user's dry snare and wet reference exports. No proprietary source code, preset internals or reference audio is distributed here. The reference plugin's exact settings and gain staging were not supplied; this is a starting point for listening feedback, not a reconstruction of its complete algorithm.

Comparison conditions: 44.1 kHz stereo; identical dry input; Cyber Room Modern, size 50%, decay 1 s, pre-delay 0 ms, modulation off, diffusion 75%, damping 50%, width 100%, wet filters 120 Hz/12 kHz, ducking off and 100% wet. Room / Reference loads these controls with a practical insert mix of 35%; enable Wet Only to audition the measured wet-only setup.

| Measurement | Wet reference | Cyber 0.2.0 | Cyber 0.2.1 |
|---|---:|---:|---:|
| Estimated broadband RT60 | 0.976 s | 0.834 s | 1.043 s |
| Export RMS | -29.48 dBFS | -38.29 dBFS | -29.25 dBFS |
| Energy within first 20 ms | 10.85% | 29.02% | 5.72% |
| Left/right correlation | 0.014 | 0.175 | -0.005 |
| Side/mid RMS ratio | 0.987 | 0.839 | 1.005 |
| Spectral energy from 2–8 kHz | 13.75% | 12.34% | 10.68% |

The measured decay, wet level and stereo spread are closer. The new Room places less energy upfront than the reference and remains darker in the upper frequencies. These are explicit remaining differences for the user's next listening pass.

RT60 is estimated by fitting the -5 to -35 dB portion of the reverse-integrated energy curve. A snare is not an ideal impulse, so these figures describe this particular rendered test, not all sounds or parameter settings. Spectral energy fractions use 2048-point Hann windows with 1024-sample hops. Files were not normalized. The matched RMS levels do not prove equal perceived loudness.

Offline render command:

```powershell
.\build\ReverbIntegrationTests_artefacts\Release\ReverbIntegrationTests.exe --render-room dry-snare.wav cyber-room.wav
```

The output path must not already exist. The render uses the full processor, including the wet filters, at the settings above. All standard sample-rate, numerical-stability, freeze, automation and state checks also pass for this build.
