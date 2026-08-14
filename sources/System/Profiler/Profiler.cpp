#include "Profiler.h"
#include "System/System/System.h"

// Helper function to safely calculate time differences with wrap-around
uint32_t time_diff(uint32_t newer, uint32_t older) {
  return newer -
         older; // This works correctly for uint32_t even with wrap-around
}

// MovingAverageProfiler implementation
MovingAverageProfiler::MovingAverageProfiler(const char *name) : name_(name) {}

void MovingAverageProfiler::addSample(uint32_t duration) {
  // Store the sample
  samples[sample_count % WINDOW_SIZE] = duration;
  sample_count++;

  System *sys = System::GetInstance();
  uint32_t current_time = sys->Micros();

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
  uint32_t count = (sample_count < WINDOW_SIZE) ? sample_count : WINDOW_SIZE;
  if (count == 0) {
    last_log_time = current_time;
    return;
  }

  uint64_t total = 0;
  for (uint32_t i = 0; i < count; i++) {
    total += samples[i];
  }

  uint32_t avg = (uint32_t)(total / count);
  Trace::Log("PROFILER", "%-30s: avg=%lu us (samples: %d)", name_.c_str(),
             (unsigned long)avg, count);

  // Reset for next window
  sample_count = 0;
  last_log_time = current_time;
}

// map to hold named profilers
etl::map<etl::string<32>, MovingAverageProfiler, MAX_PROFILERS>
    Profiler::moving_avg_profilers_;

// Profiler implementation
Profiler::Profiler(const char *name) : Profiler(etl::string<32>(name), false) {}

Profiler::Profiler(const etl::string<32> &name, bool use_moving_avg)
    : name_(name.c_str()), start_time_(0), use_moving_avg_(use_moving_avg),
      total_time_(0), call_count_(0), last_log_time_(0) {
#if ENABLE_PROFILING
  start_time_ = System::GetInstance()->Micros();
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

Profiler Profiler::MovingAverage(const char *name) {
  return Profiler(etl::string<32>(name), true);
}

Profiler::~Profiler() {
#if ENABLE_PROFILING

  uint32_t end_time = System::GetInstance()->Micros();
  uint32_t duration = time_diff(end_time, start_time_);

  if (use_moving_avg_) {
    auto it = moving_avg_profilers_.find(name_);
    if (it != moving_avg_profilers_.end()) {
      it->second.addSample(duration);
    }
  } else {
    Trace::Log("PROFILER", "%-30s: %6lu us", name_.c_str(),
               (unsigned long)duration);
  }
#endif
}

// AverageProfiler implementation
AverageProfiler::AverageProfiler(const char *name)
    : name_(name), total_time_(0), call_count_(0), last_log_time_(0) {}

AverageProfiler::~AverageProfiler() { logStats(); }

void AverageProfiler::addSample(uint32_t duration) {
  total_time_ += duration;
  call_count_++;

  // Log stats every 1 second (1,000,000 microseconds)
  System *sys = System::GetInstance();
  uint32_t current_time = sys->Micros();
  if (last_log_time_ == 0) {
    last_log_time_ = current_time;
  } else if (time_diff(current_time, last_log_time_) > 1000000) { // 1 second
    logStats();
    last_log_time_ = current_time;
  }
}

void AverageProfiler::logStats() {
  if (call_count_ > 0) {
    uint32_t avg = total_time_ / call_count_;
    printf("[PROFILER] %-30s: avg=%4lu us, calls=%5lu, total=%6lu us\n",
           name_, static_cast<unsigned long>(avg),
           static_cast<unsigned long>(call_count_),
           static_cast<unsigned long>(total_time_));

    // Reset counters after logging
    total_time_ = 0;
    call_count_ = 0;
  }
}
