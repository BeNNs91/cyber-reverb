# CYBER-REVERB — Development Requirements

**Project:** CYBER-REVERB  
**Framework:** C++ / JUCE  
**Format:** VST3 and Standalone  
**Platform:** Windows 11  
**Primary DAW:** Studio One  
**Suite:** Cyber Audio Plugins

## 1. Project Objectives

Build a professional-quality, lightweight algorithmic reverb plugin with a modern, compact GUI consistent with the existing Cyber plugin suite.

**Primary priorities:**
1. Exceptionally clean reverb processing.
2. Low CPU consumption, even with multiple instances.
3. Three reverb algorithms, each with Modern and Vintage character.
4. Real-time visual feedback representing the actual processed signal.
5. External and internal sidechain ducking.
6. Smooth parameter adjustments without clicks, pops or audio interruptions.
7. Minimal latency and reliable DAW integration.

## 2. Reverb Engines

Implement three independently tuned reverb algorithms.

### 2.1 Hall
- Smooth, natural, spacious reverb.
- Controlled early reflections and dense late decay.
- Suitable for vocals, snares, synths and percussion.
- Avoid excessive metallic ringing or resonances.
- Recommended architecture: feedback delay network (FDN).

### 2.2 Large Hall
- Expansive, immersive acoustic space.
- Longer, richer and more diffuse reverb tails.
- Strong stereo spaciousness.
- Suitable for atmospheres, pads, cinematic effects and DnB transitions.
- Expanded FDN with carefully tuned diffusion and damping.

### 2.3 Plate
- Dense, smooth reverb with a fast buildup.
- Bright, polished response.
- Suitable for snares, drums, vocals and synthesizers.
- Dattorro-inspired plate topology.
- Avoid harsh metallic resonances unless intentionally introduced in Vintage mode.

## 3. Modern and Vintage Characters

Every reverb algorithm must support both character modes.

**Modern**
- Transparent processing.
- Clean, smooth reverb tails.
- Minimal unwanted coloration.
- Controlled high-frequency response.
- Excellent stereo clarity.
- Default character mode.

**Vintage**
- Warmer, darker tone.
- Gentle modulation and subtle movement.
- More distinctive resonances and coloration.
- Inspired by older digital and hardware reverbs.
- Optional subtle bandwidth reduction where appropriate.
- No unintentional clipping, clicks or harsh aliasing.

Switching modes must not create audible discontinuities.

## 4. Main Reverb Controls

| Parameter | Proposed range | Default |
|---|---|---|
| Pre-Delay | 0–250 ms | 24 ms |
| Size | 0–100% | 65% |
| Decay | 0.1–30 seconds | 2.8 seconds |
| Diffusion | 0–100% | 75% |
| Damping | 0–100% | 50% |
| Stereo Width | 0–200% | 100% |
| Dry/Wet Mix | 0–100% | 35% |
| Input Gain | -24 to +24 dB | 0 dB |
| Output Gain | -24 to +24 dB | 0 dB |

Requirements:
- Smooth continuous parameter changes.
- No clicks or abrupt jumps.
- Parameter automation from the DAW.
- Appropriate nonlinear parameter scaling.
- Preserve stable reverb behaviour at extreme settings.

## 5. Wet Signal Filters

Provide independent filters applied to the reverb signal.

**High-pass filter**
- 20 Hz–2 kHz.
- Default: 120 Hz.
- Adjustable cutoff frequency.

**Low-pass filter**
- 1 kHz–20 kHz.
- Default: 12 kHz.
- Adjustable cutoff frequency.

Additional requirements:
- Filters must not unintentionally affect the dry signal.
- Smooth cutoff automation.
- Stable filtering at all supported sample rates.
- Show simple frequency-response indicators in the GUI.

## 6. Sidechain and Ducking

Implement internal and external sidechain functionality.

**Sidechain sources**
- Internal: detect transients from the plugin's main input.
- External: receive an independent DAW sidechain input.
- Off: disable ducking.

