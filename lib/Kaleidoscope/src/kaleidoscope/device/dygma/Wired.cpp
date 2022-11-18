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
#include <Kaleidoscope-LEDControl.h>
#include <KeyboardioHID.h>
#include <Wire.h>
#include "pico/unique_id.h"

#include "kaleidoscope/util/crc16.h"
#include "kaleidoscope/driver/color/GammaCorrection.h"
#include "kaleidoscope/driver/keyscanner/Base_Impl.h"

#define I2C_SDA_PIN 26  // SWe 20220719: I2C1 data out-/in-put, MASTER role
#define I2C_SCL_PIN 27  // SWe 20220719: I2C1 clock output, MASTER role
#define WIRE_ Wire1
#define I2C_CLOCK_KHZ 200
#define I2C_FLASH_CLOCK_KHZ 200  // flashing doesn't work reliably at higher clock speeds
//#define SIDE_POWER 1  // side power switch pa10; SWe 20220719: old, used in Neuron
#define SIDE_nRESET_1  22  //19   // SWe 20220719: nRESET signal OUT to keyboard side 1; HIGH = running, LOW = reset
#define SIDE_nRESET_2  10  //12   // SWe 20220719: nRESET signal OUT to keyboard side 2; HIGH = running, LOW = reset
#define nPWR_OK   1  // SWe 20220719: Power nOK IN-PULLUP from the 3.3V LDO, open drain, needs internal pull-up. NOTE: this is not implemented in the Development Board, only in the real WIRED Neuron2.
// SWe 20220719: LED pins
#define RGBW_LED_RED    6  // SWe 20220719: RED RGBW led OUT, PWM3 A can be used to control its intensity
#define RGBW_LED_GREEN  0  // SWe 20220719: GREEN RGBW led OUT, PWM0 A can be used to control its intensity
#define RGBW_LED_BLUE   2  // SWe 20220719: BLUE RGBW led OUT, PWM1 A can be used to control its intensity
#define RGBW_LED_WHITE  4  // SWe 20220719: WHITE RGBW led OUT, PWM2 A can be used to control its intensity
#define USB_VBUS_DET  7  // SWe 20220719: IN tied to 3.3V which is there when we have power, might be needed for the USB detection. NOTE: when powered from the keyboard side, we have 3.3V but no USB interface.
// SWe 20220719: analog pins
#define USB_CC1  28  // SWe 20220719: USB CC1 pin, can be used to check how much power the host does support by checking its analog value
#define USB_CC2  29  // SWe 20220719: USB CC2 pin, can be used to check how much power the host does support by checking its analog value
// SWe 20220719: ADC Vref input, tied to 3.3V with resistor and capacitor for filtering and buffering
// SWe 20220719: optional pins
#define SW1          3  // SWe 20220719: not used, but is an option to be used as push button input (from the BOOTSEL switch)
//#define UART0_CTS   14  // SWe 20220719: not connected, but has option to be used e.g. as UART interface
#define UART0_RTS   15  // SWe 20220719: not connected, but has option to be used e.g. as UART interface
#define UART0_RX    17  // SWe 20220719: not connected, but has option to be used e.g. as UART interface
// SWe 20220719: not connected pins, should be set as input
#define NC5    5
#define NC12  12
#define NC13  13
//#define NC18  18
#define NC19  19
#define NC24  24
#define NC25  25

#define LAYOUT_ISO 0
#define LAYOUT_ANSI 1

namespace kaleidoscope {
namespace device {
namespace dygma {

/********* WiredHands *********/

struct WiredHands {
  static wired::Hand leftHand;
  static wired::Hand rightHand;

  static void setup();
  static void initializeSides();

  static uint8_t layout;

  static void setSidePower(bool power);
  static bool getSidePower() { return side_power_; }

  static void keyscanInterval(uint16_t interval);
  static uint16_t keyscanInterval() { return keyscan_interval_; }

