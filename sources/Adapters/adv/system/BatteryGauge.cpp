/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "BatteryGauge.h"

#include "System/Console/Trace.h"
#include "i2c.h"
#include "platform.h"

#define I2C_TIMEOUT_DELAY_MS 100

// bq27441-G1 I2C address (7-bit address: 0x55)
#define BQ27441_I2C_ADDR 0x55

// Control commands
#define BQ27441_CONTROL_CMD 0x00
#define BQ27441_CONTROL_STATUS 0x00
#define BQ27441_CONTROL_DEVICE_TYPE 0x01
#define BQ27441_CONTROL_FW_VERSION 0x02
#define BQ27441_CONTROL_PREV_MACWRITE 0x07
#define BQ27441_CONTROL_CHEM_ID 0x08
#define BQ27441_CONTROL_BAT_INSERT 0x0C
#define BQ27441_CONTROL_BAT_REMOVE 0x0D
#define BQ27441_CONTROL_SET_CFGUPDATE 0x13
#define BQ27441_CONTROL_SOFT_RESET 0x42
#define BQ27441_CONTROL_EXIT_CFGUPDATE 0x43
#define BQ27441_CONTROL_SEALED 0x20
#define BQ27441_CONTROL_UNSEALED 0x80

// Extended Data Commands
#define BQ27441_BLOCK_DATA_CONTROL 0x61
#define BQ27441_DATA_BLOCK_CLASS 0x3E
#define BQ27441_DATA_BLOCK 0x3F
#define BQ27441_BLOCK_DATA 0x40
#define BQ27441_BLOCK_DATA_CHECKSUM 0x60

// Flags registers
#define BQ27441_FLAGS_REG 0x06
#define BQ27441_FLAGS_CFGUPMODE 0x10
#define BQ27441_FLAGS_ITPOR                                                    \
  0x20 // IT Power-On Reset bit in FLAGS() register - bit 5 of low byte

// Data Classes
#define BQ27441_CLASS_SAFETY 0x00
#define BQ27441_CLASS_CHG_TERMINATION 0x36
#define BQ27441_CLASS_CONFIG 0x40
#define BQ27441_CLASS_DISCHARGE 0x3D
#define BQ27441_CLASS_REGISTERS 0x41
#define BQ27441_CLASS_POWER 0x45
#define BQ27441_CLASS_IT_CFG 0x50
#define BQ27441_CLASS_CURRENT_THRESH 0x47
#define BQ27441_CLASS_STATE 0x52
#define BQ27441_CLASS_CODES 0x58
#define BQ27441_CLASS_CHARGE_CODES 0x5B
#define BQ27441_CLASS_TEMP_MODEL 0x62

// Battery parameters to configure
// 2650mAh for LIP1708 but we'll be conservative for now and only use 2000mAh
#define DESIGN_CAPACITY 2000
#define DESIGN_ENERGY 9805     // Design Capacity * 3.7V
#define TERMINATE_VOLTAGE 3000 // 3.0V terminate voltage
#define TAPER_RATE 233         // (DESIGN_CAPACITY / (0.1 * 115mA))

// The Design Capacity register (0x3C)
// According to the datasheet, Design Capacity is a standard data command (0x3C)
// that can be accessed in SEALED mode
#define BQ27441_DESIGN_CAPACITY 0x3C

// need to use this as HAL_Delay() not available when configureBatteryGauge is
// called during early bootup
void busywait(uint32_t delay) {
  auto start = millis();
  while ((millis() - start) < delay) {
    ;
  }
}

