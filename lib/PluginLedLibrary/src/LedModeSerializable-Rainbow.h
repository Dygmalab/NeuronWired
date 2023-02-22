#pragma once

#include "LedModeSerializable.h"
#include "cstdio"

#ifdef KEYSCANNER
#include <LEDManagement.hpp>
#endif

class LedModeSerializable_Rainbow : public LedModeSerializable {
 public:
  explicit LedModeSerializable_Rainbow(uint32_t id)
    : LedModeSerializable(id) {
  }

#ifdef NEURON_WIRED
  void update() override {
    if (!kaleidoscope::Runtime.has_leds)
      return;

    if (!kaleidoscope::Runtime.hasTimeExpired(rainbowLastUpdate,
                                              base_settings.delay_ms)) {
      return;
    } else {
      rainbowLastUpdate += base_settings.delay_ms;
    }
    rainbowHue   = base_settings.step;
    cRGB rainbow = hsvToRgb(rainbowHue, rainbowSaturation, base_settings.brightness);
    rainbow.w    = 0;
    rainbowHue += 1;
    if (rainbowHue >= 255) {
      rainbowHue -= 255;
    }
    base_settings.step = rainbowHue;
    kaleidoscope::Runtime.device().ledDriver().setCrgbNeuron(rainbow);
  }
#endif

#ifdef KEYSCANNER
  void update() override {
    rainbowHue   = base_settings.step;
    RGBW rainbow = LEDManagement::HSVtoRGB(rainbowHue, rainbowSaturation, base_settings.brightness);
    rainbow.w    = 0;
    rainbowHue += 1;
    if (rainbowHue >= 255) {
      rainbowHue -= 255;
    }
    base_settings.step = rainbowHue;
    LEDManagement::set_all_leds(rainbow);
    LEDManagement::set_updated(true);
  }
#endif

 private:
  uint16_t rainbowHue        = 0;
  uint16_t rainbowSaturation = 255;
  uint8_t rainbowLastUpdate  = 0;
};

static LedModeSerializable_Rainbow ledModeSerializableRainbow{CRC32_STR("LedModeSerializable_Rainbow")};