  static String getChipID();

  static void ledBrightnessCorrection(uint8_t brightness);
  static uint8_t ledBrightnessCorrection() {
    return led_brightness_correction_;
  }

 private:
  static uint16_t keyscan_interval_;
  static uint8_t led_brightness_correction_;
  static bool side_power_;
  static uint16_t settings_base_;
  static uint16_t settings_brightness_;
  static constexpr uint8_t iso_only_led_ = 19;
};

wired::Hand WiredHands::leftHand(0);
wired::Hand WiredHands::rightHand(1);
uint8_t WiredHands::layout;
bool WiredHands::side_power_;
uint16_t WiredHands::settings_base_;
uint16_t WiredHands::settings_brightness_;
uint8_t WiredHands::led_brightness_correction_ = 255;
uint16_t WiredHands::keyscan_interval_ = 50;

void WiredHands::setSidePower(bool power) {
    //digitalWrite(SIDE_POWER, power ? HIGH : LOW);  // SWe 20220719: old Neuron power on/off
  digitalWrite(SIDE_nRESET_1, power ? HIGH : LOW);  // SWe 20220719: new Neuron2 reset or no reset
  digitalWrite(SIDE_nRESET_2, power ? HIGH : LOW);  // SWe 20220719: new Neuron2 reset or no reset
  side_power_ = power;
}

void WiredHands::setup() {
  uint8_t NCS[6] = {NC5,NC12,NC13,NC19,NC24,NC25};
  for (size_t i = 0; i < 6; i++)
  {
    pinMode(NCS[i], INPUT);      // set pin to input
  }

  settings_base_ = ::EEPROMSettings.requestSlice(sizeof(keyscan_interval_));
  settings_brightness_ =
      ::EEPROMSettings.requestSlice(sizeof(led_brightness_correction_));

  // If keyscan is max, assume that EEPROM is uninitialized, and store the
  // defaults.
  uint16_t interval;
  Runtime.storage().get(settings_base_, interval);
  if (interval == 0xffff) {
    Runtime.storage().put(settings_base_, keyscan_interval_);
    Runtime.storage().commit();
  }
  Runtime.storage().get(settings_base_, keyscan_interval_);

  uint8_t brightness;
  Runtime.storage().get(settings_brightness_, brightness);
  if (brightness == 0xff) {
    Runtime.storage().put(settings_brightness_, led_brightness_correction_);
    Runtime.storage().commit();
  }
  Runtime.storage().get(settings_brightness_, led_brightness_correction_);
}

void WiredHands::keyscanInterval(uint16_t interval) {
  leftHand.setKeyscanInterval(interval);
  rightHand.setKeyscanInterval(interval);
  keyscan_interval_ = interval;
  Runtime.storage().put(settings_base_, keyscan_interval_);
  Runtime.storage().commit();
}

void WiredHands::ledBrightnessCorrection(uint8_t brightness) {
  leftHand.setBrightness(brightness);
  rightHand.setBrightness(brightness);
  led_brightness_correction_ = brightness;
  Runtime.storage().put(settings_brightness_, led_brightness_correction_);
  Runtime.storage().commit();
}

String WiredHands::getChipID() {
  char buf[17];
  pico_get_unique_board_id_string(buf, sizeof(buf));
  return String(buf);
}

void WiredHands::initializeSides() {
  // key scan interval from eeprom
  leftHand.setKeyscanInterval(keyscan_interval_);
  rightHand.setKeyscanInterval(keyscan_interval_);

  // led brightness from eeprom
  leftHand.setBrightness(led_brightness_correction_);
  rightHand.setBrightness(led_brightness_correction_);

  // get ANSI/ISO at every side replug
  uint8_t l_layout = leftHand.readLayout();
  uint8_t r_layout = rightHand.readLayout();

  // setup layout variable, this will affect led mapping - defaults to ISO if
  // nothing reported
  // FIXME
  if (l_layout == 1 || r_layout == 1)
    layout = 1;
  else
    layout = 0;

  /*
   * if the neuron starts up with no sides connected, it will assume ISO. This
   * turns on an extra LED (hardware LED 19 on left side). If an ANSI left is
   * then plugged in, the keyboard will switch to ANSI, but LED 19 can't get
   * wiped because the ANSI LED map doesn't include this LED. It will be driven
   * from the SLED1735's memory with the same colour as before, which causes
   * weird looking colours to come on on other seemingly unrelated keys. So: on
   * a replug, set LED 19 to off to be safe.
   */
  leftHand.led_data.leds[iso_only_led_] = {0, 0, 0};

  // get activated LED plugin to refresh
  ::LEDControl.refreshAll();
}

/********* LED Driver *********/

bool WiredLEDDriver::isLEDChangedNeuron;
uint8_t WiredLEDDriver::isLEDChangedLeft[LED_BANKS];
uint8_t WiredLEDDriver::isLEDChangedRight[LED_BANKS];
cRGB WiredLEDDriver::neuronLED;
constexpr uint8_t WiredLEDDriver::led_map[][WiredLEDDriverProps::led_count + 1];

constexpr uint8_t WiredLEDDriverProps::key_led_map[];

void WiredLEDDriver::setBrightness(uint8_t brightness) {
  WiredHands::ledBrightnessCorrection(brightness);
  for (uint8_t i = 0; i < LED_BANKS; i++) {
    isLEDChangedLeft[i] = true;
    isLEDChangedRight[i] = true;
  }
}

uint8_t WiredLEDDriver::getBrightness() {
  return WiredHands::ledBrightnessCorrection();
}

void WiredLEDDriver::syncLeds() {
  // left and right sides
  for (uint8_t i = 0; i < LED_BANKS; i++) {
    // only send the banks that have changed - try to improve jitter performance
    if (isLEDChangedLeft[i]) {
      WiredHands::leftHand.sendLEDBank(i);
      isLEDChangedLeft[i] = false;
    }
    if (isLEDChangedRight[i]) {
      WiredHands::rightHand.sendLEDBank(i);
      isLEDChangedRight[i] = false;
    }
  }

  if (isLEDChangedNeuron) {
    updateNeuronLED();
    isLEDChangedNeuron = false;
  }
}

void WiredLEDDriver::updateNeuronLED() {
  static constexpr struct {
    uint8_t r, g, b, w;
  } pins = { RGBW_LED_RED, RGBW_LED_GREEN, RGBW_LED_BLUE, RGBW_LED_WHITE };
  auto constexpr gamma8 = kaleidoscope::driver::color::gamma_correction;

  // invert as these are common anode, and make sure we reach 65535 to be able
  // to turn fully off.
  analogWrite(pins.r, (pgm_read_byte(&gamma8[neuronLED.r])) << 8);
  analogWrite(pins.g, (pgm_read_byte(&gamma8[neuronLED.g])) << 8);
  analogWrite(pins.b, (pgm_read_byte(&gamma8[neuronLED.b])) << 8);
}

void WiredLEDDriver::setCrgbAt(uint8_t i, cRGB crgb) {
  // prevent reading off the end of the led_map array
  if (i >= WiredLEDDriverProps::led_count) return;

  // neuron LED
  if (i == WiredLEDDriverProps::led_count - 1) {
    isLEDChangedNeuron |= !(neuronLED.r == crgb.r && neuronLED.g == crgb.g &&
                            neuronLED.b == crgb.b);
    neuronLED = crgb;
    return;
  }

  // get the SLED index
  uint8_t sled_num = led_map[WiredHands::layout][i];
  if (sled_num < LEDS_PER_HAND) {
    cRGB oldColor = WiredHands::leftHand.led_data.leds[sled_num];
    WiredHands::leftHand.led_data.leds[sled_num] = crgb;
    isLEDChangedLeft[uint8_t(sled_num / 8)] |=
        !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b);
  } else if (sled_num < 2 * LEDS_PER_HAND) {
    cRGB oldColor =
        WiredHands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND];
    WiredHands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND] = crgb;
    isLEDChangedRight[uint8_t((sled_num - LEDS_PER_HAND) / 8)] |=
        !(oldColor.r == crgb.r && oldColor.g == crgb.g && oldColor.b == crgb.b);
  } else {
    // TODO(anyone):
    // how do we want to handle debugging assertions about crazy user
    // code that would overwrite other memory?
  }
}

