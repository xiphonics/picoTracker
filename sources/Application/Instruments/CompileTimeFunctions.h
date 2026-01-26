#pragma once

// power function
consteval double pow_ct(double base, double exp) {
  if (exp == 0.0)
    return 1.0;
  if (exp < 0.0)
    return 1.0 / pow_ct(base, -exp);

  double result = 1.0;
  int int_exp = static_cast<int>(exp);
  double frac_exp = exp - int_exp;

  // Integer part
  for (int i = 0; i < int_exp; i++) {
    result *= base;
  }

  // Fractional part approximation using Taylor series for 2^x
  if (frac_exp != 0.0) {
    double x = frac_exp * 0.693147180559945309417; // ln2
    double term = 1.0;
    double sum = 1.0;
    for (int i = 1; i < 20; i++) {
      sum += (term *= x / i);
    }
    result *= sum;
  }
  return result;
}

// Calculate semitone ratio in Q16.16 format at compile time
consteval uint32_t calculateSemitoneRatioQ16(int index) {
  int semitones = index - 128; // -128 to +127
  double ratio = pow_ct(2.0, semitones / 12.0);
  return static_cast<uint32_t>(ratio * 65536.0 + 0.5);
}

// Generate the lookup table at compile time
template <size_t... Is>
consteval auto makeSemitoneRatioTable(std::index_sequence<Is...>) {
  return std::array<uint32_t, sizeof...(Is)>{calculateSemitoneRatioQ16(Is)...};
}
