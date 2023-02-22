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

namespace kaleidoscope {
namespace plugin {

void LEDRainbowEffectDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
  if (!kaleidoscope::Runtime.hasTimeExpired(rainbowLastUpdateKeyScanner, 1000)) {
    return;
  }
  rainbowLastUpdateKeyScanner += 1000;
  sendLedMode(parent_->led_mode);
}

void LEDRainbowEffectDefy::TransientLEDMode::onActivate() {
  sendLedMode(parent_->led_mode);
}

void LEDRainbowEffectDefy::brightness(uint8_t brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowEffectDefy::update_delay(uint8_t delay) {
  led_mode.base_settings.delay_ms = delay;
}


// ---------

void LEDRainbowWaveEffectDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
  if (!kaleidoscope::Runtime.hasTimeExpired(rainbowWaveLastUpdateKeyScanner, 1000)) {
    return;
  }
  rainbowWaveLastUpdateKeyScanner += 1000;
  sendLedMode(parent_->led_mode);
}

void LEDRainbowWaveEffectDefy::TransientLEDMode::onActivate() {
  sendLedMode(parent_->led_mode);
}

void LEDRainbowWaveEffectDefy::brightness(uint8_t brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowWaveEffectDefy::update_delay(uint8_t delay) {
  led_mode.base_settings.delay_ms = delay;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::LEDRainbowEffectDefy LEDRainbowEffectDefy;
kaleidoscope::plugin::LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
