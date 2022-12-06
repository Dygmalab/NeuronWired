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
#include "LedModeSerializable.h"

namespace kaleidoscope {
namespace plugin {
class LEDRainbowEffectDefy : public Plugin,
                             public LEDModeInterface {
 public:
  LEDRainbowEffectDefy(void) {
    led_mode.base_settings.brightness=50;
    led_mode.base_settings.delay_ms=40;
  }

  void brightness(uint8_t);

  uint8_t brightness() {
    return led_mode.base_settings.brightness;
  }

  void update_delay(uint8_t);
  uint8_t update_delay(void) {
    return led_mode.base_settings.delay_ms;
  }

  LedModeSerializable led_mode{0x00};

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

    uint16_t rainbow_hue = 0;   //  stores 0 to 614

    uint8_t rainbow_steps = 1;  //  number of hues we skip in a 360 range per update
    uint8_t rainbow_last_update = 0;

    uint8_t rainbow_saturation = 255;
  };

 private:
};

class LEDRainbowWaveEffectDefy : public Plugin, public LEDModeInterface {
 public:
  LEDRainbowWaveEffectDefy(void) {}

  void brightness(uint8_t);
  uint8_t brightness(void) {
    return rainbow_value;
  }
  void update_delay(uint8_t);
  uint8_t update_delay(void) {
    return rainbow_update_delay;
  }

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:

    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(const LEDRainbowWaveEffectDefy *parent)
        : parent_(parent) {}

    void update() final;

   protected:
    void onActivate() override;

   private:

    const LEDRainbowWaveEffectDefy *parent_;

    uint16_t rainbow_hue = 0;  //  stores 0 to 614

    uint8_t rainbow_wave_steps = 1;  //  number of hues we skip in a 360 range per update
    uint8_t rainbow_last_update = 0;

    uint8_t rainbow_saturation = 255;
  };

  uint8_t rainbow_update_delay = 40; // delay between updates (ms)
  uint8_t rainbow_value = 50;
};
}
}

extern kaleidoscope::plugin::LEDRainbowEffectDefy LEDRainbowEffectDefy;
extern kaleidoscope::plugin::LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
