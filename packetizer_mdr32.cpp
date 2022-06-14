#include "packetizer_mdr32.h"


class PacketizerObject: public WaitSystem::Module, public Packetizer {
	Packetizer::Setup &setup;
public:
    struct pckt packet;
    U8 ready_packet[2048];
    bool have_ready_packet;
    bool setted;
    bool have_values;
		bool inited;
		
    I4 total;
    L2Transport::Queue_rx*		l2_transport_rx;
    L2Transport::Queue_tx*		l2_transport_tx;
    L2Transport::Queue_sent*	l2_transport_sent;

    class Rx: public Queue_prx {public:
        PacketizerObject &base;
        Rx(PacketizerObject &base): base(base){}
        int recv_rtt(int &seq, TsNs &tstmp){
            return base.recv_rtt(seq, tstmp);
        }
		int recv_ott(int &seq, TsNs &tstmp){
			return base.recv_ott(seq, tstmp);
		}
    }prx;
    class Tx: public Queue_ptx {public:
        PacketizerObject &base;
        Tx(PacketizerObject &base): base(base){}
        int send_rtt(int seq){
            return base.send_rtt(seq);
        }
		int send_ott(int seq, TsNs &send_ts){
			return base.send_ott(seq, send_ts);
		}
    }ptx;
    class Sent: public Queue_psent {public:
        PacketizerObject &base;
        Sent(PacketizerObject &base): base(base){}

    }psent;

