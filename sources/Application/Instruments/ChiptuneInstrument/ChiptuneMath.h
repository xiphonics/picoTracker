#pragma once

#include <cstdint>

static inline uint32_t multiplyQ32(uint32_t a, uint32_t b) {
  uint64_t tmp = (uint64_t)a * b;
  return (uint32_t)(tmp >> 32);
}

static inline uint16_t interpolateU16(const uint16_t *lut, uint8_t v) {
  uint8_t idx = v >> 2; // 0..63
  uint8_t frac = v & 3; // 0..3

  uint16_t c0 = lut[idx];
  uint16_t c1 = lut[idx + 1]; // safe because of sentinel

  return c0 + (((uint32_t)(c1 - c0) * frac) >> 2);
}

static inline int8_t interpolateS8(const int8_t *lut, uint8_t v) {
  uint8_t idx = v >> 2; // 0..63
  uint8_t frac = v & 3; // 0..3

  int8_t c0 = lut[idx];
  int8_t c1 = lut[idx + 1]; // safe because of sentinel

  return c0 + (((int32_t)(c1 - c0) * frac) >> 2);
}

static inline int32_t sign(int32_t x) { return (x > 0) - (x < 0); }