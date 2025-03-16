**Current Tasks**

1. Layering (Tim)
  - _First get two of the same sounds playing:_
    - Play two sounds within same period
    - Need to add array values at each common time stamp
    - Need to normalise by dividing by 2 over the common period
  - _Can then scale up by layering multiple different sounds:_
    - Normalise by dividing by N interlapping sounds
  - _To be Aware of:_
    - Need to keep track of sample lengths
      - Need accurate tracking of sample start and end to safely normalise
    - Need to use DMA for speed and accuracy...
2. Sampling (Marco C)
  - _Want to record inputs over a time period_
    - Sampling starts and ends when button pressed
    - Can begin by testing with binary inputs
    - When sampling ends, loop should play with pulses at correct time stamps
3. Piezo Sensors (Marco)
  - When sensor pressed, Pico should read intensity of strike
  - Strike intensity should then be used to scale input wave amplitude
4. Filtering Final Output (Eoghan)
  - Want to be able to filter out frequencies at speaker input to change the sound
  - _Filter type?_
    - Bandpass ideally
    - Butterworth provides smooth output - alternatives?
    - If using butterworth - what order do we need?
    - Want to minimise complexity and required space on breadboard
5. SD Card

6. Direct Memory Access [DMA]
   
7. Screen
   - Can be used as an oscilloscope to display the speaker input
   - Can also be used to display sound level (dB)
   - Multiple options, each requiring significant coding...