// Battery gauge configuration
bool configureBatteryGauge() {
  HAL_StatusTypeDef status;
  uint8_t txBuf[4] = {0};
  uint8_t rxBuf[2] = {0};

  Trace::Log("BATTERY", "Starting BQ27441 fuel gauge configuration");

  // Check if configuration is needed by reading the current Design Capacity
  Trace::Log("BATTERY", "Checking current Design Capacity to determine if "
                        "configuration is needed");

  txBuf[0] = BQ27441_DESIGN_CAPACITY;

  status = HAL_I2C_Master_Transmit(&I2C, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                   I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("Failed to send Design Capacity read command (status: %d)",
                 status);
    return false;
  }

  status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxBuf, 2,
                                  I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("Failed to read Design Capacity (status: %d)", status);
    return false;
  }

  // Calculate the current Design Capacity (LSB first)
  uint16_t currentDesignCapacity = rxBuf[0] | (rxBuf[1] << 8);
  Trace::Log("BATTERY", "Current Design Capacity: %d mAh (0x%02X 0x%02X)",
             currentDesignCapacity, rxBuf[0], rxBuf[1]);

  // Check if the Design Capacity matches our expected value
  if (currentDesignCapacity == DESIGN_CAPACITY) {
    Trace::Log("BATTERY",
               "Design Capacity already set to expected value (%d mAh)",
               DESIGN_CAPACITY);
    return true;
  }

  Trace::Log("BATTERY",
             "Design Capacity needs to be updated from %d mAh to %d mAh",
             currentDesignCapacity, DESIGN_CAPACITY);
  Trace::Log("BATTERY", "Step 1: Unsealing the fuel gauge");

  // Step 1: Unseal the fuel gauge
  // Send CONTROL_UNSEALED (0x8000) command
  txBuf[0] = BQ27441_CONTROL_CMD;
  txBuf[1] = 0x00;
  txBuf[2] = BQ27441_CONTROL_UNSEALED;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 3,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to unseal the fuel gauge (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "Sent first unseal command");

  // Send CONTROL_UNSEALED (0x8000) command again
  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 3,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to unseal the fuel gauge (second command, status: %d)",
                 status);
    return false;
  }
  Trace::Log("BATTERY",
             "Sent second unseal command, device should now be unsealed");

  busywait(10);

  // Step 2: Enter CONFIG UPDATE mode
  Trace::Log("BATTERY", "Step 2: Entering CONFIG UPDATE mode");
  txBuf[0] = BQ27441_CONTROL_CMD;
  txBuf[1] = BQ27441_CONTROL_SET_CFGUPDATE;
  txBuf[2] = 0x00;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 3,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to send CONFIG UPDATE command (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "Sent CONFIG UPDATE command");

  // Step 3: Verify CONFIG UPDATE mode by checking FLAGS register (bit 4 should
  // be set) This matches step 3 in the datasheet example
  Trace::Log("BATTERY", "Step 3: Verifying CONFIG UPDATE mode");
  busywait(100); // Wait for mode to change

  // Try up to 10 times to confirm CFGUPMODE flag is set
  bool cfgUpdateMode = false;
  for (int i = 0; i < 10; i++) {
    txBuf[0] = BQ27441_FLAGS_REG;
    status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                     HAL_MAX_DELAY);
    if (status != HAL_OK) {
      Trace::Error("Failed to send FLAGS register read command (attempt %d)",
                   i + 1);
      busywait(100);
      continue;
    }

    status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxBuf, 2,
                                    HAL_MAX_DELAY);
    if (status != HAL_OK) {
      Trace::Error("Failed to read FLAGS register (attempt %d)", i + 1);
      busywait(100);
      continue;
    }

    uint16_t flags = rxBuf[0] | (rxBuf[1] << 8);
    Trace::Log("BATTERY", "FLAGS register: 0x%04X (attempt %d)", flags, i + 1);

    if (flags & BQ27441_FLAGS_CFGUPMODE) {
      cfgUpdateMode = true;
      Trace::Log("BATTERY", "CONFIG UPDATE mode flag set successfully");
      break;
    }

    Trace::Log("BATTERY", "CONFIG UPDATE mode flag not set yet, waiting...");
    busywait(100);
  }

  if (!cfgUpdateMode) {
    Trace::Error("Failed to enter CONFIG UPDATE mode after multiple attempts");
    return false;
  }

  Trace::Log("BATTERY", "Entered CONFIG UPDATE mode");

  // Step 3: Enable BlockData control
  Trace::Log("BATTERY", "Step 3: Enabling BlockData control");
  txBuf[0] = BQ27441_BLOCK_DATA_CONTROL;
  txBuf[1] = 0x00;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to enable BlockData control (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "BlockData control enabled");

  // Step 4: Set State subclass (0x52) for Design Capacity
  Trace::Log("BATTERY", "Step 4: Setting State subclass (0x%02X)",
             BQ27441_CLASS_STATE);
  txBuf[0] = BQ27441_DATA_BLOCK_CLASS;
  txBuf[1] = BQ27441_CLASS_STATE;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to set State subclass (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "State subclass set successfully");

  // Step 5: Set block offset to 0x00
  Trace::Log("BATTERY", "Step 5: Setting block offset to 0x00");
  txBuf[0] = BQ27441_DATA_BLOCK;
  txBuf[1] = 0x00;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to set block offset (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "Block offset set successfully");

  // Add a longer delay (100ms) after setting block offset as recommended online
  // This helps ensure the battery gauge has time to prepare data
  busywait(100);

  // Step 6: Read the current checksum
  Trace::Log("BATTERY", "Step 6: Reading current checksum");
  txBuf[0] = BQ27441_BLOCK_DATA_CHECKSUM;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to send checksum read command (status: %d)", status);
    return false;
  }

  busywait(10); // Increased from 1ms to 10ms

  status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxBuf, 1,
                                  HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to read checksum (status: %d)", status);
    return false;
  }

  uint8_t oldChecksum = rxBuf[0];
  Trace::Log("BATTERY", "Current checksum: 0x%02X", oldChecksum);

  // Step 7: Read the current Design Capacity (bytes 10-11)
  // According to datasheet example:
  // Read both Design Capacity bytes starting at 0x4A (offset = 10).
  // Block data starts at 0x40, so to read the data of a specific offset, use
  // address 0x40 + mod(offset, 32). rd 0x4A OLD_DesCap_MSB; rd 0x4B
  // OLD_DesCap_LSB;

  Trace::Log("BATTERY", "Step 7: Reading current Design Capacity");

  // First read MSB (offset 10)
  txBuf[0] = 0x4A; // 0x40 + (10 % 32) = 0x4A

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to send Design Capacity MSB read command (status: %d)",
                 status);
    return false;
  }

  busywait(10);

  status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, &rxBuf[0], 1,
                                  HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to read Design Capacity MSB (status: %d)", status);
    return false;
  }

  // Then read LSB (offset 11)
  txBuf[0] = 0x4B; // 0x40 + (11 % 32) = 0x4B

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to send Design Capacity LSB read command (status: %d)",
                 status);
    return false;
  }

  busywait(10);

  status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, &rxBuf[1], 1,
                                  HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to read Design Capacity LSB (status: %d)", status);
    return false;
  }

  // Note: The datasheet example shows MSB in rxBuf[0] and LSB in rxBuf[1]
  uint16_t oldDesignCapacity = (rxBuf[0] << 8) | rxBuf[1];
  Trace::Log("BATTERY", "Current Design Capacity: %d mAh (0x%02X 0x%02X)",
             oldDesignCapacity, rxBuf[0], rxBuf[1]);

  // Step 8: Write the new Design Capacity
  // According to datasheet example:
  // wr 0x4A New_DesCap_MSB; wr 0x4B New_DesCap_LSB;
  Trace::Log("BATTERY", "Step 8: Writing new Design Capacity (%d mAh)",
             DESIGN_CAPACITY);

  // Write MSB to 0x4A
  // The BQ27441 uses big-endian format for storing integers
  // For example, 1500 decimal = 0x05DC → write 0x05 to offset 10 and 0xDC to
  // offset 11
  txBuf[0] = 0x4A;                          // 0x40 + (10 % 32) = 0x4A
  txBuf[1] = (DESIGN_CAPACITY >> 8) & 0xFF; // MSB (high byte)

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to write Design Capacity MSB (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "Design Capacity MSB written: 0x%02X", txBuf[1]);

  busywait(10);

  // Write LSB to 0x4B
  txBuf[0] = 0x4B;                   // 0x40 + (11 % 32) = 0x4B
  txBuf[1] = DESIGN_CAPACITY & 0xFF; // LSB (low byte)

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to write Design Capacity LSB (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY",
             "New Design Capacity written successfully: %d mAh (0x%02X 0x%02X)",
             DESIGN_CAPACITY, DESIGN_CAPACITY & 0xFF,
             (DESIGN_CAPACITY >> 8) & 0xFF);

  // Step 9: Calculate new checksum
  Trace::Log("BATTERY", "Step 9: Calculating new checksum");

  // Calculate checksum using the approach from the sample code
  // First get the old MSB and LSB bytes
  uint8_t oldMSB = (oldDesignCapacity >> 8) & 0xFF;
  uint8_t oldLSB = oldDesignCapacity & 0xFF;

  // Get the new MSB and LSB bytes
  uint8_t newMSB = (DESIGN_CAPACITY >> 8) & 0xFF;
  uint8_t newLSB = DESIGN_CAPACITY & 0xFF;

  // Calculate intermediate value
  uint8_t temp = (255 - oldChecksum - oldMSB - oldLSB) % 256;

  // Calculate new checksum
  uint8_t newChecksum = 255 - ((temp + newMSB + newLSB) % 256);

  Trace::Log("BATTERY",
             "Checksum calculation: old_checksum=0x%02X, old_MSB=0x%02X, "
             "old_LSB=0x%02X",
             oldChecksum, oldMSB, oldLSB);
  Trace::Log(
      "BATTERY",
      "Checksum calculation: new_MSB=0x%02X, new_LSB=0x%02X, temp=0x%02X",
      newMSB, newLSB, temp);
  Trace::Log("BATTERY", "New calculated checksum: 0x%02X", newChecksum);

  // Step 10: Write the new checksum
  Trace::Log("BATTERY", "Step 10: Writing new checksum (0x%02X)", newChecksum);

  // Add a longer delay before writing the checksum
  busywait(50);

  // Write the checksum directly to register 0x60 (BLOCKDATACHECK)
  txBuf[0] = BQ27441_BLOCK_DATA_CHECKSUM;
  txBuf[1] = newChecksum;

  // Log the exact bytes we're sending
  Trace::Log("BATTERY", "Writing checksum: register=0x%02X, value=0x%02X",
             txBuf[0], txBuf[1]);

  // Try writing checksumwith a longer timeout
  status =
      HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 2, 1000);
  if (status != HAL_OK) {
    Trace::Error("Failed to write checksum (status: %d)", status);
    return false;
  }

  Trace::Log("BATTERY", "Checksum write successful");

  // Add a longer delay after writing the checksum
  busywait(50);

  txBuf[0] = BQ27441_BLOCK_DATA_CHECKSUM;
  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                   HAL_MAX_DELAY);
  if (status == HAL_OK) {
    status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxBuf, 1,
                                    HAL_MAX_DELAY);
    if (status == HAL_OK) {
      Trace::Log("BATTERY",
                 "Verification: Checksum read back as 0x%02X (expected 0x%02X)",
                 rxBuf[0], newChecksum);
      if (rxBuf[0] != newChecksum) {
        Trace::Error("Checksum verification failed - written value doesn't "
                     "match read value");
      }
    }
  }

  // checksum ok
  Trace::Log("BATTERY", "Updated Design Capacity from %d mAh to %d mAh",
             oldDesignCapacity, DESIGN_CAPACITY);

  // Step 11: Exit CONFIG UPDATE mode using SOFT_RESET as in the sample code
  Trace::Log("BATTERY", "Step 11: Exiting CONFIG UPDATE mode using SOFT_RESET");

  // Send SOFT_RESET command
  txBuf[0] = BQ27441_CONTROL_CMD;
  txBuf[1] = BQ27441_CONTROL_SOFT_RESET & 0xFF;        // Low byte
  txBuf[2] = (BQ27441_CONTROL_SOFT_RESET >> 8) & 0xFF; // High byte

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 3,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to send SOFT_RESET command (status: %d)", status);
    return false;
  }

  // Wait for CONFIG UPDATE mode flag to clear
  Trace::Log("BATTERY", "Waiting for CONFIG UPDATE mode to exit");
  busywait(100);
  Trace::Log("BATTERY",
             "Sent exit CONFIG UPDATE command, checking if flag is cleared");

  // Wait for CFGUPMODE flag to clear
  bool exitedCfgMode = false;
  // per the datasheet:
  // "Confirm CFGUPDATE mode by polling Flags() register until bit 4 is set. May
  // take up to 1 second."
  // so we'll poll up to 10 times with 100ms delay between each poll
  for (int i = 0; i < 10; i++) {
    // Read FLAGS register
    txBuf[0] = BQ27441_FLAGS_REG;
    status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 1,
                                     HAL_MAX_DELAY);
    if (status != HAL_OK) {
      Trace::Error("Failed to send FLAGS register read command");
      continue;
    }

    busywait(1);

    status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxBuf, 2,
                                    HAL_MAX_DELAY);
    if (status != HAL_OK) {
      Trace::Error("Failed to read FLAGS register");
      continue;
    }

    if (!(rxBuf[0] & BQ27441_FLAGS_CFGUPMODE)) {
      exitedCfgMode = true;
      Trace::Log("BATTERY",
                 "CONFIG UPDATE mode flag cleared successfully on attempt %d",
                 i + 1);
      break;
    }
    Trace::Log("BATTERY",
               "CONFIG UPDATE mode flag still set (attempt %d), flags: 0x%02X",
               i + 1, rxBuf[0]);

    // 100ms delay to give us a total of 1 second of retries (10 attempts)
    busywait(100);
  }

  if (!exitedCfgMode) {
    Trace::Error("Failed to exit CONFIG UPDATE mode (flag still set)");
    return false;
  }

  // Step 12: Seal the fuel gauge
  Trace::Log("BATTERY", "Step 12: Sealing the fuel gauge");
  txBuf[0] = BQ27441_CONTROL_CMD;
  txBuf[1] = 0x20;
  txBuf[2] = 0x00;

  status = HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txBuf, 3,
                                   HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("Failed to seal the fuel gauge (status: %d)", status);
    return false;
  }
  Trace::Log("BATTERY", "Fuel gauge sealed successfully");

  Trace::Log("BATTERY",
             "BQ27441 fuel gauge configuration completed successfully");
  return true;
}

