/* -*- mode: c++ -*-
 * kaleidoscope::plugin::wiredFirmwareVersion -- Tell the firmware version via Focus
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

#include "Kaleidoscope.h"
#include "Kaleidoscope-FocusSerial.h"
#include "DefyFirmwareVersion.h"


#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION WIRED_FIRMWARE_VERSION
#endif

namespace kaleidoscope {
namespace plugin {

EventHandlerResult FirmwareVersion::onFocusEvent(const char *command) {
  const char *cmd = PSTR("version");
  if (::Focus.handleHelp(command, cmd))
    return EventHandlerResult::OK;

  if (strcmp_P(command, cmd) != 0)
    return EventHandlerResult::OK;

  ::Focus.sendRaw(F(FIRMWARE_VERSION));

  return EventHandlerResult::EVENT_CONSUMED;
}


}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::FirmwareVersion FirmwareVersion;
