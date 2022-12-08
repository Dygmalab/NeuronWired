/* Kaleidoscope-LEDEffect-Rainbow - Rainbow LED effects for Kaleidoscope.
 * Copyright (C) 2017-2018  Keyboard.io, Inc.
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

#include "LEDEffect-Rainbow-Defy.h"
#include "LedCommon.h"

namespace kaleidoscope {
namespace plugin {

void LEDRainbowEffectDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
}
void LEDRainbowEffectDefy::TransientLEDMode::onActivate() {
  Runtime.device().setLedMode(&(parent_->led_mode));
}

void LEDRainbowEffectDefy::brightness(byte brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowEffectDefy::update_delay(byte delay) {
  led_mode.base_settings.delay_ms = delay;
}


// ---------

void LEDRainbowWaveEffectDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
}
void LEDRainbowWaveEffectDefy::TransientLEDMode::onActivate() {
  Runtime.device().setLedMode(&(parent_->led_mode));
}

void LEDRainbowWaveEffectDefy::brightness(byte brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowWaveEffectDefy::update_delay(byte delay) {
  led_mode.base_settings.delay_ms = delay;
}

}
}

kaleidoscope::plugin::LEDRainbowEffectDefy LEDRainbowEffectDefy;
kaleidoscope::plugin::LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
