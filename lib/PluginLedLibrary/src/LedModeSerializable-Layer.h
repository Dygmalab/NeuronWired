#pragma once

#include "LedModeSerializable.h"
#include "cstdio"

class LedModeSerializable_Layer : public LedModeSerializable {
 public:
  explicit LedModeSerializable_Layer(uint32_t id)
    : LedModeSerializable(id) {
  }
  uint8_t serialize(uint8_t *output) const override {
    uint8_t index = LedModeSerializable::serialize(output);
    output[index] = layer;
    return index;
  }

  uint8_t deSerialize(const uint8_t *input) override {
    uint8_t index = LedModeSerializable::deSerialize(input);
    layer         = input[index];
    return index;
  }
#ifdef NEURON_WIRED
  void update() override {
  }
#endif

#ifdef KEYSCANNER
  void update() override {
    LEDManagement::Layer actualLayer{};
    if (this->layer < LEDManagement::layers.size()) {
      actualLayer = LEDManagement::layers.at(this->layer);
    }
    for (uint8_t i = 0; i < NUMBER_OF_LEDS; i++) {
      RGBW &color = LEDManagement::palette[actualLayer.leds[i]];
      LEDManagement::set_led_at(color, i);
    }
    LEDManagement::set_updated(true);
  }
#endif
  uint8_t layer;

 private:
};

static LedModeSerializable_Layer ledModeSerializableLayer{CRC32_STR("LedModeSerializable_Layer")};