**Controls**

| Parameter | Proposed range | Default |
|---|---|---|
| Threshold | -60 to 0 dB | -24 dB |
| Ratio | 1:1 to 20:1 | 4:1 |
| Attack | 0.1–100 ms | 5 ms |
| Release | 20–2000 ms | 250 ms |
| Duck Amount | 0–100% | 70% |

Requirements:
- Reduce wet reverb level when the detector is triggered.
- Restore the wet signal smoothly when the trigger falls.
- Maintain the underlying reverb tail while ducking.
- Do not apply ducking to the dry signal.
- Provide a visible ducking activity indicator.
- Handle absent external sidechain input gracefully.
- Expose a configurable external sidechain bus through JUCE.

## 7. Reverb Visualiser

The visualiser is the central feature of the GUI.

**Main display**
- Large horizontal processed-audio display.
- Orange for incoming dry audio.
- Cyan for the wet reverb signal.
- Purple/blue for spectral energy and tail density.
- Display actual audio measurements, not decorative animation.
- Show the decay and evolution of the reverb.
- Include time and frequency reference markers where practical.

**Stereo display**
- Compact stereo-field visualisation on the right of the main display.
- Show left/right channel energy and stereo correlation or spread.
- Reflect the actual processed signal.

**Technical requirements**
- Visualisation must not interfere with audio processing.
- Use lock-free or similarly real-time-safe transfer of audio measurements.
- Prefer approximately 30 FPS initially, configurable up to 60 FPS if efficient.
- Avoid unnecessary GPU or CPU overhead.
- Reduce processing when the GUI is closed.

## 8. Freeze and Tail Behaviour

**Freeze**
- Sustain the current reverb energy.
- Prevent uncontrolled feedback buildup.
- Smoothly engage/disengage.
- Useful for drones, atmospheres and transitions.

**Wet Only**
- Output 100% wet signal.
- Designed for DAW send/return workflows.
- Preserve underlying user mix setting when toggled off.

**Tail behaviour**
- Natural: allow reverb to decay after input stops.
- Cut: immediately mute the wet tail when explicitly requested, with a short click-free fade.
- Bypass: allow an optional natural-tail bypass.
- Avoid audio clicks when stopping playback or changing presets.

## 9. Advanced Reverb Controls

Provide a collapsible advanced section containing:

- Modulation depth: 0–100%.
- Modulation rate: approximately 0.01–5 Hz.
- Tail behaviour selection.
- Algorithm-specific advanced controls where useful in future updates.

The default advanced controls should be subtle and musical.

## 10. GUI Requirements

**Design direction**
- Same dark futuristic theme as CyberMorph, CyberClip and other Cyber plugins.
- Charcoal/black background.
- Cyan accents for active controls.
- Orange accents for Vintage mode and dry audio.
- Compact and clean.
- Minimal unnecessary padding.
- Clear parameter labels and numerical readouts.
- Resizable interface.
- Scalable interface elements.

**Layout**

Top bar:
- CYBER-REVERB branding.
- Preset name and previous/next navigation.
- Save preset.
- Reverb algorithm buttons: Hall, Large Hall, Plate.
- Modern/Vintage character selector.

Centre:
- Large processed reverb visualiser.
- Integrated stereo-field display.
- Input gain and stereo input meters on the left.
- Output gain and stereo output meters on the right.

Main controls:
- Pre-Delay.
- Size.
- Decay.
- Diffusion.
- Damping.
- Width.
- Mix.
- Freeze.
- Wet Only.

Bottom:
- Wet HP/LP filters.
- Sidechain and ducking controls.
- Collapsible advanced controls.

Footer:
- Current reverb engine.
- Active character mode.
- Optional lightweight performance diagnostics.

The supplied GUI mockup should be treated as the visual design reference, not as an exact pixel-for-pixel implementation.

## 11. DSP Architecture and Performance

