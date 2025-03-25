// src/audio.hpp

#pragma once

#include <cstdint>

struct DrumSample {
    const uint16_t* data;
    uint32_t length;
};

// represents an actively playing instance of a sound
struct Voice {
    bool active = false;
    uint8_t drum_id = 0; // which drum sound is playing, ie. kick or snare
    uint32_t position = 0; // position in the sound array
    uint32_t velocity = 1000; // playback velocity from the piezos
};