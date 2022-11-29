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

#pragma once

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include <Arduino.h>
#include <hardware/dma.h>
#include "hardware/spi.h"

namespace kaleidoscope::device::dygma::wired {
static bool initSPII_;
class SPII {
 public:
  bool side_;
  explicit SPII(bool side);

  void initSPI();

  uint8_t writeTo(uint8_t *data, size_t length);

  uint8_t readFrom(uint8_t *data, size_t length);

  void disable();

  uint8_t crc_errors();
  virtual ~SPII();

};

}
#endif
