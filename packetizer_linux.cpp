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

    UC checksum(unsigned short* buff, int _16bitword)
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
        enable_wait(l2_transport_tx);
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
        flags |= evaluate_every_cycle;
    }

    int recv(){

    }
    int send(int seq){
        int total_len = 0;
        U8 buffer[packet.size];
        bzero(buffer, packet.size);
        //<ETH>
        struct ethheader *eth = (struct ethheader *)(buffer);
        memcpy(eth->h_source, packet.srcMAC, 6);
        memcpy(eth->h_dest, packet.dstMAC, 6);
        eth->h_proto = htons(0x0800);
        //</ETH>
        total_len += sizeof(struct ethheader);
        //<IP>
        struct ipheader *iphdr = (struct ipheader*)(buffer + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->version = 4;
        iphdr->tos=16;
        iphdr->id = htons(10241);
        iphdr->ttl = 64;
        iphdr->protocol = IPPROTO_UDP;
        iphdr->saddr = inet_addr("192.168.1.207"); // Крайне временное решение
        iphdr->daddr = inet_addr("192.168.1.68");
        iphdr->check = 0;
        //</IP>
        total_len += sizeof(struct ipheader);
        //<UDP>
        struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->source = htons(packet.srcPORT);
        udp->dest = htons(packet.dstPORT);
        udp->check = 0;
        //</UDP>
        total_len += sizeof(struct udpheader);
        //<RTT>
        struct rttheader *rtt = (struct rttheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader));
        rtt->sequence = seq;
        //</RTT>
        total_len += sizeof(struct rttheader);
        udp->len = htons((packet.size- sizeof(struct ipheader) - sizeof(struct ethheader)));
        iphdr->tot_len = htons(packet.size - sizeof(struct ethheader));
        for (int i = total_len + 1; i < packet.size; i++){
            buffer[i] = i % 10 + 48;
        }

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
            packet.dstPORT = 44031;
            packet.srcPORT = 44031;
            print("READY!");
            setted = true;
            tx->setReady();
        }
        while (WaitSystem::Queue* queue = enum_ready_queues()){
            if(queue == &l2_transport_tx){
                print("READY");
                tx->setReady();
                disable_wait(l2_transport_tx);
            }
        }

    }
};
Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup){
    return new PacketizerObject(waitSystem, setup);
}
