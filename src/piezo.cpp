// src/piezo.cpp

#include "configs.hpp"
#include "hardware/adc.h"
#include <hardware/timer.h>

#include "piezo.hpp"

Piezo::Piezo() {
    printf("Piezo::Piezo\n");
    adc_init();

    // Set maximum sample rate for the ADC
    adc_set_clkdiv(0);

    adc_gpio_init(ADC0_PIN);
    adc_gpio_init(ADC1_PIN);

    // set ADC0 as first selected
    adc_select_input(0);

    // intialise mux select pins as outputs
    gpio_init(MUX_S0_PIN);
    gpio_set_dir(MUX_S0_PIN, GPIO_OUT);
    gpio_init(MUX_S1_PIN);
    gpio_set_dir(MUX_S1_PIN, GPIO_OUT);
    gpio_init(MUX_S2_PIN);
    gpio_set_dir(MUX_S2_PIN, GPIO_OUT);

    // default mux channel is 0
    selectMuxChannel(0);

    // Initialize all sensor states as ready
    for (auto& state : piezo_states_) {
        state.ready_for_trigger = true;
        state.peak_end_time_us = 0;
        state.max_value = 0;
        state.recovery_end_time_us = 0;
    }
}

void Piezo::selectMuxChannel(uint8_t channel) {
    printf("Piezo::selectMuxChannel\n");
    // convert the channel to binary
    // adding 5 to all the binary outputs just for space reasons on the breadboard
    // (A5-A7 are on the left of the mux, while A0-A2 are on the right)
    uint8_t binary_ch = channel + LOWEST_MUX_IN;

    uint8_t s0, s1, s2;
    s0 = binary_ch & 0x1;
    s1 = (binary_ch >> 1) & 0x1;
    s2 = (binary_ch >> 2) & 0x1;

    printf("S0 = %d\n", s0);
    printf("S1 = %d\n", s1);
    printf("S2 = %d\n", s2);


    gpio_put(MUX_S0_PIN, binary_ch & 0x1);
    gpio_put(MUX_S1_PIN, (binary_ch >> 1) & 0x1);
    gpio_put(MUX_S2_PIN, (binary_ch >> 2) & 0x1);

    // random instructions to let the mux settle, takes approx 60ns according to datasheet
    // or around 10 instructions without overclocking
    // so below are random instructions to delay by 4-7 instructions since there will definitely
    // be more instructions between reads
    // it saves us from having to think about it somewhere else
    int delay = 0x1;
    delay ^= 0x1;
    delay += 0x2;
    delay -= 0x1;
    delay &= 0x2;
}

// function polls two of the piezos and updates which two piezos to poll next
void Piezo::update() {
    printf("Piezo::update\n");
    // read adc0 and adc1 for raw values
    adc_select_input(0);
    uint16_t raw0 = adc_read();
    adc_select_input(1);
    uint16_t raw1 = adc_read();
    printf("Readings: %d and %d\n", raw0, raw1);

    // get the respective piezo indices
    int piezo_index_A = mux_channel_index_;
    int piezo_index_B = mux_channel_index_ + HALF_NUM_PIEZOS;
    printf("Piezo indices: %d and %d\n", piezo_index_A, piezo_index_B);

    uint32_t current_time_us = time_us_32();

    processPiezoReading(piezo_index_A, raw0, current_time_us);
    processPiezoReading(piezo_index_B, raw1, current_time_us);

    // update mux channel index for next update
    mux_channel_index_ = (mux_channel_index_ + 1) % HALF_NUM_PIEZOS;
    // select next mux channel to be ready
    selectMuxChannel(mux_channel_index_);

}

void Piezo::processPiezoReading(uint8_t piezo_index, uint16_t reading, uint32_t current_time_us) {
    printf("Piezo::processPiezoReading\n");
    if (piezo_index >= NUM_PIEZOS) return;

    // access the piezo's current state (reference to avoid copying)
    PiezoState& state = piezo_states_[piezo_index];

    // check if we're in the recovery period
    if (current_time_us < state.recovery_end_time_us) return;

    // we'll only enter here if we're already capturing a peak
    if (!state.ready_for_trigger) {
        // update the maximum if it's bigger than what we've already seen
        if (reading > state.max_value) {
            state.max_value = reading;
        }

        // check if we should stop capturing for this particular strike
        if (current_time_us >= state.peak_end_time_us) {
            // peak capture is done, trigger the callback with the max value
            if (piezo_callback_) {
                piezo_callback_(piezo_index, state.max_value);
            }
            state.ready_for_trigger = true;
            state.recovery_end_time_us = current_time_us + (PIEZO_RECOVERY_TIME_US);
        }
    }

    // if we are ready for the trigger, check if reading exceeds the threshold
    else if (reading > PIEZO_THRESHOLD[piezo_index]) {
        // start capturing the peak
        state.max_value = reading;
        state.peak_end_time_us = current_time_us + PIEZO_CAPTURE_TIME_US;
        state.ready_for_trigger = false;
    }
}