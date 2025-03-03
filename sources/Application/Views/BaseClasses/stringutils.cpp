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

void deleteChar(char *name, uint8_t pos) {
  int len = std::strlen(name);

  // If length is 1 or position is invalid, do nothing
  if (len <= 1 || pos < 0 || pos >= len) {
    return;
  }

  // Shift characters left starting from the position
  for (int i = pos; i < len - 1; ++i) {
    name[i] = name[i + 1];
  }

  // Null-terminate the string at the new end
  name[len - 1] = '\0';
}