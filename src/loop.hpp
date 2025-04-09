#pragma once
#include <array>
#include <cstdint>
#include <vector>

struct LoopEvent {
  // Time in samples since the start of the recording
  uint32_t timestamp;
  // Which sound to play
  uint8_t drum_id;
  // How hard to play it
  uint16_t velocity;
};

// A class to record and play back a loop of drum events
class LoopTrack {
public:
  // Start recording a new loop
  void startRecording();
  // Stop recording and start playing the loop
  void stopRecording();
  // Add a drum event to the loop
  void addEvent(uint8_t drum_id, uint16_t velocity,
                uint32_t current_sample_time);
  // Play back the loop
  void tick(uint32_t current_sample_time, void (*playFunc)(uint8_t, uint16_t));
  // Clear the loop
  void clear();

  // Getters
  // Is the loop currently being recorded
  bool isRecording() const;
  // Is the loop currently playing
  bool isPlaying() const;
  // Get the length of the loop in samples
  uint32_t getLength() const;

private:
  static constexpr size_t MAX_EVENTS = 12;
  // The recorded events
  std::array<LoopEvent, MAX_EVENTS> events_;
  size_t event_count_ = 0;
  // Are we currently recording
  bool recording_ = false;
  // Are we currently playing
  bool playing_ = false;
  // The sample time when recording started
  uint32_t record_start_sample_ = 0;
  // The length of the loop in samples
  uint32_t loop_length_ = 0;
};
