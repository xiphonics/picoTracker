# Host tests

This directory builds a host-only test binary using doctest in `sources/Externals/doctest`.

## Build

From repo root:

```
cmake -S tests -B build-host
cmake --build build-host
```

## Run

```
./build-host/picoTracker_tests
```
