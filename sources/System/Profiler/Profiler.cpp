#include "Profiler.h"

// map to hold named profilers
etl::map<etl::string<32>, MovingAverageProfiler, MAX_PROFILERS>
    Profiler::moving_avg_profilers_;
