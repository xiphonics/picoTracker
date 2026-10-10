/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 xiphonics, inc.
 */
#ifndef PICOTRACKER_PATH_H
#define PICOTRACKER_PATH_H

#include "Externals/etl/include/etl/string.h"
#include <cstddef>
#include <cstring>

namespace picoTrackerPath {

// Windows MAX_PATH includes the terminating null; paths here are ASCII bytes.
constexpr size_t BufferSize = 260;
using PathString = etl::string<BufferSize - 1>;

inline bool Resolve(const char *cwd, const char *path, PathString &out) {
  if (cwd == nullptr || cwd[0] != '/' || path == nullptr || path[0] == '\0' ||
      std::strlen(path) >= BufferSize) {
    return false;
  }
  out = path[0] == '/' ? "/" : cwd;
  if (out.is_truncated()) {
    return false;
  }

  while (*path != '\0') {
    while (*path == '/') {
      ++path;
    }
    const char *segment = path;
    while (*path != '\0' && *path != '/') {
      ++path;
    }
    const size_t length = path - segment;
    if (length == 0 || (length == 1 && segment[0] == '.')) {
      continue;
    }
    if (length == 2 && segment[0] == '.' && segment[1] == '.') {
      const size_t separator = out.find_last_of('/');
      out.resize(separator == 0 ? 1 : separator);
      continue;
    }
    const size_t separatorLength = out.size() == 1 ? 0 : 1;
    if (out.size() + separatorLength + length > out.capacity()) {
      return false;
    }
    if (separatorLength != 0) {
      out.push_back('/');
    }
    out.append(segment, length);
  }
  return true;
}

} // namespace picoTrackerPath
#endif
