
#include "WavFile.h"
#include "Application/Model/Config.h"
#include "Foundation/Types/Types.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include <stdlib.h>

int WavFile::bufferChunkSize_ = -1;
unsigned char WavFile::readBuffer_[BUFFER_SIZE];

short Swap16(short from) {
#ifdef __ppc__
  short result;
  ((char *)&result)[0] = ((char *)&from)[1];
  ((char *)&result)[1] = ((char *)&from)[0];
  return result;
#else
  return from;
#endif
}

int Swap32(int from) {
#ifdef __ppc__
  int result;
  ((char *)&result)[0] = ((char *)&from)[3];
  ((char *)&result)[1] = ((char *)&from)[2];
  ((char *)&result)[2] = ((char *)&from)[1];
  ((char *)&result)[3] = ((char *)&from)[0];
  return result;
#else
  return from;
#endif
}

WavFile::WavFile(I_File *file) {
  samples_ = 0;
  size_ = 0;
  readBufferSize_ = 0;
  sampleBufferSize_ = 0;
  file_ = file;
};

WavFile::~WavFile() {
  if (file_) {
    file_->Close();
    delete file_;
  }
};

WavFile *WavFile::Open(const char *name) {
  // Trace::Log("WAVFILE", "wave open from %s", name);

  // open file
  FileSystem *fs = FileSystem::GetInstance();
  I_File *file = fs->Open(name, "r");

  if (!file)
    return 0;

  WavFile *wav = new WavFile(file);

  // Get data

  /*        file->Seek(0,SEEK_SET) ;
          file->Read(fileBuffer,filesize,1) ;
          uchar *ptr=fileBuffer ;*/

  // Trace::Dump("Loading sample from %s",path) ;

  long position = 0;

  // Read 'RIFF'
  unsigned int chunk;

  position += wav->readBlock(position, 4);
  memcpy(&chunk, wav->readBuffer_, 4);
  chunk = Swap32(chunk);

  if (chunk != 0x46464952) {
    Trace::Error("Bad RIFF format %x", chunk);
    delete (wav);
    return 0;
  }

  // Read size
  unsigned int size;
  position += wav->readBlock(position, 4);
  memcpy(&size, wav->readBuffer_, 4);
  size = Swap32(size);

  // Read WAVE
  position += wav->readBlock(position, 4);
  memcpy(&chunk, wav->readBuffer_, 4);
  chunk = Swap32(chunk);

  if (chunk != 0x45564157) {
    Trace::Error("Bad WAV format");
    delete wav;
    return 0;
  }

  // Read fmt
  position += wav->readBlock(position, 4);
  memcpy(&chunk, wav->readBuffer_, 4);
  chunk = Swap32(chunk);

  if (chunk != 0x20746D66) {
    Trace::Error("Bad WAV/fmt format");
    delete wav;
    return 0;
  }

  // Read subchunk size
  position += wav->readBlock(position, 4);
  memcpy(&size, wav->readBuffer_, 4);
  size = Swap32(size);

  if (size < 16) {
    Trace::Error("Bad fmt size format");
    delete wav;
    return 0;
  }
  int offset = size - 16;

  // Read compression
  unsigned short comp;
  position += wav->readBlock(position, 2);
  memcpy(&comp, wav->readBuffer_, 2);
  comp = Swap16(comp);

  if (comp != 1) {
    Trace::Error("Unsupported compression");
    delete wav;
    return 0;
  }

  // Read NumChannels (mono/Stereo)
  unsigned short nChannels;
  position += wav->readBlock(position, 2);
  memcpy(&nChannels, wav->readBuffer_, 2);
  nChannels = Swap16(nChannels);

  // Read Sample rate
  unsigned int sampleRate;

  position += wav->readBlock(position, 4);
  memcpy(&sampleRate, wav->readBuffer_, 4);
  sampleRate = Swap32(sampleRate);

  // Skip byteRate & blockalign
  position += 6;

  short bitPerSample;
  position += wav->readBlock(position, 2);
  memcpy(&bitPerSample, wav->readBuffer_, 2);
  bitPerSample = Swap16(bitPerSample);

  if ((bitPerSample != 16) && (bitPerSample != 8)) {
    Trace::Error("Only 8/16 bit supported");
    delete wav;
    return 0;
  };
  bitPerSample /= 8;
  wav->bytePerSample_ = bitPerSample;

  // some bad files have bigger chunks
  if (offset) {
    position += offset;
  }

  // read data subchunk header
  // Trace::Dump("data subch") ;

  position += wav->readBlock(position, 4);
  memcpy(&chunk, wav->readBuffer_, 4);
  chunk = Swap32(chunk);

  while (chunk != 0x61746164) {
    position += wav->readBlock(position, 4);
    memcpy(&size, wav->readBuffer_, 4);
    size = Swap32(size);

    position += size;
    position += wav->readBlock(position, 4);
    memcpy(&chunk, wav->readBuffer_, 4);
    chunk = Swap32(chunk);
  }

  wav->sampleRate_ = sampleRate;
  wav->channelCount_ = nChannels;

  // Read data size in byte

  position += wav->readBlock(position, 4);
  memcpy(&size, wav->readBuffer_, 4);
  size = Swap32(size);
  Trace::Debug("File size: %i", size);

  wav->size_ =
      size / nChannels / bitPerSample; // Size in samples (stereo/16bits)
  Trace::Debug("File size_: %i", wav->size_);
  // All samples are saved as 16bit/sample into disk
  wav->sampleBufferSize_ = wav->size_ * nChannels * 2;
  Trace::Debug("File sampleBufferSize_: %i", wav->sampleBufferSize_);
  wav->readCount_ = size;

  wav->dataPosition_ = position;
  return wav;
};

