#ifndef _PROFILER_H_
#define _PROFILER_H_

#define MAX_PROFILERS 4

#include <Externals/etl/include/etl/map.h>
#include <Externals/etl/include/etl/string.h>
#include <System/Console/Trace.h>
#include <cstdio>

// Helper function to safely calculate time differences with wrap-around
uint32_t time_diff(uint32_t newer, uint32_t older);

// Enable/disable profiling with a preprocessor flag
// defaults to disabled
#ifndef ENABLE_PROFILING
#define ENABLE_PROFILING 1
#endif

// Moving average window size (1 second at 60fps)
#define MOVING_AVG_WINDOW 60

class MovingAverageProfiler {
public:
  MovingAverageProfiler(const char *name);
  void addSample(uint32_t duration);

private:
  static const uint8_t WINDOW_SIZE = 60; // 1 second at 60fps
  uint32_t samples[WINDOW_SIZE] = {0};
  uint32_t sample_count = 0;
  uint32_t last_log_time = 0;
  etl::string<32> name_;
};

class Profiler {
public:
  // Standard profiler constructor (one-time measurement)
  explicit Profiler(const char *name);

  // Constructor for moving average profiler
  static Profiler MovingAverage(const char *name);

  ~Profiler();

private:
  // Static map for multiple moving average profilers
  static etl::map<etl::string<32>, MovingAverageProfiler, MAX_PROFILERS>
      moving_avg_profilers_;

  // Private constructor for moving average profilers
  Profiler(const etl::string<32> &name, bool use_moving_avg);

  etl::string<32> name_;
  uint32_t start_time_;
  bool use_moving_avg_;
  uint32_t total_time_;
  uint32_t call_count_;
  uint32_t last_log_time_;
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
  AverageProfiler(const char *name);
  ~AverageProfiler();
  void addSample(uint32_t duration);

private:
  void logStats();

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
