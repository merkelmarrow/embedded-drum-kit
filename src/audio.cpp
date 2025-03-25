// src/audio.cpp

#include "audio.hpp"
#include "kick.hpp"
#include "src/configs.hpp"
#include "src/snare.hpp"
#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>

AudioEngine::AudioEngine() {
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

  gpio_set_function(DAC_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_MOSI_PIN, GPIO_FUNC_SPI);
  gpio_set_function(DAC_CS_PIN, GPIO_FUNC_SPI);
}