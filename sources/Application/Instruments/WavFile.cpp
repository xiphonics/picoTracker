
#include "WavFile.h"
#include "Application/Model/Config.h"
#include "Foundation/Types/Types.h"
#include "Services/Time/TimeService.h"
#include "System/Console/Trace.h"
#include <stdlib.h>

#ifdef LOAD_IN_FLASH
#include "hardware/flash.h"
#include "hardware/sync.h"
#endif

int WavFile::bufferChunkSize_ = -1;
unsigned char WavFile::readBuffer_[512];

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

WavFile::WavFile(PI_File *file) {
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
#ifndef LOAD_IN_FLASH
  SAFE_FREE(samples_);
#endif
};

WavFile *WavFile::Open(const char *name) {
  Trace::Log("WAVFILE", "wave open from %s", name);

  // open file
  PicoFileSystem *fs = PicoFileSystem::GetInstance();
  PI_File *file = fs->Open(name, "r");

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

  wav->size_ =
      size / nChannels / bitPerSample; // Size in samples (stereo/16bits)

  wav->dataPosition_ = position;

  return wav;
};

void *WavFile::GetSampleBuffer(int note) { return samples_; };

int WavFile::GetSize(int note) { return size_; };

int WavFile::GetChannelCount(int note) { return channelCount_; };

int WavFile::GetSampleRate(int note) { return sampleRate_; };

long WavFile::readBlock(long start, long size) {
  // Read buffer is a fixed size, nothing should be requested bigger than this
  // TODO: remove size option and work with what we have
#ifdef PICOBUILD
  assert((unsigned long)size <= FLASH_PAGE_SIZE);
#endif
  if (size > readBufferSize_) {
    readBufferSize_ = size;
  }
  file_->Seek(start, SEEK_SET);
  file_->Read(readBuffer_, size);
  return size;
};

bool WavFile::GetBuffer(long start, long size) {
  // TODO: Many of the calculations in this function don't make any
  // sense anymore, refactor
  // 64 bits is the maximum size we can read without overflowing
  // readBuffer_ in the worst case scenario
#ifdef PICOBUILD
  assert((unsigned long)size < FLASH_PAGE_SIZE / 2);
#endif
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
    if (bufferChunkSize_ > 0)
      TimeService::GetInstance()->Sleep(1);
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

#ifdef LOAD_IN_FLASH
bool __not_in_flash_func(WavFile::LoadInFlash)(int &flashEraseOffset,
                                               int &flashWriteOffset,
                                               int &flashLimit) {

  // Size needed in flash before accounting for page size
  int FlashBaseBufferSize = 2 * channelCount_ * size_;
  // Store the size of samples
  sampleBufferSize_ = FlashBaseBufferSize;
  // Size actually occupied in flash
  int FlashPageBufferSize = ((FlashBaseBufferSize / FLASH_PAGE_SIZE) +
                             ((FlashBaseBufferSize % FLASH_PAGE_SIZE) != 0)) *
                            FLASH_PAGE_SIZE;

  if (flashWriteOffset + FlashPageBufferSize > flashLimit) {
    Trace::Error("Sample doesn't fit in available Flash (need: %i - avail: %i)",
                 FlashPageBufferSize, flashLimit - flashWriteOffset);
    return false;
  }

  // Pointer to location in flash
  samples_ = (short *)(XIP_BASE + flashWriteOffset);

  // Any operation on the flash need to ensure that nothing else reads or writes
  // on it We disable IRQs and ensure that we don't have multiprocessing on at
  // this time
  int irqs = save_and_disable_interrupts();

  // TODO: Need to remove this hacky delay workaround and properly fix
  // underlying issue with disabling interrupts needed to import samples into
  // Flash !!

  // This is required due to strange issue with above interrupts disable causing
  // a crash without this delay but only in deoptimised debug builds
  for (int i = 0; i < 100000; i++) {
    if (i % 10000 == 0) {
      Trace::Log("WAVFILE", ".");
    }
  }

  // If data doesn't fit in previously erased page, we'll have to erase
  // additional ones
  if (FlashPageBufferSize > (flashEraseOffset - flashWriteOffset)) {
    int additionalData =
        FlashPageBufferSize - flashEraseOffset + flashWriteOffset;
    int sectorsToErase = ((additionalData / FLASH_SECTOR_SIZE) +
                          ((additionalData % FLASH_SECTOR_SIZE) != 0)) *
                         FLASH_SECTOR_SIZE;
    Trace::Debug("About to erase %i sectors in flash region 0x%X - 0x%X",
                 sectorsToErase, flashEraseOffset,
                 flashEraseOffset + sectorsToErase);
    // Erase required number of sectors
    flash_range_erase(flashEraseOffset, sectorsToErase);
    // Move erase pointer to new position
    flashEraseOffset += sectorsToErase;
  }

  // Actual buffer needed to read whole file (may be lower than
  // FlashBaseBufferSize if it's an 8bit sample)
  int bufferSize = size_ * channelCount_ * bytePerSample_;
  // Where the data starts in the WAV (after header)
  int bufferStart = dataPosition_;

  // Read the buffer but in small chunk to let the system breathe
  // if the files are big
  uint count = bufferSize;
  uint offset = 0;
  uint readSize = count > FLASH_PAGE_SIZE ? FLASH_PAGE_SIZE : count;

  while (count > 0) {
    readSize = (count > readSize) ? readSize : count;

    file_->Seek(bufferStart, SEEK_SET);
    file_->Read(readBuffer_, readSize);

    // Have to expand 8 bit data (if needed) before writing to flash
    unsigned char *src = (unsigned char *)readBuffer_;
    short *dst = (short *)readBuffer_;
    for (int i = readSize - 1; i >= 0; i--) {
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

    // We need to write double the bytes if we needed to expand to 16 bit
    // Write size will be either 256 (which is the flash page size) or 512
    int writeSize = (bytePerSample_ == 1) ? readSize * 2 : readSize;
    // Adjust to page size
    writeSize =
        ((writeSize / FLASH_PAGE_SIZE) + ((writeSize % FLASH_PAGE_SIZE) != 0)) *
        FLASH_PAGE_SIZE;

    // There will be trash at the end, but sampleBufferSize_ gives me the
    // bounds
    // Trace::Debug("About to write %i sectors in flash region 0x%X - 0x%X",
    //  writeSize, flashWriteOffset + offset,
    //  flashWriteOffset + offset + writeSize);

    flash_range_program(flashWriteOffset + offset, (uint8_t *)readBuffer_,
                        writeSize);
    bufferStart += readSize;
    count -= readSize;
    flashWriteOffset += writeSize;
  }

  // Lastly we restore the IRQs
  restore_interrupts(irqs);
  return true;
};
#endif

void WavFile::Close() {
  file_->Close();
  SAFE_DELETE(file_);
  readBufferSize_ = 0;
};

int WavFile::GetRootNote(int note) { return 60; }
