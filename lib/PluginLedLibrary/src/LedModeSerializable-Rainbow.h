#pragma once

#include "LedModeSerializable.h"
#include "LedCommon.h"
#include <cstdio>

class LedModeSerializable_Rainbow : public LedModeSerializable {
public:
  explicit LedModeSerializable_Rainbow(uint32_t id) : LedModeSerializable(id) {
  }

#ifdef NEURON
  void update() override {
	if (!kaleidoscope::Runtime.has_leds)
	  return;

	if (!kaleidoscope::Runtime.hasTimeExpired(rainbowLastUpdate,
											  base_settings.delay_ms)) {
	  return;
	} else {
	  rainbowLastUpdate += base_settings.delay_ms;
	}
	cRGB rainbow = HSItoRGBW(rainbowHue, rainbowSaturation, base_settings.brightness);

	rainbowHue += 1;
	if (rainbowHue==360) {
	  rainbowHue = 0;
	}
	base_settings.step = rainbowHue;
	kaleidoscope::Runtime.device().setCrgbNeuron(rainbow);
  }
#endif

#ifdef KEYSCANNER
  void update() override {

	cRGB rainbow = HSItoRGBW(rainbowHue, rainbowSaturation, base_settings.brightness);

	rainbowHue += 1;
	if (rainbowHue==360) {
	  rainbowHue = 0;
	}
	base_settings.step = rainbowHue;
	kaleidoscope::Runtime.device().setCrgbNeuron(rainbow);
  }
#endif
private:
  uint16_t rainbowHue = 0;
  uint16_t rainbowSaturation = 255;
  uint8_t rainbowLastUpdate = 0;
};

static LedModeSerializable_Rainbow ledModeSerializableRainbow{COMPILE_TIME_CRC32_STR("LedModeSerializable_Rainbow")};