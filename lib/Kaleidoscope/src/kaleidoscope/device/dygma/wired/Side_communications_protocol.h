#ifndef SIDE_COMMUNICATIONS_H_
#define SIDE_COMMUNICATIONS_H_

namespace Side_communications_protocol {

enum Devices: uint8_t {
  UNKNOWN = 0,
  KEYSCANNER_DEFY_LEFT,
  KEYSCANNER_DEFY_RIGHT,
  NEURON_DEFY_WIRED,
  NEURON_DEFY_WIRELESS,
};

static_assert(sizeof(Devices)==sizeof(uint8_t));

enum Commands: uint8_t {
  IS_DEAD = 0,
  IS_ALIVE,
  GET_VERSION,
  SET_ALIVE_INTERVAL,
  //Keys
  HAS_KEYS = 10,
  SET_KEYSCAN_INTERVAL,
  //LEDS
  SET_BRIGHTNESS=20,
  SET_MODE_LED,
  SET_LED,
  SET_LED_BANK,
  SET_PALETTE_COLORS,
  SET_LAYER_KEYMAP_COLORS,
  SET_LAYER_UNDERGLOW_COLORS,
};
static_assert(sizeof(Commands)==sizeof(uint8_t));

struct Context {
  Commands command;
  Devices device;
  uint8_t messageSize;
};
static_assert(sizeof(Context)==(sizeof(uint8_t)*3));

static const constexpr uint8_t MAX_TRANSFER_SIZE = 128;
union Packet{
  struct{
	Context context;
	uint8_t data[MAX_TRANSFER_SIZE-sizeof(Context)];
  };
  uint8_t buf[MAX_TRANSFER_SIZE];
};
static_assert(sizeof(Packet)==MAX_TRANSFER_SIZE);

class Default{
public:
  Default() {
	message.context.device = UNKNOWN;
	message.context.command = IS_DEAD;
	message.context.messageSize = 0;
  };

  explicit Default(Devices device, Commands cmd = IS_ALIVE) {
	message.context.device = device;
	message.context.command = cmd;
	message.context.messageSize = 0;
  };

  virtual uint8_t serialize() {
	memcpy(&message.context, message.buf, sizeof(Context));
	return sizeof(Context);
  };
  virtual uint8_t deSerialize() {
	memcpy(message.buf, &message.context, sizeof(Context));
	return sizeof(Context);
  };
  Packet message;
};

class KeyStrokes : public Default {
public:
  KeyStrokes(Devices device) : Default(device, HAS_KEYS) {
	message.context.messageSize = sizeof(keys);
  }
  uint8_t serialize() override {
	uint8_t lastIndex = Default::serialize();
	memcpy(&message.buf[lastIndex], keys, sizeof(keys));
	return sizeof(keys) + lastIndex;
  }
  uint8_t deSerialize() override {
	uint8_t lastIndex = Default::deSerialize();
	memcpy(keys, &message.buf[lastIndex], sizeof(keys));
	return sizeof(keys) + lastIndex;
  }
  uint8_t keys[5];
};

class Crc : public Default {
public:
  Crc(Devices device, Commands cmd = HAS_KEYS) : Default(device, cmd) {
	message.context.messageSize = sizeof(crc);
  }
  uint8_t serialize() override {
	uint8_t lastIndex = Default::serialize();
	memcpy(&message.buf[lastIndex], &crc, sizeof(crc));
	return sizeof(crc) + lastIndex;
  }
  uint8_t deSerialize() override {
	uint8_t lastIndex = Default::deSerialize();
	memcpy(&crc, &message.buf[lastIndex], sizeof(crc));
	return sizeof(crc) + lastIndex;
  }
  uint32_t crc;
};
}
#endif
