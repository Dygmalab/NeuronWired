/*
   Copyright (c) 2015, Arduino LLC
   Original code (pre-library): Copyright (c) 2011, Peter Barrett

   Permission to use, copy, modify, and/or distribute this software for
   any purpose with or without fee is hereby granted, provided that the
   above copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
   BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
   OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
   ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
   SOFTWARE.
 */


#include "HID.h"
#include "HIDReportObserver.h"
#include "DescriptorPrimitives.h"
#include "HIDAliases.h"
#include "MultiReport/Keyboard.h"

HID_ &HID() {
  static HID_ obj;
  return obj;
}

void HID_::AppendDescriptor(HIDSubDescriptor *node) {
//  descriptor.resize(descriptor.size() + node->length);
//  for (int i = 0; i < node->length; ++i) {
//	descriptor.push_back(node->data[i]);
//  }
}

int HID_::SendReport(uint8_t id, const void *data, int len) {
  auto result = SendReport_(id, data, len);
  HIDReportObserver::observeReport(id, data, len, result);
  return result;
}

int HID_::SendReport_(uint8_t id, const void *data, int len) {
  /* On SAMD, we need to send the whole report in one batch; sending the id, and
   * the report itself separately does not work, the report never arrives. Due
   * to this, we merge the two into a single buffer, and send that.
   *
   * While the same would work for other architectures, AVR included, doing so
   * costs RAM, which is something scarce on AVR. So on that platform, we opt to
   * send the id and the report separately instead. */
#ifdef ARDUINO_ARCH_SAMD
  uint8_t p[64];
  p[0] = id;
  memcpy(&p[1], data, len);
  return USB_Send(pluggedEndpoint, p, len + 1);
#elseif AVR
  auto ret = USB_Send(pluggedEndpoint, &id, 1);
  if (ret < 0)
	return ret;
  auto ret2 = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, len);
  if (ret2 < 0)
	return ret2;
  return ret + ret2;
#endif
  bool b = usb_hid.sendReport(id, data, len);
  return b;

}

HID_::HID_() : protocol(HID_REPORT_PROTOCOL), idle(0) {
  setReportData.reportId = 0;
  setReportData.leds = 0;
}

uint8_t const descriptor_tmp[] = {  //  NKRO Keyboard
	D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,
	D_USAGE, D_USAGE_KEYBOARD,
	D_COLLECTION, D_APPLICATION,
	D_REPORT_ID, HID_REPORTID_NKRO_KEYBOARD,
	D_USAGE_PAGE, D_PAGE_KEYBOARD,

	/* Key modifier byte */
	D_USAGE_MINIMUM, HID_KEYBOARD_FIRST_MODIFIER,
	D_USAGE_MAXIMUM, HID_KEYBOARD_LAST_MODIFIER,
	D_LOGICAL_MINIMUM, 0x00,
	D_LOGICAL_MAXIMUM, 0x01,
	D_REPORT_SIZE, 0x01,
	D_REPORT_COUNT, 0x08,
	D_INPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),

	/* 5 LEDs for num lock etc, 3 left for advanced, custom usage */
	D_USAGE_PAGE, D_PAGE_LEDS,
	D_USAGE_MINIMUM, 0x01,
	D_USAGE_MAXIMUM, 0x08,
	D_REPORT_COUNT, 0x08,
	D_REPORT_SIZE, 0x01,
	D_OUTPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),

	/* NKRO Keyboard */
	D_USAGE_PAGE, D_PAGE_KEYBOARD,

	// Padding 4 bits, to skip NO_EVENT & 3 error states.
	D_REPORT_SIZE, 0x04,
	D_REPORT_COUNT, 0x01,
	D_INPUT, (D_CONSTANT),

	D_USAGE_MINIMUM, HID_KEYBOARD_A_AND_A,
	D_USAGE_MAXIMUM, HID_LAST_KEY,
	D_LOGICAL_MINIMUM, 0x00,
	D_LOGICAL_MAXIMUM, 0x01,
	D_REPORT_SIZE, 0x01,
	D_REPORT_COUNT, (KEY_BITS - 4),
	D_INPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),

#if (KEY_BITS%8)
	// Padding to round up the report to byte boundary.
	D_REPORT_SIZE, (8 - (KEY_BITS%8)),
	D_REPORT_COUNT, 0x01,
	D_INPUT, (D_CONSTANT),
