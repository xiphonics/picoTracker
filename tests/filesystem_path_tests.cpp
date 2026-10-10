#include "Adapters/picoTracker/filesystem/picoTrackerPath.h"
#include "doctest/doctest.h"

using picoTrackerPath::PathString;
using picoTrackerPath::Resolve;

TEST_CASE("filesystem resolves paths from its own working directory") {
  PathString path;
  REQUIRE(Resolve("/projects/song/samples", "../lgptsav.dat", path));
  CHECK(path == "/projects/song/lgptsav.dat");
  REQUIRE(Resolve("/projects/song", "/samples//drums/./kick.wav", path));
  CHECK(path == "/samples/drums/kick.wav");
  REQUIRE(Resolve("/projects/song", "../../..", path));
  CHECK(path == "/");
  REQUIRE(Resolve("/", ".", path));
  CHECK(path == "/");
  REQUIRE(Resolve("/samples", "..kit/kick.wav", path));
  CHECK(path == "/samples/..kit/kick.wav");
}

TEST_CASE("filesystem applies MAX_PATH including the null terminator") {
  PathString base("/");
  base.append(248, 'a');
  PathString path;
  REQUIRE(Resolve(base.c_str(), "123456789", path));
  CHECK(path.size() == 259);
  CHECK_FALSE(Resolve(base.c_str(), "1234567890", path));
  base = "/";
  base.append(258, 'a');
  REQUIRE(Resolve("/", base.c_str(), path));
  CHECK(path.size() == 259);
  REQUIRE(Resolve(base.c_str(), "..", path));
  CHECK(path == "/");
}

TEST_CASE("filesystem rejects empty and invalid path inputs") {
  PathString path;
  CHECK_FALSE(Resolve("/samples", "", path));
  CHECK_FALSE(Resolve("/samples", nullptr, path));
  CHECK_FALSE(Resolve(nullptr, "kick.wav", path));
  CHECK_FALSE(Resolve("samples", "kick.wav", path));
  etl::string<260> tooLong("/");
  tooLong.append(259, 'a');
  CHECK_FALSE(Resolve("/", tooLong.c_str(), path));
}
