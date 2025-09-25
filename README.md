# GP6-DK Electronic Drum Kit

_Status_: Team lead of group project (4 members), completed. Awarded the highest grade in the year group (87.5%).

_Tech Stack_: Raspberry Pi Pico 2W (RP2350 MCU) and Pico SDK, C/C++ (no OS, real time fixed-point audio mixing), CMake, chained DMA double-buffering, piezoelectric signal conditioning and multiplexing, SPI, audio signal conditioning and amplification, FFmpeg.

### Intro

The brief for 3C10 Circuit and Systems Design was open ended. “Design a system that can sample or create audio signals, process them, and convert the processed signals for output to an audio device”. We wanted to create a kit that would allow the user to create drum beats live. While other groups went with the sequencer approach (with preprogrammed time-sequenced outputs), we wanted a more powerful platform: real-time drumming, full syncopation, velocity-sensitive inputs, and layering of many synthesiser and drum tracks. Since this all had to run on a single tiny Pi Pico, this project was going to live or die by the software’s performance optimisation.


<p align="center">
  <img src="https://github.com/user-attachments/assets/8eb29894-2832-4018-83ee-eccb14d2ca60" alt="GP6-DK" style="width:50%; height:auto;">
</p>

### What we built
We built the GP6-DK: a six-pad, velocity sensitive electronic drum kit based around the Raspberry Pi Pico 2W microcontroller. The GP6-DK allows the user to record, loop, and overdub synthesiser and drum beats from a sample bank of 12 sounds.  High quality real-time mixed audio output is delivered via a 12-bit Digital to Analog Converter (DAC), interfaced over SPI and driven by a timed Direct Memory Access (DMA) system at 44.1 kHz and < 3 ms latency. The entire architecture is designed to minimise CPU intervention, keeping inputs snappy and allowing the user to play up to 50 sounds simultaneously. Testers reported the kit being genuinely enjoyable to use.

#### Hardware

- _**Pads and sensing**_: Six piezoelectric pads with a tuned charge amplifier (NE5532 inverting amplifier + buffer) circuit and clamp network. Two high speed analog multiplexers slice the six inputs for the Pico ADC.
- _**DAC and output**_: 12-bit MCP4922 over SPI with 16-bit command framing. LM386N-4 power amplifier with volume control and output conditioning to drive the speaker.

#### Firmware
- _**Event-based loop engine**_: Software avoids manipulating raw audio. On a piezo hit, the Piezo class measures the peak over a 1 ms window, debounces over 50 ms, and stores the hit as a timestamped event. Playback re-triggers the events each loop.
- _**Polyphonic mixing**_: Voice structs track drum_id, velocity, position index, and active status. Mixing involves velocity scaling, adding voices, and fixed point normalisation to 12-bit before DAC formatting.
- _**DMA Streaming**_: PWM wraps at exactly 44.1 kHz to trigger a Data Request (DREQ), main DMA streams from the active mixed buffer. At the buffer end, a chain DMA atomically updates the main DMA read address and restarts it, resulting in glitchless audio.

### Impact

This project shows that high-quality and low latency multi-voice synthesis is perfectly achievable to the hobbyist on a very low cost MCU (from as low as €5) by offloading timing-critical I/O to DMA and representing music as events and voices. It’s a tiny platform that still has the performance headroom to be extended with more features.
