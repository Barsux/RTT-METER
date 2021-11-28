#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;
  int seq;
  struct pckt packet;
  bool can_send, have_settings;
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
    , setup(setup),can_send(false), have_settings(false) , seq(1), mgmt_job(), mgmt_report(), packetizer_tx(), packetizer_rx(), packetizer_sent()
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
  }

  void attach_mgmt(Mgmt::Queue_job* job, Mgmt::Queue_report* report){
    disable_wait(mgmt_job); disable_wait(mgmt_report);
    mgmt_job = job; mgmt_report = report;
    enable_wait(mgmt_job);
  }

  void begin_work(){
      waitSystem->enable_wait(this, &timer);
      waitSystem->start_timer(&timer, 1000000000ULL);
      enable_wait(packetizer_rx); enable_wait(packetizer_sent); enable_wait(packetizer_tx);
      have_settings = true;
  }

  void evaluate() {
      while (WaitSystem::Queue* queue = enum_ready_queues())
            if(!have_settings){
                if (queue==mgmt_job) {
                    packetizer_rx->packet = mgmt_job->packet;
                    packet = mgmt_job->packet;
                    mgmt_job->clear();
                    packetizer_rx->setReady();
                    begin_work();
                    print("Accepted convertable values");
                    print("READY!");
                }
            }
            else{
                if(queue == packetizer_tx){
                    can_send = true;
                }
                else if(!packet.is_server && queue == &timer){
                    packetizer_tx->send(seq);
                    seq++;
                    timer.clear();
                }
                else if(queue == packetizer_rx){
                    int sequence; U64 ts;
                    int r = packetizer_rx->recv(sequence, ts);
                    if(r > 0){
                        char t[128]; utc2str(t, sizeof(t), ts);
                        print("RECV L2 PACKET OF SEQUENCE = %d AT %s\n",sequence, t);
                    }
                }
                else if(queue == packetizer_sent){
                    char t[128];
                    utc2str(t, sizeof(t), packetizer_sent->utc_sent);
                    print("SENT L2 PACKET OF SEQUENCE = %d AT %s", seq, t);
                    packetizer_sent->clear();
                }
            }
        }
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

