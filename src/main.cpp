// -*- mode: c++ -*-
/* Raise-Firmware -- Factory firmware for the Dygma Raise
 * Copyright (C) 2019, 2020  DygmaLab, SE.
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

#ifndef BUILD_INFORMATION
#define BUILD_INFORMATION "locally built"
#endif

#include <Arduino.h>
#include "Kaleidoscope.h"
#include "hardware/watchdog.h"
#include "Kaleidoscope-MouseKeys.h"
#include "Kaleidoscope-LEDControl.h"
#include "Kaleidoscope-PersistentLEDMode.h"
#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-EEPROM-Settings.h"
#include "Kaleidoscope-EEPROM-Keymap.h"
#include "Kaleidoscope-IdleLEDs.h"
#include "Kaleidoscope-LED-Palette-Theme.h"
#include "Kaleidoscope-DynamicSuperKeys.h"
#include "Kaleidoscope-DynamicMacros.h"
#include "Kaleidoscope-MagicCombo.h"
#include "Kaleidoscope-USB-Quirks.h"
#include "Kaleidoscope-LayerFocus.h"
#include "DefyFirmwareVersion.h"
#include "Watchdog_timer.h"

// Support for host power management (suspend & wakeup)
#include "Kaleidoscope-HostPowerManagement.h"

#include "Kaleidoscope-OneShot.h"
#include "Kaleidoscope-Qukeys.h"
#include "Kaleidoscope-Escape-OneShot.h"

#include "LED-CapsLockLight.h"
#include "EEPROMPadding.h"

#include "EEPROMUpgrade.h"
#include "Upgrade.h"

#include "LEDEffect-Rainbow-Defy.h"
#include "LEDEffect-Stalker-Defy.h"
#include "LEDEffect-SolidColor-Defy.h"
#include "Colormap-Defy.h"
#include "LED-Palette-Theme-Defy.h"
#include "kaleidoscope/device/dygma/defyWN/universalModules/SettingsConfigurator.h"
#include "Spi_slave.h"
#include "IntegrationTest.h"

Watchdog_timer watchdog_timer;

enum {
  QWERTY,
  NUMPAD,
  _LAYER_MAX
};  // layers

/* This comment temporarily turns off astyle's indent enforcement so we can make
 * the keymaps actually resemble the physical key layout better
 */
// clang-format off
KEYMAPS(
[QWERTY] = KEYMAP_STACKED
(
    Key_Escape, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6
    , Key_Tab, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_VolumeUp
    , Key_CapsLock, Key_A, Key_S, Key_D, Key_F, Key_G, Key_VolumeDown
    , Key_Backslash, Key_Z, Key_X, Key_C, Key_V, Key_B
    , Key_LeftControl, Key_LeftGui, Key_Backspace, Key_Delete
    , Key_LeftShift, Key_LeftAlt, Key_Enter, Key_Space

    , LT(1,Space), Key_8, Key_9, Key_0, Key_Minus, Key_Equals, Key_Backspace
    , Key_Y, Key_U, Key_I, Key_O, Key_P, Key_LeftBracket, Key_RightBracket
    , Key_H, Key_J, Key_K, Key_L, Key_Semicolon, Key_Quote, Key_RightShift
    , Key_N, Key_M, Key_Comma, Key_Period, Key_Slash, Key_RightShift
    , Key_LEDEffectNext, Key_Home, Key_UpArrow, Key_End
    , Key_RightArrow, Key_DownArrow, Key_LeftArrow, Key_Enter
),

[NUMPAD] = KEYMAP_STACKED
(
    Key_Escape, Key_F1, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6
    , Key_Tab, Key_NoKey, Key_UpArrow, Key_NoKey, Key_NoKey, Key_NoKey, Key_NoKey
    , Key_CapsLock, Key_LeftArrow, Key_DownArrow, Key_RightArrow, Key_NoKey, Key_NoKey, Key_NoKey
    , Key_LeftShift, Key_Backslash, Key_NoKey, Key_NoKey, Key_NoKey, Key_NoKey
    , Key_LeftControl, Key_LeftGui, Key_LeftAlt, Key_Space
    , Key_Space, Key_Backspace, Key_Enter, Key_Delete

    , Key_F7, Key_F8, Key_F9, Key_F10, Key_F11, Key_F12, Key_Backspace
    , Key_KeypadSubtract, Key_7, Key_8, Key_9, Key_KeypadDivide, Key_NoKey, Key_Enter
    , Key_KeypadAdd, Key_4, Key_5, Key_6, Key_KeypadMultiply, Key_NoKey, Key_Backslash
    , Key_KeypadDot, Key_1, Key_2, Key_3, Key_UpArrow, Key_RightShift
    , Key_0, Key_Space, Key_LeftArrow, Key_DownArrow
    , Key_RightArrow, Key_RightControl, Key_Delete, MoveToLayer(QWERTY)
)
);
// clang-format on


/** toggleLedsOnSuspendResume toggles the LEDs off when the host goes to sleep,
 * and turns them back on when it wakes up.
 */
