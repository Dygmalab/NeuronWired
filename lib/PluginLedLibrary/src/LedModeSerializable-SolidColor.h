#pragma once

#include "LedModeSerializable.h"
#include "cstdio"

#ifdef KEYSCANNER
#include "LEDManagement.hpp"
#endif

class LedModeSerializable_SolidColor : public LedModeSerializable {
 public:
  explicit LedModeSerializable_SolidColor(uint32_t id)
    : LedModeSerializable(id) {
  }
  uint8_t serialize(uint8_t *output) const override {
    uint8_t index     = LedModeSerializable::serialize(output);
    output[index]     = r_;
    output[index + 1] = g_;
    output[index + 2] = b_;
    output[index + 3] = w_;
    return index;
  }

  uint8_t deSerialize(const uint8_t *input) override {
    uint8_t index          = LedModeSerializable::deSerialize(input);
    r_                     = input[index];
    g_                     = input[index + 1];
    b_                     = input[index + 2];
    w_                     = input[index + 3];
    base_settings.delay_ms = 100;
    return index;
  }
#ifdef NEURON_WIRED
  void update() override {
    kaleidoscope::Runtime.device().ledDriver().setCrgbNeuron({r_, g_, b_, w_});
  }
#endif

#ifdef KEYSCANNER
  void update() override {
    LEDManagement::set_all_leds({r_, g_, b_, w_});
    LEDManagement::set_updated(true);
  }
#endif
  uint8_t r_, g_, b_, w_;

 private:
  uint16_t rainbowHue        = 0;
  uint16_t rainbowSaturation = 255;
  uint8_t rainbowLastUpdate  = 0;
};

static LedModeSerializable_SolidColor
  ledModeSerializableSolidColor{CRC32_STR("LedModeSerializable_SolidColor")};