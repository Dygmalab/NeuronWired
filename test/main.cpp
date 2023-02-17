#include <Arduino.h>
#include <Wire.h>
#include "Kaleidoscope.h"
#include "unity.h"
#include "LEDEffect-SolidColor-Defy.h"
#include "SPISlave.h"

static kaleidoscope::plugin::LEDSolidColorDefy solidRedDefy(0, 0, 0, 0);

KALEIDOSCOPE_INIT_PLUGINS(LEDControl)


void userVerification() {
  sleep_ms(1000);
}

void sendPacket(Packet &message) {
  port1.sendPacket(&message);
  port0.sendPacket(&message);
}

void sendColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  Packet message{};
  message.header.command  = SET_MODE_LED;
  solidRedDefy.led_mode.r_ = r;
  solidRedDefy.led_mode.g_ = g;
  solidRedDefy.led_mode.b_ = b;
  solidRedDefy.led_mode.w_ = w;
  message.header.size     = solidRedDefy.led_mode.serialize(message.data);
  sendPacket(message);
}

void sendColorRed() {
  TEST_MESSAGE("Setting Keyboard COLOR RED");
  sendColor(255, 0, 0, 0);
  userVerification();
}

void sendColorGreen() {
  TEST_MESSAGE("Setting Keyboard COLOR Green");
  sendColor(0, 255, 0, 0);
  userVerification();
}

void sendColorBlue() {
  TEST_MESSAGE("Setting Keyboard COLOR Blue");
  sendColor(0, 0, 255, 0);
  userVerification();
}

void sendColorWhite() {
  TEST_MESSAGE("Setting Keyboard COLOR White");
  sendColor(0, 0, 0, 255);
  userVerification();
}

void spiSidesAreOnline() {
  uint8_t keys[6];
  uint8_t online = port1.readFrom(keys, sizeof(keys));
  TEST_ASSERT_MESSAGE(online != 0, "SPI1 is not online, maybe the cable is not connected?");
  online = port0.readFrom(keys, sizeof(keys));
  TEST_ASSERT_MESSAGE(online != 0, "SPI0 is not online, maybe the cable is not connected?");
}

int runUnityTests() {
  UNITY_BEGIN();
  RUN_TEST(spiSidesAreOnline);
  RUN_TEST(sendColorRed);
  RUN_TEST(sendColorGreen);
  RUN_TEST(sendColorBlue);
  RUN_TEST(sendColorWhite);
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
  kaleidoscope::Runtime.device().side.resetRight();
  kaleidoscope::Runtime.device().side.resetLeft();
  sleep_ms(1000);
  Serial.println("Start testing!");
  runUnityTests();
}

void loop() {

}

void setup1() {
  port1.init();
  port0.init();
};
