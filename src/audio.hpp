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
  uint32_t velocity = 1000; // playback velocity from the piezos
};

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine() = default;

  void play_sound(uint8_t drum_id, uint16_t velocity);
  void fill_audio_buffer(uint16_t *buffer, uint32_t length);

private:
  std::array<DrumSample, NUM_DRUM_SAMPLES> samples_;
  std::array<Voice, NUM_VOICES> voices_;

  // double buffers for sound output
  alignas(4) std::array<uint16_t, AUDIO_BUFFER_SIZE> audio_buffer_A_;
  alignas(4) std::array<uint16_t, AUDIO_BUFFER_SIZE> audio_buffer_B_;
  volatile bool buffer_flip_ = false; // which buffer is being filled/sent

  // spi and dma config //

  // pointer to the RP2350s spi0 SPI controller
  spi_inst_t *spi = spi0;

  // just use channel 0
  uint32_t dma_channel = 0;

  // the second channel used in the double buffer system
  // when the main dma finishes a buffer, this chain DMA updates the
  // main dma to point to the next buffer
  uint32_t chain_dma_channel = 1;
};