
#ifndef _COMMUNICATIONS_H_
#define _COMMUNICATIONS_H_

#include "KeyScanner_communications_protocol.h"
#include "Callback.h"
using namespace KeyScanner_communications_protocol;

class Communications {
 public:
  void init();

  void run();

  bool sendPacket(Packet data);

  BindingCallbacks<Commands, Packet> callbacks{};
  Callback<Devices> active{};

 private:
  constexpr static uint32_t timeout = 200;
  struct SideInfo {
    SideInfo(Devices _devices)
      : device(_devices) {}
    Devices device;
    bool online{false};
    uint32_t lastCommunication = 2000;
    bool port{false};
  };

  SideInfo left{KeyScanner_communications_protocol::KEYSCANNER_DEFY_LEFT};
  SideInfo right{KeyScanner_communications_protocol::KEYSCANNER_DEFY_RIGHT};

  void checkActive(SideInfo &side);
};

extern Communications Communications;

#endif  //_COMMUNICATIONS_H_
