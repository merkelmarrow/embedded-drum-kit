// src/audio.hpp

#pragma once

#include "configs.hpp"
#include <array>
#include <cstdint>
#include <hardware/spi.h>

struct DrumSample {
  const int16_t *data;
  uint32_t length;
};

// represents an actively playing instance of a sound
struct Voice {
  bool active = false;
  uint8_t drum_id = 0;      // which drum sound is playing, ie. kick or snare
  uint32_t position = 0;    // position in the sound array
  uint16_t velocity = 1000; // playback velocity from the piezos
};

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine() = default;

  void init();

  void testVoices();

  // uses pre-normalised velocity to avoid looped sounds being waaay louder
  void triggerVoiceFromLoop(uint8_t drum_id, uint16_t normalized_velocity);

  void playSound(uint8_t drum_id, uint16_t velocity);
  void fillAudioBuffer(uint16_t *buffer, uint32_t length);

  static void dmaIRQHandler();

private:
  std::array<DrumSample, NUM_DRUM_SAMPLES> samples_;
  std::array<Voice, NUM_VOICES> voices_;

  // private helper to find an available voice slot and assign a sound
  void _allocateVoice(uint8_t drum_id, uint16_t normalized_velocity);

  // double buffers for sound output
  alignas(4) std::array<uint16_t, AUDIO_BUFFER_SIZE> audio_buffer_A_;
  alignas(4) std::array<uint16_t, AUDIO_BUFFER_SIZE> audio_buffer_B_;
  volatile bool buffer_flip_ = false; // which buffer is being filled/sent

  // pointer to the RP2350s spi0 SPI controller
  spi_inst_t *spi_ = spi0;

  // just use channel 0
  uint32_t dma_channel_ = 0;

  // the second channel used in the double buffer system
  // when the main dma finishes a buffer, this chain DMA updates the
  // main dma to point to the next buffer
  uint32_t chain_dma_channel_ = 1;

  // helper method to convert signed samples to unsigned centred at 2048 for DAC
  static inline uint16_t convertToDacFormat(int16_t sample);
};

extern AudioEngine audioEngine;