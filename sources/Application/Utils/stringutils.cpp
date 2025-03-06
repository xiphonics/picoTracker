#include <cstring>
#include <stdint.h>

char getNext(char c, bool reverse) {
  // Valid characters in order
  const char validChars[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._";
  const int numChars = sizeof(validChars) - 1; // Exclude null terminator

  // Find the index of the current character
  for (int i = 0; i < numChars; ++i) {
    if (validChars[i] == c) {
      // Calculate next index based on direction (forward or reverse)
      int nextIndex =
          reverse ? (i - 1 + numChars) % numChars : (i + 1) % numChars;
      return validChars[nextIndex];
    }
  }

  // If character is not valid, return the first valid character in the list
  return reverse ? validChars[numChars - 1] : validChars[0];
};