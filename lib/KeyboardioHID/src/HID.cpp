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
  descriptor.resize(descriptor.size() + node->length);
  for (int i = 0; i < node->length; ++i) {
	descriptor.push_back(node->data[i]);
  }
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

int HID_::begin() {
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(descriptor.data(), descriptor.size());
  usb_hid.setBootProtocol(0);
  usb_hid.begin();
  return 0;
}
