#pragma once
#include <cstdint>

// MIDI chord tests use the default chromatic scale.
class Project {
public:
  uint8_t GetScale() { return 0; }
  uint8_t GetScaleRoot() { return 0; }
};

class Player {
public:
  static Player *GetInstance() {
    static Player player;
    return &player;
  }
  Project *GetProject() { return &project_; }

private:
  Project project_;
};
