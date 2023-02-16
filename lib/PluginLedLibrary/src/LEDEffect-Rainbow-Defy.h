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

#pragma once

#include "Kaleidoscope-LEDControl.h"
#include "LedModeSerializable-Rainbow.h"
#include "LedModeCommunication.h"
#include "LedModeSerializable-RainbowWave.h"

namespace kaleidoscope {
namespace plugin {
class LEDRainbowEffectDefy : public Plugin,
                             public LEDModeInterface,
                             public LedModeCommunication {
 public:
  LEDRainbowEffectDefy(void) {
    led_mode.base_settings.brightness = 255;
    led_mode.base_settings.delay_ms   = 40;
  }

  void brightness(uint8_t);

  uint8_t brightness() {
    return led_mode.base_settings.brightness;
  }

  void update_delay(uint8_t);

  uint8_t update_delay(void) {
    return led_mode.base_settings.delay_ms;
  }

  LedModeSerializable_Rainbow &led_mode = ledModeSerializableRainbow;

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(LEDRainbowEffectDefy *parent)
      : parent_(parent) {}

    void update() final;

   protected:
    void onActivate() override;

   private:
    LEDRainbowEffectDefy *parent_;
    uint16_t rainbowLastUpdateKeyScanner = 0;
  };

 private:
};


class LEDRainbowWaveEffectDefy : public Plugin,
                                 public LEDModeInterface,
                                 public LedModeCommunication {
 public:
  LEDRainbowWaveEffectDefy(void) {
    led_mode.base_settings.brightness = 255;
    led_mode.base_settings.delay_ms   = 40;
  }

  void brightness(uint8_t);

  uint8_t brightness() {
    return led_mode.base_settings.brightness;
  }

  void update_delay(uint8_t);

  uint8_t update_delay(void) {
    return led_mode.base_settings.delay_ms;
  }

  LedModeSerializable_RainbowWave &led_mode = ledModeSerializableRainbowWave;

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(LEDRainbowWaveEffectDefy *parent)
      : parent_(parent) {}

    void update() final;
    uint16_t rainbowWaveLastUpdateKeyScanner = 0;

   protected:
    void onActivate() override;

   private:
    LEDRainbowWaveEffectDefy *parent_;
  };

 private:
};
}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::LEDRainbowEffectDefy LEDRainbowEffectDefy;
extern kaleidoscope::plugin::LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
