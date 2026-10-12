#pragma once

// Channel tests render without advancing the sequencer clock.
class SyncMaster {
public:
  static SyncMaster *GetInstance() {
    static SyncMaster sync;
    return &sync;
  }
  bool TableSlice() { return false; }
};
