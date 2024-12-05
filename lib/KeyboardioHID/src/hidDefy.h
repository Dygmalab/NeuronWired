/*
Copyright (c) 2023, 2024 Dygmalab S.L.

Copyright (c) 2015, Arduino LLC
Original code (pre-library): Copyright (c) 2011, Peter Barrett

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef HID_h
#define HID_h

#include <stdint.h>
#include <Arduino.h>
#include "HID-Settings.h"
#include <Adafruit_TinyUSB.h>

#include "DescriptorPrimitives.h"
#include "MultiReport/Keyboard.h"

#define _USING_HID

// HID 'Driver'
// ------------
#define HID_GET_REPORT        0x01
#define HID_GET_IDLE          0x02
#define HID_GET_PROTOCOL      0x03
#define HID_SET_REPORT        0x09
#define HID_SET_IDLE          0x0A
#define HID_SET_PROTOCOL      0x0B

#define HID_HID_DESCRIPTOR_TYPE         0x21
#define HID_REPORT_DESCRIPTOR_TYPE      0x22
#define HID_PHYSICAL_DESCRIPTOR_TYPE    0x23

// HID subclass HID1.11 Page 8 4.2 Subclass
#define HID_SUBCLASS_NONE 0
#define HID_SUBCLASS_BOOT_INTERFACE 1

// HID Keyboard/Mouse bios compatible protocols HID1.11 Page 9 4.3 Protocols
#define HID_PROTOCOL_NONE 0
#define HID_PROTOCOL_KEYBOARD 1
#define HID_PROTOCOL_MOUSE 2

// Normal or bios protocol (Keyboard/Mouse) HID1.11 Page 54 7.2.5 Get_Protocol Request
// "protocol" variable is used for this purpose.
#define HID_BOOT_PROTOCOL	0
#define HID_REPORT_PROTOCOL	1

// HID Request Type HID1.11 Page 51 7.2.1 Get_Report Request
#define HID_REPORT_TYPE_INPUT   1
#define HID_REPORT_TYPE_OUTPUT  2
#define HID_REPORT_TYPE_FEATURE 3

class HIDSubDescriptor {
 public:
  HIDSubDescriptor(const uint8_t *d, const uint16_t l) : data(d), length(l) { }
  const uint8_t * data;
  const uint16_t length;
};

enum
{
    REPORT_ID_KEYBOARD = 0x01,
    REPORT_ID_MOUSE = 0x02,
    REPORT_ID_CONSUMER_CONTROL = 0x03,     // Media, volume etc ..
    REPORT_ID_SYSTEM_CONTROL = 0x04,
    REPORT_ID_RAW = 0x05
};

enum
{
    RAW_USAGE_UNKNOWN = 0x00,
    RAW_USAGE_ANSI = 0x01,
    RAW_USAGE_ISO = 0x02,
    RAW_USAGE_DEFY = 0x03,
};

class HID_ {
 public:

  HID_();
  int begin();
  int SendReport(uint8_t id, const void* data, int len);
  bool SendLastReport();
  void AppendDescriptor(HIDSubDescriptor* node);

  void hid_report_descriptor_get( const uint8_t ** pp_desc, uint32_t * p_desc_len );

  uint8_t getLEDs() {
    return setReportData.leds;
  };

  uint8_t getShortName(char *name);
  int SendReport_(uint8_t id, const void* data, int len);
  Adafruit_USBD_HID usb_hid;
private:
  char keyboarName[20] = "Defy RP2040";
//  std::vector<uint8_t> descriptor;

  uint8_t protocol;
  uint8_t idle;
  struct {
    uint8_t reportId;
    uint8_t leds;
  } setReportData;
};

// Replacement for global singleton.
// This function prevents static-initialization-order-fiasco
// https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
HID_& HID();

#define D_HIDREPORT(length) { 9, 0x21, 0x01, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }

/*******************************************************/
/*               HID descriptor builder                */
/*******************************************************/

