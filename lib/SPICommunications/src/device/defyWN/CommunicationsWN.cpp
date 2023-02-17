#ifdef ARDUINO_ARCH_RP2040

#include <arduino/Adafruit_USBD_CDC.h>
#include "Communications.h"
#include "SPISlave.h"

//namespace kaleidoscope::plugin {

void Communications::init() {
  port0.init();
  port1.init();
}

void Communications::run() {
//  const bool now_active = millis() - lastTimeCommunication <= timeout;

//  //If it was active and there and now it no longer active then notify the chanel
//  if (active && !now_active) {
//    active = now_active;
//    //Clear the packets as now the channel is no longer active
//    Packet packet;
//    if (!queue_is_empty(&rx_messages_))
//      queue_remove_blocking(&rx_messages_, &packet);
//    if (!queue_is_empty(&tx_messages_))
//      queue_remove_blocking(&tx_messages_, &packet);
//    active_callback_(active);
//  }
//
//  //If it was not active and now is active then notify the chanel that now is active
//  if (!active && now_active) {
//    active = now_active;
//    active_callback_(active);
//  }

  if (!queue_is_empty(&port0.rx_messages_)) {
    Packet packet;
    queue_remove_blocking(&port0.rx_messages_, &packet);
    callbacks_.call(packet.header.command, packet);
  }

  if (!queue_is_empty(&port1.rx_messages_)) {
    Packet packet;
    queue_remove_blocking(&port1.rx_messages_, &packet);
    callbacks_.call(packet.header.command, packet);
  }


}

//kaleidoscope::EventHandlerResult Communications::onSetup() {
//  init();
//  return EventHandlerResult::OK;
//}
//
//kaleidoscope::EventHandlerResult Communications::beforeReportingState() {
//  run();
//  return EventHandlerResult::OK;
//}

bool Communications::sendPacket(const KeyScanner_communications_protocol::Packet &packet) {
  return false;
}

//}  // namespace kaleidoscope::plugin

class Communications Communications;

#endif