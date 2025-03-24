// src/piezo.cpp

#include "configs.hpp"
#include "hardware/adc.h"
#include <hardware/timer.h>

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
    uint8_t binary_ch = channel + LOWEST_MUX_IN;

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

// function polls two of the piezos and updates which two piezos to poll next
void Piezo::update() {
    // select current mux channel
    selectMuxChannel(mux_channel_index_);

    // read adc0 and adc1 for raw values
    adc_select_input(0);
    uint16_t raw0 = adc_read();
    adc_select_input(1);
    uint16_t raw1 = adc_read();


    int piezo_index_A = mux_channel_index_;
    int piezo_index_B = mux_channel_index_ + HALF_NUM_PIEZOS;

    uint32_t current_time_us = time_us_32();

    // might be able to remove the check for piezo A
    // if optimisation is an issue later
    // since this function is being called so often
    if (piezo_index_A < NUM_PIEZOS) {
        if (raw0 < PIEZO_THRESHOLD[piezo_index_A]) {
            if (current_time_us - last_trigger_time_us_[piezo_index_A] > PIEZO_DEBOUNCE_TIME_US) {
                last_trigger_time_us_[piezo_index_A] = current_time_us;

                if (piezo_callback_) {
                    piezo_callback_(piezo_index_A, raw0);
                }
            }
        }
    }

    if (piezo_index_B < NUM_PIEZOS) {
        if (raw0 < PIEZO_THRESHOLD[piezo_index_B]) {
            if (current_time_us - last_trigger_time_us_[piezo_index_B] > PIEZO_DEBOUNCE_TIME_US) {
                last_trigger_time_us_[piezo_index_B] = current_time_us;

                if (piezo_callback_) {
                    piezo_callback_(piezo_index_B, raw1);
                }
            }
        }
    }

    mux_channel_index_ = (mux_channel_index_ + 1) % HALF_NUM_PIEZOS;

}