// src/configs.hpp

#pragma once

#include <cstdint>

// Uncomment the next line to enable debugging statements
#define DEBUG_ENABLED

// Uncomment the next line to enable function call statements
//#define FUNCTION_CALL_DEBUG

#ifdef DEBUG_ENABLED
    #define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

#ifdef FUNCTION_CALL_DEBUG
    #define FUNCTION_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define FUNCTION_PRINT(fmt, ...) ((void)0)
#endif

namespace {
    // helper function to round up to nearest integer
    int roundup_div(int n, int d) {
        return (n + d - 1) / d; // uses integer division which truncates
    }
}

// Half num piezos should be greater than half the number of piezos
constexpr uint8_t NUM_PIEZOS = 2;

// don't touch
const uint8_t HALF_NUM_PIEZOS = roundup_div(NUM_PIEZOS, 2);

constexpr uint8_t MUX_S0_PIN = 2;
constexpr uint8_t MUX_S1_PIN = 3;
constexpr uint8_t MUX_S2_PIN = 4;

constexpr uint8_t ADC0_PIN = 26; // for 3 piezos
constexpr uint8_t ADC1_PIN = 27; // for another 3 piezos
constexpr uint8_t ADC2_PIN = 28; // for a potentiometer

// ie. if A5 is the lowest mux input used, then LOWEST_MUX_IN = 5
// note that the inputs in have to be used sequentially after the lowest
// for some of the logic to work
// ie. A5, A6 and A7
constexpr uint8_t LOWEST_MUX_IN = 5;


constexpr uint16_t BASE_PIEZO_THRESHOLD = 50;
// kept as an array in case some piezos are more sensitive than others (due to component variations)
constexpr uint16_t PIEZO_THRESHOLD[NUM_PIEZOS] = {
    BASE_PIEZO_THRESHOLD, BASE_PIEZO_THRESHOLD
};

// piezo recovery time in microsends (after a strike)
// time (us) = time (ms) * 1000
constexpr uint32_t PIEZO_RECOVERY_TIME_US = 50 * 1000;

// piezo capture time (time after first trigger to look for the max value of the peak)
// time (us) = time (ms) * 1000
constexpr uint32_t PIEZO_CAPTURE_TIME_US = 1 * 1000;