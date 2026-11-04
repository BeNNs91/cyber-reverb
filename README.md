# CYBER-REVERB

Windows x64 JUCE algorithmic reverb. VST3 and Standalone. **0.2.1 listening-tuning build**, intended for listening and Studio One routing tests before production use.

0.1.1 fixes an out-of-bounds fractional delay read at circular-buffer rollover, adds non-finite audio recovery, and reduces the default editor to 1080×720 with smaller labels and size-scaled readouts. A regression test reproduces the invalid boundary read in 0.1.0 and passes with the fix. The full processor is also stress-tested with mono/stereo audio, irregular blocks, live automation, a saturated visualiser FIFO, and non-finite input samples at all five supported sample rates. This fixes a verified memory bug; confirmation of the reported Studio One warning still requires retesting in the host.

See CHANGELOG.md for the engine redesign and the 0.2.1 Room reference-tuning pass. Select Room / Reference to audition the measured one-second Room setup; enable Wet Only for a wet-only comparison.

## Build

Dependencies: Windows 11, Visual Studio 2026 Build Tools with Desktop C++, Windows SDK, CMake 3.22+ and Git. CMake fetches JUCE `8.0.16-security`; a local JUCE checkout can be supplied to avoid downloading it. JUCE licensing applies to distribution; obtain the appropriate JUCE licence for your project.

```powershell
.\build.ps1
# Or reuse the shared Cyber checkout:
.\build.ps1 -JucePath C:/Users/benns/Documents/Codex/CyberPlugins/.dependencies/juce-8.0.16
```

For VS 2022, configure manually using `-G "Visual Studio 17 2022" -A x64`. The build script selects VS 2026. Build products:

* `build/CyberReverb_artefacts/Release/VST3/cyber-reverb.vst3` (copy the entire bundle folder)
* `build/CyberReverb_artefacts/Release/Standalone/cyber-reverb.exe`

Put the VST3 bundle in `C:/Program Files/Common Files/VST3`, beside the other Cyber plugins, then restart/rescan Studio One. Standalone audio settings are available in JUCE's Options menu; it processes live input, not an internal demo sound. Use headphones when testing microphone input with live output.

## Controls and workflow

Three engines: Small Room, Hall, Springy Plate. All support Modern and Vintage character. Input/output gain, pre-delay, size, RT60 target, diffusion, damping, width (0–200%), mix, wet HP/LP filters and modulation are automatable. Shift-drag gives fine adjustment and double-click restores a control's default. Rotary readouts can be typed.

Freeze fades out new excitation and moves the feedback towards unity while removing modulation/damping losses. Wet Only overrides the mix without changing its saved value. Natural lets tails decay; Cut smoothly mutes the wet output while selected, and Tail bypass passes dry while existing tails continue. The dedicated Bypass button and host bypass also use natural-tail bypass. Bypass retains input/output gain. Freeze is reported to the host as a one-hour tail; normal tails are reported as decay + 1 second.

Sidechain is Off by default. Internal detects the main input; External detects the optional sidechain bus. An absent external connection produces no ducking. The cyan activity lamp lights above 0.5 dB gain reduction. Ducking changes wet level only; the feedback tank keeps running.

Factory presets cover all six engine/character combinations, drums, vocals, synths and atmospheres. Save/Load uses `.cyberreverb` files under Documents/Cyber Audio/Presets/Reverb. DAW recall stores all parameters. Undo/redo covers parameter state; A/B is not implemented because no common A/B framework was found in the reviewed CyberMorph headers.

The display shows the most recent four seconds of audio, with stereo dry peaks in orange and wet peaks in cyan, an actual wet-left-channel FFT waterfall in blue/purple/cyan, and wet stereo sample points. It is a rolling measurement, not a predicted impulse response. At silence it remains still/empty. Analysis is enabled only while an editor is open. No audio recording or disk writes occur during processing.

## Architecture

`Source/ReverbDSP.h` is independent C++17 DSP. Small Room uses six stereo early-reflection taps and a short-path 8-line orthogonal Hadamard FDN; Hall uses 16 longer lines for a more diffuse tail. Springy Plate uses shorter, brighter tank delays, input allpasses and two cross-coupled allpass/delay/damping tanks inspired by Dattorro; it is an independent implementation, not a literal recreation of a specific hardware unit. All engines run continuously, with a 25 ms crossfade when selecting one. This preserves tails and avoids resetting delay memory at engine changes, at the cost of running all three engines. Size and pre-delay changes move interpolated taps and can produce musical pitch movement.

