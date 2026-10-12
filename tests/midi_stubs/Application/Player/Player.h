#pragma once
#include <cstdint>

class Player {
public:
  static Player *GetInstance() {
    static Player player;
    return &player;
  }
  void PlayNote(uint16_t, uint16_t, uint16_t, uint16_t) { ++notesPlayed; }
  void StopNote(uint16_t, uint16_t) { ++notesStopped; }
  void OnSongStartButton(uint16_t, uint16_t, bool, bool) {
    ++starts;
    running = true;
  }
  bool IsRunning() { return running; }
  void Stop() {
    ++stops;
    running = false;
  }
  uint16_t notesPlayed = 0;
  uint16_t notesStopped = 0;
  uint16_t starts = 0;
  uint16_t stops = 0;
  bool running = false;
};
