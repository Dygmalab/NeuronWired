
#pragma once

#include <cstdint>

class LedModeSerializable {
 public:
  struct Settings {
    uint8_t mode;
    uint16_t step;
    uint8_t brightness;
    uint8_t delay_ms;
  } base_settings;

  explicit LedModeSerializable(uint8_t mode) {
    base_settings.mode = mode;
  }
  virtual uint8_t serialize(uint8_t *output) const {
    memcpy(output, (const uint8_t *) &base_settings, sizeof(Settings));
    return sizeof(Settings);
  };

  virtual uint8_t deSerialize(uint8_t *input) {
    memcpy((uint8_t *) &base_settings, input, sizeof(Settings));
    return sizeof(Settings);
  };
};

