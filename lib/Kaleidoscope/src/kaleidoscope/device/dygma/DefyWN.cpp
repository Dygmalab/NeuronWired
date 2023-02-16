/* -*- mode: c++ -*-
 * kaleidoscope::device::dygma::Wired -- Kaleidoscope device plugin for Dygma Wired
 * Copyright (C) 2017-2020  Keyboard.io, Inc
 * Copyright (C) 2017-2020  Dygma Lab S.L.
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

#ifdef ARDUINO_RASPBERRY_PI_PICO

#include "kaleidoscope/Runtime.h"
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Wire.h>
#include "pico/unique_id.h"

#include "kaleidoscope/util/crc16.h"
#include "kaleidoscope/driver/color/GammaCorrection.h"
#include "kaleidoscope/driver/keyscanner/Base_Impl.h"
#include "DefyWN.h"
#include "Colormap-Defy.h"
#include "Side_communications_protocol.h"
#include "kaleidoscope/device/dygma/defyWN/Hand.h"

#define I2C_SDA_PIN         26  // SWe 20220719: I2C1 data out-/in-put, MASTER role
#define I2C_SCL_PIN         27  // SWe 20220719: I2C1 clock output, MASTER role
#define WIRE_               Wire1
#define I2C_CLOCK_KHZ       100
#define I2C_FLASH_CLOCK_KHZ 100  // flashing doesn't work reliably at higher clock speeds
//#define SIDE_POWER 1  // side power switch pa10; SWe 20220719: old, used in Neuron
#define SIDE_nRESET_1 22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2 10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset
#define nPWR_OK       1   // SWe 20220719: Power nOK IN-PULLUP from the 3.3V LDO, open drain, needs internal pull-up. NOTE: this is not implemented in the Development Board, only in the real WIRED Neuron2.
// SWe 20220719: LED pins
#define RGBW_LED_RED   6  // SWe 20220719: RED RGBW led OUT, PWM3 A can be used to control its intensity
#define RGBW_LED_GREEN 0  // SWe 20220719: GREEN RGBW led OUT, PWM0 A can be used to control its intensity
#define RGBW_LED_BLUE  2  // SWe 20220719: BLUE RGBW led OUT, PWM1 A can be used to control its intensity
#define RGBW_LED_WHITE 4  // SWe 20220719: WHITE RGBW led OUT, PWM2 A can be used to control its intensity
// SWe 20220719: analog pins
#define USB_CC1 28  // SWe 20220719: USB CC1 pin, can be used to check how much power the host does support by checking its analog value
#define USB_CC2 29  // SWe 20220719: USB CC2 pin, can be used to check how much power the host does support by checking its analog value
// SWe 20220719: ADC Vref input, tied to 3.3V with resistor and capacitor for filtering and buffering
// SWe 20220719: optional pins

namespace kaleidoscope {
namespace device {
namespace dygma {

class Hands {
 public:
  inline static defyWN::Hand leftHand{Side_communications_protocol::Devices::KEYSCANNER_DEFY_LEFT};
  inline static defyWN::Hand rightHand{Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT};

  static void setup();

  static void setKeyscanInterval(uint16_t interval);
  static uint16_t getKeyscanInterval() { return settings_.keyscan_interval; }

  static void setLedBrightnessCorrection(uint8_t brightness);
  static uint8_t getLedBrightnessCorrection() {
    return settings_.led_brightness_correction;
  }

 private:
  struct Settings {
    Settings() {}
    uint16_t keyscan_interval         = 15;
    uint8_t led_brightness_correction = 255;
  };
  inline static Settings settings_{};
  inline static uint16_t settings_base_;
};


void Hands::setup() {
  settings_base_ = ::EEPROMSettings.requestSlice(sizeof(Settings));
  //TODO: Fix communication of keyScanner interval and led brightness and save in flash
}

void Hands::setKeyscanInterval(uint16_t interval) {
  //TODO: set keyScann in flash and send message
}

void Hands::setLedBrightnessCorrection(uint8_t brightness) {
  //TODO: set brightness in flash and send message
}


/********* LED Driver *********/


void LedDriverWN::setBrightness(uint8_t brightness) {
  Hands::setLedBrightnessCorrection(brightness);
}

uint8_t LedDriverWN::getBrightness() {
  return Hands::getLedBrightnessCorrection();
}

void LedDriverWN::syncLeds() {
  if (isLEDChangedNeuron) {
    updateNeuronLED();
    isLEDChangedNeuron = false;
  }
}

