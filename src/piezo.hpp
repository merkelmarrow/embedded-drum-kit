// src/piezo.hpp

#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "configs.hpp"

typedef void (*PiezoCallback)(int piezo_index, uint16_t velocity);


class Piezo {
    public:
    void init();
    void setPiezoCallback(PiezoCallback cb) { piezo_callback_ = cb; }

    private:
    // current multiplexer channel index
    uint8_t mux_channel_index_ = 0;

    // last trigger time (for debouncing) for each sensor
    std::array<uint32_t, NUM_PIEZOS> last_trigger_time_us_ = {0};

    // callback to trigger when a hit is detected
    PiezoCallback piezo_callback_ = nullptr;

};