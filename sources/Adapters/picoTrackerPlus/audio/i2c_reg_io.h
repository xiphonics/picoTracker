#ifndef _I2C_REG_IO_H_
#define _I2C_REG_IO_H_

#include "hardware/i2c.h"

int reg_write_byte(i2c_inst_t *i2c, const uint addr, const uint8_t reg,
                   uint8_t val);

int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf,
              const uint8_t nbytes);

int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf,
             const uint8_t nbytes);
#endif