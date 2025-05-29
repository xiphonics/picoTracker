#ifndef _PROFILER_H_
#define _PROFILER_H_

#include "pico/stdio.h"
#include "pico/time.h"
#include <cstdio>
#include <cstring>

// Enable/disable profiling with a preprocessor flag
// defaults to disabled
#ifndef ENABLE_PROFILING
#define ENABLE_PROFILING 0
#endif

// Moving average window size (1 second at 60fps)
#define RENDER_AVG_WINDOW 60

class RenderProfiler {
  static const int WINDOW_SIZE = 60; // 1 second at 60fps
  uint64_t samples[WINDOW_SIZE] = {0};
  int sample_count = 0;
  uint64_t last_log_time = 0;
  const char *name_;

public:
  RenderProfiler(const char *name) : name_(name) {}

  void addSample(uint64_t duration) {
    samples[sample_count % WINDOW_SIZE] = duration;
    sample_count++;

    uint64_t current_time = time_us_64();
    if (current_time - last_log_time >= 1000000) { // 1 second
      uint64_t total = 0;
      int count = (sample_count < WINDOW_SIZE) ? sample_count : WINDOW_SIZE;
      for (int i = 0; i < count; i++) {
        total += samples[i];
      }
      uint64_t avg = count > 0 ? total / count : 0;
      printf("[RENDER] %-30s: avg=%6llu us\n", name_, avg);
      last_log_time = current_time;
    }
  }
};

class Profiler {
  static RenderProfiler *render_profiler; // Declare static member
  const char *name_;
  uint64_t start_time_;
  bool is_render_;
  uint64_t total_time_;
  uint32_t call_count_;
  uint64_t last_log_time_;

public:
  Profiler(const char *name)
      : name_(name), is_render_(strstr(name, "Render") != nullptr) {
#if ENABLE_PROFILING
    start_time_ = time_us_64();
    total_time_ = 0;
    call_count_ = 0;
    last_log_time_ = 0;
    if (is_render_ && render_profiler == nullptr) {
      static RenderProfiler instance("Render");
      render_profiler = &instance;
    }
#endif
  }

  ~Profiler() {
#if ENABLE_PROFILING
    uint64_t end_time = time_us_64();
    uint64_t duration = end_time - start_time_;

    if (is_render_ && render_profiler) {
      render_profiler->addSample(duration);
    } else {
      printf("[PROFILER] %-30s: %6llu us\n", name_, duration);
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

  void addSample(uint64_t duration) {
    static int sample_count = 0;
    sample_count++;

    // Debug: Print every 1000th sample
    if (sample_count % 1000 == 0) {
      printf("[PROF_DEBUG] %-30s: sample %d, duration=%llu\n", name_,
             sample_count, duration);
    }

    total_time_ += duration;
    call_count_++;

    // Log stats every 1 second (1,000,000 microseconds)
    static uint64_t last_log_time = 0;
    uint64_t current_time = time_us_64();
    if (current_time - last_log_time > 1000000) { // 1 second
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
  uint64_t total_time_;
  uint32_t call_count_;
  uint64_t last_log_time_;
};

// Macro for measuring average time of a specific code block
#if ENABLE_PROFILING
#define PROFILE_AVERAGE(name)                                                  \
  static AverageProfiler avg_prof(name);                                       \
  uint64_t start_time_avg = time_us_64();                                      \
  struct scope_guard {                                                         \
    AverageProfiler &prof;                                                     \
    uint64_t &start;                                                           \
    ~scope_guard() { prof.addSample(time_us_64() - start); }                   \
  } guard{avg_prof, start_time_avg};
#else
#define PROFILE_AVERAGE(name)
#endif

#endif // _PROFILER_H_