void *WavFile::GetSampleBuffer(int note) { return samples_; };

void WavFile::SetSampleBuffer(short *ptr) { samples_ = ptr; }

int WavFile::GetSize(int note) { return size_; };

int WavFile::GetChannelCount(int note) { return channelCount_; };

int WavFile::GetSampleRate(int note) { return sampleRate_; };

long WavFile::readBlock(long start, long size) {
  if (size > readBufferSize_) {
    readBufferSize_ = size;
  }
  file_->Seek(start, SEEK_SET);
  file_->Read(readBuffer_, size);
  return size;
};

bool WavFile::GetBuffer(long start, long size) {
  samples_ = (short *)readBuffer_;

  // compute the file buffer size we need to read

  int bufferSize = size * channelCount_ * bytePerSample_;
  int bufferStart = dataPosition_ + start * channelCount_ * bytePerSample_;

  // Read the buffer but in small chunk to let the system breathe
  // if the files are big

  int count = bufferSize;
  int offset = 0;
  char *ptr = (char *)samples_;
  int readSize = (bufferChunkSize_ > 0) ? bufferChunkSize_
                 : count > 4096         ? 4096
                                        : count;

  while (count > 0) {
    readSize = (count > readSize) ? readSize : count;
    readBlock(bufferStart, readSize);
    memcpy(ptr + offset, readBuffer_, readSize);
    bufferStart += readSize;
    count -= readSize;
    offset += readSize;
  }

  // expand 8 bit data if needed

  unsigned char *src = (unsigned char *)samples_;
  short *dst = samples_;
  for (int i = size - 1; i >= 0; i--) {
    if (bytePerSample_ == 1) {
      dst[i] = (src[i] - 128) * 256;
    } else {
      *dst = Swap16(*dst);
      dst++;
      if (channelCount_ > 1) {
        *dst = Swap16(*dst);
        dst++;
      }
    }
  }
  return true;
};

uint32_t WavFile::GetDiskSize(int note) { return sampleBufferSize_; }

// rewind to start of data (no header)
bool WavFile::Rewind() {
  file_->Seek(dataPosition_, SEEK_SET);
  readCount_ = size_ * channelCount_ * bytePerSample_;
  return true;
};

// incrementally read file, use rewind method to go to beginning
bool WavFile::Read(void *buff, uint32_t btr, uint32_t *bytesRead) {
  // Calculate read size based on byte per sample
  uint32_t readSize = (bytePerSample_ == 1) ? btr / 2 : btr;

  // Adjust for bytes remaining
  readSize = readCount_ > readSize ? readSize : readCount_;

  // Read from file
  *bytesRead = file_->Read(buff, readSize);

  // For 8-bit samples, we'll expand to 16-bit
  if (bytePerSample_ == 1) {
    // Process 8-bit samples, converting them to 16-bit
    uint8_t *src = (uint8_t *)buff;
    uint16_t *dst = (uint16_t *)buff;

    // We need to process from end to beginning to avoid overwriting data
    for (int i = *bytesRead - 1; i >= 0; i--) {
      dst[i] = (src[i] - 128) * 256;
    }

    // Adjust bytes read count for 16-bit output
    *bytesRead = *bytesRead * 2;
  } else {
    // Process 16-bit samples (byte swap if needed)
    uint16_t *data = (uint16_t *)buff;
    uint32_t sampleCount = *bytesRead / 2; // 16-bit = 2 bytes per sample

    // Process all samples with proper bounds checking
    for (uint32_t i = 0; i < sampleCount; i++) {
      data[i] = Swap16(data[i]);
    }
  }

  // Update remaining bytes counter
  readCount_ -= readSize;
  return true;
}

void WavFile::Close() {
  file_->Close();
  SAFE_DELETE(file_);
  readBufferSize_ = 0;
};

int WavFile::GetRootNote(int note) { return 60; }
