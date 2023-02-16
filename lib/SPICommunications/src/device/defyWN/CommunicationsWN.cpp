#ifdef ARDUINO_ARCH_RP2040

#include "Communications.h"

namespace kaleidoscope::plugin {

void Communications::init() {
}
void Communications::run() {
}
kaleidoscope::EventHandlerResult Communications::onSetup() {
  return EventHandlerResult::OK;
}
kaleidoscope::EventHandlerResult Communications::beforeReportingState() {
  run();
  return EventHandlerResult::OK;
}

bool Communications::sendPacket(const KeyScanner_communications_protocol::Packet &packet) {
  return false;
}

}  // namespace kaleidoscope::plugin

kaleidoscope::plugin::Communications Communications;

#endif