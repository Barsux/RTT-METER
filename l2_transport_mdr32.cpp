#include "l2_transport_mdr32.h"


class L2Transport_mdr32: public WaitSystem::Module, public L2Transport {
	L2Transport::Setup &setup;
public:
	int sequence;
	bool inited;
	bool sended;
	class Rx: public Queue_rx {public:
		L2Transport_mdr32 &base;
		Rx(L2Transport_mdr32 &base): base(base) {
    }
		int recv(void * dst, TsNs &utc_rx , int maxsize) {
			return base.recv(dst, utc_rx, maxsize);
		}
	}queue_rx;
	class Tx: public Queue_tx {public:
		L2Transport_mdr32 &base;
		Tx(L2Transport_mdr32 &base): base(base) {
    }
		int send(U32 * buffer) {
			return base.send(buffer);
		}
	}queue_tx;
	Queue_sent queue_sent;
	
	L2Transport_mdr32(WaitSystem* waitSystem, L2Transport::Setup &setup): WaitSystem::Module(waitSystem)
		, inited(false), sended(false), setup(setup), queue_rx(*this), queue_tx(*this)
	{
		awaited = true;
		module_debug = "ETH";
		rx = &queue_rx; sent = &queue_sent;
		tx = &queue_tx; enable_wait(tx);
		flags = (Flags)(flags | evaluate_every_cycle);
		MAC mac;
		memset(mac, 0, 6);
		str2mac(mac, setup.srcMAC);
		eth_init(mac);
		inited = true;
	}
	
	void inits(){
		
	}
	
	int recv(void * dst, TsNs &utc_rx, int maxsize){
		return recvto((U32*)dst, utc_rx);
	}
	
	int send(void * buffer){
		sendto((U32*)buffer);
		sended = true;
		return 1;
	}
	void check(){		
		volatile U16 STAT = MDR_ETHERNET1->ETH_STAT;
		//5-7 биты указывают количество принятых, необработанных пакетов, в нашем случае не больше одного, значит можно брать только 5-ый бит.
		if((STAT >> 5) & 1){
			MDR_PORTD->RXTX ^= (1<<8);
			rx->setReady();
		}
		//8-й бит указывает пуст ли буффер передатчика 1-да 0-нет
		if((STAT >> 8) & 1){
			MDR_PORTD->RXTX ^= (1<<7);
			if(sended){
				sended = false;
				sent->utc_sent = TsNs();
				//sent->setReady();
			}
			tx->setReady();
		}
		
	}
	void evaluate(){
		if(!inited)inits();		
	}
};
L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup) {
	return new L2Transport_mdr32(waitSystem, setup);
}

