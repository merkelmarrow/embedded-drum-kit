// src/main.cpp

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdio.h>

#include "audio.hpp"
#include "configs.hpp"
#include "piezo.hpp"
#include "loop.hpp"
#include "buttons.cpp"

// callback function for sensor triggers
void drumHitCallback(uint8_t piezo_index, uint16_t velocity) {
  DEBUG_PRINT("Drum hit: Piezo %d, Vel %d\n", piezo_index, velocity);
  audioEngine.playSound(piezo_index, velocity);
}

int main() {
  stdio_init_all();

  sleep_ms(5000); // wait for serial monitor connection
  FUNCTION_PRINT("main\n");

  audioEngine.init();
  gpio_init(GPIO_BUTTON_A);
  gpio_set_dir(GPIO_BUTTON_A, GPIO_IN);
  gpio_pull_up(GPIO_BUTTON_A);  // or pull_up if your button setup uses active low

  gpio_init(GPIO_BUTTON_B);
  gpio_set_dir(GPIO_BUTTON_B, GPIO_IN);
  gpio_pull_up(GPIO_BUTTON_B);

  gpio_init(LED_GREEN);
  gpio_set_dir(LED_GREEN, GPIO_OUT);
  gpio_put(LED_GREEN, 0); // turn off the green LED
  gpio_init(LED_RED);
  gpio_set_dir(LED_RED, GPIO_OUT);
  gpio_put(LED_RED, 0); // turn off the red LED
  gpio_init(LED_ORANGE);
  gpio_set_dir(LED_ORANGE, GPIO_OUT);
  gpio_put(LED_ORANGE, 0); // turn off the orange LED
  // instantiate the piezo
  Piezo piezo;
  piezo.setPiezoCallback(drumHitCallback);

  while (true) {
    piezo.update();
    checkLoopButtons();
  }

  return 0;
}