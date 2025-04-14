#ifndef _PI_FILE_H_
#define _PI_FILE_H_

class PI_File {
public:
  PI_File() {}
  virtual ~PI_File() {}

  virtual int Read(void *ptr, int size) = 0;
  virtual int GetC() = 0;
  virtual int Write(const void *ptr, int size, int nmemb) = 0;
  virtual void Seek(long offset, int whence) = 0;
  virtual long Tell() = 0;
  virtual bool Close() = 0;
  virtual bool DeleteFile() = 0;
  virtual int Error() = 0;
};

#endif // _PI_FILE_H_
