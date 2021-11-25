#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;
  int seq;
  bool can_send;
  struct measurement measure;
  //Очереди mgmt
  Mgmt::Queue_job*                  mgmt_job;
  Mgmt::Queue_report*            mgmt_report;

  Global_setup::Queue_toSet*       setup_set;
  Global_setup::Queue_toSave*     setup_save;

  Packetizer::Queue_prx*        packetizer_rx;
  Packetizer::Queue_ptx*        packetizer_tx;
  Packetizer::Queue_psent*    packetizer_sent;


  CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup),can_send(false) , seq(1), mgmt_job(), mgmt_report(), packetizer_tx(), packetizer_rx(), packetizer_sent()
  {
    module_debug = "CORE";
    flags |= evaluate_every_cycle;
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
      enable_wait(packetizer_tx);
      waitSystem->enable_wait(this, &timer);
      waitSystem->start_timer(&timer, 5000000000ULL);
  }

  void attach_mgmt(Mgmt::Queue_job* job, Mgmt::Queue_report* report){
    disable_wait(mgmt_job); disable_wait(mgmt_report);
    mgmt_job = job; mgmt_report = report;
    enable_wait(mgmt_job);
  }


  void evaluate() {
      while (WaitSystem::Queue* queue = enum_ready_queues())
            if (queue==mgmt_job) {
                packetizer_rx->packet = mgmt_job->packet;
                mgmt_job->clear();
                packetizer_rx->setReady();
                print("Accepted convertable values");
            }
            else if(queue == packetizer_tx){
                can_send = true;
                print("I can send");
                disable_wait(packetizer_tx);
            }
            else if(queue == &timer && can_send){
                packetizer_tx->send(seq);
                seq++;
                packetizer_tx->setReady();
                can_send = false;
            }
        }
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

