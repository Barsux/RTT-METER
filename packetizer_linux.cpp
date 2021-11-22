#include "packetizer_linux.h"

class PacketizerObject: public WaitSystem::Module, public Packetizer {
    Packetizer::Setup &setup;
public:
    bool setted;
    struct pckt packet;
    bool is_server;
    bool ready; // Пока не пойму как правильней это сделать //
    L2Transport::Queue_rx*     l2_transport_rx;
    L2Transport::Queue_tx*     l2_transport_tx;
    L2Transport::Queue_sent*  l2_transport_sent;

    Global_setup::Queue_toSet*       setup_set;
    Global_setup::Queue_toSave*     setup_save;

    class Rx: public Queue_prx {public:
        PacketizerObject &base;
        Rx(PacketizerObject &base): base(base){}
        int recv(){
            return base.recv();
        }
    } prx;
    class Tx: public Queue_ptx {public:
        PacketizerObject &base;
        Tx(PacketizerObject &base): base(base){}
        int send(int seq){
            return base.send(seq);
        }
    } ptx;
    class Sent: public Queue_psent {public:
        PacketizerObject &base;
        Sent(PacketizerObject &base): base(base){}

    } psent;

    U checksum(unsigned short* buff, int _16bitword)
    {
        unsigned long sum;
        for(sum=0;_16bitword>0;_16bitword--)
            sum+=htons(*(buff)++);
        sum = ((sum >> 16) + (sum & 0xFFFF));
        sum += (sum>>16);
        return (unsigned short)(~sum);
    }

    void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
        disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
        l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
        enable_wait(l2_transport_rx);
        enable_wait(l2_transport_sent);
    }

    void attach_Global_setup(Global_setup::Queue_toSave* save, Global_setup::Queue_toSet* set){
        setup_set = set; setup_save = save;
        disable_wait(setup_set); disable_wait(setup_save);
        enable_wait(setup_set);
    }

    PacketizerObject(WaitSystem* waitSystem, Packetizer::Setup &setup): WaitSystem::Module(waitSystem)
            , setted(false) , ready(false), setup(setup), prx(*this), ptx(*this), psent(*this)
    {
        module_debug = "PACKETIZER";
        rx = &prx;
        tx = &ptx;
        sent = &psent;
        enable_wait(tx);
    }

    int recv(){

    }
    int send(int seq){
        int total_len = 0;
        U8 buffer[packet.size];
        bzero(buffer, packet.size);
        //<ETH>
        #pragma pack(push, 1)
        struct ethheader *eth = (struct ethheader *)(buffer);
        memcpy(eth->src, packet.srcMAC, 6);
        memcpy(eth->dst, packet.dstMAC, 6);
        eth->protocol = htons(0x0800);
        //</ETH>
        //<IP>
        struct ipheader *iphdr = (struct ipheader*)(buffer + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->ver = 4;
        iphdr->tos = 16;
        iphdr->ident = htons(10201);
        iphdr->ttl = 64;
        iphdr->protocol = 17;
        iphdr->sourceip = packet.srcIP;
        iphdr->destip = packet.dstIP;
        iphdr->check = 0;
        //</IP>
        total_len += sizeof(struct ipheader);
        //<UDP>
        struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->src = htons(packet.srcPORT);
        udp->dst = htons(packet.dstPORT);
        udp->check = 0;
        //</UDP>
        total_len += sizeof(struct udpheader);
        //int dumm = 0;
        //for (int i = total_len + 1; i < packet.size; i++){
        //    buffer[i] = dumm % 10 + '0';
        //    dumm++;
        //}
        udp->len = htons((packet.size - sizeof(struct ipheader) - sizeof(struct ethheader)));
        iphdr->len = htons(packet.size - sizeof(struct ipheader));
        iphdr->check = checksum((unsigned short*)(buffer + sizeof(struct ethheader)), (sizeof(struct ipheader)/2));
        #pragma pack(pop)
        iovec iovec; iovec.iov_base = buffer; iovec.iov_len = packet.size;
        msghdr msg = {}; msg.msg_iov = &iovec; msg.msg_iovlen = 1;

        short status = l2_transport_tx->send(msg);
        if (status > 0) print("PACKET SENT!");


    }

    void evaluate(){
        if(!setted){
            str2mac(packet.srcMAC, "70:85:C2:C8:BF:25");
            str2mac(packet.dstMAC, "70:85:C2:C8:BF:25");
            packet.size = 1000;
            packet.dstPORT = 5850;
            packet.srcPORT = 5850;
            packet.srcIP = "192.168.1.207";
            packet.dstIP = "192.168.1.68";
            print("READY!");
            setted = true;
            enable_wait(l2_transport_tx);
        }
        while (WaitSystem::Queue* queue = enum_ready_queues()){
            if(queue == &l2_transport_tx){
                tx->setReady();
                disable_wait(l2_transport_tx);
            }
        }

    }
};
Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup){
    return new PacketizerObject(waitSystem, setup);
}