void LedDriverWN::updateNeuronLED() {
  static constexpr struct {
    uint8_t r, g, b, w;
  } pins                = {RGBW_LED_RED, RGBW_LED_GREEN, RGBW_LED_BLUE, RGBW_LED_WHITE};
  auto constexpr gamma8 = kaleidoscope::driver::color::gamma_correction;

  // invert as these are common anode, and make sure we reach 65535 to be able
  // to turn fully off.
  analogWrite(pins.r, (pgm_read_byte(&gamma8[neuronLED.r])) << 8);
  analogWrite(pins.g, (pgm_read_byte(&gamma8[neuronLED.g])) << 8);
  analogWrite(pins.b, (pgm_read_byte(&gamma8[neuronLED.b])) << 8);
  analogWrite(pins.w, (pgm_read_byte(&gamma8[neuronLED.w])) << 8);
}

void LedDriverWN::setCrgbAt(uint8_t i, cRGB crgb) {
  // prevent reading off the end of the led_map array
  if (i >= LedDriverProps::led_count)
    return;

  // neuron LED
  if (i == LedDriverProps::led_count - 2) {
    isLEDChangedNeuron |= !(neuronLED.r == crgb.r && neuronLED.g == crgb.g && neuronLED.b == crgb.b && neuronLED.w == crgb.w);
    neuronLED = crgb;
    return;
  }

  // get the SLED index
  uint8_t sled_num = led_map[i];
  if (sled_num < LEDS_PER_HAND) {
    cRGB oldColor                           = Hands::leftHand.led_data.leds[sled_num];
    Hands::leftHand.led_data.leds[sled_num] = crgb;
    isLEDChangedLeft[uint8_t(sled_num / 8)] |=
      !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b && oldColor.w == crgb.w);
  } else if (sled_num < 2 * LEDS_PER_HAND) {
    cRGB oldColor =
      Hands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND];
    Hands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND] = crgb;
    isLEDChangedRight[uint8_t((sled_num - LEDS_PER_HAND) / 8)] |=
      !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b && oldColor.w == crgb.w);
  } else {
    // TODO(anyone):
    // how do we want to handle debugging assertions about crazy user
    // code that would overwrite other memory?
  }
}

void LedDriverWN::setCrgbNeuron(cRGB crgb) {
  isLEDChangedNeuron |= !(neuronLED.r == crgb.r && neuronLED.g == crgb.g && neuronLED.b == crgb.b && neuronLED.w == crgb.w);
  neuronLED = crgb;
}

cRGB LedDriverWN::getCrgbAt(uint8_t i) {
  if (i >= LedDriverProps::led_count)
    return {0, 0, 0};

  uint8_t sled_num = led_map[i];
  if (sled_num < LEDS_PER_HAND) {
    return Hands::leftHand.led_data.leds[sled_num];
  } else if (sled_num < 2 * LEDS_PER_HAND) {
    return Hands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND];
  } else {
    return {0, 0, 0, 0};
  }
}

void LedDriverWN::setup() {
  analogWriteResolution(16);
}

/********* Key scanner *********/

void KeyScannerWN::readMatrix() {
  Hands::leftHand.previous_key_data  = Hands::leftHand.key_data_;
  Hands::rightHand.previous_key_data = Hands::rightHand.key_data_;
}

void KeyScannerWN::actOnMatrixScan() {
  for (byte row = 0; row < Props_::matrix_rows; row++) {
    for (byte col = 0; col < Props_::left_columns; col++) {
      uint8_t keynum = (row * Props_::left_columns) + col;
      uint8_t keyState;

      // left
      keyState = (bitRead(Hands::leftHand.previous_key_data.all, keynum) << 0) |
                 (bitRead(Hands::leftHand.key_data_.all, keynum) << 1);
      if (keyState)
        ThisType::handleKeyswitchEvent(Key_NoKey, KeyAddr(row, col), keyState);

      // right
      keyState = (bitRead(Hands::rightHand.previous_key_data.all, keynum) << 0) |
                 (bitRead(Hands::rightHand.key_data_.all, keynum) << 1);
      if (keyState)
        ThisType::handleKeyswitchEvent(
          Key_NoKey, KeyAddr(row, (Props_::matrix_columns - 1) - col), keyState);
    }
  }
}

void KeyScannerWN::scanMatrix() {
  readMatrix();
  actOnMatrixScan();
}

void KeyScannerWN::maskKey(KeyAddr key_addr) {
  if (!key_addr.isValid())
    return;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    Hands::rightHand.keyMask.rows[row] |=
      1 << (Props_::right_columns - (col - Props_::left_columns));
  } else {
    Hands::leftHand.keyMask.rows[row] |= 1 << (Props_::right_columns - col);
  }
}

void KeyScannerWN::unMaskKey(KeyAddr key_addr) {
  if (!key_addr.isValid())
    return;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    Hands::rightHand.keyMask.rows[row] &=
      ~(1 << (Props_::right_columns - (col - Props_::left_columns)));
  } else {
    Hands::leftHand.keyMask.rows[row] &= ~(1 << (Props_::right_columns - col));
  }
}

