// src/audio.cpp

#include "audio.hpp"
#include "configs.hpp"
#include "loop.hpp"

#include "closed_hi_hat.hpp"
#include "kick.hpp"
#include "snare.hpp"
#include "src/crash_cymbal.hpp"
#include "src/open_hi_hat.hpp"
#include "src/ride_cymbal.hpp"

#include <cstdint>
#include <cstring>

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/regs/intctrl.h>
#include <hardware/spi.h>
#include <hardware/structs/dma.h>
#include <hardware/structs/io_bank0.h>
#include <pico/time.h>

#define MAX_LOOP_DURATION_SAMPLES (44100 * 4)


AudioEngine audioEngine;
LoopTrack loop;
uint32_t sample_counter = 0;
uint32_t loop_start_sample = 0;


AudioEngine::AudioEngine()
    : dma_channel_(dma_claim_unused_channel(true)),
      chain_dma_channel_(dma_claim_unused_channel(true)) {}

void AudioEngine::init() {
  FUNCTION_PRINT("AudioEngine::init\n");
  DEBUG_PRINT("Claimed DMA channels: Main=%d, Chain=%d\n", dma_channel_,
              chain_dma_channel_);

  // initialize sample data
  samples_[0] = {(const int16_t *)kick, KICK_LENGTH};
  samples_[1] = {(const int16_t *)snare, SNARE_LENGTH};
  samples_[2] = {(const int16_t *)closed_hi_hat, CLOSED_HI_HAT_LENGTH};
  samples_[3] = {(const int16_t *)open_hi_hat, OPEN_HI_HAT_LENGTH};
  samples_[4] = {(const int16_t *)crash_cymbal, CRASH_CYMBAL_LENGTH};
  samples_[5] = {(const int16_t *)ride_cymbal, RIDE_CYMBAL_LENGTH};

  // initialize all voices to inactive
  // probably not needed but just in case
  for (auto &voice : voices_) {
    voice.active = false;
    voice.drum_id = 0;
    voice.position = 0;
    voice.velocity = 1000;
  }
  DEBUG_PRINT("All %d voices initialized\n", NUM_VOICES);

  // initialise spi
  DEBUG_PRINT("Initializing SPI on spi0 at %lu baud\n", SPI_BAUD_RATE);
  spi_init(spi_, SPI_BAUD_RATE);
  spi_set_format(spi_, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  DEBUG_PRINT("SPI format set: 16-bit, CPOL=0, CPHA=0, MSB first\n");

  // set GPIO pins for SPI
  DEBUG_PRINT("Setting SPI pins: SCK=%d, MOSI=%d, CS=%d\n", DAC_SCK_PIN,
              DAC_MOSI_PIN, DAC_CS_PIN);
  gpio_set_function(DAC_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_MOSI_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_CS_PIN, GPIO_FUNC_SPI);

  // set up PWM for triggering the DMA at exactly 44.1kHz
  uint slice_num = pwm_gpio_to_slice_num(PWM_TIMER_PIN);
  DEBUG_PRINT("Configuring PWM on slice %d, pin %d\n", slice_num,
              PWM_TIMER_PIN);

  // set PWM pin function
  gpio_set_function(PWM_TIMER_PIN, GPIO_FUNC_PWM);

  // calculate PWM settings
  // for 44.1kHz with 150MHz clock: 150000000 / 44100 = 3401.36
  float clock_div = 1.0f;
  uint32_t wrap_value =
      (uint32_t)(PICO_CLOCK_SPEED / (clock_div * SAMPLE_RATE_HZ)) - 1;

  DEBUG_PRINT("PWM settings - Clock div: %.2f, Wrap value: %lu\n", clock_div,
              wrap_value);

  // initilalise PWM
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, clock_div);
  pwm_config_set_wrap(&config, wrap_value);
  pwm_init(slice_num, &config, false); // Don't start yet

  // set PWM duty cycle to 50%
  pwm_set_chan_level(slice_num, PWM_CHAN_A, wrap_value / 2);
  DEBUG_PRINT("PWM initialized with 50%% duty cycle\n");

  // configure the main DMA channel for audio data transfer
  dma_channel_config dma_config = dma_channel_get_default_config(dma_channel_);
  channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
  channel_config_set_dreq(&dma_config, DREQ_PWM_WRAP0 + slice_num);
  channel_config_set_read_increment(&dma_config, true);
  channel_config_set_write_increment(&dma_config, false);

  DEBUG_PRINT("Main DMA config - Size: 16-bit, DREQ: PWM_WRAP%d, Read inc: "
              "true, Write inc: false\n",
              slice_num);

  // configure the chain DMA for buffer switching
  dma_channel_config chain_config =
      dma_channel_get_default_config(chain_dma_channel_);
  channel_config_set_transfer_data_size(&chain_config, DMA_SIZE_32);
  channel_config_set_read_increment(&chain_config, false);
  channel_config_set_write_increment(&chain_config, false);
  channel_config_set_chain_to(&chain_config, dma_channel_);

  DEBUG_PRINT("Chain DMA config - Size: 32-bit, Read inc: false, Write inc: "
              "false, Chain to: %d\n",
              dma_channel_);

  // configure chain DMA (it'll be triggered by IRQ handler)
  dma_channel_configure(chain_dma_channel_, &chain_config,
                        &dma_hw->ch[dma_channel_].al3_read_addr_trig,
                        NULL, // Will be set in IRQ handler
                        1, false);

  // set up DMA interrupts
  irq_set_exclusive_handler(DMA_IRQ_0, dmaIRQHandler);
  irq_set_priority(DMA_IRQ_0, 0x40);
  dma_channel_set_irq0_enabled(dma_channel_, true);
  irq_set_enabled(DMA_IRQ_0, true);
  DEBUG_PRINT("DMA IRQ enabled\n");

  // clear and fill initial audio buffers
  DEBUG_PRINT("Clearing audio buffers\n");
  memset(audio_buffer_A_.data(), 0, AUDIO_BUFFER_SIZE * sizeof(uint16_t));
  memset(audio_buffer_B_.data(), 0, AUDIO_BUFFER_SIZE * sizeof(uint16_t));

  DEBUG_PRINT("Filling initial audio buffers\n");
  fillAudioBuffer(audio_buffer_A_.data(), AUDIO_BUFFER_SIZE);
  fillAudioBuffer(audio_buffer_B_.data(), AUDIO_BUFFER_SIZE);

  // initialise buffer_flip flag
  buffer_flip_ = false;

  // start main DMA with buffer A
  DEBUG_PRINT("Starting main DMA with buffer A\n");
  dma_channel_configure(dma_channel_, &dma_config, &spi_get_hw(spi_)->dr,
                        audio_buffer_A_.data(), AUDIO_BUFFER_SIZE, true);

  // start PWM to trigger DMA transfers
  DEBUG_PRINT("Starting PWM to trigger DMA\n");
  pwm_set_enabled(slice_num, true);
  
  // Comment this to disable the loop recording without buttons
  loop.startRecording();
  DEBUG_PRINT("Loop recording started\n");
  loop_start_sample = sample_counter;

}