// clang-format off
// Consumer Control Report Descriptor Template
#define TUD_HID_REPORT_DESC_CONSUMER_DYGMA(...) \
HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER    )              ,\
HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL )              ,\
HID_COLLECTION ( HID_COLLECTION_APPLICATION )              , /* Report ID if any */\
 __VA_ARGS__ \
 HID_LOGICAL_MIN  ( 0x00                                ) ,\
 HID_LOGICAL_MAX_N( 0x03FF, 2                           ) ,\
 HID_USAGE_MIN    ( 0x00                                ) ,\
 HID_USAGE_MAX_N  ( 0x03FF, 2                           ) ,\
 HID_REPORT_COUNT ( 4                                   ) ,\
 HID_REPORT_SIZE  ( 16                                  ) ,\
 HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ) ,\
HID_COLLECTION_END                            \
/*Enable back clang*/
// clang-format on

#if (KEY_BITS % 8)
    /* Padding to round up the report to byte boundary. */
#define KEYBITS_PADDING    D_REPORT_SIZE, (8 - (KEY_BITS % 8)), D_REPORT_COUNT, 0x01, D_INPUT, (D_CONSTANT),
#else
#define KEYBITS_PADDING
#endif

#define HID_DEFY_REPORT_DESCRIPTOR( usage_raw ) {                                                                                           \
    D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP, D_USAGE, D_USAGE_KEYBOARD, D_COLLECTION, D_APPLICATION, D_REPORT_ID, HID_REPORTID_NKRO_KEYBOARD,      \
    D_USAGE_PAGE, D_PAGE_KEYBOARD,                                                                                                              \
                                                                                                                                                \
    /* Key modifier byte */                                                                                                                     \
    D_USAGE_MINIMUM, HID_KEYBOARD_FIRST_MODIFIER, D_USAGE_MAXIMUM, HID_KEYBOARD_LAST_MODIFIER,                                                  \
    D_LOGICAL_MINIMUM, 0x00, D_LOGICAL_MAXIMUM, 0x01, D_REPORT_SIZE, 0x01, D_REPORT_COUNT, 0x08,                                                \
    D_INPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),                                                                                                \
                                                                                                                                                \
    /* 5 LEDs for num lock etc, 3 left for advanced, custom usage */                                                                            \
    D_USAGE_PAGE, D_PAGE_LEDS, D_USAGE_MINIMUM, 0x01, D_USAGE_MAXIMUM, 0x08, D_REPORT_COUNT, 0x08, D_REPORT_SIZE, 0x01,                         \
    D_OUTPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),                                                                                               \
                                                                                                                                                \
    /* NKRO Keyboard */                                                                                                                         \
    D_USAGE_PAGE, D_PAGE_KEYBOARD,                                                                                                              \
                                                                                                                                                \
    /* Padding 4 bits, to skip NO_EVENT & 3 error states. */                                                                                    \
    D_REPORT_SIZE, 0x04, D_REPORT_COUNT, 0x01, D_INPUT, (D_CONSTANT),                                                                           \
                                                                                                                                                \
    D_USAGE_MINIMUM, HID_KEYBOARD_A_AND_A, D_USAGE_MAXIMUM, HID_LAST_KEY,                                                                       \
    D_LOGICAL_MINIMUM, 0x00, D_LOGICAL_MAXIMUM, 0x01, D_REPORT_SIZE, 0x01,                                                                      \
    D_REPORT_COUNT, (KEY_BITS - 4), D_INPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),                                                                \
                                                                                                                                                \
    /* Padding to round up the report to byte boundary. */                                                                                      \
    KEYBITS_PADDING                                                                                                                             \
                                                                                                                                                \
    D_END_COLLECTION,                                                                                                                           \
                                                                                                                                                \
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),                                                                                  \
    TUD_HID_REPORT_DESC_CONSUMER_DYGMA(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL)),                                                              \
    TUD_HID_REPORT_DESC_SYSTEM_CONTROL(HID_REPORT_ID(REPORT_ID_SYSTEM_CONTROL)),                                                                \
}

#endif // HID_h
