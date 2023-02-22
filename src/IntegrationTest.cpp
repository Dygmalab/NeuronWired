/* -*- mode: c++ -*-
 * kaleidoscope::plugin::EEPROMUpgrade -- Raise EEPROM upgrade helper
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

#include "IntegrationTest.h"

#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-IdleLEDs.h>
#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-FocusSerial.h>

namespace kaleidoscope {
namespace plugin {


EventHandlerResult IntegrationTest::onFocusEvent(const char *command) {
  if (::Focus.handleHelp(command, PSTR("integration.test")))
    return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("integration."), 12) != 0)
    return EventHandlerResult::OK;


  if (strcmp_P(command + 12, PSTR("test")) == 0) {
    activated_ = true;
  }

  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult IntegrationTest::beforeReportingState() {
  if (!activated_) return EventHandlerResult::OK;
  switch (state_) {

  case INIT:
    state_         = LED_MODE;
    led_mode_      = 3;
    start_led_mode = ::LEDControl.get_mode_index();
    break;
  case LED_MODE: {
    if (led_mode_ == 7) {
      ::LEDControl.set_mode(start_led_mode);
      activated_ = false;
    }
    ::LEDControl.set_mode(led_mode_++);
    next_state_ = LED_MODE;
    state_      = WAIT;
    start_time_ = Runtime.millisAtCycleStart();
  } break;
  case KEYS:
    break;
  case WAIT:
    if (Runtime.hasTimeExpired(start_time_, 1000)) {
      state_ = next_state_;
      return EventHandlerResult::OK;
    }
    break;
  }
  if (!activated_)
    return EventHandlerResult::OK;
  return EventHandlerResult::OK;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::IntegrationTest IntegrationTest;
