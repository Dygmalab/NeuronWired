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


#include "hidDefy.h"
#include "DescriptorPrimitives.h"
#include "HIDAliases.h"
#include "HIDReportObserver.h"
#include "MultiReport/Keyboard.h"

/*
 * Extern functions for providing the HID report descriptor. Needs to be defined on the application level.
 */
extern void hid_report_descriptor_usb_get( const uint8_t ** pp_desc, uint32_t * p_desc_len );

HID_ &HID()
{
    static HID_ obj;
    return obj;
}

mutex_t reportMutex;

void HID_::AppendDescriptor(HIDSubDescriptor *node)
{
    //  descriptor.resize(descriptor.size() + node->length);
    //  for (int i = 0; i < node->length; ++i) {
    //	descriptor.push_back(node->data[i]);
    //  }
}

int HID_::SendReport(uint8_t id, const void *data, int len)
{
    auto result = SendReport_(id, data, len);
    HIDReportObserver::observeReport(id, data, len, result);
    return result;
}


tu_fifo_t tx_ff_hid;

uint8_t tx_ff_buf_hid[8000];

struct NextReport
{
    uint8_t id;
    uint16_t len;
};

int HID_::SendReport_(uint8_t id, const void *data, int len)
{
    /* On SAMD, we need to send the whole report in one batch; sending the id, and
     * the report itself separately does not work, the report never arrives. Due
     * to this, we merge the two into a single buffer, and send that.
     *
     * While the same would work for other architectures, AVR included, doing so
     * costs RAM, which is something scarce on AVR. So on that platform, we opt to
     * send the id and the report separately instead. */
//#if defined(ARDUINO_ARCH_AVR)
//    uint8_t p[64];
//    p[0] = id;
//    memcpy(&p[1], data, len);
//    return USB_Send(pluggedEndpoint, p, len + 1);
//#elseif AVR
//    auto ret = USB_Send(pluggedEndpoint, &id, 1);
//    if (ret < 0) return ret;
//    auto ret2 = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, len);
//    if (ret2 < 0) return ret2;
//    return ret + ret2;
//#elif defined(ARDUINO_RASPBERRY_PI_PICO)
//
//    while (!usb_hid.ready())
//    {
//        tight_loop_contents();
//    }
//    bool b = usb_hid.sendReport(id, data, len);
//    return b;
//#elif defined(ARDUINO_NRF52_ADAFRUIT)

    if (TinyUSBDevice.suspended())
    {
        TinyUSBDevice.remoteWakeup();
    }
    else if (TinyUSBDevice.mounted())
    {
        if(id==HID_REPORTID_MOUSE)
        {
            usb_hid.sendReport(id, (uint8_t *const)data, len);
            return 1;
        }
        mutex_enter_blocking(&reportMutex);
        NextReport nextReport{id, static_cast<uint16_t>(len)};
        tu_fifo_write_n(&tx_ff_hid, &nextReport, (uint16_t)(sizeof(nextReport)));
        tu_fifo_write_n(&tx_ff_hid, data, (uint16_t)len);
        mutex_exit(&reportMutex);
    }

    return 1;
//#else
//
//#error "Unsupported architecture"
//
//#endif
}

bool HID_::SendLastReport()
{
    bool success = true;
    mutex_enter_blocking(&reportMutex);
    if (tu_fifo_count(&tx_ff_hid) != 0)
    {
        struct
        {
            NextReport nextReport;
            uint8_t dataReport[256];
        } nextReportWithData;
        tu_fifo_peek_n(&tx_ff_hid, &nextReportWithData.nextReport, (uint16_t)(sizeof(nextReportWithData.nextReport)));
        tu_fifo_peek_n(&tx_ff_hid, &nextReportWithData, (uint16_t)(sizeof(nextReportWithData.nextReport)) + nextReportWithData.nextReport.len);

        success = usb_hid.sendReport(nextReportWithData.nextReport.id, nextReportWithData.dataReport, nextReportWithData.nextReport.len);

        if (success || TinyUSBDevice.suspended())
        {
            tu_fifo_advance_read_pointer(&tx_ff_hid, (uint16_t)(sizeof(nextReportWithData.nextReport)) + nextReportWithData.nextReport.len);
        }
    }
    mutex_exit(&reportMutex);
    return success;
}

HID_::HID_() : protocol(HID_REPORT_PROTOCOL), idle(0)
{
    setReportData.reportId = 0;
    setReportData.leds = 0;
}

int HID_::begin()
{
    const uint8_t * p_descriptor;
    uint32_t descriptor_len;

    /* Initialize the mutex */
    mutex_init(&reportMutex);

    /* Set the USB HID report descriptor */
    hid_report_descriptor_usb_get( &p_descriptor, &descriptor_len );

    usb_hid.setPollInterval(1);
    usb_hid.setReportDescriptor(p_descriptor, descriptor_len);
    usb_hid.setBootProtocol(0);
    usb_hid.begin();
    tu_fifo_config(&tx_ff_hid, tx_ff_buf_hid, TU_ARRAY_SIZE(tx_ff_buf_hid), 1, true);

    return 0;
}