// Read battery state of charge using I2C from bq27441-G1 fuel gauge
// returns percentage of battery charge
uint8_t getBatterySOC() {

  uint8_t cmdBuf[1] = {BQ27441_CMD_VOLTAGE}; // Command to read voltage
  uint8_t rxData[2] = {0};

  // Read State of Charge (SOC) for percentage
  cmdBuf[0] = BQ27441_CMD_SOC;
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
      &I2C, BQ27441_I2C_ADDR << 1, cmdBuf, 1, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("Failed to transmit SOC command to battery gauge");
    // Return 0 for now if SOC read fails
    return 0;
  }

  busywait(1); // Small delay to ensure command processing

  // Read the SOC data (2 bytes)
  status = HAL_I2C_Master_Receive(&I2C, BQ27441_I2C_ADDR << 1, rxData, 2,
                                  HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("Failed to receive SOC data from battery gauge");
    // for now just return 0 if SOC read fails
    return 0;
  }

  // Combine the two bytes into a 16-bit value (LSB first per datasheet)
  int batterySOC = (rxData[1] << 8) | rxData[0];

  // SOC is returned as percentage (0-100)
  // Trace::Log("BATTERY", "Battery SOC: %d%%", batterySOC);

  // Return the SOC percentage
  return batterySOC;
}

