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
#include "SPIComunications.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

namespace kaleidoscope::device::dygma::wired {

SPII::SPII(bool side) : side_(side) {
  if (!initSPII_) {
    multicore_launch_core1(SPIComunications::init);
    initSPII_ = true;
  }
}

SPII::~SPII() {
}

uint8_t SPII::crc_errors() {
  return 0;
}
uint8_t SPII::writeTo(uint8_t *data, size_t length) {
  auto tx_messages = side_ ? &SPIComunications::tx_messages_right : &SPIComunications::tx_messages_left;
  if (data[0] >= 0x80) {
    SPIComunications::Message m;
    m.context.cmd = 1;
    m.context.size = length;
    for (uint8_t i = 1; i < length; ++i) {
      m.buf[sizeof(SPIComunications::Context) + i] = data[i];
    }
    m.buf[sizeof(SPIComunications::Context)] = data[0] - 0x80;
    tx_messages->push(m);
  }
  return 0;
}
uint8_t SPII::readFrom(uint8_t *data, size_t length) {
  auto key_data = side_ ? SPIComunications::right_keys : SPIComunications::left_keys;
  auto new_key = side_ ? SPIComunications::new_key_right : SPIComunications::new_key_left;
  data[0] = new_key;
  data[1] = key_data[0];
  data[2] = key_data[1];
  data[3] = key_data[2];
  data[4] = key_data[3];
  data[5] = key_data[4];
  return 6;
}
}

#endif