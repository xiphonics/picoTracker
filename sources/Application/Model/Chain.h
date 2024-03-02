#ifndef _CHAIN_H_
#define _CHAIN_H_

#define CHAIN_COUNT 0x80
#define NO_MORE_CHAIN 0x81

class Chain {
public:
  Chain();
  ~Chain();
  unsigned short GetNext();
  bool IsUsed(unsigned char i) { return isUsed_[i]; };
  void SetUsed(unsigned char c);
  void ClearAllocation();

  unsigned char data_[CHAIN_COUNT * 16];
  unsigned char transpose_[CHAIN_COUNT * 16];

private:
  bool isUsed_[CHAIN_COUNT];
};

#endif
