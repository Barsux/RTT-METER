#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;
  int seq, stuck;
  struct pckt packet;
  struct measurement msmt[60000];
  bool can_send, have_settings;
  //Очереди mgmt
  Mgmt::Queue_job*                  mgmt_job;
  Mgmt::Queue_report*            mgmt_report;

  Global_setup::Queue_toSet*       setup_set;
  Global_setup::Queue_toSave*     setup_save;

  Packetizer::Queue_prx*        packetizer_rx;
  Packetizer::Queue_ptx*        packetizer_tx;
  Packetizer::Queue_psent*    packetizer_sent;


  CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup),can_send(false), have_settings(false), seq(1), stuck(0), mgmt_job(), mgmt_report(), packetizer_tx(), packetizer_rx(), packetizer_sent()
  {
    module_debug = "CORE";
    flags |= evaluate_every_cycle;
  }
  WaitSystem::Queue timer;
  WaitSystem::Queue work;


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
      U64 work_duration = packet.duration * 1000000000ULL;
      U64 packet_duration = 1000000000ULL / packet.pckt_per_s;
      waitSystem->enable_wait(this, &timer);
      waitSystem->enable_wait(this, &work);
      waitSystem->start_timer(&timer, packet_duration);
      waitSystem->start_timer(&work, work_duration);
  }

  void evaluate() {
      while (WaitSystem::Queue* queue = enum_ready_queues()){
            if(queue == &work) {
                have_settings = false;
                work.clear();
                I4 packet_loss = 0;
                I64 rtt_avg = 0, rtt_max = 0;
                for(int i = 0; i < packet.amount; i++){
                    I4 delay = msmt[i].incoming_message - msmt[i].upcoming_message;
                    if(delay > 0){
                        rtt_avg += delay;
                        if(delay > rtt_max) rtt_max = delay;
                    }
                    else{
                        packet_loss++;
                    }
                }
                char output[344];
                snprintf(output, 344, "      _______________________________________________________________________________\n"
                                                    "      Average RTT   Maximum RTT        Total loss packets     Percent of loss packets\n"
                                                           "%12.4fmS%12.4fmS%12d               %12.3f%%\n"
                                                    "      _______________________________________________________________________________\n",
                                                    (double)(rtt_avg / packet.amount) / 1000000, (double) rtt_max / 1000000, packet_loss, packet_loss/ packet.amount * 100);
                print("End measurement!");
                mgmt_report->report(output);
            }
            if(!have_settings){
                if (queue==mgmt_job) {

                    packetizer_rx->packet = mgmt_job->packet;
                    packet = mgmt_job->packet;
                    mgmt_job->clear();
                    packetizer_rx->setReady();
                    enable_wait(packetizer_rx); enable_wait(packetizer_sent); enable_wait(packetizer_tx);
                    have_settings = true;
                    bzero(msmt, packet.amount);
                    if(!packet.is_server) begin_work();
                    print("READY!");
                    print("Begin measurement!");
                }
            }
            else{
                if(queue == packetizer_tx) {
                    can_send = true;
                    packetizer_tx->clear();
                }
                if(packet.is_server){
                    if(queue == packetizer_rx){
                        int sequence; U64 ts;
                        int r = packetizer_rx->recv(sequence, ts);
                        if(r > 0){
                            char t[128]; utc2str(t, sizeof(t), ts);
                            seq = sequence;
                            printf("RECV L2 PACKET OF SEQUENCE = %d AT %s\n",sequence, t);
                            packetizer_tx->send(sequence);
                        }
                    }
                    else if(queue == packetizer_sent){
                        char t[128];
                        utc2str(t, sizeof(t), packetizer_sent->utc_sent);
                        printf("\nSENT L2 PACKET OF SEQUENCE = %d AT %s", packetizer_sent->sequence, t);
                        packetizer_sent->clear();
                    }
                }
                else{
                    if(queue == packetizer_rx){
                        int sequence; U64 ts;
                        int r = packetizer_rx->recv(sequence, ts);
                        if(r > 0 && msmt[sequence].incoming_message == 0) {
                            msmt[sequence].incoming_message = ts;
                            stuck--;
                        }
                    }
                    else if(queue == &timer && can_send){
                        timer.clear();
                        if(seq % packet.pckt_per_s == 0) {
                            if(stuck > packet.pckt_per_s / 2) {
                                print("Cannot reach host!");
                                exit(EXIT_FAILURE);
                            }
                            else {
                                print("%d seconds left.", packet.duration - seq / packet.pckt_per_s);
                            }
                        }
                        int status  = packetizer_tx->send(seq); if(status < 0);
                        seq++;
                    }
                    else if(queue == packetizer_sent){
                        stuck++;
                        U64 ts; int sq;
                        ts = packetizer_sent->utc_sent;
                        sq = packetizer_sent->sequence;
                        if(msmt[sq].upcoming_message == 0) msmt[sq].upcoming_message = ts;
                        packetizer_sent->clear();
                    }
                }
            }
        }
    }
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

