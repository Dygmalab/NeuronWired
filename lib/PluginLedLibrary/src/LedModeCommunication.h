
#ifndef _LEDMODECOMMUNICATION_H_
#define _LEDMODECOMMUNICATION_H_

#include "KeyScanner_communications_protocol.h"
#include "Communications.h"

class LedModeCommunication {
 protected:
  static void sendLedMode(LedModeSerializable &led_mode_serializable) {
    KeyScanner_communications_protocol::Packet packet{};
    packet.header.command = KeyScanner_communications_protocol::SET_MODE_LED;
    packet.header.size    = led_mode_serializable.serialize(packet.data);
    Communications.sendPacket(packet);
  }
};
#endif  //_LEDMODECOMMUNICATION_H_
