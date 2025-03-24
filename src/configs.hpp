// src/configs.hpp

#pragma once

#include <cstdint>

constexpr uint8_t NUM_PIEZOS = 6;

constexpr uint8_t MUX_S0_PIN = 2;
constexpr uint8_t MUX_S1_PIN = 3;
constexpr uint8_t MUX_S2_PIN = 4;

constexpr uint8_t ADC0_PIN = 26; // for 3 piezos
constexpr uint8_t ADC1_PIN = 27; // for another 3 piezos
constexpr uint8_t ADC2_PIN = 28; // for a potentiometer