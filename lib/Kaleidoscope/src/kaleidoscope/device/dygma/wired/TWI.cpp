/* -*- mode: c++ -*-
 * kaleidoscope::device::dygma::Wired -- Kaleidoscope device plugin for Dygma Wired
 * Copyright (C) 2017-2019  Keyboard.io, Inc
 * Copyright (C) 2017-2019  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include "Wire.h"

#include "TWI.h"
#include "kaleidoscope/util/crc16.h"

#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define WIRE_ Wire1

namespace kaleidoscope {
namespace device {
namespace dygma {
namespace wired {

uint8_t TWI::writeTo(uint8_t *data, size_t length) {
  WIRE_.beginTransmission(addr_);

  // calc cksum
  uint16_t crc16 = 0xffff;
  uint8_t *buffer = data;
  for (uint8_t i = 0; i < length; i++) {
    crc16 = _crc_ccitt_update(crc16, *buffer);
    buffer++;
  }

  // make cksum high byte and low byte
  uint8_t crc_bytes[2];
  crc_bytes[0] = crc16 >> 8;
  crc_bytes[1] = crc16;

  if (!WIRE_.write(data, length)) return 1;
  if (!WIRE_.write(crc_bytes, 2)) return 1;
  if (WIRE_.endTransmission(true) != 0) return 1;
  return 0;
}

uint8_t TWI::readFrom(uint8_t* data, size_t length) {
  uint8_t counter = 0;
  uint32_t timeout;
  uint8_t *buffer = data;

  if (!WIRE_.requestFrom(addr_, length + 2, true)) { // + 2 for the cksum
    // in case slave is not responding - return 0 (0 length of received data).
    recovery();
    return 0;
  }
  while (counter < length) {
    *data = WIRE_.read();
    data++;
    counter++;
  }

  uint16_t crc16 = 0xffff;
  uint16_t rx_cksum = (WIRE_.read() << 8) + WIRE_.read();
  for (uint8_t i = 0; i < length; i++) {
    crc16 = _crc_ccitt_update(crc16, *buffer);
    buffer++;
  }

  // check received CRC16
  if (crc16 != rx_cksum) {
    crc_errors_++;
    return 0;
  }

  return length;
}

void TWI::recovery() {
  WIRE_.end();

  //try i2c bus recovery at 100kHz = 5uS high, 5uS low
  pinMode(I2C_SDA_PIN, OUTPUT);//keeping I2C_SDA_PIN high during recovery
  digitalWrite(I2C_SDA_PIN, true);
  pinMode(I2C_SCL_PIN, OUTPUT);
  for (int i = 0; i < 10; i++) { //9nth cycle acts as NACK
    digitalWrite(I2C_SCL_PIN, true);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, false);
    delayMicroseconds(5);
  }

  //a STOP signal (I2C_SDA_PIN from low to high while CLK is high)
  digitalWrite(I2C_SDA_PIN, false);
  delayMicroseconds(5);
  digitalWrite(I2C_SCL_PIN, true);
  delayMicroseconds(2);
  digitalWrite(I2C_SDA_PIN, true);
  delayMicroseconds(2);
  //bus status is now : FREE
  pinMode(I2C_SDA_PIN, INPUT);
  pinMode(I2C_SCL_PIN, INPUT);

  WIRE_.setSDA(I2C_SDA_PIN);
  WIRE_.setSCL(I2C_SCL_PIN);
  WIRE_.begin();
  WIRE_.setClock(clock_khz_ * 1000);
}

void TWI::disable() {
  WIRE_.end();
}

void TWI::init(uint16_t clock_khz) {
  clock_khz_ = clock_khz;
  WIRE_.setSDA(I2C_SDA_PIN);
  WIRE_.setSCL(I2C_SCL_PIN);
  WIRE_.begin();
  WIRE_.setClock(clock_khz * 1000);
}


}
}
}
}

#endif