bool KeyScannerWN::isKeyMasked(KeyAddr key_addr) {
  if (!key_addr.isValid())
    return false;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= 8) {
    return Hands::rightHand.keyMask.rows[row] & (1 << (7 - (col - 8)));
  } else {
    return Hands::leftHand.keyMask.rows[row] & (1 << (7 - col));
  }
}

void KeyScannerWN::maskHeldKeys() {
  memcpy(Hands::leftHand.keyMask.rows, Hands::leftHand.key_data_.rows, sizeof(Hands::leftHand.keyMask));
  memcpy(Hands::rightHand.keyMask.rows, Hands::rightHand.key_data_.rows, sizeof(Hands::rightHand.keyMask));
}

bool KeyScannerWN::isKeyswitchPressed(KeyAddr key_addr) {
  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    return (bitRead(Hands::rightHand.key_data_.rows[row],
                    (Props_::matrix_columns - 1) - col) != 0);
  } else {
    return (bitRead(Hands::leftHand.key_data_.rows[row], col) != 0);
  }
}

bool KeyScannerWN::wasKeyswitchPressed(KeyAddr key_addr) {
  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    return (bitRead(Hands::rightHand.previous_key_data.rows[row],
                    (Props_::matrix_columns - 1) - col) != 0);
  } else {
    return (bitRead(Hands::leftHand.previous_key_data.rows[row], col) != 0);
  }
}

uint8_t KeyScannerWN::pressedKeyswitchCount() {
  return __builtin_popcountll(Hands::leftHand.key_data_.all) +
         __builtin_popcountll(Hands::rightHand.key_data_.all);
}

uint8_t KeyScannerWN::previousPressedKeyswitchCount() {
  return __builtin_popcountll(Hands::leftHand.previous_key_data.all) +
         __builtin_popcountll(Hands::rightHand.previous_key_data.all);
}

void KeyScannerWN::setKeyscanInterval(uint8_t interval) {
  //  Hands::leftHand.setKeyscanInterval(interval);
  //  Hands::rightHand.setKeyscanInterval(interval);
}

void KeyScannerWN::setup() {
  static constexpr uint8_t keyscanner_pins[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42};
  for (int i = 0; i < sizeof(keyscanner_pins); i++) {
    // pinMode(keyscanner_pins[i], OUTPUT);
    // digitalWrite(keyscanner_pins[i], LOW);
  }
}

void KeyScannerWN::reset() {
  Hands::leftHand.key_data_.all  = 0;
  Hands::rightHand.key_data_.all = 0;
  Runtime.hid().keyboard().releaseAllKeys();
  Runtime.hid().keyboard().sendReport();
}

/********* Hardware plugin *********/
void DefyWN::setup() {
  Hands::setup();
  KeyScannerWN::setup();
  LedDriverWN::setup();
}

void DefyWN::side::prepareForFlash() {
  WIRE_.end();
  // also turn off i2c pins to stop attiny from getting enough current through
  // i2c to stay on
  pinMode(I2C_SDA_PIN, OUTPUT);
  pinMode(I2C_SCL_PIN, OUTPUT);
  digitalWrite(I2C_SDA_PIN, false);
  digitalWrite(I2C_SCL_PIN, false);
  // wipe key states, to prevent accidental key repeats
  KeyScanner::reset();

  WIRE_.setSDA(I2C_SDA_PIN);
  WIRE_.setSCL(I2C_SCL_PIN);
  WIRE_.begin();
  WIRE_.setClock(I2C_FLASH_CLOCK_KHZ * 1000);
}

uint8_t DefyWN::side::getPowerRight() {
  return digitalRead(SIDE_nRESET_1);
}
void DefyWN::side::setPowerRight(bool power) {
  digitalWrite(SIDE_nRESET_1, power ? HIGH : LOW);
}

uint8_t DefyWN::side::getPowerLeft() {
  return digitalRead(SIDE_nRESET_2);
}
void DefyWN::side::setPowerLeft(bool power) {
  digitalWrite(SIDE_nRESET_2, power ? HIGH : LOW);
}
void DefyWN::side::resetRight() {
  digitalWrite(SIDE_nRESET_1, LOW);
  sleep_ms(10);
  digitalWrite(SIDE_nRESET_1, HIGH);
  sleep_ms(50);  //For bootloader
}

void DefyWN::side::resetLeft() {
  digitalWrite(SIDE_nRESET_2, LOW);
  sleep_ms(10);
  digitalWrite(SIDE_nRESET_2, HIGH);
  sleep_ms(50);  //For bootloader
}

std::string DefyWN::getChipID() {
  char buf[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
  pico_get_unique_board_id_string(buf, sizeof(buf));
  return {buf};
}
void DefyWN::sendPacket(Packet &p) {
  //TODO: Check here if is online
  spi_1.sendPacket(p);
  spi_0.sendPacket(p);
}

}  // namespace dygma
}  // namespace device
}  // namespace kaleidoscope

#endif
