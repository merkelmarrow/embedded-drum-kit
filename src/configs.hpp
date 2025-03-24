// src/configs.hpp

#pragma once

#include <cstdint>

namespace {
    // helper function to round up to nearest integer
    int roundup_div(int n, int d) {
        return (n + d - 1) / d;
    }
}

// Half num piezos should be greater than half the number of piezos
constexpr uint8_t NUM_PIEZOS = 6;

// don't touch
const uint8_t HALF_NUM_PIEZOS = roundup_div(NUM_PIEZOS, 2);

constexpr uint8_t MUX_S0_PIN = 2;
constexpr uint8_t MUX_S1_PIN = 3;
constexpr uint8_t MUX_S2_PIN = 4;

constexpr uint8_t ADC0_PIN = 26; // for 3 piezos
constexpr uint8_t ADC1_PIN = 27; // for another 3 piezos
constexpr uint8_t ADC2_PIN = 28; // for a potentiometer

// ie. if A5 is the lowest mux input used, then LOWEST_MUX_IN = 5
constexpr uint8_t LOWEST_MUX_IN = 5;


constexpr uint16_t BASE_PIEZO_THRESHOLD = 1000;
// kept as an array in case some piezos are more sensitive than others (due to component variations)
constexpr uint16_t PIEZO_THRESHOLD[NUM_PIEZOS] = {
    BASE_PIEZO_THRESHOLD, BASE_PIEZO_THRESHOLD, 
    BASE_PIEZO_THRESHOLD, BASE_PIEZO_THRESHOLD, 
    BASE_PIEZO_THRESHOLD, BASE_PIEZO_THRESHOLD
};

// define the piezo debounce time in microseconds
constexpr uint32_t PIEZO_DEBOUNCE_TIME_US = 5000;