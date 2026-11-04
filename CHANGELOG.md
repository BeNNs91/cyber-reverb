# CYBER-REVERB changelog

## 0.2.0 — First listening-tuning pass

- Default GUI opens at the minimum 900×600 size, with scaled controls and text.
- Internal sidechain displays a thin white detector-envelope trace in the main visualiser. It is hidden for Off and External sources.
- Small Room: six stereo early-reflection taps and a shorter 8-line diffuse tail; defaults to 0.85 s decay and 12 ms pre-delay.
- Hall: longer 16-line diffuse network for a smoother, spacious tail.
- Springy Plate: shorter, brighter cross-coupled tank paths with stronger allpass dispersion and less input diffusion for bounce and resonance. This is a creative plate/spring hybrid direction, not a physical spring model.
- Vintage adds more modulation and darker damping than before, with the existing smoothing.
- Updated factory presets for the three new engine identities.
- Previous Hall/Large Hall saved states migrate to Hall; previous Plate states remain in the plate slot. Existing parameter IDs and plugin identity are retained. Old presets necessarily sound different after DSP retuning.
- Valhalla Room is a listening reference for Small Room only. No Valhalla source code, captured data or matched preset measurements were used.

Validation: fractional delay-boundary regression, five sample rates, impulse decay, 30-second freeze holds, extreme automation, mono/stereo processing, non-finite input, state migration, sidechain analysis and GUI snapshot checks. Listening feedback and host-specific confirmation remain pending.

## 0.1.1

- Fixed out-of-bounds fractional delay reads at circular-buffer rollover.
- Added non-finite output recovery and expanded regression tests.
- Reduced default GUI from 1380×920 to 1080×720, including smaller scaled text.

## 0.1.0

- Initial JUCE VST3/Standalone build, three engines, controls, presets, ducking, meters and real-signal visualisation.