`PluginProcessor` owns the JUCE parameter tree, sidechain bus, smoothed controls, output filters and detector. Buffers are allocated in `prepareToPlay`; audio processing avoids heap allocation, mutexes, file I/O and GUI calls. A single-producer/single-consumer JUCE AbstractFifo transfers measurement samples to the editor; surplus samples are dropped if the GUI cannot keep up. Timing and meters use atomics. Denormals are disabled. Feedback guards limit extreme internal samples at ±4; these are emergency bounds rather than a master output limiter.

`PluginEditor` draws vector panels and controls using the supplied mockup as a reference. It opens at 900×600 and is resizable from 900×600 to 1536×1024 with fixed aspect ratio, includes an advanced-section toggle and updates at 30 FPS. Labels, buttons and rotary readouts scale down with the window. CyberMorph's JUCE/CMake/APVTS and custom rotary-control conventions were reviewed before implementation. No extra DSP dependency is required.

## Reproducible tests and benchmark

```powershell
ctest --test-dir build -C Release --output-on-failure
.\build\Release\ReverbDspTests.exe
.\build\ReverbIntegrationTests_artefacts\Release\ReverbIntegrationTests.exe C:/temp/reverb-preview.png
```

DSP tests render five-second impulse responses for all engines at 44.1/48/88.2/96/192 kHz, verify non-silent output, finite values and decaying late energy, perform 30-second freeze holds for each engine, then stress rapidly changing extremes with the same control-smoothing timescale as the processor. They print CPU as elapsed wall time / rendered audio duration ×100. Run Release builds on an idle machine, repeat three times and report median results alongside CPU model. All three tanks run for each selected-engine test, as in the plugin. These are DSP-core measurements; the GUI footer measures the complete callback budget as a percentage of real time, including callback scheduling/timing overhead, not the DAW's system-wide CPU meter.

Initial local measurement on an Intel Core i9-14900K: approximately **1.7% of one core at 48 kHz** for the DSP core, with all three tanks running. Full callback preview measured approximately 1.9% at default settings. These are local measurements, not a cross-machine performance guarantee. Raw results are in `docs/benchmark-i9-14900K.txt`.

Integration tests cover dry-path filter isolation, stereo channel preservation, parameter recall, disconnected and connected external sidechain, zero reported latency and optional editor PNG rendering.

## Studio One listening checklist

1. Insert on stereo audio and on mono audio; verify both output channels, dry gain and no added host latency.
2. Send a short impulse/snare and compare all six characters. Check ringing and tail density on vocals/pads as well as drums.
3. Automate pre-delay, size, decay, character and engine through their full ranges. Delay-time movement may shift pitch; there should be no reset clicks or missing audio blocks.
4. Enable the plugin sidechain in Studio One. Send a kick into that bus, choose External, and verify ducking with the main source silent and sounding. Remove the route and check graceful recovery. Repeat with Internal.
5. Freeze a quiet tail, leave it running for several minutes, and disengage. Verify bounded level and smooth transitions. Extreme sustained input can produce high wet levels: no master limiter is imposed.
6. Test Natural, Cut, Tail bypass, plugin Bypass, host bypass, stop/start, project save/reopen, preset save/load and Wet Only restoration.
7. Compare 44.1/48/88.2/96/192 kHz sessions, buffer sizes 32–2048 and multiple instances. Check GUI open/closed performance, resizing and scrolling display agreement with audio.

## Known limitations / acceptance still pending

This is an initial test implementation. Listening comparisons, perceptual RT60 calibration, long-duration freeze tests, Studio One's actual sidechain routing/automation and session recall require host testing. Passing numerical tests and compilation does not establish professional reverb quality. Decay is calculated from nominal delay traversal; diffusion, interpolation and damping can shorten the measured frequency-dependent tail. Vintage currently changes damping and modulation; it does not emulate a specific converter or introduce nonlinear saturation/bandwidth decimation. Cut mutes the output with a short fade and lets tank memory decay internally; returning to Natural quickly can reveal the remaining tail. The waterfall FFT uses a 1024-sample window, so bass frequency resolution is limited. FPS is fixed at 30; a 60 FPS option and A/B remain future work. User preset names are not saved as separate DAW metadata. Freeze at fractional delay lengths can slowly lose energy from interpolation, so it is not a mathematically lossless infinite hold. No CPU target is claimed until measured on the test machine.
