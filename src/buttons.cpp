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
    // Reading the pins and gives back true if the button is pressed
    bool record_button = gpio_get(BUTTON_RECORD);
    bool clear_button = gpio_get(BUTTON_CLEAR);

    // Edge detection (button press)
    if (record_button && !last_record_button) {
        // When it's not recording or playing, start recording 
        if (!loop.isRecording() && !loop.isPlaying()) {
            loop.startRecording();
            DEBUG_PRINT("Loop recording started\n");
        } else if (loop.isRecording()) { // When it's recording, stop recording and start playing
            loop.stopRecording();
            DEBUG_PRINT("Loop recording stopped and playing\n");
        } else { // When it's playing, stop playing
            loop.clear();
            DEBUG_PRINT("Loop stopped and cleared\n");
        }
    }

    // If button B is pressed, clear the loop
    if (clear_button && !last_clear_button) {
        loop.clear();
        DEBUG_PRINT("Loop manually cleared\n");
    }

    // Update the last button states
    last_record_button = record_button;
    last_clear_button = clear_button;
}
