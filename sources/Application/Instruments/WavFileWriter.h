#ifndef _WAV_FILE_WRITER_H_
#define _WAV_FILE_WRITER_H_

#include "Application/Utils/fixed.h"
#include "System/FileSystem/FileSystem.h"

class WavFileWriter {
public:
  WavFileWriter(const char *path);
  ~WavFileWriter();
  void AddBuffer(fixed *, int size); // size in samples
  void Close();

private:
  int sampleCount_;
  short *buffer_;
  int bufferSize_;
  PI_File *file_;
};
#endif