void AudioEngine::fillAudioBuffer(uint16_t *buffer, uint32_t length) {
  // count active voices
  int active_count = 0;
  for (const auto &v : voices_)
    if (v.active)
      active_count++;
  DEBUG_PRINT("Fill: %d active\n", active_count);

  // fill the buffer with mixed audio
  for (uint32_t i = 0; i < length; i++) {
    int32_t mix_sum = 0;

    // process each voice
    for (int v = 0; v < NUM_VOICES; v++) {
      Voice &voice = voices_[v];

      if (voice.active) {
        const DrumSample &sample = samples_[voice.drum_id];

        if (voice.position < sample.length) {
          // get the value at this current position
          int16_t sample_val = sample.data[voice.position];

          // apply the velocity scaling
          int32_t scaled_val = ((int32_t)sample_val * voice.velocity) >> 12;

          // add to the mix
          mix_sum += scaled_val;

          // update position
          voice.position++;

          // debug position
          // if (voice.position % 5000 == 0) {
          //  DEBUG_PRINT("Voice %d: %lu/%lu\n", v, voice.position,
          //              sample.length);
          // }
        } else {
          // free the voice if we're at the end of the sample
          DEBUG_PRINT("Voice %d ended at %lu\n", v, voice.position);
          voice.active = false;
        }
      }
    }

    // clamp to the 12 bit range
    if (mix_sum > 2047)
      mix_sum = 2047;
    if (mix_sum < -2048)
      mix_sum = -2048;

    // store in the buffer
    buffer[i] = convertToDacFormat((int16_t)mix_sum);

    if (loop.isRecording() &&
        (sample_counter - loop_start_sample >= MAX_LOOP_DURATION_SAMPLES)) {
        loop.stopRecording();
        DEBUG_PRINT("Loop auto-stopped after 4 seconds\n");
    }

    // Call tick to check if anything should play
    loop.tick(sample_counter, [](uint8_t drum_id, uint16_t velocity){
      audioEngine.playSound(drum_id, velocity);
  
    });

    sample_counter++;
  }

  DEBUG_PRINT("Buffer: data[0]=0x%04x\n", buffer[0]);
}

