
#include <Arduino.h>
#include <RP2040USB.h>
#include "unity.h"
#include <Adafruit_TinyUSB.h>

#define RGBW_LED_RED    6  // SWe 20220719: RED RGBW led OUT, PWM3 A can be used to control its intensity
#define RGBW_LED_GREEN  0  // SWe 20220719: GREEN RGBW led OUT, PWM0 A can be used to control its intensity
#define RGBW_LED_BLUE   2  // SWe 20220719: BLUE RGBW led OUT, PWM1 A can be used to control its intensity
#define RGBW_LED_WHITE  4  // SWe 20220719: WHITE RGBW led OUT, PWM2 A can be used to control its intensity

// Report ID
enum {
  RID_KEYBOARD = 1,
  RID_MOUSE,
  RID_CONSUMER_CONTROL, // Media, volume etc ..
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] =
	{
		TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
		TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(RID_MOUSE)),
		TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))
	};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);


void test_led(uint8_t pin_led) {
  pinMode(pin_led, OUTPUT);
  analogWrite(pin_led, 255 << 8);
//  const String &input = Serial.readStringUntil('\n');
//  Serial.print(input);
//  TEST_ASSERT_EQUAL_STRING("y", input.c_str());
  analogWrite(pin_led, 0 << 8);
}

void test_red(void) {
  test_led(RGBW_LED_RED);
}

void test_green(void) {
  test_led(RGBW_LED_GREEN);
}

void test_blue(void) {
  test_led(RGBW_LED_BLUE);
}

void test_white(void) {
  test_led(RGBW_LED_WHITE);
}

void test_mouse(void) {
  TEST_ASSERT_MESSAGE(usb_hid.ready(), "HID is not available");

  int8_t const delta = 5;
  usb_hid.mouseMove(RID_MOUSE, delta, delta); // right + down
  // delay a bit before attempt to send keyboard report
  delay(10);
}

void test_keyboard(void) {
  TEST_ASSERT_MESSAGE(usb_hid.ready(), "HID is not available");

  // use to send key release report
  static bool has_key = false;

  uint8_t keycode[6] = {0};
  keycode[0] = HID_KEY_A;

  usb_hid.keyboardReport(RID_KEYBOARD, 0, keycode);

  delay(100);
  has_key = true;
  // send empty key report if previously has key pressed
  if (has_key)
	usb_hid.keyboardRelease(RID_KEYBOARD);
  has_key = false;

  // delay a bit before attempt to send consumer report
  delay(10);
}

void test_consumer(void) {
  TEST_ASSERT_MESSAGE(usb_hid.ready(), "HID is not available");
  // Consumer Control is used to control Media playback, Volume, Brightness etc ...
  // Consumer report is 2-byte containing the control code of the key
  // For list of control check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h

  // use to send consumer release report
  static bool has_consumer_key = false;

  // send volume down (0x00EA)
  usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_VOLUME_DECREMENT);
  has_consumer_key = true;
  delay(100);

  // release the consume key by sending zero (0x0000)
  if (has_consumer_key)
	usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_red);
  RUN_TEST(test_green);
  RUN_TEST(test_blue);
  RUN_TEST(test_white);
  RUN_TEST(test_mouse);
  RUN_TEST(test_keyboard);
  RUN_TEST(test_consumer);
  return UNITY_END();
}

/**
  * For Arduino framework
  */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  Serial.begin(115200);
  Serial.setTimeout(INT32_MAX);
  usb_hid.begin();
  // wait until device mounted
  while (!TinyUSBDevice.mounted())
	delay(1);
  Serial.println("Start testing!");

  runUnityTests();

}
void loop() {
}
