// src/piezo.hpp

#pragma once

#include <array>
#include <cstdint>
#include <stdio.h>

#include "configs.hpp"

typedef void (*PiezoCallback)(uint8_t piezo_index, uint16_t velocity);

// each pizeo is stateful, they need to remember if they are sampling inside the debounce time,
// what the peak measurement of any particular strike was, etc.
struct PiezoState {
    bool ready_for_trigger = true; // at least the debounce time has elapsed
    uint32_t peak_end_time_us = 0; // when peak capture should end
    uint16_t max_value = 0; // maximum value during peak capture
    uint32_t recovery_end_time_us = 0; // when recovery period ends
};


class Piezo {
    public:

    Piezo();
    ~Piezo() = default;

    void setPiezoCallback(PiezoCallback cb) { 
        FUNCTION_PRINT("Piezo::setPiezoCallback\n");
        piezo_callback_ = cb; 
    }
    void update();

    private:
    // current multiplexer channel index
    uint8_t mux_channel_index_ = 0;

    // storage of each sensor's state
    std::array<PiezoState, NUM_PIEZOS> piezo_states_;

    // callback to trigger when a hit is detected
    PiezoCallback piezo_callback_ = nullptr;

    // helper: selects a given channel on the multiplexer (using the select outputs)
    void selectMuxChannel(uint8_t channel);

    // process each piezo reading and update it's state
    void processPiezoReading(uint8_t piezo_index, uint16_t reading, uint32_t current_time_us);

};