**Architecture**
- Separate DSP from GUI.
- Modular algorithm implementations.
- Share reusable delay, filter, modulation and parameter-smoothing components.
- Use preallocated buffers.
- Avoid heap allocation, blocking locks and disk access in the audio callback.
- Process stereo correctly.
- Prevent denormal-related CPU spikes.
- Ensure reverb feedback remains stable.

**CPU target**
- Aim for under 1–2% of one modern desktop CPU core per instance at 48 kHz, measured with a repeatable benchmark.
- Prioritise audio quality over hitting this target when necessary.
- Benchmark Hall, Large Hall and Plate independently.
- Ensure the visualiser has minimal overhead.

**Latency**
- Target zero additional algorithmic latency.
- Pre-delay is an intentional audio effect, not DAW processing latency.
- Avoid unnecessary lookahead or buffering.

**Sample rates**
- Support 44.1, 48, 88.2, 96 and 192 kHz.
- Ensure decay times and modulation remain consistent across sample rates.

## 12. Presets and DAW Integration

- Factory presets for all three engines and both characters.
- Presets for drums, snares, vocals, synths and atmospheres.
- Previous/next preset buttons.
- User preset save/load.
- Full DAW state recall.
- Parameter automation.
- Sidechain bus integration.
- Undo/redo where practical.
- A/B comparison if supported by the shared Cyber framework.
- Correct handling of sample-rate, buffer-size and session changes.

## 13. Testing and Acceptance Criteria

The plugin must be tested for:

1. Click-free parameter changes and automation.
2. Smooth reverb tails without excessive ringing.
3. Consistent decay times across supported sample rates.
4. Correct sidechain response and routing in Studio One.
5. No uncontrolled feedback or numerical instability.
6. Stereo compatibility and mono-input behaviour.
7. Freeze stability.
8. Tail preservation and bypass behaviour.
9. Accurate wet/dry filtering and metering.
10. Low CPU usage across multiple instances.
11. Reliable preset loading and DAW project recall.
12. GUI resizing and visualiser performance.

For DSP quality, use impulse-response analysis, listening tests and comparative reference testing rather than treating successful compilation as proof of audio quality.

## 14. Recommended Development Phases

**Phase 1 — Foundation**
- Set up JUCE VST3 and Standalone project.
- Implement Hall engine.
- Add fundamental reverb controls.
- Create initial Cyber-themed GUI.
- Verify audio stability and build process.

**Phase 2 — Algorithms**
- Implement Large Hall and Plate.
- Tune each algorithm independently.
- Introduce Modern and Vintage character variations.
- Test transitions and tail quality.

**Phase 3 — Processing**
- Implement HP/LP filtering.
- Add internal and external sidechain ducking.
- Add Freeze and Wet Only.
- Implement input/output gain and meters.

**Phase 4 — Visualisation**
- Implement processed-signal visualiser.
- Add stereo-field display.
- Optimise visualisation CPU usage.

**Phase 5 — Polish**
- Factory presets.
- GUI refinements.
- Parameter automation.
- CPU optimisation.
- Reverb quality and stability tests.
- Studio One validation.

## 15. Instructions for Codex

- Review the architecture and conventions of the existing Cyber plugin projects before implementation.
- Create a dedicated Git feature branch.
- Keep changes modular and maintainable.
- Prioritise the reverb's sonic quality before cosmetic refinements.
- Do not introduce unnecessarily complex DSP dependencies.
- Document build commands, dependencies and architecture in the README.
- Include reproducible testing and CPU benchmarking procedures.
- Commit development milestones separately so algorithms can be compared and rolled back.
- Do not silently remove requested features.
- Report implementation limitations and DSP trade-offs clearly.
- Use the GUI mockup as the design reference.

**Final goal:** A compact, polished, CPU-efficient reverb with six distinct sonic characters, musical ducking and excellent visual feedback that becomes the default reverb within the Cyber Audio workflow.