void toggleLedsOnSuspendResume(kaleidoscope::plugin::HostPowerManagement::Event event) {
  switch (event) {
  case kaleidoscope::plugin::HostPowerManagement::Suspend:
    LEDControl.disable();
    break;
  case kaleidoscope::plugin::HostPowerManagement::Resume:
    LEDControl.enable();
    break;
  case kaleidoscope::plugin::HostPowerManagement::Sleep:
    break;
  }
}

/** hostPowerManagementEventHandler dispatches power management events (suspend,
 * resume, and sleep) to other functions that perform action based on these
 * events.
 */
void hostPowerManagementEventHandler(kaleidoscope::plugin::HostPowerManagement::Event event) {
  //TODO: Check of to manage this.
  toggleLedsOnSuspendResume(event);
}

enum {
  COMBO_TOGGLE_NKRO_MODE
};

static uint32_t protocol_toggle_start = 0;

static void toggleKeyboardProtocol(uint8_t combo_index) {
  USBQuirks.toggleKeyboardProtocol();
  protocol_toggle_start = Kaleidoscope.millisAtCycleStart();
}

static void protocolBreathe() {
  if (Kaleidoscope.hasTimeExpired(protocol_toggle_start, uint16_t(10000))) {
    protocol_toggle_start = 0;
  }
  if (protocol_toggle_start == 0)
    return;

  uint8_t hue = 120;
  if (Kaleidoscope.hid().keyboard().getProtocol() == HID_BOOT_PROTOCOL) {
    hue = 0;
  }

  cRGB color = breath_compute(hue);
  ::LEDControl.setCrgbAt(KeyAddr(4, 0), color);
  ::LEDControl.setCrgbAt(KeyAddr(3, 0), color);
  ::LEDControl.setCrgbAt(KeyAddr(4, 2), color);
  ::LEDControl.setCrgbAt(KeyAddr(0, 6), color);
  ::LEDControl.syncLeds();
}

USE_MAGIC_COMBOS(
  {.action = toggleKeyboardProtocol,
   // Left Ctrl + Left Shift + Left Alt + 6
   .keys = {R4C0, R3C0, R4C2, R0C6}});

kaleidoscope::plugin::EEPROMPadding JointPadding(8);
static kaleidoscope::plugin::LEDSolidColorDefy solidRedDefy(255, 0, 0, 0);
static kaleidoscope::plugin::LEDSolidColorDefy solidGreenDefy(0, 255, 0, 0);
static kaleidoscope::plugin::LEDSolidColorDefy solidBlueDefy(0, 0, 255, 0);
static kaleidoscope::plugin::LEDSolidColorDefy solidWhiteDefy(0, 0, 0, 255);
static kaleidoscope::plugin::LEDStalkerDefy stalkerDefy{};


// clang-format off
KALEIDOSCOPE_INIT_PLUGINS(
  FirmwareVersion,
  Upgrade,
  USBQuirks,
  MagicCombo,
  IdleLEDs,
  EEPROMSettings,
  EEPROMKeymap,
  FocusSettingsCommand,
  FocusEEPROMCommand,
  LEDCapsLockLight,
  LEDControl,
  PersistentLEDMode,
  FocusLEDCommand,
  LEDPaletteThemeDefy,
  JointPadding,
  ColormapEffectDefy,
  LEDRainbowWaveEffectDefy,LEDRainbowEffectDefy,stalkerDefy,solidRedDefy, solidGreenDefy, solidBlueDefy,solidWhiteDefy,
  PersistentIdleLEDs,
  SettingsConfigurator,
  Qukeys,
  DynamicSuperKeys,
  DynamicMacros,
  Focus,
  MouseKeys,
  OneShot,
  EscapeOneShot,
  LayerFocus,
  EEPROMUpgrade,
  IntegrationTest,
  HostPowerManagement);
// clang-format on


void setup() {

  multicore_reset_core1();

  TinyUSBDevice.setID(BOARD_VENDORID, BOARD_PRODUCTID);
  TinyUSBDevice.setManufacturerDescriptor(BOARD_MANUFACTURER);
  TinyUSBDevice.setProductDescriptor(BOARD_PRODUCT);

  Serial.begin(115200);

  // Initialize the communications before Kaleidoscope to make sure the correct order of the incoming message processing
  Communications.init();

  // First start the serial communications to avoid restarting unnecesarily
  Kaleidoscope.setup();
  // Reserve space in the keyboard's EEPROM for the keymaps
  EEPROMKeymap.setup(10);
  // Reserve space for the number of Colormap layers we will use
  ColormapEffectDefy.max_layers(10);

  DynamicSuperKeys.setup(0, 1024);
  DynamicMacros.reserve_storage(2048);
  EEPROMUpgrade.reserveStorage();
  EEPROMUpgrade.upgrade();

  // Keep the HID begin after the Kaleidoscope setup.
  HID().begin();
}

void loop() {
  // Application code goes here...

  HID().SendLastReport();

  Kaleidoscope.loop();
  Communications.run();
  protocolBreathe();
}
