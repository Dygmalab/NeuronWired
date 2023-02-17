
#ifndef _COMMUNICATIONS_H_
#define _COMMUNICATIONS_H_

#include "KeyScanner_communications_protocol.h"
#include "Callback.h"

class Communications{
 public:
  void init();

  void run();

  bool sendPacket(const KeyScanner_communications_protocol::Packet &data);

//  EventHandlerResult onSetup();
//  EventHandlerResult beforeReportingState();

  BindingCallbacks<KeyScanner_communications_protocol::Commands, KeyScanner_communications_protocol::Packet> callbacks_{};

 protected:
  std::vector<KeyScanner_communications_protocol::Packet> tx_packets_{};
  std::vector<KeyScanner_communications_protocol::Packet> rx_packets_{};
};
//}  // namespace plugin
//}  // namespace kaleidoscope

extern Communications Communications;


#endif  //_COMMUNICATIONS_H_
