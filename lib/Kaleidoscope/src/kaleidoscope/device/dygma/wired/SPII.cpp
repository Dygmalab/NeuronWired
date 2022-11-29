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

#include "SPII.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SPII::SPII(bool side) {
}

SPII::~SPII() {
}

void SPII::initSide() {
}

uint8_t SPII::crc_errors() {
  return 0;
}
uint8_t SPII::writeTo(uint8_t *data, size_t length) { return 0; }
uint8_t SPII::readFrom(uint8_t *data, size_t length) {
  return 0;
}
}

#endif