cRGB WiredLEDDriver::getCrgbAt(uint8_t i) {
  if (i >= WiredLEDDriverProps::led_count) return {0, 0, 0};

  uint8_t sled_num = led_map[WiredHands::layout][i];
  if (sled_num < LEDS_PER_HAND) {
    return WiredHands::leftHand.led_data.leds[sled_num];
  } else if (sled_num < 2 * LEDS_PER_HAND) {
    return WiredHands::rightHand.led_data.leds[sled_num - LEDS_PER_HAND];
  } else {
    return {0, 0, 0};
  }
}

void WiredLEDDriver::setup() {
  pinMode(SIDE_nRESET_1, OUTPUT);
  pinMode(SIDE_nRESET_2, OUTPUT);
  WiredHands::setSidePower(false);

  // arduino zero analogWrite(255) isn't fully on as its actually working with a
  // 16bit counter and the mapping is a bit shift.
  // so change to 16 bit resolution to avoid the mapping and do the mapping
  // ourselves in updateHubleLED() to ensure LEDs can be set fully off
  analogWriteResolution(16);
  updateNeuronLED();

  delay(100);
  WiredHands::setSidePower(true);
  delay(500);  // wait for sides to power up and finish bootloader
}

/********* Key scanner *********/

wired::keydata_t WiredKeyScanner::leftHandState;
wired::keydata_t WiredKeyScanner::rightHandState;
wired::keydata_t WiredKeyScanner::previousLeftHandState;
wired::keydata_t WiredKeyScanner::previousRightHandState;
wired::keydata_t WiredKeyScanner::leftHandMask;
wired::keydata_t WiredKeyScanner::rightHandMask;
bool WiredKeyScanner::lastLeftOnline;
bool WiredKeyScanner::lastRightOnline;

