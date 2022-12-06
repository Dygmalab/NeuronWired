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
  if (!Runtime.has_leds)
	return;

  if (!Runtime.hasTimeExpired(rainbow_last_update,
							  parent_->led_mode.base_settings.delay_ms)) {
	return;
  } else {
	rainbow_last_update += parent_->led_mode.base_settings.delay_ms;
  }

  cRGB rainbow = HSItoRGBW(rainbow_hue, rainbow_saturation, parent_->led_mode.base_settings.brightness);

  rainbow_hue += rainbow_steps;
  if (rainbow_hue >= 255) {
	rainbow_hue -= 255;
  }
  ::LEDControl.set_all_leds_to(rainbow);
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
  if (!Runtime.has_leds)
	return;

  if (!Runtime.hasTimeExpired(rainbow_last_update,
							  parent_->rainbow_update_delay)) {
	return;
  } else {
	rainbow_last_update += parent_->rainbow_update_delay;
  }

  for (auto led_index : Runtime.device().LEDs().all()) {
	uint16_t led_hue = rainbow_hue + 16*(led_index.offset()/4);
	// We want led_hue to be capped at 255, but we do not want to clip it to
	// that, because that does not result in a nice animation. Instead, when it
	// is higher than 255, we simply substract 255, and repeat that until we're
	// within cap. This lays out the rainbow in a kind of wave.
	while (led_hue >= 255) {
	  led_hue -= 255;
	}

	cRGB rainbow = hsvToRgb(led_hue, rainbow_saturation, parent_->rainbow_value);
	::LEDControl.setCrgbAt(led_index.offset(), rainbow);
  }
  rainbow_hue += rainbow_wave_steps;
  if (rainbow_hue >= 255) {
	rainbow_hue -= 255;
  }
}

void LEDRainbowWaveEffectDefy::TransientLEDMode::onActivate(void) {
  Serial.printf("Wave Effect activeteee\n");
}

void LEDRainbowWaveEffectDefy::brightness(byte brightness) {
  rainbow_value = brightness;
}

void LEDRainbowWaveEffectDefy::update_delay(byte delay) {
  rainbow_update_delay = delay;
}

}
}

kaleidoscope::plugin::LEDRainbowEffectDefy LEDRainbowEffectDefy;
kaleidoscope::plugin::LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
