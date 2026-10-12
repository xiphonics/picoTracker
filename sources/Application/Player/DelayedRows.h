/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef _DELAYED_ROWS_H_
#define _DELAYED_ROWS_H_

#include "Externals/etl/include/etl/vector.h"
#include "RowFx.h"

struct DelayedRow {
  uint8_t note;
  uint8_t instrument;
  uint8_t transpose;
  TrackerCommand cmd1;
  TrackerCommand cmd2;
  uint16_t param1;
  uint16_t param2;
};

class DelayedRows {
public:
  void Add(const DelayedRow &row) {
    uint8_t ticks = 1;
    if (row.cmd1 == TrackerCommand::InstrumentCommandDelay) {
      ticks = (row.param1 & 0x0F) + 1;
    }
    if (row.cmd2 == TrackerCommand::InstrumentCommandDelay) {
      ticks = (row.param2 & 0x0F) + 1;
    }
    pending_.push_back({.row = row, .ticks = ticks});
  }

  template <typename Trigger> void Tick(Trigger trigger) {
    // Scan every event: later rows with shorter delays can expire first.
    for (auto it = pending_.begin(); it != pending_.end();) {
      if (--it->ticks == 0) {
        const DelayedRow row = it->row;
        it = pending_.erase(it);
        trigger(row);
      } else {
        ++it;
      }
    }
  }

  void Clear() { pending_.clear(); }

private:
  struct Pending {
    DelayedRow row;
    uint8_t ticks;
  };
  // At most one row enters per tick; the maximum countdown is 16 ticks.
  etl::vector<Pending, 16> pending_;
};

#endif