void WiredKeyScanner::readMatrix() {
  previousLeftHandState = leftHandState;
  previousRightHandState = rightHandState;

  if (WiredHands::leftHand.readKeys()) {
    leftHandState = WiredHands::leftHand.getKeyData();
    // if ANSI, then swap r3c0 and r3c1 to match the PCB
    if (WiredHands::layout == LAYOUT_ANSI) {
      // only swap if bits are different
      if ((leftHandState.rows[3] & (1 << 0)) ^
          leftHandState.rows[3] & (1 << 1)) {
        leftHandState.rows[3] ^= (1 << 0);  // flip the bit
        leftHandState.rows[3] ^= (1 << 1);  // flip the bit
      }
    }
  }

  if (WiredHands::rightHand.readKeys()) {
    rightHandState = WiredHands::rightHand.getKeyData();
    // if ANSI, then swap r1c0 and r2c0 to match the PCB
    if (WiredHands::layout == LAYOUT_ANSI) {
      if ((rightHandState.rows[1] & (1 << 0)) ^
          rightHandState.rows[2] & (1 << 0)) {
        rightHandState.rows[1] ^= (1 << 0);
        rightHandState.rows[2] ^= (1 << 0);
      }
    }
  }

  // if a side has just been replugged, initialise it
  if ((WiredHands::leftHand.online && !lastLeftOnline) ||
      (WiredHands::rightHand.online && !lastRightOnline))
    WiredHands::initializeSides();

  // if a side has just been unplugged, wipe its state
  if (!WiredHands::leftHand.online && lastLeftOnline) leftHandState.all = 0;

  if (!WiredHands::rightHand.online && lastRightOnline) rightHandState.all = 0;

  // store previous state of whether the sides are plugged in
  lastLeftOnline = WiredHands::leftHand.online;
  lastRightOnline = WiredHands::rightHand.online;
}

