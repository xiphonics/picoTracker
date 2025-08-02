#include <sys/errno.h>
#include <sys/time.h>
#include <unistd.h>

// This is a stub implementation for the _getentropy function, which is
// required by the C library but not provided on bare-metal systems.
// This implementation signals an error (EIO) and returns -1, which
// is a safe default for applications that do not require true random
// number generation.
int _getentropy(void *buffer, size_t length) {
  // Set the error number to EIO (I/O error) to indicate that
  // we don't have a source of entropy.
  errno = EIO;
  return -1;
}

// This is a stub implementation for the _gettimeofday function.
// The C library requires this for time-related operations.
// On our stm32 board we dont have a real-time clock,
// so we return a default value
int _gettimeofday(struct timeval *tv, void *tz) {
  if (tv) {
    tv->tv_sec = 0;  // Default to 0 seconds
    tv->tv_usec = 0; // Default to 0 microseconds
  }
  // Returning 0 indicates success, which is fine for non-time-critical
  // applications.
  return 0;
}