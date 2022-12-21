#ifndef SPICOMMUNICATIONS_H_
#define SPICOMMUNICATIONS_H_

namespace Communications {

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
  HAS_KEYS,
  SET_MODE_LED,
  SYNC_MODE_LED,
  UPDATE_LED_BANK,
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
union Message{
  struct{
	Context context;
	uint8_t data[MAX_TRANSFER_SIZE-sizeof(Context)];
  };
  uint8_t buf[MAX_TRANSFER_SIZE];
};
static_assert(sizeof(Message)==MAX_TRANSFER_SIZE);

class DefaultMessage{
public:
  DefaultMessage() {
	message.context.device = UNKNOWN;
	message.context.command = IS_DEAD;
	message.context.messageSize = 0;
  };

  explicit DefaultMessage(Devices device, Commands cmd = IS_ALIVE) {
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
  Message message;
};

class MessageKeyStrokes : public DefaultMessage {
public:
  MessageKeyStrokes(Devices device, Commands cmd = HAS_KEYS) : DefaultMessage(device, cmd) {
	message.context.messageSize = sizeof(keys);
  }
  uint8_t serialize() override {
	uint8_t lastIndex = DefaultMessage::serialize();
	memcpy(&message.buf[lastIndex], keys, sizeof(keys));
	return sizeof(keys) + lastIndex;
  }
  uint8_t deSerialize() override {
	uint8_t lastIndex = DefaultMessage::deSerialize();
	memcpy(keys, &message.buf[lastIndex], sizeof(keys));
	return sizeof(keys) + lastIndex;
  }
  uint8_t keys[5];
};

class MessageCrc : public DefaultMessage {
public:
  MessageCrc(Devices device, Commands cmd = HAS_KEYS) : DefaultMessage(device, cmd) {
	message.context.messageSize = sizeof(crc);
  }
  uint8_t serialize() override {
	uint8_t lastIndex = DefaultMessage::serialize();
	memcpy(&message.buf[lastIndex], &crc, sizeof(crc));
	return sizeof(crc) + lastIndex;
  }
  uint8_t deSerialize() override {
	uint8_t lastIndex = DefaultMessage::deSerialize();
	memcpy(&crc, &message.buf[lastIndex], sizeof(crc));
	return sizeof(crc) + lastIndex;
  }
  uint32_t crc;
};
}
#endif