void WiredKeyScanner::actOnMatrixScan() {
  for (byte row = 0; row < Props_::matrix_rows; row++) {
    for (byte col = 0; col < Props_::left_columns; col++) {
      uint8_t keynum = (row * Props_::left_columns) + col;
      uint8_t keyState;

      // left
      keyState = (bitRead(previousLeftHandState.all, keynum) << 0) |
                 (bitRead(leftHandState.all, keynum) << 1);
      if (keyState)
        ThisType::handleKeyswitchEvent(Key_NoKey, KeyAddr(row, col), keyState);

      // right
      keyState = (bitRead(previousRightHandState.all, keynum) << 0) |
                 (bitRead(rightHandState.all, keynum) << 1);
      if (keyState)
        ThisType::handleKeyswitchEvent(
            Key_NoKey, KeyAddr(row, (Props_::matrix_columns - 1) - col),
            keyState);
    }
  }
}

void WiredKeyScanner::scanMatrix() {
  readMatrix();
  actOnMatrixScan();
}

void WiredKeyScanner::maskKey(KeyAddr key_addr) {
  if (!key_addr.isValid()) return;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    rightHandMask.rows[row] |=
        1 << (Props_::right_columns - (col - Props_::left_columns));
  } else {
    leftHandMask.rows[row] |= 1 << (Props_::right_columns - col);
  }
}

void WiredKeyScanner::unMaskKey(KeyAddr key_addr) {
  if (!key_addr.isValid()) return;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    rightHandMask.rows[row] &=
        ~(1 << (Props_::right_columns - (col - Props_::left_columns)));
  } else {
    leftHandMask.rows[row] &= ~(1 << (Props_::right_columns - col));
  }
}

bool WiredKeyScanner::isKeyMasked(KeyAddr key_addr) {
  if (!key_addr.isValid()) return false;

  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= 8) {
    return rightHandMask.rows[row] & (1 << (7 - (col - 8)));
  } else {
    return leftHandMask.rows[row] & (1 << (7 - col));
  }
}

void WiredKeyScanner::maskHeldKeys() {
  memcpy(leftHandMask.rows, leftHandState.rows, sizeof(leftHandMask));
  memcpy(rightHandMask.rows, rightHandState.rows, sizeof(rightHandMask));
}

bool WiredKeyScanner::isKeyswitchPressed(KeyAddr key_addr) {
  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    return (bitRead(rightHandState.rows[row],
                    (Props_::matrix_columns - 1) - col) != 0);
  } else {
    return (bitRead(leftHandState.rows[row], col) != 0);
  }
}

bool WiredKeyScanner::wasKeyswitchPressed(KeyAddr key_addr) {
  auto row = key_addr.row();
  auto col = key_addr.col();

  if (col >= Props_::left_columns) {
    return (bitRead(previousRightHandState.rows[row],
                    (Props_::matrix_columns - 1) - col) != 0);
  } else {
    return (bitRead(previousLeftHandState.rows[row], col) != 0);
  }
}

uint8_t WiredKeyScanner::pressedKeyswitchCount() {
  return __builtin_popcountll(leftHandState.all) +
         __builtin_popcountll(rightHandState.all);
}

uint8_t WiredKeyScanner::previousPressedKeyswitchCount() {
  return __builtin_popcountll(previousLeftHandState.all) +
         __builtin_popcountll(previousRightHandState.all);
}

void WiredKeyScanner::setKeyscanInterval(uint8_t interval) {
  WiredHands::leftHand.setKeyscanInterval(interval);
  WiredHands::rightHand.setKeyscanInterval(interval);
}