#endif

	D_END_COLLECTION,
	D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,           // USAGE_PAGE (Generic Desktop)
	D_USAGE, D_USAGE_MOUSE,                         // USAGE (Mouse)
	D_COLLECTION, D_APPLICATION,                    // COLLECTION (Application)
	D_REPORT_ID, HID_REPORTID_MOUSE,                // REPORT_ID (Mouse)

	/* 8 Buttons */
	D_USAGE_PAGE, D_PAGE_BUTTON,                    // USAGE_PAGE (Button)
	D_USAGE_MINIMUM, 0x01,                          // USAGE_MINIMUM (Button 1)
	D_USAGE_MAXIMUM, 0x08,                          // USAGE_MAXIMUM (Button 8)
	D_LOGICAL_MINIMUM, 0x00,                        // LOGICAL_MINIMUM (0)
	D_LOGICAL_MAXIMUM, 0x01,                        // LOGICAL_MAXIMUM (1)
	D_REPORT_COUNT, 0x08,                           // REPORT_COUNT (8)
	D_REPORT_SIZE, 0x01,                            // REPORT_SIZE (1)
	D_INPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),    // INPUT (Data,Var,Abs)

	/* X, Y, Wheel */
	D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,           // USAGE_PAGE (Generic Desktop)
	D_USAGE, 0x30,                                  // USAGE (X)
	D_USAGE, 0x31,                                  // USAGE (Y)
	D_USAGE, 0x38,                                  // USAGE (Wheel)
	D_LOGICAL_MINIMUM, 0x81,                        // LOGICAL_MINIMUM (-127)
	D_LOGICAL_MAXIMUM, 0x7f,                        // LOGICAL_MAXIMUM (127)
	D_REPORT_SIZE, 0x08,                            // REPORT_SIZE (8)
	D_REPORT_COUNT, 0x03,                           // REPORT_COUNT (3)
	D_INPUT, (D_DATA | D_VARIABLE | D_RELATIVE),    // INPUT (Data,Var,Rel)

	/* Horizontal wheel */
	D_USAGE_PAGE, D_PAGE_CONSUMER,                  // USAGE_PAGE (Consumer)
	D_PAGE_ORDINAL, 0x38, 0x02,                     // PAGE (AC Pan)
	D_LOGICAL_MINIMUM, 0x81,                        // LOGICAL_MINIMUM (-127)
	D_LOGICAL_MAXIMUM, 0x7f,                        // LOGICAL_MAXIMUM (127)
	D_REPORT_SIZE, 0x08,                            // REPORT_SIZE (8)
	D_REPORT_COUNT, 0x01,                           // REPORT_COUNT (1)
	D_INPUT, (D_DATA | D_VARIABLE | D_RELATIVE),    // INPUT (Data,Var,Rel)

	/* End */
	D_END_COLLECTION,                          // END_COLLECTION,
	D_USAGE_PAGE, 0x0C,                           /* usage page (consumer device) */
	D_USAGE, 0x01,                                /* usage -- consumer control */
	D_COLLECTION, D_APPLICATION,                  /* collection (application) */
	D_REPORT_ID, HID_REPORTID_CONSUMERCONTROL,    /* report id */
	/* 4 Media Keys */
	D_LOGICAL_MINIMUM, 0x00,                      /* logical minimum */
	D_MULTIBYTE(D_LOGICAL_MAXIMUM), 0xFF, 0x03,   /* logical maximum (3ff) */
	D_USAGE_MINIMUM, 0x00,                        /* usage minimum (0) */
	D_MULTIBYTE(D_USAGE_MAXIMUM), 0xFF, 0x03,     /* usage maximum (3ff) */
	D_REPORT_COUNT, 0x04,                         /* report count (4) */
	D_REPORT_SIZE, 0x10,                          /* report size (16) */
	D_INPUT, 0x00,                                /* input */
	D_END_COLLECTION,
	D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,         /* USAGE_PAGE (Generic Desktop) */
	D_USAGE, 0x80,                                /* USAGE (System Control) */
	D_COLLECTION, D_APPLICATION,                  /* COLLECTION (Application) */
	D_REPORT_ID, HID_REPORTID_SYSTEMCONTROL,      /* REPORT_ID */
	/* 1 system key */
	D_LOGICAL_MINIMUM, 0x00,                      /* LOGICAL_MINIMUM (0) */
	D_MULTIBYTE(D_LOGICAL_MAXIMUM), 0xff, 0x00,   /* LOGICAL_MAXIMUM (255) */
	D_USAGE_MINIMUM, 0x00,                        /* USAGE_MINIMUM (Undefined) */
	D_USAGE_MAXIMUM, 0xff,                        /* USAGE_MAXIMUM (System Menu Down) */
	D_REPORT_COUNT, 0x01,                         /* REPORT_COUNT (1) */
	D_REPORT_SIZE, 0x08,                          /* REPORT_SIZE (8) */
	D_INPUT, (D_DATA | D_ARRAY | D_ABSOLUTE),     /* INPUT (Data,Ary,Abs) */
	D_END_COLLECTION                              /* END_COLLECTION *//* end collection *///
};

int HID_::begin() {
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(descriptor_tmp, sizeof(descriptor_tmp));
  usb_hid.setBootProtocol(0);
  usb_hid.begin();
  return 0;
}
