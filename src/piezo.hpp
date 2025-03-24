// src/piezo.hpp

#pragma once

#include <array>
#include <cstdint>

#include "configs.hpp"

class Piezo {
    public:
    void init();

    private:
    // current multiplexer channel index
    uint8_t mux_channel_index = 0;
    std::array<uint32_t, NUM_PIEZOS> last_trigger_time = {0};
};