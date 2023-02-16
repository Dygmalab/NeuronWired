/* Kaleidoscope-LEDEffect-SolidColor - Solid color LED effects for Kaleidoscope.
 * Copyright (C) 2017  Keyboard.io, Inc.
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

#include "LEDEffect-SolidColor-Defy.h"

namespace kaleidoscope {
namespace plugin {

void LEDSolidColorDefy::TransientLEDMode::onActivate(void) {
  parent_->led_mode.r_ = parent_->r_;
  parent_->led_mode.g_ = parent_->g_;
  parent_->led_mode.b_ = parent_->b_;
  parent_->led_mode.w_ = parent_->w_;
  sendLedMode(parent_->led_mode);
}

void LEDSolidColorDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
}
}  // namespace plugin
}  // namespace kaleidoscope
