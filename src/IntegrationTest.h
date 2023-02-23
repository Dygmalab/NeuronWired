/* -*- mode: c++ -*-
 * kaleidoscope::plugin::wiredEEPROM -- Raise EEPROM upgrade helper
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

class IntegrationTest : public Plugin {
 public:
  EventHandlerResult onFocusEvent(const char *command);
  EventHandlerResult beforeReportingState();

 private:
  enum State {
    INIT,
    WAIT,
    LED_MODE,
    KEY_NEXT_LED,
    RELEASE_KEY,
  };
  State state_{INIT};
  State next_state_{WAIT};
  bool activated_{false};
  uint8_t led_mode_{0};
  uint8_t start_led_mode{0};
  uint16_t start_time_{0};
};

}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::IntegrationTest IntegrationTest;
