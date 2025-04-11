#include "audio.hpp"
#include "configs.hpp"
#include "hardware/gpio.h"
#include "loop.hpp"

extern LoopTrack loop;
extern uint32_t sample_counter; // need access to this from audio.cpp

#define BUTTON_RECORD GPIO_BUTTON_A
#define BUTTON_CLEAR GPIO_BUTTON_B

bool last_record_button = false;
bool last_clear_button = false;
bool last_overdub_button = false;
bool last_bank_switch_button = false;

void checkBankSwitchButton() {
  bool bank_switch_button = !gpio_get(SAMPLE_SWITCH_BUTTON);

  if (bank_switch_button && !last_bank_switch_button) {
    audioEngine.switchSoundBank();
  }

  last_bank_switch_button = bank_switch_button;
}

void updateLeds() {
  // turn off LEDs before setting the right one
  gpio_put(LED_GREEN, 0);
  gpio_put(LED_RED, 0);
  gpio_put(LED_ORANGE, 0);
  gpio_put(LED_BLUE, 0);

  if (loop.isRecording()) {
    // recording = green
    gpio_put(LED_GREEN, 1);
  } else if (loop.isPlaying()) {
    // playing = orange if overdub on, red if not
    if (loop.isOverdubEnabled()) {
      gpio_put(LED_ORANGE, 1);
    } else {
      gpio_put(LED_RED, 1);
    }
  } else {
    // only playing live sounds = blue
    gpio_put(LED_BLUE, 1);
  }
}

void checkLoopButtons() {
  // active low
  bool record_button = !gpio_get(BUTTON_RECORD);
  bool clear_button = !gpio_get(BUTTON_CLEAR);
  bool overdub_button = !gpio_get(BUTTON_OVERDUB);

  if (record_button && !last_record_button) {
    // if in live-only mode
    if (!loop.isRecording() && !loop.isPlaying()) {
      // start recording
      loop.startRecording();
      DEBUG_PRINT("Loop recording started\n");
      // if in record mode
    } else if (loop.isRecording()) {
      // stop initial recording, start playback
      uint32_t current_time = sample_counter;
      loop.stopRecording(current_time);
      DEBUG_PRINT("Loop recording stopped, playback started\n");
      // if playing recording
    } else if (loop.isPlaying()) {
      // record button does nothing while playing
      // overdubbing controlled by third button
      // clearing handled by the clear button
      DEBUG_PRINT("Record button pressed while playing - no action.\n");
    }
    updateLeds();
  }

  if (clear_button && !last_clear_button) {
    loop.clear();
    updateLeds();
  }

  if (overdub_button && !last_overdub_button) {
    loop.toggleOverdub();
    updateLeds();
  }

  last_record_button = record_button;
  last_clear_button = clear_button;
  last_overdub_button = overdub_button;
}