void WiredKeyScanner::setup() {
  static constexpr uint8_t keyscanner_pins[] = {
      2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 30,
      31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42};
  for (int i = 0; i < sizeof(keyscanner_pins); i++) {
    // pinMode(keyscanner_pins[i], OUTPUT);
    // digitalWrite(keyscanner_pins[i], LOW);
  }
}

void WiredKeyScanner::reset() {
  leftHandState.all = 0;
  rightHandState.all = 0;
  Runtime.hid().keyboard().releaseAllKeys();
  Runtime.hid().keyboard().sendReport();
}

/********* Hardware plugin *********/
void Wired::setup() {
  WiredHands::setup();
  KeyScanner::setup();
  LEDDriver::setup();

  // initialise Wire of scanner - have to do this here to avoid problem with
  // static object intialisation ordering
  WIRE_.setSDA(I2C_SDA_PIN);
  WIRE_.setSCL(I2C_SCL_PIN);
  WIRE_.begin();
  WIRE_.setClock(I2C_CLOCK_KHZ * 1000);

  WiredHands::initializeSides();
}

void Wired::side::prepareForFlash() {
  WIRE_.end();

  setPower(LOW);
  // also turn off i2c pins to stop attiny from getting enough current through
  // i2c to stay on
  pinMode(I2C_SDA_PIN, OUTPUT);
  pinMode(I2C_SCL_PIN, OUTPUT);
  digitalWrite(I2C_SDA_PIN, false);
  digitalWrite(I2C_SCL_PIN, false);

  // wipe key states, to prevent accidental key repeats
  WiredKeyScanner::reset();

  setPower(HIGH);

  WIRE_.setSDA(I2C_SDA_PIN);
  WIRE_.setSCL(I2C_SCL_PIN);
  WIRE_.begin();
  WIRE_.setClock(I2C_FLASH_CLOCK_KHZ * 1000);
  // wait for side bootloader to be ready
  sleep_ms(50);
}

uint8_t Wired::side::getPower() { return WiredHands::getSidePower(); }
void Wired::side::setPower(uint8_t power) { WiredHands::setSidePower(power); }

uint8_t Wired::side::leftVersion() {
  return WiredHands::leftHand.readVersion();
}
uint8_t Wired::side::rightVersion() {
  return WiredHands::rightHand.readVersion();
}

uint8_t Wired::side::leftCRCErrors() {
  return WiredHands::leftHand.crc_errors();
}
uint8_t Wired::side::rightCRCErrors() {
  return WiredHands::rightHand.crc_errors();
}

uint8_t Wired::side::leftSLEDVersion() {
  return WiredHands::leftHand.readSLEDVersion();
}
uint8_t Wired::side::rightSLEDVersion() {
  return WiredHands::rightHand.readSLEDVersion();
}

uint8_t Wired::side::leftSLEDCurrent() {
  return WiredHands::leftHand.readSLEDCurrent();
}
uint8_t Wired::side::rightSLEDCurrent() {
  return WiredHands::rightHand.readSLEDCurrent();
}
void Wired::side::setSLEDCurrent(uint8_t current) {
  WiredHands::rightHand.setSLEDCurrent(current);
  WiredHands::leftHand.setSLEDCurrent(current);
}

Wired::settings::Layout Wired::settings::layout() {
  return WiredHands::layout == LAYOUT_ANSI ? Layout::ANSI : Layout::ISO;
}
uint8_t Wired::settings::joint() { return WiredHands::rightHand.readJoint(); }

uint16_t Wired::settings::keyscanInterval() {
  return WiredHands::keyscanInterval();
}
String Wired::settings::getChipID() {
  return WiredHands::getChipID();
}
void Wired::settings::keyscanInterval(uint16_t interval) {
  WiredHands::keyscanInterval(interval);
}

}  // namespace dygma
}  // namespace device
}  // namespace kaleidoscope

#endif
