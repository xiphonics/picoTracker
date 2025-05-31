#ifndef _PROFILER_H_
#define _PROFILER_H_

#define MAX_PROFILERS 4

#include "Adapters/picoTracker/utils/utils.h"
#include <Externals/etl/include/etl/map.h>
#include <Externals/etl/include/etl/string.h>
#include <cstdio>

// Helper function to safely calculate time differences with wrap-around
inline uint32_t time_diff(uint32_t newer, uint32_t older) {
  return newer -
         older; // This works correctly for uint32_t even with wrap-around
}

// Enable/disable profiling with a preprocessor flag
// defaults to disabled
#ifndef ENABLE_PROFILING
#define ENABLE_PROFILING 0
#endif

// Moving average window size (1 second at 60fps)
#define MOVING_AVG_WINDOW 60

class MovingAverageProfiler {
  static const uint8_t WINDOW_SIZE = 60; // 1 second at 60fps
  uint32_t samples[WINDOW_SIZE] = {0};
  int sample_count = 0;
  uint32_t last_log_time = 0;
  etl::string<32> name_;

public:
  MovingAverageProfiler(const char *name) : name_(name) {}

  void addSample(uint32_t duration) {
    // Store the sample
    samples[sample_count % WINDOW_SIZE] = duration;
    sample_count++;

    uint32_t current_time = micros();

    // Initialize last_log_time on first call
    if (last_log_time == 0) {
      last_log_time = current_time;
      return;
    }

    // Only log once per second
    uint32_t time_since_last = time_diff(current_time, last_log_time);
    if (time_since_last < 1000000) { // Not yet 1 second
      return;
    }

    // Calculate average of the samples in our window
    int count = (sample_count < WINDOW_SIZE) ? sample_count : WINDOW_SIZE;
    if (count == 0) {
      last_log_time = current_time;
      return;
    }

    uint64_t total = 0;
    for (int i = 0; i < count; i++) {
      total += samples[i];
    }

    uint32_t avg = (uint32_t)(total / count);
    printf("[PROFILER] %-30s: avg=%lu us (samples: %d)\n", name_.c_str(),
           (unsigned long)avg, count);

    // Reset for next window
    sample_count = 0;
    last_log_time = current_time;
  }
};

class Profiler {
  // Static map for multiple moving average profilers
  static etl::map<etl::string<32>, MovingAverageProfiler, MAX_PROFILERS>
      moving_avg_profilers_;
  etl::string<32> name_;
  uint32_t start_time_;
  bool use_moving_avg_;
  uint32_t total_time_;
  uint32_t call_count_;
  uint32_t last_log_time_;

  // Private constructor for moving average profilers
  Profiler(const etl::string<32> &name, bool use_moving_avg)
      : name_(name.c_str()), start_time_(0), use_moving_avg_(use_moving_avg),
        total_time_(0), call_count_(0), last_log_time_(0) {
#if ENABLE_PROFILING
    start_time_ = micros();
    if (use_moving_avg_) {
      // Automatically create the profiler if it doesn't exist
      auto it = moving_avg_profilers_.find(name);
      if (it == moving_avg_profilers_.end()) {
        moving_avg_profilers_.insert(
            etl::make_pair(name, MovingAverageProfiler(name.c_str())));
      }
    }
#endif
  }

public:
  // Standard profiler constructor (one-time measurement)
  explicit Profiler(const char *name) : Profiler(etl::string<32>(name), false) {}

  // Constructor for moving average profiler
  static Profiler MovingAverage(const char *name) {
    return Profiler(etl::string<32>(name), true);
  }

  ~Profiler() {
#if ENABLE_PROFILING
    uint32_t end_time = micros();
    uint32_t duration = time_diff(end_time, start_time_);

    if (use_moving_avg_) {
      auto it = moving_avg_profilers_.find(name_);
      if (it != moving_avg_profilers_.end()) {
        it->second.addSample(duration);
      }
    } else {
      printf("[PROFILER] %-30s: %6lu us\n", name_.c_str(), (unsigned long)duration);
    }
#endif
  }
};

// Macro for easy profiling of scopes
#if ENABLE_PROFILING
#define PROFILE_SCOPE(name) Profiler profiler_##__LINE__(name)
#define PROFILE_FUNCTION() Profiler profiler_##__LINE__(__FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

// Helper for measuring average time over multiple calls
class AverageProfiler {
public:
  AverageProfiler(const char *name)
      : name_(name), total_time_(0), call_count_(0), last_log_time_(0) {}

  ~AverageProfiler() { logStats(); }

  void addSample(uint32_t duration) {
    static uint32_t sample_count = 0;
    sample_count++;

    // Debug: Print every 1000th sample
    if (sample_count % 1000 == 0) {
      printf("[PROFILER] %-30s: sample %lu, duration=%lu\n", name_,
             (unsigned long)sample_count, (unsigned long)duration);
    }

    total_time_ += duration;
    call_count_++;

    // Log stats every 1 second (1,000,000 microseconds)
    static uint32_t last_log_time = 0;
    uint32_t current_time = micros();
    if (time_diff(current_time, last_log_time) > 1000000) { // 1 second
      logStats();
      last_log_time = current_time;
    }
  }

private:
  void logStats() {
    if (call_count_ > 0) {
      // Always log all stats for now
      uint32_t avg = total_time_ / call_count_;
      printf("[PROF] %-30s: avg=%4u us, calls=%5u, total=%6llu us\n", name_,
             avg, call_count_, total_time_);

      // Reset counters after logging
      total_time_ = 0;
      call_count_ = 0;
    }
  }

  const char *name_;
  uint32_t total_time_;
  uint32_t call_count_;
  uint32_t last_log_time_;
};

// Macro for measuring average time of a specific code block
#if ENABLE_PROFILING
#define PROFILE_AVERAGE(name)                                                  \
  static AverageProfiler avg_prof(name);                                       \
  uint32_t start_time_avg = micros();                                          \
  struct scope_guard {                                                         \
    AverageProfiler &prof;                                                     \
    uint32_t &start;                                                           \
    ~scope_guard() { prof.addSample(time_diff(micros(), start)); }             \
  } guard{avg_prof, start_time_avg};
#else
#define PROFILE_AVERAGE(name)
#endif

#endif // _PROFILER_H_