uint16_t AudioEngine::convertToDacFormat(int16_t sample) {
  // convert from 12 bit signed bit to unsigned 12 bit for DAC
  uint16_t dac_value = (sample + 2048) & 0x0FFF;

  // add control bits for the mcp4922
  // 0x3000 = 0b0011000000000000 = DAC out A, 1x gain, active
  return 0x3000 | dac_value;
}

void AudioEngine::dmaIRQHandler() {
  // clear the interrupt request
  dma_hw->ints0 = 1u << audioEngine.dma_channel_;

  // AI suggested this fix, not sure about it
  static uint32_t buffer_a_addr = (uint32_t)audioEngine.audio_buffer_A_.data();
  static uint32_t buffer_b_addr = (uint32_t)audioEngine.audio_buffer_B_.data();

  // switch buffers based on the flip flag
  if (audioEngine.buffer_flip_) {
    // buffer B just finished, switch to A
    dma_channel_set_read_addr(audioEngine.chain_dma_channel_, &buffer_a_addr,
                              false);
    dma_channel_start(audioEngine.chain_dma_channel_);

    // fill buffer B while A is playing
    audioEngine.fillAudioBuffer(audioEngine.audio_buffer_B_.data(),
                                AUDIO_BUFFER_SIZE);

    // flip the flag for next time
    audioEngine.buffer_flip_ = false;
    DEBUG_PRINT("IRQ: B-A\n");
  } else {
    // buffer A just finished, switch to B
    dma_channel_set_read_addr(audioEngine.chain_dma_channel_, &buffer_b_addr,
                              false);
    dma_channel_start(audioEngine.chain_dma_channel_);

    // fill buffer A while B is playing
    audioEngine.fillAudioBuffer(audioEngine.audio_buffer_A_.data(),
                                AUDIO_BUFFER_SIZE);

    // Flip the flag for next time
    audioEngine.buffer_flip_ = true;
    DEBUG_PRINT("IRQ: A-B\n");
  }
}

void AudioEngine::playSound(uint8_t drum_id, uint16_t velocity) {
  if (drum_id >= NUM_DRUM_SAMPLES)
    return;

  DEBUG_PRINT("Playing sound: drum_id=%d, velocity=%d\n", drum_id, velocity);

  uint16_t normalised_velocity = 0;

  if (loop.isRecording()){
    loop.addEvent(drum_id, velocity, sample_counter);
  }
  if (velocity > HARDEST_HIT_PIEZO_VELOCITY) {
    normalised_velocity = TWELVE_BIT_MAX; // cap at maximum
    DEBUG_PRINT("Velocity capped at %d\n", TWELVE_BIT_MAX);
  } else {
    // linear mapping: (val - inMin) * (outMax - outMin) / (inMax - inMin) +
    // outMin

    // val = velocity.
    // inMin = 100.
    // outMax = 4095.
    // outMin = 0.
    // inMax = 1200.
    // inMin = 100.
    normalised_velocity =
        (uint16_t)((velocity - BASE_PIEZO_THRESHOLD) * TWELVE_BIT_MAX /
                   (HARDEST_HIT_PIEZO_VELOCITY - BASE_PIEZO_THRESHOLD));
    DEBUG_PRINT("Velocity normalized to %d\n", normalised_velocity);
  }

  // find first inactive voice and occupy it
  for (auto &voice : voices_) {
    if (!voice.active) {
      voice.active = true;
      voice.drum_id = drum_id;
      voice.position = 0;
      voice.velocity = normalised_velocity;
      DEBUG_PRINT("Voice allocated - Drum %d, Velocity %d\n", drum_id,
                  normalised_velocity);
      return;
    }
  }

  // if no voice is available, find the oldest voice to replace (steal the
  // voice)
  uint32_t oldest_position = 0;
  int oldest_index = -1;

  for (uint8_t i = 0; i < NUM_VOICES; i++) {
    if (voices_[i].position > oldest_position) {
      oldest_position = voices_[i].position;
      oldest_index = i;
    }
  }

  if (oldest_index >= 0) {
    voices_[oldest_index].active = true;
    voices_[oldest_index].drum_id = drum_id;
    voices_[oldest_index].position = 0;
    voices_[oldest_index].velocity = normalised_velocity;
    DEBUG_PRINT("Voice stolen for sample %d\n", drum_id);
  }
}