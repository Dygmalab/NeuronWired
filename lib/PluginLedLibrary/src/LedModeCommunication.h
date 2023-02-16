
#ifndef _LEDMODECOMMUNICATION_H_
#define _LEDMODECOMMUNICATION_H_
#include "Side_communications_protocol.h"

class LedModeCommunication {
 protected:
  static void sendLedMode(LedModeSerializable &led_mode_serializable) {
    Side_communications_protocol::Packet packet;
    packet.context.command = Side_communications_protocol::SET_MODE_LED;
    packet.context.size    = led_mode_serializable.serialize(packet.data);
    kaleidoscope::Runtime.device().sendPacket(packet);
  }
};
#endif  //_LEDMODECOMMUNICATION_H_
