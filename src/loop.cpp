#include "loop.hpp"

// Start recording a new loop
void LoopTrack::startRecording() {
    // Clear the events and start recording
    events_.clear();
    // Set the recording flag
    recording_ = true;
    // Clear the playing flag
    playing_ = false;
    // Reset the loop length
    record_start_sample_ = 0;
}

void LoopTrack::stopRecording() {
    // Stop recording and start playing
    recording_ = false;
    // Set the playing flag
    playing_ = true;
    // Calculate the loop length
    if (!events_.empty()) {
        loop_length_ = events_.back().timestamp;
    }
}

void LoopTrack::addEvent(uint8_t drum_id, uint16_t velocity, uint32_t current_sample_time) {
    // Add a drum event to the loop
    if (!recording_) return;
    // If this is the first event, record the start time
    if (record_start_sample_ == 0)
        // Record the start time
        record_start_sample_ = current_sample_time;
    // Add the event
    events_.push_back({current_sample_time - record_start_sample_, drum_id, velocity});
}

void LoopTrack::tick(uint32_t current_sample_time, void (*playFunc)(uint8_t, uint16_t)) {
    // Play back the loop
    if (!playing_ || loop_length_ == 0) return;

    // Calculate the position in the loop
    uint32_t position_in_loop = (current_sample_time - record_start_sample_) % loop_length_;
    for (const auto& e : events_) {
        // If the event is at the current position, play it
        if (e.timestamp == position_in_loop) {
            // Play the event
            playFunc(e.drum_id, e.velocity);
        }
    }
}

void LoopTrack::clear() {
    // Clear the loop
    events_.clear();
    playing_ = false;
    recording_ = false;
}

// Getters
bool LoopTrack::isRecording() const { return recording_; }
bool LoopTrack::isPlaying() const { return playing_; }
uint32_t LoopTrack::getLength() const { return loop_length_; }
