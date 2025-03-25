// src/audio.cpp

#include "audio.hpp"
#include "kick.hpp"
#include "src/configs.hpp"

AudioEngine::AudioEngine() {
  FUNCTION_PRINT("AudioEngine::AudioEngine\n");

  // initialise sample data
  samples_[0] = {kick, KICK_LENGTH};
}