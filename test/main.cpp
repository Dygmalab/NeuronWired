#include <Arduino.h>
#include <Wire.h>
#include <arduino/Adafruit_USBD_CDC.h>
#include "Kaleidoscope.h"
#include "Communications.h"
#include "SPISlave.h"
#include "LEDEffect-SolidColor-Defy.h"
static kaleidoscope::plugin::LEDSolidColorDefy solidRedDefy(0, 0, 0, 0);

KALEIDOSCOPE_INIT_PLUGINS(LEDControl)

uint8_t finishCallbacks = 0;

void userVerification() {
  sleep_ms(1000);
}

void sendPacket(Packet &message) {
  Communications.sendPacket(message);
}

void sendColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  Packet message{};
  message.header.command   = SET_MODE_LED;
  solidRedDefy.led_mode.r_ = r;
  solidRedDefy.led_mode.g_ = g;
  solidRedDefy.led_mode.b_ = b;
  solidRedDefy.led_mode.w_ = w;
  message.header.size      = solidRedDefy.led_mode.serialize(message.data);
  sendPacket(message);
}

void sendColorRed() {
  Serial.println("Setting Keyboard COLOR RED");
  sendColor(255, 0, 0, 0);
  userVerification();
}

void sendColorGreen() {
  Serial.println("Setting Keyboard COLOR Green");
  sendColor(0, 255, 0, 0);
  userVerification();
}

void sendColorBlue() {
  Serial.println("Setting Keyboard COLOR Blue");
  sendColor(0, 0, 255, 0);
  userVerification();
}

void sendColorWhite() {
  Serial.println("Setting Keyboard COLOR White");
  sendColor(0, 0, 0, 255);
  userVerification();
}

void sendColorBlack() {
  Serial.println("Setting Keyboard COLOR White");
  sendColor(0, 0, 0, 0);
  userVerification();
}

void spiSidesAreOnline() {
  uint8_t keys[6];
  if (port0.device == Communications_protocol::UNKNOWN) {
    Serial.println("SPI0 is not online, maybe the cable is not connected?");
  }
  if (port1.device == Communications_protocol::UNKNOWN) {
    Serial.println("SPI1 is not online, maybe the cable is not connected?");
  }
}

void sendKeys() {
  Packet packet{};
  packet.header.command = Communications_protocol::HAS_KEYS;
  packet.header.size    = 5;
  packet.data[0]        = 2;
  packet.data[1]        = 0;
  packet.data[2]        = 0;
  packet.data[3]        = 0;
  packet.data[4]        = 0;
  sendPacket(packet);
  Communications.callbacks.bind(HAS_KEYS, [](Packet p) {
    Serial.println("Has Key recibed");
    finishCallbacks--;
  });
}

constexpr static uint8_t open_leds[33]{0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

constexpr static uint8_t short_leds[33]{0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x3F, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void getOpenLeds() {
  Packet packet{};
  packet.header.command = Communications_protocol::GET_OPEN_LED;
  packet.header.size    = 0;
  sendPacket(packet);
  finishCallbacks = 2;
  Communications.callbacks.bind(GET_OPEN_LED, [](Packet p) {
    finishCallbacks--;
    bool b = memcmp(open_leds, p.data, p.header.size);
    if (p.header.device == Communications_protocol::KEYSCANNER_DEFY_LEFT) {
      Serial.println("TESTING OPEN LED LEFT SIDE");
      if (b != 0) Serial.println("There is a problem with an open led");
    }
    if (p.header.device == Communications_protocol::KEYSCANNER_DEFY_LEFT) {
      Serial.println("TESTING OPEN LED RIGHT SIDE");
      if (b != 0) Serial.println("There is a problem with an open led");
    }
  });
}

void getShortLeds() {
  Packet packet{};
  packet.header.command = Communications_protocol::GET_SHORT_LED;
  packet.header.size    = 0;
  sendPacket(packet);
  finishCallbacks++;
  Communications.callbacks.bind(GET_SHORT_LED, [](Packet p) {
    bool b = memcmp(short_leds, p.data, p.header.size);
    if (p.header.device == Communications_protocol::KEYSCANNER_DEFY_LEFT) {
      Serial.println("TESTING SHORT LED LEFT SIDE");
      if (b != 0) Serial.println("There is a problem with an open led");
    }
    if (p.header.device == Communications_protocol::KEYSCANNER_DEFY_LEFT) {
      Serial.println("TESTING SHORT LED RIGHT SIDE");
      if (b != 0) Serial.println("There is a problem with an open led");
    }
  });
}

void runUnityTests() {
  spiSidesAreOnline();
  Serial.flush();
  sendColorRed();
  Serial.flush();
  sendColorGreen();
  Serial.flush();
  sendColorBlue();
  Serial.flush();
  sendColorWhite();
  Serial.flush();
  sendColorBlack();
  Serial.flush();
  getOpenLeds();
  Serial.flush();
  getShortLeds();
  Serial.flush();
}

/**
  * For Arduino framework
  */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  Serial.begin(115200);
  Serial.setTimeout(INT32_MAX);
  sleep_ms(1000);
  Serial.println("Start testing!");
}

void loop() {
  static bool once = false;
  if (!once) {
    runUnityTests();
    once = true;
  }
}

void loop1() {
  Communications.run();
}

void setup1() {
  Communications.init();
};
