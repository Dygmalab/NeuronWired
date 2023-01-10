/* -*- mode: c++ -*-
 * kaleidoscope::plugin::LEDCapsLockLight -- Highlight CapsLock when active
 * Copyright (C) 2020  Dygma Lab S.L.
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

#include <Kaleidoscope.h>

namespace kaleidoscope {
namespace plugin {

class FLASHUpgrade: public Plugin {
 public:
  // Kaleidoscope Focus library functions
  EventHandlerResult onFocusEvent(const char *command);
  // On Setup handler function
  EventHandlerResult onSetup();

  EventHandlerResult beforeReportingState();
  EventHandlerResult onKeyswitchEvent(Key &mapped_Key, KeyAddr key_addr, uint8_t key_state);

 private:
  bool activated=false;
  uint16_t press_time{1500};
  uint16_t pressed_time;
  bool serial_pre_activation=false;
};

}
}

extern kaleidoscope::plugin::FLASHUpgrade FlashUpgrade;
