#ifdef ARDUINO_ARCH_RP2040

#include "Communications.h"
#include "SPISlave.h"

void Communications::init() {
  port0.init();
  port1.init();

  callbacks.bind(IS_ALIVE, [this](Packet p) {
    if (p.header.device == KEYSCANNER_DEFY_RIGHT) {
      right.lastCommunication = millis();
      if (port0.device == KEYSCANNER_DEFY_RIGHT) right.port = false;
      if (port1.device == KEYSCANNER_DEFY_RIGHT) right.port = true;
    }
    if (p.header.device == KEYSCANNER_DEFY_LEFT) {
      left.lastCommunication = millis();
      if (port0.device == KEYSCANNER_DEFY_LEFT) left.port = false;
      if (port1.device == KEYSCANNER_DEFY_LEFT) left.port = true;
    }
  });
  callbacks.bind(HAS_KEYS, [this](Packet p) {
    if (p.header.device == KEYSCANNER_DEFY_RIGHT) {
      right.lastCommunication = millis();
    }
    if (p.header.device == KEYSCANNER_DEFY_LEFT) {
      left.lastCommunication = millis();
    }
  });
}

void Communications::run() {
  checkActive(left);
  checkActive(right);

  if (!queue_is_empty(&port0.rx_messages_)) {
    Packet packet;
    queue_remove_blocking(&port0.rx_messages_, &packet);
    callbacks.call(packet.header.command, packet);
  }

  if (!queue_is_empty(&port1.rx_messages_)) {
    Packet packet;
    queue_remove_blocking(&port1.rx_messages_, &packet);
    callbacks.call(packet.header.command, packet);
  }
}

bool Communications::sendPacket(Packet packet) {
  //TODO: Check that it is online
  Devices device_to_send = packet.header.device;
  packet.header.device   = KeyScanner_communications_protocol::NEURON_DEFY_WIRED;
  if (device_to_send == KeyScanner_communications_protocol::UNKNOWN) {
    if (port0.device != UNKNOWN)
      queue_add_blocking(&port0.tx_messages_, &packet);
    if (port1.device != UNKNOWN)
      queue_add_blocking(&port1.tx_messages_, &packet);
  }
  if (device_to_send == KeyScanner_communications_protocol::KEYSCANNER_DEFY_LEFT) {
    if (left.port) {
      if (port1.device != UNKNOWN)
        queue_add_blocking(&port1.tx_messages_, &packet);
    } else {
      if (port0.device != UNKNOWN)
        queue_add_blocking(&port0.tx_messages_, &packet);
    }
  }
  if (device_to_send == KeyScanner_communications_protocol::KEYSCANNER_DEFY_RIGHT) {
    if (right.port) {
      if (port1.device != UNKNOWN)
        queue_add_blocking(&port1.tx_messages_, &packet);
    } else {
      if (port0.device != UNKNOWN)
        queue_add_blocking(&port0.tx_messages_, &packet);
    }
  }
  return true;
}

void Communications::checkActive(Communications::SideInfo &side) {
  const bool now_active = millis() - side.lastCommunication <= timeout;

  //If it was active and there and now it no longer active then notify the chanel
  if (side.online && !now_active) {
    side.online   = now_active;
    SPISlave &spi = side.port ? port1 : port0;
    spi.device    = KeyScanner_communications_protocol::UNKNOWN;
    //Clear the packets as now the channel is no longer active
    Packet packet;
    while (!queue_is_empty(&spi.rx_messages_)) {
      queue_remove_blocking(&spi.rx_messages_, &packet);
    }
    while (!queue_is_empty(&spi.tx_messages_)) {
      queue_remove_blocking(&spi.tx_messages_, &packet);
    }
  }

  //If it was not active and now is active then notify the chanel that now is active
  if (!side.online && now_active) {
    side.online = now_active;
    active(side.device);
  }
}


class Communications Communications;

#endif