uint32_t getBatteryVoltage() {
  uint8_t cmdBuf[1] = {BQ27441_CMD_VOLTAGE}; // Command to read voltage
  uint8_t rxData[2] = {0};
  int batteryVoltage = 0;

  // Send the voltage command to the fuel gauge
  // Note: The 7-bit address needs to be shifted left by 1 bit for the HAL API
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
      &hi2c4, BQ27441_I2C_ADDR << 1, cmdBuf, 1, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("Failed to transmit voltage command to battery gauge");
    return -1;
  }

  // According to datasheet, need to wait briefly between command and read
  busywait(1);

  // Read the voltage data (2 bytes)
  status = HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxData, 2,
                                  HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("Failed to receive voltage data from battery gauge");
    return -1;
  }

  // Combine the two bytes into a 16-bit value (LSB first per datasheet)
  batteryVoltage = (rxData[1] << 8) | rxData[0];

  // The voltage is returned in mV
  return batteryVoltage;
}

// returns temperature in degrees Celsius
int32_t getBatteryTemperature() {
  // Read the temperature from the BQ27441 battery gauge
  // Temperature is stored in the TEMP register (0x02)
  // The value is returned in units of 0.1K

  HAL_StatusTypeDef status;
  uint8_t txData[1] = {BQ27441_CMD_TEMP};
  uint8_t rxData[2] = {0};
  int32_t temperature = 0;

  // Send the TEMP register command
  status =
      HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txData, 1, 1000);
  if (status != HAL_OK) {
    Trace::Error(
        "Failed to send temperature read command to battery gauge (status: %d)",
        status);
    return -273; // Return absolute zero as an error indicator
  }

  // Read the temperature data
  status =
      HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxData, 2, 1000);
  if (status != HAL_OK) {
    Trace::Error(
        "Failed to receive temperature data from battery gauge (status: %d)",
        status);
    return -273; // Return absolute zero as an error indicator
  }

  // Combine the two bytes into a 16-bit value (LSB first per datasheet)
  uint16_t rawTemp = (rxData[1] << 8) | rxData[0];

  // Convert from 0.1K to degrees Celsius
  // K = C + 273.15, so C = K - 273.15
  // The value is in 0.1K, so we need to subtract 2731.5
  // Since we're returning an integer, we'll divide by 10 at the end
  temperature = ((int32_t)rawTemp - 2731) / 10;
  return temperature;
}

int16_t getBatteryCurrent() {
  // Because the DSG doesn't seem to give the correct charging state, we use the
  // current measurement to determine charging status Positive current indicates
  // charging, negative current indicates discharging

  HAL_StatusTypeDef status;
  uint8_t txData[1] = {BQ27441_CMD_CURRENT}; // Current register address
  uint8_t rxData[2] = {0};                   // Current is a 16-bit register

  // Send the current register command
  status =
      HAL_I2C_Master_Transmit(&hi2c4, BQ27441_I2C_ADDR << 1, txData, 1, 1000);
  if (status != HAL_OK) {
    Trace::Error(
        "Failed to send current read command to battery gauge (status: %d)",
        status);
    return CURRENT_READ_ERROR;
  }

  // Read the current data
  status =
      HAL_I2C_Master_Receive(&hi2c4, BQ27441_I2C_ADDR << 1, rxData, 2, 1000);
  if (status != HAL_OK) {
    Trace::Error(
        "Failed to receive current data from battery gauge (status: %d)",
        status);
    return CURRENT_READ_ERROR;
  }

  // Combine the two bytes into a 16-bit signed value (LSB first per datasheet)
  int16_t current = (rxData[1] << 8) | rxData[0];
  return current;
}
