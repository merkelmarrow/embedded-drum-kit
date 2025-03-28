// src/main.cpp

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdio.h>

#include "audio.hpp"
#include "configs.hpp"
#include "piezo.hpp"

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

  // instantiate the piezo
  Piezo piezo;
  piezo.setPiezoCallback(drumHitCallback);

  while (true) {
    piezo.update();
  }

  return 0;
}