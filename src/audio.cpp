// src/audio.cpp

#include "audio.hpp"
#include "kick.hpp"
#include "src/configs.hpp"
#include "src/snare.hpp"
#include <cstdint>
#include <cstring>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>

AudioEngine::AudioEngine()
    : dma_channel_(dma_claim_unused_channel(true)),
      chain_dma_channel_(dma_claim_unused_channel(true)) {
  FUNCTION_PRINT("AudioEngine::AudioEngine\n");

  // initialise sample data
  samples_[0] = {(const int16_t *)kick, KICK_LENGTH};
  samples_[1] = {(const int16_t *)snare, SNARE_LENGTH};

  spi_init(spi_, SPI_BAUD_RATE);

  // configuring spi instance spi0, with data bit length of 16 (transferring 16
  // bits at a time)
  // SPI_CPOL_0: clock polarity setting = 0
  // this means the clock line idles at low
  // the clock signal starts at 0 and pulses to 1 during the data transfer
  // SPI_CPHA_0: clock phase setting = 0
  // this means data is sampled on the rising edge of the clock
  // data is changed on the falling edge
  // SPI_MSB_FIRST: most sig bit transmitted first
  spi_set_format(spi_, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  // set the DAC pins to SPI
  gpio_set_function(DAC_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_MOSI_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_CS_PIN, GPIO_FUNC_SPI);

  // MAIN DMA CONFIG

  // creates a default configuration for our dma channel
  // dma_channel_ holds the channel number we already claimed
  dma_channel_config dma_config = dma_channel_get_default_config(dma_channel_);

  // configure the main DMA channel to transfer 16-bit data
  // (the dac requires 16 bits to be sent at a time)
  channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);

  // set up a "data request" trigger from the SPI hardward
  // this makes the DMA wait for the SPI TX FIFO to have room before
  // transferring each piece of data, ensuring synchronisation.
  // the "true" parameter means we're requesting for TX not RX
  channel_config_set_dreq(&dma_config, spi_get_dreq(spi_, true));

  // configure the DMA to incremement its read address after each transfer
  channel_config_set_read_increment(&dma_config, true);

  // configure the DMA not to incremement its write address.
  // we always want to write to the same SPI data register location
  // so each sample goes to the same hardware destination
  channel_config_set_write_increment(&dma_config, false);

  // CHAIN DMA CONFIG
  // the chain DMA is a second DMA channel that automatically reconfigures
  // the mian DMA channel when it finished the buffer

  // get a default config for our chain DMA channel
  dma_channel_config chain_config =
      dma_channel_get_default_config(chain_dma_channel_);

  // set the chain DMA to transfer 4 bytes (32 bits)
  // because we're sending a memory address and the pico is a 32 bit system
  channel_config_set_transfer_data_size(&chain_config, DMA_SIZE_32);

  // don't increment the read address (we're reading from a fixed address)
  // either bufferA's or bufferB's address
  channel_config_set_read_increment(&chain_config, false);

  // don't increment write address
  channel_config_set_write_increment(&chain_config, false);

  // this is what creates the double-buffer cyclical system
  // when the chain DMA finishes, it will automatically restart the main DMA
  // channel this creates a continuous cycle, main DMA -> chain DMA -> main DMA
  channel_config_set_chain_to(&chain_config, dma_channel_);

  // this function configures the detail of the DMA
  dma_channel_configure(
      // which dma channel to configure
      chain_dma_channel_,

      // address of configuration we just set up
      &chain_config,

      // destination: this is the address of where the dma will write to
      // this is the control register of the main DMA channel that holds
      // the "read" address". By writing here, we change where the main DMA
      // reads from next. The "al3_read_addr_trig" means:
      // al3: Alias 3 (a special address that also triggers the DMA)
      // read_addr: the read address of the register
      // trig: writing here triggers the dma to start
      &dma_hw->ch[dma_channel_].al3_read_addr_trig,

      // source: where the chain DMA will read from
      // we set this to null for now but it will update dynamically
      // in the irq handler to point to buffer A or B
      nullptr,

      // count: the number of transfers to do
      // we only need to transfer once, the address of the next audio buffer
      1,

      // don't start the dma chain yet
      // it will be triggered by the IRQ handler when the main DMA completes
      false);

  // DMA_IRQ_0 represents the interrupt request line for DMA channel 0
  // dmaIRQHandler() is our function that will be called when the DMA transfer
  // completes "exclusive" means this handler will be the only handler for this
  // interrupt
  irq_set_exclusive_handler(DMA_IRQ_0, dmaIRQHandler);
  dma_channel_set_irq0_enabled(dma_channel_, true);

  // enable global interrupt for the DMA controller
  irq_set_enabled(DMA_IRQ_0, true);

  // clear the audio buffers to start with silence
  memset(audio_buffer_A_.data(), 0, AUDIO_BUFFER_SIZE * sizeof(uint16_t));
  memset(audio_buffer_B_.data(), 0, AUDIO_BUFFER_SIZE * sizeof(uint16_t));

  // fill first buffer with the first chunk of audio data
  // fillAudioBuffer will mix the active voices to fill the buffer
  fillAudioBuffer(audio_buffer_A_.data(), AUDIO_BUFFER_SIZE);

  // configure and start the first dma channel immediately
  // &spi_get_hw(spi_)->dr, destination is the SPI data register (to send to the
  // dac)
  dma_channel_configure(dma_channel_, &dma_config, &spi_get_hw(spi_)->dr,
                        audio_buffer_A_.data(), AUDIO_BUFFER_SIZE, true);
}