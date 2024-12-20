// Add to C:\Users\Bill\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.13\libraries\SPI\SPI.h
  void transfer(const void *txBuf, void *rxBuf, size_t count);

//Add to C:\Users\Bill\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.13\libraries\SPI\SPI.c
void SPIClassSAMD::transfer(const void *txBuf, void *rxBuf, size_t count) {
  _p_sercom->transferDataSPI(txBuf, rxBuf, count);
}

//Add to C:\Users\Bill\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.13\cores\arduino\SERCOM.h
   void transferDataSPI(const void* txBuf, void* rxBuf, size_t count);

// Add to C:\Users\Bill\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.13\cores\arduino\api\HstdwareSPI.h
    virtual void transfer(const void *tx, void *rx, size_t count) = 0;

//Add to C:\Users\Bill\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.13\cores\arduino\SERCOM.cpp

void SERCOM::transferDataSPI(const void *txBuf, void *rxBuf, size_t count) {
  const uint8_t *tx = reinterpret_cast<const uint8_t *>(txBuf);
  uint8_t *rx = reinterpret_cast<uint8_t *>(rxBuf);
  size_t ir = 0;
  size_t it = 0;
  if (rx) {
    while (it < 2 && it < count) {
      if (sercom->SPI.INTFLAG.bit.DRE) {
        sercom->SPI.DATA.reg = tx ? tx[it] : 0XFF;
        it++;
      }
    }
    while (it < count) {
      if (sercom->SPI.INTFLAG.bit.RXC) {
        rx[ir++] = sercom->SPI.DATA.reg;
        sercom->SPI.DATA.reg = tx ? tx[it] : 0XFF;
        it++;
      }
    }
    while (ir < count) {
      if (sercom->SPI.INTFLAG.bit.RXC) {
        rx[ir++] = sercom->SPI.DATA.reg;
      }
    }
  } else if (tx && count) {  // might hang if count == 0
    // Writing '0' to this bit will disable the SPI receiver immediately.
    // The receive buffer will be flushed, data from ongoing receptions
    // will be lost and STATUS.BUFOVF will be cleared.
    sercom->SPI.CTRLB.bit.RXEN = 0;
    while (it < count) {
      if (sercom->SPI.INTFLAG.bit.DRE) {
        sercom->SPI.DATA.reg = tx[it++];
      }
    }
    // wait till all data sent
    while (sercom->SPI.INTFLAG.bit.TXC == 0) {
    }
    // Writing '1' to CTRLB.RXEN when the SPI is enabled will set
    // SYNCBUSY.CTRLB, which will remain set until the receiver is
    // enabled, and CTRLB.RXEN will read back as '1'.
    sercom->SPI.CTRLB.bit.RXEN = 1;
    while (sercom->SPI.CTRLB.bit.RXEN == 0) {
    }
  }
}