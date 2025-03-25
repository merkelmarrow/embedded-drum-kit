// src/main.cpp

#include <pico/stdio.h>
#include <stdio.h>
#include <pico/stdlib.h>

#include "piezo.hpp"


// instantiate the piezo
// the constructor initialises the ADC and other important stuff


// callback function for sensor triggers
void drumHitCallback(uint8_t piezo_index, uint16_t velocity) {
    printf("Drum hit detected: Piezo %d, Velocity %d\n", piezo_index, velocity);
}

int main() {
    stdio_init_all();

    sleep_ms(10000); // wait for serial monitor connection
    printf("main\n");
    Piezo piezo;

    piezo.setPiezoCallback(drumHitCallback);

    while (true) {
        piezo.update();
    }

    return 0;
}