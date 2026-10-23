# Volca Inspired Bass (prototype)

Original mono bass synth inspired by the general architecture of classic analog bass synths and Korg Volca Bass. Not affiliated with or endorsed by Korg. No copied interface artwork, firmware, or samples.

## Features
- Three independently mixed saw/square oscillators, detune, octave shift
- Resonant 2-pole lowpass, filter envelope amount, soft drive
- Mono MIDI note input, velocity, ADSR-style amplifier envelope
- 15 automatable parameters, preset state serialization
- JUCE VST3 for Windows x64, built by GitHub Actions

## Build without installing a compiler locally
1. Create a GitHub repository and upload all files, preserving `.github/workflows/windows-vst3.yml`.
2. In Actions select **Build Windows VST3** > **Run workflow**.
3. Download the artifact named `VolcaInspired-Windows-VST3` from the completed run.
4. Copy the `.vst3` bundle to `C:\Program Files\Common Files\VST3\` and rescan plugins in your DAW.

## Important
This is an **untested prototype**, not an authenticated Korg emulation. The workflow is supplied but has not been run. FL Studio 11 VST3 loading depends on the user's actual bridging/wrapper configuration. This version is 64-bit only. Filter is a simplified resonant two-pole design, not an exact analog circuit model. The envelope is an approximation and should be tested at various sample rates and buffer sizes. MIDI note priority is last-note, without legato/glide or an onboard sequencer.
