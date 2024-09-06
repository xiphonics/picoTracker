#ifndef SERIAL_DEBUG_UI_H_
#define SERIAL_DEBUG_UI_H_

#include "pico/stdlib.h"

class SerialDebugUI {
public:
  SerialDebugUI();
  bool readSerialIn(char *buffer, short size);
  void dispatchCmd(char *cmd);
  void catFile(const char *path);
  void listFiles(const char *path);
  void rmFile(const char *path);

private:
  int lp_ = 0;
};

#endif