// src/main.cpp

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include <hardware/clocks.h>

#include "audio.hpp"
#include "buttons.cpp"
#include "configs.hpp"
#include "piezo.hpp"

// callback function for sensor triggers
void drumHitCallback(uint8_t piezo_index, uint16_t velocity) {
  uint8_t current_sound_bank = audioEngine.getCurrentBank();
  uint8_t drum_id_to_play = piezo_index + (current_sound_bank * NUM_PIEZOS);
  audioEngine.playSound(drum_id_to_play, velocity);
}

int main() {

  set_sys_clock_khz(250000, true);
  stdio_init_all();

  sleep_ms(5000); // wait for serial monitor connection
  FUNCTION_PRINT("main\n");

  audioEngine.init();

  gpio_init(GPIO_BUTTON_A);
  gpio_set_dir(GPIO_BUTTON_A, GPIO_IN);
  gpio_pull_up(GPIO_BUTTON_A);

  gpio_init(GPIO_BUTTON_B);
  gpio_set_dir(GPIO_BUTTON_B, GPIO_IN);
  gpio_pull_up(GPIO_BUTTON_B);

  gpio_init(BUTTON_OVERDUB);
  gpio_set_dir(BUTTON_OVERDUB, GPIO_IN);
  gpio_pull_up(BUTTON_OVERDUB);

  gpio_init(LED_GREEN);
  gpio_set_dir(LED_GREEN, GPIO_OUT);
  gpio_init(LED_RED);
  gpio_set_dir(LED_RED, GPIO_OUT);
  gpio_init(LED_ORANGE);
  gpio_set_dir(LED_ORANGE, GPIO_OUT);
  gpio_init(LED_BLUE);
  gpio_set_dir(LED_BLUE, GPIO_OUT);

  gpio_init(SAMPLE_SWITCH_BUTTON);
  gpio_set_dir(SAMPLE_SWITCH_BUTTON, GPIO_IN);
  gpio_pull_up(SAMPLE_SWITCH_BUTTON);

  updateLeds();

  // instantiate the piezo
  Piezo piezo;
  piezo.setPiezoCallback(drumHitCallback);

  while (true) {
    piezo.update();
    checkLoopButtons();
    checkBankSwitchButton();
  }

  return 0;
}