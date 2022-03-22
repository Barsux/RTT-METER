#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;
  L2Transport::Queue_rx*   l2_transport_rx;
  L2Transport::Queue_tx*   l2_transport_tx;
  L2Transport::Queue_sent* l2_transport_sent;
	volatile U16 counter = 0;

  CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), l2_transport_rx(), l2_transport_tx()
  {
    module_debug = "CORE";
		type = (Types)systtype;
		flags = (Flags)(flags | evaluate_every_cycle);
  }
  WaitSystem::Queue timer;

  void evaluate() {
		
    while (WaitSystem::Queue* queue = enum_ready_queues())
    if (queue==&timer) {
      print("TIMER");
      timer.clear();
      enable_wait(l2_transport_tx);
    }
		/*
		else
    if (queue==l2_transport_rx) {
      U8 packet[2048]; int cbPacket; U64 utc_rx;
      while (true) {
        cbPacket = l2_transport_rx->recv(utc_rx, packet, sizeof(packet)); if (cbPacket<=0) break;
        //char t[128]; utc2str(t, sizeof(t), utc_rx);
        //print("RECV L2 PACKET  AT %s => %i B", t, cbPacket);
      }
      l2_transport_rx->clear();
    } else
    if (queue==l2_transport_tx) {
      U8 p[1024]; memset(p, 0, sizeof(p));
      MAC &dst = *(MAC*)&p[0]; MAC &src = *(MAC*)&p[6]; U16 &pt_BE = *(U16*)&p[12];
      str2mac(dst, "60:45:cb:9b:cd:4e");
      str2mac(src, "11:22:33:44:55:66");
      pt_BE = HTONS(0xaabb);
      int cb = l2_transport_tx->send(p, sizeof(p));
      if (cb>0) {
        print("SEND L2 PACKET => %i B", cb);
        l2_transport_tx->clear();
        disable_wait(l2_transport_tx);
      }
    } else
    if (queue==l2_transport_sent) {
      char t[128]; utc2str(t, sizeof(t), l2_transport_sent->utc_sent);
      print("L2 PACKET: SENT AT %s", t);
      l2_transport_sent->clear();
    }
		*/
  }
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

