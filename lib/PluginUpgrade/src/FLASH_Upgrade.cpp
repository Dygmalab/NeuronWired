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

#include "FLASH_Upgrade.h"
#include "kaleidoscope/plugin/FocusSerial.h"

namespace kaleidoscope {
namespace plugin {

EventHandlerResult FLASHUpgrade::onFocusEvent(const char *command) {
  if (::Focus.handleHelp(command, PSTR("upgrade.start\nupgrade.end")))
	return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("upgrade."), 8)!=0)
	return EventHandlerResult::OK;


  if (strcmp_P(command + 8, PSTR("start"))==0) {
	serial_pre_activation = true;
  }

  if (strcmp_P(command + 8, PSTR("end"))==0) {
	serial_pre_activation = false;
	activated = false;
	pressed_time = 0;
  }

  return EventHandlerResult::EVENT_CONSUMED;
}
EventHandlerResult FLASHUpgrade::onSetup() {
  return EventHandlerResult::OK;
}
EventHandlerResult FLASHUpgrade::onKeyswitchEvent(Key &mapped_Key, KeyAddr key_addr, uint8_t key_state) {
  if (!serial_pre_activation)
	return EventHandlerResult::OK;

  if (!key_addr.isValid() || (key_state & INJECTED)!=0) {
	return EventHandlerResult::OK;
  }

  if (activated && key_addr.col()==0 && key_addr.row()==0 && keyToggledOff(key_state)) {
	activated = false;
	pressed_time = 0;
	return EventHandlerResult::EVENT_CONSUMED;
  }

  if (key_addr.col()==0 && key_addr.row()==0 && keyToggledOn(key_state)) {
	activated = true;
	pressed_time = Runtime.millisAtCycleStart();;
	return EventHandlerResult::EVENT_CONSUMED;
  }

  return EventHandlerResult::OK;
}
EventHandlerResult FLASHUpgrade::beforeReportingState() {
  if (!serial_pre_activation)
	return EventHandlerResult::OK;
  if (!activated)
	return EventHandlerResult::OK;
  if (Runtime.hasTimeExpired(pressed_time, press_time)) {
	Runtime.rebootBootloader();
	return EventHandlerResult::OK;
  }
  return EventHandlerResult::OK;
}
}
}

kaleidoscope::plugin::FLASHUpgrade FlashUpgrade;
