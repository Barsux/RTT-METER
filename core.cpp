#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
	Core::Setup &setup;
	OTT * measurements; int nmeasurements;
	Packetizer::Queue_prx*		packetizer_rx;
	Packetizer::Queue_ptx*		packetizer_tx;
	Packetizer::Queue_psent*    packetizer_sent;
	volatile U16 counter = 0;
	U16 pointer = 0;
	volatile I4 seq;
	bool can_send = false;
	CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
		,setup(setup), packetizer_tx(), packetizer_rx(), packetizer_sent(), nmeasurements(0)
	{
		awaited = false;
		module_debug = "CORE";
		flags = (Flags)(flags | evaluate_every_cycle);
	}
	WaitSystem::Queue timer;
	
	void attach_packetizer(Packetizer::Queue_prx* rx, Packetizer::Queue_ptx* tx, Packetizer::Queue_psent* sent){
		disable_wait(packetizer_tx); disable_wait(packetizer_rx); disable_wait(packetizer_sent);
		packetizer_tx = tx; 
		packetizer_rx = rx; 
		packetizer_sent = sent; 
		enable_wait(packetizer_rx);
		enable_wait(packetizer_tx);
		enable_wait(packetizer_sent);
		counter = 200;
	}
	void init(){}
	void check(){}
	void msmt_add_arr(OTT item) {
		measurements = (OTT*)realloc(measurements, sizeof(OTT*)*(nmeasurements+1));
		measurements[nmeasurements] = item; nmeasurements++;
	}
	void msmt_add_seq(int sequence, TsNs &recv_ts){
		for(int i = 0; i < nmeasurements; i++){
			if(measurements->seq == sequence){
				measurements[i].inc_ts = recv_ts;
			}				
		}
	}
	void evaluate() {
		while (WaitSystem::Queue* queue = enum_ready_queues()){
			//RTT
			/*
			if(queue == packetizer_tx) {
				packetizer_tx->clear();
				can_send = true;
			}
			else if(queue == packetizer_rx){
				packetizer_rx->clear();
				
				TsNs UTC = TsNs();
				I4 sequence;
				I2 r = packetizer_rx->recv_rtt(sequence, UTC);
				if(r > 0){
					seq = sequence;
					packetizer_tx->send_rtt(sequence);
				}
			}
			*/
			
			//OTT
			if(queue == packetizer_tx){
				packetizer_tx->clear();
				OTT send_msmt;
				send_msmt.seq = pointer;
				I2 r = packetizer_tx->send_ott(pointer, send_msmt.out_ts);
				if(r > 0){
					pointer++;
					//msmt_add_arr(send_msmt);
				}
			}
			else if(queue == packetizer_rx){
				packetizer_rx->clear();
				int seq; TsNs recv_ts;
				I2 r = packetizer_rx->recv_ott(seq, recv_ts);
				if(r > 0){
					msmt_add_seq(seq, recv_ts);
				}
			}
			
		}
	}
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

