#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;

  //Очереди L2 [Портировать в pckt]
  L2Transport::Queue_rx*     l2_transport_rx;
  L2Transport::Queue_tx*     l2_transport_tx;
  L2Transport::Queue_sent* l2_transport_sent;

  //Очереди mgmt
  Mgmt::Queue_job*                  mgmt_job;
  Mgmt::Queue_report*            mgmt_report;

  Global_setup::Queue_toSet*       setup_set;
  Global_setup::Queue_toSave*     setup_save;

  Packetizer::Queue_prx*        packetizer_rx;
  Packetizer::Queue_ptx*        packetizer_tx;
  Packetizer::Queue_psent*    packetizer_sent;


  CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), l2_transport_rx(), l2_transport_tx(), mgmt_job(), mgmt_report(), packetizer_tx(), packetizer_rx(), packetizer_sent()
  {
    module_debug = "CORE";
  }
  WaitSystem::Queue timer;

  void attach_Global_setup(Global_setup::Queue_toSave* save, Global_setup::Queue_toSet* set){
    setup_set = set; setup_save = save;
    disable_wait(setup_set); disable_wait(setup_save);
    enable_wait(setup_set);
  }

  void attach_packetizer(Packetizer::Queue_prx* rx, Packetizer::Queue_ptx* tx, Packetizer::Queue_psent* sent){
    disable_wait(packetizer_tx); disable_wait(packetizer_rx); disable_wait(packetizer_sent);
    packetizer_tx = tx;
    packetizer_rx = rx;
    packetizer_sent = sent;
    enable_wait(packetizer_rx); enable_wait(packetizer_sent);
  }

  void attach_mgmt(Mgmt::Queue_job* job, Mgmt::Queue_report* report){
    disable_wait(mgmt_job); disable_wait(mgmt_report);
    mgmt_job = job; mgmt_report = report;
    enable_wait(mgmt_job);
  }

  void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
    disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
    l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
    enable_wait(l2_transport_rx);
    enable_wait(l2_transport_sent);
    waitSystem->enable_wait(this, &timer);
    waitSystem->start_timer(&timer, 500000000ULL);
  }
  void evaluate() {
    while (WaitSystem::Queue* queue = enum_ready_queues())
    if (queue==&timer) {
      print("TIMER");
      timer.clear();
      enable_wait(l2_transport_tx);
    }
    else if (queue==l2_transport_rx) {
      U8 packet[2048]; int cbPacket; U64 utc_rx;
      while (true) {
        cbPacket = l2_transport_rx->recv(utc_rx, packet, sizeof(packet)); if (cbPacket<=0) break;
        char t[128]; utc2str(t, sizeof(t), utc_rx);
        print("RECV L2 PACKET  AT %s => %i B", t, cbPacket);
      }
      l2_transport_rx->clear();
    }
    else if (queue==l2_transport_tx) {
      U8 p[1024]; memset(p, 0, sizeof(p));
      MAC &dst = *(MAC*)&p[0]; MAC &src = *(MAC*)&p[6]; U16 &pt_BE = *(U16*)&p[12];
      str2mac(dst, "60:45:cb:9b:cd:4e");
      str2mac(src, "11:22:33:44:55:66");
      pt_BE = htons(0xaabb);
      int cb = l2_transport_tx->send(p, sizeof(p));
      if (cb>0) {
        print("SEND L2 PACKET => %i B", cb);
        l2_transport_tx->clear();
        disable_wait(l2_transport_tx);
      }
    }
    else if (queue==l2_transport_sent) {
      char t[128]; utc2str(t, sizeof(t), l2_transport_sent->utc_sent);
      print("L2 PACKET: SENT AT %s", t);
      l2_transport_sent->clear();
    }
    else if (queue==mgmt_job){
        print("Accepted convertable values!");
        mgmt_job->clear();
    }
  }
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

