#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "loop.hpp"
#include "stdio.h"

#define DEBUG_PRINT(...) printf(__VA_ARGS__)

extern LoopTrack loop;
#define GPIO_BUTTON_A 0
#define GPIO_BUTTON_B 1

#define BUTTON_RECORD GPIO_BUTTON_A
#define BUTTON_CLEAR GPIO_BUTTON_B

bool last_record_button = false;
bool last_clear_button = false;

void checkLoopButtons() {
    bool record_button = gpio_get(BUTTON_RECORD);
    bool clear_button = gpio_get(BUTTON_CLEAR);

    // Edge detection (button press)
    if (record_button && !last_record_button) {
        if (!loop.isRecording() && !loop.isPlaying()) {
            loop.startRecording();
            DEBUG_PRINT("Loop recording started\n");
        } else if (loop.isRecording()) {
            loop.stopRecording();
            DEBUG_PRINT("Loop recording stopped and playing\n");
        } else {
            loop.clear();
            DEBUG_PRINT("Loop stopped and cleared\n");
        }
    }

    if (clear_button && !last_clear_button) {
        loop.clear();
        DEBUG_PRINT("Loop manually cleared\n");
    }

    last_record_button = record_button;
    last_clear_button = clear_button;
}