    U2 IPCHECK(U2 *addr, U4 count) {
        register U32 sum = 0;
        while (count > 1) {
            sum += * addr++;
            count -= 2;
        }
        if(count > 0) sum += ((*addr)&htons(0xFF00));
        while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);
        sum = ~sum;
        return ((U2)sum);
    }

    U2 CRC8(U8 *pcBlock, U2 len)
    {
        U2 crc = 0xFF;
        while (len--)
            crc = CRC8TABLE[crc ^ *pcBlock++];
        return crc;
    }

    void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
        disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
        l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
        enable_wait(l2_transport_rx);
        enable_wait(l2_transport_sent);
        enable_wait(l2_transport_tx);
    }


    PacketizerObject(WaitSystem* waitSystem, Packetizer::Setup &setup): WaitSystem::Module(waitSystem)
            , setup(setup), setted(false), inited(false), have_values(false), prx(*this), ptx(*this), psent(*this), l2_transport_rx(), l2_transport_tx(), l2_transport_sent()
    {
		awaited=false;
        module_debug = "PACKETIZER";
        rx = &prx;
        tx = &ptx;
        sent = &psent;
        enable_wait(tx);
        enable_wait(rx);
        memset(ready_packet, 0, 2048);
        total = sizeof(struct ethheader) + sizeof(struct ipheader) + sizeof(struct udpheader);
        flags = (Flags)(flags | evaluate_every_cycle);
		
		have_values = false;
		packet.dstPORT = 5850;
		packet.srcPORT = 5850;
		packet.is_server = true;
		inited = true;
		str2ip4(packet.srcIP, setup.ip4);
		str2mac(packet.srcMAC, setup.srcMAC);
		/*
		packet.size = 256;
		char dstmac[] = "00:e0:4f:3e:02:27";
		str2mac(packet.dstMAC, dstmac);
		char dstip[] = "0.0.0.0";
		str2ip4(packet.dstIP, dstip);
		*/
		//create_rtt_packet();
		
		
    }
    void create_rtt_packet(){
        U8 buffer[packet.size + SPACER];
        memset(buffer, 0, packet.size + SPACER);
        struct ethheader *eth = (struct ethheader *)(buffer + SPACER);
        for(int i = 0; i < ETH_ALEN; i++){
            eth->h_source[i] = packet.srcMAC[i];
            eth->h_dest[i] = packet.dstMAC[i];
        }
        eth->h_proto = htons(0x0800);

        struct ipheader *iphdr = (struct ipheader*)(buffer + SPACER + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->version = 4;
        iphdr->tos=16;
        iphdr->id = htons(10241);
        iphdr->ttl = 64;
        iphdr->protocol = IPPROTO_UDP;
        iphdr->saddr = packet.srcIP;
        iphdr->daddr = packet.dstIP;
        iphdr->check = 0;

        struct udpheader *udp = (struct udpheader *)(buffer + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->source = htons(packet.srcPORT);
        udp->dest = htons(packet.dstPORT);
        udp->check = 0;

        udp->len = htons(packet.size - ETHHDR_LEN - IPHDR_LEN);
        iphdr->tot_len = htons(packet.size - ETHHDR_LEN);
        iphdr->check = IPCHECK((U2 *)iphdr, iphdr->ihl<<2);
        int i = SPACER + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader) + sizeof(rttheader);
		for (i; i < packet.size; i++){
            buffer[i] = i % 10 + 48;
        }
        for(int j = 0; j < packet.size; j++){
            ready_packet[j] = buffer[j];
        }
        have_ready_packet = true;
    }
	
	void create_ott_packet(){
		U8 buffer[packet.size + SPACER];
        memset(buffer, 0, packet.size + SPACER);
        struct ethheader *eth = (struct ethheader *)(buffer + SPACER);
        for(int i = 0; i < ETH_ALEN; i++){
            eth->h_source[i] = packet.srcMAC[i];
            eth->h_dest[i] = packet.dstMAC[i];
        }
        eth->h_proto = htons(0x0800);

        struct ipheader *iphdr = (struct ipheader*)(buffer + SPACER + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->version = 4;
        iphdr->tos=16;
        iphdr->id = htons(10241);
        iphdr->ttl = 64;
        iphdr->protocol = IPPROTO_UDP;
        iphdr->saddr = packet.srcIP;
        iphdr->daddr = packet.dstIP;
        iphdr->check = 0;

        struct udpheader *udp = (struct udpheader *)(buffer + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->source = htons(packet.srcPORT);
        udp->dest = htons(packet.dstPORT);
        udp->check = 0;

        udp->len = htons(packet.size - ETHHDR_LEN - IPHDR_LEN);
        iphdr->tot_len = htons(packet.size - ETHHDR_LEN);
        iphdr->check = IPCHECK((U2 *)iphdr, iphdr->ihl<<2);
        int i = SPACER + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader) + sizeof(rttheader) + sizeof(timeheader);
        for (i; i < packet.size; i++){
            buffer[i] = i % 10 + 48;
        }
        for(int j = 0; j < packet.size; j++){
            ready_packet[j] = buffer[j];
        }
        have_ready_packet = true;
		PRINT("Ott created packet");
	}

    int send_rtt(int seq){
        struct rttheader *rtt = (struct rttheader *)(ready_packet + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader));
        rtt->size = packet.size;
        rtt->sequence = seq;
        rtt->CRC = 0;
        rtt->CRC = CRC8(&((ready_packet + total)[SPACER]), packet.size - total);
		*(uint32_t *)&ready_packet[0] = packet.size; 
        short status = l2_transport_tx->send((U32*)ready_packet);
        return status>=0? status: -1;
    }
	
	int send_ott(int seq, TsNs &send_ts){
		if(!setted){
			setted = true;
			packet.size = 256;
			char dstmac[] = "00:e0:4f:3e:02:27";
			str2mac(packet.dstMAC, dstmac);
			char dstip[] = "0.0.0.0";
			str2ip4(packet.dstIP, dstip);
			create_ott_packet();
			PRINT("Setted!");
		}
		struct rttheader *rtt = (struct rttheader *)(ready_packet + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader));
        rtt->size = packet.size;
        rtt->sequence = seq;
        rtt->CRC = 0;
		struct timeheader *ttt = (struct timeheader *)(ready_packet + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader) + sizeof(rttheader));
		ttt->clock = SystemCoreClock;
		send_ts.renew();
		ttt->timestamp = send_ts.toU64();
		//PRINT("Core clock sended: %lu", ttt->clock);
		//PRINT("Timestamp sended: %llu", ttt->timestamp);
        rtt->CRC = CRC8(&((ready_packet + total)[SPACER]), packet.size - total);
		*(uint32_t *)&ready_packet[0] = packet.size; 
        short status = l2_transport_tx->send((U32*)ready_packet);
        return status>=0? status: -1;
	}
	
    int recv_rtt(int &seq, TsNs &tstmp){
        int MAXSIZE = 2048;
        int status;
        U8 buffer[MAXSIZE];

        status = l2_transport_rx->recv(buffer, tstmp, MAXSIZE); 
		if(status < 0) return -1;
        U16 ip_proto = htons(0x0800);
		
        struct ethheader *eth = (struct ethheader *)(buffer);
        if(eth->h_proto != ip_proto) return -1;
		if (memcmp(packet.srcMAC, eth->h_dest, ETH_ALEN) != 0) {
			return -1;
		}
        struct ipheader *ip = (struct ipheader *) (buffer + sizeof(struct ethheader));
        if(ip->protocol != IPPROTO_UDP) return -1;

		if(ip->daddr != packet.srcIP) return -1;
        struct udpheader *uh = (struct udpheader *)(buffer +sizeof(ethheader) + sizeof(ipheader));
        if(uh->source == htons(5850) || uh->dest == htons(5850)) return -1;
			
        struct rttheader *rtt = (struct rttheader *)(buffer + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader));
        seq = rtt->sequence;
        U2 inCRC = rtt->CRC;
        rtt->CRC = 0;
        U2 upCRC = CRC8(buffer + total, packet.size - total);
		
        if(!have_values && packet.is_server){
			have_values = true;
			packet.size = rtt->size;
			memcpy(packet.dstMAC, eth->h_source, ETH_ALEN);
			packet.dstIP = ip->saddr;
			if(!have_ready_packet)create_rtt_packet();
        }
        if(inCRC == upCRC)	return  1;
		else  				return -1;
	}
	
	int recv_ott(int &seq, TsNs &tstmp){
        int MAXSIZE = 2048;
        int status;
        U8 buffer[MAXSIZE];

        status = l2_transport_rx->recv(buffer, tstmp, MAXSIZE); 
		if(status < 0) return -1;
        U16 ip_proto = htons(0x0800);
		
        struct ethheader *eth = (struct ethheader *)(buffer);
        if(eth->h_proto != ip_proto) return -1;
		/*
		if (memcmp(packet.srcMAC, eth->h_dest, ETH_ALEN) != 0) {
			return -1;
		}*/
        struct ipheader *ip = (struct ipheader *) (buffer + sizeof(struct ethheader));
        if(ip->protocol != IPPROTO_UDP) return -1;

		//if(ip->daddr != packet.srcIP) return -1;
        struct udpheader *uh = (struct udpheader *)(buffer +sizeof(ethheader) + sizeof(ipheader));
        if(uh->source == htons(5850) || uh->dest == htons(5850)) return -1;
			
        struct rttheader *rtt = (struct rttheader *)(buffer + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader));
        seq = rtt->sequence;
        U2 inCRC = rtt->CRC;
        rtt->CRC = 0;
        U2 upCRC = CRC8(buffer + total, packet.size - total);
		
		struct timeheader *ttt = (struct timeheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader) + sizeof(rttheader));
		//PRINT("Core clock received: %lu", ttt->clock);
		//PRINT("Timestamp received: %llu", ttt->timestamp);

        if(!have_values && packet.is_server){
			have_values = true;
			packet.size = rtt->size;
			memcpy(packet.dstMAC, eth->h_source, ETH_ALEN);
			packet.dstIP = ip->saddr;
			if(!have_ready_packet)create_ott_packet();
        }
		
        if(inCRC == upCRC) return 1;
		else return -1;
	}

	void init(){}
	
	void check(){}

    void evaluate(){
		if(!inited) init();
		//tx->setReady();
        while (WaitSystem::Queue* queue = enum_ready_queues()){
			//L2 TX
			if(queue == l2_transport_tx){
				l2_transport_tx->clear();
				tx->setReady();
			}
			
			//L2 RX
            else if(queue == l2_transport_rx){
				l2_transport_rx->clear();
				rx->setReady();
            }
			
			//L2 SENT
            else if(queue == l2_transport_sent){
				l2_transport_sent->clear();
				sent->utc_sent = l2_transport_sent->utc_sent;
				sent->sequence = l2_transport_sent->sequence;
				sent->setReady();
			}
			
			//PACKET TX
			else if(queue == tx){
				tx->clear();
            }
		}
	}
};
Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup){
    return new PacketizerObject(waitSystem, setup);
}