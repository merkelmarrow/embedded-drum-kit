// src/piezo.cpp

#include "configs.hpp"
#include "hardware/adc.h"

#include "piezo.hpp"

Piezo::Piezo() {
    adc_init();

    adc_gpio_init(ADC0_PIN);
    adc_gpio_init(ADC1_PIN);

    // set ADC0 as first selected
    adc_select_input(0);

    // intialise mux select pins as outputs
    gpio_init(MUX_S0_PIN);
    gpio_set_dir(MUX_S0_PIN, GPIO_OUT);
    gpio_init(MUX_S1_PIN);
    gpio_set_dir(MUX_S1_PIN, GPIO_OUT);

    // default mux channel is 0
    selectMuxChannel(0);
}

void Piezo::selectMuxChannel(uint8_t channel) {
    // convert the channel to binary
    // adding 5 to all the binary outputs just for space reasons on the breadboard
    // (A5-A7 are on the left of the mux, while A0-A2 are on the right)
    uint8_t binary_ch = channel + 0x5;

    gpio_put(MUX_S0_PIN, binary_ch & 0x1);
    gpio_put(MUX_S1_PIN, (binary_ch >> 1) & 0x1);
    gpio_put(MUX_S2_PIN, (binary_ch >> 2) & 0x1);

    // random instructions to allow mux to settle, takes approx 60ns according to datasheet
    // or around 10 instructions without overclocking
    // so below are random instructions to delay by 4-7 instructions
    int delay = 0x1;
    delay ^= 0x1;
    delay += 0x2;
    delay -= 0x1;
    delay &= 0x2;

}